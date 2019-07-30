"""
usocket 模块提供对BSD套接字接口的访问。
"""

AF_INET   = ...  # type: int
AF_INET6  = ...  # type: int

SOCK_STREAM = ...  # type: int 
SOCK_DGRAM = ...  # type: int
SOCK_RAW = ...  # type: int
SO_REUSEADDR = ...  # type: int

IPPROTO_TCP  = ...  # type: int
IPPROTO_UDP  = ...  # type: int

class socket(family, type, protocol) -> None:
    """
    创建新的套接字，使用指定的地址、类型和协议号。
    - usocket.socket(usocket.AF_INET,usocket.SOCK_STREAM)
    """
    ...

    def __init__(self) -> None:
    ...

    def getaddrinfo(host, port) -> None:
        """
        将主机域名（host）和端口（port）转换为用于创建套接字的5元组序列。元组列表的结构如下:

        - (family, type, proto, canonname, sockaddr)
        示例：

        - info = socket.getaddrinfo("rt-thread.org", 10000)
        - print(info)
        - [(2, 1, 0, '', ('118.31.15.152', 10000))]
        """
        ...

    def close() -> None:
        """关闭套接字。一旦关闭后，套接字所有的功能都将失效。远端将接收不到任何数据 (清理队列数据后)。 虽然在垃圾回收时套接字会自动关闭，但还是推荐在必要时用 close() 去关闭。"""
        ...

    def bind(address) -> None:
        """将套接字绑定到地址，套接字不能是已经绑定的。"""
        ...

    def listen(backlog) -> None:
        """
        listen([backlog])
        监听套接字，使服务器能够接收连接。
        backlog：接受套接字的最大个数，至少为0，如果没有指定，则默认一个合理值。
        """
        ...

    def accept() -> None:
        """
        接收连接请求。 注意： 只能在绑定地址端口号和监听后调用，返回 conn 和 address。

        - conn：新的套接字对象，可以用来收发消息
        - address：连接到服务器的客户端地址
        """
        ...

    def connect(address) -> None:
        """
        连接服务器。

       - address：服务器地址和端口号的元组或列表
        """
        ...

    def send(bytes) -> None:
        """
        发送数据，并返回成功发送的字节数，返回字节数可能比发送的数据长度少。

        - bytes：bytes类型数据
        """
        ...

    def recv(bufsize) -> None:
        """
        接收数据，返回接收到的数据对象。

        - bufsize：指定一次接收的最大数据量
        示例：

        - data = conn.recv(1024)
        """
        ...

    def sendto(bytes, address) -> None:
        """
        送数据，目标由address决定，常用于UDP通信，返回发送的数据大小。

        - bytes：bytes类型数据
        - address：目标地址和端口号的元组

        示例：

        - data = sendto("hello RT-Thread", ("192.168.10.110", 100))
        """
        ...

    def recvfrom(bufsize) -> None:
        """
        接收数据，常用于UDP通信，并返回接收到的数据对象和对象的地址。

        - bufsize：指定一次接收的最大数据量
        示例：

        - data,addr=fd.recvfrom(1024)
        """
        ...

    def setsockopt(level, optname, value) -> None:
        """
        根据选项值设置套接字。

        - level：套接字选项级别
        - optname：套接字的选项
        - value：可以是一个整数，也可以是一个表示缓冲区的bytes类对象。
        示例：

        - s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        """
        ...

    def settimeout(value) -> None:
        """
        设置超时时间，单位：秒。 示例：

        s.settimeout(2)
        """
        ...

    def setblocking(flag) -> None:
        """
        设置阻塞或非阻塞模式: 如果 flag 是 false，设置非阻塞模式。
        """
        ...

    def read(size) -> None:
        """
        - read([size])
        Read up to size bytes from the socket. 
        Return a bytes object. If size is not given, it reads all data available from the socket until EOF; 
        as such the method will not return until the socket is closed. This function tries to read as much data as requested (no “short reads”). 
        This may be not possible with non-blocking socket though, and then less data will be returned.
        """
        ...

    def readinto(buf) -> None:
        """
        readinto(buf[, nbytes])
        Read bytes into the buf. 
        If nbytes is specified then read at most that many bytes. 
        Otherwise, read at most len(buf) bytes. 
        Just as read(), this method follows “no short reads” policy. 
        Return value: number of bytes read and stored into buf.
        """
        ...
        
    def readline() -> None:
        """
        接收一行数据，遇换行符结束，并返回接收数据的对象 。
        """
        ...

    def write(buf) -> None:
        """
        将字节类型数据写入套接字，并返回写入成功的数据大小。
        """
        ...

