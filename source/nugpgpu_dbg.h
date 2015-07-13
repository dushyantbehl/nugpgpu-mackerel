#ifndef __NUGPGPU_DBG_H__
#define __NUGPGPU_DBG_H__

#include "nugpgpu_drv.h"
#include "nugpgpu_regrw.h"

#define INTENT_FACTOR 4

/* Logging macro to be used for nugpgpu */
#define LOG_INFO    KERN_INFO"nugpgpu: %*s"
#define LOG_ERR     KERN_ERR"nugpgpu: %*s"
#define LOG_WARN    KERN_WARNING"nugpgpu: %*s"
#define LOG_END     , depth*INTENT_FACTOR, ""

#define LOG_INFO_EMPTY  printk(LOG_INFO "\n" LOG_END);
#define LOG_ERR_EMPTY  printk(LOG_ERR "\n" LOG_END);
#define LOG_WARN_EMPTY  printk(LOG_WARN "\n" LOG_END);

#define TRACE_INIT  depth = 0; LOG_INFO_EMPTY LOG_ERR_EMPTY LOG_WARN_EMPTY
#define TRACE_IN    depth++;
#define TRACE_OUT   depth--;

/* 
 * Prints the access to a register from the given function.
 * Also prints the value of caller and the arguments to read/write.
 */
#define PRINT_REG(REG,VAL) do{  \
            if ( debug ) {      \
              printk(LOG_INFO "%pS\t %s(register - 0x%u, value - 0x%u)\n" LOG_END,     \
                     __builtin_return_address(0), __func__,(unsigned)REG,(unsigned)VAL);\
            }     \
        } while(0);

#define shout(S, ...) printk(LOG_INFO "\ti915 shout:: %s:%d - " S LOG_END,__FILENAME__,__LINE__, ##__VA_ARGS__);

extern int depth;
extern int debug;

void nugpgpu_dump(struct nugpgpu_private *gpu_priv);

#endif
