# -*- coding: utf-8 -*-  

import os
import re
import sys
import json
import copy
import time
import hashlib
import datetime
import subprocess
from adb_transfer import adb_push_file
from adb_transfer import adb_read_data
from adb_transfer import adb_write_data

loacl_path = ''
remote_path = ''
adb_exec_path = ''
adb_serial_number = ''

adb_sync_mod_name = 'sync_mod'
adb_sync_key = 'path'
name = 'name'
md5 = 'md5'
buff_size = 512

def get_pc_dir_info(path):
    result = []
    paths = os.listdir(path)
    for i, item in enumerate(paths):
        sub_path = os.path.join(path, item)
        if os.path.isdir(sub_path):
            file_info = {}
            file_info[name] = path[len(loacl_path) :] + item + '/'
            file_info[md5] = ''
            result.append(file_info)
            result += get_pc_dir_info(sub_path + '/')
        else:
            myhash = hashlib.md5()
            f = open(sub_path,'rb')
            while True:
                b = f.read(8096)
                if not b :
                    break
                myhash.update(b)
            f.close()
            file_info = {}
            file_info[name] = path[len(loacl_path) :] + item
            file_info[md5] = str(myhash.hexdigest())
            result.append(file_info)
    return result

def get_dev_dir_info(path):
    param = {}
    result = None
    param[adb_sync_key] = path
    data = adb_read_data(adb_sync_mod_name, param, adb_path = adb_exec_path, serial_number = adb_serial_number)
    if data:
        result = json.loads(data)
    return result

def get_sync_info(pc_info, dev_info):
    sync_info = {}
    delete_list = []
    sync_list = []
    temp = {}

    for item in json_pc:
        temp[item[name]] = item[md5]
    for item in dev_info:
        if not item[name] in list(temp.keys()):
            delete_list.append(item[name])
        elif item[md5] == temp[item[name]]:
            del temp[item[name]]

    for item in list(temp.keys()):
        sync_list.append(item)

    sync_info['delete'] = delete_list
    sync_info['sync'] = sync_list

    return sync_info

def dev_file_delete(path, delete_list):
    buff = ''
    curr_len = 0
    for item in delete_list:
        if curr_len + len(item) < buff_size - 4:
            if curr_len == 0:
                buff = '['
                buff += '\"' + item + '\"'
            else:
                buff += ',' + '\"' + item + '\"'
            curr_len = len(buff)
        else:
            buff += ']'
            buff += b'\x00' * (buff_size-curr_len)
            curr_len = 0
    if curr_len != 0:
        buff += ']'
        buff += b'\x00' * (buff_size-curr_len)
        curr_len = 0
    param = {}
    param[adb_sync_key] = path
    adb_write_data(adb_sync_mod_name, param, buff, adb_path = adb_exec_path, serial_number = adb_serial_number)

def dev_file_delete_(path, delete_list):
    success_list = []
    failure_list = []
    result = {}
    for item in delete_list:
        buff = []
        buff.append(item)
        json_str = json.dumps(buff).encode('utf-8') + b'\x00'
        if len(json_str) > buff_size:
            print('err! file name is to long')
        else:
            param = {}
            param[adb_sync_key] = path
            print('delete ' + item)
            if adb_write_data(adb_sync_mod_name, param, json_str, adb_path = adb_exec_path, serial_number = adb_serial_number) == 'OK':
                success_list.append(item)
            else:
                failure_list.append(item)
    result['success'] = success_list
    result['failure'] = failure_list
    return result

def dev_file_sync(sync_list):
    return adb_push_file(loacl_path, remote_path, sync_list, adb_path = adb_exec_path, serial_number = adb_serial_number)

def list_merge_path(list):
    i = 0
    j = 0
    sort_res = sorted(list,key = lambda i:len(i),reverse=False)
    while i < len(sort_res):
        j = i + 1
        while j < len(sort_res):
            t1 = sort_res[i]
            t2 = sort_res[j]
            if t2.count(t1, 0, len(t1)) > 0:
                sort_res.pop(j)
            else:
                j += 1
        i += 1
    return sort_res

def list_filtration_folders(list):
    res = []
    for item in list:
        if item[-1] != '/':
            res.append(item)
    return res

def file_path_check():
    global loacl_path
    global remote_path
    global adb_exec_path

    loacl_path = loacl_path.replace('\\', '/')
    remote_path = remote_path.replace('\\', '/')
    if adb_exec_path != '':
        adb_exec_path = adb_exec_path.replace('\\', '/')
        if not os.path.exists(adb_exec_path):
            print(adb_exec_path + ' adb exec path not exist')
            return False
        else:
            if os.path.isdir(adb_exec_path):
                if adb_exec_path[-1] != '/':
                    adb_exec_path += '/'
                if not os.path.exists(adb_exec_path + 'adb.exe'):
                    print(adb_exec_path + 'adb.exe' + ' not exist')
                    return False
    if loacl_path[-1] != '/':
        loacl_path = loacl_path + '/'
    if remote_path[-1] != '/':
        remote_path = remote_path + '/'

    if not os.path.exists(loacl_path):
        print(loacl_path + ' folder does not exist')
        return False
    return True

def string_insensitive_sort(str_list):
    listtemp = [(x.lower(),x) for x in str_list]
    listtemp.sort()
    return [x[1] for x in listtemp]

def user_parameter_parsing(args):
    global adb_exec_path
    global adb_serial_number

    for i in range(0, len(args)):
        if args[i] == '-s':
            adb_serial_number = args[i+1]
        elif args[i] == '-p':
            adb_exec_path = args[i+1]

if __name__=='__main__':
    loacl_path = sys.argv[1] if len(sys.argv) > 1 else '.'
    remote_path = sys.argv[2] if len(sys.argv) > 2 else '/'
    if len(sys.argv) > 3:
        user_parameter_parsing(sys.argv[3:])
    if file_path_check() == False:
        exit(0)
    starttime = time.time()
    print('loacl:' + loacl_path)
    print('remote:' + remote_path)
    print('---------------- get dev info ----------------')
    json_dev = get_dev_dir_info(remote_path)
    if not isinstance(json_dev, list):
        exit(0)
    print('----------------  get pc info ----------------')
    json_pc = get_pc_dir_info(loacl_path)
    print('----------------  sync check  ----------------')
    sync_info = get_sync_info(json_pc, json_dev)
    sync_list = list_filtration_folders(sync_info['sync'])
    delete_list = list_merge_path(sync_info['delete'])
    print('----------------   sync file  ----------------')
    sync_list_sort = string_insensitive_sort(sync_list)
    sync_res = dev_file_sync(sync_list_sort)
    print('----------------  delete file ----------------')
    delete_list_sort = string_insensitive_sort(delete_list)
    delete_res = dev_file_delete_(remote_path, delete_list_sort)
    sync_try_res = {}
    if len(sync_res['failure']) > 0:
        print('----------------  sync retry  ----------------')
        sync_try_res = dev_file_sync(sync_res['failure'])
    delete_try_res = {}
    if len(delete_res['failure']) > 0:
        print('---------------- delete retry ----------------')
        delete_try_res = dev_file_delete_(delete_res['failure'])
    print('----------------   sync end   ----------------')
    pc_list = []
    for item in json_pc:
        pc_list.append(item[name])
    all_count = len(list_filtration_folders(sync_info['sync']))
    delt_count = len(list_filtration_folders(sync_info['delete']))
    fail_count = len(sync_try_res['failure']) if sync_try_res else 0
    sync_count = all_count - fail_count
    skip_count = len(list_filtration_folders(pc_list)) - sync_count
    endtime = time.time()
    print('sync:' + str(sync_count) + '    fail:' + str(fail_count) + '    skip:' + str(skip_count) + '    delete:' + str(delt_count) + '    time:' + str(round(endtime - starttime, 2)) + 's')
