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

&emsp;现在我们完成了表单的 HTML 代码：

```javascript
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8" />
    <title>Your first HTML form</title>
  </head>

  <body>
    <form action="/my-handling-form-page" method="post">
      <ul>
        <li>
          <label for="name">Name:</label>
          <input type="text" id="name" name="user_name" />
        </li>
        <li>
          <label for="mail">E-mail:</label>
          <input type="email" id="mail" name="user_mail" />
        </li>
        <li>
          <label for="msg">Message:</label>
          <textarea id="msg" name="user_message"></textarea>
        </li>
        <li class="button">
          <button type="submit">Send your message</button>
        </li>
      </ul>
    </form>
  </body>
</html>
```

&emsp;如何排布好表单是公认的难点。这超出了本文的讨论范围，所以现在我们只需要让你添加一些 CSS 来让它看起来很好。

&emsp;首先，在你的 HTML 头部中添加一个 `<style>` 元素。应该是这样的：

```javascript
<style>
</style>
```

&emsp;在样式标签中，添加如下的 CSS，如下所示：

```javascript
form {
  /* 居中表单 */
  margin: 0 auto;
  width: 400px;
  /* 显示表单的轮廓 */
  padding: 1em;
  border: 1px solid #CCC;
  border-radius: 1em;
}

ul {
  list-style: none;
  padding: 0;
  margin: 0;
}

form li + li {
  margin-top: 1em;
}

label {
  /* 确保所有 label 大小相同并正确对齐 */
  display: inline-block;
  width: 90px;
  text-align: right;
}

input, textarea {
  /* 确保所有文本输入框字体相同
     textarea 默认是等宽字体 */
  font: 1em sans-serif;

  /* 使所有文本输入框大小相同 */
  width: 300px;
  box-sizing: border-box;

  /* 调整文本输入框的边框样式 */
  border: 1px solid #999;
}

input:focus, textarea:focus {
  /* 给激活的元素一点高亮效果 */
  border-color: #000;
}

textarea {
  /* 使多行文本输入框和它们的 label 正确对齐 */
  vertical-align: top;

  /* 给文本留下足够的空间 */
  height: 5em;
}

.button {
  /* 把按钮放到和文本输入框一样的位置 */
  padding-left: 90px; /* 和 label 的大小一样 */
}

button {
  /* 这个外边距的大小与 label 和文本输入框之间的间距差不多 */
  margin-left: .5em;
}
```

## 向你的 web 服务器发送表单数据

&emsp;最后一部分，也许是最棘手的部分，是在服务器端处理表单数据。如前所述，大多数时候 HTML 表单是向用户请求数据并将其发送到 web 服务器的一种方便的方式。

&emsp;`<form>` 元素将定义如何通过 action 属性和 method 属性来发送数据的位置和方式。

&emsp;但这还不够，我们还需要为我们的数据提供一个名称。这些名字对双方都很重要：在浏览器端，它告诉浏览器给数据各自哪个名称，在服务器端，它允许服务器按名称处理每个数据块。

&emsp;要将数据命名为表单，你需要在每个表单小部件上使用 name 属性来收集特定的数据块。让我们再来看看我们的表单代码：

```javascript
<form action="/my-handling-form-page" method="post">
  <div>
    <label for="name">Name:</label>
    <input type="text" id="name" name="user_name">
  </div>
  <div>
    <label for="mail">E-mail:</label>
    <input type="email" id="mail" name="user_email">
  </div>
  <div>
    <label for="msg">Message:</label>
    <textarea id="msg" name="user_message"></textarea>
  </div>

  ...
```

&emsp;在我们的例子中，表单会发送三个已命名的数据块 "user_name"、"user_email" 和 "user_message"，这些数据将使用 HTTP POST 方法，把信息发送到 URL 为 "/my-handling-form-page" 目录下。

&emsp;在服务器端，位于 URL "/my-handling-form-page" 上的脚本将接收的数据作为 HTTP 请求中包含的 3 个键/值项的列表。这个脚本处理这些数据的方式取决于你。每个服务器端语言(PHP、Python、Ruby、Java、c 等等)都有自己的机制。深入到这个主题已经超出了本指南的范围，但是如果你想了解更多，我们已经在发送表单数据文章中提供了一些示例。 

## 如何构造 HTML 表单

&emsp;有了基础知识，我们现在更详细地了解了用于为表单的不同部分提供结构和意义的元素。

&emsp;HTML 表单的灵活性使它们成为 HTML 中最复杂的结构之一;你可以使用专用的表单元素和属性构建任何类型的基本表单。在构建 HTML 表单时使用正确的结构将有助于确保表单可用性和可访问性。

## `<form>` 元素

&emsp;`<form>` 元素按照一定的格式定义了表单和确定表单行为的属性。当你想要创建一个 HTML 表单时，都必须从这个元素开始，然后把所有内容都放在里面。许多辅助技术或浏览器插件可以发现 `<form>` 元素并实现特殊的钩子，使它们更易于使用。

&emsp;我们早在之前的文章中就遇见过它了。 

> &emsp;note：严格禁止在一个表单内嵌套另一个表单。嵌套会使表单的行为不可预知，而这取决于正在使用的浏览器。

&emsp;请注意，在 `<form>` 元素之外使用表单小部件是可以的，但是如果你这样做了，那么表单小部件与任何表单都没有任何关系。这样的小部件可以在表单之外使用，但是你应该对于这些小部件有特别的计划，因为它们自己什么也不做。你将不得不使用 JavaScript 定制他们的行为。

&emsp;HTML5 在 HTML 表单元素中引入 form 属性。它让你显式地将元素与表单绑定在一起，即使元素不在 `<form>` 中。不幸的是，就目前而言，跨浏览器对这个特性的实现还不足以使用。

## `<fieldset>` 和 `<legend>` 元素

&emsp;`<fieldset>` 元素是一种方便的用于创建具有相同目的的小部件组的方式，出于样式和语义目的。你可以在 `<fieldset>` 开口标签后加上一个 `<legend>` 元素来给 `<fieldset>` 标上标签。`<legend>` 的文本内容正式地描述了 `<fieldset>` 里所含有部件的用途。

&emsp;许多辅助技术将使用 `<legend>` 元素，就好像它是相应的 `<fieldset>` 元素里每个部件的标签的一部分。例如，在说出每个小部件的标签之前，像 Jaws 或 NVDA 这样的屏幕阅读器会朗读出 legend 的内容。

&emsp;这里有一个小例子：

```javascript
<!DOCTYPE html>
<html>

<head>
    <meta charset="utf-8">
    <title>fieldset and legend example</title>
</head>

<body>
    <form>
        <fieldset>
            <legend>Fruit juice size</legend>
            <p> <input type="radio" name="size" id="size_1" value="small"> <label for="size_1">Small</label> </p>
            <p> <input type="radio" name="size" id="size_2" value="medium"> <label for="size_2">Medium</label> </p>
            <p> <input type="radio" name="size" id="size_3" value="large"> <label for="size_3">Large</label> </p>
        </fieldset>
    </form>
</body>

</html>
```

&emsp;当阅读上述表格时，屏幕阅读器将会读第一个小部件 "Fruit juice size small"、"Fruit juice size medium" 为第二个，"Fruit juice size large" 为第三个。

&emsp;本例中的用例是最重要的。每当你有一组单选按钮时，你应该将它们嵌套在 `<fieldset>` 元素中。还有其他用例，一般来说，`<fieldset>` 元素也可以用来对表单进行分段。理想情况下，长表单应该在拆分为多个页面，但是如果表单很长，却必须在单个页面上，那么将以不同的关联关系划分的分段，分别放在不同的 fieldset 里，可以提高可用性。

&emsp;因为它对辅助技术的影响，`<fieldset>` 元素是构建可访问表单的关键元素之一。无论如何，你有责任不去滥用它。如果可能，每次构建表单时，尝试侦听屏幕阅读器如何解释它。如果听起来很奇怪，试着改进表单结构。

## `<label>` 元素

&emsp;正如我们在前一篇文章中看到的，`<label>` 元素是为 HTML 表单小部件定义标签的正式方法。如果你想构建可访问的表单，这是最重要的元素 —— 当实现的恰当时，屏幕阅读器会连同有关的说明和表单元素的标签一起朗读。以我们在上一篇文章中看到的例子为例：

```javascript
<label for="name">Name:</label> <input type="text" id="name" name="user_name">
```

&emsp;`<label>` 标签与 `<input>` 通过他们各自的 for 属性和 id 属性正确相关联（label 的 for 属性和它对应的小部件的 id 属性），这样，屏幕阅读器会读出诸如 "Name, edit text" 之类的东西。

&emsp;如果标签没有正确设置，屏幕阅读器只会读出 "Edit text blank" 之类的东西，这样会没什么帮助。

&emsp;注意，一个小部件可以嵌套在它的 `<label>` 元素中，就像这样：

```javascript
<label for="name">
  Name: <input type="text" id="name" name="user_name">
</label>
```

&emsp;尽管可以这样做，但人们认为设置 for 属性才是最好的做法，因为一些辅助技术不理解标签和小部件之间的隐式关系。

## 标签也可点击！

&emsp;正确设置标签的另一个好处是可以在所有浏览器中单击标签来激活相应的小部件。这对于像文本输入这样的例子很有用，这样你可以通过点击标签，和点击输入区效果一样，来聚焦于它，这对于单选按钮和复选框尤其有用 —— 这种控件的可点击区域可能非常小，设置标签来使它们可点击区域变大是非常有用的。

&emsp;举个例子：

```javascript
<form>
  <p>
    <input type="checkbox" id="taste_1" name="taste_cherry" value="1">
    <label for="taste_1">I like cherry</label>
  </p>
  <p>
    <input type="checkbox" id="taste_2" name="taste_banana" value="2">
    <label for="taste_2">I like banana</label>
  </p>
</form>
```

## 多个标签

&emsp;严格地说，你可以在一个小部件上放置多个标签，但是这不是一个好主意，因为一些辅助技术可能难以处理它们。在多个标签的情况下，你应该将一个小部件和它的标签嵌套在一个 `<label>` 元素中。

&emsp;让我们考虑下面这个例子：

```javascript
<p>Required fields are followed by <abbr title="required">*</abbr>.</p>

<!--这样写：-->
<div>
  <label for="username">Name:</label>
  <input type="text" name="username">
  <label for="username"><abbr title="required">*</abbr></label>
</div>

<!--但是这样写会更好：-->
<div>
  <label for="username">
    <span>Name:</span>
    <input id="username" type="text" name="username">
    <abbr title="required">*</abbr>
  </label>
</div>

<!--但最好的可能是这样：-->
<div>
  <label for="username">Name: <abbr title="required">*</abbr></label>
  <input id="username" type="text" name="username">
</div>
```

&emsp;顶部的段落定义了所需元素的规则。它必须在开始时确保像屏幕阅读器这样的辅助技术在用户找到必需的元素之前显示或念出它们。这样，他们就知道星号表达的是什么意思了。根据屏幕阅读器的设置，屏幕阅读器会把星号读为 "star" 或 "required"，取决于屏幕阅读器的设置 —— 不管怎样，要念出来的都会在第一段清楚的呈现出来。

+ 在第一个例子中，标签根本没有和 input 一起被念出来 —— 读出来的只是 "edit the blank"，和单独被念出的标签。多个 `<label>` 元素会使屏幕阅读器迷惑。
+ 在第二个例子中，事情变得清晰一点了 —— 标签和输入一起，读出的是 "name star name edit text"，但标签仍然是单独读出的。这还是有点令人困惑，但这次还是稍微好一点了，因为 input 和 label 联系起来了。
+ 第三个例子是最好的 —— 标签是一起读出的，标签和输入读出的是 "name star edit text"。

## 用于表单的通用 HTML 结构

&emsp;除了特定于 HTML 表单的结构之外，还应该记住表单同样是 HTML。这意味着你可以使用 HTML 的所有强大功能来构造一个 HTML 表单。

&emsp;正如你在示例中可以看到的，用 `<div>` 元素包装标签和它的小部件是很常见的做法。`<p>` 元素也经常被使用，HTML 列表也是如此（后者在构造多个复选框或单选按钮时最为常见）。

&emsp;除了 `<fieldset>` 元素之外，使用 HTML 标题（例如，`<h1>`、`<h2>`）和分段（如 `<section>`）来构造一个复杂的表单也是一种常见的做法。

&emsp;最重要的是，你要找到一种你觉得很舒服的风格去码代码，而且它也能带来可访问的、可用的形式。

&emsp;它包含了从功能上划分开并分别包含在 `<section>` 元素中的部分，以及一个 `<fieldset>` 来包含单选按钮。

## 自主学习：构建一个表单结构

&emsp;让我们把这些想法付诸实践，建立一个稍微复杂一点的表单结构 —— 一个支付表单。这个表单将包含许多你可能还不了解的小部件类型 — 现在不要担心这个；在下一篇文章（原生表单小部件）中，你将了解它们是如何工作的。现在，当你遵循下面的指令时，请仔细阅读这些描述，并开始理解我们使用的包装器元素是如何构造表单的，以及为什么这么做。

1. 在开始之前，在计算机上的一个新目录中，创建一个 空白模板文件 和我们的支付表单的 CSS 样式的本地副本。

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

```javascript
h1 {
    margin-top: 0;
}

ul {
    margin: 0;
    padding: 0;
    list-style: none;
}

form {
    margin: 0 auto;
    width: 400px;
    padding: 1em;
    border: 1px solid #CCC;
    border-radius: 1em;
}

div+div {
    margin-top: 1em;
}

label span {
    display: inline-block;
    width: 120px;
    text-align: right;
}

input, textarea {
    font: 1em sans-serif;
    width: 250px;
    box-sizing: border-box;
    border: 1px solid #999;
}

input[type=checkbox], input[type=radio] {
    width: auto;
    border: none;
}

input:focus, textarea:focus {
    border-color: #000;
}

textarea {
    vertical-align: top;
    height: 5em;
    resize: vertical;
}

fieldset {
    width: 250px;
    box-sizing: border-box;
    margin-left: 136px;
    border: 1px solid #999;
}

button {
    margin: 20px 0 0 124px;
}

label {
  position: relative;
}
```

2. 首先，通过添加下面这行代码到你的 HTML `<head>` 使你的 HTML 应用 CSS。

```javascript
<link href="payment-form.css" rel="stylesheet">
```

3. 接下来，通过添加外部 `<form>` 元素来开始一张表单：

```javascript
<form>

</form>
```

4. 在 `<form>` 标签内，以添加一个标题和段落开始，告诉用户必需的字段是如何标记的：

```javascript
<h1>Payment form</h1>
<p>Required fields are followed by <strong><abbr title="required">*</abbr></strong>.</p>
```

5. 接下来，我们将在表单中添加一个更大的代码段，在我们之前的代码下面。在这里，你将看到，我们正在将联系人信息字段包装在一个单独的 `<section>` 元素中。此外，我们有一组两个单选按钮，每个单选按钮都放在自己的列表中 (`<li>`)) 元素。最后，我们有两个标准文本 `<input>` 和它们相关的 `<label>` 元素，每个元素包含在 `<p>` 中，加上输入密码的密码输入。现在将这些代码添加到你的表单中：

```javascript
<section>
    <h2>Contact information</h2>
    <fieldset>
      <legend>Title</legend>
      <ul>
          <li>
            <label for="title_1">
              <input type="radio" id="title_1" name="title" value="K" >
              King
            </label>
          </li>
          <li>
            <label for="title_2">
              <input type="radio" id="title_2" name="title" value="Q">
              Queen
            </label>
          </li>
          <li>
            <label for="title_3">
              <input type="radio" id="title_3" name="title" value="J">
              Joker
            </label>
          </li>
      </ul>
    </fieldset>
    <p>
      <label for="name">
        <span>Name: </span>
        <strong><abbr title="required">*</abbr></strong>
      </label>
      <input type="text" id="name" name="username">
    </p>
    <p>
      <label for="mail">
        <span>E-mail: </span>
        <strong><abbr title="required">*</abbr></strong>
      </label>
      <input type="email" id="mail" name="usermail">
    </p>
    <p>
      <label for="pwd">
        <span>Password: </span>
        <strong><abbr title="required">*</abbr></strong>
      </label>
      <input type="password" id="pwd" name="password">
    </p>
</section>
```

6. 现在，我们将转到表单的第二个 `<section>` —— 支付信息。在这里，我们有三个不同的小部件以及它们的标签，每个都包含在一个 `<p>` 中。第一个是选择信用卡类型的下拉菜单 (`<select>`)。第二个是输入一个信用卡号的类型编号的 `<input>` 元素。最后一个是输入 date 类型的 `<input>` 元素，用来输入卡片的过期日期（这将在支持的浏览器中出现一个日期选择器小部件，并在非支持的浏览器中回退到普通的文本输入）。同样，在之前的代码后面输入以下内容：

```javascript
<section>
    <h2>Payment information</h2>
    <p>
      <label for="card">
        <span>Card type:</span>
      </label>
      <select id="card" name="usercard">
        <option value="visa">Visa</option>
        <option value="mc">Mastercard</option>
        <option value="amex">American Express</option>
      </select>
    </p>
    <p>
      <label for="number">
        <span>Card number:</span>
        <strong><abbr title="required">*</abbr></strong>
      </label>
        <input type="number" id="number" name="cardnumber">
    </p>
    <p>
      <label for="date">
        <span>Expiration date:</span>
        <strong><abbr title="required">*</abbr></strong>
        <em>formatted as mm/yy</em>
      </label>
      <input type="date" id="date" name="expiration">
    </p>
</section>
```

7. 我们要添加的最后一个部分要简单得多，它只包含了一个 submit 类型的 `<button>`，用于提交表单数据。现在把这个添加到你的表单的底部：

```javascript
<p> <button type="submit">Validate the payment</button> </p>
```

## 总结

&emsp;现在，已经具备了正确地构造 HTML 表单所需的所有知识;下一篇文章将深入介绍各种不同类型的表单小部件，将希望从用户那里收集信息。

## 不同的表单控件

## 原生表单部件

&emsp;我们从详细了解原始 HTML `<input>` 元素的类型开始，同时学习在收集不同类型数据时可用的选择。

&emsp;在上面，我们标记了一个功能性的 web 表单示例，介绍了一些表单部件和常见的结构元素，并重点介绍了无障碍的最佳实践。现在，我们将详细研究不同表单部件的功能，查看了哪些选项可用于收集不同类型的数据。这个指南有些详尽，涵盖了所有可用的原生表单小部件。

&emsp;要了解在浏览器中可以使用什么类型的原生表单小部件来收集数据，以及如何使用 HTML 实现它们。

> &emsp;note：widget 在本页面中被统一翻译为部件，但在其他地方可能也被译为组件。

&emsp;你可能已经遇见过了一些表单元素，包括：`<form>`、`<fieldset>`、`<legend>`、`<textarea>`、`<label>`、`<button>` 和 `<input>`。这篇文章提到了：

+ 常见的输入（input）类型元素：button、checkbox、file、hidden、image、password、radio、reset、submit 和 text。
+ 所有表单部件共有的一些属性。

> &emsp;note：本文中讨论的大多数特性都在浏览器中得到了广泛的支持，但并非所有表单部件都受浏览器支持。我们在接下来的两篇文章中提到了 HTML5 带来的新的表单部件。如果你想要更准确的细节，你应该参考我们的 HTML 表单元素参考，特别是我们的种类繁多的 `<input>` 类型参考。

## 文本输入框

&emsp;文本输入框是最基本的表单小部件。这是一种非常方便的方式，可以让用户输入任何类型的数据。我们已经看到过几个简单的例子。

> &emsp;note：HTML 表单文本字段是简单的纯文本输入控件。这意味着你不能将它们用作富文本编辑（粗体、斜体等）。你遇到的所有富文本编辑器都是使用 HTML、CSS 和 JavaScript 所创建的自定义小部件。

&emsp;所有文本框都有一些通用规范：

+ 它们可以被标记为 readonly（用户不能修改输入值）甚至是 disabled（输入值永远不会与表单数据的其余部分一起发送）。
+ 它们可以有一个 placeholder；这是文本输入框中出现的文本，用来简略描述输入框的目的。
+ 它们可以使用 size（框的物理尺寸）和 maxlength（可以输入的最大字符数）进行限制。
+ 如果浏览器支持的话，它们可以从拼写检查（使用 spellcheck 属性）中获益。

> &emsp;note：`<input>` 元素是如此特别因为它可以通过简单设置 type 属性，来接收多种类型的数据。它被用于创建大多数类型的表单小部件，包括单行文本字段、没有文本输入的控件、时间和日期控件和按钮。

### 单行文本框

&emsp;












## 参考链接
**参考链接:🔗**
+ [HTML 开发者指南](https://developer.mozilla.org/zh-CN/docs/Learn/HTML)
+ [HTML 参考](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Reference)
