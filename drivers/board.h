/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-09-22     Bernard      add board.h to this bsp
 */

// <<< Use Configuration Wizard in Context Menu >>>
#ifndef __BOARD_H__
#define __BOARD_H__

#define HARDWARE_VERSION 0x0204U

#include <stm32l4xx.h>
#include "drv_gpio.h"
#include "drv_clock.h"

#ifdef __ICCARM__
// Use *.icf ram symbal, to avoid hardcode.
extern char __ICFEDIT_region_IRAM1_end__;
#define STM32_SRAM_END          &__ICFEDIT_region_IRAM1_end__
#else
#define STM32_SRAM_SIZE         96
#define STM32_SRAM_END          (0x20000000 + STM32_SRAM_SIZE * 1024)
#endif

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define HEAP_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end;
#define HEAP_BEGIN    (&__bss_end)
#endif

#define HEAP_END                STM32_SRAM_END
#define STM32_SRAM2_SIZE        32
#define STM32_SRAM2_BEGIN       (0x10000000u)
#define STM32_SRAM2_END         (0x10000000 + STM32_SRAM2_SIZE * 1024)
#define STM32_SRAM2_HEAP_SIZE   ((uint32_t)STM32_SRAM2_END - (uint32_t)STM32_SRAM2_BEGIN)

void rt_hw_board_init(void);

void Error_Handler(void);

void rt_get_cpu_id(rt_uint32_t cpuid[3]);

#endif

//*** <<< end of configuration section >>>    ***
