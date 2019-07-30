"""
uos 模块包含了对文件系统的访问操作，是对应 CPython 模块的一个子集。
"""

def chdir(path) -> None:
    """更改当前目录。"""
    ...

def getcwd() -> None:
    """获取当前目录。"""
    ...

def listdir(dir : str) -> None:
    """没有参数就列出当前目录，否则列出给定目录。"""
    ...

def mkdir(path : str) -> None:
    """创建一个目录。"""
    ...

def remove(path : str) -> None:
    """删除文件。"""
    ...

def rmdir(path : str) -> None:
    """删除目录。"""
    ...

def rename(old_path : str, new_path : str) -> None:
    """重命名文件或者文件夹。"""
    ...

def stat(path : str) -> None:
    """获取文件或目录的状态。"""
    ...

def sync() -> None:
    """同步所有的文件系统。"""
    ...
	
def mkfs(fs_type : str, dev_name : str) -> None:
    """在指定的设备上创建 fs_type 类型的文件系统。example: os.mkfs("elm", "fs")"""
    ...
