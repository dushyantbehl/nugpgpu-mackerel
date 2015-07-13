#ifndef __NUGPGPU_MISC_H_
#define __NUGPGPU_MISC_H_

#include <linux/delay.h>
#include <linux/cdev.h>

#define _MASKED_BIT_ENABLE(a)   (((a) << 16) | (a))
#define _MASKED_BIT_DISABLE(a)  ((a) << 16)

// Taken from i915/drv.h

/**
 * _wait_for - magic (register) wait macro
 *
 * Does the right thing for modeset paths when run under kdgb or similar atomic
 * contexts. Note that it's important that we check the condition again after
 * having timed out, since the timeout could be due to preemption or similar and
 * we've never had a chance to check the condition before the timeout.
 **/
#define _wait_for(COND, MS, W) ({ \
  unsigned long timeout__ = jiffies + msecs_to_jiffies(MS) + 1;   \
  int ret__ = 0;                                                  \
  while (!(COND)) {                                               \
          if (time_after(jiffies, timeout__)) {                   \
                  if (!(COND))                                    \
                          ret__ = -ETIMEDOUT;                     \
                  break;                                          \
          }                                                       \
          if (W)  {                                               \
                  msleep(W);                                      \
          } else {                                                \
                  cpu_relax();                                    \
          }                                                       \
  }                                                               \
  ret__;                                                          \
})

#define wait_for(COND, MS) _wait_for(COND, MS, 1)
#define wait_for_atomic(COND, MS) _wait_for(COND, MS, 0)
#define wait_for_atomic_us(COND, US) _wait_for((COND), DIV_ROUND_UP((US), 1000), 0)


#endif
