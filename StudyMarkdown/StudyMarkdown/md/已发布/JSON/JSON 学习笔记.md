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

&emsp;JSON 与 JavaScript 的关系：可以直白的理解为 JSON 是 JavaScript 对象的字符串表示法。它使用文本表示一个 JavaScript 对象的信息，（JSON）本质是一个字符串。

## JSON 语法

&emsp;JSON 语法是 JavaScript 对象表示语法的子集。

+ 数据在 名称/值（key : value）对中
+ 数据由逗号分隔
+ 大括号 {} 保存对象
+ 中括号 [] 保存数组，数组可以包含多个对象

## JSON 值

&emsp;JSON 值可以是：

+ 数字（整数或浮点数）
+ 字符串（在双引号中）
+ 逻辑值（`true` 或 `false`）
+ 数组（在中括号中）
+ 对象（在大括号中）
+ `null`

## JSON 文件

+ JSON 文件的文件类型是 `.json`
+ JSON 文本的 MIME 类型是 `application/json`

## 为什么 JSON 比 XML 更好？

+ XML 比 JSON 更难解析。JSON 可以直接使用现有的 JavaScript 对象解析。针对 AJAX 应用，JSON 比 XML 数据加载更快，而且更简单：

&emsp;使用 XML：

1. 获取 XML 文档
2. 使用 XML DOM 迭代循环文档
3. 接数据解析出来复制给变量

&emsp;使用 JSON：

1. 获取 JSON 字符串
2. JSON.Parse 解析 JSON 字符串

## 循环 JavaScript 对象

&emsp;可以使用 for-in 来循环对象的属性：

```c++
var myObj = { "name":"runoob", "alexa":10000, "site":null };
for (x in myObj) {
    document.getElementById("demo").innerHTML += x + "<br>";
}
```

&emsp;在 for-in 循环对象的属性时，使用中括号（`[]`）来访问属性的值：

```c++
var myObj = { "name":"runoob", "alexa":10000, "site":null };
for (x in myObj) {
    document.getElementById("demo").innerHTML += myObj[x] + "<br>";
}
```

## JSON 解析数据异常

&emsp;JSON 不能存储 `Date` 对象。如果需要存储 `Date` 对象，需要将其转换为字符串。之后再将字符串转换为 `Date` 对象。

```c++
var text = '{ "name":"Runoob", "initDate":"2013-12-14", "site":"www.runoob.com"}';
var obj = JSON.parse(text);
obj.initDate = new Date(obj.initDate);
 
document.getElementById("demo").innerHTML = obj.name + "创建日期: " + obj.initDate;
```

&emsp;可以启用 `JSON.parse` 的第二个参数 `reviver`，一个转换结果的函数，对象的每个成员调用此函数。

```c++
var text = '{ "name":"Runoob", "initDate":"2013-12-14", "site":"www.runoob.com"}';
var obj = JSON.parse(text, function (key, value) {
    if (key == "initDate") {
        return new Date(value);
    } else {
        return value;
}});
 
document.getElementById("demo").innerHTML = obj.name + "创建日期：" + obj.initDate;
```

&emsp;JSON 不允许包含函数，但可以将函数作为字符串存储，之后再将字符串转换为函数，不建议在 JSON 中使用函数。

```c++
var text = '{ "name":"Runoob", "alexa":"function () {return 10000;}", "site":"www.runoob.com"}';
var obj = JSON.parse(text);
obj.alexa = eval("(" + obj.alexa + ")");
 
document.getElementById("demo").innerHTML = obj.name + " Alexa 排名：" + obj.alexa();
```

&emsp;`eval(string)`：函数可计算某个字符串，并执行其中的的 JavaScript 代码。

```c++
eval("var a=1"); // 声明一个变量 a 并赋值1。
eval("2+3"); // 执行加运算，并返回运算值。
eval("mytest()"); // 执行 `mytest()` 函数。
eval("{b:2}"); // 声明一个对象。
```

&emsp;JSON 不能存储 `Date` 对象。`JSON.stringify()` 会将所有日期转换为字符串。

```c++
var obj = { "name":"Runoob", "initDate":new Date(), "site":"www.runoob.com"};
var myJSON = JSON.stringify(obj);
document.getElementById("demo").innerHTML = myJSON;
```

&emsp;JSON 不允许包含函数，`JSON.stringify()` 会删除 JavaScript 对象的函数，包括 key 和 value。

```c++
var obj = { "name":"Runoob", "alexa":function () {return 10000;}, "site":"www.runoob.com"};
var myJSON = JSON.stringify(obj);
document.getElementById("demo").innerHTML = myJSON; // alexa key 被删除
```

&emsp;可以在执行 `JSON.stringify()` 函数前将函数转换为字符串来避免以上问题的发生：

```c++
var obj = { "name":"Runoob", "alexa":function () {return 10000;}, "site":"www.runoob.com"};
obj.alexa = obj.alexa.toString();

var myJSON = JSON.stringify(obj);
document.getElementById("demo").innerHTML = myJSON;
```

&emsp;由于 JSON 语法是 JavaScript 语法的子集，JavaScript 函数 `eval()` 可用于将 JSON 文本转换为 JavaScript 对象。`eval()` 函数使用的是 JavaScript 编译器，可解析 JSON 文本，然后生成 JavaScript 对象。必须把文本包围在括号中，这样才能避免语法错误：

```c++
var obj = eval ("(" + txt + ")");
```

## JSON 解析器

&emsp;`eval()` 函数可编译并执行任何 JavaScript 代码。这隐藏了一个潜在的安全问题，使用 JSON 解析器（`parse()`）将 JSON 转换为 JavaScript 对象是更安全的做法。JSON 解析器只能识别 JSON 文本，而不会编译脚本。在浏览器中，这提供了原生的 JSON 支持，而且 JSON 解析器的速度更快。较新的浏览器和最新的 ECMAScript(JavaScript) 标准中均包含了原生的对 JSON 的支持。


&emsp;对于服务器返回的 JSON 字符串，如果 jQuery 异步请求没做类型说明，或者以字符串方式接受，那么需要做一次对象化处理，方式不是太麻烦，就是将该字符串放于 `eval()` 中执行一次。这种方式也适合以普通 JavaScipt 方式获取 json 对象，以下举例说明：

```c++
var u = eval('('+user+')');
```

&emsp;为什么要 `eval` 这里要添加 `('('+user+')')` 呢？原因在于：`eval` 本身的问题。由于 JSON 是以 `{}` 的方式来开始以及结束的，在 JavaScript 中，它会被当成一个 **语句块** 来处理，所以必须强制性的将它转换成一种表达式。加上圆括号的目的是迫使 `eval` 函数在处理 JavaScript 代码的时候强制将括号内的表达式（expression）转化为对象，而不是作为语句（statement）来执行。举一个例子，例如对象字面量 `{}`，如若不加外层的括号，那么 `eval` 会将大括号识别为 JavaScript 代码块的开始和结束标记，那么 `{}` 将会被认为是执行了一句空语句。所以下面两个执行结果是不同的：

```c++
alert(eval("{}"); // return undefined
alert(eval("({})"); // return object[Object]
```

## 参考链接
**参考链接:🔗**
+ [JSON.parse()](https://www.runoob.com/json/json-parse.html)
+ [JSON 教程](https://www.runoob.com/json/json-tutorial.html)
+ [JavaScript JSON](https://www.runoob.com/js/js-json.html)
