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
【测试服】
1. 合并 RN 的内容。
2. 修改直播结束时没有作业就不显示作业页面。
3. 修改直播回放结束时没有作业时只显示重播按钮。 
4. 修改录播课问答重复回复相同内容时的提示文案。
（感觉作业页面不管有没有作业都应该显示出来，提高此功能的曝光率。上午的处理方式是没有作业时：显示：班级作业 0/0 立即开始按钮灰色不能交互，这样遇到没有作业的情况也能暗示用户有作业这个功能，不然如果用户一直没见过作业页面，他估计很久一段时间都不知道有这个功能。）    

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













## 参考链接
**参考链接:🔗**
+ [jQuery 教程](https://www.runoob.com/jquery/jquery-tutorial.html)

