/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-25     heyuanjie87  the first version
 */

#include <adb_service.h>
#include <rtdevice.h>
#include <dfs_posix.h>

struct adb_shdev
{
    struct rt_device parent;
    struct rt_ringbuffer *rbuf;
    struct rt_ringbuffer *wbuf;
    struct rt_event notify;
};

#define ADEV_EXIT    (0x01)
#define ADEV_EREADY  (0x10)
#define ADEV_READ    (0x02)
#define ADEV_WRITE   (0x04)
#define ADEV_WREADY  (0x08)

struct shell_ext
{
    struct adb_packet *cur;
    adb_queue_t recv_que;
    int rque_buf[4];
    struct adb_shdev *dev;
    rt_thread_t shid;
    struct rt_event notify;
    rt_thread_t worker;
    int old_flag;
    int mode;
};

static struct adb_shdev _shdev;
static struct rt_mutex _lock;

static int _evwait(struct rt_event *notify, int ev, int ms)
{
    int r = 0;

    rt_event_recv(notify, ev,
                  RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                  rt_tick_from_millisecond(ms),
                  (rt_uint32_t*)&r);

    return r;
}

static int _adwait(struct adb_shdev *ad, int ev, int ms)
{
    if (!ad->parent.user_data)
        return ADEV_EXIT;

    return _evwait(&ad->notify, ev, ms);
}

static int _adreport(struct adb_shdev *ad, int ev)
{
    if (!ad->parent.user_data)
        return ADEV_EXIT;

    rt_event_send(&ad->notify, ev);

    return 0;
}

static int _safe_rb_dlen(struct adb_shdev *ad)
{
    int l = 0;

    if (!ad->parent.user_data)
        return 0;

    rt_mutex_take(&_lock, -1);
    if (ad->wbuf)
        l = rt_ringbuffer_data_len(ad->wbuf);
    rt_mutex_release(&_lock);

    return l;
}

static int _safe_rb_slen(struct adb_shdev *ad)
{
    int l = 0;

    if (!ad->parent.user_data)
        return 0;

    rt_mutex_take(&_lock, -1);
    if (ad->wbuf)
        l = rt_ringbuffer_space_len(ad->wbuf);
    rt_mutex_release(&_lock);

    return l;
}

static bool _safe_rb_read(struct adb_shdev *ad, void *buf, int size)
{
    int l = 0;

    rt_mutex_take(&_lock, -1);
    if (ad->wbuf)
        l = rt_ringbuffer_get(ad->wbuf, (unsigned char *)buf, size);
    rt_mutex_release(&_lock);

    return (l != 0);
}

static int _safe_rb_write(struct adb_shdev *ad, void *buf, int size)
{
    int l = -1;

    rt_mutex_take(&_lock, -1);
    if (ad->rbuf) {
        l = rt_ringbuffer_put(ad->rbuf, (const unsigned char *)buf, size);
    }
    rt_mutex_release(&_lock);

    return l;
}

static rt_err_t _shell_service_device_init(struct rt_device *dev)
{
    struct adb_shdev *ad = (struct adb_shdev *)dev;

    rt_event_init(&ad->notify, "as-sh", 0);
    ad->rbuf = rt_ringbuffer_create(32);
    ad->wbuf = rt_ringbuffer_create(256);
    if (!ad->rbuf || !ad->wbuf)
    {
        if (ad->rbuf)
            rt_ringbuffer_destroy(ad->rbuf);
        if (ad->wbuf)
            rt_ringbuffer_destroy(ad->wbuf);
        rt_event_detach(&ad->notify);

        return -1;
    }

    return 0;
}

static rt_err_t _shell_service_device_open(struct rt_device *dev, rt_uint16_t oflag)
{
    dev->open_flag = oflag & 0xff;

    return 0;
}

static rt_err_t _shell_service_device_close(struct rt_device *dev)
{
    struct adb_shdev *ad = (struct adb_shdev *)dev;

    rt_mutex_take(&_lock, -1);
    if (dev->user_data)
    {
        struct adb_service *ser;

        ser = (struct adb_service *)dev->user_data;
        ser->online = 0;
        dev->user_data = 0;
    }
    rt_ringbuffer_destroy(ad->wbuf);
    rt_ringbuffer_destroy(ad->rbuf);
    ad->wbuf = 0;
    ad->rbuf = 0;
    rt_mutex_release(&_lock);

    _adreport(ad, ADEV_EXIT);
   if (_adwait(ad, ADEV_EREADY, 100) & ADEV_EREADY)
       rt_event_detach(&ad->notify);

    return 0;
}

static rt_size_t _shell_service_device_read(rt_device_t dev, rt_off_t pos,
                                            void *buffer, rt_size_t size)
{
    struct adb_shdev *ad = (struct adb_shdev *)dev;
    int len = 0;

    if (!dev->user_data)
        goto _exit;

_retry:
    rt_mutex_take(&_lock, -1);
    len = rt_ringbuffer_get(ad->rbuf, buffer, size);
    rt_mutex_release(&_lock);
    if (len == 0)
    {
        int ret = _adwait(ad, ADEV_READ | ADEV_EXIT, 100);
        if (ret & ADEV_EXIT)
        {
            _adreport(ad, ADEV_EREADY);
            goto _exit;
        }

        if (ret & ADEV_READ)
            goto _retry;
    }

_exit:
    if (len == 0)
        len = -EAGAIN;

    return len;
}

static rt_size_t _shell_service_device_write(rt_device_t dev, rt_off_t pos,
                                             const void *buffer, rt_size_t size)
{
    struct adb_shdev *ad = (struct adb_shdev *)dev;
    int wlen, r = 0;
    char *spos = (char *)buffer;

    if (!dev->user_data)
        return 0;

    while (_safe_rb_slen(ad) < size)
    {
        r = _adwait(ad, ADEV_WREADY, 20);
        /* wait event timeout */
        if (r == 0)
            return 0;
    }

    if (rt_interrupt_get_nest())
    {
        return rt_ringbuffer_put(ad->wbuf, (unsigned char *)spos, size);
    }


    rt_mutex_take(&_lock, -1);
    wlen = rt_ringbuffer_put(ad->wbuf, (unsigned char *)spos, size);
    rt_mutex_release(&_lock);
    _adreport(ad, ADEV_WRITE);

    return wlen;
}

static rt_err_t _shell_service_device_ctrl(rt_device_t dev,
                                           int cmd, void *args)
{
    int ret = 0;

    return ret;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops shell_ops =
{
    _shell_service_device_init,
    _shell_service_device_open,
    _shell_service_device_close,
    _shell_service_device_read,
    _shell_service_device_write,
    _shell_service_device_ctrl
};
#endif

static int _device_init(rt_device_t shell_device, void *usrdat)
{
    int ret;

    shell_device->type = RT_Device_Class_Char;
#ifdef RT_USING_DEVICE_OPS
    shell_device->ops = &shell_ops;
#else
    shell_device->init = _shell_service_device_init;
    shell_device->open = _shell_service_device_open;
    shell_device->close = _shell_service_device_close;
    shell_device->read = _shell_service_device_read;
    shell_device->write = _shell_service_device_write;
    shell_device->control = _shell_service_device_ctrl;
#endif
    shell_device->user_data = usrdat;

    ret = rt_device_register(shell_device, "as-sh", RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);

    return ret;
}

static bool send_ready(struct shell_ext *ext, char *args)
{
    struct adb_packet *p;
    
    ext->mode = rt_strlen(args);
    p = adb_packet_new(ext->mode + 1);
    if (!p)
        return false;

    p->msg.data_length = ext->mode + 1;
    rt_memcpy(p->payload, args, ext->mode);
    p->payload[ext->mode] = '\n';
    if (!adb_packet_enqueue(&ext->recv_que, p, 0))
    {
        adb_packet_delete(p);
        return false;
    }

    return true;
}

extern int libc_stdio_set_console(const char* device_name, int mode);
extern int libc_stdio_get_console(void);

static int _shell_open(struct adb_service *ser, char *args)
{
    int ret = -1;
    struct shell_ext *ext;

    ext = (struct shell_ext *)ser->extptr;

#ifdef RT_USING_FINSH
    ext->shid = rt_thread_find(FINSH_THREAD_NAME);
    if (!ext->shid)
    {
        return -1;
    }
#endif

    ret = _device_init(&_shdev.parent, ser);
    if (ret == 0)
    {
        int pr = 23;

        rt_mb_init(&ext->recv_que, "as-sh", ext->rque_buf,
                   sizeof(ext->rque_buf) / sizeof(ext->rque_buf[0]), 0);
        rt_event_init(&ext->notify, "as-sh", 0);

        ext->dev = &_shdev;

        ext->dev->parent.rx_indicate = rt_console_get_device()->rx_indicate;
        ext->old_flag = ioctl(libc_stdio_get_console(), F_GETFL, 0);
        ioctl(libc_stdio_get_console(), F_SETFL, (void *)(ext->old_flag | O_NONBLOCK));

#ifdef RT_USING_FINSH
        rt_thread_control(ext->shid, RT_THREAD_CTRL_CHANGE_PRIORITY,
                          (void *)&pr);
#endif

        libc_stdio_set_console("as-sh", O_RDWR);
        rt_console_set_device("as-sh");

#ifdef RT_USING_FINSH
        rt_thread_resume(ext->shid);
#endif

        rt_thread_mdelay(50);

        ret = rt_thread_startup(ext->worker);
        if (ret == 0)
        {
            if (!send_ready(ext, args))
            {
                //todo
                ret = -1;
            }
        }
    }

    return ret;
}

static int _shell_close(struct adb_service *ser)
{
    int ret = 0;
    struct shell_ext *ext;

    ext = (struct shell_ext *)ser->extptr;

    ser->online = 0;
    rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
    libc_stdio_set_console(RT_CONSOLE_DEVICE_NAME, O_RDWR);
    rt_console_get_device()->rx_indicate = ext->dev->parent.rx_indicate;

    rt_thread_resume(ext->shid);
    rt_thread_mdelay(50);

    _evwait(&ext->notify, ADEV_EXIT, 100);
    rt_mb_detach(&ext->recv_que);
    rt_event_detach(&ext->notify);

    rt_device_unregister(&ext->dev->parent);

    return ret;
}

static bool _shell_enqueue(struct adb_service *ser, struct adb_packet *p, int ms)
{
    struct shell_ext *ext;
    bool ret;

    if (!ser->online)
        return false;

    ext = (struct shell_ext *)ser->extptr;
    ret = adb_packet_enqueue(&ext->recv_que, p, ms);

    return ret;
}

static const struct adb_service_ops _ops =
{
    _shell_open,
    _shell_close,
    _shell_enqueue
};

static int do_readdev(struct adb_service *ser, struct shell_ext *ext)
{
    int size;
    struct adb_packet *p;

	  size = _safe_rb_dlen(ext->dev);
    if (size == 0)
        return 0;
		p = adb_packet_new(size);
    if (!p)
        return 0;
    if (!_safe_rb_read(ext->dev, p->payload, size))
    {
        adb_packet_delete(p);
        return ADEV_EXIT;
    }
    if (!adb_service_sendpacket(ser, p, 60))
    {
        adb_packet_delete(p);
    }

    return _adreport(ext->dev, ADEV_WREADY);
}

static int do_writedev(struct adb_service *ser, struct shell_ext *ext)
{
    struct adb_packet *p;
    char *pos;
    int len;

    if (!ext->cur)
    {
        if (!adb_packet_dequeue(&ext->recv_que, &ext->cur, 10))
            return 0;
        ext->cur->split = 0;
    }

    p = ext->cur;
    pos = p->payload + p->split;
    len = _safe_rb_write(ext->dev, pos, p->msg.data_length);
    if (len < 0)
        return ADEV_EXIT;

    if (len > 0 && ext->dev->parent.rx_indicate != RT_NULL)
    {
        ext->dev->parent.rx_indicate(&(ext->dev->parent), len);
    }

    p->split += len;
    p->msg.data_length -= len;
    if (p->msg.data_length == 0)
    {
        ext->cur = 0;
        adb_packet_delete(p);
    }

    return _adreport(ext->dev, ADEV_READ);
}

static void service_thread(void *arg)
{
    struct adb_service *ser;
    struct shell_ext *ext;
    unsigned revt;
    int exit = 40;

    ser = arg;
    ext = ser->extptr;
    ser->online = 1;

    while (ser->online)
    {
        if (do_writedev(ser, ext) != 0)
            break;

        revt = _adwait(ext->dev, ADEV_WRITE | ADEV_EXIT, 20);
        if (revt & ADEV_EXIT)
            break;
        if (revt & ADEV_WRITE)
        {
            exit = 10;
            if (do_readdev(ser, ext) != 0)
                break;
        }
        else if (ext->mode != 0)
        {
            if (--exit == 0)
                break;
        }
    }
    ser->online = 0;

    rt_mutex_take(&_lock, -1);
    ext->dev->parent.user_data = 0;
    rt_mutex_release(&_lock);

    adb_packet_delete(ext->cur);
    adb_packet_clear(&ext->recv_que);
    adb_service_close_report(ser);

    rt_event_send(&ext->notify, ADEV_EXIT);
}

static struct adb_service *_shell_create(struct adb_service_handler *h)
{
    struct adb_service *ser;
    struct shell_ext *ext;

    if (_shdev.parent.ref_count)
        return RT_NULL;
    if (rt_thread_find("as-sh"))
        return RT_NULL;

    ser = adb_service_alloc(&_ops, sizeof(struct shell_ext));
    if (ser)
    {
        ext = (struct shell_ext *)ser->extptr;
        ext->dev = &_shdev;
        ext->worker = rt_thread_create("as-sh",
                                       service_thread,
                                       ser,
                                       1024,
                                       22,
                                       20);
    }

    return ser;
}

static void _shell_destroy(struct adb_service_handler *h, struct adb_service *s)
{
    rt_free(s);
}

int adb_shell_init(void)
{
    static struct adb_service_handler _h;

    _h.name = "shell:";
    _h.create = _shell_create;
    _h.destroy = _shell_destroy;

    rt_mutex_init(&_lock, "as-sh", 0);

    return adb_service_handler_register(&_h);
}
INIT_APP_EXPORT(adb_shell_init);

static void exitas(int argc, char **argv)
{
    rt_thread_t tid;

    if (_shdev.parent.ref_count == 0)
    {
        rt_kprintf("adb shell service not run\n");
        return;
    }

    rt_mutex_take(&_lock, -1);
    tid = rt_thread_find("as-sh");
    if (tid)
    {
        struct adb_service *ser;

        ser = tid->parameter;
        ser->online = 0;
    }
    rt_mutex_release(&_lock);
}
MSH_CMD_EXPORT(exitas, exit adb shell service);
