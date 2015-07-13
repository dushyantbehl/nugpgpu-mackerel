/**
 * mackerel read/write functions generated separetly because of linux issue -
 * asm-generic/io.h and asm/io.h both generate readX and writeX functions.
 */

#include "mackerel.h"

#include <asm-generic/io.h>
/*
 * Reading from memory
 */
inline uint8_t mackerel_read_addr_8( mackerel_addr_t base, int offset)
{
    volatile uint8_t p = (volatile uint8_t) readb(base + offset);
    return p;
}

inline uint16_t mackerel_read_addr_16( mackerel_addr_t base, int offset)
{
    volatile uint16_t p = (volatile uint16_t) readw(base + offset);
    return p;
}

inline uint32_t mackerel_read_addr_32( mackerel_addr_t base, int offset)
{
    volatile uint32_t p = (volatile uint32_t) readl(base + offset);
    return p;
}

inline uint64_t mackerel_read_addr_64( mackerel_addr_t base, int offset)
{
    volatile uint64_t p = (volatile uint64_t) readq(base + offset);
    return p;
}

/*
 * Writing to memory
 */
inline void mackerel_write_addr_8( mackerel_addr_t base,
                                             int offset, uint8_t v)
{
    writeb(v, (void *)(base + offset));
}

inline void mackerel_write_addr_16( mackerel_addr_t base,
                                               int offset, uint16_t v)
{
    writew(v, (void *)(base + offset));
}

inline void mackerel_write_addr_32( mackerel_addr_t base,
                                               int offset, uint32_t v)
{
    writel(v, (void *)(base + offset));
}

inline void mackerel_write_addr_64( mackerel_addr_t base,
                                               int offset, uint64_t v)
{
    writeq(v, (void *)(base + offset));
}
