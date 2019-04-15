/*
 * This file is part of the CmBacktrace Library.
 *
 * Copyright (c) 2016-2018, zylx, <1346773219@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Initialize function and other general function.
 * Created on: 2016-12-15
 */
#include <rtthread.h>
#include <rthw.h>
#include <cm_backtrace.h>
#include <string.h>

#ifndef CMB_LR_WORD_OFFSET
#define CMB_LR_WORD_OFFSET   6
#endif 

#define CMB_SP_WORD_OFFSET   (CMB_LR_WORD_OFFSET + 1)

#if defined(__CC_ARM)
    #pragma O1
#elif defined(__ICCARM__)
    #pragma optimize=none
#elif defined(__GNUC__)
    #pragma GCC optimize ("O0")
#endif

#if defined(__CC_ARM)
    static __inline __asm void cmb_set_psp(uint32_t psp) {
        msr psp, r0
    }
#elif defined(__ICCARM__)
/* IAR iccarm specific functions */
/* Close Raw Asm Code Warning */
#pragma diag_suppress=Pe940
    static void cmb_set_psp(uint32_t psp)
    {
      __asm("msr psp, r0");
    }
#pragma diag_default=Pe940
#elif defined(__GNUC__)
    __attribute__( ( always_inline ) ) static inline void cmb_set_psp(uint32_t psp) {
        __asm volatile ("MSR psp, %0\n\t" :: "r" (psp) );
    }
#else
    #error "not supported compiler"
#endif

RT_WEAK rt_err_t exception_hook(void *context) {
    extern long list_thread(void);
    volatile uint8_t _continue = 1;
        
    rt_enter_critical();

#ifdef RT_USING_FINSH
    list_thread();
#endif

    /* the PSP is changed by RT-Thread HardFault_Handler, so restore it to HardFault context */
#if (defined (__VFP_FP__) && !defined(__SOFTFP__)) || (defined (__ARMVFP__)) || (defined(__ARM_PCS_VFP) || defined(__TARGET_FPU_VFP))
    cmb_set_psp(cmb_get_psp() + 4 * 10);
#else
    cmb_set_psp(cmb_get_psp() + 4 * 9);
#endif

    cm_backtrace_fault(*((uint32_t *)(cmb_get_sp() + sizeof(uint32_t) * CMB_LR_WORD_OFFSET)), cmb_get_sp() + sizeof(uint32_t) * CMB_SP_WORD_OFFSET);

    while (_continue == 1);

    return RT_EOK;
}

int rt_cm_backtrace_init(void) {
    cm_backtrace_init("rtthread","1.0","1.0");
    
    rt_hw_exception_install(exception_hook);
    
    return 0;
}
INIT_DEVICE_EXPORT(rt_cm_backtrace_init);

long cmb_test(int argc, char **argv) {
    volatile int * SCB_CCR = (volatile int *) 0xE000ED14; // SCB->CCR
    int x, y, z;
    
    if (argc < 2)
    {
        rt_kprintf("Please input 'cmb_test <DIVBYZERO|UNALIGNED>' \n");
        return 0;
    }

    if (!strcmp(argv[1], "DIVBYZERO"))
    {
        *SCB_CCR |= (1 << 4); /* bit4: DIV_0_TRP. */
        x = 10;
        y = 0;
        z = x / y;
        rt_kprintf("z:%d\n", z);
        
        return 0;
    }
    else if (!strcmp(argv[1], "UNALIGNED"))
    {
        volatile int * p;
        volatile int value;
        *SCB_CCR |= (1 << 3); /* bit3: UNALIGN_TRP. */

        p = (int *) 0x00;
        value = *p;
        rt_kprintf("addr:0x%02X value:0x%08X\r\n", (int) p, value);

        p = (int *) 0x04;
        value = *p;
        rt_kprintf("addr:0x%02X value:0x%08X\r\n", (int) p, value);

        p = (int *) 0x03;
        value = *p;
        rt_kprintf("addr:0x%02X value:0x%08X\r\n", (int) p, value);
        
        return 0;
    }
    return 0;
}
MSH_CMD_EXPORT(cmb_test, cm_backtrace_test: cmb_test <DIVBYZERO|UNALIGNED> );
