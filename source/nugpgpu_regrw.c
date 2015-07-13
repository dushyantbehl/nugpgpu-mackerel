/*
 * nugpgpu: Register read / write code
 * Based on i915 driver source code.
 */
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/spinlock.h>

#include "nugpgpu_misc.h"
#include "nugpgpu_regrw.h"
#include "nugpgpu_drv.h"

#define NEEDS_FORCEWAKE(reg) \
        ((reg) < nugpgpu_FORCEWAKE_MAX_OFFSET && (reg) != nugpgpu_FORCEWAKE)

static void
gpu_unclaimed_reg_debug(struct nugpgpu_private *gpu_priv, uint32_t reg_offset, bool read, bool before)
{
  const char *op = read ? "reading" : "writing to";
  const char *when = before ? "before" : "after";

  if( nugpgpu_FPGA_DBG_rd(&gpu_priv->nugpgpu_dev ) & nugpgpu_FPGA_DBG_RM_NOCLAIM) {
    printk(LOG_WARN "Unclaimed register called by nugpgpu %s %s register 0x%x\n" LOG_END, when, op, reg_offset);
    nugpgpu_FPGA_DBG_wr(&gpu_priv->nugpgpu_dev, nugpgpu_FPGA_DBG_RM_NOCLAIM);
  }
}

/* 
 * Refer to this thread for information on forcewake shenanigans.
 * http://www.spinics.net/lists/intel-gfx/msg57987.html
 * */

void gpu_forcewake_get(struct nugpgpu_private *gpu_priv)
{
  unsigned long irqflags;
  TRACE_IN

  spin_lock_irqsave(&gpu_priv->mmio_funcs.lock, irqflags);
  if (gpu_priv->forcewake_count++ == 0){
    printk(LOG_INFO "going to call the forcewake get\n" LOG_END);
    forcewake_get(gpu_priv, 0, true);
  }
  spin_unlock_irqrestore(&gpu_priv->mmio_funcs.lock, irqflags);
  TRACE_OUT
}

void gpu_forcewake_put(struct nugpgpu_private *gpu_priv)
{
  unsigned long irqflags;
  TRACE_IN

  spin_lock_irqsave(&gpu_priv->mmio_funcs.lock, irqflags);
  if (--gpu_priv->forcewake_count == 0){
    printk(LOG_INFO "going to call the forcewake work\n" LOG_END);
    forcewake_put(gpu_priv, 0, true);
  }
  spin_unlock_irqrestore(&gpu_priv->mmio_funcs.lock, irqflags);
  TRACE_OUT
}


void forcewake_get(struct nugpgpu_private *gpu_priv, uint32_t reg_offset, bool force)
{
  TRACE_IN
  if( force || NEEDS_FORCEWAKE(reg_offset) ){

    if( wait_for_atomic(((nugpgpu_FORCEWAKE_ACK_HSW_rd( &gpu_priv->nugpgpu_dev ) 
                          & nugpgpu_FORCEWAKE_KERNEL) == 0),
                          nugpgpu_FORCEWAKE_ACK_TIMEOUT_MS) )
      printk(LOG_ERR "Forcewake old clear timed out\n" LOG_END);

    nugpgpu_FORCEWAKE_MT_wr(&gpu_priv->nugpgpu_dev, 
                            _MASKED_BIT_ENABLE(nugpgpu_FORCEWAKE_KERNEL));

    nugpgpu_ECOBUS_rd(&gpu_priv->nugpgpu_dev);

    if(wait_for_atomic((nugpgpu_FORCEWAKE_ACK_HSW_rd( &gpu_priv->nugpgpu_dev)
                        & nugpgpu_FORCEWAKE_KERNEL),
                        nugpgpu_FORCEWAKE_ACK_TIMEOUT_MS))
      printk(LOG_ERR "Forcewake new clear timed out\n" LOG_END);

    if(wait_for_atomic_us((nugpgpu_GEN6_GT_THREAD_STATUS_REG_rd( &gpu_priv->nugpgpu_dev)
                           & nugpgpu_HSW_GT_THREAD_STATUS_CORE_MASK) == 0, 500))
      printk(LOG_ERR "GT thread waiting status timed out\n" LOG_END);
  }
  TRACE_OUT
}

void forcewake_put(struct nugpgpu_private *gpu_priv, uint32_t reg_offset, bool force)
{
  uint32_t gtfifodbg;
  TRACE_IN
  if( force || NEEDS_FORCEWAKE(reg_offset) ){
    nugpgpu_FORCEWAKE_MT_wr(&gpu_priv->nugpgpu_dev, _MASKED_BIT_DISABLE(nugpgpu_FORCEWAKE_KERNEL));
    nugpgpu_ECOBUS_rd(&gpu_priv->nugpgpu_dev);

    gtfifodbg = nugpgpu_GTFIFODBG_rd(&gpu_priv->nugpgpu_dev);
    if ( gtfifodbg & nugpgpu_GT_FIFO_CPU_ERROR_MASK ){
        printk(LOG_WARN "MMIO read/write has been dropped GTFIFODBG - %x\n" LOG_END, gtfifodbg);
        nugpgpu_GTFIFODBG_wr(&gpu_priv->nugpgpu_dev, nugpgpu_GT_FIFO_CPU_ERROR_MASK);
    }
  }
  TRACE_OUT
}

#define __gpu_read(x) \
static u##x \
_gpu_read##x(struct nugpgpu_private *gpu_priv, gpu_read##x reg_read, off_t reg) {\
  unsigned long irqflags; \
  u##x val = 0; \
  spin_lock_irqsave(&gpu_priv->mmio_funcs.lock, irqflags); \
  if (gpu_priv->forcewake_count == 0) \
    forcewake_get(gpu_priv, reg, false); \
  val = reg_read( &gpu_priv->nugpgpu_dev ); \
  if (gpu_priv->forcewake_count == 0) \
    forcewake_put(gpu_priv, reg, false); \
  spin_unlock_irqrestore(&gpu_priv->mmio_funcs.lock, irqflags); \
  return val; \
}

#define __gpu_write(x) \
static void \
_gpu_write##x(struct nugpgpu_private *gpu_priv, gpu_write##x reg_write, off_t reg, u##x val) {\
  unsigned long irqflags; \
  spin_lock_irqsave(&gpu_priv->mmio_funcs.lock, irqflags); \
  gpu_unclaimed_reg_debug(gpu_priv, reg, false, true); \
  reg_write(&gpu_priv->nugpgpu_dev, val); \
  gpu_unclaimed_reg_debug(gpu_priv, reg, false, false); \
  spin_unlock_irqrestore(&gpu_priv->mmio_funcs.lock, irqflags); \
}

__gpu_read(8)
__gpu_read(16)
__gpu_read(32)
__gpu_read(64)

__gpu_write(8)
__gpu_write(16)
__gpu_write(32)
__gpu_write(64)


void nugpgpu_regrw_init(struct nugpgpu_private *gpu_priv)
{
  printk(LOG_INFO "nugpgpu_regrw_init\n" LOG_END);
  TRACE_IN

  spin_lock_init(&gpu_priv->mmio_funcs.lock);

  gpu_priv->mmio_funcs.mmio_readb = _gpu_read8;
  gpu_priv->mmio_funcs.mmio_readw = _gpu_read16;
  gpu_priv->mmio_funcs.mmio_readl = _gpu_read32;
  gpu_priv->mmio_funcs.mmio_readq = _gpu_read64;

  gpu_priv->mmio_funcs.mmio_writeb = _gpu_write8;
  gpu_priv->mmio_funcs.mmio_writew = _gpu_write16;
  gpu_priv->mmio_funcs.mmio_writel = _gpu_write32;
  gpu_priv->mmio_funcs.mmio_writeq = _gpu_write64;

  TRACE_OUT
}
