"""
cmath 模块提供了对复数的数学函数的访问。这个模块中的函数接受整数、浮点数或复数作为参数。他们还将接受任何有复数或浮点方法的 Python 对象：这些方法分别用于将对象转换成复数或浮点数，然后将该函数应用到转换的结果中。
"""

e = ...  # type: int
pi = ...  # type: int

def cos(z) -> None:
    """返回z的余弦。"""
    ...

def exp(z) -> None:
    """返回z的指数。"""
    ...

def log(z) -> None:
    """返回z的对数。"""
    ...

def log10(z) -> None:
    """返回z的常用对数。"""
    ...

def phase(z) -> None:
    """返回z的相位, 范围是(-pi, +pi]，以弧度表示。"""
    ...

def polar(z) -> None:
    """返回z的极坐标。"""
    ...

def rect(r, phi) -> None:
    """返回模量r和相位phi的复数。"""
    ...

def sin(z) -> None:
    """返回z的正弦。"""
    ...

def sqrt(z) -> None:
    """返回z的平方根。"""
    ...

