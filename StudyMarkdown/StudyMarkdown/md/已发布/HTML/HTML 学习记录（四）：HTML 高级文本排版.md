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






























## 参考链接
**参考链接:🔗**
+ [HTML 开发者指南](https://developer.mozilla.org/zh-CN/docs/Learn/HTML)
+ [HTML 参考](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Reference)
