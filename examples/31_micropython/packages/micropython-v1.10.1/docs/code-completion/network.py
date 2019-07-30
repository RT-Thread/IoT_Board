"""
此模块提供网络驱动程序和路由配置。特定硬件的网络驱动程序在此模块中可用，用于配置硬件网络接口。然后，配置接口提供的网络服务可以通过 usocket 模块使用。
"""

STA_IF = ...  # type: int
AP_IF = ...  # type: int

class WLAN(object):
    """
    创建一个 WLAN 网络接口对象。
    支持的接口是 network.STA_IF（STA 模式，可以连接到上游的 WiFi 热点上） 和 network.AP_IF（AP 模式，允许其他 WiFi 客户端连接到自身的热点)。
    下面方法的可用性取决于接口的类型。
    例如，只有STA 接口可以使用 WLAN.connect() 方法连接到 AP 热点上。
    """

    STAT_IDLE = ...  # type: int
    STAT_CONNECTING = ...  # type: int
    STAT_WRONG_PASSWORD = ...  # type: int
    STAT_NO_AP_FOUND = ...  # type: int
    STAT_CONNECT_FAIL = ...  # type: int
    STAT_GOT_IP = ...  # type: int

    def __init__(self, interface_id: int) -> None:
    """
    """
    ...

    def active(is_active : int) -> None:
        """
        - active([is_active])
        如果向该方法传入布尔数值，传入 True 则使能卡，传入 False 则禁止网卡。否则，如果不传入参数，则查询当前网卡的状态。
        """
        ...

    def connect(ssid, password) -> None:
        """使用指定的账号和密码链接指定的无线热点。"""
        ...

    def disconnect() -> None:
        """从当前链接的无线网络中断开。"""
        ...

    def scan() -> None:
        """
        扫描当前可以连接的无线网络。
        只能在 STA 模式下进行扫描，使用元组列表的形式返回 WiFi 接入点的相关信息。
        （ssid, bssid, channel, rssi, authmode, hidden）
        """
        ...

    def status(param) -> None:
        """
        - status([param])
        返回当前无线连接的状态。
        当调用该方法时没有附带参数，就会返回值描述当前网络连接的状态。
        如果还没有从热点连接中获得 IP 地址，此时的状态为 STATION_IDLE。
        如果已经从连接的无线网络中获得 IP 地址，此时的状态为 STAT_GOT_IP。
        当调用该函数使用的参数为 rssi 时，则返回 rssi 的值，该函数目前只支持这一个参数。
        """
        ...

    def isconnected() -> None:
        """
        在 STA 模式时，如果已经连接到 WiFi 网络，并且获得了 IP 地址，则返回 True。
        如果处在 AP 模式，此时已经与客户端建立连接，则返回 True。其他情况下都返回 False。"""
        ...

    def ifconfig(config: tuple) -> None:
        """
        ifconfig([(ip, subnet, gateway, dns)])
        获取或者设置网络接口的参数，IP 地址，子网掩码，网关，DNS 服务器。
        当调用该方法不附带参数时，该方法会返回一个包含四个元素的元组来描述上面的信息。
        想要设置上面的值，传入一个包含上述四个元素的元组，例如：
        - nic.ifconfig(('192.168.0.4', '255.255.255.0', '192.168.0.1', '8.8.8.8'))
        """
        ...

    def config(param) -> None:
        """
        - config(param=value, ...)
        获取或者设置一般网络接口参数，这些方法允许处理标准的 ip 配置之外的其他参数，如 WLAN.ifconfig() 函数处理的参数。
        这些参数包括特定网络和特定硬件的参数。
        对于参数的设置，应该使用关键字的语法，可以一次性设置多个参数。"""
        ...
