# JavaScript 学习过程记录：let/var、Scope、对象、原型/原型链/原型式继承、class/class 继承

> &emsp;动态编程语言是指可在运行阶段时执行那些在编译阶段执行的操作的编程语言。比如，在 JavaScript 中， 我们可以在程序运行时改变变量的类型，或者为一个对象增加一个新属性或者方法。这正好与静态编程语言相反，在静态编程语言的运行阶段，一般是无法执行这些改变的。

## 什么是 JavaScript ?

&emsp;JavaScript 是一种具有函数优先的轻量级、解释型或即时编译型的动态编程语言。JavaScript 是一门基于原型、头等函数的语言，是一门多范式的语言，它支持面向对象程序设计、指令式编程以及函数式编程。它提供语法来操控文本、数组、日期以及正则表达式等，不支持 I/O，比如网络、存储和图形等，但这些都可以由它的宿主环境提供支持。它已经由 ECMA（欧洲电脑制造商协会）通过 ECMAScript 实现语言的标准化。它被世界上的绝大多数网站所使用，也被世界主流浏览器（Chrome、IE、Firefox、Safari、Opera）支持。

&emsp;原型编程 是一种 面向对象编程 的风格。在这种风格中，我们不会显式地定义类，而会通过向其它类的实例（对象）中添加属性和方法来创建类，甚至偶尔使用空对象创建类。简单来说，这种风格是在不定义 class 的情况下创建一个 object。

&emsp;JavaScript 与 Java 在名字或语法上都有很多相似性（同为 1995 年发行），但这两门编程语言从设计之初就有很大的不同，JavaScript 的语言设计主要受到了 Self（一种基于原型的编程语言）和 Scheme（一门函数式编程语言）的影响。在语法结构上它又与 C 语言有很多相似，支持许多 C 语言的结构化编程语法，例如 if 条件语句、switch 语句、while 循环、do-while 循环等。但作用域是一个例外：JavaScript 在过去只支持使用 var 关键字来定义变量的函数作用域。ECMAScript 2015 加入了 let 关键字来支持块级作用域。意味着 JavaScript 现在既支持函数作用域又支持块级作用域。和 C 语言一样，JavaScript 中的表达式和语句是不同的。有一点格式上的不同，JavaScript 支持自动在语句末添加分号，因此允许忽略语句末尾的分号。

&emsp;在客户端，JavaScript 在传统意义上被实现为一种解释语言，但在最近，它已经可以被即时编译（JIT）执行。随着最新的 HTML5 和 CSS3 语言标准的推行它还可用于游戏、桌面和移动应用程序的开发和在服务器端网络环境运行如：Node.js。

&emsp;JavaScript 被归类为解释型语言，因为目前主流的引擎都是每次执行时加载代码并解译。V8 是将所有代码解译后再开始执行，其他引擎则是逐行解译（SpiderMonkey 会将解译过的指令暂存，以提高性能，称为即时编译），但由于 V8 的核心部分多数用 JavaScript 撰写（而 SpiderMonkey 是用C++），因此在不同的测试上，两者性能互有优劣。与其相对应的是编译型语言，例如 C 语言，以编译型语言编写的程序在执行之前，必须经过编译，将代码编译为机器代码，再加以执行。[JavaScript-维基百科](https://zh.wikipedia.org/wiki/JavaScript)

&emsp;JavaScript (JS) 是一种编程语言，为通常用于客户端（client-side）的网页动态脚本，不过，也常通过像 Node.js 这样的包，用于服务器端（server-side）。

&emsp;不应该把 JavaScript 和 Java 混淆。"Java" 和 "JavaScript" 都是 Oracle 公司在美国和其他国家注册的商标，但是这两种编程语言在语法、语义和使用方面都明显不同。

&emsp;Brendan Eich （彼时受雇于 Netscape ）为服务器端构想的语言 JavaScript，不久便在 1995 年 9 月被加入 Netscape Navigator 2.0。JavaScript 很快获得了成功，而 Internet Explorer 3.0 也在 1996 年 8 月，引入了对 JavaScript 的支持，冠以 JScript 之名。

&emsp;1996 年 11 月，Netscape 开始与 ECMA 国际化组织合作以使 JavaScript 成为行业标准。从此以后，标准化的 JavaScript 就被称为 ECMAScript 并由 ECMA-262 规范，其最新版（第八版）在 2017 年 6 月生效。

&emsp;JavaScript 通常用于浏览器，使开发者能通过 DOM 来操纵网页内容、或透过 AJAX 与 IndexedDB 来操作数据；还可以用它在 canvas 上面绘图、通过各种 APIs 与运行浏览器的各种设备交互……等等。由于近年来的发展、以及各浏览器的 APIs 性能改善，JavaScript 成了世界上最常用的编程语言之一。随着除浏览器外最流行的跨平台 JavaScript 运行环境 —— Node.js 平台的成功而大大提升。Node.js 使开发者可以在 PC 上使用 JavaScript 作为脚本语言以自动化处理和构建功能完备的 HTTP 和 Web Sockets (en-US) 服务器。

&emsp;JavaScript（缩写：JS）是一门完备的 动态编程语言。当应用于 HTML 文档时，可为网站提供动态交互特性。由布兰登·艾克（Brendan Eich，Mozilla 项目、Mozilla 基金会和 Mozilla 公司的联合创始人）发明。

&emsp;JavaScript 的应用场合极其广泛，简单到幻灯片、照片库、浮动布局和响应按钮点击，复杂到游戏、2D/3D 动画、大型数据库驱动程序等等。

&emsp;JavaScript 相当简洁，却非常灵活。开发者们基于 JavaScript core 编写了大量实用工具，可以使开发工作事半功倍。其中包括：

+ 浏览器应用程序接口（API）—— 浏览器内置的 API 提供了丰富的功能，比如：动态创建 HTML 和设置 CSS 样式、从用户的摄像头采集处理视频流、生成 3D 图像与音频样本等等。
+ 第三方 API —— 让开发者可以在自己的站点中整合其它内容提供者（Twitter、Facebook 等）提供的功能。
+ 第三方框架和库 —— 用来快速构建网站和应用。

&emsp;JavaScript 是一种脚本，一门编程语言，它可以在网页上实现复杂的功能，网页展现给你的不再是简单的静态信息，而是实时的内容更新，交互式的地图，2D/3D 动画，滚动播放的视频等等。JavaScript 怎能缺席。它是标准 Web 技术蛋糕的第三层，另外两层是 HTML 和 CSS。

&emsp;解释代码 vs 编译代码:

&emsp;作为程序员，你或许听说过这两个术语：解释（interpret）和 编译(compile)。在解释型语言中，代码自上而下运行，且实时返回运行结果。代码在由浏览器执行前，不需要将其转化为其他形式。代码将直接以文本格式（text form）被接收和处理。

&emsp;相对的，编译型语言需要先将代码转化（编译）成另一种形式才能运行。比如 C/C++ 先被编译成汇编语言，然后才能由计算机运行。程序将以二进制的格式运行，这些二进制内容是由程序源代码产生的。

&emsp;JavaScript 是轻量级解释型语言。浏览器接受到  JavaScript 代码，并以代码自身的文本格式运行它。技术上，几乎所有 JavaScript 转换器都运用了一种叫做即时编译（just-in-time compiling）的技术；当 JavaScript 源代码被执行时，它会被编译成二进制的格式，使代码运行速度更快。尽管如此，JavaScript 仍然是一门解释型语言，因为编译过程发生在代码运行中，而非之前。

&emsp;服务器端代码 vs 客户端代码

&emsp;你或许还听说过服务器端（server-side）和 客户端（client-side）代码这两个术语，尤其是在 web 开发时。客户端代码是在用户的电脑上运行的代码，在浏览一个网页时，它的客户端代码就会被下载，然后由浏览器来运行并展示。这就是客户端 JavaScript。

&emsp;而服务器端代码在服务器上运行，接着运行结果才由浏览器下载并展示出来。流行的服务器端 web 语言包括：PHP、Python、Ruby、ASP.NET 以及...... JavaScript！JavaScript 也可用作服务器端语言，比如现在流行的 Node.js 环境。

&emsp;动态代码 vs 静态代码

&emsp;"动态" 一词既适用于客户端 JavaScript，又适用于描述服务器端语言。是指通过按需生成新内容来更新 web 页面 / 应用，使得不同环境下显示不同内容。服务器端代码会在服务器上动态生成新内容，例如从数据库中提取信息。而客户端 JavaScript 则在用户端浏览器中动态生成新内容，比如说创建一个新的 HTML 表格，用从服务器请求到的数据填充，然后在网页中向用户展示这个表格。两种情况的意义略有不同，但又有所关联，且两者（服务器端和客户端）经常协同作战。

&emsp;没有动态更新内容的网页叫做 "静态" 页面，所显示的内容不会改变。

&emsp;JavaScript 是一种具有函数优先的轻量级，解释型或即时编译型的编程语言。虽然它是作为开发 Web 页面的脚本语言而出名的，但是它也被用到了很多非浏览器环境中，例如 Node.js、Apache CouchDB 和 Adobe Acrobat。JavaScript 是一种基于原型编程、多范式的动态脚本语言，并且支持面向对象、命令式和声明式（如函数式编程）风格。

&emsp;JavaScript 的标准是 ECMAScript。截至 2012 年，所有的现代浏览器都完整的支持 ECMAScript 5.1，旧版本的浏览器至少支持 ECMAScript 3 标准。2015 年 6 月 17 日，ECMA 国际组织发布了 ECMAScript 的第六版，该版本正式名称为 ECMAScript 2015，但通常被称为 ECMAScript 6 或者 ES6。自此，ECMAScript 每年发布一次新标准。

&emsp;JavaScript®（通常简写为 JS）是一种轻量的、解释性的、面向对象的头等函数语言，其最广为人知的应用是作为网页的脚本语言，但同时它也在很多非浏览器环境下使用。JS 是一种动态的基于原型和多范式的脚本语言，支持面向对象、命令式和函数式的编程风格。

&emsp;JavaScript 运行在网页的客户端，能被用来设计和编程网页在事件发生时的行为。JavaScript 不仅易学而且强大，因此广泛用于对网页的控制。

&emsp;与流行的误解相反，JavaScript 并不是 "解释性 Java"。简单来说，JavaScript 是一个动态脚本语言，支持基于原型的对象构造。其基本语法被设计地与 Java 和 C++ 接近，来减少学习语言所需要的新概念。语言结构，如条件语句（if）、循环（for，while）、分支（switch）、异常捕获（try...catch）等和这些语言一致或者很接近。

&emsp;JavaScript 既是一个 面向过程的语言 又是一个 面向对象的语言。在 JavaScript 中，通过在运行时给空对象附加方法和属性来创建对象，与编译语言如 C++ 和 Java 中常见的通过语法来定义类相反。对象构造后，它可以用作是创建相似对象的原型。

&emsp;JavaScript 的动态特性包括运行时构造对象、可变参数列表、函数变量、动态脚本执行（通过 eval）、对象内枚举（通过 for ... in）和源码恢复（JavaScript 程序可以将函数反编译回源代码）。

## 有哪些 JavaScript 的实现?

&emsp;mozilla.org 上托管了两个 JavaScript 实现。首个 JavaScript 由网景公司的 Brendan Eich 创建，并不断地更新以符合 ECMA-262 Edition 5 及其之后的标准版本。这个引擎，代号 SpiderMonkey，是由 C/C++ 语言开发的。而 Rhino 引擎，主要由 Norris Boyd（同样也是在网景公司）创建，则是一个 Java 语言开发的 JavaScript 实现。与 SpiderMonkey 类似，Rhino 符合 ECMA-262 Edition 5 标准。

&emsp;有很多优化技术如 TraceMonkey (Firefox 3.5)、JägerMonkey (Firefox 4) 和 IonMonkey 被不断添加到了 SpiderMonkey JavaScript 引擎。并且提升 JavaScript 执行效率的工作一直在进行。

&emsp;除了以上实现，还有其他一些流行的 JavaScript 引擎，如：

+ Google 的 V8，在 Google Chrome 浏览器和较新的 Opera 浏览器中使用。这同时也是 Node.js 使用的引擎。
+ JavaScriptCore (SquirrelFish/Nitro)，被用在了一些 WebKit 浏览器如 Apple Safari。
+ Carakan，用在旧版本 Opera 中。
+ The Chakra 引擎，用在 Internet Explorer 中（尽管它实现的语言被正式地称作为 "JScript" 来避免商标问题）。

&emsp;每个 mozilla.org 的 JavaScript 引擎都提供了公用 API 使程序开发者能将其 JavaScript 嵌入自己的软件中。目前最常见的 JavaScript 宿主环境是网页浏览器。浏览器一般通过 API 创建 "宿主对象" 来负责将 DOM 反射到 JavaScript 中。

&emsp;另一个常见的 JavaScript 应用是作为服务端脚本语言。JavaScript 服务器提供宿主对象代表 HTTP 请求和响应，随后可以通过 JavaScript 程序来动态的生成 Web 页面。Node.js 便是一个流行的例子。

&emsp;JavaScript 目前广泛应用于众多知名应用中，对于网页和移动开发者来说，深入理解 JavaScript 就尤为必要。

&emsp;我们有必要先从这门语言的历史谈起。在 1995 年 Netscape 一位名为 Brendan Eich 的工程师创造了 JavaScript，随后在 1996 年初，JavaScript 首先被应用于 Netscape 2 浏览器上。最初的 JavaScript 名为 LiveScript，但是因为一个糟糕的营销策略而被重新命名，该策略企图利用 Sun Microsystem 的 Java 语言的流行性，将它的名字从最初的 LiveScript 更改为 JavaScript —— 尽管两者之间并没有什么共同点。这便是之后混淆产生的根源。

&emsp;几个月后，Microsoft 随 IE 3 发布推出了一个与之基本兼容的语言 JScript。又过了几个月，Netscape 将 JavaScript 提交至 Ecma International（一个欧洲标准化组织），ECMAScript 标准第一版便在 1997 年诞生了，随后在 1999 年以 ECMAScript 第三版的形式进行了更新，从那之后这个标准没有发生过大的改动。由于委员会在语言特性的讨论上发生分歧，ECMAScript 第四版尚未推出便被废除，但随后于 2009 年 12 月发布的 ECMAScript 第五版引入了第四版草案加入的许多特性。第六版标准已经于 2015 年 6 月发布。

> &emsp;备注：由于这种用法更常见，从这里开始，将使用 JavaScript 来指代 ECMAScript。

&emsp;与大多数编程语言不同，JavaScript 没有输入或输出的概念。它是一个在宿主环境（host environment）下运行的脚本语言，任何与外界沟通的机制都是由宿主环境提供的。浏览器是最常见的宿主环境，但在非常多的其他程序中也包含 JavaScript 解释器，如 Adobe Acrobat、Adobe Photoshop、SVG 图像、Yahoo! 的 Widget 引擎，Node.js 之类的服务器端环境，NoSQL 数据库（如开源的 Apache CouchDB）、嵌入式计算机，以及包括 GNOME （注：GNU/Linux 上最流行的 GUI 之一）在内的桌面环境等等。

&emsp;JavaScript 是一种多范式的动态语言，它包含类型、运算符、标准内置（built-in）对象和方法。它的语法来源于 Java 和 C，所以这两种语言的许多语法特性同样适用于 JavaScript。JavaScript 通过原型链而不是类来支持面向对象编程（有关 ES6 类的内容参考这里 [Classes](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Classes)，有关对象原型参考见此 [继承与原型链](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Inheritance_and_the_prototype_chain)）。JavaScript 同样支持函数式编程 —— 因为它们也是对象，函数也可以被保存在变量中，并且像其他对象一样被传递。

&emsp;一个变量，就是一个用于存放数值的容器。这个数值可能是一个用于累加计算的数字，或者是一个句子中的字符串。变量的独特之处在于它存放的数值是可以改变的。变量的另一个特性就是它们能够存储任何的东西 -- 不只是字符串和数字。变量可以存储更复杂的数据，甚至是函数。

&emsp;我们说，变量是用来存储数值的，那么有一个重要的概念需要区分。变量不是数值本身，它们仅仅是一个用于存储数值的容器。你可以把变量想象成一个个用来装东西的纸箱子。

&emsp;千万不要把两个概念弄混淆了，"一个变量存在，但是没有数值" 和 "一个变量并不存在" — 他们完全是两回事 — 在前面你看到的盒子的类比中，不存在意味着没有可以存放变量的 "盒子"。没有定义的值意味着有一个 "盒子"，但是它里面没有任何值。

## var 与 let 的区别

&emsp;此时，你可能会想："为什么我们需要两个关键字来定义变量?"，"为什么有 var 和 let 呢？"。

&emsp;原因是有些历史性的。回到最初创建 JavaScript 时，是只有 var 的。 在大多数情况下，这种方法可以接受，但有时在工作方式上会有一些问题 —— 它的设计会令人困惑或令人讨厌。因此 let 是在现代版本中的 JavaScript 创建的一个新的关键字，用于创建与 var 工作方式有些不同的变量，解决了过程中的问题。

&emsp;首先，如果你编写一个声明并初始化变量的多行 JavaScript 程序，你可以在初始化一个变量之后用 var 声明它，它仍然可以工作。 例如：

```javascript
myName = 'Chris';

function logName() {
  console.log(myName);
}

logName();

var myName;
```

> &emsp;Note: 只有在 web 文档中运行多行 JavaScript 时才会有这种效果，当在 JavaScript 控制台中键入单独的行，这将不起作用。

&emsp;这是由于变量的提升，但提升操作不再适用于 let。如果将上面例子中的 var 替换成 let 将不起作用并引起一个错误。这是一件好事 —— 因为初始化后再声明一个变量会使代码变得混乱和难以理解。其次，当你使用 var 时，可以根据需要多次声明相同名称的变量，但是 let 不能。 以下将有效：

```javascript
var myName = 'Chris';
var myName = 'Bob';
```

&emsp;但是以下内容会在第二行引发错误：

```javascript
let myName = 'Chris';
let myName = 'Bob';
```

&emsp;你必须这样做：

```javascript
let myName = 'Chris';
myName = 'Bob';
```

&emsp;同样，这是一个明智的语言决定。没有理由重新声明变量 —— 这只会让事情变得更加混乱。出于这些以及其他原因，我们建议你在代码中尽可能多地使用 let，而不是 var。因为没有理由使用 var，除非你需要用代码支持旧版本的 Internet Explorer (它直到第 11 版才支持 let，现代的 Windows Edge 浏览器支持的很好)。

&emsp;JavaScript 是一种 "动态类型语言"，这意味着不同于其他一些语言(注：如 C、JAVA)，你不需要指定变量将包含什么数据类型（例如 number 或 string）。例如，如果你声明一个变量并给它一个带引号的值，浏览器就会知道它是一个字符串：

```javascript
let myString = 'Hello';
```

&emsp;即使它包含数字，但它仍然是一个字符串，所以要小心：

```javascript
let myNumber = '500'; // oops, this is still a string
typeof myNumber;
myNumber = 500; // much better — now this is a number
typeof myNumber
```

&emsp;使用了一个名为 typeof 的特殊的操作符 —— 它会返回所传递给它的变量的数据类型。第一次在上面的代码中调用它，它应该返回 string，因为此时 myNumber 变量包含一个字符串 '500'。

> &emsp;Note：你可能会看到有些人在他们的代码中使用 `==` 和 `!=` 来判断相等和不相等，这些都是 JavaScript 中的有效运算符，但它们与 `===` 和 `!==`不同，前者测试值是否相同， 但是数据类型可能不同，而后者的严格版本测试值和数据类型是否相同。严格的版本往往导致更少的错误，所以我们建议使用这些严格的版本。

&emsp;当你知道字符串中的子字符串开始的位置，以及想要结束的字符时，`slice()` 可以用来提取它。尝试以下：

```javascript
let browserType = 'mozilla';
browserType.slice(0,3);
```

&emsp;这时返回 "moz" —— 第一个参数是开始提取的字符位置，第二个参数是提取的最后一个字符的后一个位置。所以提取从第一个位置开始，直到但不包括最后一个位置。（此例中）你也可以说第二个参数等于被返回的字符串的长度。

&emsp;此外，如果你知道要在某个字符之后提取字符串中的所有剩余字符，则不必包含第二个参数，而只需要包含要从中提取的字符位置，便可提取出字符串中的其余字符。尝试以下：

```javascript
browserType.slice(2);
```

&emsp;这返回 "zilla" —— 这是因为 2 的字符位置是字母 z，并且因为没有包含第二个参数，所以返回的子字符串是字符串中的所有剩余字符。

&emsp;当我们尝试添加（或连接）一个字符串和一个数字时，会发生什么？让我们在我们的控制台中尝试一下:

```javascript
'Front ' + 242;
```

&emsp;你可能会认为这会抛出一个错误，但它运行得很好。试图将字符串表示为一个数字并不是很讲的通，但是用数字表示一个字符串则不然，因此浏览器很聪明地将数字转换为字符串，并将这两个字符串连接在一起。

&emsp;你甚至可以用两个数字来这么操作 —— 你可以通过用引号将数字包装成一个字符串。尝试以下方法（我们使用 typeof 操作符来检查变量是一个数字还是一个字符串）:

```javascript
let myDate = '19' + '67';
typeof myDate;
```

&emsp;如果你有一个数值变量，你想要将其转换为字符串，并且不改变其他地方，或者你想将一个字符串转换为一个数字而不改变其其他地方，那么你可以使用以下两个构造:

+ 如果可以的话， Number 对象将把传递给它的任何东西转换成一个数字。试一试:

```javascript
let myString = '123';
let myNum = Number(myString);
typeof myNum;
```

+ 另一方面，每个数字都有一个名为 toString() 的方法，它将把它转换成等价的字符串。试一试:

```javascript
let myNum = 123;
let myString = myNum.toString();
typeof myString;
```

&emsp;通常，你会看到一个包含在一个长长的字符串中的原始数据，你可能希望将有用的项目分成更有用的表单，然后对它们进行处理，例如将它们显示在数据表中。为此，我们可以使用 split() 方法。在其最简单的形式中，这需要一个参数：你要将字符串分隔的字符，并返回分隔符之间的子串，作为数组中的项。

```javascript
let myData = 'Manchester,London,Liverpool,Birmingham,Leeds,Carlisle';
let myArray = myData.split(',');
myArray;
```

&emsp;也可以使用 join() 方法进行相反的操作。尝试以下：

```javascript
let myNewString = myArray.join(',');
myNewString;
```

&emsp;将数组转换为字符串的另一种方法是使用 toString() 方法。toString() 可以比 join() 更简单，因为它不需要一个参数，但更有限制。使用 join() 可以指定不同的分隔符（尝试使用与逗号不同的字符运行）。

```javascript
let dogNames = ["Rocket","Flash","Bella","Slugger"];
dogNames.toString(); // Rocket,Flash,Bella,Slugger
```

&emsp;我们还没有涵盖添加和删除数组元素，现在让我们来看看。我们将使用上面提到的 myArray 数组。 如果你尚未遵循该部分，请先在控制台中创建数组：

```javascript
let myArray = ['Manchester', 'London', 'Liverpool', 'Birmingham', 'Leeds', 'Carlisle'];
```

&emsp;首先，要在数组末尾添加或删除一个项目，我们可以使用 push() 和 pop()。

&emsp;让我们先使用 push() —— 注意，你需要添加一个或多个要添加到数组末尾的元素。尝试下面的代码：

```javascript
myArray.push('Cardiff');
myArray;
myArray.push('Bradford', 'Brighton');
myArray;
```

&emsp;当方法调用完成时，将返回数组的新长度。如果要将新数组长度存储在变量中。例如：

```javascript
var newLength = myArray.push('Bristol');
myArray;
newLength;
```

&emsp;从数组中删除最后一个元素的话直接使用 pop() 就可以。例如：

```javascript
myArray.pop();
```

&emsp;当方法调用完成时，将返回已删除的项目。你也可以这样做：

```javascript
let removedItem = myArray.pop();
myArray;
removedItem;
```

&emsp;unshift() 和 shift() 从功能上与 push() 和 pop() 完全相同，只是它们分别作用于数组的开始，而不是结尾。

&emsp;首先 unshift() —— 尝试一下这个命令：

```javascript
myArray.unshift('Edinburgh');
myArray;
```

&emsp;现在 shift() —— 尝试一下：

```javascript
let removedItem = myArray.shift();
myArray;
removedItem;
```

## JavaScript 代码块

&emsp;JavaScript 有许多内置的函数，可以让你做很多有用的事情，而无需自己编写所有的代码。事实上，许多你调用（运行或者执行的专业词语）浏览器内置函数时调用的代码并不是使用 JavaScript 来编写 —— 大多数调用浏览器后台的函数的代码，是使用像 C++ 这样更低级的系统语言编写的，而不是像 JavaScript 这样的 web 编程语言。

&emsp;严格说来，内置浏览器函数并不是函数 —— 它们是方法。这听起来有点可怕和令人困惑，但不要担心 —— 函数和方法在很大程度上是可互换的，至少在我们的学习阶段是这样的。二者区别在于方法是在对象内定义的函数。浏览器内置函数（方法）和变量（称为属性）存储在结构化对象内，以使代码更加高效，易于处理。

&emsp;你可能会以稍微不同的方式看到定义和调用的函数。到目前为止，我们刚刚创建了如下函数：

```javascript
function myFunction() {
  alert('hello');
}
```

&emsp;但是你也可以创建一个没有名称的函数：

```javascript
function() {
  alert('hello');
}
```

&emsp;这个函数叫做匿名函数 — 它没有函数名! 它也不会自己做任何事情。你通常将匿名函数与事件处理程序一起使用，例如，如果单击相关按钮，以下操作将在函数内运行代码：

```javascript
var myButton = document.querySelector('button');

myButton.onclick = function() {
  alert('hello');
}
```

&emsp;你将主要使用匿名函数来运行负载的代码以响应事件触发（如点击按钮） - 使用事件处理程序。

&emsp;匿名函数也称为函数表达式。函数表达式与函数声明有一些区别。函数声明会进行声明提升（declaration hoisting），而函数表达式不会。

> &emsp;Note：参数有时称为参数（arguments），属性（properties）或甚至属性（attributes）。

&emsp;还应该注意，有时参数不是必须的 —— 你不必指定它们。如果没有，该功能一般会采用某种默认行为。作为示例，数组 join() 函数的参数是可选的：

```javascript
var myArray = ['I', 'love', 'chocolate', 'frogs'];
var madeAString = myArray.join(' ');
// returns 'I love chocolate frogs'
var madeAString = myArray.join();
// returns 'I,love,chocolate,frogs'
```

&emsp;如果没有包含参数来指定加入/分隔符，默认情况下会使用逗号。

### Scope（作用域）

&emsp;当前的执行上下文。值和表达式在其中 "可见" 或可被访问到的上下文。如果一个变量或者其他表达式不 "在当前的作用域中"，那么它就是不可用的。作用域也可以根据代码层次分层，以便子作用域可以访问父作用域，通常是指沿着链式的作用域链查找，而不能从父作用域引用子作用域中的变量和引用。

&emsp;当然，一个 Function 将生成一个闭包（通常是返回一个函数引用），这个函数引用从外部作用域（在当前环境下）可以访问闭包内部的作用域。例如，下面的代码是无效的，并不是闭包的形式）：

```javascript
function exampleFunction() {
    var x = "declared inside function";  // x 只能在 exampleFunction 函数中使用
    console.log("Inside function");
    console.log(x);
}

console.log(x);  // 引发 error
```

&emsp;但是，由于变量在函数外被声明为全局变量，因此下面的代码是有效的（当前作用域不存在的变量和引用，就沿着作用域链继续寻找）：

```javascript
var x = "declared outside function";

exampleFunction();

function exampleFunction() {
    console.log("Inside function");
    console.log(x);
}

console.log("Outside function");
console.log(x);
```

&emsp;英文原文中，只提到了闭包的简单特例，也就是父作用域引用子作用域的变量或者引用。这儿做一个补充，当一个函数（foo）执行返回一个内部函数（bar）引用时，bar 将会保存 foo 的作用域引用。例如：

```javascript
function foo() {
    const str = "bar in foo";
    return function bar() {
        return str;
    }
}

var fun = foo();
fun(); // "bar in foo"
```

### 函数作用域和冲突

&emsp;scope 即作用域 — 处理函数时一个非常重要的概念。当你创建一个函数时，函数内定义的变量和其他东西都在它们自己的单独的范围内，意味着它们被锁在自己独立的隔间中，不能被函数外的代码访问。所有函数的最外层被称为全局作用域，在全局作用域内定义的值可以在任意地方访问。

&emsp;JavaScript 由于各种原因而建立，但主要是由于安全性和组织性。有时你不希望变量可以在代码中的任何地方访问 - 你从其他地方调用的外部脚本可能会开始搞乱你的代码并导致问题，因为它们恰好与代码的其他部分使用了相同的变量名称，造成冲突。这可能是恶意的，或者是偶然的。

&emsp;例如，假设你有一个 HTML 文件，它调用两个外部 JavaScript 文件，并且它们都有一个使用相同名称定义的变量和函数：

```javascript
<!-- Excerpt from my HTML -->
<script src="first.js"></script>
<script src="second.js"></script>
<script>
  greeting();
</script>
```

```javascript
// first.js
let name = 'Chris';
function greeting() {
  alert('Hello ' + name + ': welcome to our company.');
}
```

```javascript
// second.js
let name = 'Zaptec';
function greeting() {
  alert('Our company is called ' + name + '.');
}
```

&emsp;这两个函数都使用 greeting() 形式调用，但是你只能访问到 first.js 文件的 greeting() 函数（第二个文件被忽视了）。另外，第二次尝试使用 let 关键字定义 name 变量导致了一个错误。

## JavaScript 对象/原型继承

&emsp;在 JavaScript 中，大多数事物都是对象，从作为核心功能的字符串和数组，到建立在 JavaScript 之上的浏览器 API。你甚至可以自己创建对象，将相关的函数和变量高效地封装打包成便捷的数据容器。对于进一步学习 JavaScript 语言知识而言，理解这种面向对象（object-oriented, OO）的特性是必不可少的。

&emsp;对象基础：对象是一个包含相关数据和方法的集合（通常由一些变量和函数组成，我们称之为对象里面的属性和方法）。

### 括号表示法

&emsp;另外一种访问属性的方式是使用括号表示法（bracket notation），替代这样的代码：

```javascript
person.age
person.name.first
```

&emsp;使用如下所示的代码：

```javascript
person['age']
person['name']['first']
```

&emsp;也可以通过括号表示法设置对象的成员的值，与点语法完全相同：

```javascript
person.age = 45
person['name']['last'] = 'Cratchit'
```

&emsp;这看起来很像访问一个数组的元素，从根本上来说是一回事儿，你使用了关联了值的名字，而不是索引去选择元素。难怪对象有时被称之为关联数组（associative array）了 —— 对象做了字符串到值的映射，而数组做的是数字到值的映射。


&emsp;设置成员并不意味着你只能更新已经存在的属性的值，你完全可以创建新的成员，尝试以下代码：

```javascript
person['eyes'] = 'hazel'
person.farewell = function() { alert("Bye everybody!") }
```

&emsp;现在你可以测试你新创建的成员：

```javascript
person['eyes']
person.farewell()
```

&emsp;括号表示法一个有用的地方是它不仅可以动态的去设置对象成员的值，还可以动态的去设置成员的名字。

&emsp;比如说，我们想让用户能够在他们的数据里存储自己定义的值类型，通过两个 input 框来输入成员的名字和值，通过以下代码获取用户输入的值：

```javascript
var myDataName = nameInput.value
var myDataValue = nameValue.value
```

&emsp;我们可以这样把这个新的成员的名字和值加到 person 对象里：

```javascript
person[myDataName] = myDataValue
```

&emsp;为了测试这个功能，尝试在你的代码里添加以下几行，就在 person 对象的右花括号的下面：

```javascript
var myDataName = 'height'
var myDataValue = '1.75m'
person[myDataName] = myDataValue
```

&emsp;然后我们可以使用如下的两行代码进行测试：

```javascript
alert(person.height1); // 因为 person 当前没有 height1 这个成员变量，所以 alert 出来的是 Underfine
alert(person.height1); // 上面我们用括号表示法给 person 添加了 height 成员变量，所以 alert 出来的是 1.75m
```

&emsp;这是使用点表示法无法做到的，点表示法只能接受字面量的成员的名字，不接受变量作为名字。

&emsp;发现如下也是可以的，和 `person['eyes'] = 'hazel'` 实现的效果相同，给 person 添加了一个新的成员变量，当然这是完全指定 height1 成员变量的名字，和上面完全自己手动输入成员名的场景是不同的。 

```javascript
person.height1 = '1.88m';
alert(person.height1);
```

&emsp;OOP 的基本思想是：在程序里，我们通过使用对象去构建现实世界的模型，把原本很难（或不可能）被使用的功能，简单化并提供出来，以供访问。

&emsp;对象可以包含相关的数据和代码，这些数据和代码用于表示你所建造的模型是什么样子，以及拥有什么样的行为或功能。对象包（object package，或者叫命名空间 namespace）存储（官方用语：封装）着对象的数据（常常还包括函数），使数据的组织和访问变得更容易了；对象也常用作数据存储体（data stores），用于在网络上运输数据，十分便捷。

&emsp;在一些面向对象的语言中，我们用类（class）的概念去描述一个对象（你在下面就能看到 JavaScript 使用了一个完全不同的术语）- 类并不完全是一个对象，它更像是一个定义对象特质的模板。

&emsp;当一个对象需要从类中创建出来时，类的构造函数就会运行来创建这个实例。这种创建对象的过程我们称之为实例化 - 实例对象被类实例化。

> &emsp;note：多态 —— 这个高大上的词正是用来描述多个对象拥有实现共同方法的能力。

&emsp;有些人认为 JavaScript 不是真正的面向对象的语言，比如它没有像许多面向对象的语言一样有用于创建 class 类的声明。JavaScript 用一种称为构建函数的特殊函数来定义对象和它们的特征。构建函数非常有用，因为很多情况下你不知道实际需要多少个对象（实例）。构建函数提供了创建你所需对象（实例）的有效方法，将对象的数据和特征函数按需联结至相应对象。

&emsp;不像 "经典" 的面向对象的语言，从构建函数创建的新实例的特征并非全盘复制，而是通过一个叫做原形链的参考链链接过去的。（Object prototypes），所以这并非真正的实例，严格的讲， JavaScript 在对象间使用和其它语言的共享机制不同。

> &emsp;note："经典" 的面向对象的语言并非好事，就像上面提到的，OOP 可能很快就变得非常复杂，JavaScript 找到了在不变的特别复杂的情况下利用面向对象的优点的方法。

&emsp;让我们来看看 JavaScript 如何通过构建函数对象来创建类。让我们看看如何通过一个普通的函数定义一个 "人"：

```javascript
function createNewPerson(name) {
  var obj = {};
  obj.name = name;
  obj.greeting = function () {
    alert('Hi! I\'m ' + this.name + '.');
  }
  return obj;
}
```

&emsp;现在可以通过调用这个函数创建一个新的叫 salva 的人：

```javascript
var salva = createNewPerson('salva');
salva.name;
salva.greeting();
```

&emsp;上述代码运行良好，但是有点冗长；如果我们知道如何创建一个对象，就没有必要创建一个新的空对象并且返回它。幸好 JavaScript 通过构建函数提供了一个便捷的方法，方法如下：

```javascript
function Person(name) {
  this.name = name;
  this.greeting = function() {
    alert('Hi! I\'m ' + this.name + '.');
  };
}
```

&emsp;这个构建函数是 JavaScript 版本的类。你会发现，它只定义了对象的属性和方法，除了没有明确创建一个对象和返回任何值之外，它有了我们期待的函数所拥有的全部功能。这里使用了 this 关键词，即无论是该对象的哪个实例被这个构建函数创建，它的 name 属性就是传递到构建函数形参 name 的值，它的 greeting() 方法中也将使用相同的传递到构建函数形参 name 的值。

> &emsp;note：一个构建函数通常是大写字母开头，这样便于区分构建函数和普通函数。

&emsp;那如何调用构建函数创建新的实例呢？

```javascript
var person1 = new Person('Bob');
var person2 = new Person('Sarah');
```

&emsp;关键字 new 跟着一个含参函数，用于告知浏览器我们想要创建一个对象，非常类似函数调用，并把结果保存到变量中。每个示例类都是根据 Person 构建函数的方式定义的。

&emsp;当新的对象被创立, 变量 person1 与 person2 有效地包含了以下值：

```javascript
{
  name : 'Bob',
  greeting : function() {
    alert('Hi! I\'m ' + this.name + '.');
  }
}

{
  name : 'Sarah',
  greeting : function() {
    alert('Hi! I\'m ' + this.name + '.');
  }
}
```

&emsp;值得注意的是每次当我们调用构造函数时，我们都会重新定义一遍 greeting()，这不是个理想的方法。为了避免这样，我们可以在原型里定义函数，后面我们会看到原型方式的构建。

### Object() 构造函数

&emsp;首先，能使用 Object() 构造函数来创建一个新对象。是的，一般对象都有构造函数，它创建了一个空的对象。

```javascript
var person1 = new Object();
```

&emsp;这样就在 person1 变量中存储了一个空对象。然后，可以根据需要，使用点或括号表示法向此对象添加属性和方法。

```javascript
person1.name = 'Chris';
person1['age'] = 38;
person1.greeting = function() {
  alert('Hi! I\'m ' + this.name + '.');
}
```

&emsp;还可以将对象文本传递给 Object() 构造函数作为参数，以便用属性/方法填充它。

```javascript
var person1 = new Object({
  name : 'Chris',
  age : 38,
  greeting : function() {
    alert('Hi! I\'m ' + this.name + '.');
  }
});
```

&emsp;JavaScript 有个内嵌的方法 create(), 它允许基于现有对象创建新的对象。

```javascript
var person2 = Object.create(person1);
```

&emsp;可以看到，person2 是基于 person1 创建的，它们具有相同的属性和方法。这非常有用，因为它允许你创建新的对象而无需定义构造函数。缺点是比起构造函数，浏览器在更晚的时候才支持 create() 方法（IE9, IE8 或甚至以前相比），加上一些人认为构造函数让你的代码看上去更整洁 —— 你可以在一个地方创建你的构造函数，然后根据需要创建实例，这让你能很清楚地知道它们来自哪里。

&emsp;但是，如果你不太担心对旧浏览器的支持，并且你只需要一个对象的一些副本，那么创建一个构造函数可能会让你的代码显得过度繁杂。这取决于你的个人爱好。有些人发现 create() 更容易理解和使用。

### 对象原型

&emsp;通过原型 (prototype) 这种机制，JavaScript 中的对象从其他对象继承功能特性；这种继承机制与经典的面向对象编程语言不同。下面将探讨这些差别，解释原型链如何工作，并了解如何通过 prototype 属性向已有的构造器添加方法。

#### 基于原型的语言？

&emsp;JavaScript 常被描述为一种基于原型的语言 (prototype-based language) —— 每个对象拥有一个原型对象，对象以其原型为模板、从原型继承方法和属性。原型对象也可能拥有原型，并从中继承方法和属性，一层一层、以此类推。这种关系常被称为原型链 (prototype chain)，它解释了为何一个对象会拥有定义在其他对象中的属性和方法。

&emsp;准确地说，这些属性和方法定义在 Object 的构造器函数（constructor functions）之上的 prototype 属性上，而非对象实例本身。

&emsp;在传统的 OOP 中，首先定义 "类"，此后创建对象实例时，类中定义的所有属性和方法都被复制到实例中。在 JavaScript 中并不如此复制 —— 而是在对象实例和它的构造器之间建立一个链接（它是 `__proto__` 属性，是从构造函数的 prototype 属性派生的），之后通过上溯原型链，在构造器中找到这些属性和方法。


> &emsp;note：理解对象的原型（可以通过 Object.getPrototypeOf(obj) 或者已被弃用的 `__proto__` 属性获得）与构造函数的 prototype 属性之间的区别是很重要的。前者是每个实例上都有的属性，后者是构造函数的属性。也就是说，Object.getPrototypeOf(new Foobar()) 和 Foobar.prototype 指向着同一个对象。

&emsp;在 javascript 中，函数可以有属性。每个函数都有一个特殊的属性叫作原型（prototype），正如下面所展示的。

```javascript
function doSomething(){}
console.log( doSomething.prototype );
// It does not matter how you declare the function, a function in javascript will always have a default prototype property.
var doSomething = function(){};
console.log( doSomething.prototype );
```

&emsp;doSomething 函数有一个默认的原型属性，输出如下：

```javascript
Object
constructor: ƒ doSomething()
[[Prototype]]: Object
constructor: ƒ Object()
hasOwnProperty: ƒ hasOwnProperty()
isPrototypeOf: ƒ isPrototypeOf()
propertyIsEnumerable: ƒ propertyIsEnumerable()
toLocaleString: ƒ toLocaleString()
toString: ƒ toString()
valueOf: ƒ valueOf()
__defineGetter__: ƒ __defineGetter__()
__defineSetter__: ƒ __defineSetter__()
__lookupGetter__: ƒ __lookupGetter__()
__lookupSetter__: ƒ __lookupSetter__()
__proto__: Object
get __proto__: ƒ __proto__()
set __proto__: ƒ __proto__()
```

&emsp;我们可以添加一些属性到 doSomething 的原型上面：

```javascript
function doSomething(){}
doSomething.prototype.foo = "bar";
console.log( doSomething.prototype );
```

```javascript
Object
foo: "bar"
constructor: ƒ doSomething()
[[Prototype]]: Object
```

&emsp;我们可以使用 new 运算符来在现在的这个原型基础之上，创建一个 doSomething 的实例。正确使用 new 运算符的方法就是在正常调用函数时，在函数名的前面加上一个 new 前缀. 通过这种方法，在调用函数前加一个 new，它就会返回一个这个函数的实例化对象。然后，就可以在这个对象上面添加一些属性：

```javascript
doSomething
prop: "some value"
[[Prototype]]: Object
foo: "bar"
constructor: ƒ doSomething()
[[Prototype]]: Object
```

&emsp;就像上面看到的，doSomeInstancing 的 `__proto__` 属性就是 doSomething.prototype。但是这又有什么用呢？好吧，当你访问 doSomeInstancing 的一个属性，浏览器首先查找 doSomeInstancing 是否有这个属性。如果 doSomeInstancing 没有这个属性，然后浏览器就会在 doSomeInstancing 的 `__proto__` 中查找这个属性（也就是 doSomething.prototype）。如果 doSomeInstancing 的 `__proto__` 有这个属性，那么 doSomeInstancing 的 `__proto__` 上的这个属性就会被使用。 否则，如果 doSomeInstancing 的 `__proto__` 没有这个属性，浏览器就会去查找 doSomeInstancing 的 `__proto__` 的 `__proto__`，看它是否有这个属性。默认情况下，所有函数的原型属性的 `__proto__` 就是 window.Object.prototype。所以 doSomeInstancing 的 `__proto__` 的 `__proto__`（也就是 doSomething.prototype 的 `__proto__`（也就是 Object.prototype））会被查找是否有这个属性。如果没有在它里面找到这个属性，然后就会在 doSomeInstancing 的 `__proto__` 的 `__proto__` 的 `__proto__` 里面查找。然而这有一个问题: doSomeInstancing 的 `__proto__` 的 `__proto__` 的 `__proto__` 不存在。最后，原型链上面的所有的 `__proto__` 都被找完了，浏览器所有已经声明了的 `__proto__` 上都不存在这个属性，然后就得出结论，这个属性是 undefined。

```javascript
function doSomething() {}
doSomething.prototype.foo = "bar";
var doSomeInstancing = new doSomething();
doSomeInstancing.prop = "some value";
console.log("doSomeInstancing.prop:      " + doSomeInstancing.prop);
console.log("doSomeInstancing.foo:       " + doSomeInstancing.foo);
console.log("doSomething.prop:           " + doSomething.prop);
console.log("doSomething.foo:            " + doSomething.foo);
console.log("doSomething.prototype.prop: " + doSomething.prototype.prop);
console.log("doSomething.prototype.foo:  " + doSomething.prototype.foo);
```

```javascript
doSomeInstancing.prop:      some value
doSomeInstancing.foo:       bar
doSomething.prop:           undefined
doSomething.foo:            undefined
doSomething.prototype.prop: undefined
doSomething.prototype.foo:  bar
```

&emsp;那么，调用 person1 的 "实际定义在 Object 上" 的方法时，会发生什么？比如：

```javascript
person1.valueOf()
```

&emsp;这个方法仅仅返回了被调用对象的值。在这个例子中发生了如下过程：

+ 浏览器首先检查，person1 对象是否具有可用的 valueOf() 方法。
+ 如果没有，则浏览器检查 person1 对象的原型对象（即 Person 构造函数的 prototype 属性所指向的对象）是否具有可用的 valueof() 方法。
+ 如果也没有，则浏览器检查 Person() 构造函数的 prototype 属性所指向的对象的原型对象（即 Object 构造函数的 prototype 属性所指向的对象）是否具有可用的 valueOf() 方法。这里有这个方法，于是该方法被调用。

> &emsp;note：注意：必须重申，原型链中的方法和属性没有被复制到其他对象 —— 它们被访问需要通过前面所说的 "原型链" 的方式。

> &emsp;note：没有官方的方法用于直接访问一个对象的原型对象 —— 原型链中的 "连接" 被定义在一个内部属性中，在 JavaScript 语言标准中用 `[[prototype]]` 表示。然而，大多数现代浏览器还是提供了一个名为 `__proto__` （前后各有 2 个下划线）的属性，其包含了对象的原型。你可以尝试输入 `person1.__proto__` 和 `person1.__proto__.__proto__`，看看代码中的原型链是什么样的。（`person1.__proto__.__proto__.__proto__` 为 null）

#### prototype 属性：继承成员被定义的地方

&emsp;那么，那些继承的属性和方法在哪儿定义呢？如果你查看 [Object](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Object) 参考页，会发现左侧列出许多属性和方法 —— 大大超过我们在 person1 对象中看到的继承成员的数量。某些属性或方法被继承了，而另一些没有——为什么呢？原因在于，继承的属性和方法是定义在 prototype 属性之上的（你可以称之为子命名空间 (sub namespace) ）—— 那些以 Object.prototype. 开头的属性，而非仅仅以 Object. 开头的属性。prototype 属性的值是一个对象，我们希望被原型链下游的对象继承的属性和方法，都被储存在其中。

&emsp;于是 Object.prototype.watch()、Object.prototype.valueOf() 等等成员，适用于任何继承自 Object() 的对象类型，包括使用构造器创建的新的对象实例。Object.is()、Object.keys()，以及其他不在 prototype 对象内的成员，不会被 "对象实例" 或 "继承自 Object() 的对象类型" 所继承。这些方法/属性仅能被 Object() 构造器自身使用。

> &emsp;note：这看起来很奇怪 —— 构造器本身就是函数，你怎么可能在构造器这个函数中定义一个方法呢？其实函数也是一个对象类型，你可以查阅 [Function()](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Function) 构造器的参考文档以确认这一点。

&emsp;JavaScript 中到处都是通过原型链继承的例子。比如，你可以尝试从 String、Date、Number 和 Array 全局对象的原型中寻找方法和属性。它们都在原型上定义了一些方法，因此当你创建一个字符串时：

```javascript
var myString = 'This is my string.';
```

&emsp;myString 立即具有了一些有用的方法，如 split()、indexOf()、replace() 等。

> &emsp;Import：prototype 属性大概是 JavaScript 中最容易混淆的名称之一。你可能会认为，this 关键字指向当前对象的原型对象，其实不是（还记得么？原型对象是一个内部对象，应当使用 `__proto__` 访问）。prototype 属性包含（指向）一个对象，你在这个对象中定义需要被继承的成员。

```javascript
var person2 = Object.create(person1);
```

&emsp;create() 实际做的是从指定原型对象创建一个新的对象。这里以 person1 为原型对象创建了 person2 对象。在控制台输入 `person2.__proto__` 结果返回对象 person1。 

#### constructor 属性

&emsp;每个实例对象都从原型中继承了一个 constructor 属性，该属性指向了用于构造此实例对象的构造函数。

&emsp;在控制台输入下面的指令：

```javascript
person1.constructor
person2.constructor
```

&emsp;都将返回 Person() 构造器，因为该构造器包含这些实例的原始定义。

```javascript
var person3 = new person1.constructor('Karen', 'Stephenson', 26, 'female', ['playing drums', 'mountain climbing']);
```

&emsp;constructor 属性返回 Object 的构造函数（用于创建实例对象）。注意，此属性的值是对函数本身的引用，而不是一个包含函数名称的字符串。此外，constructor 属性还有其他用途。比如，想要获得某个对象实例的构造器的名字，可以这么用：

```javascript
instanceName.constructor.name
```

#### 修改原型

> &emsp;下面一段类似 OC 中的动态给类添加方法，但是不能在类定义确定以后再给类动态的添加属性/成员变量。JavaScript 太灵活了，以前觉得脚本语言好学，现在发现完全错误了，脚本语言看似简单的表现只是使用上的简单，简单的背后隐藏的是比静态语言更加复杂的实现逻辑！

&emsp;这段代码将为构造器的 prototype 属性添加一个新的方法：

```javascript
Person.prototype.farewell = function() {
  alert(this.name.first + ' has left the building. Bye for now!');
}
```

&emsp;保存代码，在浏览器中加载页面，然后在控制台输入：

```javascript
person1.farewell();
```

&emsp;你会看到一条警告信息，其中还显示了构造器中定义的人名；这很有用。但更关键的是，整条继承链动态地更新了，任何由此构造器创建的对象实例都自动获得了这个方法。

&emsp;再想一想这个过程。我们的代码中定义了构造器，然后用这个构造器创建了一个对象实例，此后向构造器的 prototype 添加了一个新的方法：

```javascript
function Person(first, last, age, gender, interests) {

  // 属性与方法定义

};

var person1 = new Person('Tammi', 'Smith', 32, 'neutral', ['music', 'skiing', 'kickboxing']);

Person.prototype.farewell = function() {
  alert(this.name.first + ' has left the building. Bye for now!');
}
```

&emsp;但是 farewell() 方法仍然可用于 person1 对象实例 —— 旧有对象实例的可用功能被自动更新了。这证明了先前描述的原型链模型。这种继承模型下，上游对象的方法不会复制到下游的对象实例中；下游对象本身虽然没有定义这些方法，但浏览器会通过上溯原型链、从上游对象中找到它们。这种继承模型提供了一个强大而可扩展的功能系统。

&emsp;你很少看到属性定义在 prototype 属性中，因为如此定义不够灵活。比如，你可以添加一个属性：

```javascript
Person.prototype.fullName = 'Bob Smith';
```

&emsp;但这不够灵活，因为人们可能不叫这个名字。用 name.first 和 name.last 组成 fullName 会好很多：

```javascript
Person.prototype.fullName = this.name.first + ' ' + this.name.last;
```

&emsp;然而，这么做是无效的，因为本例中 this 引用全局范围，而非函数范围。访问这个属性只会得到 undefined undefined。但这个语句若放在 先前定义在 prototype 上的方法中则有效，因为此时语句位于函数范围内，从而能够成功地转换为对象实例范围。你可能会在 prototype 上定义常属性 (constant property) （指那些你永远无需改变的属性），但一般来说，在构造器内定义属性更好。

#### 构造器最佳实践

&emsp;事实上，一种极其常见的对象定义模式是，在构造器（函数体）中定义属性、在 prototype 属性上定义方法。如此，构造器只包含属性定义，而方法则分装在不同的代码块，代码更具可读性。例如：

```javascript
// 构造器及其属性定义

function Test(a,b,c,d) {
  // 属性定义
};

// 定义第一个方法
Test.prototype.x = function () { ... }

// 定义第二个方法
Test.prototype.y = function () { ... }

// 等等……
```

## Object

&emsp;Object 是 JavaScript 的一种数据类型。它用于存储各种键值集合和更复杂的实体。Objects 可以通过 Object() 构造函数或者使用 对象字面量 的方式创建。

&emsp;在 JavaScript 中，几乎所有的对象都是 Object 类型的实例，它们都会从 Object.prototype 继承属性和方法，虽然大部分属性都会被覆盖（shadowed）或者说被重写了（overridden）。 除此之外，Object 还可以被故意的创建，但是这个对象并不是一个 "真正的对象"（例如：通过 Object.create(null)），或者通过一些手段改变对象，使其不再是一个 "真正的对象"（比如说：Object.setPrototypeOf）。

&emsp;通过原型链，所有的 object 都能观察到 Object 原型对象（Object prototype object）的改变，除非这些受到改变影响的属性和方法沿着原型链被进一步的重写。尽管有潜在的危险，但这为覆盖或扩展对象的行为提供了一个非常强大的机制。

&emsp;Object 构造函数为给定的参数创建一个包装类对象（object wrapper），具体有以下情况：

+ 如果给定值是 null 或 undefined，将会创建并返回一个空对象
+ 如果传进去的是一个基本类型的值，则会构造其包装类型的对象
+ 如果传进去的是引用类型的值，仍然会返回这个值，经他们复制的变量保有和源对象相同的引用地址
+ 当以非构造函数形式被调用时，Object 的行为等同于 new Object()。

## Function

&emsp;每个 JavaScript 函数实际上都是一个 Function 对象。运行 `(function(){}).constructor === Function // true` 便可以得到这个结论。

&emsp;Function() 创建一个新的 Function 对象。直接调用此构造函数可以动态创建函数，但会遇到和 eval() 类似的的安全问题和（相对较小的）性能问题。然而，与 eval() 不同的是，Function 构造函数创建的函数只能在全局作用域中运行。

&emsp;Function.prototype.displayName 函数的显示名称。

&emsp;Function.prototype.length 函数期望的参数数量。

&emsp;Function.prototype.name 函数的名称。

&emsp;`Function.prototype.apply(thisArg [, argsArray])` 调用一个函数并将其 this 的值设置为提供的 thisArg。参数可用以通过数组对象传递。

&emsp;`Function.prototype.bind(thisArg[, arg1[, arg2[, ...argN]]])` 创建一个新的函数，该函数在调用时，会将 this 设置为提供的 thisArg。在调用新绑定的函数时，可选的参数序列（`[, arg1[, arg2[, ...argN]]]`）会被提前添加到参数序列中（即调用绑定创建的函数时，传递的参数会接续在可选参数序列后）。

&emsp;`Function.prototype.call(thisArg[, arg1, arg2, ...argN])` 调用一个函数，并将其 this 值设置为提供的值。也可以选择传输新参数。

&emsp;Function.prototype.toString() 返回表示函数源码的字符串。覆盖了 Object.prototype.toString 方法。

### Function 构造函数与函数声明之间的不同

&emsp;由 Function 构造函数创建的函数不会创建当前环境的闭包，它们总是被创建于全局环境，因此在运行时它们只能访问全局变量和自己的局部变量，不能访问它们被 Function 构造函数创建时所在的作用域的变量。这一点与使用 eval() 执行创建函数的代码不同。

```javascript
var x = 10;

function createFunction1() {
    var x = 20;
    return new Function('return x;'); // 这里的 x 指向最上面全局作用域内的 x
}

function createFunction2() {
    var x = 20;
    function f() {
        return x; // 这里的 x 指向上方本地作用域内的 x
    }
    return f;
}

var f1 = createFunction1();
console.log(f1());          // 10
var f2 = createFunction2();
console.log(f2());          // 20
```

&emsp;虽然这段代码可以在浏览器中正常运行，但在 Node.js 中 f1() 会产生一个 "找不到变量 x" 的 ReferenceError。这是因为在 Node 中顶级作用域不是全局作用域，而 x 其实是在当前模块的作用域之中。


## JavaScript 中的继承

&emsp;介绍如何创建 "子" 对象类别（构造器）并从 "父" 类别中继承功能。

### 原型式的继承

&emsp;到目前为止我们已经了解了一些关于原型链的实现方式以及成员变量是如何通过它来实现继承，但是之前涉及到的大部分都是浏览器内置函数（比如 String、Date、Number 和 Array），那么我们如何创建一个继承自另一对象的 JavaScript 对象呢？

&emsp;正如前面课程所提到的，有些人认为 JavaScript 并不是真正的面向对象语言，在经典的面向对象语言中，你可能倾向于定义类对象，然后你可以简单地定义哪些类继承哪些类（参考 [C++ inheritance](https://www.tutorialspoint.com/cplusplus/cpp_inheritance.htm) 里的一些简单的例子），JavaScript 使用了另一套实现方式，继承的对象函数并不是通过复制而来，而是通过原型链继承（通常被称为 原型式继承 —— prototypal inheritance）。

&emsp;示例代码能看到一个只定义了一些属性的 Person() 构造器，与之前通过模块来实现所有功能的 Person 的构造器类似。

```javascript
function Person(first, last, age, gender, interests) {
  this.name = {
    first,
    last
  };
  this.age = age;
  this.gender = gender;
  this.interests = interests;
};
```

&emsp;所有的方法都定义在构造器的原型上，比如：

```javascript
Person.prototype.greeting = function() {
  alert('Hi! I\'m ' + this.name.first + '.');
};
```

&emsp;比如我们想要创建一个 Teacher 类，就像我们前面在面向对象概念解释时用的那个一样。这个类会继承 Person 的所有成员，同时也包括：

+ 一个新的属性，subject —— 这个属性包含了教师教授的学科。
+ 一个被更新的 greeting() 方法，这个方法打招呼听起来比一般的 greeting() 方法更正式一点 —— 对于一个教授一些学生的老师来说。

&emsp;我们要做的第一件事是创建一个 Teacher() 构造器 —— 将下面的代码加入到现有代码之下：

```javascript
function Teacher(first, last, age, gender, interests, subject) {
  Person.call(this, first, last, age, gender, interests);

  this.subject = subject;
}
```

&emsp;这在很多方面看起来都和 Person 的构造器很像，但是这里有一些我们从没见过的奇怪玩意 —— call() 函数。基本上，这个函数允许你调用一个在这个文件里别处定义的函数。第一个参数指明了在你运行这个函数时想对 "this" 指定的值，也就是说，你可以重新指定你调用的函数里所有 "this" 指向的对象。其他的变量指明了所有目标函数运行时接受的参数。

> &emsp;note：在这个例子里我们在创建一个新的对象实例时同时指派了继承的所有属性，但是注意你需要在构造器里将它们作为参数来指派，即使实例不要求它们被作为参数指派（比如也许你在创建对象的时候已经得到了一个设置为任意值的属性）。

&emsp;所以在这个例子里，我们很有效的在 Teacher() 构造函数里运行了 Person() 构造函数，得到了和在 Teacher() 里定义的一样的属性，但是用的是传送给 Teacher()，而不是 Person() 的值（我们简单使用这里的 this 作为传给 call() 的 this，意味着 this 指向 Teacher() 函数）。

&emsp;在构造器里的最后一行代码简单地定义了一个新的 subject 属性，这将是教师会有的，而一般人没有的属性。

### 从无参构造函数继承

&emsp;请注意，如果你继承的构造函数不从传入的参数中获取其属性值，则不需要在 call() 中为其指定其他参数。所以，例如，如果你有一些相当简单的东西：

```javascript
function Brick() {
  this.width = 10;
  this.height = 20;
}
```

&emsp;可以这样继承 width 和 height 属性（以及下面描述的其他步骤）：

```javascript
function BlueGlassBrick() {
  Brick.call(this);

  this.opacity = 0.5;
  this.color = 'blue';
}
```

&emsp;请注意，我们仅传入了 this 到 call() 中 - 不需要其他参数，因为我们不会继承通过参数设置的父级的任何属性。

### 设置 Teacher() 的原型和构造器引用

&emsp;到目前为止，一切都很好，但我们遇到了一个问题。

&emsp;我们已经定义了一个新的构造器，并且它具有一个 prototype 属性，默认情况下，该属性仅包含一个引用构造函数本身的对象。它不包含 Person 构造器的 prototype 属性的属性。

&emsp;要查看此，请在 JavaScript 控制台中输入 Object.getOwnPropertyNames(Teacher.prototype) 然后再次输入它，将 Teacher 替换为 Person。新构造器也不会继承这些方法。

&emsp;要查看此，请比较 Person.prototype.greeting 和 Teacher.prototype.greeting 的输出。我们需要让 Teacher() 继承 Person() 的原型上定义的方法。那么我们该怎么做呢？

&emsp;上面一段的含义就是以当前的继承方式，Teacher 构造器仅继承了 Person 构造器中的定义的成员，并不能继承 Person.prototype 原型上定义的方法。那么我们该怎么做呢？   

&emsp;在之前添加的内容下面添加以下行：

```javascript
Teacher.prototype = Object.create(Person.prototype);
```

&emsp;在这里，我们的朋友 create() 再次来救援。在本例中，我们使用它来创建一个新对象，并使其成为 Teacher.prototype 的值。新对象将 Person.prototype 作为其原型，并将因此继承，当需要的情况下，Person.prototype 上的所有方法都是可提供的。

&emsp;在我们继续前进之前，我们需要再做一件事：添加最后一行 Teacher.prototype 的 constructor 属性现在等于 Person()，因为我们只需设置 Teacher.prototype 来引用从 Person.prototype 继承其属性的对象！尝试保存代码，在浏览器中加载页面，然后在控制台中输入 Teacher.prototype.constructor 进行验证。可以发现 Teacher.prototype.constructor 和 Person.prototype.constructor 的值都是 Person。

&emsp;这可能会成为一个问题，因此我们需要设置此权利。为此，你可以返回到源代码并在底部添加以下行：

```javascript
Object.defineProperty(Teacher.prototype, 'constructor', {
    value: Teacher,
    enumerable: false, // so that it does not appear in 'for in' loop
    writable: true });
```

&emsp;现在，如果你保存并刷新，输入 Teacher.prototype.constructor 应该会根据需要返回 Teacher()，此外，我们现在在继承 Person()！

### 向 Teacher() 添加一个新的greeting()函数

&emsp;为了完善代码，还需在构造函数 Teacher() 上定义一个新的函数 greeting()。最简单的方法是在 Teacher 的原型上定义它 — 把以下代码添加到你代码的底部：

```javascript
Teacher.prototype.greeting = function() {
  var prefix;

  if(this.gender === 'male' || this.gender === 'Male' || this.gender === 'm' || this.gender === 'M') {
    prefix = 'Mr.';
  } else if(this.gender === 'female' || this.gender === 'Female' || this.gender === 'f' || this.gender === 'F') {
    prefix = 'Mrs.';
  } else {
    prefix = 'Mx.';
  }

  alert('Hello. My name is ' + prefix + ' ' + this.name.last + ', and I teach ' + this.subject + '.');
};
```

&emsp;这样就会出现老师打招呼的弹窗，老师打招呼会使用条件结构判断性别从而使用正确的称呼。

## 对象成员总结

1. 那些定义在构造器函数中，用于给予对象实例的属性或方法。它们都很容易发现 —— 在你自己的代码中，它们是构造函数中使用 this.x = x 类型的行；在浏览器内已经构建好的代码中，它们是可用于对象实例的成员（这些对象实例通常使用 new 关键字调用构造函数来创建，例如 var myInstance = new myConstructor()）。
2. 那些直接在构造函数上定义，仅在构造函数上可用的属性或方法。它们通常仅在浏览器的内置对象中可用，并通过被直接链接到构造函数来识别，而不是实例。例如 Object.keys()。它们一般被称作静态属性或静态方法。
3. 那些在构造函数原型上定义，由所有实例和对象类继承的属性或方法。它们包括在构造函数的原型属性上定义的任何成员，如 myConstructor.prototype.x()。
4. 那些在对象实例上可用的对象，它可以是像我们上面看到的构造函数实例化时创建的对象（例如 let teacher1 = new Teacher( 'Chris' ); 以及其属性 teacher1.name），也可以是一个对象字面量（let teacher1 = { name : 'Chris' } 以及其属性 teacher1.name）。

## ECMAScript 2015 类

&emsp;ECMAScript 2015 引入了class syntax 到 JavaScript，作为使用更简单、更简洁的语法编写可重用类的一种方式，这更类似于 C++ 或 Java 中的类。在 JavaScript 中 class 声明创建一个基于原型继承的具有给定名称的新类。下面将 Person 和 Teacher 示例从原型继承转换为类（class），以展示 class 在 JavaScript 中是如何完成的。

> &emsp;note：所有现代浏览器都支持这种现代的编写类的方式，但是如果你处理的项目需要支持不支持此语法的浏览器（最明显的是 Internet Explorer），那么仍然值得了解底层原型继承。

&emsp;Person 示例的重写版本，类样式：

```javascript
class Person {
  constructor(first, last, age, gender, interests) {
    this.name = {
      first,
      last
    };
    this.age = age;
    this.gender = gender;
    this.interests = interests;
  }

  greeting() {
    console.log(`Hi! I'm ${this.name.first}`);
  };

  farewell() {
    console.log(`${this.name.first} has left the building. Bye for now!`);
  };
}
```

&emsp;类（class）语句表示我们正在创建一个新类。在此块中，我们定义了该类的所有功能：

+ constructor() 方法定义了一个构造函数来表示我们的 Person 类。
+ greeting() 和 farewell() 是类方法。任何你想要和这个类联系的方法都被定义在其中，在构造器之后。在这个例子里，我们使用了模板字符串（[template literals](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Template_literals)）而不是字符串拼接以使代码更易读。

&emsp;我们现在可以使用 new 运算符来实例化对象实例，就像我们之前使用过的方式一样：

```javascript
let han = new Person('Han', 'Solo', 25, 'male', ['Smuggling']);
han.greeting(); // Hi! I'm Han

let leia = new Person('Leia', 'Organa', 19, 'female', ['Government']);
leia.farewell(); // Leia has left the building. Bye for now
```

> &emsp;note：在 JS 引擎下，类会被转换为原型继承模型 —— 这只是语法糖。但我相信你会觉得这是一种更容易的写法。

### 类语法继承

&emsp;上面我们创建了一个类来代表一个人 Person。它们具有一系列所有人共有的属性；下面将创建我们的专用 Teacher 类，使其使用现代类语法从 Person 继承。这称为创建子类或子类化。

&emsp;要创建一个子类，我们使用 extends 关键字来告诉 JavaScript 我们想要的在我们的类的基础上的类，而且需要在 constructor 中调用 super()。

```javascript
class Teacher extends Person {
  constructor(subject, grade) {
    super(); // Now 'this' is initialized by calling the parent constructor.
    this.subject = subject;
    this.grade = grade;
  }
}
```

&emsp;由于 super() 运算符实际上是父类构造函数，将 Parent 类构造函数的必要参数传递给它也会初始化我们子类中的父类属性，从而继承它：

```javascript
class Teacher extends Person {
  constructor(first, last, age, gender, interests, subject, grade) {
    super(first, last, age, gender, interests);

    // subject and grade are specific to Teacher
    this.subject = subject;
    this.grade = grade;
  }
}
```

&emsp;现在，当我们实例化 Teacher 对象实例时，我们可以按照我们的预期调用定义在 Teacher 和 Person 上的方法和属性：

```javascript
let snape = new Teacher('Severus', 'Snape', 58, 'male', ['Potions'], 'Dark arts', 5);
snape.greeting(); // Hi! I'm Severus.
snape.farewell(); // Severus has left the building. Bye for now.
snape.age // 58
snape.subject; // Dark arts
```

&emsp;就像我们对 Teacher 所做的那样，我们可以创建 Person 的其他子类，以使它们更加专业，而无需修改基类。

### Getters and Setters

&emsp;有时，我们可能想要更改我们创建的类中的属性值，或者我们不知道属性的最终值是什么。以 Teacher 为例，在创建 subject 之前，我们可能不知道 Teacher 将教授什么 subject，或者他们的 subject 可能会在学期之间发生变化。

&emsp;我们可以用 getter 和 setter 来处理这种情况。

&emsp;让我们用 getter 和 setter 来增强 Teacher 类。该课程的开始时间与我们上次查看它时相同。

&emsp;Getters 和 setters 成对工作。getter 返回变量的当前值，其相应的 setter 将变量的值更改为它定义的值。

&emsp;修改后的 Teacher class 如下所示：

```javascript
class Teacher extends Person {
  constructor(first, last, age, gender, interests, subject, grade) {
    super(first, last, age, gender, interests);
    // subject and grade are specific to Teacher
    this._subject = subject;
    this.grade = grade;
  }

  get subject() {
    return this._subject;
  }

  set subject(newSubject) {
    this._subject = newSubject;
  }
}
```

&emsp;在上面的类中，我们有一个 subject 属性的 getter 和 setter。我们使用 `_` 创建一个单独的值来存储我们的 subject 属性。如果不使用此约定，则每次调用 get 或 set 时都会收到错误。此时：

+ 要显示 snape 对象的 `_subject` 属性的当前值，我们可以使用 snape.subject getter 方法。
+ 要将新值分配给 `_subject` 属性，我们可以使用 `snape.subject = "new value"` setter 方法。

&emsp;下面的示例显示了两个实际操作中的功能：

```javascript
// Check the default value
console.log(snape.subject) // Returns "Dark arts"

// Change the value
snape.subject = "Balloon animals" // Sets _subject to "Balloon animals"

// Check it again and see if it matches the new value
console.log(snape.subject) // Returns "Balloon animals"
```

> &emsp;note：Getter 和 setter 有时非常有用，例如，当你希望在每次请求或设置属性时运行一些代码时。但是，对于简单的情况，没有 getter 或 setter 的普通属性访问就可以了。类似 OC 中我们想要在属性的读写时做一些自定义操作时重写 setter 和 getter 函数。

### 何时在 JavaScript 中使用继承？

&emsp;确实，JavaScript 中的原型和继承相对于强类型语言中的类和类继承复杂太多了！

&emsp;特别是在读完这段文章内容之后，你也许会想 "天啊，这实在是太复杂了". 是的，你是对的，原型和继承代表了 JavaScript 这门语言里最复杂的一些方面，但是 JavaScript 的强大和灵活性正是来自于它的对象体系和继承方式，这很值得花时间去好好理解下它是如何工作的。

&emsp;在某种程度上来说，你一直都在使用继承 - 无论你是使用 WebAPI 的不同特性还是调用字符串、数组等浏览器内置对象的方法和属性的时候，你都在隐式地使用继承。

&emsp;就在自己代码中使用继承而言，你可能不会使用的非常频繁，特别是在小型项目中或者刚开始学习时，因为当你不需要对象和继承的时候，仅仅为了使用而使用它们只是在浪费时间而已。但是随着你的代码量的增大，你会越来越发现它的必要性。如果你开始创建一系列拥有相似特性的对象时，那么创建一个包含所有共有功能的通用对象，然后在更特殊的对象类型中继承这些特性，将会变得更加方便有用。

> &emsp;note：考虑到 JavaScript 的工作方式，由于原型链等特性的存在，在不同对象之间功能的共享通常被叫做委托，特殊的对象将功能委托给通用的对象类型完成。这也许比将其称之为继承更为贴切，因为 "被继承" 了的功能并没有被拷贝到正在 "进行继承" 的对象中，相反它仍存在于通用的对象中。

&emsp;在使用继承时，建议你不要使用过多层次的继承，并仔细追踪定义方法和属性的位置。很有可能你的代码会临时修改了浏览器内置对象的原型，但你不应该这么做，除非你有足够充分的理由。过多的继承会在调试代码时给你带来无尽的混乱和痛苦。

&emsp;总之，对象是另一种形式的代码重用，就像函数和循环一样，有他们特定的角色和优点。如果你发现自己创建了一堆相关的变量和函数，还想一起追踪它们并将其灵活打包的话，对象是个不错的主意。对象在你打算把一个数据集合从一个地方传递到另一个地方的时候非常有用。这些都可以在不使用构造器和继承的情况下完成。如果你只是需要一个单一的对象实例，也许使用对象常量会好些，你当然不需要使用继承。

&emsp;这个时候你应该理解了 JavaScript 中的对象和 OOP 基础，原型和原型继承机制，如何创建类（constructors）和对象实例，为类增加功能，通过从其他类继承而创建新的子类。

## 什么是 JSON？

&emsp;JavaScript 对象表示法（JSON）是用于将结构化数据表示为 JavaScript 对象的标准格式，通常用于在网站上表示和传输数据（例如从服务器向客户端发送一些数据，因此可以将其显示在网页上）。

&emsp;JSON 是一种按照 JavaScript 对象语法的数据格式。虽然它是基于 JavaScript 语法，但它独立于 JavaScript，这也是为什么许多程序环境能够读取（解读）和生成 JSON。JSON 可以作为一个对象或者字符串存在，前者用于解读 JSON 中的数据，后者用于通过网络传输 JSON 数据。这不是一个大事件 —— JavaScript 提供一个全局的可访问的 JSON 对象来对这两种数据进行转换。

&emsp;一个 JSON 对象可以被储存在它自己的文件中，这基本上就是一个文本文件，扩展名为 .json， 还有 MIME type 用于 application/json.

## 参考链接
**参考链接:🔗**
+ [JavaScript 参考](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference)
+ [JavaScript 指南](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Guide)
+ [JavaScript Related Topics](https://developer.mozilla.org/zh-CN/docs/Learn/JavaScript)
+ [JavaScript 主题学习区](https://developer.mozilla.org/zh-CN/docs/learn/JavaScript)
+ [重新介绍 JavaScript（JS 教程）](https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/A_re-introduction_to_JavaScript)
