# HTML 学习记录（三）：HTML ......

## 元数据：`<meta>` 元素

&emsp;元数据就是描述数据的数据，而 HTML 有一个 "官方的" 方式来为一个文档添加元数据 —— `<meta>` 元素。当然，其它的在这篇文章中提到的东西也可以被当作元数据。有很多不同种类的 `<meta>` 元素可以被包含进你的页面的 `<head>` 元素，但是现在我们还不会尝试去解释所有类型，这只会引起混乱。我们会解释一些你常会看到的类型，先让你有个概念。

### 指定你的文档中字符的编码

```javascript
<meta charset="utf-8">
```

&emsp;这个元素简单的指定了文档的字符编码 —— 在这个文档中被允许使用的字符集。utf-8 是一个通用的字符集，它包含了任何人类语言中的大部分的字符。意味着该 web 页面可以显示任意的语言；所以对于你的每一个页面都使用这个设置会是一个好主意！比如说，你的页面可以很好的处理英文和日文。比如说，如果你将你的字符集设置为 ISO-8859-1，那么页面将出现乱码。

> &emsp;note：一些浏览器（比如 Chrome）会自动修正错误的编码，所以取决于你所使用的浏览器，你或许不会看到这个问题。无论如何，你仍然应该为你的页面手动设置编码为 utf-8，来避免在其他浏览器中可能出现的潜在问题。

### 添加作者和描述

&emsp;许多 `<meta>` 元素包含了 name 和 content 属性：

+ name 指定了 meta 元素的类型；说明该元素包含了什么类型的信息。
+ content 指定了实际的元数据内容。

&emsp;这两个 meta 元素对于定义你的页面的作者和提供页面的简要描述是很有用的。让我们看一个例子：

```javascript
<meta name="author" content="Chris Mills">
<meta name="description" content="The MDN Web Docs Learning Area aims to provide
complete beginners to the Web with all they need to know to get
started with developing web sites and applications.">
```

&emsp;指定作者在某些情况下是很有用的：如果你需要联系页面的作者，问一些关于页面内容的问题。一些内容管理系统能够自动获取页面作者的信息，然后用于某些用途。

&emsp;指定包含关于页面内容的关键字的页面内容的描述是很有用的，因为它可能或让你的页面在搜索引擎的相关的搜索出现得更多（这些行为在术语上被称为：搜索引擎优化，或 SEO。）

### 在搜索引擎中 description 的使用

&emsp;description 也被使用在搜索引擎显示的结果页中。你会看到 description `<meta>` 和 `<title>` 元素如何在搜索结果里显示。

> &emsp;许多 `<meta>` 特性已经不再使用。例如，keyword `<meta>` 元素（`<meta name="keywords" content="fill, in, your, keywords, here">`）—— 提供关键词给搜索引擎，根据不同的搜索词，查找到相关的网站 —— 已经被搜索引擎忽略了，因为作弊者填充了大量关键词到 keyword，错误地引导搜索结果。

### 其他类型的元数据

&emsp;当你在网站上查看源码时，你也会发现其它类型的元数据。你在网站上看到的许多功能都是专有创作，旨在向某些网站（如社交网站）提供可使用的特定信息。

&emsp;例如，Facebook 编写的元数据协议 Open Graph Data 为网站提供了更丰富的元数据。在 MDN Web 文档源代码中，你会发现：

```javascript
<meta property="og:image" content="https://developer.mozilla.org/static/img/opengraph-logo.png">
<meta property="og:description" content="The Mozilla Developer Network (MDN) provides information about Open Web technologies including HTML, CSS, and APIs for both Web sites and HTML5 Apps. It also documents Mozilla products, like Firefox OS.">
<meta property="og:title" content="Mozilla Developer Network">
```

&emsp;上面代码展现的一个效果就是，当你在 Facebook 上链接到 MDN 时，该链接将显示一个图像和描述：这为用户提供更丰富的体验。

&emsp;Twitter 还拥有自己的类型的专有元数据协议（称为：Twitter Cards），当网站的 URL 显示在 twitter.com 上时，它具有相似的效果。例如下面：

```javascript
<meta name="twitter:title" content="Mozilla Developer Network">
```

## 在你的站点增加自定义图标

&emsp;为了进一步丰富你的网站设计，你可以在元数据中添加对自定义图标（favicon，为 "favorites icon" 的缩写）的引用，这些将在特定的场合（浏览器的收藏，或书签列表）中显示。

&emsp;这个不起眼的图标已经存在很多很多年了，16x16 像素是这种图标的第一种类型。你可以看见这些图标出现在浏览器每一个打开的标签页中以及书签页中。

&emsp;页面添加图标的方式有：

1. 将其保存在与网站的索引页面相同的目录中，以 .ico 格式保存（大多数浏览器将支持更通用的格式，如 .gif 或 .png，但使用 ICO 格式将确保它能在如 Internet Explorer 6 那样古老的浏览器显示）
2. 
















## 参考链接
**参考链接:🔗**
+ [HTML 开发者指南](https://developer.mozilla.org/zh-CN/docs/Learn/HTML)
+ [HTML 参考](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Reference)
