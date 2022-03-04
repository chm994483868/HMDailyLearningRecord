# JSON 学习笔记

> &emsp;JSON 是用于存储和传输数据的格式。JSON 通常用于服务端和客户端之间传递数据，JSON 是一种轻量级的数据交换格式。

## 什么是 JSON？

&emsp;JSON：英文全称 JavaScript Object Notation(JavaScript 对象表示法)。JSON 是存储和交换文本信息的语法，类似 XML。JSON 比 XML 更小、更快，更易解析。

+ JSON 指的是 JavaScript 对象表示法（JavaScript Object Notation）。
+ JSON 独立于语言：JSON 使用 Javascript 语法来描述数据对象，但是 JSON 仍然独立于语言和平台。JSON 解析器和 JSON 库支持许多不同的编程语言。目前非常多的动态（PHP、JSP、.NET）编程语言都支持 JSON。

> &emsp;JSON 使用 JavaScript 语法，但是 JSON 格式仅仅是一个文本。文本可以被任何编程语言读取及作为数据格式传递。

## JSON - 转换为 JavaScript 对象（JSON 格式化后为 JavaScript 对象）

&emsp;JSON 文本格式在语法上与创建 JavaScript 对象的代码相同。由于这种相似性，无需解析器，JavaScript 程序能够使用内建的 `parse()` 函数，用 JSON 数据来生成原生的 JavaScript 对象。

```c++
var text = '{ "sites" : [' +
'{ "name":"Runoob" , "url":"www.runoob.com" },' +
'{ "name":"Google" , "url":"www.google.com" },' +
'{ "name":"Taobao" , "url":"www.taobao.com" } ]}';

obj = JSON.parse(text);
document.getElementById("demo").innerHTML = obj.sites[1].name + " " + obj.sites[1].url;
```
&emsp;`JSON.parse()` 用于将一个 JSON 字符串转换为 JavaScript 对象。
&emsp;`JSON.stringify()` 用于将 JavaScript 值转换为 JSON 字符串。
