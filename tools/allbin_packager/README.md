# all.bin 打包说明

1. 准备 bootloader.bin
2. 编译输出 rt-thread.bin
3. 使用 OTA 打包工具（ota packager），设置为不加密、不压缩方式，固件分区名称为 app，填入版本号，点击开始打包，将 rt-thread.bin 打包为 rt-thread.rbl
4. 将 bootloader.bin、rt-thread.rbl 拷贝到 AllBinPackager.py 目录下
5. 配置 config.json，填写文件名、分区偏移地址、分区大小（十六进制）
6. 在 AllBinPackager.py 所在目录下打开命令行，输入 `python  AllBinPackager.py` 完成 all.bin 打包
