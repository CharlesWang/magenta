// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <mxio/vfs.h>
#include <system/listnode.h>

typedef struct dnode dnode_t;

#define DN_NAME_MAX 255
#define DN_NAME_INLINE 47
#define DN_NAME_LEN(flags) ((flags) & 0xFF)

#define DN_TYPE_MASK    0xF00
#define DN_TYPE_DIR     0x100
#define DN_TYPE_FILE    0x200
#define DN_TYPE_DEVICE  0x300
#define DN_TYPE_SYMLINK 0x400
#define DN_TYPE(flags) ((flags) & DN_TYPE_MASK)

struct dnode {
    dnode_t* parent;
    const char* name;
    uint32_t flags;
    uint32_t refcount;
    vnode_t* vnode;
    list_node_t children;
    list_node_t dn_entry; // entry in parent's list
    list_node_t vn_entry; // entry in vnode's list
    char namedata[DN_NAME_INLINE + 1];
};

mx_status_t dn_create(dnode_t** dn, const char* name, size_t len, vnode_t* vn);
void dn_delete(dnode_t* dn);

#if 0
static inline void dn_acquire(dnode_t* dn) {
    dn->refcount++;
}
void dn_release(dnode_t* dn);
#endif

mx_status_t dn_lookup(dnode_t* dn, dnode_t** out, const char* name, size_t len);

void dn_add_child(dnode_t* parent, dnode_t* child);

mx_status_t dn_readdir(dnode_t* parent, void* cookie, void* data, size_t len);
