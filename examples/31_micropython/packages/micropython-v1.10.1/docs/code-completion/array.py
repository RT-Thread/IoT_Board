"""
array 模块定义了一个对象类型，它可以简洁地表示基本值的数组：字符、整数、浮点数。支持代码格式: b, B, h, H, i, I, l, L, q, Q, f, d (最后2个需要支持浮点数)。
"""

class array(object):
    def __init__(self) -> None:
    """
    用给定类型的元素创建数组。数组的初始内容由 iterable 提供，如果没有提供，则创建一个空数组。
    typecode：数组的类型
    iterable：数组初始内容
    示例：

    - import array
    - a = array.array('i', [2, 4, 1, 5])'
    - b = array.array('f')
    - print(a)
    - array('i', [2, 4, 1, 5])
    - print(b)
    - array('f')
    """
    ...

    def append(val) -> None:
        """
        将一个新元素追加到数组的末尾。
        示例：

        - a = array.array('f', [3, 6])
        - print(a)
        - array('f', [3.0, 6.0])
        - a.append(7.0)
        - print(a)
        - array('f', [3.0, 6.0, 7.0])
        """
        ...

    def extend(iterable) -> None:
        """
        将一个新的数组追加到数组的末尾，注意追加的数组和原来数组的数据类型要保持一致。
        示例：

        - a = array.array('i', [1, 2, 3])
        - b = array.array('i', [4, 5])
        - a.extend(b)
        - print(a)
        - array('i', [1, 2, 3, 4, 5])
        """
        ...

