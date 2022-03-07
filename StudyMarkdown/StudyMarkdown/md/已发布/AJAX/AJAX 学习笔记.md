# AJAX 学习笔记

&emsp;AJAX = Asynchronous JavaScript and XML（异步的 JavaScript 和 XML）。

&emsp;AJAX 是一种用于创建快速动态网页的技术。AJAX 最大的优点是在不重新加载整个页面的情况下，可以与服务器交换数据并更新部分网页内容，通过在后台与服务器进行少量数据交换，AJAX 可以使网页实现异步更新。这意味着可以在不重新加载整个网页的情况下，对网页的某部分进行更新。（传统的网页（不使用 AJAX）如果需要更新内容，必需重载整个网页面。）

&emsp;AJAX 不需要任何浏览器插件，但需要用户允许 JavaScript 在浏览器上执行。

## AJAX 应用

+ 运用 XHTML+CSS 来表达资讯
+ 运用 JavaScript 操作 DOM（Document Object Model）来执行动态效果
+ 运用 XML 和 XSLT 操作资料
+ 运用 XMLHttpRequest 或新的 Fetch API 与网页服务器进行异步资料交换

> &emsp;注意：AJAX 与 Flash、Silverlight 和 Java Applet 等 RIA 技术是有区分的。

## AJAX 是基于现有的 Internet 标准

&emsp;AJAX 是基于现有的 Internet 标准，并且联合使用它们：

+ XMLHttpRequest 对象 (异步的与服务器交换数据)
+ JavaScript/DOM (信息显示/交互)
+ CSS (给数据定义样式)
+ XML (作为转换数据的格式)

> &emsp;AJAX应用程序与浏览器和平台无关的！

## Google Suggest

&emsp;在 2005 年，Google 通过其 Google Suggest 使 AJAX 变得流行起来。Google Suggest 使用 AJAX 创造出动态性极强的 web 界面：当在谷歌的搜索框输入关键字时，JavaScript 会把这些字符发送到服务器，然后服务器会返回一个搜索建议的列表。

```c++
<!DOCTYPE html>
<html>

<head>
    <meta charset="utf-8">
    <script>
        function loadXMLDoc() {
            var xmlhttp;
            if (window.XMLHttpRequest) {
                //  IE7+, Firefox, Chrome, Opera, Safari 浏览器执行代码
                xmlhttp = new XMLHttpRequest();
            } else {
                // IE6, IE5 浏览器执行代码
                xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
            }
            xmlhttp.onreadystatechange = function() {
                if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                    document.getElementById("myDiv").innerHTML = xmlhttp.responseText;
                }
            }
            xmlhttp.open("GET", "/try/ajax/ajax_info.txt", true);
            xmlhttp.send();
        }
    </script>
</head>

<body>

    <div id="myDiv">
        <h2>使用 AJAX 修改该文本内容</h2>
    </div>
    <button type="button" onclick="loadXMLDoc()">修改内容</button>

</body>

</html>
```

## AJAX - 创建 XMLHttpRequest 对象

&emsp;`XMLHttpRequest` 是 AJAX 的基础，所有现代浏览器均支持 `XMLHttpRequest` 对象（IE5 和 IE6 使用 `ActiveXObject`）。`XMLHttpRequest` 用于在后台与服务器交换数据。这意味着可以在不重新加载整个网页的情况下，对网页的某部分进行更新。

&emsp;所有现代浏览器（IE 7+、Firefox、Chrome、Safari 以及 Opera）均内建 `XMLHttpRequest` 对象。

&emsp;创建 `XMLHttpRequest` 对象的语法：

```c++
variable = new XMLHttpRequest();
```

&emsp;老版本的 Internet Explorer （IE 5 和 IE 6）使用 ActiveX 对象：

```c++
variable = new ActiveXObject("Microsoft.XMLHTTP");
```

&emsp;为了应对所有的现代浏览器，包括 IE 5 和 IE 6，请检查浏览器是否支持 `XMLHttpRequest` 对象。如果支持，则创建 `XMLHttpRequest` 对象。如果不支持，则创建 `ActiveXObject`：

```c++
var xmlhttp;
if (window.XMLHttpRequest) {
    // IE 7+, Firefox, Chrome, Opera, Safari 浏览器执行代码
    xmlhttp = new XMLHttpRequest();
} else {
    // IE 6, IE 5 浏览器执行代码
    xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
}
```

## AJAX - 向服务器发送请求

&emsp;`XMLHttpRequest` 对象用于和服务器交换数据。

### 向服务器发送请求

&emsp;如需将请求发送到服务器，我们使用 `XMLHttpRequest` 对象的 `open()` 和 `send()` 方法：

```c++
xmlhttp.open("GET", "ajax_info.txt", true);
xmlhttp.send();
```

&emsp;`open(method, url, async)`：规定请求的类型、URL 以及是否异步处理请求。

+ method：请求的类型；GET 或 POST
+ url：文件在服务器上的位置
+ async：true（异步）或 false（同步）

&emsp;`send(string)`：将请求发送到服务器。

+ string：仅用于 POST 请求

## GET 还是 POST？

&emsp;与 POST 相比，GET 更简单也更快，并且在大部分情况下都能用。然而，在以下情况中，请使用 POST 请求：

+ 不愿使用缓存文件（更新服务器上的文件或数据库）
+ 向服务器发送大量数据（POST 没有数据量限制）
+ 发送包含未知字符的用户输入时，POST 比 GET 更稳定也更可靠

### GET 请求

&emsp;一个简单的 GET 请求：

```c++
xmlhttp.open("GET", "/try/ajax/demo_get.php", true);
xmlhttp.send();
```

&emsp;在上面的例子中，可能得到的是缓存的结果。为了避免这种情况，请向 URL 添加一个唯一的 ID：

```c++
xmlhttp.open("GET", "/try/ajax/demo_get.php?t=" + Math.random(), true);
xmlhttp.send();
```

&emsp;如果希望通过 GET 方法发送信息，请向 URL 添加信息（参数）：

```c++
xmlhttp.open("GET", "/try/ajax/demo_get2.php?fname=Henry&lname=Ford", true);
xmlhttp.send();
```

### POST 请求

&emsp;一个简单的 POST 请求：

```c++
xmlhttp.open("POST", "/try/ajax/demo_post.php", true);
xmlhttp.send();
```

&emsp;如果需要像 HTML 表单那样 POST 数据，可使用 `setRequestHeader()` 来添加 HTTP 头。然后在 `send()` 方法中规定希望发送的数据：

```c++
xmlhttp.open("POST", "/try/ajax/demo_post2.php", true);
xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
xmlhttp.send("fname=Henry&lname=Ford");
```

&emsp;`setRequestHeader(header, value)`：方法用于向请求添加 HTTP 头。

+ header: 规定头的名称
+ value: 规定头的值

### url - 服务器上的文件

&emsp;`open()` 方法的 `url` 参数是服务器上文件的地址：

```c++
xmlhttp.open("GET", "ajax_test.html", true);
```

&emsp;该文件可以是任何类型的文件，比如 .txt 和 .xml，或者服务器脚本文件，比如 .asp 和 .php（在传回响应之前，能够在服务器上执行任务）。

### 异步 - True 或 False？

&emsp;AJAX 指的是异步 JavaScript 和 XML（Asynchronous JavaScript and XML）。

&emsp;`XMLHttpRequest` 对象如果要用于 AJAX 的话，其 `open()` 方法的 `async` 参数必须设置为 `true`：

```c++
xmlhttp.open("GET", "ajax_test.html", true);
```

&emsp;对于 web 开发人员来说，发送异步请求是一个巨大的进步。很多在服务器执行的任务都相当费时。AJAX 出现之前，这可能会引起应用程序挂起或停止。通过 AJAX，JavaScript 无需等待服务器的响应，而是：

+ 在等待服务器响应时执行其他脚本
+ 当响应就绪后对响应进行处理

### Async=true

&emsp;当使用 `async=true` 时，请规定在响应处于 `onreadystatechange` 事件中的就绪状态时执行的函数：

```c++
xmlhttp.onreadystatechange = function() {
    if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
        document.getElementById("myDiv").innerHTML = xmlhttp.responseText;
    }
}

xmlhttp.open("GET", "/try/ajax/ajax_info.txt", true);
xmlhttp.send();
```

### Async = false

&emsp;如需使用 `async=false`，请将 `open()` 方法中的第三个参数改为 `false`：

```c++
xmlhttp.open("GET", "test1.txt", false);
```

&emsp;不推荐使用 `async=false`，但是对于一些小型的请求，也是可以的。请记住，JavaScript 会等到服务器响应就绪才继续执行。如果服务器繁忙或缓慢，应用程序会挂起或停止。

> &emsp;当使用 `async=false` 时，请不要编写 `onreadystatechange` 函数 - 把代码放到 `send()` 语句后面即可：

```c++
xmlhttp.open("GET", "/try/ajax/ajax_info.txt", false);
xmlhttp.send();
document.getElementById("myDiv").innerHTML = xmlhttp.responseText;
```

## AJAX - 服务器 响应

&emsp;如需获得来自服务器的响应，请使用 `XMLHttpRequest` 对象的 `responseText` 或 `responseXML` 属性。

+ responseText 获得字符串形式的响应数据。
+ responseXML 获得 XML 形式的响应数据。

### responseText 属性

&emsp;如果来自服务器的响应并非 XML，请使用 `responseText` 属性。`responseText` 属性返回字符串形式的响应，因此可以这样使用：

```c++
document.getElementById("myDiv").innerHTML = xmlhttp.responseText;
```

### responseXML 属性

&emsp;如果来自服务器的响应是 XML，而且需要作为 XML 对象进行解析，请使用 `responseXML` 属性：

```c++
xmlhttp.onreadystatechange = function() {
    if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
        xmlDoc = xmlhttp.responseXML;
        txt = "";
        x = xmlDoc.getElementsByTagName("ARTIST");
        for (i = 0; i < x.length; i++) {
            txt = txt + x[i].childNodes[0].nodeValue + "<br>";
        }
        document.getElementById("myDiv").innerHTML = txt;
    }
}
xmlhttp.open("GET", "cd_catalog.xml", true);
xmlhttp.send();
```

## AJAX - onreadystatechange 事件

&emsp;`onreadystatechange` 事件：当请求被发送到服务器时，我们需要执行一些基于响应的任务。每当 `readyState` 改变时，就会触发 `onreadystatechange` 事件。`readyState` 属性存有 `XMLHttpRequest` 的状态信息。

&emsp;下面是 `XMLHttpRequest` 对象的三个重要的属性：

1. `onreadystatechange` 存储函数（或函数名），每当 `readyState` 属性改变时，就会调用该函数。
2. `readyState` 存有 `XMLHttpRequest` 的状态。从 0 到 4 发生变化。

  + 0: 请求未初始化
  + 1: 服务器连接已建立
  + 2: 请求已接收
  + 3: 请求处理中
  + 4: 请求已完成，且响应已就绪

3. `status` 200: "OK" 404: 未找到页面

&emsp;在 `onreadystatechange` 事件中，我们规定当服务器响应已做好被处理的准备时所执行的任务。当 `readyState` 等于 4 且状态为 200 时，表示响应已就绪：

```c++
xmlhttp.onreadystatechange = function() {
    if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
        document.getElementById("myDiv").innerHTML = xmlhttp.responseText;
    }
}
```

> &emsp;`onreadystatechange` 事件被触发 4 次（0 - 4）, 分别是：0-1、1-2、2-3、3-4，对应着 `readyState` 的每个变化。

### 使用回调函数

&emsp;回调函数是一种以参数形式传递给另一个函数的函数。如果网站上存在多个 AJAX 任务，那么应该为创建 `XMLHttpRequest` 对象编写一个标准的函数，并为每个 AJAX 任务调用该函数。该函数调用应该包含 `URL` 以及发生 `onreadystatechange` 事件时执行的任务（每次调用可能不尽相同）：

```c++
<script>
    var xmlhttp;

    function loadXMLDoc(url, cfunc) {
        if (window.XMLHttpRequest) {
            // IE7+, Firefox, Chrome, Opera, Safari 代码
            xmlhttp = new XMLHttpRequest();
        } else {
            // IE6, IE5 代码
            xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
        }
        xmlhttp.onreadystatechange = cfunc;
        xmlhttp.open("GET", url, true);
        xmlhttp.send();
    }

    function myFunction() {
        loadXMLDoc("/try/ajax/ajax_info.txt", function() {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                document.getElementById("myDiv").innerHTML = xmlhttp.responseText;
            }
        });
    }
</script>
```

&emsp;服务器常用的状态码及其对应的含义如下：

+ 200：服务器响应正常。
+ 304：该资源在上次请求之后没有任何修改（这通常用于浏览器的缓存机制，使用 GET 请求时尤其需要注意）。
+ 400：无法找到请求的资源。
+ 401：访问资源的权限不够。
+ 403：没有权限访问资源。
+ 404：需要访问的资源不存在。
+ 405：需要访问的资源被禁止。
+ 407：访问的资源需要代理身份验证。
+ 414：请求的 URL 太长。
+ 500：服务器内部错误。

### 对 onreadystatechange 属性的理解

&emsp;[对 onreadystatechange 属性的理解](https://www.runoob.com/w3cnote/onreadystatechange-attribute-learn.html)

## AJAX XML 实例

&emsp;AJAX 可用来与 XML 文件进行交互式通信。

&emsp;当服务器响应就绪时，会构建一个 HTML 表格，从 XML 文件中提取节点（元素），最后使用 XML 数据的 填充 id="demo" 的表格元素：

```c++
function myFunction(xml) {
    var i;
    var xmlDoc = xml.responseXML;
    var table = "<tr><th>Artist</th><th>Title</th></tr>";
    var x = xmlDoc.getElementsByTagName("CD");
    for (i = 0; i < x.length; i++) {
        table += "<tr><td>" +
            x[i].getElementsByTagName("ARTIST")[0].childNodes[0].nodeValue +
            "</td><td>" +
            x[i].getElementsByTagName("TITLE")[0].childNodes[0].nodeValue +
            "</td></tr>";
    }
    document.getElementById("demo").innerHTML = table;
}
```

## AJAX JSON 实例

&emsp;AJAX 可用来与 JSON 文件进行交互式通信。

&emsp;当服务器响应就绪时，使用 `JSON.parse()` 方法将数据转换为 JavaScript 对象：

```c++
<script>
    function loadXMLDoc() {
        var xmlhttp;
        if (window.XMLHttpRequest) {
            // IE7+, Firefox, Chrome, Opera, Safari 浏览器执行代码
            xmlhttp = new XMLHttpRequest();
        } else {
            // IE6, IE5 浏览器执行代码
            xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
        }
        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                var myArr = JSON.parse(this.responseText);
                myFunction(myArr)
            }
        }
        xmlhttp.open("GET", "/try/ajax/json_ajax.json", true);
        xmlhttp.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
        xmlhttp.send();
    }

    function myFunction(arr) {
        var out = "";
        var i;
        for (i = 0; i < arr.length; i++) {
            out += '<a href="' + arr[i].url + '">' +
                arr[i].title + '</a><br>';
        }
        document.getElementById("myDiv").innerHTML = out;
    }
</script>
```

## 参考链接
**参考链接:🔗**
+ [AJAX 教程](https://www.runoob.com/ajax/ajax-tutorial.html)
+ [对 onreadystatechange 属性的理解](https://www.runoob.com/w3cnote/onreadystatechange-attribute-learn.html)

