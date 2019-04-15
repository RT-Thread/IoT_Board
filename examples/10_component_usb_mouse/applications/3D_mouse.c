/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-22     Ernest Chen  the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "icm20608.h"
#include <math.h>

#define DBG_SECTION_NAME "3D_mouse"
#define DBG_LEVEL DBG_LOG
#include <rtdbg.h>

static rt_sem_t mouse_sem = RT_NULL;

static struct rt_thread usb_thread;
ALIGN(RT_ALIGN_SIZE)
static char usb_thread_stack[0x1000];

static struct rt_thread icm_thread;
ALIGN(RT_ALIGN_SIZE)
static char icm_thread_stack[0x1000];

static struct rt_thread key_thread;
ALIGN(RT_ALIGN_SIZE)
static char key_thread_stack[0x1000];

/* 变动识别值 */
const static float mouse_rang_scope = 6.0f;
/* 读取角度有效角度 */
const static float mouse_angle_range = 80.0f;
/* 移动值的最大值 */
const static float mouse_move_range = 127.0f;
/* 角度移动比 */
#define mouse_ratio (mouse_move_range / mouse_angle_range)
/* 移动步长 */
const static rt_uint8_t mouse_pixel_len = 5;
/* 鼠标响应时间 */
const static rt_uint32_t mouse_sample_times = 0;

static float mouse_cmp_last_x, mouse_cmp_last_y;

#define Gyro_Gr 0.0010653f
#define pi 3.141593f
#define Kp 10.0f
#define Ki 0.008f
#define halfT 0.01f

static float q0 = 1, q1 = 0, q2 = 0, q3 = 0;
static float exInt = 0, eyInt = 0, ezInt = 0;

struct icm_position
{
    icm20608_device_t icm20608_device;
    rt_mutex_t lock;

    float x;           // 传感器 x 位置
    float y;           // 传感器 y 位置
    float z;           // 传感器 z 位置
    rt_int8_t buff[4]; // 发送鼠标的值
};
typedef struct icm_position *icm_position_t;

icm_position_t icm_device = RT_NULL;

static void mouse_meas_check(float *temp)
{
    if (*temp > mouse_angle_range)
    {
        *temp = mouse_angle_range;
    }
    else if (*temp < -mouse_angle_range)
    {
        *temp = -mouse_angle_range;
    }
}

#ifdef __GNUC__

const float __atan2f_lut[4] = {
    -0.0443265554792128, //p7
    -0.3258083974640975, //p3
    +0.1555786518463281, //p5
    +0.9997878412794807  //p1
};
const float __atan2f_pi_2 = 1.5707963;

/* 求平方根 */
static float _sqrt(float x)
{
    float xhalf = 0.5f * x;
    int i = *(int *)&x;
    i = 0x5f375a86 - (i >> 1);
    x = *(float *)&i;
    x = x * (1.5f - xhalf * x * x);

    return 1.0 / x;
}

/* 求绝对值 */
static float _fabs(float x)
{
    return (x >= 0 ? x : (-x));
}

/* 求x / y的反正切值 */
static float _atan2f(float y, float x)
{
    float a, b, c, r, xx;
    int m;
    union {
        float f;
        int i;
    } xinv;

    xx = _fabs(x);
    xinv.f = xx;
    m = 0x3F800000 - (xinv.i & 0x7F800000);
    xinv.i = xinv.i + m;
    xinv.f = 1.41176471f - 0.47058824f * xinv.f;
    xinv.i = xinv.i + m;
    b = 2.0 - xinv.f * xx;
    xinv.f = xinv.f * b;
    b = 2.0 - xinv.f * xx;
    xinv.f = xinv.f * b;

    c = _fabs(y * xinv.f);

    xinv.f = c;
    m = 0x3F800000 - (xinv.i & 0x7F800000);
    xinv.i = xinv.i + m;
    xinv.f = 1.41176471f - 0.47058824f * xinv.f;
    xinv.i = xinv.i + m;
    b = 2.0 - xinv.f * c;
    xinv.f = xinv.f * b;
    b = 2.0 - xinv.f * c;
    xinv.f = xinv.f * b;

    xinv.f = xinv.f + c;
    a = (c > 1.0f);
    c = c - a * xinv.f;
    r = a * __atan2f_pi_2;

    xx = c * c;
    a = (__atan2f_lut[0] * c) * xx + (__atan2f_lut[2] * c);
    b = (__atan2f_lut[1] * c) * xx + (__atan2f_lut[3] * c);
    xx = xx * xx;
    r = r + a * xx;
    r = r + b;

    b = M_PI;
    b = b - 2.0f * r;
    r = r + (x < 0.0f) * b;
    b = (_fabs(x) < 0.000001f);
    c = !b;
    r = c * r;
    r = r + __atan2f_pi_2 * b;
    b = r + r;
    r = r - (y < 0.0f) * b;

    return r;
}

static double _asin(double x)
{
    return _atan2f(x, _sqrt(1.0 - x * x));
}
#endif /* __GNUC__ */

/* 获取俯仰角、偏航角、翻滚角 */
static void get_angle(icm_position_t dev, float ax, float ay, float az, float gx, float gy, float gz)
{
    float pitch = 0, roll = 0, yaw = 0;
    float norm;
    float vx, vy, vz;
    float ex, ey, ez;

    float q0q0 = q0 * q0;
    float q0q1 = q0 * q1;
    float q0q2 = q0 * q2;
    float q1q1 = q1 * q1;
    float q1q3 = q1 * q3;
    float q2q2 = q2 * q2;
    float q2q3 = q2 * q3;
    float q3q3 = q3 * q3;
    gx *= Gyro_Gr;
    gy *= Gyro_Gr;
    gz *= Gyro_Gr;

    if (az == 0)
        return;

#ifdef __GNUC__
    norm = _sqrt(ax * ax + ay * ay + az * az);
#else
    norm = sqrt(ax * ax + ay * ay + az * az);
#endif /* __GNUC__ */

    ax = ax / norm;
    ay = ay / norm;
    az = az / norm;

    vx = 2 * (q1q3 - q0q2);
    vy = 2 * (q0q1 + q2q3);
    vz = q0q0 - q1q1 - q2q2 + q3q3;

    ex = (ay * vz - az * vy);
    ey = (az * vx - ax * vz);
    ez = (ax * vy - ay * vx);

    exInt = exInt + ex * Ki;
    eyInt = eyInt + ey * Ki;
    ezInt = ezInt + ez * Ki;

    gx = gx + Kp * ex + exInt;
    gy = gy + Kp * ey + eyInt;
    gz = gz + Kp * ez + ezInt;

    q0 = q0 + (-q1 * gx - q2 * gy - q3 * gz) * halfT;
    q1 = q1 + (q0 * gx + q2 * gz - q3 * gy) * halfT;
    q2 = q2 + (q0 * gy - q1 * gz + q3 * gx) * halfT;
    q3 = q3 + (q0 * gz + q1 * gy - q2 * gx) * halfT;

#ifdef __GNUC__
    norm = _sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
#else
    norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
#endif /* __GNUC__ */

    q0 = q0 / norm;
    q1 = q1 / norm;
    q2 = q2 / norm;
    q3 = q3 / norm;

#ifdef __GNUC__
    pitch = -_asin(2 * (q0 * q2 - q1 * q3)) * 57.2957795f;
    roll = _asin(2 * (q0 * q1 + q2 * q3)) * 57.2957795f;
    yaw = -_atan2f(2 * (q1 * q2 + q0 * q3), (q0q0 + q1q1 - q2q2 - q3q3)) * 57.29578f;
#else
    pitch = -asin(2 * (q0 * q2 - q1 * q3)) * 57.2957795f;
    roll = asin(2 * (q0 * q1 + q2 * q3)) * 57.2957795f;
    yaw = -atan2f(2 * (q1 * q2 + q0 * q3), (q0q0 + q1q1 - q2q2 - q3q3)) * 57.29578f;
#endif /* __GNUC__ */

    mouse_meas_check(&pitch);
    dev->x = pitch * mouse_ratio;
    mouse_meas_check(&roll);
    dev->y = roll * mouse_ratio;
    mouse_meas_check(&yaw);
    dev->z = yaw * mouse_ratio;
}

static void mouse_get_pos(icm_position_t dev)
{
    rt_err_t result;
    rt_int16_t accel_x, accel_y, accel_z;
    rt_int16_t gyros_x, gyros_y, gyros_z;

    RT_ASSERT(dev);

    result = rt_mutex_take(dev->lock, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        goto __exit;
    }

    result = icm20608_get_accel(dev->icm20608_device, &accel_x, &accel_y, &accel_z);
    if (result != RT_EOK)
    {
        rt_mutex_release(dev->lock);
        goto __exit;
    }

    result = icm20608_get_gyro(dev->icm20608_device, &gyros_x, &gyros_y, &gyros_z);
    if (result != RT_EOK)
    {
        rt_mutex_release(dev->lock);
        goto __exit;
    }

    get_angle(dev, (float)accel_x, (float)accel_y, (float)accel_z, (float)gyros_x, (float)gyros_y, (float)gyros_z);

    rt_mutex_release(dev->lock);

__exit:
    if (result != RT_EOK)
    {
        LOG_E("The sensor does not work");
        return;
    }
}

static icm_position_t mouse_init_icm(void)
{
    rt_err_t result;

    rt_thread_mdelay(1000);
    icm_device = rt_calloc(1, sizeof(struct icm_position));
    if (icm_device == RT_NULL)
    {
        LOG_E("Can't allocate memory for three-dimensional mouse ");

        return RT_NULL;
    }

    /* 初始化传感器 icm20608 */
    icm_device->icm20608_device = icm20608_init("i2c1");
    if (icm_device->icm20608_device == RT_NULL)
    {
        LOG_E("The sensor initializes failure");
        rt_free(icm_device);

        return RT_NULL;
    }
    else
    {
        LOG_D("The 3D mouse initializes success");
    }

    icm_device->lock = rt_mutex_create("mutex_mouse", RT_IPC_FLAG_FIFO);
    if (icm_device->lock == RT_NULL)
    {
        LOG_E("Can't create mutex for three-dimensional mouse");
        rt_free(icm_device);

        return RT_NULL;
    }

    mouse_sem = rt_sem_create("mouse_sem", 0, RT_IPC_FLAG_FIFO);
    if (mouse_sem == RT_NULL)
    {
        LOG_E("Can't create sem for mouse device");
        rt_mutex_delete(icm_device->lock);
        rt_free(icm_device);
        return 0;
    }

    /* 传感器零值校准 */
    result = icm20608_calib_level(icm_device->icm20608_device, 10);
    if (result != RT_EOK)
    {
        LOG_E("The sensor calibrates failure");
        rt_mutex_delete(icm_device->lock);
        rt_sem_delete(mouse_sem);
        rt_free(icm_device);

        return RT_NULL;
    }

    return icm_device;
}

static void mouse_go_release_status(void)
{
    /* 初始状态值 */
    rt_int8_t buff[4] = {0x08, 0, 0, 0};

    icm_device->buff[0] = buff[0];
    icm_device->buff[1] = buff[1];
    icm_device->buff[2] = buff[2];
    icm_device->buff[3] = buff[3];
}

static void mouse_send_data(rt_device_t device)
{
    RT_ASSERT(device != RT_NULL);

    rt_device_write(device, HID_REPORT_ID_MOUSE, icm_device->buff, 4);
    mouse_go_release_status();
}

/* 负值表示向左移动，正值表示向右移动 */
static void mouse_move_x(rt_device_t device, float ratio_x)
{
    RT_ASSERT(device != RT_NULL);

    if (icm_device->x < 0)
    {
        icm_device->buff[1] = (rt_int8_t)(-mouse_pixel_len * ratio_x);
    }
    else
    {
        icm_device->buff[1] = (rt_int8_t)(mouse_pixel_len * ratio_x);
    }

    mouse_send_data(device);
}

/* 正值表示向下移动，负值表示向上移动 */
static void mouse_move_y(rt_device_t device, float ratio_y)
{
    RT_ASSERT(device != RT_NULL);

    if (icm_device->y < 0)
    {
        icm_device->buff[2] = (rt_int8_t)(-mouse_pixel_len * ratio_y);
    }
    else
    {
        icm_device->buff[2] = (rt_int8_t)(mouse_pixel_len * ratio_y);
    }
    mouse_send_data(device);
}

static void mouse_move_xy(rt_device_t device, float ratio_x, float ratio_y)
{
    RT_ASSERT(device != RT_NULL);

    if (icm_device->x < 0)
    {
        icm_device->buff[1] = (rt_int8_t)(-mouse_pixel_len * ratio_x);
    }
    else
    {
        icm_device->buff[1] = (rt_int8_t)(mouse_pixel_len * ratio_x);
    }

    if (icm_device->y < 0)
    {
        icm_device->buff[2] = (rt_int8_t)(-mouse_pixel_len * ratio_y);
    }
    else
    {
        icm_device->buff[2] = (rt_int8_t)(mouse_pixel_len * ratio_y);
    }
    mouse_send_data(device);
}

static void usb_thread_entry(void *parameter)
{
    rt_device_t device = (rt_device_t)parameter;
    rt_uint8_t i = 0;

    while (1)
    {
        if (rt_sem_take(mouse_sem, RT_WAITING_FOREVER) == RT_EOK)
        {
            rt_uint8_t temp_x = 0, temp_y = 0, move_max = 0;
            float move_distance = 0.0f;

            rt_mutex_take(icm_device->lock, RT_WAITING_FOREVER);

#ifdef __GNUC__
            temp_x = (rt_uint8_t)_fabs(icm_device->x);
            temp_y = (rt_uint8_t)_fabs(icm_device->y);
            move_distance = _sqrt(temp_x * temp_x + temp_y * temp_y);
#else
            temp_x = (rt_uint8_t)fabs(icm_device->x);
            temp_y = (rt_uint8_t)fabs(icm_device->y);
            move_distance = sqrt(temp_x * temp_x + temp_y * temp_y);
#endif
            move_max = temp_x > temp_y ? temp_x : temp_y;

            /* 根据倾斜程度获取移动值 */
            for (i = 0; i < move_max / mouse_pixel_len; i++)
            {
                LOG_D("move_max :%4d, x: %4d, y :%4d ", move_max, temp_x, temp_y);
                if (i < temp_x && i < temp_y)
                {
                    mouse_move_xy(device, temp_x / move_distance, temp_y / move_distance);
                }
                else if (i < temp_x && i >= temp_y)
                {
                    mouse_move_x(device, temp_x / move_distance);
                }
                else if (i >= temp_x && i < temp_y)
                {
                    mouse_move_y(device, temp_y / move_distance);
                }
                else
                {
                    break;
                }
            }
            rt_mutex_release(icm_device->lock);
        }
        rt_thread_mdelay(mouse_sample_times);
    }
}

static void icm_thread_entry(void *parameter)
{
    while (1)
    {
        float temp_x = 0.0, temp_y = 0.0;

        rt_mutex_take(icm_device->lock, RT_WAITING_FOREVER);
        /* 获取鼠标移动范围 */
        mouse_get_pos(icm_device);

        /* 获取差值 */
        temp_x = icm_device->x - mouse_cmp_last_x;
        temp_y = icm_device->y - mouse_cmp_last_y;

        /* 避免抖动 */
        if (temp_x > mouse_rang_scope || temp_x < -mouse_rang_scope || temp_y > mouse_rang_scope || temp_y < -mouse_rang_scope)
        {
            /* 存储这次鼠标位移值 */
            mouse_cmp_last_x = icm_device->x;
            mouse_cmp_last_y = icm_device->y;
            rt_sem_release(mouse_sem);
        }
        rt_mutex_release(icm_device->lock);
        rt_thread_mdelay(mouse_sample_times);
    }
}

static void key_thread_entry(void *parameter)
{
    int mouse_key_2, mouse_key_0;
    rt_device_t device = (rt_device_t)parameter;
    rt_err_t result = -RT_ERROR;

    while (1)
    {
        if (rt_pin_read(PIN_KEY0) == PIN_LOW)
        {
            mouse_key_0 = 1;
        }
        else if (rt_pin_read(PIN_KEY2) == PIN_LOW)
        {
            mouse_key_2 = 1;
        }
        else
        {
            mouse_key_2 = 0;
            mouse_key_0 = 0;
        }

        if (mouse_key_2 | mouse_key_0)
        {
            result = rt_mutex_take(icm_device->lock, RT_WAITING_FOREVER);
            if (result == RT_EOK)
            {
                /* 获取鼠标键值，默认为 0 */
                icm_device->buff[0] = 0x08 | mouse_key_2 | mouse_key_0 << 1;
                if (mouse_key_2 & mouse_key_0)
                {
                    LOG_D("left & right down");
                }
                else if (mouse_key_2)
                {
                    LOG_D("left down");
                }
                else if (mouse_key_0)
                {
                    LOG_D("right down");
                }
                mouse_send_data(device);
                rt_mutex_release(icm_device->lock);
                rt_thread_mdelay(50);
            }
        }
        rt_thread_mdelay(mouse_sample_times / 5);
    }
}

static void mouse_init_key(void)
{
    /* 鼠标左键 */
    rt_pin_mode(PIN_KEY0, PIN_MODE_INPUT);
    /* 鼠标右键 */
    rt_pin_mode(PIN_KEY2, PIN_MODE_INPUT);
}

static int application_usb_init(void)
{
    /* 查找名称为 hidd 的设备 */
    rt_device_t device = rt_device_find("hidd");
    /* 初始化六轴传感器设备 */
    icm_device = mouse_init_icm();
    /* 初始化按键 */
    mouse_init_key();

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(icm_device != RT_NULL);

    /*打开查找到的 hid 设备 */
    rt_device_open(device, RT_DEVICE_FLAG_WRONLY);

    /* 初始化 USB 线程*/
    rt_thread_init(&usb_thread,
                   "hidd",
                   usb_thread_entry, device,
                   usb_thread_stack, sizeof(usb_thread_stack),
                   10, 20);
    rt_thread_startup(&usb_thread);

    /* 初始化六轴传感器线程 */
    rt_thread_init(&icm_thread,
                   "icm20608",
                   icm_thread_entry, RT_NULL,
                   icm_thread_stack, sizeof(icm_thread_stack),
                   10, 20);
    rt_thread_startup(&icm_thread);

    /* 初始化按键线程 */
    rt_thread_init(&key_thread,
                   "key",
                   key_thread_entry, device,
                   key_thread_stack, sizeof(key_thread_stack),
                   10, 20);
    rt_thread_startup(&key_thread);

    return 0;
}
INIT_APP_EXPORT(application_usb_init);
