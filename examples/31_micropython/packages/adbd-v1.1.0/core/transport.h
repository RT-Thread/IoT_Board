#ifndef __ADB_TR_H_
#define __ADB_TR_H_

struct adb;

#define TR_TCPIP 1
#define TR_USB   2

#define TRE_READ   0x01
#define TRE_WRITE  0x02
#define TRE_ERROR  0x04

struct adb_tr_ops
{
    int (*read)(int fd, void *buf, int size);
    int (*write)(int fd, void *buf, int size);
    int (*poll)(int fd, int evt, int ms);
    void (*close)(int fd);
};

int adb_transport_register(int trtype, int fd, const struct adb_tr_ops *ops);
void adb_transport_unregister(int trtype);

#endif
