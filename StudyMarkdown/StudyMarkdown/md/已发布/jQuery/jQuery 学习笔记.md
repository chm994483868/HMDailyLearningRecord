# jQuery 学习笔记

&emsp;jQuery 是一个 JavaScript 库。jQuery 极大地简化了 JavaScript 编程。

## jQuery 简介

&emsp;先来感受一下 jQuery：下面的代码段给页面中的所有 `p` 标签添加一个 `click` 事件，点击后会隐藏该 `p` 标签。

```JavaScript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("p").click(function() {
            $(this).hide();
        });
    });
</script>
```

&emsp;jQuery 库包含以下功能：

+ HTML 元素选取
+ HTML 元素操作
+ HTML 事件函数
+ HTML DOM 遍历和修改
+ CSS 操作
+ JavaScript 特效和动画
+ AJAX 异步请求方式
+ Utilities

> &emsp;除此之外，jQuery 还提供了大量的插件。

&emsp;jQuery 团体知道 JavaScript 在不同浏览器中存在着大量的兼容性问题，目前 jQuery 兼容于所有主流浏览器, 包括 Internet Explorer 6。目前 jQuery 有三个大版本：

+ 1.x：兼容 IE 678，使用最为广泛的，官方只做 BUG 维护，功能不再新增。因此一般项目来说，使用 1.x 版本就可以了，最终版本：1.12.4 (2016 年 5 月 20 日)
+ 2.x：不兼容 IE 678，很少有人使用，官方只做 BUG 维护，功能不再新增。如果不考虑兼容低版本的浏览器可以使用 2.x，最终版本：2.2.4 (2016 年 5 月 20 日)
+ 3.x：不兼容 IE 678，只支持最新的浏览器。除非特殊要求，一般不会使用 3.x 版本的，很多老的 jQuery 插件不支持这个版本。目前该版本是官方主要更新维护的版本。最新版本：3.6.0

&emsp;jQuery 库是一个 JavaScript 文件，可以使用 HTML 的 `<script>` 标签引用它：

```c++
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
```

> &emsp;是否很疑惑为什么我们没有在 `<script>` 标签中使用 `type="text/javascript"`？在 HTML5 中，不必那样做了，JavaScript 是 HTML5 以及所有现代浏览器中的默认脚本语言！

&emsp;如果不希望下载并存放 jQuery，那么也可以通过 CDN（内容分发网络） 引用它。Staticfile CDN、百度、又拍云、新浪、谷歌和微软的服务器都存有 jQuery 。如果你的站点用户是国内的，建议使用百度、又拍云、新浪等国内 CDN 地址，如果你站点用户是国外的可以使用谷歌和微软。

&emsp;使用 Staticfile CDN、百度、又拍云、新浪、谷歌或微软的 jQuery，有一个很大的优势：许多用户在访问其他站点时，已经从百度、又拍云、新浪、谷歌或微软加载过 jQuery。所以结果是，当他们访问你的站点时，会从缓存中加载 jQuery，这样可以减少加载时间。同时，大多数 CDN 都可以确保当用户向其请求文件时，会从离用户最近的服务器上返回响应，这样也可以提高加载速度。

## jQuery 语法

&emsp;通过 jQuery，可以选取（查询，query）HTML 元素，并对它们执行 "操作"（actions）。

&emsp;jQuery 语法是通过选取 HTML 元素，并对选取的元素执行某些操作。

&emsp;基础语法：`$(selector).action()`

+ 美元符号定义 jQuery
+ 选择符（selector）"查询" 和 "查找" HTML 元素
+ jQuery 的 `action()` 执行对元素的操作

&emsp;一些实例:

+ `$(this).hide()` - 隐藏当前元素
+ `$("p").hide()` - 隐藏所有 `<p>` 元素
+ `$("p.test").hide()` - 隐藏所有 `class = "test"` 的 `<p>` 元素
+ `$("#test").hide()` - 隐藏 `id = "test"` 的元素

> &emsp;jQuery 使用的语法是 XPath 与 CSS 选择器语法的组合。

### 文档就绪事件

&emsp;也许已经注意到在我们的实例中的所有 jQuery 函数位于一个 `document ready` 函数中：

```c++
$(document).ready(function(){
   // 开始写 jQuery 代码...
});
```

&emsp;这是为了防止文档在完全加载（就绪）之前运行 jQuery 代码，即在 DOM 加载完成后才可以对 DOM 进行操作。

&emsp;如果在文档没有完全加载之前就运行函数，操作可能失败。下面是两个具体的例子：

+ 试图隐藏一个不存在的元素
+ 获得未完全加载的图像的大小

&emsp;简洁写法（与以上写法效果相同）：

```c++
$(function(){
   // 开始写 jQuery 代码...
});
```

&emsp;以上两种方式可以选择喜欢的方式实现文档就绪后执行 jQuery 方法。

&emsp;jQuery 入口函数:（可以执行多次，第 N 次都不会被上一次覆盖）

```c++
$(document).ready(function(){
    // 执行代码
});

// 或者

$(function(){
    // 执行代码
});
```

&emsp;JavaScript 入口函数:（只能执行一次，如果第二次，那么第一次的执行会被覆盖）

```c++
window.onload = function () {
    // 执行代码
}
```

&emsp;jQuery 入口函数与 JavaScript 入口函数的区别：

+ jQuery 的入口函数是在 html 所有标签(DOM)都加载之后，就会去执行。
+ JavaScript 的 `window.onload` 事件是等到所有内容，包括外部图片之类的文件加载完后，才会执行。

## jQuery 选择器

&emsp;jQuery 选择器允许对 HTML 元素组或单个元素进行操作。

&emsp;jQuery 选择器基于元素的 id、类、类型、属性、属性值等 "查找"（或选择）HTML 元素。它基于已经存在的 CSS 选择器，除此之外，它还有一些自定义的选择器。jQuery 中所有选择器都以美元符号开头：`$()`。

### 元素选择器

&emsp;jQuery 元素选择器基于元素名选取元素。在页面中选取所有 `<p>` 元素：

```c++
$("p")
```

```c++
$(document).ready(function(){
  $("button").click(function(){
    $("p").hide();
  });
});
```

### #id 选择器

&emsp;jQuery `#id` 选择器通过 HTML 元素的 `id` 属性选取指定的元素。页面中元素的 `id` 应该是唯一的，所以要在页面中选取唯一的元素需要通过 `#id` 选择器。

&emsp;通过 `id` 选取元素语法如下：(和上面的元素名字符串比，前面多了一个 `#` 号)

```c++
$("#test")
```

```c++
$(document).ready(function(){
  $("button").click(function(){
    $("#test").hide();
  });
});
```

### .class 选择器

&emsp;jQuery 类选择器可以通过指定的 `class` 查找元素。

&emsp;语法如下：(和上面的元素名字符串比，前面多了一个 `.`)

```c++
$(".test")
```

&emsp;用户点击按钮后所有带有 `class="test"` 属性的元素都隐藏：

```c++
$(document).ready(function(){
  $("button").click(function(){
    $(".test").hide();
  });
});
```

&emsp;更多选择器的实例：

+ `$("*")` 选取所有元素
+ `$(this)` 选取当前的 HTML 元素
+ `$("p.intro")` 选取 `class` 为 `intro` 的 `<p>` 元素
+ `$("p:first")` 选取第一个 `<p>` 元素
+ `$("ul li:first")` 选取第一个 `<ul>` 元素的第一个 `<li>` 元素
+ `$("ul li:first-child")` 选取每个 `<ul>` 元素的第一个 `<li>` 元素
+ `$("[href]")` 选取带有 `href` 属性的元素
+ `$("a[target='_blank']")` 选取所有 `target` 属性值等于 `_blank` 的 `<a>` 元素
+ `$("a[target!='_blank']")` 选取所有 `target` 属性值不等于 `_blank` 的 `<a>` 元素
+ `$(":button")` 选取所有 `type="button"` 的 `<input>` 元素 和 `<button>` 元素
+ `$("tr:even")` 选取偶数位置的 `<tr>` 元素
+ `$("tr:odd")` 选取奇数位置的 `<tr>` 元素

### 独立文件中使用 jQuery 函数

&emsp;如果网站包含许多页面，并且希望 jQuery 函数易于维护，那么把 jQuery 函数放到独立的 .js 文件中。把它们放到一个单独的文件中会更好，就像这样（通过 `src` 属性来引用文件）：

```c++
<head>

<script src="http://cdn.static.runoob.com/libs/jquery/1.10.2/jquery.min.js"></script>
<script src="my_jquery_functions.js"></script>

</head>
```

## jQuery 事件（DOM 事件）

&emsp;jQuery 是为事件处理特别设计的。

&emsp;事件处理程序指的是当 HTML 中发生某些事件时所调用的方法。事件示例：

+ 在元素上移动鼠标。
+ 选取单选按钮
+ 点击元素
+ ...

### 鼠标事件

+ click 事件：当单击元素时，发生 click 事件。`click()` 方法触发 click 事件，或规定当发生 click 事件时运行的函数。
+ dblclick 事件：当双击元素时，触发 dblclick 事件。`dblclick()` 方法触发 dblclick 事件，或规定当发生 dblclick 事件时运行的函数。dblclick 事件也会产生 click 事件。如果这两个事件都被应用于同一个元素，则会产生问题。
+ mouseenter 事件：当鼠标指针穿过（进入）被选元素时，会发生 mouseenter 事件。`mouseenter()` 方法触发 mouseenter 事件，或添加当发生 mouseenter 事件时运行的函数。与 mouseover 事件不同，mouseenter 事件只有在鼠标指针进入被选元素时被触发，mouseover 事件在鼠标指针进入任意子元素时也会被触发。该事件通常与 mouseleave 事件一起使用。
+ mouseleave 事件：当鼠标指针离开被选元素时，会发生 mouseleave 事件。`mouseleave()` 方法触发 mouseleave 事件，或添加当发生 mouseleave 事件时运行的函数。与 mouseout 事件不同，mouseleave 事件只有在鼠标指针离开被选元素时被触发，mouseout 事件在鼠标指针离开任意子元素时也会被触发。该事件通常与 mouseenter 事件一起使用。
+ hover 事件：`hover(inFunction, outFunction)` 方法规定当鼠标指针悬停在被选元素上时要运行的两个函数。方法触发 mouseenter 和 mouseleave 事件。如果只指定一个函数，则 mouseenter 和 mouseleave 都执行它。`inFunction`：必需，规定 mouseenter 事件发生时运行的函数。`outFunction`：可选，规定 mouseleave 事件发生时运行的函数。

### 键盘事件

+ keypress 事件：与 keypress 事件相关的事件顺序：1. keydown - 键按下的过程 2. keypress - 键被按下 3. keyup - 键被松开。`keypress()` 方法触发 keypress 事件，或规定当发生 keypress 事件时运行的函数。keypress 事件与 keydown 事件类似。当按钮被按下时发生该事件。然而，keypress 事件不会触发所有的键（比如 ALT、CTRL、SHIFT、ESC）。请使用 keydown() 方法来检查这些键。
+ keydown 事件：与 keydown 事件相关的事件顺序：1. keydown - 键按下的过程 2. keypress - 键被按下 3. keyup - 键被松开。当键盘键被按下时发生 keydown 事件。`keydown()` 方法触发 keydown 事件，或规定当发生 keydown 事件时运行的函数。请使用 `event.which` 属性来返回哪个键盘键被按下。
+ keyup 事件：与 keyup 事件相关的事件顺序：1. keydown - 键按下的过程 2. keypress - 键被按下 3. keyup - 键被松开。当键盘键被松开时发生 keyup 事件。`keyup()` 方法触发 keyup 事件，或规定当发生 keyup 事件时运行的函数。请使用 `event.which` 属性来返回哪个键被按下。

### 表单事件

+ submit 事件：当提交表单时，会发生 submit 事件。该事件只适用于 `<form>` 元素。`submit()` 方法触发 submit 事件，或规定当发生 submit 事件时运行的函数。
+ change 事件：当元素的值改变时发生 change 事件（仅适用于表单字段）。`change()` 方法触发 change 事件，或规定当发生 change 事件时运行的函数。当用于 `select` 元素时，change 事件会在选择某个选项时发生。当用于 text field 或 text area 时，change 事件会在元素失去焦点时发生。
+ focus 事件：当元素获得焦点时（当通过鼠标点击选中元素或通过 tab 键定位到元素时），发生 focus 事件。`focus()` 方法触发 focus 事件，或规定当发生 focus 事件时运行的函数。该方法通常与 `blur()` 方法一起使用。
+ blur 事件：当元素失去焦点时发生 blur 事件。`blur()` 方法触发 blur 事件，或规定当发生 blur 事件时运行的函数。该方法常与 `focus()` 方法一起使用。

### 文档/窗口事件

+ load 事件：`load()` 方法在 jQuery 版本 1.8 中已废弃。`load()` 方法添加事件处理程序到 load 事件。当指定的元素已加载时，会发生 load 事件。该事件适用于任何带有 URL 的元素（比如图像、脚本、框架、内联框架）以及 window 对象。根据不同的浏览器（Firefox 和 IE），如果图像已被缓存，则也许不会触发 load 事件。还存在一个名为 `load()` 的 jQuery AJAX 方法。根据不同的参数决定调用哪个方法。
+ resize 事件：当调整浏览器窗口大小时，发生 resize 事件。`resize()` 方法触发 resize 事件，或规定当发生 resize 事件时运行的函数。
+ scroll 事件：当用户滚动指定的元素时，会发生 scroll 事件。scroll 事件适用于所有可滚动的元素和 window 对象（浏览器窗口）。`scroll()` 方法触发 scroll 事件，或规定当发生 scroll 事件时运行的函数。
+ unload 事件：`unload()` 方法在 jQuery 版本 1.8 中被废弃，在 3.0 版本被移除。Firefox 与 Chrome 会阻止弹窗，所以没办法看到效果。当用户离开页面时，会发生 unload 事件。当发生以下情况下，会触发 unload 事件：
  1. 点击某个离开页面的链接
  2. 在地址栏中键入了新的 URL
  3. 使用前进或后退按钮
  4. 关闭浏览器窗口
  5. 重新加载页面
  `unload()` 方法规定当 unload 事件发生时会发生什么。`unload()` 方法只应用于 window 对象。unload 事件在不同浏览器中效果不一样，请确保使用前在所有浏览器测试该方法。

### jQuery 事件方法语法

&emsp;在 jQuery 中，大多数 DOM 事件都有一个等效的 jQuery 方法。页面中指定一个点击事件：

```c++
$("p").click();
```

&emsp;定义了点击后触发事件。可以通过一个事件函数实现：

```c++
$("p").click(function(){
    // 动作触发后执行的代码
});
```

### 常用的 jQuery 事件方法

#### $(document).ready()

&emsp;`$(document).ready()` 方法允许我们在文档完全加载完后执行函数。

#### click()

&emsp;




















## 参考链接
**参考链接:🔗**
+ [jQuery 教程](https://www.runoob.com/jquery/jquery-tutorial.html)

