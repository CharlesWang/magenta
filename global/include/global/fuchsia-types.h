// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is automatically generated by gen-types.sh. Do not edit by hand.

#pragma once

#define NO_ERROR (0)

// ======= Internal failures =======
#define ERR_INTERNAL (-1)

// ERR_NOT_SUPPORTED: subject of the operation does not support the operation
#define ERR_NOT_SUPPORTED (-2)

// ERR_NOT_FOUND: requested entity is not found
#define ERR_NOT_FOUND (-3)

// ERR_NO_MEMORY: memory allocation failed
#define ERR_NO_MEMORY (-4)

// ERR_NO_RESOURCES: allocation of resource other than memory failed (handle (-id, etc)
#define ERR_NO_RESOURCES (-5)

// ======= Parameter errors =======
// ERR_INVALID_ARGS: an argument is invalid (-ex. null pointer
#define ERR_INVALID_ARGS (-10)

// ERR_WRONG_TYPE: The subject of the operation is the wrong type to perform the operation.
// Example: Attempting a message_read on a thread handle.
#define ERR_WRONG_TYPE (-54)

// ERR_BAD_SYSCALL: syscall number is invalid
#define ERR_BAD_SYSCALL (-11)

// ERR_BAD_HANDLE: handle number does not refer to valid handle in your process
#define ERR_BAD_HANDLE (-12)

// ERR_OUT_OF_RANGE: an argument is outside of the allowed range for that parameter
// the allowed range could change with the state of the system
#define ERR_OUT_OF_RANGE (-13)

// ERR_NOT_ENOUGH_BUFFER: the buffer provided by the caller is too small for the requested operation
#define ERR_NOT_ENOUGH_BUFFER (-14)

// ERR_ALREADY_EXISTS: attempting to create an entity that already exists at that name or id
#define ERR_ALREADY_EXISTS (-15)

// ======= Precondition or state errors =======
// ERR_BAD_STATE: operation failed because the current state of the object does not allow it (-
// or a precondition of the operation is not satisfied
#define ERR_BAD_STATE (-20)

// ERR_FLOW_CONTROL:
//   write cannot complete due to temporary buffer full
//   read cannot complete due to temporary no data to read
#define ERR_FLOW_CONTROL (-21)    // XXX argue specific name later

// ERR_NOT_READY: subject of the operation is temporary unavailable
// expected to transition to operational or permanent failure
#define ERR_NOT_READY (-22)

// ERR_TIMED_OUT: time out for the operation expired before the condition became true
#define ERR_TIMED_OUT (-23)

// ERR_HANDLE_CLOSED: a handle being waited on was closed
#define ERR_HANDLE_CLOSED (-24)

// TODO: Remove from userspace in favor of HANDLE_CLOSED and update kernel internal uses to more
// specific code(s).
#define ERR_CANCELLED (-24)

// ERR_CHANNEL_CLOSED: operation will never succeed because other end does not exist
#define ERR_CHANNEL_CLOSED (-25)   // RENAME to ERR_REMOTE_CLOSED

// ERR_NOT_AVAILABLE: object is owned or controlled by another entity
#define ERR_NOT_AVAILABLE (-26)    // NEW

// ======= Permission check errors =======
#define ERR_ACCESS_DENIED (-30)

// Input-output errors
#define ERR_IO (-40)
// TODO: Make more generic (ERR_IO_NACK?)
#define ERR_I2C_NACK (-41)   // kernel only, remove or find better error
// TODO: Rename to something more generic like ERR_DATA_INTEGRITY
    // operation failed due to data being detected as corrupt or invalid
#define ERR_CHECKSUM_FAIL (-42)   // RENAME to DATA_INTEGRITY

// Filesystem specific errors
#define ERR_BAD_PATH (-50)
#define ERR_NOT_DIR (-51)
#define ERR_NOT_FILE (-52)
// TODO: Confusing name - is this the same as POSIX ELOOP?
#define ERR_RECURSE_TOO_DEEP (-53)


// Garbage bin
//////////////////////////////////////
// TODO: Replace with INVALID_ARGS
#define ERR_NOT_VALID (-90)

// TODO: Should just be NOT_SUPPORTED
#define ERR_NOT_IMPLEMENTED (-91)

// TODO: Replace with ERR_INVALID_ARGS or ERR_NOT_ENOUGH_BUFFER
#define ERR_TOO_BIG (-92)

// TODO: This appears to be used as a bool (-does it need a distinct code?
#define ERR_FAULT (-93)

// TODO: Replace with either ACCESS_DENIED or NOT_SUPPORTED as appropriate
#define ERR_NOT_ALLOWED (-94)

// TODO: These all seem like state errors (-should they just be ERR_BAD_STATE?
#define ERR_ALREADY_STARTED (-95)
#define ERR_NOT_BLOCKED (-96)
#define ERR_THREAD_DETACHED (-97)

// TODO: This is a variant of ERR_BAD_STATE
#define ERR_NOT_MOUNTED (-98)

#define ERR_BUSY (-99)   // REMOVE - kernel/pcie should transition to ERR_NOT_READY/ERR_NOT_AVAILABLE
                                        // futex should return BAD_STATE
                                        // io port thing should return NOT_AVAILABLE
                                        // pcidevicedispatcher should return NOT_AVAILABLE
                                        // no ERR_BUSY from syscalls

// USER SPECIFIC
/////////////////////
#define ERR_USER_BASE (-16384)
