## **usocket** – 套接字模块

`usocket` 模块提供对BSD套接字接口的访问。 

### 常数

#### 地址簇
- socket.AF_INET =2 — TCP/IP – IPv4
- socket.AF_INET6 =10 — TCP/IP – IPv6

#### 套接字类型
- socket.SOCK_STREAM =1 — TCP流
- socket.SOCK_DGRAM =2 — UDP数据报
- socket.SOCK_RAW =3 — 原始套接字
- socket.SO_REUSEADDR =4 — socket可重用

#### IP协议号
- socket.IPPROTO_TCP =16
- socket.IPPROTO_UDP =17

#### 套接字选项级别
- socket.SOL_SOCKET =4095

### 函数

#### **socket.socket**

`socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)` 

创建新的套接字，使用指定的地址、类型和协议号。

#### **socket.getaddrinfo**(host, port) 
将主机域名（host）和端口（port）转换为用于创建套接字的5元组序列。元组列表的结构如下:

```
(family, type, proto, canonname, sockaddr)
```

示例：

```
>>> info = socket.getaddrinfo("rt-thread.org", 10000)
>>> print(info)
[(2, 1, 0, '', ('118.31.15.152', 10000))]
```

#### **socket.close**()  
关闭套接字。一旦关闭后，套接字所有的功能都将失效。远端将接收不到任何数据 (清理队列数据后)。 虽然在垃圾回收时套接字会自动关闭，但还是推荐在必要时用 close() 去关闭。

#### **socket.bind**(address)  
将套接字绑定到地址，套接字不能是已经绑定的。

#### **socket.listen**([backlog])  
监听套接字，使服务器能够接收连接。
```
backlog：接受套接字的最大个数，至少为0，如果没有指定，则默认一个合理值。
```

#### **socket.accept**()  
接收连接请求。 
**注意：** 
   只能在绑定地址端口号和监听后调用，返回 conn 和 address。

```
conn：新的套接字对象，可以用来收发消息
address：连接到服务器的客户端地址
```

#### **socket.connect**(address)  
连接服务器。

```
address：服务器地址和端口号的元组或列表
```

#### **socket.send**(bytes)  
发送数据，并返回成功发送的字节数，返回字节数可能比发送的数据长度少。

```
bytes：bytes类型数据
```

#### **socket.recv**(bufsize)  
接收数据，返回接收到的数据对象。

```
bufsize：指定一次接收的最大数据量
```

示例：

```
data = conn.recv(1024)
```

#### **socket.sendto**(bytes, address)  
发送数据，目标由address决定，常用于UDP通信，返回发送的数据大小。

```
bytes：bytes类型数据
address：目标地址和端口号的元组
```

示例：

```
data = sendto("hello RT-Thread", ("192.168.10.110", 100))
```

#### **socket.recvfrom**(bufsize)  
接收数据，常用于UDP通信，并返回接收到的数据对象和对象的地址。

```
bufsize：指定一次接收的最大数据量
```

示例：

```
data,addr=fd.recvfrom(1024)
```

#### **socket.setsockopt**(level, optname, value)  
根据选项值设置套接字。

```
level：套接字选项级别
optname：套接字的选项
value：可以是一个整数，也可以是一个表示缓冲区的bytes类对象。
```

示例：

```
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
```

#### **socket.settimeout**(value)  
设置超时时间，单位：秒。 
示例：

```
s.settimeout(2)
```

#### **socket.setblocking**(flag)  
设置阻塞或非阻塞模式: 如果 flag 是 false，设置非阻塞模式。

#### **socket.read**([size])  
Read up to size bytes from the socket. Return a bytes object. If size is not given, it reads all data available from the socket until EOF; as such the method will not return until the socket is closed. This function tries to read as much data as requested (no “short reads”). This may be not possible with non-blocking socket though, and then less data will be returned.

#### **socket.readinto**(buf[, nbytes])  
Read bytes into the buf. If nbytes is specified then read at most that many bytes. Otherwise, read at most len(buf) bytes. Just as read(), this method follows “no short reads” policy.
Return value: number of bytes read and stored into buf.

#### **socket.readline**()  
接收一行数据，遇换行符结束，并返回接收数据的对象 。 

#### **socket.write**(buf)  
将字节类型数据写入套接字，并返回写入成功的数据大小。 

### 示例

#### TCP Server example

```
>>> import usocket 
>>> s = usocket.socket(usocket.AF_INET,usocket.SOCK_STREAM)  # Create STREAM TCP socket
>>> s.bind(('192.168.12.32', 6001))   
>>> s.listen(5)
>>> s.setblocking(True)
>>> sock,addr=s.accept()              
>>> sock.recv(10)                    
b'rt-thread\r'
>>> s.close()
```

#### TCP Client example

```
>>> import usocket 
>>> s = usocket.socket(usocket.AF_INET,usocket.SOCK_STREAM)
>>> s.connect(("192.168.10.110",6000))  
>>> s.send("micropython")               
11
>>> s.close()
```

`connect to a web site example`:
```
s = socket.socket()
s.connect(socket.getaddrinfo('www.micropython.org', 80)[0][-1])
```

更多的内容可参考 [usocket](http://docs.micropython.org/en/latest/pyboard/library/usocket.html) 。
