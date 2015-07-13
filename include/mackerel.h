/**
 * Taken from barrelfish os.
 */

/*
 * Copyright (c) 2008, 2009, ETH Zurich.
 * All rights reserved.
 *
 * This file is distributed under the terms in the attached LICENSE file.
 * If you do not find this file, copies can be found by writing to:
 * ETH Zurich D-INFK, Haldeneggsteig 4, CH-8092 Zurich. Attn: Systems Group.
 */

#ifndef _NUGPGPU_MACKEREL_H
#define _NUGPGPU_MACKEREL_H

#include <linux/types.h>

/*
 * Device address space types
 */
typedef char    *mackerel_addr_t;

/*
 * Reading from memory
 */
inline uint8_t mackerel_read_addr_8( mackerel_addr_t base, int offset);
inline uint16_t mackerel_read_addr_16( mackerel_addr_t base, int offset);
inline uint32_t mackerel_read_addr_32( mackerel_addr_t base, int offset);
inline uint64_t mackerel_read_addr_64( mackerel_addr_t base, int offset);

/*
 * Writing to memory
 */
inline void mackerel_write_addr_8( mackerel_addr_t base, int offset, uint8_t v);
inline void mackerel_write_addr_16( mackerel_addr_t base, int offset, uint16_t v);
inline void mackerel_write_addr_32( mackerel_addr_t base, int offset, uint32_t v);
inline void mackerel_write_addr_64( mackerel_addr_t base, int offset, uint64_t v);
#endif 
