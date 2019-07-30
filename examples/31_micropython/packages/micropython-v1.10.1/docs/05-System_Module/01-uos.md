## **uos** – 基本的操作系统服务

`uos` 模块包含了对文件系统的访问操作，是对应 CPython 模块的一个子集。

### 函数

#### **uos.chdir**(path)  
更改当前目录。

#### **uos.getcwd**()  
获取当前目录。

#### **uos.listdir**([dir])
没有参数就列出当前目录，否则列出给定目录。

#### **uos.mkdir**(path)  
创建一个目录。

#### **uos.remove**(path)  
删除文件。

#### **uos.rmdir**(path)  
删除目录。

#### **uos.rename**(old_path, new_path)  
重命名文件或者文件夹。

#### **uos.stat**(path)  
获取文件或目录的状态。

#### **uos.sync**()  
同步所有的文件系统。

### 示例

```
>>> import uos
>>> uos.                        # Tab 
__name__        uname           chdir           getcwd
listdir         mkdir           remove          rmdir
stat            unlink          mount           umount
>>> uos.mkdir("rtthread")
>>> uos.getcwd()
'/'
>>> uos.chdir("rtthread")
>>> uos.getcwd()
'/rtthread'
>>> uos.listdir()
['web_root', 'rtthread', '11']
>>> uos.rmdir("11")
>>> uos.listdir()
['web_root', 'rtthread']
>>> 
```

更多内容可参考 [uos](http://docs.micropython.org/en/latest/pyboard/library/uos.html) 。
