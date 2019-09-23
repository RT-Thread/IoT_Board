## machine.ADC

**machine.ADC** 类是 machine 模块下的一个硬件类，用于指定 ADC 设备的配置和控制，提供对 ADC 设备的操作方法。

- ADC（Analog-to-Digital Converter，模数转换器），用于将连续变化的模拟信号转化为离散的数字信号。
- ADC 设备两个重要参数：采样值、分辨率；
  - 采样值：当前时间由模拟信号转化的数值信号的数值；
  - 分辨率：以二进制（或十进制）数的位数来表示，一般有 8 位、10 位、12 位、16 位等，它说明模数转换器对输入信号的分辨能力，位数越多，表示分辨率越高，采样值会更精确。 

### 构造函数

在 RT-Thread MicroPython 中 `ADC` 对象的构造函数如下：

#### **class machine.ADC**(id, channel)

- **id**：使用的 ADC 设备编号，`id = 1` 表示编号为 1 的 ADC 设备，或者表示使用的 ADC 设备名，如 `id = "adc"` 表示设备名为 `adc` 的 ADC 设备；
- **channel**：使用的 ADC 设备通道号，每个 ADC 设备对应多个通道；

例如：`ADC(1,4)` 表示当前使用编号为 1 的 ADC 设备的 4 通道。

### 方法

#### **ADC.init**(channel)

根据输入的层参数初始化 ADC 对象，入参为使用的 ADC 对象通道号；

#### **ADC.deinit**()

用于关闭 ADC 对象，ADC 对象 deinit 之后需要重新 init 才能使用。

#### **ADC.read**()

用于获取并返回当前 ADC 对象的采样值。例如当前采样值为 2048，对应设备的分辨率为 12位，当前设备参考电压为 3.3V ，则该 ADC 对象通道上实际电压值的计算公式为：**采样值 * 参考电压  /  （1 <<  分辨率位数）**，即 `vol = 2048 / 4096 * 3.3 V = 1.15V`。

### 示例

``` python
>>> from machine import ADC      # 从 machine 导入 ADC 类
>>> adc = ADC(1, 13)             # 创建 ADC 对象，当前使用编号为 1 的 ADC 设备的 13 通道
>>> adc.read()                   # 获取 ADC 对象采样值
4095
>>> adc.deinit()                 # 关闭 ADC 对象
>>> adc.init(13)                 # 开启并重新配置 ADC 对象
```
