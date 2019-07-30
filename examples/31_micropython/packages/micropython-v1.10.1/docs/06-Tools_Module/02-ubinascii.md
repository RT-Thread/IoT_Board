## **ubinascii** – 二进制/ ASCII转换

`ubinascii` 模块包含许多在二进制和各种 ascii 编码的二进制表示之间转换的方法。

### 函数

#### **ubinascii.hexlify**(data[, sep])  
将字符串转换为十六进制表示的字符串。 

示例：

```
>>> ubinascii.hexlify('hello RT-Thread')
b'68656c6c6f2052542d546872656164'
>>> ubinascii.hexlify('summer')
b'73756d6d6572'
```
如果指定了第二个参数sep，它将用于分隔两个十六进制数。 

示例：

```
如果指定了第二个参数sep，它将用于分隔两个十六进制数。 
示例：
>>> ubinascii.hexlify('hello RT-Thread'," ")
b'68 65 6c 6c 6f 20 52 54 2d 54 68 72 65 61 64'
>>> ubinascii.hexlify('hello RT-Thread',",")
b'68,65,6c,6c,6f,20,52,54,2d,54,68,72,65,61,64'
```

#### **ubinascii.unhexlify**(data)  
转换十六进制字符串为二进制字符串，功能和 hexlify 相反。 

示例：

```
>>> ubinascii.unhexlify('73756d6d6572')
b'summer'
```

#### **ubinascii.a2b_base64**(data)  
Base64编码的数据转换为二进制表示。返回字节串。

#### **ubinascii.b2a_base64**(data)  
编码base64格式的二进制数据。返回的字符串。

更多内容可参考 [ubinascii](http://docs.micropython.org/en/latest/pyboard/library/ubinascii.html)  。
