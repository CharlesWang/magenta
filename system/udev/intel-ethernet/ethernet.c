// Copyright 2016 The Fuchsia Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <ddk/device.h>
#include <ddk/driver.h>
#include <ddk/binding.h>
#include <ddk/protocol/pci.h>
#include <ddk/protocol/ethernet.h>
#include <hw/pci.h>

#include <mxu/list.h>

#include <runtime/thread.h>
#include <runtime/mutex.h>

#include <magenta/syscalls.h>
#include <magenta/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef mx_status_t status_t;
#include "ie.h"

#define INTERVAL 10000000000ULL

typedef struct ethernet_device ethernet_device_t;

struct ethernet_device {
    ethdev_t eth;
    mxr_mutex_t lock;
    mx_device_t dev;
    pci_protocol_t* pci;
    mx_device_t* pcidev;
    mx_handle_t ioh;
    mx_handle_t irqh;
    mxr_thread_t* thread;
};

#define get_eth_device(d) containerof(d, ethernet_device_t, dev)

static int irq_thread(void* arg) {
    ethernet_device_t* edev = arg;
    for (;;) {
        mx_status_t r;
        if ((r = _magenta_pci_interrupt_wait(edev->irqh)) < 0) {
            printf("eth: irq wait failed? %d\n", r);
            break;
        }
        mxr_mutex_lock(&edev->lock);
        if (eth_handle_irq(&edev->eth) & ETH_IRQ_RX) {
            device_state_set(&edev->dev, DEV_STATE_READABLE);
        }
        mxr_mutex_unlock(&edev->lock);
    }
    return 0;
}

static mx_status_t eth_recv(mx_device_t* dev, void* data, size_t len) {
    ethernet_device_t* edev = get_eth_device(dev);
    mx_status_t r = ERR_BAD_STATE;
    mxr_mutex_lock(&edev->lock);
    r = eth_rx(&edev->eth, data);
    if (r <= 0) {
        device_state_clr(dev, DEV_STATE_READABLE);
    }
    mxr_mutex_unlock(&edev->lock);
    return r;
}

static mx_status_t eth_send(mx_device_t* dev, const void* data, size_t len) {
    ethernet_device_t* edev = get_eth_device(dev);
    if (len > ETH_TXBUF_DSIZE) {
        return ERR_INVALID_ARGS;
    }
    mx_status_t r = len;
    mxr_mutex_lock(&edev->lock);
    r = eth_tx(&edev->eth, data, len);
    mxr_mutex_unlock(&edev->lock);
    return r;
}

static mx_status_t eth_get_mac_addr(mx_device_t* dev, uint8_t* out_addr) {
    ethernet_device_t* edev = get_eth_device(dev);
    memcpy(out_addr, edev->eth.mac, sizeof(edev->eth.mac));
    return NO_ERROR;
}

static bool eth_is_online(mx_device_t* dev) {
    return true;
}

static size_t eth_get_mtu(mx_device_t* dev) {
    return ETH_RXBUF_SIZE;
}

static ethernet_protocol_t ethernet_ops = {
    .send = eth_send,
    .recv = eth_recv,
    .get_mac_addr = eth_get_mac_addr,
    .is_online = eth_is_online,
    .get_mtu = eth_get_mtu,
};

static mx_status_t eth_release(mx_device_t* dev) {
    ethernet_device_t* edev = get_eth_device(dev);
    eth_reset_hw(&edev->eth);
    edev->pci->enable_bus_master(edev->pcidev, true);
    _magenta_handle_close(edev->irqh);
    _magenta_handle_close(edev->ioh);
    free(dev);
    return ERR_NOT_SUPPORTED;
}

static mx_protocol_device_t device_ops = {
    .release = eth_release,
};

static mx_status_t eth_bind(mx_driver_t* drv, mx_device_t* dev) {
    ethernet_device_t* edev;
    if ((edev = calloc(1, sizeof(ethernet_device_t))) == NULL) {
        return ERR_NO_MEMORY;
    }
    edev->lock = MXR_MUTEX_INIT;

    pci_protocol_t* pci;
    if (device_get_protocol(dev, MX_PROTOCOL_PCI, (void**)&pci)) {
        printf("no pci protocol\n");
        goto fail;
    }
    edev->pcidev = dev;
    edev->pci = pci;

    mx_status_t r;
    if ((r = pci->claim_device(dev)) < 0) {
        return r;
    }

    if (pci->set_irq_mode(dev, MX_PCIE_IRQ_MODE_MSI, 1)) {
        if (pci->set_irq_mode(dev, MX_PCIE_IRQ_MODE_LEGACY, 1)) {
            printf("eth: failed to set irq mode\n");
            goto fail;
        } else {
            printf("eth: using legacy irq mode\n");
        }
    }
    if ((edev->irqh = pci->map_interrupt(dev, 0)) < 0) {
        printf("eth: failed to map irq\n");
        goto fail;
    }

    // map iomem
    uint64_t sz;
    mx_handle_t h;
    void *io;
    if ((h = pci->map_mmio(dev, 0, MX_CACHE_POLICY_UNCACHED_DEVICE, &io, &sz)) < 0) {
        printf("eth: cannot map io %d\n", h);
        goto fail;
    }
    edev->eth.iobase = (uintptr_t) io;
    edev->ioh = h;

    if ((r = pci->enable_bus_master(dev, true)) < 0) {
        printf("eth: cannot enable bus master %d\n", r);
        goto fail;
    }

    if (eth_reset_hw(&edev->eth)) {
        goto fail;
    }

    mx_paddr_t iophys;
    void* iomem;
    if ((r = _magenta_alloc_device_memory(ETH_ALLOC, &iophys, &iomem)) < 0) {
        printf("eth: cannot alloc buffers %d\n", r);
        goto fail;
    }

    eth_setup_buffers(&edev->eth, iomem, iophys);
    eth_init_hw(&edev->eth);

    if (device_init(&edev->dev, drv, "intel-ethernet", &device_ops)) {
        goto fail;
    }
    edev->dev.protocol_id = MX_PROTOCOL_ETHERNET;
    edev->dev.protocol_ops = &ethernet_ops;
    if (device_add(&edev->dev, dev)) {
        goto fail;
    }

    mxr_thread_create(irq_thread, edev, "eth-irq-thread", &edev->thread);
    mxr_thread_detach(edev->thread);

    return NO_ERROR;

fail:
    if (edev->ioh) {
        edev->pci->enable_bus_master(edev->pcidev, true);
        _magenta_handle_close(edev->irqh);
        _magenta_handle_close(edev->ioh);
    }
    free(edev);
    return ERR_NOT_SUPPORTED;
}

//TODO: figure out more complete set of supported DIDs
static mx_bind_inst_t binding[] = {
    BI_ABORT_IF(NE, BIND_PROTOCOL, MX_PROTOCOL_PCI),
    BI_ABORT_IF(NE, BIND_PCI_VID, 0x8086),
    BI_MATCH_IF(EQ, BIND_PCI_DID, 0x100E), // Qemu
    BI_MATCH_IF(EQ, BIND_PCI_DID, 0x15A3), // Broadwell
    BI_MATCH_IF(EQ, BIND_PCI_DID, 0x1570), // Skylake
    BI_ABORT(),
};

mx_driver_t _driver_intel_ethernet BUILTIN_DRIVER = {
    .name = "intel-ethernet",
    .ops = {
        .bind = eth_bind,
    },
    .binding = binding,
    .binding_size = sizeof(binding),
};