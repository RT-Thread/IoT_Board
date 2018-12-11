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

//#define DBG_DISENABLE
#define DBG_ENABLE
#define DBG_SECTION_NAME "mouse"
#define DBG_LEVEL DBG_INFO
#define DBG_COLOR
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

const static float mouse_rang_scope = 6.0f;                          /* compare scope */
const static float mouse_angle_range = 80.0f;                        /* valid angle */
const static float mouse_move_range = 127.0f;                        /* movement range,default max range */
#define mouse_ratio  (mouse_move_range / mouse_angle_range)          /* ratio of icm20608/usb */
const static rt_uint8_t mouse_pixel_len = 5;                         /* move pixel */
const static rt_uint32_t mouse_sample_times = 0;                     /* control mouse point response speed */

static float mouse_cmp_last_x, mouse_cmp_last_y; /* as compare number with last sample */

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
    float x; //sensor position
    float y;
    float z;

    rt_uint8_t key;
    rt_int8_t buff[4]; //mouse data
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

/* get angle of picth, roll and yaw */
static void imuUpdate(icm_position_t dev, float ax, float ay, float az, float gx, float gy, float gz)
{
    float pitch = 0, roll = 0, yaw = 0; //default wrong angle
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
    norm = sqrt(ax * ax + ay * ay + az * az);
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

    norm = sqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
    q0 = q0 / norm;
    q1 = q1 / norm;
    q2 = q2 / norm;
    q3 = q3 / norm;

    pitch = -asin(2 * (q0 * q2 - q1 * q3)) * 57.2957795f;
    roll = asin(2 * (q0 * q1 + q2 * q3)) * 57.2957795f;
    yaw = -atan2f(2 * (q1 * q2 + q0 * q3), (q0q0 + q1q1 - q2q2 - q3q3)) * 57.29578f;

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

    imuUpdate(dev, (float)accel_x, (float)accel_y, (float)accel_z, (float)gyros_x, (float)gyros_y, (float)gyros_z);

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

    /* waiting for sensor going into calibration mode */
    rt_thread_mdelay(1000);

    /* allocate memory for mouse analog */
    icm_device = rt_calloc(1, sizeof(struct icm_position));
    if (icm_device == RT_NULL)
    {
        LOG_E("Can't allocate memory for three-dimensional mouse ");

        return RT_NULL;
    }

    /* initialize icm20608, registered device driver */
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
    
    /* creat semaphore */
    mouse_sem = rt_sem_create("mouse_sem", 0, RT_IPC_FLAG_FIFO);
    if (mouse_sem == RT_NULL)
    {
        LOG_E("Can't create sem for mouse device");
        rt_mutex_delete(icm_device->lock);
        rt_free(icm_device);
        return 0;
    }

    /* calibrate icm20608 zero level and average 10 times with sampling data */
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
    rt_int8_t buff[4] = {0x08, 0, 0, 0}; //release status

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

/* plus with moving left,otherwise, moving right */
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

/* plus with moving down,otherwise, moving up */
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

            temp_x = (rt_uint8_t)fabs(icm_device->x);
            temp_y = (rt_uint8_t)fabs(icm_device->y);
            move_distance = sqrt(temp_x * temp_x + temp_y * temp_y);//move distance

            move_max = temp_x > temp_y ? temp_x : temp_y;

            /* move pixel according to pixel_len value */
            for (i = 0; i < move_max / mouse_pixel_len; i++)
            {
                LOG_I("move_max :%4d, x: %4d, y :%4d ", move_max, temp_x, temp_y);
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
        /* get x and y scale */
        mouse_get_pos(icm_device);

        LOG_D("icm_device->x:%d ,icm_device->y: %d ",(int)icm_device->x, (int)icm_device->y); //for debug
        /* get differences from last send data */
        temp_x = icm_device->x - mouse_cmp_last_x;
        temp_y = icm_device->y - mouse_cmp_last_y;

        /* avoid horizontal shake and hold on */
        if (temp_x > mouse_rang_scope || temp_x < -mouse_rang_scope || temp_y > mouse_rang_scope || temp_y < -mouse_rang_scope)
        {
            /* store this time data to compare */
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
                icm_device->buff[0] = 0x08 | mouse_key_2 | mouse_key_0 << 1; // value of press key left or right, default 0
                LOG_I("mouse_key_2:%d ,mouse_key_0: %d ", mouse_key_2, mouse_key_0);

                mouse_send_data(device);
                rt_mutex_release(icm_device->lock);
                rt_thread_mdelay(50);//avoid continous press
            }
        }
        rt_thread_mdelay(mouse_sample_times / 5);
    }
}

static void mouse_init_key(void)
{
    rt_pin_mode(PIN_KEY0, PIN_MODE_INPUT); //left
    rt_pin_mode(PIN_KEY2, PIN_MODE_INPUT); //right
}

static int application_usb_init(void)
{
    rt_device_t device = rt_device_find("hidd"); // find hid device
    icm_device = mouse_init_icm();               // icm20608 sensor device init
    mouse_init_key();                            // mouse left and right key init

    RT_ASSERT(device != RT_NULL);
    RT_ASSERT(icm_device != RT_NULL);

    rt_device_open(device, RT_DEVICE_FLAG_WRONLY); // open only write hid device  

    /* init usb device thread */
    rt_thread_init(&usb_thread,
                   "hidd",
                   usb_thread_entry, device,
                   usb_thread_stack, sizeof(usb_thread_stack),
                   10, 20);
    rt_thread_startup(&usb_thread);

    /* init icm20608 device thread */
    rt_thread_init(&icm_thread,
                   "icm20608",
                   icm_thread_entry, RT_NULL,
                   icm_thread_stack, sizeof(icm_thread_stack),
                   10, 20);
    rt_thread_startup(&icm_thread);

    /* init key device thread */
    rt_thread_init(&key_thread,
                   "key",
                   key_thread_entry, device,
                   key_thread_stack, sizeof(key_thread_stack),
                   10, 20);
    rt_thread_startup(&key_thread);

    return 0;
}
INIT_APP_EXPORT(application_usb_init); // usb mouse application init
