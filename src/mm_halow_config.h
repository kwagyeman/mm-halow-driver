/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 OpenMV LLC.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Host integration contract for the mm-halow driver: everything the driver
 * needs from the embedding environment is declared here, and the embedder
 * provides it in mm_halow_configport.h (or the file named by
 * MM_HALOW_CONFIG_FILE).  This mirrors the cyw43-driver configuration scheme.
 */
#ifndef MM_HALOW_INCLUDED_CONFIG_H
#define MM_HALOW_INCLUDED_CONFIG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Import port-specific configuration file.
#ifdef MM_HALOW_CONFIG_FILE
#include MM_HALOW_CONFIG_FILE
#else
#include <mm_halow_configport.h>
#endif

/*******************************************************************************/
// Hooks the port must provide (no usable defaults).

// MM_HALOW_BEGIN_ATOMIC_SECTION() -> uintptr_t state
// MM_HALOW_END_ATOMIC_SECTION(state)
//   Mask/restore interrupts around the driver's short critical sections.
#ifndef MM_HALOW_BEGIN_ATOMIC_SECTION
#error "port must define MM_HALOW_BEGIN_ATOMIC_SECTION/MM_HALOW_END_ATOMIC_SECTION"
#endif

// mm_halow_ticks_ms() -> uint32_t
//   A free-running millisecond tick counter.
#ifndef mm_halow_ticks_ms
#error "port must define mm_halow_ticks_ms"
#endif

// Pin accessors, applied to the MM_HALOW_CS/RESET/WAKE/BUSY/IRQ pin values
// defined by the port.
#ifndef mm_halow_hal_pin_write
#error "port must define the mm_halow_hal_pin_read/write/input/output accessors"
#endif

// mm_halow_in_irq() -> bool
//   True when executing in interrupt context (on Cortex-M: IPSR != 0).
#ifndef mm_halow_in_irq
#error "port must define mm_halow_in_irq"
#endif

/*******************************************************************************/
// Hooks with defaults.

// Runs while the driver busy-waits, so the host can service its own pending
// events.  It must not raise/longjmp out of the driver.
#ifndef MM_HALOW_EVENT_POLL_HOOK
#define MM_HALOW_EVENT_POLL_HOOK
#endif

// Diagnostic output.
#ifndef MM_HALOW_PRINTF
#include <stdio.h>
#define MM_HALOW_PRINTF(...) printf(__VA_ARGS__)
#endif
#ifndef MM_HALOW_VPRINTF
#include <stdarg.h>
#include <stdio.h>
#define MM_HALOW_VPRINTF(fmt, args) vprintf(fmt, args)
#endif

#ifndef MM_HALOW_WEAK
#define MM_HALOW_WEAK __attribute__((weak))
#endif

// Error codes returned by the driver API (negated).  Default to the C library
// values; an embedder with its own errno space overrides these.
#ifndef MM_HALOW_EPERM
#include <errno.h>
#define MM_HALOW_EPERM      EPERM
#define MM_HALOW_EIO        EIO
#define MM_HALOW_EINVAL     EINVAL
#define MM_HALOW_EAGAIN     EAGAIN
#define MM_HALOW_ENOMEM     ENOMEM
#define MM_HALOW_ENODEV     ENODEV
#define MM_HALOW_ENOENT     ENOENT
#define MM_HALOW_ENOTCONN   ENOTCONN
#define MM_HALOW_ENXIO      ENXIO
#define MM_HALOW_ETIMEDOUT  ETIMEDOUT
#define MM_HALOW_ERANGE     ERANGE
#define MM_HALOW_EOPNOTSUPP EOPNOTSUPP
#endif

// Hostname reported to the DHCP server (a char pointer or array).
#ifndef MM_HALOW_HOST_NAME
#define MM_HALOW_HOST_NAME "mm-halow"
#endif

/*******************************************************************************/
// Functions the port must implement (see README.md).

// SPI bus: mode 0, MSB first, MM_HALOW_SPI_BAUDRATE.  The chip select is a
// plain GPIO (MM_HALOW_CS) driven by the driver, not by the SPI peripheral.
void mm_halow_port_spi_init(void);
void mm_halow_port_spi_deinit(void);
void mm_halow_port_spi_transfer(size_t len, const uint8_t *src, uint8_t *dest);

// The backing memory for the driver's private heap (MM_HALOW_HEAP_SIZE bytes).
// The port owns keeping the allocation alive (e.g. registering it as a GC root
// on a garbage-collected host).
uint8_t *mm_halow_port_heap_alloc(size_t size);
void mm_halow_port_heap_free(uint8_t *ptr);

// A hardware random 32-bit value.
uint32_t mm_halow_port_random_u32(void);

// Fallback station MAC address, used only when the transceiver's OTP holds
// none.  Must be stable across boots.
void mm_halow_port_get_mac(uint8_t mac_addr[6]);

#if MM_HALOW_PIN_IRQ
// Optional falling-edge interrupt on the IRQ line.  The port's ISR must call
// mm_halow_port_irq_handler(); the driver enables/disables delivery with
// mm_halow_port_irq_enable() to coalesce bursts.
void mm_halow_port_irq_config(bool enabled);
void mm_halow_port_irq_enable(bool enabled);
void mm_halow_port_irq_handler(void);
#endif

#endif // MM_HALOW_INCLUDED_CONFIG_H
