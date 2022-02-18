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

```c++
function myFunction() {
    var x = 1;
    if (true) {
        var x = 2; // 同样的变量
        console.log(x); // 2
    }
    console.log(x); // 2，这里即使出了 if 的范围，x 依然等于 2
}

function myFunction() {
    let x = 1;
    if (true) {
        let x = 2; // 不同的变量
        console.log(x); // 2
    }
    console.log(x); // 1，和上面相比，这里的 x 还是 if 上面声明的 x
}
```

&emsp;JavaScript 声明变量的时候，虽然用 `var` 关键字声明和不用关键字声明，很多时候运行并没有问题，但是这两种方式还是有区别的。可以正常运行的代码并不代表是合适的代码。

```c++
// num1 为全局变量，num2 为 window 的一个属性
var num1 = 1;
num2 = 2;

// delete num1; 无法删除
// delete num2; 删除
function model() {
var num1 = 1; // 本地变量
num2 = 2; // window 的属性

// 匿名函数
(function(){
    var num = 1; // 本地变量
    num1 = 2; // 继承作用域（闭包）
    num3 = 3; // window 的属性
}())
}
```

&emsp;`const` 关键字用来声明 JavaScript 中的常量（与变量相对，不可修改，但同样是用于存储信息的 "容器"。），常量的值不能通过重新赋值来改变，并且不能重新声明。

```c++
// 定义常量 a 并赋值为 0
const a = 0;

// 报错（不能重新赋值）
a = 1;

// 报错 (不能重新声明)
const a = 2;

// 输出 0
console.log("a is: " + a);
```

&emsp;JavaScript 允许重复声明变量，后声明的覆盖之前的。（注意：这是和 OC、Swift、C、C++ 等语言相比的一个很明显的区别。）

```c++
var a = 1;
var a = "x";
console.log(a); // 输出 x, 1 已经被覆盖了
```

&emsp;JavaScript 除了同名变量会覆盖，同样的函数也会覆盖。

&emsp;JavaScript 允许重复定义函数，JavaScript 没有重载这个概念，它依据函数名来区分函数。后定义的同名函数覆盖之前的，与参数无关。

```c++
function test() {
    console.log("test");
}
test(); // 输出：test arg number: 0 arg1: undefined

function test(arg1) {
    console.log("test arg number: " + arguments.length + " arg1: " + arg1);
}
test(1, 2); // 输出：test arg number: 2 arg1: 1
```

&emsp;实参个数如果比形参少，那么剩下的默认赋值为 `undefined`，如果实参传的比形参数量多，那么是全部都会被传进去的，只不过没有对应的形参可以引用（但可以用 `arguments` 来获取剩下的参数）。

```c++
function test(arg1) {
    for(var i = 0; i < arguments.length; i++) {
        console.log(arguments[i]);
    }
}
test(1, 2);
// 输出：
// 1
// 2
```
