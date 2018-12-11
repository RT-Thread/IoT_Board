#include <rtthread.h>
#include "qrcode.h"

#if defined(RT_USING_FINSH) && defined(FINSH_USING_MSH)
#include <finsh.h>
static void qrcode(uint8_t argc, char **argv)
{
#define DEFAULT_QR_VERSION 3
#define DEFAULT_QR_STRING "HELLO WORLD"

    QRCode qrc;
    uint8_t x, y, *qrcodeBytes = (uint8_t *)rt_calloc(1, qrcode_getBufferSize(DEFAULT_QR_VERSION));
    int8_t result;
    char *qrstr = DEFAULT_QR_STRING;

    if (qrcodeBytes)
    {
        if (argc > 1)
        {
            qrstr = argv[1];
        }

        result = qrcode_initText(&qrc, qrcodeBytes, DEFAULT_QR_VERSION, ECC_LOW, qrstr);

        if (result >= 0)
        {
            rt_kprintf("\n");
            for (y = 0; y < qrc.size; y++)
            {
                for (x = 0; x < qrc.size; x++)
                {
                    if (qrcode_getModule(&qrc, x, y))
                    {
                        rt_kprintf("**");
                    }
                    else
                    {
                        rt_kprintf("  ");
                    }
                }
                rt_kprintf("\n");
            }
        }
        else
        {
            rt_kprintf("QR CODE(%s) General FAILED(%d)\n", qrstr, result);
        }
        rt_free(qrcodeBytes);
    }
    else
    {
        rt_kprintf("Warning: no memory!\n");
    }
}
MSH_CMD_EXPORT(qrcode, qrcode generator: qrcode [string]);
#endif /* defined(RT_USING_FINSH) && defined(FINSH_USING_MSH) */