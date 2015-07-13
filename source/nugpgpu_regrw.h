/*
 * nugpgpu: Register read / write code
 * Based on i915 driver source code.
 */
#ifndef __NUGPGPU_REGRW_H__
#define __NUGPGPU_REGRW_H__

#include <linux/spinlock_types.h>
#include "nugpgpu_misc.h"
#include "../generated/nugpgpu_dev.h"

struct nugpgpu_private;

typedef uint8_t  (*gpu_read8) (struct nugpgpu_t *);
typedef uint16_t (*gpu_read16) (struct nugpgpu_t *);
typedef uint32_t (*gpu_read32) (struct nugpgpu_t *);
typedef uint64_t (*gpu_read64) (struct nugpgpu_t *);

typedef void (*gpu_write8) (struct nugpgpu_t *, uint8_t );
typedef void (*gpu_write16) (struct nugpgpu_t *, uint16_t);
typedef void (*gpu_write32) (struct nugpgpu_t *, uint32_t);
typedef void (*gpu_write64) (struct nugpgpu_t *, uint64_t);

struct gpu_mmio_funcs {
  spinlock_t lock;

  uint8_t  (*mmio_readb)(struct nugpgpu_private *, gpu_read8  , off_t);
  uint16_t (*mmio_readw)(struct nugpgpu_private *, gpu_read16 , off_t);
  uint32_t (*mmio_readl)(struct nugpgpu_private *, gpu_read32 , off_t);
  uint64_t (*mmio_readq)(struct nugpgpu_private *, gpu_read64 , off_t);

  void (*mmio_writeb)(struct nugpgpu_private *, gpu_write8  , off_t, uint8_t);
  void (*mmio_writew)(struct nugpgpu_private *, gpu_write16 , off_t, uint16_t);
  void (*mmio_writel)(struct nugpgpu_private *, gpu_write32 , off_t, uint32_t);
  void (*mmio_writeq)(struct nugpgpu_private *, gpu_write64 , off_t, uint64_t);
};

void nugpgpu_regrw_init(struct nugpgpu_private *gpu_priv);

void nugpgpu_regrw_init(struct nugpgpu_private *gpu_priv);

void gpu_forcewake_get(struct nugpgpu_private *gpu_priv);
void gpu_forcewake_put(struct nugpgpu_private *gpu_priv);

void forcewake_get(struct nugpgpu_private *gpu_priv, uint32_t reg, bool force);
void forcewake_put(struct nugpgpu_private *gpu_priv, uint32_t reg, bool force);

#define NUGPGPU_READ(reg)       \
          gpu_priv->mmio_funcs.mmio_readl(gpu_priv, reg##_rd , reg)
#define NUGPGPU_WRITE(reg, val) \
          gpu_priv->mmio_funcs.mmio_writel(gpu_priv, reg##_wr , reg, (val))

#define NUGPGPU_READQ(reg)       \
          gpu_priv->mmio_funcs.mmio_readq(gpu_priv, reg##_rd , reg)
#define NUGPGPU_WRITEQ(reg_read, reg, val) \
          gpu_priv->mmio_funcs.mmio_writeq(gpu_priv, reg##_wr , reg, (val))
#endif
