# JavaScript HTML DOM 学习笔记

&emsp;通过 HTML DOM，可以访问 JavaScript HTML 文档的所有元素。

## HTML DOM（文档对象模型）

&emsp;当网页被加载时，浏览器会创建页面的文档对象模型（Document Object Model）。HTML DOM 模型被构造为对象的树。

&emsp;通过可编程的对象模型，JavaScript 获得了足够的能力来创建动态的 HTML。

+ JavaScript 能够改变页面中的所有 HTML 元素。
+ JavaScript 能够改变页面中的所有 HTML 属性。
+ JavaScript 能够改变页面中的所有 CSS 样式。
+ JavaScript 能够对页面中的所有事件做出反应。

### 查找 HTML 元素

+ 通过 `id` 找到 HTML 元素。
+ 通过标签名找到 HTML 元素。
+ 通过类名找到 HTML 元素。

#### 通过标签名查找 HTML 元素

&emsp;通过标签名查找 HTML 元素：查找 `id = "main"` 的元素，然后查找 `id = "main"` 元素中的所有 `<p>` 元素：

```c++
<div id="main">
    <p>DOM 是非常有用的。</p>
    <p>该实例展示了 <b>getElementByTagName</b> 方法</p>
</div>
<script>
    var x = document.getElementById("main");
    var y = x.getElementsByTagName("p");
    
    console.log(y); // HTMLCollection(2)
    console.log(Array.prototype.slice.call(y)); // Array(2)
    console.log(Array.prototype.slice.apply(y)); // Array(2)
    console.log(Array.prototype.slice.bind(y)()); // Array(2)

    document.write('id = "main" 元素中的第一个段落为：' + y[0].innerHTML);
</script>
```

#### 通过类名找到 HTML 元素

&emsp;通过 `getElementsByClassName` 函数来查找 `class="intro"` 的元素：

```c++
<p class="intro">你好世界！</p>
<p>该实例展示了 <b>getElementsByClassName</b> 方法！</p>
<script>
    x = document.getElementsByClassName("intro");
    document.write("<p>文本来自 class 为 intro 段落：" + x[0].innerHTML + "</p>");
</script>
```

&emsp;`getElementsByTagName` 和 `getElementsByClassName` 这两个方法查找多个 DOM 元素，返回的是 `HTMLCollection` 类型，是伪数组而不是真数组，故不能使用数组的方法。

&emsp;可以使用数组原型配合 `slice` 方法，利用 `call`、`apply`、`bind` 方法将伪数组转为真数组。

## JavaScript HTML DOM - 改变 HTML

+ 改变 HTML 输出流
+ 改变 HTML 内容
+ 改变 HTML 属性

## JavaScript HTML DOM - 改变 CSS

+ 改变 HTML 样式

```c++
document.getElementById(id).style.property = 新样式;
```

```c++
<p id="p1">Hello World!</p>
<p id="p2">Hello World!</p>

<script>
    document.getElementById("p2").style.color = "blue";
    document.getElementById("p2").style.fontFamily = "Arial";
    document.getElementById("p2").style.fontSize = "larger";
</script>
```

+ 使用事件

&emsp;HTML DOM 允许通过触发事件来执行代码。

```c++
<h1 id="id1">我的标题 1</h1>
<button type="button" onclick="document.getElementById('id1').style.color = 'red'">点我，标题 1 会变红色！</button>
```

## JavaScript HTML DOM 事件

### 对事件做出反应

&emsp;可以在事件发生时执行 JavaScript，比如当用户在 HTML 元素上点击时。

&emsp;`h1` 标签也可以添加 `onclick` 点击事件。

```c++
<h1 onclick="this.innerHTML='Ooops!'">点击文本！</h1>
```

### HTML 事件属性

&emsp;如需向 HTML 元素分配事件，可以使用事件属性。

&emsp;向 button 元素分配 `onclick` 事件。

```c++
<p>点击按钮执行 <em>displayDate()</em> 函数。</p>
<button onclick="displayDate()">点这里</button>
<script>
    function displayDate() {
        document.getElementById("demo").innerHTML = Date();
    }
</script>
<p id="demo"></p>
```
### 使用 HTML DOM 来分配事件

&emsp;HTML DOM 允许使用 JavaScript 来向 HTML 元素分配事件。

```c++
<p>点击按钮执行 <em>displayDate()</em> 函数。</p>
<button id="myBtn">点这里</button>
<script>
    document.getElementById("myBtn").onclick = function() {
        displayDate()
    };

    function displayDate() {
        document.getElementById("demo").innerHTML = Date();
    }
</script>
<p id="demo"></p>
```

### onload 和 onunload 事件

&emsp;`onload` 和 `onunload` 事件会在用户进入或离开页面时被触发。

&emsp;`onload` 事件可用于检测访问者的浏览器类型和浏览器版本，并基于这些信息来加载网页的正确版本。

&emsp;`onload` 和 `onunload` 事件可用于处理 cookie。

```c++
<body onload="checkCookies()">
    <script>
        function checkCookies() {
            if (navigator.cookieEnabled == true) {
                alert("Cookies 可用");
            } else {
                alert("Cookies 不可用");
            }
        }
    </script>
    <p>弹窗-提示浏览器 cookies 是否可用。</p>
</body>
```

### onchange 事件

&emsp;`onchange` 事件常结合对输入字段的验证来使用。

```c++
<script>
    function myFunction() {
        var x = document.getElementById("fname");
        x.value = x.value.toUpperCase();
    }
</script>
<input type="text" id="fname" onchange="myFunction()">
```

### onmouseover 和 onmouseout 事件

&emsp;`onmouseover` 和 `onmouseout` 事件可用于在鼠标移至 HTML 元素上方或移出元素时触发函数。

```c++
<div onmouseover="mOver(this)" onmouseout="mOut(this)" style="background-color:#D94A38;width: 120px;height: 20px;padding: 40px;">Mouse Over Me</div>
<script>
    function mOver(obj) {
        obj.innerHTML = "Thank You";
    }

    function mOut(obj) {
        obj.innerHTML = "Mouse Over Me";
    }
</script>
```

### onmousedown、onmouseup 以及 onclick 事件

&emsp;`onmousedown`、`onmouseup` 以及 `onclick` 构成了鼠标点击事件的所有部分。当点击鼠标按钮时，会触发 `onmousedown` 事件，当释放鼠标按钮时，会触发 `onmouseup` 事件，最后，当完成鼠标点击时，会触发 `onclick` 事件。

## JavaScript HTML DOM EventListener


### addEventListener() 方法

&emsp;当点击按钮时触发监听事件：

```c++
<p>该实例使用 addEventListener() 方法在按钮中添加点击事件。</p>
<button id="myBtn">点我</button>
<p id="demo"></p>
<script>
    document.getElementById("myBtn").addEventListener("click", displayDate);
    document.getElementById("myBtn").addEventListener("click", changeColor);

    function displayDate() {
        document.getElementById("demo").innerHTML = Date();
    }

    function changeColor() {
        document.getElementById("demo").style.color = 'red';
    }
</script>
```

+ `addEventListener()` 方法用于向指定元素添加事件句柄。
+ `addEventListener()` 方法添加的事件句柄不会覆盖已存在的事件句柄。
+ 可以向一个元素添加多个事件句柄。
+ 可以向同个元素添加多个同类型的事件句柄，如：两个 "click" 事件。
+ 可以向任何 DOM 对象添加事件监听，不仅仅是 HTML 元素。如： window 对象。
+ `addEventListener()` 方法可以更简单的控制事件（冒泡与捕获）。
+ 当使用 `addEventListener()` 方法时，JavaScript 从 HTML 标记中分离开来，可读性更强，在没有控制 HTML 标记时也可以添加事件监听。
+ 可以使用 `removeEventListener()` 方法来移除事件的监听。

### 传递参数

&emsp;当传递参数值时，使用 `匿名函数` 调用带参数的函数：

```c++
var p1 = 5;
var p2 = 7;
document.getElementById("myBtn").addEventListener("click", function() {
    myFunction(p1, p2);
});

function myFunction(a, b) {
    var result = a * b;
    document.getElementById("demo").innerHTML = result;
}
```

### 事件冒泡或事件捕获？

&emsp;事件传递有两种方式：冒泡与捕获。

&emsp;事件传递定义了元素事件触发的顺序。 如果将 `<p>` 元素插入到 `<div>` 元素中，点击 `<p>` 元素，哪个元素的 `click` 事件先被触发呢？在 **冒泡** 中，内部元素的事件会先被触发，然后再触发外部元素，即：`<p>` 元素的点击事件先触发，然后会触发 `<div>` 元素的点击事件。在 **捕获** 中，外部元素的事件会先被触发，然后才会触发内部元素的事件，即：`<div>` 元素的点击事件先触发，然后再触发 `<p>` 元素的点击事件。

&emsp;`addEventListener()` 方法可以指定 `useCapture` 参数来设置传递类型：

```c++
addEventListener(event, function, useCapture);
```

&emsp;默认值为 `false`, 即冒泡传递，当值为 `true` 时, 事件使用捕获传递。

### removeEventListener() 方法

&emsp;`removeEventListener()` 方法移除由 `addEventListener()` 方法添加的事件句柄:

```c++
element.removeEventListener("mousemove", myFunction);
```

## JavaScript HTML DOM 元素（节点）

&emsp;如何向文档中添加和移除元素（节点）。

### 创建新的 HTML 元素（节点）-appendChild()

&emsp;要创建新的 HTML 元素 (节点)需要先创建一个元素，然后在已存在的元素中添加它。

```c++
<div id="div1">
    <p id="p1">这是一个段落。</p>
    <p id="p2">这是另外一个段落。</p>
</div>
<script>
    var para = document.createElement("p");
    var node = document.createTextNode("这是一个新的段落。");
    para.appendChild(node);

    var element = document.getElementById("div1");
    element.appendChild(para);
</script>
```

### 创建新的 HTML 元素 (节点) - insertBefore()

&emsp;`appendChild()` 方法，它用于添加新元素到尾部。如果需要将新元素添加到开始位置，可以使用 `insertBefore()` 方法:

```c++
<div id="div1">
    <p id="p1">这是一个段落。</p>
    <p id="p2">这是另外一个段落。</p>
</div>

<script>
    var para = document.createElement("p");
    var node = document.createTextNode("这是一个新的段落。");
    para.appendChild(node);

    var element = document.getElementById("div1");
    var child = document.getElementById("p1");
    
    // 把新元素插入到 p1 前面 
    element.insertBefore(para, child);
</script>
```

### 移除已存在的元素

&emsp;要移除一个元素，需要知道该元素的父元素。

```c++
<div id="div1">
    <p id="p1">这是一个段落。</p>
    <p id="p2">这是另外一个段落。</p>
</div>
 
<script>
    var parent = document.getElementById("div1");
    var child = document.getElementById("p1");
    parent.removeChild(child);
</script>
```

> &emsp;如果能够在不引用父元素的情况下删除某个元素，就太好了，不过很遗憾。DOM 需要清楚需要删除的元素，以及它的父元素。

&emsp;以下代码是已知要查找的子元素，然后查找其父元素，再删除这个子元素（删除节点必须知道父节点）：

```c++
var child = document.getElementById("p1");
child.parentNode.removeChild(child);
```

### 替换 HTML 元素 - replaceChild()

&emsp;可以使用 `replaceChild()` 方法来替换 HTML DOM 中的元素。

```c++
<div id="div1">
    <p id="p1">这是一个段落。</p>
    <p id="p2">这是另外一个段落。</p>
</div>
 
<script>
    var para = document.createElement("p");
    var node = document.createTextNode("这是一个新的段落。");
    para.appendChild(node);
 
    var parent = document.getElementById("div1");
    var child = document.getElementById("p1");
    parent.replaceChild(para, child);
</script>
```

## JavaScript HTML DOM 集合(Collection)

&emsp;DOM 集合。

### HTMLCollection 对象

&emsp;`getElementsByTagName()` 方法返回 `HTMLCollection` 对象。`HTMLCollection` 对象类似包含 HTML 元素的一个数组。

&emsp;`HTMLCollection` 对象的 `length` 属性定义了集合中元素的数量。

&emsp;修改所有 `<p>` 元素的背景颜色:

```c++
var myCollection = document.getElementsByTagName("p");
var i;
for (i = 0; i < myCollection.length; i++) {
    myCollection[i].style.backgroundColor = "red";
}
```

&emsp;`HTMLCollection` 不是一个数组！`HTMLCollection` 看起来可能是一个数组，但其实不是。它可以像数组一样，使用索引来获取元素。`HTMLCollection` 无法使用数组的方法：`valueOf()`、`pop()`、`push()` 或 `join()`。

## JavaScript HTML DOM 节点列表

&emsp;`NodeList` 对象是一个从文档中获取的节点列表（集合）。

&emsp;`NodeList` 对象类似 `HTMLCollection` 对象。一些旧版本浏览器中的方法（如：`getElementsByClassName()`）返回的是 `NodeList` 对象，而不是 `HTMLCollection` 对象。所有浏览器的 `childNodes` 属性返回的是 `NodeList` 对象。大部分浏览器的 `querySelectorAll()` 返回 `NodeList` 对象。

&emsp;修改节点列表中所有 `<p>` 元素的背景颜色:

```c++
var myNodelist = document.querySelectorAll("p");
var i;
for (i = 0; i < myNodelist.length; i++) {
    myNodelist[i].style.backgroundColor = "red";
}
```

### HTMLCollection 与 NodeList 的区别

&emsp;`HTMLCollection` 是 HTML 元素的集合。`NodeList` 是一个文档节点的集合。

&emsp;`NodeList` 与 `HTMLCollection` 有很多类似的地方。

+ `NodeList` 与 `HTMLCollection` 都与数组对象有点类似，可以使用索引 (0, 1, 2, 3, 4, ...) 来获取元素。
+ `NodeList` 与 `HTMLCollection` 都有 `length` 属性。
+ `HTMLCollection` 元素可以通过 `name`，`id` 或索引来获取。`NodeList` 只能通过索引来获取。
+ 只有 `NodeList` 对象有包含属性节点和文本节点。

&emsp;节点列表不是一个数组！节点列表看起来可能是一个数组，但其实不是。可以像数组一样，使用索引来获取元素。节点列表无法使用数组的方法：`valueOf()`、`pop()`、`push()` 或 `join()`。

```c++
var plist = document.querySelectorAll("p");
var pcoll = document.getElementsByTagName("p");
```
&emsp;以上 `plist` 返回的就是固定的值。而获取 `pcoll` 后, 若 HTML 页面有变化且刚好添加或移除了 p 标签, 此时 `pcoll` 也会跟着变。

&emsp;`querySelectorAll()` 和 `getElementsByTagName()` 两者的主要区别就是返回值。前者返回的是 `NodeList` 集合，后者返回的是 `HTMLCollection` 集合。其前者是一个静态集合，后者是一个动态集合。其中动态集合和静态集合的最大区别在于：动态集合指的就是元素集合会随着 DOM 树元素的增加而增加，减少而减少；静态集合则不会受 DOM 树元素变化的影响。

&emsp;使用 `getElementsByTagName` 方法我们得到的结果就像是一个对象的索引，而通过 `querySelectorAll` 方法我们得到的是一个对象的克隆；所以当这个对象数据量非常大的时候，显然克隆这个对象所需要花费的时间是很长的。

&emsp;`NodeList` 是 DOM 快照，节点数量和类型的快照，就是对节点增删，`NodeList` 感觉不到，但是对节点内部内容修改，是可以感觉到的，比如修改 `innerHTML`。`HTMLCollection 是 live 绑定的，节点的增删是敏感的。
