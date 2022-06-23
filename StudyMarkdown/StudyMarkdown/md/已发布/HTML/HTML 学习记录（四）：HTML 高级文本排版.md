# HTML 学习记录（四）：HTML 高级文本排版

&emsp;HTML 中还有许多没有在 HTML 文字处理基础页面中提到的其他元素可以用于格式化文本。这里的元素不太为人所知，但了解这些元素仍然很有用。在这篇文章里，你将学习如何标记引文、描述列表、计算机代码和其他类似的文本、下标和上标、联系信息等。

## 描述列表

&emsp;在 HTML 基础部分，我们讨论了如何在 HTML 中标记基本的列表，但是我们没有提到你偶尔会遇到的第三种类型的列表 — 描述列表 (description list)。这种列表的目的是标记一组项目及其相关描述，例如：术语和定义，或者是问题和答案等。让我们看一组术语和定义的示例：

&emsp;描述列表使用与其他列表类型不同的闭合标签 — `<dl>`，此外，每一项都用 `<dt>` (description term) 元素闭合。每个描述都用 `<dd>` (description definition) 元素闭合。让我们来完成下面的标记例子：

```javascript
<dl>
  <dt>内心独白</dt>
    <dd>戏剧中，某个角色对自己的内心活动或感受进行念白表演，这些台词只面向观众，而其他角色不会听到。</dd>
  <dt>语言独白</dt>
    <dd>戏剧中，某个角色把自己的想法直接进行念白表演，观众和其他角色都可以听到。</dd>
  <dt>旁白</dt>
    <dd>戏剧中，为渲染幽默或戏剧性效果而进行的场景之外的补充注释念白，只面向观众，内容一般都是角色的感受、想法、以及一些背景信息等。</dd>
</dl>
```

&emsp;浏览器的默认样式会在描述列表的描述部分（description definition）和描述术语（description terms）之间产生缩进。MDN 非常严密地遵循这一惯例，同时也鼓励关于术语的其他更多的定义（but also embolden the terms for extra definition）。

## 引用

&emsp;HTML 也有用于标记引用的特性，至于使用哪个元素标记，取决于你引用的是一块还是一行。

### 块引用

&emsp;如果一个块级内容（一个段落、多个段落、一个列表等）从其他地方被引用，你应该把它用 `<blockquote>` 元素包裹起来表示，并且在 cite 属性里用 URL 来指向引用的资源。例如，下面的例子就是引用的 MDN 的 `<blockquote>` 元素页面：

```javascript
<p>The <strong>HTML <code>&lt;blockquote&gt;</code> Element</strong> (or <em>HTML Block
Quotation Element</em>) indicates that the enclosed text is an extended quotation.</p>
```

&emsp;要把这些转换为块引用，我们要这样做：

```javascript
<blockquote cite="https://developer.mozilla.org/en-US/docs/Web/HTML/Element/blockquote">
  <p>The <strong>HTML <code>&lt;blockquote&gt;</code> Element</strong> (or <em>HTML Block
  Quotation Element</em>) indicates that the enclosed text is an extended quotation.</p>
</blockquote>
```

&emsp;浏览器在渲染块引用时默认会增加缩进，作为引用的一个指示符；MDN 是这样做的，但是也增加了额外的样式。

### 行内引用

&emsp;行内元素用同样的方式工作，除了使用 `<q>` 元素。例如，下面的标记包含了从 MDN `<q>` 页面的引用：

```javascript
<p>The quote element — <code>&lt;q&gt;</code> — is <q cite="https://developer.mozilla.org/en-US/docs/Web/HTML/Element/q">intended
for short quotations that don't require paragraph breaks.</q></p>
```

### 引文

&emsp;cite 属性内容不会被浏览器显示、屏幕阅读器阅读，需使用 JavaScript 或 CSS，浏览器才会显示 cite 的内容。如果你想要确保引用的来源在页面上是可显示的，更好的方法是为 `<cite>` 元素附上链接：

```javascript
<p>According to the <a href="https://developer.mozilla.org/en-US/docs/Web/HTML/Element/blockquote">
<cite>MDN blockquote page</cite></a>:
</p>

<blockquote cite="https://developer.mozilla.org/en-US/docs/Web/HTML/Element/blockquote">
  <p>The <strong>HTML <code>&lt;blockquote&gt;</code> Element</strong> (or <em>HTML Block
  Quotation Element</em>) indicates that the enclosed text is an extended quotation.</p>
</blockquote>

<p>The quote element — <code>&lt;q&gt;</code> — is <q cite="https://developer.mozilla.org/en-US/docs/Web/HTML/Element/q">intended
for short quotations that don't require paragraph breaks.</q> -- <a href="https://developer.mozilla.org/en-US/docs/Web/HTML/Element/q">
<cite>MDN q page</cite></a>.</p>
```

&emsp;引文默认的字体样式为斜体。你可以在 [quotations.html](https://github.com/mdn/learning-area/blob/main/html/introduction-to-html/advanced-text-formatting/quotations.html) 中参看代码。

## 缩略语

&emsp;另一个你在 web 上看到的相当常见的元素是 `<abbr>` —— 它常被用来包裹一个缩略语或缩写，并且提供缩写的解释（包含在 title 属性中）。让我们看看下面两个例子：

```javascript
<p>我们使用 <abbr title="超文本标记语言（Hyper text Markup Language）">HTML</abbr> 来组织网页文档。</p>

<p>第 33 届 <abbr title="夏季奥林匹克运动会">奥运会</abbr> 将于 2024 年 8 月在法国巴黎举行。</p>
```

&emsp;这些代码的显示效果如下（当光标移动到项目上时会出现提示）：

```javascript
我们使用 HTML 来组织网页文档。

第 33 届 奥运会 将于 2024 年 8 月在法国巴黎举行。
```

> &emsp;note：还有另一个元素 `<acronym>`，它基本上与 `<abbr>` 相同，专门用于首字母缩略词而不是缩略语。然而，这已经被废弃了 - 它在浏览器的支持中不如 `<abbr>`，并且具有类似的功能，所以没有意义。只需使用 `<abbr>`。

## 标记联系方式

&emsp;HTML 有个用于标记联系方式的元素 —— `<address>`。它仅仅包含你的联系方式，例如：

```javascript
<address>
  <p>Chris Mills, Manchester, The Grim North, UK</p>
</address>
```

&emsp;但要记住的一点是，`<address>` 元素是为了标记编写 HTML 文档的人的联系方式，而不是任何其他的内容。因此，如果这是 Chris 写的文档，上面的内容将会很好。注意，下面的内容也是可以的：

```javascript
<address>
  <p>Page written by <a href="../authors/chris-mills/">Chris Mills</a>.</p>
</address>
```

## 上标和下标

&emsp;当你使用日期、化学方程式、和数学方程式时会偶尔使用上标和下标。 `<sup>` 和 `<sub>` 元素可以解决这样的问题。例如：

```javascript
<p>咖啡因的化学方程式是 C<sub>8</sub>H<sub>10</sub>N<sub>4</sub>O<sub>2</sub>。</p>
<p>如果 x<sup>2</sup> 的值为 9，那么 x 的值必为 3 或 -3。</p>
```

&emsp;这些代码输出的结果是：

&emsp;咖啡因的化学方程式是 C8H10N4O2。（下标在 markdown 中显示不出来）

&emsp;如果 x2 的值为 9，那么 x 的值必为 3 或 -3。（上标在 markdown 中显示不出来）

## 展示计算机代码

&emsp;有大量的 HTML 元素可以来标记计算机代码：

+ `<code>`: 用于标记计算机通用代码。
+ `<pre>`: 用于保留空白字符（通常用于代码块）—— 如果你在文本中使用缩进或多余的空白，浏览器将忽略它，你将不会在呈现的页面上看到它。但是，如果你将文本包含在 `<pre></pre>` 标签中，那么空白将会以与你在文本编辑器中看到的相同的方式渲染出来。
+ `<var>`: 用于标记具体变量名。
+ `<kbd>`: 用于标记输入电脑的键盘（或其他类型）输入。
+ `<samp>`: 用于标记计算机程序的输出。

&emsp;让我们看看一些例子。

```javascript
<pre><code>const para = document.querySelector('p');

para.onclick = function() {
  alert('噢，噢，噢，别点我了。');
}</code></pre>

<p>请不要使用 <code>&lt;font&gt;</code> 、 <code>&lt;center&gt;</code> 等表象元素。</p>

<p>在上述的 JavaScript 示例中，<var>para</var> 表示一个段落元素。</p>


<p>按 <kbd>Ctrl</kbd>/<kbd>Cmd</kbd> + <kbd>A</kbd> 选择全部内容。</p>

<pre>$ <kbd>ping mozilla.org</kbd>
<samp>PING mozilla.org (63.245.215.20): 56 data bytes
64 bytes from 63.245.215.20: icmp_seq=0 ttl=40 time=158.233 ms</samp></pre>
```

## 标记时间和日期

&emsp;HTML 还支持将时间和日期标记为可供机器识别的格式的 `<time>` 元素。例如：

```javascript
<time datetime="2016-01-20">2016 年 1 月 20 日</time>
```

&emsp;为什么需要这样做？因为世界上有许多种书写日期的格式，上边的日期可能被写成：

+ 20 January 2016
+ 20th January 2016
+ Jan 20 2016
+ 20/06/16
+ 06/20/16
+ The 20th of next month
+ 20e Janvier 2016
+ 2016 年 1 月 20 日
+ And so on

&emsp;但是这些不同的格式不容易被电脑识别 — 假如你想自动抓取页面上所有事件的日期并将它们插入到日历中，`<time>` 元素允许你附上清晰的、可被机器识别的 时间/日期来实现这种需求。

&emsp;上述基本的例子仅仅提供了一种简单的可被机器识别的日期格式，这里还有许多其他支持的格式，例如：

```javascript
<!-- 标准简单日期 -->
<time datetime="2016-01-20">20 January 2016</time>
<!-- 只包含年份和月份-->
<time datetime="2016-01">January 2016</time>
<!-- 只包含月份和日期 -->
<time datetime="01-20">20 January</time>
<!-- 只包含时间，小时和分钟数 -->
<time datetime="19:30">19:30</time>
<!-- 还可包含秒和毫秒 -->
<time datetime="19:30:01.856">19:30:01.856</time>
<!-- 日期和时间 -->
<time datetime="2016-01-20T19:30">7.30pm, 20 January 2016</time>
<!-- 含有时区偏移值的日期时间 -->
<time datetime="2016-01-20T19:30+01:00">7.30pm, 20 January 2016 is 8.30pm in France</time>
<!-- 调用特定的周 -->
<time datetime="2016-W04">The fourth week of 2016</time>
```

&emsp;到这里你就完成了 HTML 语义文本元素的学习。但要记住，你在本课程中学到的并不是 HTML 文本元素的详细列表 — 我们想要尽量覆盖主要的、通用的、常见的，或者至少是有趣的部分。如果你想找到更多的 HTML 元素，可以看一看 [HTML 元素参考](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Element)（从 内联文本语义部分开始会是一个好的选择） 。在下一篇文章中我们将会学习用来组织 HTML 文档不同部分的 HTML 元素。

## 文档和网站结构

&emsp;除了定义页面的个别部分（例如 "段落" 或 "图像"）外，HTML 也被用于定义网站的区域（例如 "标题"、"导航菜单"、"主内容列"）。本文将探讨如何规划一个基本的网站结构，以及如何编写 HTML 来表示这个结构。

## 文档的基本组成部分

&emsp;网页的外观多种多样，但是除了全屏视频或游戏，或艺术作品页面，或只是结构不当的页面以外，都倾向于使用类似的标准组件：


























## 参考链接
**参考链接:🔗**
+ [HTML 开发者指南](https://developer.mozilla.org/zh-CN/docs/Learn/HTML)
+ [HTML 参考](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Reference)
