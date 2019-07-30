## **math** – 数学函数

**math** 模块提供了对 C 标准定义的数学函数的访问。

> 注意：本模块需要带有硬件 FPU，精度是32位，这个模块需要浮点功能支持。

### 常数

#### **math.e**  
自然对数的底数。

示例：
```
>>>import math
>>>print(math.e)
2.718282
```
#### **math.pi**  
圆周长与直径的比值。

示例：

```
>>> print(math.pi)
3.141593
```

### 函数

#### **math.acos(x)**
传入弧度值，计算cos(x)的反三角函数。 

#### **math.acosh(x)** 
  返回 ``x`` 的逆双曲余弦。

#### **math.asin(x)**
传入弧度值，计算sin(x)的反三角函数。 
示例：

```
>>> x = math.asin(0.5)
>>> print(x)
0.5235988
```

#### **math.asinh(x)**
  返回``x`` 的逆双曲正弦。

#### **math.atan(x)**
  返回 ``x`` 的逆切线。

#### **math.atan2(y, x)**
  Return the principal value of the inverse tangent of y/x.

#### **math.atanh(x)**
  Return the inverse hyperbolic tangent of x.

#### **math.ceil(x)**
向上取整。 
示例：

```
>>> x = math.ceil(5.6454)
>>> print(x)
6
```

#### **math.copysign(x, y)** 
  Return x with the sign of y.

#### **math.cos(x)**  
传入弧度值，计算余弦。 
示例：计算cos60°

```
>>> math.cos(math.radians(60))
0.5
```

#### **math.cosh(x)**  
  Return the hyperbolic cosine of x.

#### **math.degrees(x)**  
弧度转化为角度。 
示例：

```
>>> x = math.degrees(1.047198)
>>> print(x)
60.00002
```

#### **math.erf(x)**  
  Return the error function of x.

#### **math.erfc(x)**  
  Return the complementary error function of x.

#### **math.exp(x)**  
计算e的x次方（幂）。 
示例：

```
>>> x = math.exp(2)
>>> print(x)
7.389056
```

#### **math.expm1(x)**  
计算 math.exp(x) - 1。 

#### **math.fabs(x)**  
计算绝对值。 
示例：

```
>>> x = math.fabs(-5)
>>> print(x)
5.0
>>> y = math.fabs(5.0)
>>> print(y)
5.0
```

#### **math.floor(x)**  
向下取整。 
示例：

```
>>> x = math.floor(2.99)
>>> print(x)
2
>>> y = math.floor(-2.34)
>>> print(y)
-3
```

#### **math.fmod(x, y)**  
取x除以y的模。 
示例：

```
>>> x = math.fmod(4, 5)
>>> print(x)
4.0
```

#### **math.frexp(x)**  
  Decomposes a floating-point number into its mantissa and exponent. The returned value is the tuple (m, e) such that x == m * 2**e exactly. If x == 0 then the function returns (0.0, 0), otherwise the relation 0.5 <= abs(m) < 1 holds.

#### **math.gamma(x)**  
返回伽马函数。 
示例：

```
>>> x = math.gamma(5.21)
>>> print(x)
33.08715
```

#### **math.isfinite(x)**  
  Return True if x is finite.

#### **math.isinf(x)**  
  Return True if x is infinite.

#### **math.isnan(x)**  
  Return True if x is not-a-number

#### **math.ldexp(x, exp)**  
  Return x * (2**exp).

#### **math.lgamma(x)**  
返回伽马函数的自然对数。 
示例：

```
>>> x = math.lgamma(5.21)
>>> print(x)
3.499145
```

#### **math.log(x)**  
计算以e为底的x的对数。 
示例：

```
>>> x = math.log(10)
>>> print(x)
2.302585
```

#### **math.log10(x)**  
计算以10为底的x的对数。 
示例：

```
>>> x = math.log10(10)
>>> print(x)
1.0
```

#### **math.log2(x)**  
 计算以2为底的x的对数。 
示例：

```
>>> x = math.log2(8)
>>> print(x)
3.0
```

#### **math.modf(x)**  
  Return a tuple of two floats, being the fractional and integral parts of x. Both return values have the same sign as x.

#### **math.pow(x, y)**  
计算 x 的 y 次方（幂）。 
示例：

```
>>> x = math.pow(2, 3)
>>> print(x)
8.0
```

#### **math.radians(x)**  
角度转化为弧度。 
示例：

```
>>> x = math.radians(60)
>>> print(x)
1.047198
```

#### **math.sin(x)**  
传入弧度值，计算正弦。 
示例：计算sin90°

```
>>> math.sin(math.radians(90))
1.0
```

#### **math.sinh(x)**  
  Return the hyperbolic sine of x.

#### **math.sqrt(x)**  
计算平方根。 
示例：

```
>>> x = math.sqrt(9)
>>> print(x)
3.0
```

#### **math.tan(x)**  
传入弧度值，计算正切。 
示例：计算tan60°

```
>>> math.tan(math.radians(60))
1.732051
```

#### **math.tanh(x)**  
  Return the hyperbolic tangent of x.

#### **math.trunc(x)**  
取整。 
示例：

```
>>> x = math.trunc(5.12)
>>> print(x)
5
>>> y = math.trunc(-6.8)
>>> print(y)
-6
```

更多内容可参考  [math](http://docs.micropython.org/en/latest/pyboard/library/math.html) 。
