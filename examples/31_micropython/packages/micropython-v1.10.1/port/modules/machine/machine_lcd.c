/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 SummerGift <SummerGift@qq.com>
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

#include <stdio.h>
#include <string.h>

#include "py/mphal.h"
#include "py/runtime.h"

#if MICROPY_PY_MACHINE_LCD

#include "machine_lcd.h"
#include <drv_lcd.h>

#define MAX_CO (240 - 1)

typedef struct _machine_lcd_obj_t {
    mp_obj_base_t base;
} machine_lcd_obj_t;

STATIC void error_check(bool status, const char *msg) {
    if (!status) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_ValueError, msg));
    }
}

/// \classmethod \constructor(skin_position)
///
/// Construct an LCD object.  
STATIC mp_obj_t machine_lcd_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    
    // create lcd object
    machine_lcd_obj_t *lcd = m_new_obj(machine_lcd_obj_t);
    lcd->base.type = &machine_lcd_type;

    return MP_OBJ_FROM_PTR(lcd);
}

/// \method light(value)
///
/// Turn the backlight on/off.  True or 1 turns it on, False or 0 turns it off.
STATIC mp_obj_t machine_lcd_light(mp_obj_t self_in, mp_obj_t value) {
    if (mp_obj_is_true(value)) {
        lcd_display_on(); // set pin high to turn backlight on
    } else {
        lcd_display_off();// set pin low to turn backlight off
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_lcd_light_obj, machine_lcd_light);

/// \method fill(colour)
///
/// Fill the screen with the given colour.
///
STATIC mp_obj_t machine_lcd_fill(mp_obj_t self_in, mp_obj_t col_in) {
    int col = mp_obj_get_int(col_in);
    lcd_clear(col);
    
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_lcd_fill_obj, machine_lcd_fill);

/// \method pixel(x, y, colour)
///
/// Set the pixel at `(x, y)` to the given colour.
///
STATIC mp_obj_t machine_lcd_pixel(size_t n_args, const mp_obj_t *args) {
    int x = mp_obj_get_int(args[1]);
    int y = mp_obj_get_int(args[2]);
    
    error_check((x >= 0 && x <= MAX_CO) && (y >= 0 && y <= MAX_CO) , "The min/max X/Y coordinates is 0/239");

    int col = mp_obj_get_int(args[3]);
    lcd_draw_point_color(x, y, col);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_lcd_pixel_obj, 4, 4, machine_lcd_pixel);

/// \method text(str, x, y, size)
///
/// Draw the given text to the position `(x, y)` using the given size (16 24 32).
///
STATIC mp_obj_t machine_lcd_text(size_t n_args, const mp_obj_t *args) {
    size_t len;
    const char *data = mp_obj_str_get_data(args[1], &len);
    int x = mp_obj_get_int(args[2]);
    int y = mp_obj_get_int(args[3]);
    int size = mp_obj_get_int(args[4]);
    
    error_check((x >= 0 && x <= MAX_CO) && (y >= 0 && y <= MAX_CO) , "The min/max X/Y coordinates is 0/239");
    
    error_check(size == 16 || size == 24 || size == 32, "lcd only support font size 16 24 32");
    
    lcd_show_string(x, y, size, data);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_lcd_text_obj, 5, 5, machine_lcd_text);

/// \method line(x1, y1, x2, y2)
///
/// display a line on the lcd, from (x1, y1) to (x2, y2).
///
STATIC mp_obj_t machine_lcd_line(size_t n_args, const mp_obj_t *args) {
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);
    
    error_check((x1 >= 0 && x1 <= MAX_CO) && (y1 >= 0 && y1 <= MAX_CO) , "The min/max X/Y coordinates is 0/239");
    error_check((x2 >= 0 && x2 <= MAX_CO) && (y2 >= 0 && y2 <= MAX_CO) , "The min/max X/Y coordinates is 0/239");

    lcd_draw_line(x1, y1, x2, y2);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_lcd_line_obj, 5, 5, machine_lcd_line);

/// \method rectangle(x1, y1, x2, y2)
///
/// display a rectangle on the lcd, from (x1, y1) to (x2, y2).
///
STATIC mp_obj_t machine_lcd_rectangle(size_t n_args, const mp_obj_t *args) {
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int x2 = mp_obj_get_int(args[3]);
    int y2 = mp_obj_get_int(args[4]);

    error_check((x1 >= 0 && x1 <= MAX_CO) && (y1 >= 0 && y1 <= MAX_CO) , "The min/max X/Y coordinates is 0/239");
    error_check((x2 >= 0 && x2 <= MAX_CO) && (y2 >= 0 && y2 <= MAX_CO) , "The min/max X/Y coordinates is 0/239");

    lcd_draw_rectangle(x1, y1, x2, y2);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_lcd_rectangle_obj, 5, 5, machine_lcd_rectangle);

/// \method circle(x1, y1, r)
///
/// display a circle on the lcd, center(x1, y1) R = r.
///
STATIC mp_obj_t machine_lcd_circle(size_t n_args, const mp_obj_t *args) {
    int x1 = mp_obj_get_int(args[1]);
    int y1 = mp_obj_get_int(args[2]);
    int r  = mp_obj_get_int(args[3]);

    error_check((x1 >= 0 && x1 <= MAX_CO) && (y1 >= 0 && y1 <= MAX_CO) , "The min/max X/Y coordinates is 0/239");

    lcd_draw_circle(x1, y1, r);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_lcd_circle_obj, 4, 4, machine_lcd_circle);

/// \method set_color(back, fore)
///
/// Set background color and foreground color.
///
STATIC mp_obj_t machine_lcd_set_color(size_t n_args, const mp_obj_t *args) {
    rt_uint16_t back = mp_obj_get_int(args[1]);
    rt_uint16_t fore = mp_obj_get_int(args[2]);

    lcd_set_color(back, fore);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_lcd_set_color_obj, 3, 3, machine_lcd_set_color);

STATIC const mp_rom_map_elem_t machine_lcd_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_light), MP_ROM_PTR(&machine_lcd_light_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill),  MP_ROM_PTR(&machine_lcd_fill_obj)  },
    { MP_ROM_QSTR(MP_QSTR_pixel), MP_ROM_PTR(&machine_lcd_pixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_text),  MP_ROM_PTR(&machine_lcd_text_obj)  },
    { MP_ROM_QSTR(MP_QSTR_line),  MP_ROM_PTR(&machine_lcd_line_obj)  },
    { MP_ROM_QSTR(MP_QSTR_rectangle), MP_ROM_PTR(&machine_lcd_rectangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_circle), MP_ROM_PTR(&machine_lcd_circle_obj) }, 
    { MP_ROM_QSTR(MP_QSTR_set_color), MP_ROM_PTR(&machine_lcd_set_color_obj) }, 
    // color
    { MP_ROM_QSTR(MP_QSTR_WHITE), MP_ROM_INT(WHITE) },
    { MP_ROM_QSTR(MP_QSTR_BLACK), MP_ROM_INT(BLACK) },
    { MP_ROM_QSTR(MP_QSTR_BLUE), MP_ROM_INT(BLUE) },
    { MP_ROM_QSTR(MP_QSTR_BRED), MP_ROM_INT(BRED) },
    { MP_ROM_QSTR(MP_QSTR_GRED), MP_ROM_INT(GRED) },
    { MP_ROM_QSTR(MP_QSTR_GBLUE), MP_ROM_INT(GBLUE) },
    { MP_ROM_QSTR(MP_QSTR_RED), MP_ROM_INT(RED) },
    { MP_ROM_QSTR(MP_QSTR_MAGENTA), MP_ROM_INT(MAGENTA) },
    { MP_ROM_QSTR(MP_QSTR_GREEN), MP_ROM_INT(GREEN) },
    { MP_ROM_QSTR(MP_QSTR_CYAN), MP_ROM_INT(CYAN) },
    { MP_ROM_QSTR(MP_QSTR_YELLOW), MP_ROM_INT(YELLOW) },
    { MP_ROM_QSTR(MP_QSTR_BROWN), MP_ROM_INT(BROWN) },
    { MP_ROM_QSTR(MP_QSTR_BRRED), MP_ROM_INT(BRRED) },
    { MP_ROM_QSTR(MP_QSTR_GRAY), MP_ROM_INT(GRAY) },
    { MP_ROM_QSTR(MP_QSTR_GRAY175), MP_ROM_INT(GRAY175) },
    { MP_ROM_QSTR(MP_QSTR_GRAY151), MP_ROM_INT(GRAY151) },
    { MP_ROM_QSTR(MP_QSTR_GRAY187), MP_ROM_INT(GRAY187) },
    { MP_ROM_QSTR(MP_QSTR_GRAY240), MP_ROM_INT(GRAY240) },
};
STATIC MP_DEFINE_CONST_DICT(machine_lcd_locals_dict, machine_lcd_locals_dict_table);

const mp_obj_type_t machine_lcd_type = {
    { &mp_type_type },
    .name = MP_QSTR_LCD,
    .make_new = machine_lcd_make_new,
    .locals_dict = (mp_obj_dict_t*)&machine_lcd_locals_dict,
};

#endif // MICROPY_PY_MACHINE_LCD
