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

&emsp;`click()` 方法是当按钮点击事件被触发时会调用一个函数。该函数在用户点击 HTML 元素时执行。

```c++
$("p").click(function(){
  $(this).hide();
});
```

#### dblclick()

&emsp;当双击元素时，会发生 dblclick 事件。`dblclick()` 方法触发 dblclick 事件，或规定当发生 dblclick 事件时运行的函数：

```c++
$("p").dblclick(function(){
  $(this).hide();
});
```

#### mouseenter()

&emsp;当鼠标指针穿过元素时，会发生 mouseenter 事件。`mouseenter()` 方法触发 mouseenter 事件，或规定当发生 mouseenter 事件时运行的函数：

```c++
$("#p1").mouseenter(function(){
    alert('鼠标移到了 id="p1" 的元素上!');
});
```

#### mouseleave()

&emsp;当鼠标指针离开元素时，会发生 mouseleave 事件。`mouseleave()` 方法触发 mouseleave 事件，或规定当发生 mouseleave 事件时运行的函数：

```c++
$("#p1").mouseleave(function(){
    alert("鼠标离开了该段落");
});
```

#### mousedown()

&emsp;当鼠标指针移动到元素上方，并按下鼠标按键时，会发生 mousedown 事件。`mousedown()` 方法触发 mousedown 事件，或规定当发生 mousedown 事件时运行的函数：

```c++
$("#p1").mousedown(function(){
    alert("鼠标在该段落上按下！");
});
```

#### mouseup()

&emsp;当在元素上松开鼠标按钮时，会发生 mouseup 事件。`mouseup()` 方法触发 mouseup 事件，或规定当发生 mouseup 事件时运行的函数：

```c++
$("#p1").mouseup(function(){
    alert("鼠标在段落上松开。");
});
```

#### hover()

&emsp;`hover()` 方法用于模拟光标悬停事件。当鼠标移动到元素上时，会触发指定的第一个函数(mouseenter);当鼠标移出这个元素时，会触发指定的第二个函数(mouseleave)。

```c++
$("#p1").hover(
    function(){
        alert("进入了 p1!");
    },
    function(){
        alert("离开了 p1!");
    }
);
```

#### focus()

&emsp;当元素获得焦点时，发生 focus 事件。当通过鼠标点击选中元素或通过 tab 键定位到元素时，该元素就会获得焦点。`focus()` 方法触发 focus 事件，或规定当发生 focus 事件时运行的函数：

```c++
$("input").focus(function(){
  $(this).css("background-color","#cccccc");
});
```

#### blur()

&emsp;当元素失去焦点时，发生 blur 事件。`blur()` 方法触发 blur 事件，或规定当发生 blur 事件时运行的函数：

```c++
$("input").blur(function(){
  $(this).css("background-color","#ffffff");
});
```

###  keypress、keydown、keyup 的区别

1. keydown：在键盘上按下某键时发生，一直按着则会不断触发（opera浏览器除外），它返回的是键盘代码
2. keypress：在键盘上按下一个按键，并产生一个字符时发生，返回 ASCII 码。注意: shift、alt、ctrl 等键按下并不会产生字符，所以监听无效，换句话说只有按下能在屏幕上输出字符的按键时 keypress 事件才会触发，若一直按着某按键则会不断触发
3. keyup：用户松开某一个按键时触发，与 keydown 相对，返回键盘代码

## jQuery 效果 - 隐藏和显示

### jQuery hide() 和 show()

&emsp;通过 jQuery，可以使用 `hide()` 和 `show()` 方法来隐藏和显示 HTML 元素：

```c++
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
    <script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
    <script>
        $(document).ready(function() {
            $("#hide").click(function() {
                $("p").hide();
            });
            $("#show").click(function() {
                $("p").show();
            });
        });
    </script>
</head>

<body>
    <p>1</p>
    <p>2</p>
    <p>3</p>
    <button id="hide">点击隐藏 p 标签</button>
    <button id="show">点击显示 p 标签</button>
</body>
```

```c++
$(selector).hide(speed,callback);
$(selector).show(speed,callback);
```

&emsp;可选的 `speed` 参数规定隐藏/显示的速度，可以取以下值："slow"、"fast" 或毫秒。可选的 `callback` 参数是隐藏或显示完成后所执行的函数名称。

```c++
<script>
    $(document).ready(function() {
        $("#hide").click(function() {
            $("p").hide(500, "linear", function() {
                alert("Hide() 方法执行完成!");
            });
        });
        $("#show").click(function() {
            $("p").show(1000, "swing", function() {
                alert("Show() 方法执行完成!");
            });
        });
    });
</script>
```
&emsp;第二个参数是一个字符串，表示过渡使用哪种缓动函数。（jQuery 自身提供 "linear" 和 "swing"，其他可以使用相关的插件）。

### jQuery toggle()

&emsp;通过 jQuery，可以使用 `toggle()` 方法来切换 `hide()` 和 `show()` 方法。显示被隐藏的元素，并隐藏已显示的元素：

```c++
$("#hide").click(function() {
    $("p").toggle();
});
```

```c++
$(selector).toggle(speed,callback);
```

&emsp;可选的 `speed` 参数规定隐藏/显示的速度，可以取以下值："slow"、"fast" 或毫秒。可选的 callback 参数是隐藏或显示完成后所执行的函数名称。

&emsp;对于可选的 `callback` 参数，有以下说明：

1. `$(selector)` 选中的元素的个数为 n 个，则 `callback` 函数会执行 n 次（对于这里，当 `callback` 函数加上括号时，函数立即执行，只会调用一次，如果不加括号，元素显示或隐藏后调用函数，才会调用 n 次）
2. `callback` 函数名后加括号，会立刻执行函数体，而不是等到 显示/隐藏 完成后才执行
3. `callback` 既可以是函数名，也可以是匿名函数

## jQuery 效果 - 淡入淡出

&emsp;jQuery 可以实现元素的淡入淡出效果。

+ `fadeIn()` 淡入

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("#div1").fadeIn();
            $("#div2").fadeIn("slow");
            $("#div3").fadeIn(3000);
        });
    });
</script>
<button>点击淡入 div 元素。</button>
<div id="div1" style="width: 80px;height: 80px;display: none;background-color: red;"></div><br>
<div id="div2" style="width: 80px;height: 80px;display: none;background-color: green;"></div><br>
<div id="div3" style="width: 80px;height: 80px;display: none;background-color: blue;"></div>
```

+ `fadeOut()` 淡出

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("#div1").fadeOut();
            $("#div2").fadeOut("slow");
            $("#div3").fadeOut(3000);
        });
    });
</script>
<button>点击淡入 div 元素。</button>
<div id="div1" style="width: 80px;height: 80px;background-color: red;"></div><br>
<div id="div2" style="width: 80px;height: 80px;background-color: green;"></div><br>
<div id="div3" style="width: 80px;height: 80px;background-color: blue;"></div>
```

+ `fadeToggle()` 不同速度的淡出

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
$(document).ready(function(){
    $("button").click(function(){
        $("#div1").fadeToggle();
        $("#div2").fadeToggle("slow");
        $("#div3").fadeToggle(3000);
    });
});
</script>

<div id="div1" style="width:80px;height:80px;background-color:red;"></div><br>
<div id="div2" style="width:80px;height:80px;background-color:green;"></div><br>
<div id="div3" style="width:80px;height:80px;background-color:blue;"></div>
```

+ `fadeTo()` 淡出到指定透明度，淡出了但是又没有完全淡出

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
$(document).ready(function(){
  $("button").click(function(){
    $("#div1").fadeTo("slow",0.15);
    $("#div2").fadeTo("slow",0.4);
    $("#div3").fadeTo("slow",0.7);
  });
});
</script>

<div id="div1" style="width:80px;height:80px;background-color:red;"></div><br>
<div id="div2" style="width:80px;height:80px;background-color:green;"></div><br>
<div id="div3" style="width:80px;height:80px;background-color:blue;"></div>
```

### jQuery Fading 方法

&emsp;jQuery 拥有下面四种 fade 方法：

+ `fadeIn()`
+ `fadeOut()`
+ `fadeToggle()`
+ `fadeTo()`

#### jQuery fadeIn() 方法

&emsp;`fadeIn()` 用于淡入已隐藏的元素。

```javascript
$(selector).fadeIn(speed,callback);
```

&emsp;可选的 `speed` 参数规定效果的时长。它可以取以下值："slow"、"fast" 或毫秒数。可选的 `callback` 参数是 `fading` 完成后所执行的函数名称。

#### jQuery fadeOut() 方法

&emsp;`fadeOut()` 方法用于淡出可见元素。

```javascript
$(selector).fadeOut(speed,callback);
```

&emsp;可选的 `speed` 参数规定效果的时长。它可以取以下值："slow"、"fast" 或毫秒数。可选的 `callback` 参数是 fading 完成后所执行的函数名称。

#### jQuery fadeToggle() 方法

&emsp;`fadeToggle()` 方法可以在 `fadeIn()` 与 `fadeOut()` 方法之间进行切换。如果元素已淡出，则 `fadeToggle()` 会向元素添加淡入效果。如果元素已淡入，则 `fadeToggle()` 会向元素添加淡出效果。

```javascript
$(selector).fadeToggle(speed,callback);
```

&emsp;可选的 `speed` 参数规定效果的时长。它可以取以下值："slow"、"fast" 或毫秒数。可选的 `callback` 参数是 fadeIn/fadeOut 完成后所执行的函数名称。

#### jQuery fadeTo() 方法

&emsp;`fadeTo()` 方法允许渐变为给定的不透明度（值介于 0 与 1 之间）。

```javascript
$(selector).fadeTo(speed,opacity,callback);
```

&emsp;必需的 `speed` 参数规定效果的时长，`fadeTo()` 没有默认参数，所以必须加上："slow"、"fast" 或毫秒数。`fadeTo()` 方法中必需的 `opacity` 参数将淡入淡出效果设置为给定的不透明度（值介于 0 与 1 之间）。可选的 `callback` 参数是该函数完成后所执行的函数名称。

## jQuery 效果 - 滑动

&emsp;jQuery 滑动方法可使元素上下滑动。jQuery 拥有以下滑动方法：

+ `slideDown()`
+ `slideUp()`
+ `slideToggle()`

### jQuery slideDown() 方法

&emsp;jQuery `slideDown()` 方法用于向下滑动元素。

```javascript
$(selector).slideDown(speed, callback);
```

&emsp;可选的 `speed` 参数规定效果的时长。它可以取以下值："slow"、"fast" 或毫秒。可选的 `callback` 参数是滑动完成后所执行的函数名称。

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $('#flip').click(function() {
            $("#panel").slideDown("slow");
        });
    });
</script>
<style type="text/css">
    #panel,
    #flip {
        padding: 5px;
        text-align: center;
        background-color: #e5eecc;
        border: solid 1px #c3c3c3;
    }
    
    #panel {
        padding: 50px;
        display: none;
    }
</style>
<div id="flip">点我滑下面板</div>
<div id="panel">Hello jQuery!</div>
```

### jQuery slideUp() 方法

&emsp;jQuery slideUp() 方法用于向上滑动元素。

```javascript
$(selector).slideUp(speed, callback);
```

&emsp;可选的 `speed` 参数规定效果的时长。它可以取以下值："slow"、"fast" 或毫秒。可选的 `callback` 参数是滑动完成后所执行的函数名称。

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $('#flip').click(function() {
            $("#panel").slideUp("slow");
        });
    });
</script>
<style type="text/css">
    #panel,
    #flip {
        padding: 5px;
        text-align: center;
        background-color: #e5eecc;
        border: solid 1px #c3c3c3;
    }
    
    #panel {
        padding: 50px;
    }
</style>
<div id="flip">点我拉起面板</div>
<div id="panel">Hello jQuery!</div>
```

### jQuery slideToggle() 方法

&emsp;jQuery slideToggle() 方法可以在 slideDown() 与 slideUp() 方法之间进行切换。如果元素向下滑动，则 slideToggle() 可向上滑动它们。如果元素向上滑动，则 slideToggle() 可向下滑动它们。

```javascript
$(selector).slideToggle(speed,callback);
```

&emsp;可选的 `speed` 参数规定效果的时长。它可以取以下值："slow"、"fast" 或毫秒。可选的 `callback` 参数是滑动完成后所执行的函数名称。

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $('#flip').click(function() {
            $("#panel").slideToggle("slow");
        });
    });
</script>
<style type="text/css">
    #panel,
    #flip {
        padding: 5px;
        text-align: center;
        background-color: #e5eecc;
        border: solid 1px #c3c3c3;
    }
    
    #panel {
        padding: 50px;
        display: none;
    }
</style>
<div id="flip">点我，显示/隐藏 面板</div>
<div id="panel">Hello jQuery!</div>
```

## jQuery 效果- 动画

&emsp;jQuery animate() 方法允许创建自定义的动画。

### jQuery 动画 - animate() 方法

&emsp;jQuery animate() 方法用于创建自定义动画。

```javascript
$(selector).animate({params},speed,callback);
```

&emsp;必需的 `params` 参数定义形成动画的 CSS 属性。可选的 `speed` 参数规定效果的时长。可取："slow"、"fast" 或毫秒。可选的 `callback` 参数是动画完成后所执行的函数名称。下面的例子演示 animate() 方法的简单应用。它把 `<div>` 元素往右边移动了 250 像素：

```javascript
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("div").animate({
                left: '250px'
            });
        });
    });
</script>

<button>开始动画</button>
<p>默认情况下，所有的 HTML 元素有一个静态的位置，且是不可移动的。 如果需要改变为，我们需要将元素的 position 属性设置为 relative, fixed, 或 absolute!</p>
<div style="background: #98bf21;height: 100px;width: 100px;position: absolute;"></div>
```

&emsp;默认情况下，所有 HTML 元素都有一个静态位置，且无法移动。如需对位置进行操作，要记得首先把元素的 CSS position 属性设置为 relative、fixed 或 absolute！

### jQuery animate() - 操作多个属性

&emsp;生成动画的过程中可同时使用多个属性：

```javascript
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("div").animate({
                left: '250px',
                opacity: '0.5',
                height: '150px',
                width: '150px'
            });
        });
    });
</script>

<button>开始动画</button>
<p>默认情况下，所有的 HTML 元素有一个静态的位置，且是不可移动的。 如果需要改变为，我们需要将元素的 position 属性设置为 relative, fixed, 或 absolute!</p>
<div style="background: #98bf21;height: 100px;width: 100px;position: absolute;"></div>
```

&emsp;可以用 animate() 方法来操作所有 CSS 属性吗？可以，但是需要记住一件重要的事情：当使用 animate() 时，必须使用 Camel 标记法书写所有的属性名，比如，必须使用 `paddingLeft` 而不是 `padding-left`，使用 `marginRight` 而不是 `margin-right`，等等。同时，色彩动画并不包含在核心 jQuery 库中。

### jQuery animate() - 使用相对值

&emsp;也可以定义相对值（该值相对于元素的当前值）。需要在值的前面加上 `+=` 或 `-=`：

```javascript
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("div").animate({
                left: '250px',
                height: '+=150px',
                width: '+=150px'
            });
        });
    });
</script>

<button>开始动画</button>
<p>默认情况下，所有的 HTML 元素有一个静态的位置，且是不可移动的。 如果需要改变为，我们需要将元素的 position 属性设置为 relative, fixed, 或 absolute!</p>
<div style="background: #98bf21;height: 100px;width: 100px;position: absolute;"></div>
```

### jQuery animate() - 使用预定义的值

&emsp;甚至可以把属性的动画值设置为 "show"、"hide" 或 "toggle"：

```javascript
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("div").animate({
                height: 'toggle',
            });
        });
    });
</script>

<button>开始动画</button>
<p>默认情况下，所有的 HTML 元素有一个静态的位置，且是不可移动的。 如果需要改变为，我们需要将元素的 position 属性设置为 relative, fixed, 或 absolute!</p>
<div style="background: #98bf21;height: 100px;width: 100px;position: absolute;"></div>
```

### jQuery animate() - 使用队列功能

&emsp;默认地，jQuery 提供针对动画的队列功能。这意味着如果在彼此之后编写多个 animate() 调用，jQuery 会创建包含这些方法调用的 "内部" 队列。然后逐一运行这些 animate 调用。

```javascript
<script>
    $(document).ready(function() {
        $("button").click(function() {
            var div = $("div");
            div.animate({
                height: '300px',
                opacity: '0.4'
            }, "slow");
            div.animate({
                width: '300px',
                opacity: '0.8'
            }, "slow");
            div.animate({
                height: '100px',
                opacity: '0.4'
            }, "slow");
            div.animate({
                width: '100px',
                opacity: '0.8'
            }, "slow");
        });
    });
</script>   
<button>开始动画</button>
<p>默认情况下，所有的 HTML 元素有一个静态的位置，且是不可移动的。 如果需要改变为，我们需要将元素的 position 属性设置为 relative, fixed, 或 absolute!</p>
<div style="background: #98bf21;height: 100px;width: 100px;position: absolute;"></div>
```

&emsp;下面的例子把 `<div>` 元素往右边移动了 100 像素，然后增加文本的字号：

```javascript
<script>
    $(document).ready(function() {
        $("button").click(function() {
            var div = $("div");
            div.animate({
                left: '100px'
            }, "slow");
            div.animate({
                fontSize: '3em'
            }, "slow");
        });
    });
</script>

<button>开始动画</button>
<p>默认情况下，所有的 HTML 元素有一个静态的位置，且是不可移动的。 如果需要改变为，我们需要将元素的 position 属性设置为 relative, fixed, 或 absolute!</p>
<div style="background: #98bf21;height: 100px;width: 200px;position: absolute;">HELLO</div>
```

## jQuery 停止动画

&emsp;jQuery stop() 方法用于在动画或效果完成前对它们进行停止。stop() 方法适用于所有 jQuery 效果函数，包括滑动、淡入淡出和自定义动画。

```typescript
$(selector).stop(stopAll, goToEnd);
```

&emsp;可选的 `stopAll` 参数规定是否应该清除动画队列。默认是 `false`，即仅停止活动的动画，允许任何排入队列的动画向后执行。

&emsp;可选的 `goToEnd` 参数规定是否立即完成当前动画。默认是 `false`。因此，默认地，stop() 会清除在被选元素上指定的当前动画。

```typescript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("#flip").click(function() {
            $("#panel").slideDown(5000);
        });
        $("#stop").click(function() {
            $("#panel").stop();
        });
    });
</script>
```

&emsp;动画队列停止动画测试，只停止当前正在进行的动画，停止当前动画后，队列中的下一个动画开始进行：

```typescript
<script>
    $(document).ready(function() {
        $("#flip").click(function() {
            $("#panel").slideDown(5000);
            $("#panel").slideUp(5000);
        });
        $("#stop").click(function() {
            $("#panel").stop();
        });
    });
</script>
```

## jQuery Callback 方法

&emsp;Callback 函数在当前动画 100% 完成之后执行。

&emsp;许多 jQuery 函数涉及动画。这些函数也许会将 `speed` 或 `duration` 作为可选参数。例子：`$("p").hide("slow")`。`speed` 或 `duration` 参数可以设置许多不同的值，比如 `slow`, `fast`, `normal` 或毫秒。

```javascript
$(document).ready(function(){
  $("button").click(function(){
    $("p").hide("slow",function(){
      alert("段落现在被隐藏了");
    });
  });
});
```

&emsp;以下实例没有回调函数，警告框会在隐藏效果完成前弹出：(立即弹出 alert，并且点击了确定按钮后，p 才会直接隐藏，并没有动画效果)

```javascript
$(document).ready(function(){
  $("button").click(function(){
    $("p").hide(1000);
    alert("现在段落被隐藏了");
  });
});
```

&emsp;被立即停止的动画不会触发回调，被立即完成的动画会触发回调。

## jQuery - 链(Chaining)

&emsp;通过 jQuery，可以把动作/方法链接在一起。Chaining 允许在一条语句中运行多个 jQuery 方法（在相同的元素上）。

### jQuery 方法链接

&emsp;直到现在，我们都是一次写一条 jQuery 语句（一条接着另一条）。不过，有一种名为链接（chaining）的技术，允许我们在相同的元素上运行多条 jQuery 命令，一条接着另一条。这样的话，浏览器就不必多次查找相同的元素。如需链接一个动作，只需简单地把该动作追加到之前的动作上。

&emsp;下面的例子把 `css()`、`slideUp()` 和 `slideDown()` 链接在一起。"p1" 元素首先会变为红色，然后向上滑动，再然后向下滑动：

```javascript
$(document).ready(function() {
    $("button").click(function() {
        $("#p1").css("color", "red").slideUp(2000).slideDown(2000);
    });
});
```

&emsp;如果需要，我们也可以添加多个方法调用。当进行链接时，代码行会变得很长。不过，jQuery 语法不是很严格，可以按照希望的格式来写，包含换行和缩进。

&emsp;如下书写也可以很好地运行：

```javascript
$(document).ready(function() {
    $("button").click(function() {
        $("#p1").css("color", "red")
            .slideUp(2000)
            .slideDown(2000);
    });
});
```

&emsp;jQuery 会抛掉多余的空格，并当成一行长代码来执行上面的代码行。

## jQuery - 获取内容和属性

&emsp;jQuery 拥有可操作 HTML 元素和属性的强大方法。

### jQuery DOM 操作

&emsp;jQuery 中非常重要的部分，就是操作 DOM 的能力。jQuery 提供一系列与 DOM 相关的方法，这使访问和操作元素和属性变得很容易。

> &emsp;DOM = Document Object Model（文档对象模型）DOM 定义访问 HTML 和 XML 文档的标准："W3C 文档对象模型独立于平台和语言的界面，允许程序和脚本动态访问和更新文档的内容、结构以及样式。"

### 获得内容 - text()、html() 以及 val()

&emsp;三个简单实用的用于 DOM 操作的 jQuery 方法：

+ `text()` - 设置或返回所选元素的文本内容
+ `html()` - 设置或返回所选元素的内容（包括 HTML 标记）
+ `val()` - 设置或返回表单字段的值

&emsp;下面的例子演示如何通过 jQuery `text()` 和 `html()` 方法来获得内容：

```javascript
<script>
    $(document).ready(function() {
        $("#btn1").click(function() {
            alert("Text: " + $("#test").text());
        });
        $("#btn2").click(function() {
            alert("HTML: " + $("#test").html());
        });
    });
</script>

<p id="test">这是段落中的 <b>粗体</b> 文本。</p>
<button id="btn1">显示文本</button>
<button id="btn2">显示 HTML</button>
```

&emsp;下面的例子演示如何通过 jQuery `val()` 方法获得输入字段的值：

```javascript
<script>
    $(document).ready(function() {
        $("button").click(function() {
            alert("值为: " + $("#test").val());
        });
    });
</script>
<p>名称: <input type="text" id="test" value="XXXX"></p>
<button>显示值</button>
```

### 获取属性 - attr()

&emsp;jQuery attr() 方法用于获取属性值。下面的例子演示如何获得链接中 `href` 属性的值：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            alert($("#xxx").attr("href"));
        });
    });
</script>
<p><a href="https://www.baidu.com" id="xxx">BaiDu</a></p>

<button>显示值 href 属性的值</button>
```

### attr 和 prop 的区别介绍：

&emsp;对于 HTML 元素本身就带有的固有属性，在处理时，使用 prop 方法。对于 HTML 元素我们自己自定义的 DOM 属性，在处理时，使用 attr 方法。

```javascript
<a href="#" target="_self" class="btn">Touch me!</a>
```

&emsp;这个例子里 `<a>` 元素的 DOM 属性有: `href`、`target` 和 `class`，这些属性就是 `<a>` 元素本身就带有的属性，也是 W3C 标准里就包含有这几个属性，或者说在 IDE 里能够智能提示出的属性，这些就叫做固有属性。处理这些属性时，建议使用 `prop` 方法。

```javascript
<a href="#" id="link1" action="delete" rel="nofollow">Delete</a>
```

&emsp;这个例子里 `<a>` 元素的 DOM 属性有: `href`、`id` 和 `action`，很明显，前两个是固有属性，而后面一个 `action` 属性是我们自己自定义上去的，`<a>` 元素本身是没有这个属性的。这种就是自定义的 DOM 属性。处理这些属性时，建议使用 `attr` 方法。

## jQuery - 设置内容和属性

&emsp;设置内容 - `text()`、`html()` 以及 `val()`，使用前一章中的三个相同的方法来设置内容：

+ `text()` - 设置或返回所选元素的文本内容
+ `html()` - 设置或返回所选元素的内容（包括 HTML 标记）
+ `val()` - 设置或返回表单字段的值

&emsp;下面的例子演示如何通过 `text()`、`html()` 以及 `val()` 方法来设置内容：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("#btn1").click(function() {
            $("#test1").text("Hello world!");
        });
        $("#btn2").click(function() {
            $("#test2").html("<b>Hello world!</b>");
        });
        $("#btn3").click(function() {
            $("#test3").val("Enter...");
        });
    })
</script>

<p id="test1">这是一个段落</p>
<p id="test2">这是另外一个段落</p>
<p>输入框：<input type="text" id="test3" value="菜鸟教程"></p>
<button id="btn1">设置文本</button>
<button id="btn2">设置 HTML</button>
<button id="btn3">设置值</button>
```

### text()、html() 以及 val() 的回调函数

&emsp;上面的三个 jQuery 方法：`text()`、`html()` 以及 `val()`，同样拥有回调函数。回调函数有两个参数：被选元素列表中当前元素的下标，以及原始（旧的）值。然后以函数新值返回你希望使用的字符串。

&emsp;下面的例子演示带有回调函数的 `text()` 和 `html()`：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("#btn1").click(function() {
            $("#test1").text(function(i, origText) {
                return "旧文本: " + origText + " 新文本: Hello jQuery! (index: " + i + ")";
            });
        });
        $("#btn2").click(function() {
            $("#test2").html(function(i, origText) {
                return "旧 html: " + origText + " 新 html: Hello <b>jQuery!</b> (index: " + i + ")";
            });
        });
    });
</script>

<p id="test1">这是一个有 <b>粗体</b> 字的段落。</p>
<p id="test2">这是另外一个有 <b>粗体</b> 字的段落。</p>
<button id="btn1">显示 新/旧 文本</button>
<button id="btn2">显示 新/旧 HTML</button>
```

### 设置属性 - attr()

&emsp;jQuery attr() 方法也用于设置/改变属性值。下面的例子演示如何改变（设置）链接中 href 属性的值：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("#test").attr("href", "http://www.baidu.com");
        });
    });
</script>
<p><a href="https://www.google.com" id="test">Google</a></p>
<button>修改 href 的值</button>
```

&emsp;attr() 方法也允许同时设置多个属性。下面的例子演示如何同时设置 href 和 title 属性：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("#test").attr({
                "href": "http://www.baidu.com",
                "title": "Baidu"
            });
            // 通过修改的 title 值来修改链接名称
            title = $("#test").attr('title');
            $("#test").html(title);
        });
    });
</script>
<p><a href="https://www.google.com" id="test">Google</a></p>
<button>修改 href/title 的值</button>
```

### attr() 的回调函数

&emsp;jQuery 方法 attr()，也提供回调函数。回调函数有两个参数：被选元素列表中当前元素的下标，以及原始（旧的）值。然后以函数新值返回希望使用的字符串。下面的例子演示带有回调函数的 attr() 方法：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("#test").attr("href", function(i, origValue) {
                return origValue + "/map";
            });
        });
    });
</script>
<p><a href="https://www.baidu.com" id="test">Baidu</a></p>
<button>修改 href 的值</button>
```

## jQuery - 添加元素

&emsp;通过 jQuery，可以很容易地添加新元素/内容。

### 添加新的 HTML 内容

&emsp;我们将学习用于添加新内容的四个 jQuery 方法：

+ append() - 在被选元素的结尾插入内容
+ prepend() - 在被选元素的开头插入内容
+ after() - 在被选元素之后插入内容
+ before() - 在被选元素之前插入内容

### jQuery append() 方法

&emsp;jQuery append() 方法在被选元素的结尾插入内容（仍然在该元素的内部）。

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("#btn1").click(function() {
            $("p").append(" <b>追加文本</b>。");
        });

        $("#btn2").click(function() {
            $("ol").append("<li>追加列表项</li>");
        });
    });
</script>

<p>这是一个段落。</p>
<p>这是一个另一个段落。</p>

<ol>
    <li>List item 1</li>
    <li>List item 2</li>
    <li>List item 3</li>
</ol>

<button id="btn1">添加文本</button>
<button id="btn2">添加列表项</button>
```

### jQuery prepend() 方法 

&emsp;jQuery prepend() 方法在被选元素的开头插入内容。

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("#btn1").click(function() {
            $("p").prepend("<b>在开头追加文本</b>。");
        });
        $("#btn2").click(function() {
            $("ol").prepend("<li>在开头添加列表项</li>");
        });
    });
</script>

<p>这是一个段落。</p>
<p>这是另外一个段落。</p>
<ol>
    <li>列表 1</li>
    <li>列表 2</li>
    <li>列表 3</li>
</ol>
<button id="btn1">添加文本</button>
<button id="btn2">添加列表项</button>
```

### 通过 append() 和 prepend() 方法添加若干新元素

&emsp;在上面的例子中，我们只在被选元素的 开头/结尾 插入 文本/HTML。不过，append() 和 prepend() 方法能够通过参数接收无限数量的新元素。可以通过 jQuery 来生成 文本/HTML（就像上面的例子那样），或者通过 JavaScript 代码和 DOM 元素。

&emsp;在下面的例子中，我们创建若干个新元素。这些元素可以通过 text/HTML、jQuery 或者 JavaScript/DOM 来创建。然后我们通过 append() 方法把这些新元素追加到文本中（对 prepend() 同样有效）：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    function appendText() {
        var txt1 = "<p>文本-1。</p>"; // 使用 HTML 标签创建文本
        var txt2 = $("<p></p>").text("文本-2。"); // 使用 jQuery 创建文本
        var txt3 = document.createElement("p");
        txt3.innerHTML = "文本-3"; // 使用 DOM 创建文本 text with DOM
        $("body").append(txt1, txt2, txt3); // 追加新元素
    }
</script>

<p>这是一个段落。</p>
<button onclick="appendText()">追加文本</button>
```

### jQuery after() 和 before() 方法

&emsp;jQuery after() 方法在被选元素之后插入内容。jQuery before() 方法在被选元素之前插入内容。

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("#btn1").click(function() {
            $("img").before("<b>之前</b>");
        });
        $("#btn2").click(function() {
            $("img").after("<i>之后</i>");
        });
    });
</script>
<img src="/image/logo.png">
<br><br>
<button id="btn1">之前插入</button>
<button id="btn2">之后插入</button>
```

### 通过 after() 和 before() 方法添加若干新元素

&emsp;after() 和 before() 方法能够通过参数接收无限数量的新元素。可以通过 text/HTML、jQuery 或者 JavaScript/DOM 来创建新元素。

&emsp;在下面的例子中，我们创建若干新元素。这些元素可以通过 text/HTML、jQuery 或者 JavaScript/DOM 来创建。然后我们通过 after() 方法把这些新元素插到文本中（对 before() 同样有效）：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    function afterText() {
        var txt1 = "<b>I </b>"; // 使用 HTML 创建元素
        var txt2 = $("<i></i>").text("love "); // 使用 jQuery 创建元素
        var txt3 = document.createElement("big"); // 使用 DOM 创建元素
        txt3.innerHTML = "jQuery!";
        $("img").after(txt1, txt2, txt3); // 在图片后添加文本
    }
</script>
<img src="xxx.png">
<br><br>
<button onclick="afterText()">之后插入</button>
```

&emsp;append/prepend 是在选择元素内部嵌入。after/before 是在元素外面追加。

&emsp;after() 参数可以是个 list:

```javascript
function afterText(){
    var txt1="<b>I </b>";                    // 使用 HTML 创建元素
    var txt2=$("<i></i>").text("love ");     // 使用 jQuery 创建元素
    var txt3=document.createElement("big");  // 使用 DOM 创建元素
    txt3.innerHTML="jQuery!";
    $("img").after([txt1,txt2,txt3]);          // 在图片后添加文本
}
```

## jQuery - 删除元素

&emsp;通过 jQuery，可以很容易地删除已有的 HTML 元素。

&emsp;如需删除元素和内容，一般可使用以下两个 jQuery 方法：

+ remove() - 删除被选元素（及其子元素）
+ empty() - 从被选元素中删除子元素

### jQuery remove() 方法

&emsp;jQuery remove() 方法删除被选元素及其子元素。

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("#div1").remove();
        });
    });
</script>
<div id="div1" style="height: 100px;width: 300px;border: 1px solid black;background-color: yellow;">
    这是 div 中的一些文本。
    <p>这是 div 中的一个段落。</p>
    <p>这是 div 中的另外一个段落。</p>
</div>
<br>
<button>移除 div 元素</button>
```

### jQuery empty() 方法

&emsp;jQuery empty() 方法删除被选元素的子元素。

```javascript
$(document).ready(function() {
    $("button").click(function() {
        $("#div1").empty();
    });
});
```

### 过滤被删除的元素

&emsp;jQuery remove() 方法也可接受一个参数，允许你对被删元素进行过滤。该参数可以是任何 jQuery 选择器的语法。

&emsp;下面的例子删除 class="italic" 的所有 `<p>` 元素：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("p").remove(".italic");
        });
    });
</script>
<p>这是一个段落。</p>
<p class="italic"><i>这是另外一个段落。</i></p>
<p class="italic"><i>这是另外一个段落。</i></p>

<button>移除所有  class="italic" 的 p 元素。</button>
```

## jQuery - 获取并设置 CSS 类

&emsp;通过 jQuery，可以很容易地对 CSS 元素进行操作。

### jQuery 操作 CSS 

&emsp;jQuery 拥有若干进行 CSS 操作的方法。我们将学习下面这些：

+ addClass() - 向被选元素添加一个或多个类
+ removeClass() - 从被选元素删除一个或多个类
+ toggleClass() - 对被选元素进行添加/删除类的切换操作
+ css() - 设置或返回样式属性

### jQuery addClass() 方法

&emsp;下面的例子展示如何向不同的元素添加 class 属性。当然，在添加类时，也可以选取多个元素：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("h1, h2, p").addClass("blue");
            $("div").addClass("important");
        });
    });
</script>
<style type="text/css">
    .important {
        font-weight: bold;
        font-size: xx-large;
    }
    
    .blue {
        color: blue;
    }
</style>

<h1>标题 1</h1>
<h2>标题 2</h2>
<p>这是一个段落</p>
<p>这是另外一个段落</p>
<div>这是一些重要的文本!</div>
<br>
<button>为元素添加 class</button>
```

&emsp;也可以在 addClass() 方法中规定多个类：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("body div:first").addClass("important blue");
        });
    });
</script>
<style type="text/css">
    .important {
        font-weight: bold;
        font-size: xx-large;
    }
    
    .blue {
        color: blue;
    }
</style>

<div id="div1">这是一些文本。</div>
<div id="div2">这是一些文本。</div>
<br>
<button>为第一个 div 元素添加类</button>
```

### jQuery removeClass() 方法

&emsp;下面的例子演示如何在不同的元素中删除指定的 class 属性：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("h1, h2, p").removeClass("blue");
        });
    });
</script>
<style type="text/css">
    .important {
        font-weight: bold;
        font-size: xx-large;
    }
    
    .blue {
        color: blue;
    }
</style>

<h1 class="blue">标题 1</h1>
<h2 class="blue">标题 2</h2>
<p class="blue">这是一个段落。</p>
<p class="important">这是另外一个段落。</p>
<br>
<button>从元素中移除 class</button>
```

### jQuery toggleClass() 方法

&emsp;下面的例子将展示如何使用 jQuery toggleClass() 方法。该方法对被选元素进行添加/删除类的切换操作：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("h1, h2, p").toggleClass("blue");
        });
    });
</script>
<style type="text/css">
    .blue {
        color: blue;
    }
</style>

<h1 class="blue">标题 1</h1>
<h2 class="blue">标题 2</h2>
<p class="blue">这是一个段落。</p>
<p>这是另外一个段落。</p>
<br>
<button>切换 class</button>
```

## jQuery css() 方法

&emsp;css() 方法设置或返回被选元素的一个或多个样式属性。

### 返回 CSS 属性

&emsp;如需返回指定的 CSS 属性的值，请使用如下语法：

```javascript
css("propertyname");
```

&emsp;下面的例子将返回首个匹配元素的 background-color 值：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            alert("背景颜色 = " + $("p").css("background-color"));
        });
    });
</script>

<h2>这是一个标题</h2>
<p style="background-color: #ff0000;">这是一个段落。</p>
<p style="background-color: #00ff00;">这是一个段落。</p>
<p style="background-color: #0000ff;">这是一个段落。</p>

<button>返回第一个 p 元素的 background-color </button>
```

### 设置 CSS 属性

&emsp;如需设置指定的 CSS 属性，请使用如下语法：

```javascript
css("propertyname","value");
```

&emsp;下面的例子将为所有匹配元素设置 background-color 值：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("p").css("background-color", "yellow");
        });
    });
</script>

<h2>这是一个标题</h2>
<p style="background-color: #ff0000;">这是一个段落。</p>
<p style="background-color: #00ff00;">这是一个段落。</p>
<p style="background-color: #0000ff;">这是一个段落。</p>
<p>这是一个段落。</p>

<button>设置 p 元素的 background-color </button>
```

### 设置多个 CSS 属性

&emsp;如需设置多个 CSS 属性，请使用如下语法：

```javascript
css({"propertyname":"value","propertyname":"value",...});
```

&emsp;下面的例子将为所有匹配元素设置 background-color 和 font-size：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            $("p").css({
                "background-color": "yellow",
                "font-size": "200%"
            });
        });
    });
</script>

<h2>这是一个标题</h2>
<p style="background-color: #ff0000;">这是一个段落。</p>
<p style="background-color: #00ff00;">这是一个段落。</p>
<p style="background-color: #0000ff;">这是一个段落。</p>
<p>这是一个段落。</p>

<button>为 p 元素设置多个样式 </button>
```

## jQuery 尺寸

&emsp;通过 jQuery，很容易处理元素和浏览器窗口的尺寸。

### jQuery 尺寸方法

&emsp;jQuery 提供多个处理尺寸的重要方法：

+ width()
+ height()
+ innerWidth()
+ innerHeight()
+ outerWidth()
+ outerHeight()

### jQuery 尺寸


### jQuery width() 和 height() 方法

&emsp;width() 方法设置或返回元素的宽度（不包括内边距、边框或外边距）。height() 方法设置或返回元素的高度（不包括内边距、边框或外边距）。下面的例子返回指定的 `<div>` 元素的宽度和高度：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            var txt = "";
            txt += "div 的宽度是：" + $("#div1").width() + "</br>";
            txt += "div 的高度是：" + $("#div1").height();
            $("#div1").html(txt);
        });
    });
</script>

<div id="div1" style="height: 100px;width: 300px;padding: 10px;margin: 3px;border: 2px solid blue;background-color: lightblue;"></div>
<br>
<button>显示 div 元素的尺寸</button>
<p>width() - 返回元素的宽度</p>
<p>height() - 返回元素的高度</p>
```

### jQuery innerWidth() 和 innerHeight() 方法

&emsp;innerWidth() 方法返回元素的宽度（包括内边距）。innerHeight() 方法返回元素的高度（包括内边距）。下面的例子返回指定的 `<div>` 元素的 inner-width/height：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            var txt = "";
            txt += "div 的宽度是：" + $("#div1").width() + "</br>";
            txt += "div 的高度是：" + $("#div1").height() + "</br>";
            txt += "div 宽度，包含内边距：" + $("#div1").innerWidth() + "</br>";
            txt += "div 高度，包含内边距：" + $("#div1").innerHeight() + "</br>";
            $("#div1").html(txt);
        });
    });
</script>

<div id="div1" style="height: 100px;width: 300px;padding: 10px;margin: 3px;border: 1px solid blue;background-color: lightblue;"></div>
<br>
<button>显示 div 元素的尺寸</button>
<p>innerWidth() - 返回元素的宽度 (包含内边距)。</p>
<p>innerHeight() - 返回元素的高度 (包含内边距)。</p>
```

### jQuery outerWidth() 和 outerHeight() 方法

&emsp;outerWidth() 方法返回元素的宽度（包括内边距和边框）。outerHeight() 方法返回元素的高度（包括内边距和边框）。下面的例子返回指定的 `<div>` 元素的 outer-width/height：

```javascript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("button").click(function() {
            var txt = "";
            txt += "div 的宽度是：" + $("#div1").width() + "</br>";
            txt += "div 的高度是：" + $("#div1").height() + "</br>";
            txt += "div 宽度，包含内边距和边框：" + $("#div1").outerWidth() + "</br>";
            txt += "div 高度，包含内边距和边框：" + $("#div1").outerHeight() + "</br>";
            $("#div1").html(txt);
        });
    });
</script>

<div id="div1" style="height: 100px;width: 300px;padding: 10px;margin: 3px;border: 1px solid blue;background-color: lightblue;"></div>
<br>
<button>显示 div 元素的尺寸</button>
<p>outerWidth() - 返回元素的宽度 (包含内边距和边框)。</p>
<p>outerHeight() - 返回元素的高度 (包含内边距和边框)。</p>
```

&emsp;唯一需要注意的地方，设置了 box-sizing 后，width() 获取的是 css 设置的 width 减去 padding 和 border 的值。

## jQuery 遍历

&emsp;什么是遍历？

&emsp;jQuery 遍历，意为 "移动"，用于根据其相对于其他元素的关系来 "查找"（或选取）HTML 元素。以某项选择开始，并沿着这个选择移动，直到抵达你期望的元素为止。下图展示了一个家族树。通过 jQuery 遍历，能够从被选（当前的）元素开始，轻松地在家族树中向上移动（祖先），向下移动（子孙），水平移动（同胞）。这种移动被称为对 DOM 进行遍历。


+ `<div>` 元素是 `<ul>` 的父元素，同时是其中所有内容的祖先。
+ `<ul>` 元素是 `<li>` 元素的父元素，同时是 `<div>` 的子元素。
+ 左边的 `<li>` 元素是 `<span>` 的父元素，`<ul>` 的子元素，同时是 `<div>` 的后代。
+ `<span>` 元素是 `<li>` 的子元素，同时是 `<ul>` 和 `<div>` 的后代。
+ 两个 `<li>` 元素是同胞（拥有相同的父元素）。
+ 右边的 `<li>` 元素是 `<b>` 的父元素，`<ul>` 的子元素，同时是 `<div>` 的后代。
+ `<b>` 元素是右边的 `<li>` 的子元素，同时是 `<ul>` 和 `<div>` 的后代。

### 遍历 DOM

&emsp;jQuery 提供了多种遍历 DOM 的方法。遍历方法中最大的种类是树遍历（tree-traversal）。

## jQuery 遍历 - 祖先

&emsp;祖先是父、祖父或曾祖父等等。通过 jQuery，你能够向上遍历 DOM 树，以查找元素的祖先。

### 向上遍历 DOM 树

&emsp;这些 jQuery 方法很有用，它们用于向上遍历 DOM 树：

+ parent()
+ parents()
+ parentsUntil()

### jQuery parent() 方法

&emsp;parent() 方法返回被选元素的直接父元素。该方法只会向上一级对 DOM 树进行遍历。下面的例子返回每个 `<span>` 元素的直接父元素：

```javascript
<style>
    .ancestors * {
        display: block;
        border: 2px solid lightgrey;
        color: lightgrey;
        padding: 5px;
        margin: 15px;
    }
</style>
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("span").parent().css({
            "color": "red",
            "border": "2px solid red"
        });
    });
</script>

<div class="ancestors">
    <div style="width: 500px;">div（曾祖父元素）
        <ul>ul（祖父元素）
            <li>li（父元素）
                <span>span</span>
            </li>
        </ul>
    </div>

    <div style="width: 500px;">div（祖父元素）
        <p>p（父元素）
            <span>span</span>
        </p>
    </div>
</div>
```

### jQuery parents() 方法

&emsp;parents() 方法返回被选元素的所有祖先元素，它一路向上直到文档的根元素 (`<html>`)。下面的例子返回所有 `<span>` 元素的所有祖先：

```javascript
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>

    <style>
        .ancestors * {
            display: block;
            border: 2px solid lightgrey;
            color: lightgrey;
            padding: 5px;
            margin: 15px;
        }
    </style>
    <script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
    <script>
        $(document).ready(function() {
            $("span").parents().css({
                "color": "red",
                "border": "2px solid red"
            });
        });
    </script>
</head>

<body class="ancestors">body（曾曾祖父元素）
    <div style="width: 500px;">div（曾祖父元素）
        <ul>ul（祖父元素）
            <li>li（父元素）
                <span>span</span>
            </li>
        </ul>
    </div>
</body>

</html>
```

&emsp;你也可以使用可选参数来过滤对祖先元素的搜索。下面的例子返回所有 `<span>` 元素的所有祖先，并且它是 `<ul>` 元素：

```javascript
$(document).ready(function(){
  $("span").parents("ul").css({"color":"red","border":"2px solid red"});
});
```

### jQuery parentsUntil() 方法

&emsp;parentsUntil() 方法返回介于两个给定元素之间的所有祖先元素。下面的例子返回介于 `<span>` 与 `<div>` 元素之间的所有祖先元素：

```javascript
$(document).ready(function(){
  $("span").parentsUntil("div").css({"color":"red","border":"2px solid red"});
});
```

## jQuery 遍历 - 后代

&emsp;后代是子、孙、曾孙等等。通过 jQuery，你能够向下遍历 DOM 树，以查找元素的后代。

### 向下遍历 DOM 树

&emsp;下面是两个用于向下遍历 DOM 树的 jQuery 方法：

+ children()
+ find()

### jQuery children() 方法

&emsp;children() 方法返回被选元素的 **所有** 直接子元素。该方法只会向下一级对 DOM 树进行遍历。下面的例子返回每个 `<div>` 元素的所有直接子元素：

```javascript
<style>
    .descendants * {
        display: block;
        border: 2px solid lightgrey;
        color: lightgrey;
        padding: 5px;
        margin: 15px;
    }
</style>
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("div").children().css({
            "color": "red",
            "border": "2px solid red"
        });
    });
</script>
<div class="descendants" style="width: 500px;">div（当前元素）
    <p>p（儿子元素）
        <span>span（孙子元素）</span>
    </p>
    <p>p（儿子元素）
        <span>span（孙子元素）</span>
    </p>
</div>
```

&emsp;也可以使用可选参数来过滤对子元素的搜索。下面的例子返回类名为 "1" 的所有 `<p>` 元素，并且它们是 `<div>` 的直接子元素：

```javascript
$(document).ready(function(){
  $("div").children("p.1");
});
```

### jQuery find() 方法

&emsp;find() 方法返回被选元素的后代元素，一路向下直到最后一个后代。下面的例子返回属于 `<div>` 后代的所有 `<span>` 元素：

```javascript
$(document).ready(function(){
  $("div").find("span").css({"color":"red","border":"2px solid red"});
});
```

&emsp;使用 `*` 则是返回 `<div>` 的所有后代：

```javascript
$(document).ready(function(){
  $("div").find("*").css({"color":"red","border":"2px solid red"});
});
```

## jQuery 遍历 - 同胞(siblings)

&emsp;同胞拥有相同的父元素。通过 jQuery，能够在 DOM 树中遍历元素的同胞元素。

&emsp;在 DOM 树中水平遍历，有许多有用的方法让我们在 DOM 树进行水平遍历：

+ siblings()
+ next()
+ nextAll()
+ nextUntil()
+ prev()
+ prevAll()
+ prevUntil()

### jQuery siblings() 方法

&emsp;siblings() 方法返回被选元素的所有同胞元素。下面的例子返回 `<h2>` 的所有同胞元素：

```javascript
<style>
    .siblings * {
        display: block;
        border: 2px solid lightgrey;
        color: lightgrey;
        padding: 5px;
        margin: 15px;
    }
</style>
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("h2").siblings().css({
            "color": "red",
            "border": "2px solid red"
        });
    });
</script>

<div>div（父元素）
    <p>p</p>
    <span>span</span>
    <h2>h2</h2>
    <h3>h3</h3>
    <p>p</p>
</div>
```

&emsp;也可以使用可选参数来过滤对同胞元素的搜索。下面的例子返回属于 `<h2>` 的同胞元素的所有 `<p>` 元素：

```javascript
<script>
    $(document).ready(function() {
        $("h2").siblings("p").css({
            "color": "red",
            "border": "2px solid red"
        });
    });
</script>
```

### jQuery next() 方法

&emsp;next() 方法返回被选元素的下一个同胞元素。该方法只返回一个元素。下面的例子返回 `<h2>` 的下一个同胞元素：

```javascript
<script>
    $(document).ready(function() {
        $("h2").next().css({
            "color": "red",
            "border": "2px solid red"
        });
    });
</script>
```

### jQuery nextAll() 方法

&emsp;nextAll() 方法返回被选元素的所有跟随的同胞元素。下面的例子返回 `<h2>` 的所有跟随的同胞元素：

```javascript
<script>
    $(document).ready(function() {
        $("h2").nextAll().css({
            "color": "red",
            "border": "2px solid red"
        });
    });
</script>
```

### jQuery nextUntil() 方法

&emsp;nextUntil() 方法返回介于两个给定参数之间的所有跟随的同胞元素。下面的例子返回介于 `<h2>` 与 `<h6>` 元素之间的所有同胞元素：

```javascript
$(document).ready(function() {
    $("h2").nextUntil("h6").css({
        "color": "red",
        "border": "2px solid red"
    });
});
```

### jQuery prev(), prevAll() & prevUntil() 方法

&emsp;prev(), prevAll() 以及 prevUntil() 方法的工作方式与上面的方法类似，只不过方向相反而已：它们返回的是前面的同胞元素（在 DOM 树中沿着同胞之前元素遍历，而不是之后元素遍历）。

## jQuery 遍历- 过滤

&emsp;缩小搜索元素的范围。

&emsp;三个最基本的过滤方法是：first(), last() 和 eq()，它们允许基于其在一组元素中的位置来选择一个特定的元素。其他过滤方法，比如 filter() 和 not() 允许选取匹配或不匹配某项指定标准的元素。

### jQuery first() 方法

&emsp;first() 方法返回被选元素的首个元素。下面的例子选取首个 `<div>` 元素内部的第一个 `<p>` 元素：

```javascript
$(document).ready(function() {
    $("div p").first().css("background-color", "yellow");
});
```

### jQuery last() 方法

&emsp;last() 方法返回被选元素的最后一个元素。下面的例子选择最后一个 `<div>` 元素中的最后一个 `<p>` 元素：

```javascript
$(document).ready(function() {
    $("div p").last().css("background-color", "yellow");
});
```

### jQuery eq() 方法

&emsp;eq() 方法返回被选元素中带有指定索引号的元素。索引号从 0 开始，因此首个元素的索引号是 0 而不是 1。下面的例子选取第二个 `<p>` 元素（索引号 1）：

```javascript
$(document).ready(function() {
    $("p").eq(1).css("background-color", "yellow");
});
```

### jQuery filter() 方法

&emsp;filter() 方法允许你规定一个标准。不匹配这个标准的元素会被从集合中删除，匹配的元素会被返回。下面的例子返回带有类名 "url" 的所有 `<p>` 元素：

```javascript
$(document).ready(function() {
    $("p").filter(".url").css("background-color", "yellow");
});
```

### jQuery not() 方法

&emsp;not() 方法返回不匹配标准的所有元素。提示：not() 方法与 filter() 相反。下面的例子返回不带有类名 "url" 的所有 `<p>` 元素：

```javascript
$(document).ready(function() {
    $("p").not(".url").css("background-color", "yellow");
});
```

&emsp;`$("p").filter(".url").css("background-color","yellow");` 和 `$("p.url").css("background-color","yellow");` 效果一样。

&emsp;not 和 eq 可以实现反选的效果。选取索引值不为 1 的 p 元素，并把背景颜色设置为黄色：

```javascript
$("p").not(":eq(1)").css("background-color","yellow");
```

&emsp;`$("p").last().css("background-color","yellow");` 和 `$("p:last").css("background-color","yellow");` 效果一样。

&emsp;`$("p").first().css("background-color","yellow");` 和 `$("p:first").ss("background-color","yellow");` 效果一样。

&emsp;`$("p").not(".url");` 和 `$("p:not(.url)");` 效果一样。

## jQuery - AJAX 简介

&emsp;














## 参考链接
**参考链接:🔗**
+ [jQuery 教程](https://www.runoob.com/jquery/jquery-tutorial.html)

