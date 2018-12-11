# qrcode

## 1、介绍

**qrcode** 是一个用于将字符串生成二维码的软件包。该软件包是 **RT-Thread** 基于 [ricmoo/QRCode](https://github.com/ricmoo/QRCode) 开源库的移植。

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| examples | 例子目录，并有相应的一些说明 |
| inc  | 头文件目录 |
| src  | 源代码目录 |

### 1.2 许可证

`qrcode` 软件包延用 `QRCode` 软件包许可协议，请见 `qrcode/LICENSE` 文件。

### 1.3 依赖

- RT-Thread 3.0+

## 2、如何打开 qrcode

使用 qrcodepackage 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```shell
RT-Thread online packages
    tools packages --->
        [*] qrcode: A simple library for generating QR codes in C
            [*] Enable qrcode sample
```

- `Enable qrcode sample`: 开启二维码例程；

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 qrcode

### 生成二维码

在使用 qrcode 软件包时首先要定义一个结构体来管理二维码，

```c
QRCode qrcode;
```

然后根据要版本号来申请动态内存，用来保存生成的二维码，

```c
uint8_t *qrcodeBytes = (uint8_t *)rt_calloc(1, qrcode_getBufferSize(DEFAULT_QR_VERSION));
```

最后使用二维码生成函数生成二维码。

```c
qrcode_initText(&qrcode, qrcodeBytes, DEFAULT_QR_VERSION, ECC_LOW, "HELLO WORLD");
```

### 打印二维码

生成的二维码是点阵数据，8个点构成一个 Byte。显示二维码可以使用以下代码

```c
for (uint8 y = 0; y < qrcode.size; y++) {
    for (uint8 x = 0; x < qrcode.size; x++) {
        if (qrcode_getModule(&qrcode, x, y) {
            rt_kprintf("**");
        } else {
            rt_kprintf("  ");
        }
    }
    Serial.print("\n");
}
```

## 4、示例演示

本示例需要在 qrcode 软件包中开启二维码例程。

在 MSH 中输入命令 qrcode RT-Thread，可以在串口助手上打印出一个二维码来，利用手机扫码软件扫描后可以看到结果为 RT-Thread。

![qrcode](figures/qrcode.gif)

## 5、注意事项

- 在生成二维码时，使用的版本越高，需要申请的动态内存就越大。
- 扫描串口助手上的二维码时，注意设置串口助手白底黑字。
- 扫描串口助手上的二维码时，注意缩小文字显示，让*间的间距减小。

## 6、联系方式 & 感谢

* 维护：[qgyhd1234](https://github.com/qgyhd1234)
* 主页：https://github.com/RT-Thread-packages/qrcode
