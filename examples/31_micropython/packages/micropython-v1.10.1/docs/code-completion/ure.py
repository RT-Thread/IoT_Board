"""
ure 模块用于测试字符串的某个模式，执行正则表达式操作。
"""

DEBUG = ...  # type: int

class compile(...):
    """
    - compile(regex_str[, flags])
    编译正则表达式，返回 regex 对象。
    """
    ...

    def __init__(self) -> None:
    ...

    def match(string) -> None:
        """用 string 匹配 regex，匹配总是从字符串的开始匹配。"""
        ...

    def search(string) -> None:
        """在 string 中搜索 regex。不同于匹配，它搜索第一个匹配位置的正则表达式字符串 (结果可能会是0)。"""
        ...

    def sub(replace, string, count, flags) -> None:
        """Compile regex_str and search for it in string, replacing all matches with replace, and returning the new string."""
        ...

    def split() -> None:
        """获取缓存区内容。"""
        ...

class match(...):
    """
    - Match objects as returned by match() and search() methods。
    """
    ...

    def __init__(self) -> None:
    ...

    def group(index) -> None:
        """用 string 匹配 regex，匹配总是从字符串的开始匹配。"""
        ...

    def groups() -> None:
        """在 string 中搜索 regex。不同于匹配，它搜索第一个匹配位置的正则表达式字符串 (结果可能会是0)。"""
        ...

    def start(index) -> None:
        """start([index])"""
        ...

    def end(index) -> None:
        """end([index])
        Return the index in the original string of the start or end of the substring group that was matched. index defaults to the entire group, otherwise it will select a group.
        """
        ...

    def span() -> None:
        """Returns the 2-tuple (match.start(index), match.end(index))."""
        ...

class search(...):
    """
    - Match objects as returned by match() and search() methods。
    """
    ...

    def __init__(self) -> None:
    ...

    def group(index) -> None:
        """用 string 匹配 regex，匹配总是从字符串的开始匹配。"""
        ...

    def groups() -> None:
        """在 string 中搜索 regex。不同于匹配，它搜索第一个匹配位置的正则表达式字符串 (结果可能会是0)。"""
        ...

    def start(index) -> None:
        """start([index])"""
        ...

    def end(index) -> None:
        """end([index])
        Return the index in the original string of the start or end of the substring group that was matched. index defaults to the entire group, otherwise it will select a group.
        """
        ...

    def span() -> None:
        """Returns the 2-tuple (match.start(index), match.end(index))."""
        ...

def match(regex, string) -> None:
    """用 string 匹配 regex，匹配总是从字符串的开始匹配。"""
    ...

def search(regex, string) -> None:
    """在 string 中搜索 regex。不同于匹配，它搜索第一个匹配位置的正则表达式字符串 (结果可能会是0)。"""
    ...

def sub(regex_str, replace, string, count, flags) -> None:
    """Compile regex_str and search for it in string, replacing all matches with replace, and returning the new string."""
    ...

