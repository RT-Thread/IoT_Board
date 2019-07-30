## machine.LCD

**machine.LCD** 类是 machine 模块下面的一个硬件类，用于对 LCD 的配置和控制，提供对 LCD 设备的操作方法。

IoT board 板载一块 1.3 寸，分辨率为 `240*240` 的 LCD 显示屏，因此对该屏幕操作时，(x, y)  坐标的范围是 `0 - 239`。

### 构造函数

在 RT-Thread MicroPython 中 `LCD` 对象的构造函数如下：

#### **class machine.LCD**()
在给定总线上构造一个 `LCD` 对象，无入参，初始化的对象取决于特定硬件，初始化方式可参考 [示例](#_3)。

### 方法

#### **LCD.light**(value)

控制是否开启 LCD 背光，入参为 True 则打开 LCD 背光，入参为 False 则关闭 LCD 背光。

#### **LCD.fill**(color)

根据给定的颜色填充整个屏幕，支持多种颜色，可以传入的参数有：

```
WHITE BLACK BLUE BRED GRED GBLUE RED MAGENTA GREEN CYAN YELLOW BROWN BRRED GRAY GRAY175 GRAY151 GRAY240
```

详细的使用方法可参考[示例](#_3)。

#### **LCD.pixel**(x, y, color)

向指定的位置（x, y）画点，点的颜色为 color 指定的颜色，可指定的颜色和上一个功能相同。

注意：(x, y)  坐标的范围是 0 - 239，使用下面的方法对坐标进行操作时同样需要遵循此限制。

#### **LCD.text**(str, x, y, size)

在指定的位置（x,y）写入字符串，字符串由 str 指定，字体的大小由 size 指定，size 的大小可为 16，24，32。

#### **LCD.line**(x1, y1, x2, y2)

在 LCD 上画一条直线，起始地址为 （x1, y1），终点为（x2, y2）。

#### **LCD.rectangle**(x1, y1, x2, y2)

在 LCD 上画一个矩形，左上角的位置为（x1, y1），右下角的位置为（x2, y2）。

#### **LCD.circle**(x1, y1, r)

在 LCD 上画一个圆形，圆心的位置为（x1, y1），半径长度为 r。

### 示例

```python
from machine import LCD     # 从 machine 导入 LCD 类
lcd = LCD()                 # 创建一个 lcd 对象
lcd.light(False)            # 关闭背光
lcd.light(True)             # 打开背光
lcd.fill(lcd.BLACK)         # 将整个 LCD 填充为黑色
lcd.fill(lcd.RED)           # 将整个 LCD 填充为红色
lcd.fill(lcd.GRAY)          # 将整个 LCD 填充为灰色
lcd.fill(lcd.WHITE)         # 将整个 LCD 填充为白色
lcd.pixel(50, 50, lcd.BLUE) # 将（50,50）位置的像素填充为蓝色
lcd.text("hello RT-Thread", 0, 0, 16)   # 在（0, 0） 位置以 16 字号打印字符串
lcd.text("hello RT-Thread", 0, 16, 24)  # 在（0, 16）位置以 24 字号打印字符串
lcd.text("hello RT-Thread", 0, 48, 32)  # 在（0, 48）位置以 32 字号打印字符串
lcd.line(0, 50, 239, 50)    # 以起点（0，50），终点（239，50）画一条线
lcd.line(0, 50, 239, 50)    # 以起点（0，50），终点（239，50）画一条线
lcd.rectangle(100, 100, 200, 200) # 以左上角为（100,100），右下角（200,200）画矩形
lcd.circle(150, 150, 80)    # 以圆心位置（150,150），半径为 80 画圆
```
