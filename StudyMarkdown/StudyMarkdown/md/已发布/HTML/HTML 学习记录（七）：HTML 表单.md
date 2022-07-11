# HTML 学习记录（七）：HTML 表单

&emsp;表单是构成 Web 世界的重要组成部分 —— 它们提供了大量你所需要用来与网站进行交互所需的功能。比如注册、登录、发送评论反馈、购买商品等等。这个模块将引导你建立一个客户端／前端部分的表单。

## HTML 表单指南

&emsp;这个模块提供了一系列帮助您掌握 HTML 表单的文章。HTML 表单是与用户交互的强大工具;然而，由于历史和技术上的原因，如何充分发挥它们的潜力并不总是显而易见的。在本指南中，我们将介绍 HTML 表单的各个方面：结构、样式、验证表单数据，以及提交数据到服务器。

## 你的第一个表单

&emsp;本系列的第一篇文章提供了你第一次创建 HTML 表单的经验，包括设计一个简单表单，使用正确的 HTML 元素实现它，通过 CSS 添加一些非常简单的样式，以及如何将数据发送到服务器。

## HTML 表单是什么？

&emsp;HTML 表单是用户和 web 站点或应用程序之间交互的主要内容之一。它们允许用户将数据发送到 web 站点。大多数情况下，数据被发送到 web 服务器，但是 web 页面也可以自己拦截它并使用它。

&emsp;HTML 表单是由一个或多个小部件组成的。这些小部件可以是文本字段 (单行或多行)、选择框、按钮、复选框或单选按钮。大多数情况下，这些小部件与描述其目的的标签配对 —— 正确实现的标签能够清楚地指示视力正常的用户和盲人用户输入表单所需的内容。

&emsp;HTML 表单和常规 HTML 文档的主要区别在于，大多数情况下，表单收集的数据被发送到 web 服务器。在这种情况下，你需要设置一个 web 服务器来接收和处理数据。如何设置这样的服务器超出了本文的范围，但是如果你想了解更多，请参阅模块后面的发送表单数据。

## 设计表单

&emsp;在开始编写代码之前，最好先退一步，花点时间考虑一下你的表单。设计一个快速的模型将帮助你定义你想要询问用户的正确的数据集。从用户体验 (UX) 的角度来看，要记住：表单越大，失去用户的风险就越大。保持简单，保持专注：只要求必要的数据。在构建站点或应用程序时，设计表单是非常重要的一步。这超出了本文的范围，涵盖了表单的用户体验，但是如果你想深入了解这个主题，你应该阅读下面的文章：

&emsp;在本文中，我们将构建一个简单的联系人表单。我们的表单将包含三个文本字段和一个按钮。我们向用户询问他们的姓名、电子邮件和他们想要发送的信息。点击这个按钮将把他们的数据发送到一个 web 服务器。

## 主动学习：使用 HTML 实现我们的表单

&emsp;好了，现在我们准备进入 HTML 代码并对表单进行编码。为了构建我们的联系人表单，我们将使用以下 HTML 元素:`<form>`, `<label>`, `<input>`, `<textarea>`, and `<button>`。

&emsp;在进一步讨论之前，先创建一个简单 HTML 模板的本地副本，将在这里输入你的表单 HTML。

```javascript
<!DOCTYPE html>
<html lang="en-US">
  <head>
    <meta charset="utf-8">
    <title>My test page</title>
  </head>
  <body>
    <p>This is my page</p>
  </body>
</html>
```

### `<form>` 元素

&emsp;所有 HTML 表单都以一个 `<form>` 元素开始：

```javascript
<form action="/my-handling-form-page" method="post">

</form>
```

&emsp;这个元素正式定义了一个表单。就像 `<div>` 元素或 `<p>` 元素，它是一个容器元素，但它也支持一些特定的属性来配置表单的行为方式。它的所有属性都是可选的，但实践中最好至少要设置 `action` 属性和 `method` 属性。

+ action 属性定义了在提交表单时，应该把所收集的数据送给谁 (/那个模块)(URL) 去处理。
+ method 属性定义了发送数据的 HTTP 方法 (它可以是 "get" 或 "post").

&emsp;现在，将上面的 `<form>` 元素添加到你的 HTML 主体中。

### `<label>`, `<input>` 和 `<textarea>` 元素

&emsp;我们的联系人表单非常简单，包含三个文本字段，每个字段都有一个标签。该名称的输入字段将是一个基本的单行文本字段，电子邮件的输入字段将是一个只接受电子邮件地址的单行文本字段，而消息的输入字段将是一个基本的多行文本字段。

&emsp;就 HTML 代码而言，我们需要如下的东西来实现这些表单小部件：

```javascript
<form action="/my-handling-form-page" method="post">
  <div>
    <label for="name">Name:</label>
    <input type="text" id="name">
  </div>
  <div>
    <label for="mail">E-mail:</label>
    <input type="email" id="mail">
  </div>
  <div>
    <label for="msg">Message:</label>
    <textarea id="msg"></textarea>
  </div>
</form>
```

&emsp;使用 `<div>` 元素可以使我们更加方便地构造我们自己的代码，并且更容易样式化。注意在所有 `<label>` 元素上使用 for 属性；它是将标签链接到表单小部件的一种正规方式。这个属性引用对应的小部件的 id。这样做有一些好处。最明显的一个好处是允许用户单击标签以激活相应的小部件。如果你想更好地理解这个属性的其他好处，可以找到如何构造 HTML 表单的详细信息。

&emsp;在 `<input>` 元素中，最重要的属性是 type 属性。这个属性非常重要，因为它定义了 `<input>` 属性的行为方式。它可以从根本上改变元素，所以要注意它。稍后你将在原生表单控件文章中找到更多关于此的内容。

+ 在我们的简单示例中，我们使用值 text 作为第一个输入 —— 这个属性的默认值。它表示一个基本的单行文本字段，接受任何类型的文本输入。
+ 对于第二个输入，我们使用值 email，它定义了一个只接受格式正确的电子邮件地址的单行文本字段。这会将一个基本的文本字段转换为一种 "智能" 字段，该字段将对用户输入的数据进行一些检查。在稍后的表单数据验证文章中，你将了解到更多关于表单验证的信息。

&emsp;最后但同样重要的是，要注意 `<input>` 和 `<textarea></textarea>` 的语法。这是 HTML 的一个奇怪之处。`<input>` 标签是一个空元素，这意味着它不需要关闭标签。相反 `<textarea>` 不是一个空元素，因此必须使用适当的结束标记来关闭它。这对 HTML 表单的特定特性有影响：定义默认值的方式。要定义 `<input>` 的默认值，你必须使用 value 属性，如下所示：

```javascript
<input type="text" value="by default this element is filled with this text" />
```

&emsp;相反，如果你想定义 `<textarea>` 的默认值，你只需在 `<textarea>` 元素的开始和结束标记之间放置默认值，就像这样：

```javascript
<textarea>by default this element is filled with this text</textarea>
```

### `<button>` 元素

&emsp;我们的表格已经快准备好了，我们只需要再添加一个按钮，让用户在填写完表单后发送他们的数据。这是通过使用 `<button>` 元素完成的。在 `</form>` 这个结束标签上方添加以下内容：

```javascript
<div class="button">
  <button type="submit">Send your message</button>
</div>
```

&emsp;你会看到 `<button>` 元素也接受一个 type 属性，它接受 submit, reset 或者 button 三个值中的任一个。

+ 单击 type 属性定义为 submit 值 (也是默认值) 的按钮会发送表单的数据到 `<form>` 元素的 action 属性所定义的网页。
+ 单击 type 属性定义为 reset 值的按钮将所有表单小部件重新设置为它们的默认值。从用户体验的角度来看，这被认为是一种糟糕的做法。
+ 单击 type 属性定义为 button 值的按钮……不会发生任何事！这听起来很傻，但是用 JavaScript 构建定制按钮非常有用。

> &emsp;note：你还可以使用相应类型的 `<input>` 元素来生成一个按钮，如 `<input type="submit">`。`<button>` 元素的主要优点是，`<input>` 元素只允许纯文本作为其标签，而 `<button>` 元素允许完整的 HTML 内容，允许更复杂、更有创意的按钮文本。

## 基本表单样式

&emsp;












## 参考链接
**参考链接:🔗**
+ [HTML 开发者指南](https://developer.mozilla.org/zh-CN/docs/Learn/HTML)
+ [HTML 参考](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Reference)
