#ifndef _FILE_SYNC_S_H
#define _FILE_SYNC_S_H

#include <stdint.h>

#define MKID(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))

#define ID_LSTAT_V1 MKID('S', 'T', 'A', 'T')
#define ID_STAT_V2 MKID('S', 'T', 'A', '2')
#define ID_LSTAT_V2 MKID('L', 'S', 'T', '2')
#define ID_LIST MKID('L', 'I', 'S', 'T')
#define ID_SEND MKID('S', 'E', 'N', 'D')
#define ID_RECV MKID('R', 'E', 'C', 'V')
#define ID_DENT MKID('D', 'E', 'N', 'T')
#define ID_DONE MKID('D', 'O', 'N', 'E')
#define ID_DATA MKID('D', 'A', 'T', 'A')
#define ID_OKAY MKID('O', 'K', 'A', 'Y')
#define ID_FAIL MKID('F', 'A', 'I', 'L')
#define ID_QUIT MKID('Q', 'U', 'I', 'T')
#define ID_CMD5 MKID('C', 'M', 'D', '5') /* calculate md5,extend by heyuanjie87 */

#pragma pack(1)

struct file_syncreq
{
    uint32_t id;          // ID_STAT, et cetera.
    uint32_t path_length; // <= 1024
    // Followed by 'path_length' bytes of path (not NUL-terminated).
};

struct filesync_ext
{
    rt_thread_t worker;
    adb_queue_t recv_que;
    int rque_buf[8];
    struct adb_packet *cur;
    struct rt_event join; 
};

union file_syncmsg 
{
    struct
    {
        uint32_t id;
        uint32_t mode;
        uint32_t size;
        uint32_t time;
    } stat_v1;
#if 0 //not support in rtthread
    struct
    {
        uint32_t id;
        uint32_t error;
        uint64_t dev;
        uint64_t ino;
        uint32_t mode;
        uint32_t nlink;
        uint32_t uid;
        uint32_t gid;
        uint64_t size;
        int64_t atime;
        int64_t mtime;
        int64_t ctime;
    } stat_v2;
#endif
    struct
    {
        uint32_t id;
        uint32_t mode;
        uint32_t size;
        uint32_t time;
        uint32_t namelen;
    } dent;
    struct
    {
        uint32_t id;
        uint32_t size;
    } data;
    struct
    {
        uint32_t id;
        uint32_t msglen;
    } status;
    struct
    {
        uint32_t id;
        uint8_t value[16]; //ADB official not support 
    } md5;
};
#pragma pack()

#define SYNC_DATA_MAX (512)

#endif
