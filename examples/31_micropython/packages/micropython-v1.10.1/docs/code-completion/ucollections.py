"""
ucollections 模块实现了专门的容器数据类型，它提供了 Python 的通用内置容器的替代方案，包括了字典、列表、集合和元组。
"""

class namedtuple(name, fields):
    """
    这是工厂函数创建一个新的 namedtuple 型与一个特定的字段名称和集合。namedtuple 是元组允许子类要访问它的字段不仅是数字索引，而且还具有属性使用符号字段名访问语法。 字段是字符串序列指定字段名称。为了兼容的实现也可以用空间分隔的字符串命名的字段（但效率较低） 。
    代码示例：

    - from ucollections import namedtuple
    - MyTuple = namedtuple("MyTuple", ("id", "name"))
    - t1 = MyTuple(1, "foo")
    - t2 = MyTuple(2, "bar")
    - print(t1.name)
    - assert t2.name == t2[1]
    - ucollections.OrderedDict(...)
    """
    ...

class OrderedDict(...):
    """
    字典类型的子类，会记住并保留键/值的追加顺序。当有序的字典被迭代输出时，键/值 会按照他们被添加的顺序返回 :
    from ucollections import OrderedDict

    # To make benefit of ordered keys, OrderedDict should be initialized
    # from sequence of (key, value) pairs.
    - d = OrderedDict([("z", 1), ("a", 2)])
    # More items can be added as usual
    - d["w"] = 5
    - d["b"] = 3
    - for k, v in d.items():
    -     print(k, v)
    输出:

    - z 1 a 2 w 5 b 3
    """
    ...
