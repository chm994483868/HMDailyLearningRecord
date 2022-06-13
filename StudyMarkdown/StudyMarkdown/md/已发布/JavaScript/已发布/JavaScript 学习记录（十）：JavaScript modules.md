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

&emsp;除了我们添加了 circle.mjs 和 triangle.mjs 模块以绘制和报告圆和三角形。在每个模块中，我们都有 export 相同名称的功能，因此每个模块底部都有相同的导出语句：

```javascript
export { name, draw, reportArea, reportPerimeter };
```

&emsp;将它们导入 main.mjs 时，如果我们尝试使用：

```javascript
import { name, draw, reportArea, reportPerimeter } from './modules/square.mjs';
import { name, draw, reportArea, reportPerimeter } from './modules/circle.mjs';
import { name, draw, reportArea, reportPerimeter } from './modules/triangle.mjs';
```

&emsp;浏览器会抛出一个错误，例如 "SyntaxError: redeclaration of import name"（Firefox）。

&emsp;相反，我们需要重命名导入，使它们是唯一的：

```javascript
import { name as squareName,
         draw as drawSquare,
         reportArea as reportSquareArea,
         reportPerimeter as reportSquarePerimeter } from './modules/square.mjs';

import { name as circleName,
         draw as drawCircle,
         reportArea as reportCircleArea,
         reportPerimeter as reportCirclePerimeter } from './modules/circle.mjs';

import { name as triangleName,
        draw as drawTriangle,
        reportArea as reportTriangleArea,
        reportPerimeter as reportTrianglePerimeter } from './modules/triangle.mjs';
```

&emsp;请注意，也可以在模块文件中解决问题，例如：

```javascript
// in square.mjs
export { name as squareName,
         draw as drawSquare,
         reportArea as reportSquareArea,
         reportPerimeter as reportSquarePerimeter };
```

```javascript
// in main.mjs
import { squareName, drawSquare, reportSquareArea, reportSquarePerimeter } from '/js-examples/modules/renaming/modules/square.mjs';
```

&emsp;它也会起作用。你使用什么样的风格取决于你，但是单独保留模块代码并在导入中进行更改可能更有意义。当你从没有任何控制权的第三方模块导入时，这尤其有意义。

## 创建模块对象

&emsp;上面的方法工作的挺好，但是有一点点混乱、亢长。一个更好的解决方是，导入每一个模块功能到一个模块功能对象上。可以使用以下语法形式：

```javascript
import * as Module from '/modules/module.mjs';
```

&emsp;这将获取 module.mjs 中所有可用的导出，并使它们可以作为对象模块的成员使用，从而有效地为其提供自己的命名空间。例如：

```javascript
Module.function1()
Module.function2()
etc.
```

```javascript
import * as Canvas from './modules/canvas.mjs';

import * as Square from '/./modules/square.mjs';
import * as Circle from './modules/circle.mjs';
import * as Triangle from './modules/triangle.mjs';
```

&emsp;在每种情况下，你现在可以访问指定对象名称下面的模块导入。

```javascript
let square1 = Square.draw(myCanvas.ctx, 50, 50, 100, 'blue');
Square.reportArea(square1.length, reportList);
Square.reportPerimeter(square1.length, reportList);
```

&emsp;因此，你现在可以像以前一样编写代码（只要你在需要时包含对象名称），并且导入更加整洁。

## 模块与类（class）

&emsp;正如我们之前提到的那样，你还可以导出和导入类；这是避免代码冲突的另一种选择，如果你已经以面向对象的方式编写了模块代码，那么它尤其有用。

```javascript
class Square {
  constructor(ctx, listId, length, x, y, color) {
    ...
  }

  draw() {
    ...
  }

  ...
}
```

&emsp;然后我们导出：

```javascript
export { Square };
```

&emsp;在 main.mjs 中，我们像这样导入它：

```javascript
import { Square } from './modules/square.mjs';
```

&emsp;然后使用该类绘制我们的方块：

```javascript
let square1 = new Square(myCanvas.ctx, myCanvas.listId, 50, 50, 100, 'blue');
square1.draw();
square1.reportArea();
square1.reportPerimeter();
```

## 合并模块

&emsp;有时你会想要将模块聚合在一起。你可能有多个级别的依赖项，你希望简化事物，将多个子模块组合到一个父模块中。这可以使用父模块中以下表单的导出语法：

```javascript
export * from 'x.mjs'
export { name } from 'x.mjs'
```

## 动态加载模块

&emsp;浏览器中可用的 JavaScript 模块功能的最新部分是动态模块加载。这允许你仅在需要时动态加载模块，而不必预先加载所有模块。这有一些明显的性能优势；让我们继续阅读，看看它是如何工作的。

&emsp;这个新功能允许你将 import() 作为函数调用，将其作为参数传递给模块的路径。它返回一个 promise，它用一个模块对象来实现，让你可以访问该对象的导出，例如：

```javascript
import('/modules/myModule.mjs')
  .then((module) => {
    // Do something with the module.
  });
```

```javascript
squareBtn.addEventListener('click', () => {
  import('/js-examples/modules/dynamic-module-imports/modules/square.mjs').then((Module) => {
    let square1 = new Module.Square(myCanvas.ctx, myCanvas.listId, 50, 50, 100, 'blue');
    square1.draw();
    square1.reportArea();
    square1.reportPerimeter();
  })
});
```

## 参考链接
**参考链接:🔗**
+ [JavaScript 参考](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference)
+ [JavaScript 指南](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Guide)
+ [JavaScript Related Topics](https://developer.mozilla.org/zh-CN/docs/Learn/JavaScript)
+ [JavaScript 主题学习区](https://developer.mozilla.org/zh-CN/docs/learn/JavaScript)
+ [重新介绍 JavaScript（JS 教程）](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/A_re-introduction_to_JavaScript)
