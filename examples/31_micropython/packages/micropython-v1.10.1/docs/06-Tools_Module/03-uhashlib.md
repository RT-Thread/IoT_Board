## **uhashlib** – 哈希算法

`uhashlib` 模块实现了二进制数据哈希算法。

### 算法功能

#### **SHA256** 
当代的散列算法（SHA2系列），它适用于密码安全的目的。被包含在 MicroPython 内核中，除非有特定的代码大小限制，否则推荐任何开发板都支持这个功能。

#### **SHA1**
上一代的算法，不推荐新的应用使用这种算法，但是 SHA1 算法是互联网标准和现有应用程序的一部分，所以针对网络连接便利的开发板会提供这种功能。

#### **MD5** 
一种遗留下来的算法，作为密码使用被认为是不安全的。只有特定的开发板，为了兼容老的应用才会提供这种算法。

### 函数

#### **class uhashlib.sha256**([data])  
创建一个SHA256哈希对象并提供 data 赋值。

#### **class uhashlib.sha1**([data])  
创建一个SHA1哈希对象并提供 data 赋值。

#### **class uhashlib.md5**([data])  
创建一个MD5哈希对象并提供 data 赋值。

#### **hash.update**(data)  
将更多二进制数据放入哈希表中。

#### **hash.digest**()  
返回字节对象哈希的所有数据。调用此方法后，将无法将更多数据送入哈希。

#### **hash.hexdigest**()  
此方法没有实现， 使用 ubinascii.hexlify(hash.digest()) 达到类似效果。

更多内容可参考 [uhashlib](http://docs.micropython.org/en/latest/pyboard/library/uhashlib.html)  。
