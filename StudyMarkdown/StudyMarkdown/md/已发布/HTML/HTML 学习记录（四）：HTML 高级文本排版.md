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

+ 页眉：通常横跨于整个页面顶部有一个大标题 和/或 一个标志。这是网站的主要一般信息，通常存在于所有网页。
+ 导航栏：指向网站各个主要区段的超链接。通常用菜单按钮、链接或标签页表示。类似于标题栏，导航栏通常应在所有网页之间保持一致，否则会让用户感到疑惑，甚至无所适从。许多 web 设计人员认为导航栏是标题栏的一部分，而不是独立的组件，但这并非绝对；还有人认为，两者独立可以提供更好的无障碍访问特性，因为屏幕阅读器可以更清晰地分辨二者。
+ 主内容：中心的大部分区域是当前网页大多数的独有内容，例如视频、文章、地图、新闻等。这些内容是网站的一部分，且会因页面而异。
+ 侧边栏：一些外围信息、链接、引用、广告等。通常与主内容相关（例如一个新闻页面上，侧边栏可能包含作者信息或相关文章链接），还可能存在其他的重复元素，如辅助导航系统。
+ 页脚：横跨页面底部的狭长区域。和标题一样，页脚是放置公共信息（比如版权声明或联系方式）的，一般使用较小字体，且通常为次要内容。还可以通过提供快速访问链接来进行 SEO。

## 用于构建内容的 HTML

&emsp;以上简单示例不是很精美，但是足够说明网站的典型布局方式了。一些站点拥有更多列，其中一些远比这复杂，但一切在你掌握之中。通过合适的 CSS，你可以使用相当多种的任意页面元素来环绕在不同的页面区域来做成你想要的样子，但如前所述，我们要敬畏语义，做到正确选用元素。

&emsp;这是因为视觉效果并不是一切。我们可以修改最重要内容（例如导航菜单和相关链接）的颜色、字体大小来吸引用户的注意，但是这对视障人士是无效的，"粉红色" 和 "大字体" 对他们并不奏效。

> &emsp;note：全球色盲患者比例为 4%，换句话说，每 12 名男性就有一位色盲，每 200 名女性就有一位色盲。全盲和视障人士约占世界人口（约 70 亿）的 13％（2015 年 全球约有 9.4 亿人口存在视力问题）。

&emsp;HTML 代码中可根据功能来为区段添加标记。可使用元素来无歧义地表示上文所讲的内容区段，屏幕阅读器等辅助技术可以识别这些元素，并帮助执行 "找到主导航" 或 "找到主内容" 等任务。

&emsp;为了实现语义化标记，HTML 提供了明确这些区段的专用标签，例如：

+ `<header>`：页眉。
+ `<nav>`：导航栏。
+ `<main>`：主内容。主内容中还可以有各种子内容区段，可用 `<article>`、`<section>` 和 `<div>` 等元素表示。
+ `<aside>`：侧边栏，经常嵌套在 `<main>` 中。
+ `<footer>`：页脚。

### 研究示例代码

&emsp;上图的示例可用下面的代码表示（完整代码请参见 GitHub），请结合图片观察代码，并找出代码中可见的区段：

```javascript
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>二次元俱乐部</title>
    <link href="https://fonts.googleapis.com/css?family=Open+Sans+Condensed:300|Sonsie+One" rel="stylesheet">
    <link href="https://fonts.googleapis.com/css?family=ZCOOL+KuaiLe" rel="stylesheet">
    <link href="style.css" rel="stylesheet">
  </head>

  <body>
    <header> <!-- 本站所有网页的统一主标题 -->
      <h1>聆听电子天籁之音</h1>
    </header>

    <nav> <!-- 本站统一的导航栏 -->
      <ul>
        <li><a href="#">主页</a></li>
        <!-- 共 n 个导航栏项目，省略…… -->
      </ul>

      <form> <!-- 搜索栏是站点内导航的一个非线性的方式。 -->
        <input type="search" name="q" placeholder="要搜索的内容">
        <input type="submit" value="搜索">
      </form>
    </nav>

    <main> <!-- 网页主体内容 -->
      <article>
        <!-- 此处包含一个 article（一篇文章），内容略…… -->
      </article>

      <aside> <!-- 侧边栏在主内容右侧 -->
        <h2>相关链接</h2>
        <ul>
          <li><a href="#">这是一个超链接</a></li>
          <!-- 侧边栏有 n 个超链接，略略略…… -->
        </ul>
      </aside>
    </main>

    <footer> <!-- 本站所有网页的统一页脚 -->
      <p>© 2050 某某保留所有权利</p>
    </footer>
  </body>
</html>
```

&emsp;请花一些时间来阅读，其中的注释应该能够帮助你理解这些代码。现在能够理解上面的代码就可以，因为编写一套正确的 HTML 结构是理解文档布局的前提，布局工作就交给 CSS 吧。在学习 CSS 一章时我们再展开介绍。

## HTML 布局元素细节

&emsp;理解所有 HTML 区段元素具体含义是很有益处的，这一点将随着个人 web 开发经验的逐渐丰富日趋显现。更多细节请查阅 HTML 元素参考。现在，你只需要理解以下主要元素的意义：

+ `<main>` 存放每个页面独有的内容。每个页面上只能用一次 `<main>`，且直接位于 `<body>` 中。最好不要把它嵌套进其它元素。
+ `<article>` 包围的内容即一篇文章，与页面其它部分无关（比如一篇博文）。
+ `<section>` 与 `<article>` 类似，但 `<section>` 更适用于组织页面使其按功能（比如迷你地图、一组文章标题和摘要）分块。一般的最佳用法是：以 标题 作为开头；也可以把一篇 `<article>` 分成若干部分并分别置于不同的 `<section>` 中，也可以把一个区段 `<section>` 分成若干部分并分别置于不同的 `<article>` 中，取决于上下文。
+ `<aside>` 包含一些间接信息（术语条目、作者简介、相关链接，等等）。
+ `<header>` 是简介形式的内容。如果它是 `<body>` 的子元素，那么就是网站的全局页眉。如果它是 `<article>` 或 `<section>` 的子元素，那么它是这些部分特有的页眉（此 `<header>` 非彼 `<head>`）。
+ `<nav>` 包含页面主导航功能。其中不应包含二级链接等内容。
+ `<footer>` 包含了页面的页脚部分。

### 无语义元素

&emsp;有时你会发现，对于一些要组织的项目或要包装的内容，现有的语义元素均不能很好对应。有时候你可能只想将一组元素作为一个单独的实体来修饰来响应单一的用 CSS 或 JavaScript。为了应对这种情况，HTML 提供了 `<div>` 和 `<span>` 元素。应配合使用 `class` 属性提供一些标签，使这些元素能易于查询。

&emsp;`<span>` 是一个内联的（inline）无语义元素，最好只用于无法找到更好的语义元素来包含内容时，或者不想增加特定的含义时。例如：

```javascript
<p>国王喝得酩酊大醉，在凌晨 1 点时才回到自己的房间，踉跄地走过门口。<span class="editor-note">[编辑批注：此刻舞台灯光应变暗]</span>.</p>
```

&emsp;这里，"编辑批注" 仅仅是对舞台剧导演提供额外指引；没有具体语义。对于视力正常的用户，CSS 应将批注内容与主内容稍微隔开一些。

&emsp;`<div>` 是一个块级无语义元素，应仅用于找不到更好的块级元素时，或者不想增加特定的意义时。例如，一个电子商务网站页面上有一个一直显示的购物车组件。

```javascript
<div class="shopping-cart">
  <h2>购物车</h2>
  <ul>
    <li>
      <p><a href=""><strong>银耳环</strong></a>：$99.95.</p>
      <img src="../products/3333-0985/" alt="Silver earrings">
    </li>
    <li>
      ...
    </li>
  </ul>
  <p>售价：$237.89</p>
</div>
```

&emsp;这里不应使用 `<aside>`，因为它和主内容并没有必要的联系（你想在任何地方都能看到它）。甚至不能用 `<section>`，因为它也不是页面上主内容的一部分。所以在这种情况下就应使用 `<div>`，为满足无障碍使用特征，还应为购物车的标题设置一个可读标签。

> &emsp;note：`<div>` 非常便利但容易被滥用。由于它们没有语义值，会使 HTML 代码变得混乱。要小心使用，只有在没有更好的语义方案时才选择它，而且要尽可能少用，否则文档的升级和维护工作会非常困难。

### 换行与水平分割线

&emsp;有时会用到 `<br>` 和 `<hr>` 两个元素，需要介绍一下。

&emsp;`<br>` 可在段落中进行换行；`<br>` 是唯一能够生成多个短行结构（例如邮寄地址或诗歌）的元素。比如：

```javascript
<p>从前有个人叫小高<br>
他说写 HTML 感觉最好<br>
但他写的代码结构语义一团糟<br>
他写的标签谁也懂不了。</p>
```

&emsp;没有 `<br>` 元素，这段会直接显示在长长的一行中（如前文所讲，HTML 会忽略大部分空格）；使用 `<br>` 元素，才使得诗看上去像诗。

&emsp;`<hr>` 元素在文档中生成一条水平分割线，表示文本中主题的变化（例如话题或场景的改变）。一般就是一条水平的直线。例如：

```javascript
<p>...</p>
<hr>
<p>...</p>
```
















## 参考链接
**参考链接:🔗**
+ [HTML 开发者指南](https://developer.mozilla.org/zh-CN/docs/Learn/HTML)
+ [HTML 参考](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Reference)
