# JavaScript 学习记录（十）：JavaScript modules

## 模块化的背景

&emsp;JavaScript 程序本来很小 —— 在早期，它们大多被用来执行独立的脚本任务，在你的 web 页面需要的地方提供一定交互，所以一般不需要多大的脚本。过了几年，我们现在有了运行大量 JavaScript 脚本的复杂程序，还有一些被用在其他环境（例如 Node.js）。

&emsp;因此，近年来，有必要开始考虑提供一种将 JavaScript 程序拆分为可按需导入的单独模块的机制。Node.js 已经提供这个能力很长时间了，还有很多的 JavaScript 库和框架已经开始了模块的使用（例如，CommonJS 和基于 AMD 的其他模块系统 如 RequireJS， 以及最新的 Webpack 和 Babel）。

&emsp;好消息是，最新的浏览器开始原生支持模块功能了，这是本文要重点讲述的。这会是一个好事情 —- 浏览器能够最优化加载模块，使它比使用库更有效率：使用库通常需要做额外的客户端处理。

&emsp;使用 JavaScript 模块依赖于 import 和 export。

## 导出模块的功能

&emsp;为了获得模块的功能要做的第一件事是把它们导出来。使用 export 语句来完成。

&emsp;最简单的方法是把它（指上面的 export 语句）放到你想要导出的项前面，比如：

```javascript
export const name = 'square';

export function draw(ctx, length, x, y, color) {
  ctx.fillStyle = color;
  ctx.fillRect(x, y, length, length);

  return {
    length: length,
    x: x,
    y: y,
    color: color
  };
}
```

&emsp;你能够导出函数，var、let、const 和等会会看到的类。export 要放在最外层；比如你不能够在函数内使用 export。

&emsp;一个更方便的方法导出所有你想要导出的模块的方法是在模块文件的末尾使用一个 export 语句，语句是用花括号括起来的用逗号分割的列表。比如：

```javascript
export { name, draw, reportArea, reportPerimeter };
```

## 导入功能到你的脚本

&emsp;你想在模块外面使用一些功能，那你就需要导入他们才能使用。最简单的就像下面这样的：

```javascript
import { name, draw, reportArea, reportPerimeter } from '/js-examples/modules/basic-modules/modules/square.mjs';
```

&emsp;使用 import 语句，然后你被花括号包围的用逗号分隔的你想导入的功能列表，然后是关键字 from，然后是模块文件的路径。模块文件的路径是相对于站点根目录的相对路径，对于我们的 basic-modules 应该是 /js-examples/modules/basic-modules。

&emsp;当然，我们写的路径有一点不同 -- 我们使用点语法意味 "当前路径"，跟随着包含我们想要找的文件的路径。这比每次都要写下整个相对路径要好得多，因为它更短，使得 URL 可移植 -- 如果在站点层中你把它移动到不同的路径下面仍然能够工作（修订版 1889482）。

```javascript
/js/examples/modules/basic-modules/modules/square.mjs

// 变成

./modules/square.mjs
```

&emsp;因为你导入了这些功能到你的脚本文件，你可以像定义在相同的文件中的一样去使用它。下面展示的是在 main.mjs 中的 import 语句下面的内容。

```javascript
let myCanvas = create('myCanvas', document.body, 480, 320);
let reportList = createReportList(myCanvas.id);

let square1 = draw(myCanvas.ctx, 50, 50, 100, 'blue');
reportArea(square1.length, reportList);
reportPerimeter(square1.length, reportList);
```

## 应用模块到你的 HTML

&emsp;现在我们只需要将 main.mjs 模块应用到我们的 HTML 页面。 这与我们将常规脚本应用于页面的方式非常相似，但有一些显着的差异。

&emsp;首先，你需要把 type="module" 放到 `<script>` 标签中，来声明这个脚本是一个模块：

```javascript
<script type="module" src="main.mjs"></script>
```

&emsp;你导入模块功能的脚本基本是作为顶级模块。如果省略它，Firefox 就会给出错误 "SyntaxError: import declarations may only appear at top level of a module"。

&emsp;你只能在模块内部使用 import 和 export 语句；不是普通脚本文件。

## 其他模块与标准脚本的不同

+ 你需要注意本地测试 -- 如果你通过本地加载 HTML 文件 (比如一个 file:// 路径的文件), 你将会遇到 CORS 错误，因为 JavaScript 模块安全性需要。你需要通过一个服务器来测试。
+ 另请注意，你可能会从模块内部定义的脚本部分获得与标准脚本中不同的行为。这是因为模块自动使用严格模式。
+ 加载一个模块脚本时不需要使用 defer 属性模块会自动延迟加载。
+ 最后一个但不是不重要，你需要明白模块功能导入到单独的脚本文件的范围 -- 他们无法在全局获得。因此，你只能在导入这些功能的脚本文件中使用他们，你也无法通过 JavaScript console 中获取到他们，比如，在 DevTools 中你仍然能够获取到语法错误，但是你可能无法像你想的那样使用一些 debug 技术。

## 默认导出 versus 命名导出

&emsp;到目前为止我们导出的功能都是由 named exports 组成 —- 每个项目（无论是函数，常量等）在导出时都由其名称引用，并且该名称也用于在导入时引用它。

&emsp;还有一种导出类型叫做 default export —- 这样可以很容易地使模块提供默认功能，并且还可以帮助 JavaScript 模块与现有的 CommonJS 和 AMD 模块系统进行互操作（正如 ES6 In Depth: Modules by Jason Orendorff 的模块中所解释的那样；搜索 "默认导出"）。

&emsp;看个例子来解释它如何工作。在我们的基本模块 square.mjs 中，你可以找到一个名为 randomSquare() 的函数，它创建一个具有随机颜色，大小和位置的正方形。我们想作为默认导出，所以在文件的底部我们这样写：

```javascript
export default randomSquare;
```

&emsp;注意，不要大括号。我们可以把 export default 放到函数前面，定义它为一个匿名函数，像这样：

```javascript
export default function(ctx) {
  ...
}
```

&emsp;在我们的 main.mjs 文件中，我们使用以下行导入默认函数：

```javascript
import randomSquare from './modules/square.mjs';
```

&emsp;同样，没有大括号，因为每个模块只允许有一个默认导出，我们知道 randomSquare 就是需要的那个。上面的那一行相当于下面的缩写：

```javascript
import {default as randomSquare} from './modules/square.mjs';
```

> &emsp;note：重命名导出项的 as 语法在下面的 重命名导出与导入 部分中进行了说明。

## 避免命名冲突

&emsp;到目前为止，我们的 canvas 图形绘制模块看起来工作的很好。但是如果我们添加一个绘制其他形状的比如圆形或者矩形的模块会发生什么？这些形状可能会有相关的函数比如 draw()、reportArea()，等等；如果我们用相同的名字导入不同的函数到顶级模块文件中，我们会收到冲突和错误。幸运的是，有很多方法来避免。

## 重命名导出与导入

&emsp;在你的 import 和 export 语句的大括号中，可以使用 as 关键字跟一个新的名字，来改变你在顶级模块中将要使用的功能的标识名字。因此，例如，以下两者都会做同样的工作，尽管方式略有不同：

```javascript
// inside module.mjs
export {
  function1 as newFunctionName,
  function2 as anotherNewFunctionName
};

// inside main.mjs
import { newFunctionName, anotherNewFunctionName } from '/modules/module.mjs';
```

```javascript
// inside module.mjs
export { function1, function2 };

// inside main.mjs
import { function1 as newFunctionName,
         function2 as anotherNewFunctionName } from '/modules/module.mjs';
```

&emsp;让我们看一个真实的例子。在我们的重命名目录中，您将看到与上一个示例中相同的模块系统，除了我们添加了 circle.mjs 和 triangle.mjs 模块以绘制和报告圆和三角形。







## 参考链接
**参考链接:🔗**
+ [JavaScript 参考](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference)
+ [JavaScript 指南](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Guide)
+ [JavaScript Related Topics](https://developer.mozilla.org/zh-CN/docs/Learn/JavaScript)
+ [JavaScript 主题学习区](https://developer.mozilla.org/zh-CN/docs/learn/JavaScript)
+ [重新介绍 JavaScript（JS 教程）](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/A_re-introduction_to_JavaScript)
