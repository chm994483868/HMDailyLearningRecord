## JavaScript Window 浏览器对象模型

&emsp;浏览器对象模型（Browser Object Model(BOM)）使 JavaScript 有能力与浏览 "对话"。

## window 对象

&emsp;所有浏览器都支持 `window` 对象。它表示浏览器窗口。

&emsp;所有 JavaScript 全局对象、函数以及变量均自动成为 `window` 对象的成员。

&emsp;全局变量是 `window` 对象的属性。全局函数是 `window` 对象的方法。甚至 HTML DOM 的 `document` 也是 `window` 对象的属性之一。

```c++
window.document.getElementById("header");

// 与此相同：

document.getElementById("header");
```

### window 尺寸

&emsp;对于 IE、Chrome、Firefox、Opera、Safari：

+ `window.innerHeight` - 浏览器窗口的内部高度（包括滚动条）。
+ `window.innerWidth` - 浏览器窗口的内部宽度（包括滚动条）。

&emsp;另外还有：

+ `document.documentElement.clientHeight`
+ `document.documentElement.clientWidth`

&emsp;或者：

+ `document.body.clientHeight`
+ `document.body.clientWidth`

&emsp;可以使用 `||` 运算符覆盖所有的情况：

```c++
var w = window.innerWidth || document.documentElement.clientWidth || document.body.clientWidth;
var h = window.innerHeight || document.documentElement.clientHeight || document.body.clientHeight;
```

&emsp;`window` 的其它一些方法：

+ `window.open()` - 打开新窗口
+ `window.close()` - 关闭当前窗口
+ `window.moveTo()` - 移动当前窗口
+ `window.resizeTo()` - 调整当前窗口的尺寸

&emsp;定义全局变量与在 `window` 对象上直接定义属性差别：

1. 全局变量不能通过 `delete` 操作符删除，而 `window` 属性上定义的变量可以通过 `delete` 删除。

```c++
var num = 123;
window.str = "string";
delete num;
delete str;

console.log(num); // 123
console.log(str); // Uncaught ReferenceError: str is not defined
```

&emsp;全局变量不能通过 `delete` 删除，因为通过 `var` 定义全局变量会有一个名为 `[Configurable]` 的属性，默认值为 `false`，所以这样定义的属性不可以通过 `delete` 操作符删除。

2. 访问未声明的变量会抛出错误，但是通过查询 `window` 对象，可以知道某个可能未声明的变量是否存在。

```c++
var newValue = oldValue; // 报错停止执行：Uncaught ReferenceError: oldValue is not defined
var newValue = window.oldValue; // 不会报错，正常执行，下面打印 undefined
console.log(newValue); // undefined
```

3. 有些自执行函数里面的变量，想要外部也访问到的话，在 `window` 对象上直接定义属性。

## JavaScript Window Screen

&emsp;
