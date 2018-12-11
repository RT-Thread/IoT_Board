/*
 * File      : fal_rtt.c
 * This file is part of FAL (Flash Abstraction Layer) package
 * COPYRIGHT (C) 2006 - 2018, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-06-23     armink       the first version
 */

#include <fal.h>

#if defined(RT_USING_DFS)

#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>

struct fal_blk_device
{
    struct rt_device                parent;
    struct rt_device_blk_geometry   geometry;
    const struct fal_partition     *fal_part;
};

/* RT-Thread device interface */
#if RTTHREAD_VERSION >= 30000
static rt_err_t blk_dev_control(rt_device_t dev, int cmd, void *args)
#else
static rt_err_t blk_dev_control(rt_device_t dev, rt_uint8_t cmd, void *args)
#endif
{
    struct fal_blk_device *part = (struct fal_blk_device*) dev;

    assert(part != RT_NULL);

    if (cmd == RT_DEVICE_CTRL_BLK_GETGEOME)
    {
        struct rt_device_blk_geometry *geometry;

        geometry = (struct rt_device_blk_geometry *) args;
        if (geometry == RT_NULL)
        {
            return -RT_ERROR;
        }

        memcpy(geometry, &part->geometry, sizeof(struct rt_device_blk_geometry));
    }
    else if (cmd == RT_DEVICE_CTRL_BLK_ERASE)
    {
        rt_uint32_t *addrs = (rt_uint32_t *) args, start_addr = addrs[0], end_addr = addrs[1], phy_start_addr;
        rt_size_t phy_size;

        if (addrs == RT_NULL || start_addr > end_addr)
        {
            return -RT_ERROR;
        }

        if (end_addr == start_addr)
        {
            end_addr++;
        }

        phy_start_addr = start_addr * part->geometry.bytes_per_sector;
        phy_size = (end_addr - start_addr) * part->geometry.bytes_per_sector;

        if (fal_partition_erase(part->fal_part, phy_start_addr, phy_size) < 0)
        {
            return -RT_ERROR;
        }
    }

    return RT_EOK;
}

static rt_size_t blk_dev_read(rt_device_t dev, rt_off_t pos, void* buffer, rt_size_t size)
{
    int ret = 0;
    struct fal_blk_device *part = (struct fal_blk_device*) dev;

    assert(part != RT_NULL);

    ret = fal_partition_read(part->fal_part, pos * part->geometry.block_size, buffer, size * part->geometry.block_size);

    if (ret != (int)(size * part->geometry.block_size))
    {
        ret = 0;
    }
    else
    {
        ret = size;
    }

    return ret;
}

static rt_size_t blk_dev_write(rt_device_t dev, rt_off_t pos, const void* buffer, rt_size_t size)
{
    int ret = 0;
    struct fal_blk_device *part;
    rt_off_t phy_pos;
    rt_size_t phy_size;

    part = (struct fal_blk_device*) dev;
    assert(part != RT_NULL);

    /* change the block device's logic address to physical address */
    phy_pos = pos * part->geometry.bytes_per_sector;
    phy_size = size * part->geometry.bytes_per_sector;

    ret = fal_partition_erase(part->fal_part, phy_pos, phy_size);

    if (ret == (int) phy_size)
    {
        ret = fal_partition_write(part->fal_part, phy_pos, buffer, phy_size);
    }

    if (ret != (int) phy_size)
    {
        ret = 0;
    }
    else
    {
        ret = size;
    }

    return ret;
}

/**
 * create RT-Thread block device by specified partition
 *
 * @param parition_name partition name
 *
 * @return != NULL: created block device
 *            NULL: created failed
 */
struct rt_device *fal_blk_device_create(const char *parition_name)
{
    struct fal_blk_device *blk_dev;
    const struct fal_partition *fal_part = fal_partition_find(parition_name);
    const struct fal_flash_dev *fal_flash = NULL;

    if (!fal_part)
    {
        log_e("Error: the partition name (%s) is not found.", parition_name);
        return NULL;
    }

    if ((fal_flash = fal_flash_device_find(fal_part->flash_name)) == NULL)
    {
        log_e("Error: the flash device name (%s) is not found.", fal_part->flash_name);
        return NULL;
    }

    blk_dev = (struct fal_blk_device*) rt_malloc(sizeof(struct fal_blk_device));
    if (blk_dev)
    {
        blk_dev->fal_part = fal_part;
        blk_dev->geometry.bytes_per_sector = fal_flash->blk_size;
        blk_dev->geometry.block_size = fal_flash->blk_size;
        blk_dev->geometry.sector_count = fal_part->len / fal_flash->blk_size;

        /* register device */
        blk_dev->parent.type = RT_Device_Class_Block;
        blk_dev->parent.init = NULL;
        blk_dev->parent.open = NULL;
        blk_dev->parent.close = NULL;
        blk_dev->parent.read = blk_dev_read;
        blk_dev->parent.write = blk_dev_write;
        blk_dev->parent.control = blk_dev_control;
        /* no private */
        blk_dev->parent.user_data = RT_NULL;

        log_i("The FAL block device (%s) created successfully", fal_part->name);
        rt_device_register(RT_DEVICE(blk_dev), fal_part->name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
    }
    else
    {
        log_e("Error: no memory for create FAL block device");
    }

    return RT_DEVICE(blk_dev);
}

#endif /* defined(RT_USING_DFS) */

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)

#include <finsh.h>

static void fal(uint8_t argc, char **argv) {

#define CMD_PROBE_INDEX               0
#define CMD_READ_INDEX                1
#define CMD_WRITE_INDEX               2
#define CMD_ERASE_INDEX               3
#define CMD_BENCH_INDEX               4

    int result;
    static const struct fal_flash_dev *flash_dev = NULL;
    static const struct fal_partition *part_dev = NULL;
    size_t i = 0;

    const char* help_info[] =
    {
            [CMD_PROBE_INDEX]     = "fal probe [dev_name|part_name]   - probe flash device or partition by given name",
            [CMD_READ_INDEX]      = "fal read addr size               - read 'size' bytes starting at 'addr'",
            [CMD_WRITE_INDEX]     = "fal write addr data1 ... dataN   - write some bytes 'data' starting at 'addr'",
            [CMD_ERASE_INDEX]     = "fal erase addr size              - erase 'size' bytes starting at 'addr'",
            [CMD_BENCH_INDEX]     = "fal bench <blk_size>             - benchmark test with per block size",
    };

    if (argc < 2)
    {
        rt_kprintf("Usage:\n");
        for (i = 0; i < sizeof(help_info) / sizeof(char*); i++)
        {
            rt_kprintf("%s\n", help_info[i]);
        }
        rt_kprintf("\n");
    }
    else
    {
        const char *operator = argv[1];
        uint32_t addr, size;

        if (!strcmp(operator, "probe"))
        {
            if (argc >= 3)
            {
                char *dev_name = argv[2];
                if ((flash_dev = fal_flash_device_find(dev_name)) != NULL)
                {
                    part_dev = NULL;
                }
                else if ((part_dev = fal_partition_find(dev_name)) != NULL)
                {
                    flash_dev = NULL;
                }
                else
                {
                    rt_kprintf("Device %s NOT found. Probe failed.\n", dev_name);
                    flash_dev = NULL;
                    part_dev = NULL;
                }
            }

            if (flash_dev)
            {
                rt_kprintf("Probed a flash device | %s | addr: %ld | len: %d |.\n", flash_dev->name,
                        flash_dev->addr, flash_dev->len);
            }
            else if (part_dev)
            {
                rt_kprintf("Probed a flash partition | %s | flash_dev: %s | offset: %ld | len: %d |.\n",
                        part_dev->name, part_dev->flash_name, part_dev->offset, part_dev->len);
            }
            else
            {
                rt_kprintf("No flash device or partition was probed.\n");
                rt_kprintf("Usage: %s.\n", help_info[CMD_PROBE_INDEX]);
                fal_show_part_table();
            }
        }
        else
        {
            if (!flash_dev && !part_dev)
            {
                rt_kprintf("No flash device or partition was probed. Please run 'fal probe'.\n");
                return;
            }
            if (!rt_strcmp(operator, "read"))
            {
                if (argc < 4)
                {
                    rt_kprintf("Usage: %s.\n", help_info[CMD_READ_INDEX]);
                    return;
                }
                else
                {
                    addr = strtol(argv[2], NULL, 0);
                    size = strtol(argv[3], NULL, 0);
                    uint8_t *data = rt_malloc(size);
                    if (data)
                    {
                        if (flash_dev)
                        {
                            result = flash_dev->ops.read(addr, data, size);
                        }
                        else if (part_dev)
                        {
                            result = fal_partition_read(part_dev, addr, data, size);
                        }
                        if (result >= 0)
                        {
                            rt_kprintf("Read data success. Start from 0x%08X, size is %ld. The data is:\n", addr,
                                    size);
                            rt_kprintf("Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
                            for (i = 0; i < size; i++)
                            {
                                if (i % 16 == 0)
                                {
                                    rt_kprintf("[%08X] ", addr + i);
                                }
                                rt_kprintf("%02X ", data[i]);
                                if (((i + 1) % 16 == 0) || i == size - 1)
                                {
                                    rt_kprintf("\n");
                                }
                            }
                            rt_kprintf("\n");
                        }
                        rt_free(data);
                    }
                    else
                    {
                        rt_kprintf("Low memory!\n");
                    }
                }
            }
            else if (!strcmp(operator, "write"))
            {
                if (argc < 4)
                {
                    rt_kprintf("Usage: %s.\n", help_info[CMD_WRITE_INDEX]);
                    return;
                }
                else
                {
                    addr = strtol(argv[2], NULL, 0);
                    size = argc - 3;
                    uint8_t *data = rt_malloc(size);
                    if (data)
                    {
                        for (i = 0; i < size; i++)
                        {
                            data[i] = strtol(argv[3 + i], NULL, 0);
                        }
                        if (flash_dev)
                        {
                            result = flash_dev->ops.write(addr, data, size);
                        }
                        else if (part_dev)
                        {
                            result = fal_partition_write(part_dev, addr, data, size);
                        }
                        if (result >= 0)
                        {
                            rt_kprintf("Write data success. Start from 0x%08X, size is %ld.\n", addr, size);
                            rt_kprintf("Write data: ");
                            for (i = 0; i < size; i++)
                            {
                                rt_kprintf("%d ", data[i]);
                            }
                            rt_kprintf(".\n");
                        }
                        rt_free(data);
                    }
                    else
                    {
                        rt_kprintf("Low memory!\n");
                    }
                }
            }
            else if (!rt_strcmp(operator, "erase"))
            {
                if (argc < 4)
                {
                    rt_kprintf("Usage: %s.\n", help_info[CMD_ERASE_INDEX]);
                    return;
                }
                else
                {
                    addr = strtol(argv[2], NULL, 0);
                    size = strtol(argv[3], NULL, 0);
                    if (flash_dev)
                    {
                        result = flash_dev->ops.erase(addr, size);
                    }
                    else if (part_dev)
                    {
                        result = fal_partition_erase(part_dev, addr, size);
                    }
                    if (result >= 0)
                    {
                        rt_kprintf("Erase data success. Start from 0x%08X, size is %ld.\n", addr, size);
                    }
                }
            }
            else if (!strcmp(operator, "bench"))
            {
                if (argc < 3)
                {
                    rt_kprintf("Usage: %s.\n", help_info[CMD_BENCH_INDEX]);
                    return;
                }
                else if ((argc > 3 && strcmp(argv[3], "yes")) || argc < 4)
                {
                    rt_kprintf("DANGER: It will erase full chip or partition! Please run 'fal bench %d yes'.\n", strtol(argv[2], NULL, 0));
                    return;
                }
                /* full chip benchmark test */
                uint32_t start_time, time_cast;
                size_t write_size = strtol(argv[2], NULL, 0), read_size = strtol(argv[2], NULL, 0), cur_read_size;
                uint8_t *write_data = (uint8_t *)rt_malloc(write_size), *read_data = (uint8_t *)rt_malloc(read_size);

                if (write_data && read_data)
                {
                    memset(write_data, 0x55, write_size);
                    if (flash_dev)
                    {
                        size = flash_dev->len;
                    }
                    else if (part_dev)
                    {
                        size = part_dev->len;
                    }
                    /* benchmark testing */
                    rt_kprintf("Erasing %ld bytes data, waiting...\n", size);
                    start_time = rt_tick_get();
                    if (flash_dev)
                    {
                        result = flash_dev->ops.erase(0, size);
                    }
                    else if (part_dev)
                    {
                        result = fal_partition_erase(part_dev, 0, size);
                    }
                    if (result >= 0)
                    {
                        time_cast = rt_tick_get() - start_time;
                        rt_kprintf("Erase benchmark success, total time: %d.%03dS.\n", time_cast / RT_TICK_PER_SECOND,
                                time_cast % RT_TICK_PER_SECOND / ((RT_TICK_PER_SECOND * 1 + 999) / 1000));
                    }
                    else
                    {
                        rt_kprintf("Erase benchmark has an error. Error code: %d.\n", result);
                    }
                    /* write test */
                    rt_kprintf("Writing %ld bytes data, waiting...\n", size);
                    start_time = rt_tick_get();
                    for (i = 0; i < size; i += write_size)
                    {
                        if (flash_dev)
                        {
                            result = flash_dev->ops.write(i, write_data, write_size);
                        }
                        else if (part_dev)
                        {
                            result = fal_partition_write(part_dev, i, write_data, write_size);
                        }
                        if (result < 0)
                        {
                            break;
                        }
                    }
                    if (result >= 0)
                    {
                        time_cast = rt_tick_get() - start_time;
                        rt_kprintf("Write benchmark success, total time: %d.%03dS.\n", time_cast / RT_TICK_PER_SECOND,
                                time_cast % RT_TICK_PER_SECOND / ((RT_TICK_PER_SECOND * 1 + 999) / 1000));
                    }
                    else
                    {
                        rt_kprintf("Write benchmark has an error. Error code: %d.\n", result);
                    }
                    /* read test */
                    rt_kprintf("Reading %ld bytes data, waiting...\n", size);
                    start_time = rt_tick_get();
                    for (i = 0; i < size; i += read_size)
                    {
                        if (i + read_size <= size)
                        {
                            cur_read_size = read_size;
                        }
                        else
                        {
                            cur_read_size = size - i;
                        }
                        if (flash_dev)
                        {
                            result = flash_dev->ops.read(i, read_data, cur_read_size);
                        }
                        else if (part_dev)
                        {
                            result = fal_partition_read(part_dev, i, read_data, cur_read_size);
                        }
                        /* data check */
                        if (memcmp(write_data, read_data, cur_read_size))
                        {
                            result = -RT_ERROR;
                            rt_kprintf("Data check ERROR! Please check you flash by other command.\n");
                        }
                        /* has an error */
                        if (result < 0)
                        {
                            break;
                        }
                    }
                    if (result >= 0)
                    {
                        time_cast = rt_tick_get() - start_time;
                        rt_kprintf("Read benchmark success, total time: %d.%03dS.\n", time_cast / RT_TICK_PER_SECOND,
                                time_cast % RT_TICK_PER_SECOND / ((RT_TICK_PER_SECOND * 1 + 999) / 1000));
                    }
                    else
                    {
                        rt_kprintf("Read benchmark has an error. Error code: %d.\n", result);
                    }
                }
                else
                {
                    rt_kprintf("Low memory!\n");
                }
                rt_free(write_data);
                rt_free(read_data);
            }
            else
            {
                rt_kprintf("Usage:\n");
                for (i = 0; i < sizeof(help_info) / sizeof(char*); i++)
                {
                    rt_kprintf("%s\n", help_info[i]);
                }
                rt_kprintf("\n");
                return;
            }
            if (result < 0) {
                rt_kprintf("This operate has an error. Error code: %d.\n", result);
            }
        }
    }
}
MSH_CMD_EXPORT(fal, FAL (Flash Abstraction Layer) operate.);

#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */
