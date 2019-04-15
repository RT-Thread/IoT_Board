#!/user/bin/python
# -*- coding: utf-8 -*-

# 1. 准备 bootloader.bin
# 2. 编译输出 rt-thread.bin
# 3. 使用 OTA 打包工具（ota packager），设置为不加密、不压缩方式，固件分区名称为 app，填入版本号，点击开始打包，将 rt-thread.bin 打包为 rt-thread.rbl
# 4. 将 bootloader.bin、rt-thread.rbl 拷贝到 AllBinPackager.py 目录下
# 5. 配置 config.json，填写文件名、分区偏移地址、分区大小（十六进制）
# 6. 在 AllBinPackager.py 所在目录下打开命令行，输入 `python  AllBinPackager.py` 完成 all.bin 打包

import sys
import json

# 解决乱码问题
reload(sys)
sys.setdefaultencoding( "utf-8" )

def load_json_to_dic(file):
    load_dict = {}
    with open(file, 'r') as load_f:
        try:
            load_dict = json.load(load_f)
        except Exception as ex:
            load_dict = {}
            raise Exception, "load json file error!"
    return load_dict

class allbin():
    boot_file     = None
    boot_raw_data = None
    boot_rbl_data = None
    app_file      = None
    app_raw_data  = None
    app_rbl_data  = None

    def file_handler(self, in_file, out_file, part_size = 0, offset_addr = 0):
        self.raw_data = None
        self.rbl_data = None
        file_seek = 0

        if (in_file[-4:] == ".bin"):
            print("=== bin")
            with open(in_file, 'rb') as f:
                f.seek(0, 0)
                self.raw_data = f.read()
        elif (in_file[-4:] == ".rbl"):
            print("--- rbl")
            file_seek = part_size - 96
            with open(in_file, 'rb') as f:
                f.seek(0, 0)
                self.rbl_data = f.read(96)
                self.raw_data = f.read()

        with open(out_file, 'wb') as f:
            if (self.raw_data != None):
                f.write(self.raw_data)
            if (self.rbl_data != None):
                f.seek(file_seek, 0)
                f.write(self.rbl_data)
        return

    def allbin_packager(self, cfg_file = "config.json"):
        try:
            config_dict = load_json_to_dic(cfg_file)
            if len(config_dict) == 0:
                return -1
            if not (config_dict.has_key("bootloader") and config_dict.has_key("app")):
                return -1
            self.boot_file = config_dict["bootloader"]["file"]
            self.app_file  = config_dict["app"]["file"]

            print(self.boot_file[-4:])
            print(self.app_file[-4:])

            print("boot")
            self.boot_part_size = int(config_dict["bootloader"]["partition_size"], 16)
            self.boot_offset_addr = int(config_dict["bootloader"]["partition_offset_addr"], 16)
            self.file_handler(self.boot_file, "test_boot_rbl.bin", self.boot_part_size, self.boot_offset_addr)

            print("app")
            self.app_part_size = int(config_dict["app"]["partition_size"], 16)
            self.app_offset_addr = int(config_dict["app"]["partition_offset_addr"], 16)
            self.file_handler(self.app_file, "test_app_rbl.bin", self.app_part_size, self.app_offset_addr)

            with open("all.bin", 'wb') as allbin_f:
                with open("test_boot_rbl.bin", 'rb') as boot_f:
                    allbin_f.write(boot_f.read())
                with open("test_app_rbl.bin", 'rb') as app_f:
                    allbin_f.seek(self.app_offset_addr, 0)
                    allbin_f.write(app_f.read())

        except Exception as ex:
            raise Exception, "all bin packager failed!"
        return 0

if __name__ == "__main__":
    print('RT-Thread all.bin packager v1.0.0')
    all_bin_o = allbin()
    all_bin_o.allbin_packager()
    print('all.bin packager success!')
