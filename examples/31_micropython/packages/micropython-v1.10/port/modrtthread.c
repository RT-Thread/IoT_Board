/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Armink (armink.ztl@gmail.com)
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
 */

#include "py/mpconfig.h"
#if MICROPY_PY_RTTHREAD

#include <rtthread.h>

#include "py/runtime.h"

rt_bool_t rt_is_preempt_thread(void) {
    if (rt_interrupt_get_nest() || rt_critical_level()) {
        return RT_FALSE;
    } else {
        return RT_TRUE;
    }
}

STATIC mp_obj_t mod_is_preempt_thread(void) {
    return mp_obj_new_bool(rt_is_preempt_thread());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_is_preempt_thread_obj, mod_is_preempt_thread);

STATIC mp_obj_t mod_current_tid(void) {
    return MP_OBJ_NEW_SMALL_INT(rt_thread_self());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_current_tid_obj, mod_current_tid);

STATIC mp_obj_t mod_stacks_analyze(void) {
#ifdef RT_USING_FINSH
    extern long list_thread(void);
    list_thread();
#else
    rt_kprintf("Not available when FINSH module disable\n");
#endif

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mod_stacks_analyze_obj, mod_stacks_analyze);

STATIC mp_obj_t mp_os_mount(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    //TODO
    MP_RTT_NOT_IMPL_PRINT;
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(mp_os_mount_obj, 2, mp_os_mount);

STATIC const mp_rom_map_elem_t mp_module_rtthread_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_rtthread) },
    { MP_ROM_QSTR(MP_QSTR_is_preempt_thread), MP_ROM_PTR(&mod_is_preempt_thread_obj) },
    { MP_ROM_QSTR(MP_QSTR_current_tid), MP_ROM_PTR(&mod_current_tid_obj) },
    { MP_ROM_QSTR(MP_QSTR_stacks_analyze), MP_ROM_PTR(&mod_stacks_analyze_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_rtthread_globals, mp_module_rtthread_globals_table);

const mp_obj_module_t mp_module_rtthread = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_rtthread_globals,
};

#endif // MICROPY_PY_RTTHREAD
