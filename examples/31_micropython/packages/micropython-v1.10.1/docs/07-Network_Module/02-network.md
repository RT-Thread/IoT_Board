## network – 网络配置

此模块提供网络驱动程序和路由配置。特定硬件的网络驱动程序在此模块中可用，用于配置硬件网络接口。然后，配置接口提供的网络服务可以通过 `usocket` 模块使用。

### 专用的网络类配置

下面具体的类实现了抽象网卡的接口，并提供了一种控制各种网络接口的方法。

- [class WLAN – control built-in WiFi interfaces](./03-network-WLAN.md)