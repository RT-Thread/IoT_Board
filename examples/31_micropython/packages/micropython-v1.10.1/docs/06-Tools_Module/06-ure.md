## **ure** – 正则表达式

`ure` 模块用于测试字符串的某个模式，执行正则表达式操作。

### 匹配字符集


#### 匹配任意字符
  ``'.'``

#### 匹配字符集合，支持单个字符和一个范围
  ``'[]'``

#### 支持多种匹配元字符
  ``'^'``
  ``'$'``
  ``'?'``
  ``'*'``
  ``'+'``
  ``'??'``
  ``'*?'``
  ``'+?'``
  ``'{m,n}'``

### 函数

#### **ure.compile**(regex)  
编译正则表达式，返回 regex 对象。

#### **ure.match**(regex, string)  
用 string 匹配 regex，匹配总是从字符串的开始匹配。

#### **ure.search**(regex, string)  
在 string 中搜索 regex。不同于匹配，它搜索第一个匹配位置的正则表达式字符串 (结果可能会是0)。

#### **ure.DEBUG**  
标志值，显示表达式的调试信息。

### **正则表达式对象**:
编译正则表达式，使用 `ure.compile()` 创建实例。

#### **regex.match**(string)  
#### **regex.search**(string)  
#### **regex.split**(string, max_split=-1)  

### **匹配对象** :
匹配对象是 match() 和 search() 方法的返回值。

#### **match.group**([index])  
只支持数字组。

更多内容可参考 [ure](http://docs.micropython.org/en/latest/pyboard/library/ure.html)  。
