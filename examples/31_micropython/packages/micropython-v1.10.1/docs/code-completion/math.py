"""
math 模块提供了对 C 标准定义的数学函数的访问。
本模块需要带有硬件 FPU，精度是32位，这个模块需要浮点功能支持。
"""

e = ...  # type: int
pi = ...  # type: int

def acos(x) -> None:
    """传入弧度值，计算cos(x)的反三角函数。"""
    ...

def acosh(x) -> None:
    """返回 x 的逆双曲余弦。"""
    ...

def asin(x) -> None:
    """传入弧度值，计算sin(x)的反三角函数。 示例：
    - x = math.asin(0.5)
    - print(x)
    0.5235988"""
    ...

def asinh(x) -> None:
    """返回 x 的逆双曲正弦。"""
    ...

def atan(x) -> None:
    """返回 x 的逆切线。"""
    ...

def atan2(y, x) -> None:
    """Return the principal value of the inverse tangent of y/x."""
    ...

def atanh(x) -> None:
    """Return the inverse hyperbolic tangent of x."""
    ...

def ceil(x) -> None:
    """向上取整。 示例：
    - x = math.ceil(5.6454)
    - print(x)
    - 6
    """
    ...

def copysign(x, y) -> None:
    """Return x with the sign of y."""
    ...

def cos(x) -> None:
    """传入弧度值，计算余弦。 示例：计算cos60°
    - math.cos(math.radians(60))
    - 0.5
    """
    ...

def cosh(x) -> None:
    """Return the hyperbolic cosine of x."""
    ...

def degrees(x) -> None:
    """弧度转化为角度。 示例：
    - x = math.degrees(1.047198)
    - print(x)
    - 60.00002"""
    ...

def erf(x) -> None:
    """Return the error function of x."""
    ...

def erfc(x) -> None:
    """Return the complementary error function of x."""
    ...

def exp(x) -> None:
    """计算e的x次方（幂）。
    示例：
    - x = math.exp(2)
    - print(x)
    - 7.389056"""
    ...

def expm1(x) -> None:
    """计算 math.exp(x) - 1。"""
    ...

def fabs(x) -> None:
    """计算绝对值。 示例：
    - x = math.fabs(-5)
    - print(x)
    - 5.0
    - y = math.fabs(5.0)
    - print(y)
    - 5.0
    """
    ...

def floor(x) -> None:
    """向下取整。 示例：
    - x = math.floor(2.99)
    - print(x)
    2
    - y = math.floor(-2.34)
    - print(y)
    -3
    """
    ...

def fmod(x, y) -> None:
    """取x除以y的模。 示例：
    - x = math.fmod(4, 5)
    - print(x)
    4.0
    """
    ...

def frexp(x) -> None:
    """Decomposes a floating-point number into its mantissa and exponent. The returned value is the tuple (m, e) such that x == m * 2**e exactly. If x == 0 then the function returns (0.0, 0), otherwise the relation 0.5 <= abs(m) < 1 holds."""
    ...

def gamma(x) -> None:
    """返回伽马函数。 示例：
    - x = math.gamma(5.21)
    - print(x)
    33.08715。
    """
    ...

def isfinite(x) -> None:
    """Return True if x is finite."""
    ...

def isinf(x) -> None:
    """Return True if x is infinite."""
    ...

def isnan(x) -> None:
    """Return True if x is not-a-number"""
    ...

def ldexp(x, exp) -> None:
    """Return x * (2**exp)."""
    ...

def lgamma(x) -> None:
    """返回伽马函数的自然对数。 示例：
    - x = math.lgamma(5.21)
    - print(x)
    3.499145"""
    ...

def log(x) -> None:
    """计算以e为底的x的对数。 示例：
    - x = math.log(10)
    - print(x)
    2.302585"""
    ...

def log10(x) -> None:
    """计算以10为底的x的对数。 示例：
    - x = math.log10(10)
    - print(x)
    1.0"""
    ...

def log2(x) -> None:
    """计算以2为底的x的对数。 示例：
    - x = math.log2(8)
    - print(x)
    3.0"""
    ...

def modf(x) -> None:
    """Return a tuple of two floats, being the fractional and integral parts of x. Both return values have the same sign as x."""
    ...

def pow(x, y) -> None:
    """计算 x 的 y 次方（幂）。 示例：
    - x = math.pow(2, 3)
    - print(x)
    8.0"""
    ...

def radians(x) -> None:
    """角度转化为弧度。 示例：
    - x = math.radians(60)
    - print(x)
    1.047198"""
    ...

def sin(x) -> None:
    """传入弧度值，计算正弦。 示例：计算sin90°
    - math.sin(math.radians(90))
    1.0"""
    ...

def sinh(x) -> None:
    """Return the hyperbolic sine of x."""
    ...

def sqrt(x) -> None:
    """
    计算平方根。 
    示例：
    - x = math.sqrt(9)
    - print(x)
    3.0"""
    ...

def tan(x) -> None:
    """
    传入弧度值，计算正切。 示例：计算tan60°
    - math.tan(math.radians(60))
    1.732051"""
    ...

def tanh(x) -> None:
    """Return the hyperbolic tangent of x."""
    ...

def trunc(x) -> None:
    """
    取整。 
    示例：
    - x = math.trunc(5.12)
    - print(x)
    5
    - y = math.trunc(-6.8)
    - print(y)
    -6"""
    ...
