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

&emsp;`window.screen` 对象包含有关用户屏幕的信息。

&emsp;`window.screen` 对象在编写时可以不使用 `window` 这个前缀，`screen.availWidth` 属性返回访问者屏幕的宽度，以像素计，减去界面特性，比如窗口任务栏，`screen.availHeight` 同理。

+ `screen.availWidth` - 可用的屏幕宽度
+ `screen.availHeight` - 可用的屏幕高度

```c++
console.log(screen.availWidth) // 1920
console.log(window.innerWidth) // 988

console.log(screen.availHeight) // 1055
console.log(window.innerHeight) // 423
```

&emsp;`screen` 当前可能指屏幕硬件，`window` 指当前窗口，但是 `screen` 又是 `window` 的一个属性。

## JavaScript Window Location

&emsp;`window.location` 对象用于获得当前页面的地址 (URL)，并把浏览器重定向到新的页面。

&emsp;同样 `window.location` 对象在编写时可不使用 `window` 这个前缀。

+ `location.hostname` - 返回 web 主机的域名
+ `location.pathname` - 返回当前页面的路径和文件名
+ `location.port` - 返回 web 主机的端口 （80 或 443）
+ `location.protocol` - 返回所使用的 web 协议（http: 或 https:）
+ `location.href` - 返回当前页面的 URL

&emsp;测试本地 HTML 文件打印：

```c++
console.log(location.hostname) // 打印空
console.log(location.pathname) // /Users/hmc/Documents/GitHub/Simple/Front-End/TestHTML.html
console.log(location.port) // 打印空
console.log(location.protocol) // file:
console.log(location.href) // file:///Users/hmc/Documents/GitHub/Simple/Front-End/TestHTML.html
```

&emsp;`location.assign()` 方法加载新的文档（URL）。

```c++
<script>
    function newDoc() {
        window.location.assign("https://www.runoob.com")
    }

    function newDoc2() {
        window.location.replace("https://www.baidu.com")
    }
</script>
<input type="button" value="加载新文档(能返回)" onclick="newDoc()">
<input type="button" value="加载新文档(不能返回)" onclick="newDoc2()">
```

&emsp;`window.location.assign(url)`：加载 url 指定的新的 HTML 文档。就相当于一个链接，跳转到指定的 url，当前页面会转为新页面内容，可以点击后退返回上一个页面。

&emsp;`window.location.replace(url)`：通过加载 url 指定的文档来替换当前文档 ，这个方法是替换当前窗口页面，前后两个页面共用一个窗口，所以是没有后退返回上一页的。
