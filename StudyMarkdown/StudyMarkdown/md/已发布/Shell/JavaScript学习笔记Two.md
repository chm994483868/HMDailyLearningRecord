# JavaScript 学习笔记

&emsp;JavaScript 是 web 开发人员必须学习的 3 门语言中的一门：

1. HTML 定义了网页的内容。
2. CSS 描述了网页的布局。
3. JavaScript 控制了网页的行为。

&emsp;JavaScript 是一种轻量级的编程语言。JavaScript 是可插入 HTML 页面的编程代码。JavaScript 插入 HTML 页面后，可由所有的现代浏览器执行。

&emsp;你会经常看到 document.getElementById("some id")。这个方法是 HTML DOM 中定义的。

&emsp;DOM(Document Object Model)（文档对象模型）是用于访问 HTML 元素的正式 W3C 标准。

&emsp;JavaScript 与 Java 是两种完全不同的语言，无论在概念上还是设计上。Java（由 Sun 发明）是更复杂的编程语言。ECMA-262 是 JavaScript 标准的官方名称。JavaScript 是由 Brendan Eich 发明。它于 1995 年出现在 Netscape 中（该浏览器已停止更新），并于 1997 年被 ECMA（一个标准协会）采纳。

&emsp;`document.write` 是直接写入到页面的内容流，如果在写之前没有调用 `document.open`，浏览器会自动调用 `open`。每次写完关闭之后重新调用该函数，会导致页面被重写。

&emsp;`innerHTML` 则是 DOM 页面元素的一个属性，代表该元素的 html 内容。你可以精确到某一个具体的元素来进行更改。如果想修改 document 的内容，则需要修改 `document.documentElement.innerElement`。

&emsp;`innerHTML` 很多情况下都优于 `document.write`，其原因在于其允许更精确的控制要刷新页面的那一个部分。

&emsp;DOM：你会经常看到 `document.getElementById("id")`，这个方法是 HTML DOM 中定义的。DOM (Document Object Model)（文档对象模型）是用于访问 HTML 元素的正式 W3C 标准。

&emsp;`window.alert(5+6)` 与 `window.alert("5+6")` 输出的值是不一样的。window.alert(5+6) 会输出 11，而window.alert("5+6") 会输出 5+6。这是因为当用引号时会认为引号中是字符串，从而直接将引号中的内容打印出来。

&emsp;JavaScript 是一个程序语言，语法规则定义了语言结构。JavaScript 是一个脚本语言。它是一个轻量级，但功能强大的编程语言。

&emsp;JavaScript 对大小写是敏感的。

&emsp;JavaScript 是弱类型编程语言。定义变量都使用 var 定义，与 Java 这种强类型语言有区别。在定义后可以通过 `typeOf()` 来获取 JavaScript 中变量的数据类型。

&emsp;有个特殊情况，`typeOf` 不能用来判断 Array 还是 Object。`var arr = []; typeOf(arr) === 'object' // true` 结果为 true。

&emsp;JavaScript 是脚本语言，浏览器会在读取代码时，逐行地执行脚本代码。而对于传统编程来说，会在执行前对所有代码进行编译。

&emsp;`let` 声明的变量只在其声明的块或子块中可用，这一点，与 `var` 相似。二者之间最主要的区别在于 `var` 声明的变量的作用域是整个封闭函数。
