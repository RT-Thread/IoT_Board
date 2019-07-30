/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-15     ZeroFree     first implementation
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <finsh.h>
#include <dfs_posix.h>
#include "wav_player.h"

#include <string.h>
#include <stdint.h>

#define IOTB_PLAYER_DEBUG

// #define DBG_ENABLE
#define DBG_TAG               "IOTB_PLAYER"
#ifdef IOTB_PLAYER_DEBUG
#define DBG_LVL                      DBG_LOG
#else
#define DBG_LVL                      DBG_INFO /* DBG_ERROR */
#endif
#define DBG_COLOR
#include <rtdbg.h>

#define SOUND_DEVICE_DECODE_MP_SZ  (4096)
#define SOUND_DEVICE_DECODE_MP_CNT (2)

#define BUFSZ   (SOUND_DEVICE_DECODE_MP_SZ / 2)

struct RIFF_HEADER_DEF
{
    char riff_id[4];     // 'R','I','F','F'
    uint32_t riff_size;
    char riff_format[4]; // 'W','A','V','E'
};

struct WAVE_FORMAT_DEF
{
    uint16_t FormatTag;
    uint16_t Channels;
    uint32_t SamplesPerSec;
    uint32_t AvgBytesPerSec;
    uint16_t BlockAlign;
    uint16_t BitsPerSample;
};

struct FMT_BLOCK_DEF
{
    char fmt_id[4];    // 'f','m','t',' '
    uint32_t fmt_size;
    struct WAVE_FORMAT_DEF wav_format;
};

struct DATA_BLOCK_DEF
{
    char data_id[4];     // 'R','I','F','F'
    uint32_t data_size;
};

struct wav_info
{
    struct RIFF_HEADER_DEF header;
    struct FMT_BLOCK_DEF   fmt_block;
    struct DATA_BLOCK_DEF  data_block;
};

enum SOUND_DEVICE_STATE
{
    SOUND_DEVICE_IDLE,
    SOUND_DEVICE_PLAYBACK,
    SOUND_DEVICE_CLOSE,
    SOUND_DEVICE_INIT_DONE,
    SOUND_DEVICE_INIT_FAIL
};

struct wav_player
{
    struct rt_device *snd;
    struct rt_mempool *mp;
    char uri[64];
    play_finish finished_cb;
    int state;
};

static int wav_player_play(char *uri);

static struct rt_event iotb_player_event;
static struct wav_player *player = RT_NULL;

static rt_err_t sound_device_write_done(struct rt_device *device, void *ptr)
{
    if (ptr == RT_NULL)
    {
        LOG_E("device buf_release NULL\n");
        return -RT_ERROR;
    }
    rt_mp_free(ptr);
    return RT_EOK;
}

void iotb_player_start(const char* path)
{
    if (player == RT_NULL)
    {
        return;
    }

    if (player->state == SOUND_DEVICE_PLAYBACK)
    {
        LOG_I("[busy] sound dev is busy!");
        return;
    }
    rt_memset(player->uri, 0x0, sizeof(player->uri));
    rt_strncpy(player->uri, path, rt_strlen(path));
    rt_event_send(&iotb_player_event, (rt_uint32_t)IOTB_PLAYER_START);
}

void iotb_player_stop(void)
{
    if (player == RT_NULL)
    {
        return;
    }

    if (player->state == SOUND_DEVICE_PLAYBACK)
    {
        player->state = SOUND_DEVICE_CLOSE;
    }
}
MSH_CMD_EXPORT_ALIAS(iotb_player_stop, player_stop, stop wav play);

rt_bool_t iotb_player_isbusy(void)
{
    return (player->state == SOUND_DEVICE_PLAYBACK);
}

static void _iotb_wav_play_handle(void *arg)
{
    rt_err_t rst = RT_EOK;
    rt_uint32_t event = 0;
    if (player->state != SOUND_DEVICE_INIT_DONE)
    {
        LOG_E("[error] sound device is not init!");
        return;
    }
    player->state = SOUND_DEVICE_IDLE;
    
    while(1)
    {
        rst = rt_event_recv(&iotb_player_event,
                            IOTB_PLAYER_START,
                            RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                            RT_WAITING_FOREVER, (rt_uint32_t *)&event);
        if (rst == RT_EOK)
        {
            LOG_I("player start..******. event :0x%08x", event);
            if (event & IOTB_PLAYER_START)
            {
                if (!(player->state == SOUND_DEVICE_IDLE || player->state == SOUND_DEVICE_CLOSE))
                {
                    LOG_I("[busy] sound dev is busy!");
                    continue;
                }
                LOG_I("player start...");
                wav_player_play(player->uri);
                player->state = SOUND_DEVICE_IDLE;
                if (player->finished_cb != RT_NULL)
                {
                    player->finished_cb();
                }

                LOG_I("[done] sound play finished.");
            }
        }
        else
        {
            LOG_I("player recv error!");
        }
    }
}

int iotb_wav_player_init(play_finish cb)
{
    int result;
    rt_thread_t tid;
    if (player != RT_NULL)
        return RT_EOK;

    player = rt_malloc(sizeof(struct wav_player));
    if (player == RT_NULL)
    {
        LOG_E("Malloc memory for wav player failed!");
        result = -RT_ENOMEM;
        goto __exit;
    }
    memset(player, 0, sizeof(struct wav_player));

    player->mp = rt_mp_create("adbuf", SOUND_DEVICE_DECODE_MP_CNT, SOUND_DEVICE_DECODE_MP_SZ);
    if (player->mp == RT_NULL)
    {
        LOG_E("Malloc memory for audio mempool failed!");
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* find sound device */
    player->snd = rt_device_find("sound");
    if (player->snd == NULL)
    {
        LOG_E("sound device not found \n");
        result = -RT_ERROR;
        goto __exit;
    }

    player->state = SOUND_DEVICE_IDLE;
    /* set tx complete call back function */
    rt_device_set_tx_complete(player->snd, sound_device_write_done);
    LOG_I("player init success!");

    rt_event_init(&iotb_player_event, "player", RT_IPC_FLAG_FIFO);

    player->state = SOUND_DEVICE_INIT_DONE;

    tid = rt_thread_create("i_player",
                           _iotb_wav_play_handle,
                           RT_NULL,
                           1024 * 2,
                           14,
                           10);
    if (tid != RT_NULL)
        rt_thread_startup(tid);
    player->finished_cb = cb;
    return RT_EOK;

__exit:
    if (player->mp != RT_NULL)
    {
        rt_mp_delete(player->mp);
        player->mp = RT_NULL;
    }

    if (player != RT_NULL)
    {
        rt_free(player);
        player = RT_NULL;
    }

    return result;
}

static int wav_player_play(char *uri)
{
    int fd, samplerate;
    uint16_t *buffer = NULL;
    struct wav_info *info = NULL;

    fd = open(uri, O_RDONLY);
    if (fd < 0)
    {
        LOG_E("open file %s failed, fd = %d \n", uri, fd);
        goto __exit;
    }

    info = (struct wav_info *) malloc(sizeof(*info));
    if (!info) goto __exit;

    if (read(fd, &(info->header),     sizeof(struct RIFF_HEADER_DEF)) == 0) goto __exit;
    if (read(fd, &(info->fmt_block),  sizeof(struct FMT_BLOCK_DEF)) == 0) goto __exit;
    if (read(fd, &(info->data_block), sizeof(struct DATA_BLOCK_DEF)) == 0) goto __exit;

    LOG_I("wav information:");
    LOG_I("samplerate %d", info->fmt_block.wav_format.SamplesPerSec);
    LOG_I("channel %d", info->fmt_block.wav_format.Channels);

    samplerate = info->fmt_block.wav_format.SamplesPerSec;
    player->state = SOUND_DEVICE_PLAYBACK;
    rt_device_open(player->snd, RT_DEVICE_OFLAG_WRONLY);
    rt_device_control(player->snd, CODEC_CMD_SAMPLERATE, &samplerate);

    while (1)
    {
        int length;

        buffer = rt_mp_alloc(player->mp, RT_WAITING_FOREVER);
        // LOG_D("malloc %p", buffer);
        length = read(fd, buffer, BUFSZ);
        if (length)
        {
            if (info->fmt_block.wav_format.Channels == 1)
            {
                /* extend to stereo channels */
                int index;
                uint16_t *ptr;

                ptr = (uint16_t *)((uint8_t *)buffer + BUFSZ * 2);
                for (index = 1; index <= BUFSZ / 2; index ++)
                {
                    *(ptr - 1) = *(ptr - 2) = buffer[BUFSZ / 2 - index];
                    ptr -= 2;
                }

                length = length * 2;
            }

            rt_device_write(player->snd, 0, (uint8_t *)buffer, length);
        }
        else
        {
            LOG_I("read end %p, length %d", buffer ,length);
            rt_mp_free((uint8_t *)buffer);
            break;
        }

        if (player->state == SOUND_DEVICE_CLOSE)
        {
            break;
        }
    }
    // rt_thread_mdelay(50);
    rt_device_close(player->snd);
    player->state = SOUND_DEVICE_CLOSE;
    if (fd > 0) close(fd);
    if (info) free(info);

    return RT_EOK;

__exit:
    if (fd > 0) close(fd);
    if (info) free(info);
    return -RT_ERROR;
}

static int wavplay(int argc, char **argv)
{
    if (argc != 2)
    {
        rt_kprintf("Usage:\n");
        rt_kprintf("player_start song.wav\n");
        return -RT_ERROR;
    }

    iotb_player_start(argv[1]);

    return -RT_ERROR;
}
MSH_CMD_EXPORT_ALIAS(wavplay, player_start, player_start song.wav); // wavplay 1.wav
