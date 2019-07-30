## **cmath** – 复数的数学函数

`cmath` 模块提供了对复数的数学函数的访问。这个模块中的函数接受整数、浮点数或复数作为参数。他们还将接受任何有复数或浮点方法的 Python 对象：这些方法分别用于将对象转换成复数或浮点数，然后将该函数应用到转换的结果中。

### 函数

#### **cmath.cos**(z)  
返回``z``的余弦。

#### **cmath.exp**(z)  
返回``z``的指数。

#### **cmath.log**(z)  
返回``z``的对数。

#### **cmath.log10**(z)  
返回``z``的常用对数。

#### **cmath.phase**(z)  
返回``z``的相位, 范围是(-pi, +pi]，以弧度表示。

#### **cmath.polar**(z)  
返回``z``的极坐标。

#### **cmath.rect**(r, phi)  
返回`模量r`和相位``phi``的复数。

#### **cmath.sin**(z)  
返回``z``的正弦。

#### **cmath.sqrt**(z)  
返回``z``的平方根。

### 常数

#### **cmath.e**  
自然对数的指数。

#### **cmath.pi**  
圆周率。

更多内容可参考 [cmath](http://docs.micropython.org/en/latest/pyboard/library/cmath.html)  。
