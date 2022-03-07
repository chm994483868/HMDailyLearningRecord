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

### #id 选择器

&emsp;























## 参考链接
**参考链接:🔗**
+ [jQuery 教程](https://www.runoob.com/jquery/jquery-tutorial.html)

