# -*- coding: utf-8 -*-  

import os
import sys
import time
import subprocess
import tempfile

#adb push ./push_test.t "#sync_mod#<208>*path=/adb_sync/*/adb_sync/adb_sync.adb"
#adb pull "#sync_mod#<208>*path=/adb_sync/*/adb_sync/adb_sync.adb" ./pull_test.t

def execute_command(cmdstring, cwd=None, shell=True):
    """Execute the system command at the specified address."""

    if shell:
        cmdstring_list = cmdstring

    sub = subprocess.Popen(cmdstring_list, cwd=cwd, stdin=subprocess.PIPE,
                           stdout=subprocess.PIPE, shell=shell, bufsize=8192)

    stdout_str = ""
    while sub.poll() is None:
        stdout_str += str(sub.stdout.read())
        time.sleep(0.1)

    return stdout_str

def get_dir_size(dir):
    size = 0
    for root, dirs, files in os.walk(dir):
        size += sum([os.path.getsize(os.path.join(root, name)) for name in files])
    return size

def get_transfer_err(adb_out):
    adb_out = adb_out.split('\n')
    err_index = -1
    out_str = None
    for i in adb_out:
        err_index = i.find('adb: error:')
        if err_index >= 0:
            out_str = i[err_index + len('adb: error:') : ]
            break
    return out_str

def get_cmd(cmd, adb_path, serial_number):
    if serial_number != '':
        cmd = '-s ' + serial_number + ' ' + cmd
    if adb_path == '':
        return 'adb ' + cmd + ' '
    else:
        (adb_file_path, adb_file_name) = os.path.split(adb_path)
        if adb_file_name == '':
            return adb_path + 'adb.exe ' + cmd + ' '
        else:
            return adb_path + ' ' + cmd + ' '

def get_push_cmd(adb_path = '', serial_number = ''):
    return get_cmd('push', adb_path, serial_number)

def get_pull_cmd(adb_path = '', serial_number = ''):
    return get_cmd('pull', adb_path, serial_number)

def param_to_str(modname, parameter):
    cmdparam = ''
    if not isinstance(parameter, dict):
        return None
    for key in list(parameter.keys()):
        value = parameter[key]
        if isinstance(value, dict):
            continue
        else:
            if len(cmdparam) == 0:
                cmdparam = '#' + modname + '#' + '<' + '>'
                cmdparam += '*' + str(key) + '=' + str(value)
            else:
                cmdparam += ',' + str(key) + '=' + str(value)
    cmdparam += '*'
    return cmdparam

def adb_read_data(modname, parameter, path='/adb_pull.sync', adb_path = '', serial_number = ''):
    cmdhead = get_pull_cmd(adb_path, serial_number)
    cmdparam = param_to_str(modname, parameter)
    filename = tempfile.gettempdir() + '/' + 'abd_temp.sync'
    if not cmdparam:
        return None
    cmdparam = "\"" + cmdparam + path + "\""
    cmd = cmdhead + ' ' + cmdparam + ' ' + filename
    #recv data by adb
    adb_out = str(execute_command(cmd))
    adb_err = get_transfer_err(adb_out)
    if adb_err:
        print('adb read data err:' + adb_err)
        return None
    with open(filename, 'r') as fp:
        data = fp.read()
    if os.path.exists(filename):
        os.remove(filename)
    # print('filename' + filename)
    # print('cmd' + cmd)
    # print('adb_out' + adb_out)
    # print('data:' + data)
    return data

def adb_write_data(modname, parameter, data, path='/adb_push.sync', adb_path = '', serial_number = ''):
    cmdhead = get_push_cmd(adb_path, serial_number)
    filename = ''
    cmdparam = param_to_str(modname, parameter)
    if not cmdparam:
        return ''
    cmdparam = "\"" + cmdparam + path + "\""
    f_temp = tempfile.NamedTemporaryFile(delete=False)
    filename = f_temp.name
    f_temp.write(data)
    f_temp.close()
    cmd = cmdhead + ' ' + filename + ' ' + cmdparam
    #send data by adb
    adb_out = str(execute_command(cmd))
    adb_err = get_transfer_err(adb_out)
    if os.path.exists(filename):
        os.remove(filename)
    if adb_err:
        print('adb write data err:' + adb_err)
        return adb_err
    else:
        return 'OK'
    # print('adb_out' + adb_out)

def adb_push_file(loacl_path, remote_path, file_list, adb_path = '', serial_number = ''):
    cmdhead = get_push_cmd(adb_path, serial_number)
    adb_err = None
    name_max = 0
    success_list = []
    failure_list = []
    result = {}

    for item in file_list:
        if len(item) > name_max:
            name_max = len(item)
    name_max += 9
    for item in file_list:
        starttime = time.time()
        if item.count(loacl_path, 0, len(loacl_path)):
            pathname = item[len(loacl_path) : ]
        else:
            pathname = item
        [dirname,filename]=os.path.split(pathname)
        cmd = cmdhead + ' ' + "\"" + loacl_path + dirname + '/' + filename + "\"" + ' ' + "\"" + remote_path + dirname + '/' + filename + "\""
        cmd = cmd.replace('\\', '/')
        cmd = cmd.replace('//', '/')
        out_str = 'sync ' + dirname + '/' + filename
        sys.stdout.write(out_str)
        sys.stdout.flush()
        adb_out = str(execute_command(cmd))
        adb_err = get_transfer_err(adb_out)
        endtime = time.time()
        if adb_err:
            out_str = '\r' + 'err! ' + dirname + '/' + filename + adb_err
            failure_list.append(item)
        else:
            fsize = 0
            if os.path.isdir(loacl_path + dirname + '/' + filename):
                fsize = get_dir_size(loacl_path + dirname + '/' + filename)
            else:
                fsize = os.path.getsize(loacl_path + dirname + '/' + filename)
            out_str = '\r' + out_str + ' ' * (name_max - len(out_str)) + str(round( fsize / 1024 / ((endtime - starttime)), 1)) + 'KB/s'
            success_list.append(item)
        print(out_str)
    result['success'] = success_list
    result['failure'] = failure_list
    return result
