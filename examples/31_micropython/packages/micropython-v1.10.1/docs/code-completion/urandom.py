"""
urandom 模块实现了伪随机数生成器。
"""

def choice(obj) -> None:
    """
    随机生成对象 obj 中的元数。

    - obj：元数列表

    示例：

    - print(random.choice("DFRobot"))
    - R
    - print(random.choice("DFRobot"))
    - D
    - print(random.choice([0, 2, 4, 3]))
    - 3
    - print(random.choice([0, 2, 4, 3]))
    - 3
    - print(random.choice([0, 2, 4, 3]))
    - 2
    """
    ...

def getrandbits(size) -> None:
    """
    随机生成 0 到 size 个位二进制数范围内的正整数。 比如 ：

    - size = 4，那么便是从 0 到0b1111中随机一个正整数。
    - size = 8，那么便是从 0 到 0b11111111中随机一个正整数。
    - size：位大小

    示例：

    - print( random.getrandbits(1))  #1位二进制位，范围为0~1（十进制：0~1）
    - 1
    - print(random.getrandbits(1))
    - 0
    - print(random.getrandbits(8))  #8位二进制位，范围为0000 0000~1111 11111（十进制：0~255）
    - 224
    - print(random.getrandbits(8))
    - 155
    """
    ...

def randint(start, end) -> None:
    """
    随机生成一个 start 到 end 之间的整数。

    start：指定范围内的开始值，包含在范围内
    end：指定范围内的结束值，包含在范围内
    示例：

    - import random
    - print(random.randint(1, 4))
    - 4
    - print(random.randint(1, 4))
    - 2
    """
    ...

def random() -> None:
    """
    随机生成一个 0 到 1 之间的浮点数。 示例：

    - print(random.random())
    - 0.7111824
    - print(random.random())
    - 0.3168149
    """
    ...


def randrange(start, end, step) -> None:
    """
    随机生成 start 到 end 并且递增为 step 的范围内的正整数。例如，randrange(0, 8, 2)中，随机生成的数为 0、2、4、6 中任一个。

    - start：指定范围内的开始值，包含在范围内
    - end：指定范围内的结束值，包含在范围内
    - step：递增基数

    示例：

    - print(random.randrange(2, 8, 2))
    - 4
    - print(random.randrange(2, 8, 2))
    - 6
    - print(random.randrange(2, 8, 2))
    - 2
    """
    ...

def seed(sed) -> None:
    """
    指定随机数种子，通常和其他随机数生成函数搭配使用。 
    注意： MicroPython 中的随机数其实是一个稳定算法得出的稳定结果序列，而不是一个随机序列。
    sed 就是这个算法开始计算的第一个值。
    所以就会出现只要 sed 是一样的，那么后续所有“随机”结果和顺序也都完全一致。
    """
    ...

def uniform(start, end) -> None:
    """
    随机生成start到end之间的浮点数。

    - start：指定范围内的开始值，包含在范围内
    - stop：指定范围内的结束值，包含在范围内

    示例：

    - print(random.uniform(2, 4))
    - 2.021441
    - print(random.uniform(2, 4))
    - 3.998012
    """
    ...
