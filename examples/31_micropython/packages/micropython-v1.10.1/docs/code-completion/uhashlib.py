"""
uhashlib 模块实现了二进制数据哈希算法。
"""

SHA256 = ...  # type: int
SHA1 = ...  # type: int
MD5 = ...  # type: int

class sha256(data):
    """
    - sha256([data])
    创建一个SHA256哈希对象并提供 data 赋值。
    """

    def __init__(self) -> None:
    ...

    def update(data) -> None:
        """将更多二进制数据放入哈希表中。"""
        ...

    def digest() -> None:
        """返回字节对象哈希的所有数据。调用此方法后，将无法将更多数据送入哈希。"""
        ...

    def hexdigest() -> None:
        """此方法没有实现， 使用 ubinascii.hexlify(hash.digest()) 达到类似效果。"""
        ...

class sha1(data):
    """
    - sha1([data])
    创建一个SHA1哈希对象并提供 data 赋值。
    """

    def __init__(self) -> None:
    ...

    def update(data) -> None:
        """将更多二进制数据放入哈希表中。"""
        ...

    def digest() -> None:
        """返回字节对象哈希的所有数据。调用此方法后，将无法将更多数据送入哈希。"""
        ...

    def hexdigest() -> None:
        """此方法没有实现， 使用 ubinascii.hexlify(hash.digest()) 达到类似效果。"""
        ...


class md5(data):
    """
    - md5([data])
    创建一个MD5哈希对象并提供 data 赋值。
    """

    def __init__(self) -> None:
    ...

    def update(data) -> None:
        """将更多二进制数据放入哈希表中。"""
        ...

    def digest() -> None:
        """返回字节对象哈希的所有数据。调用此方法后，将无法将更多数据送入哈希。"""
        ...

    def hexdigest() -> None:
        """此方法没有实现， 使用 ubinascii.hexlify(hash.digest()) 达到类似效果。"""
        ...
