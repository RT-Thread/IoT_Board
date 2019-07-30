"""
ujson 模块提供 Python 对象到 JSON（JavaScript Object Notation） 数据格式的转换。
"""

def dumps(obj) -> None:
    """
    将 dict 类型转换成 str。

    - obj：要转换的对象

    示例：

    - obj = {1:2, 3:4, "a":6}
    - print(type(obj), obj) #原来为dict类型
    - <class 'dict'> {3: 4, 1: 2, 'a': 6}
    - jsObj = json.dumps(obj) #将dict类型转换成str
    - print(type(jsObj), jsObj)
    - <class 'str'> {3: 4, 1: 2, "a": 6}"""
    ...

def loads(str) -> None:
    """
    解析 JSON 字符串并返回对象。如果字符串格式错误将引发 ValueError 异常。 
    
    示例：

    - obj = {1:2, 3:4, "a":6}
    - jsDumps = json.dumps(obj)
    - jsLoads = json.loads(jsDumps)
    - print(type(obj), obj)
    - <class 'dict'> {3: 4, 1: 2, 'a': 6}
    - print(type(jsDumps), jsDumps)
    - <class 'str'> {3: 4, 1: 2, "a": 6}
    - print(type(jsLoads), jsLoads)
    - <class 'dict'> {'a': 6, 1: 2, 3: 4}
    """
    ...

