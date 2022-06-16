# HTML 学习记录（二）：HTML 入门

&emsp;为了创建一个网站，你需要了解 HTML —— 一项用于定义网页结构的基本技术。HTML 用于标识你的网页内容是应该被解析为段落、列表、头部、链接、图像、多媒体播放器、表单或是其他众多可用的元素之一，亦或是你定义的新元素。

## World Wide Web

&emsp;万维网（World Wide Web）一般称其为 WWW，W3，或 Web 是通过 Internet 访问公共网页的互连系统。网络与互联网不一样：Web 是建立在互联网之上的许多应用程序之一。

&emsp;Tim Berners-Lee 提出了被称为万维网的架构。他在 1990 年在 CERN 物理研究实验室的计算机上创建了第一个网页 server、web browser 和 网页。1991 年，他在 alt.hypertext 新闻组上宣布创作，标志着 Web 首次公开的那一刻。

&emsp;我们今天认为的 "Web" 系统由几个组件组成：

+ HTTP 协议管理服务器和客户端之间的数据传输。
+ 要访问 Web 组件，客户端提供唯一的通用标识符，称为 URL（统一资源位置）或 URI（统一资源标识符）（正式称为通用文档标识符（UDI））。
+ HTML（超文本标记语言）是发布 Web 文档的最常用格式。

&emsp;通过超链接连接资源是 Web 的定义概念，帮助其身份作为连接文档的集合。

&emsp;Tim Berners-Lee 在发明 Web 之后不久就创建了 W3C（万维网联盟），进一步规范和开发 Web。该联盟由核心 Web 兴趣小组组成，如网页浏览器开发人员，政府机构，研究人员和大学。其使命包括教育和外联。

> &emsp;note：统一资源定位器（URL）是指定在 Internet 上可以找到资源的位置的文本字符串。在 HTTP 的上下文中，URL 被叫做 "网络地址" 或 "链接"。你的浏览器在其地址栏显示 URL，例如：`https://developer.mozilla.org`。URL 也可用于文件传输（FTP） ，电子邮件（SMTP）和其他应用。

> &emsp;note：统一资源标识符（URI）是一个指向资源的字符串。最通常用在 URL 上来指定 Web 上资源文件的具体位置。相比之下，URN 是在给定的命名空间用名字指向具体的资源，如：书本的 ISBN。

> &emsp;Hyperlink（超链接）将网页或其他数据连接起来。在 HTML 中，`<a>` 标签定义了网页中的某处（例如一段文本或一张图片）到其他网页（也可能是本网页）的另一处的超链接。

> &emsp;W3C 全球资讯网协会（World Wide Web Consortium，W3C）是维护网络相关规则与架构的国际机构。它由 420 多个开发网路标准、经营宣传计划、还有维护网络相关的公开讨论会...等成员组织构成。W3C 的业界协调公司确保它们实行相同的 W3C 标准。每个标准都会有四个成熟过程：工作草案（Working Draft、WD）、候选推荐标准（Candidate Recommendation、CR）、提案推荐标准（Proposed Recommendation、PR）、以及 W3C 推荐标准（W3C Recommendation、REC）。

## HTML

&emsp;HTML（HyperText Markup Language，超文本标记语言）是一种描述语言，用来定义网页结构。

&emsp;1990 年，由于对 Web 未来发展的远见，Tim Berners-Lee 提出了超文本的概念，并在第二年在 SGML 的基础上将其正式定义为一个标记语言。IETF 于 1993 年正式开始制定 HTML 规范，并在经历了几个版本的草案后于 1995 年发布了 HTML 2.0 版本。1994 年，Berners-Lee 为了 Web 发展而成立了 W3C。1996 年，W3C 接管了 HTML 的标准化工作，并在 1 年后发布了 HTML 3.2 推荐标准。1999 年，HTML 4.0 发布，并在 2000 年成为 ISO 标准。

&emsp;自那以后，W3C 几乎放弃了 HTML 而转向 XHTML，并于 2004 年成立了另一个独立的小组 WHATWG。幸运的是，WHATWG 后来又转回来参与了 HTML 5 标准的制定，两个组织（即 W3C 和 WHATWG）在 2008 年发布了第一份草案，并在 2014 年发布了 HTML 5 标准的最终版。

> &emsp;note：Hypertext 超文本：包含了指向其他文本的链接，而不是像小说中的单一线性流。这个术语是由 Ted Nelson 在 1965 年左右提出的。

> &emsp;note：SGML 标准通用标记式语言（Standard Generalized Markup Language 或简称 SGML）是一套 ISO 所定义，用来规范宣告式标记语言的标准。在 Web 环境当中，HTML 4、XHTML 以及 XML 都是知名的以 SGML 为基础的语言。然而值得一提的是，HTML 自第五版起拥有了自己的剖析规则，不再是以 SGML 为基础的语言。

> &emsp;ISO（International Organization for Standardization）国际标准化组织是一个全球性协会，负责制定统一标准，协调各主要行业的公司。

> &emsp;IETF：Internet 工程任务组 (IETF) 是一个全球性组织，负责起草管理 Internet 背后机制的规范，特别是 TCP/IP 或 Internet 协议套件。IETF 是开放的，由志愿者管理，并由互联网协会赞助。

&emsp;自那以后，W3C 几乎放弃了 HTML 而转向 XHTML，并于 2004 年成立了另一个独立的小组 WHATWG。幸运的是，WHATWG 后来又转回来参与了 HTML5 标准的制定，两个组织（译注：即 W3C 和 WHATWG）在 2008 年发布了第一份草案，并在 2014 年发布了 HTML5 标准的最终版。

> &emsp;W3C 标准 XHTML XHTML(eXtensible HyperText Markup Language，可扩展超文本标记语言) 2000 年底，国际 W3C(World Wide Web Consortium) 组织公布发行了 XHTML 1.0 版本。XHTML 1.0 是一种在 HTML 4.0 基础上优化和改进的的新语言，目的是基于 XML 应用。XHTML 是一种增强了的 HTML，它的可扩展性和灵活性将适应未来网络应用更多的需求。
  
  XHTML 是在 2000 年 1 月 26 日被国际标准组织机构 W3C(World Wide web Consortium) 定为一个标准的，认为是 HTML 的一个最新版本，并且将逐渐替换 HTML。现在所有的浏览器都支持 XHTML，XHTML 兼容 HTML 4.0。也有人认为 XHTML 就是 HTML 4.01。
  
  如果你在学习过程中自己编写了一个符合标准的站，你可以通过 W3C 的验证，验证通过后你将会得到一个标志，通常是 XHTML 1.0 认证和 CSS 验证。可以去 www.w3.org 这个站点去验证站点，如果符合那两个规则则会分别给我们两段代码加到你的网页上向别人展示说明你采用了标准建站。

> &emsp;WHATWG（Web Hypertext Application Technology Working GroupWeb 超文本应用程序技术工作组）是一个负责维护与开发 Web 标准的社区，他们的工作成果包括 DOM、Fetch API 和 HTML。一些来自 Apple、Mozilla 和 Opera 的员工在 2004 年建立了 WHATWG。

> &emsp;HTML 5 HTML 的最新稳定版本，HTML 5 将 HTML 从用于构造一个文档的一个简单标记到一个完整的应用程序开发平台。除其他功能外，HTML5 还包括新元素和用于增强存储、多媒体和硬件访问的 JavaScript APIs。 

&emsp;HTML 文档是包含多个 HTML 元素 的文本文档。每个元素都用一对开始和结束标签包裹。每个标签以尖括号（`<>`）开始和结束。也有一部分标签中不需要包含文本，这些标签称为空标签，如：`<img>`。

&emsp;可以使用属性（Attribute）来扩展 HTML 标签。属性用来提供一些附加信息，这些信息可能会影响浏览器对元素的解析：

![anatomy-of-an-html-element.png](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/44ef4471742e4e84a7111bb09336fb4c~tplv-k3u1fbpfcp-watermark.image?)

&emsp;HTML 文件通常会以 `.htm` 或 `.html` 为扩展名。用户可以从 Web 服务器中下载，并使用任一 Web 浏览器来解析和显示。

&emsp;就其核心而言，HTML 是一种相当简单的、由不同元素组成的标记语言，它可以被应用于文本片段，使文本在文档中具有不同的含义（它是段落吗？它是项目列表吗？它是表格吗？），将文档结构化为逻辑块（文档是否有头部？有三列内容？有一个导航菜单？），并且可以将图片、影像等内容嵌入到页面中。本模块将介绍前两个用途，并且介绍一些 HTML 的基本概念和语法。

## HTML 入门

&emsp;涵盖了 HTML 绝对基础知识以帮助你入门 —— 定义元素、属性和其他重要术语，并展示了它们在语言中的位置。我们还展示了一个典型的 HTML 页面的结构和一个 HTML 元素的结构，并解释了一些重要的基本语言特征。

&emsp;将从 HTML 最基础的部分讲起，对元素（Element）、属性（Attribute）以及可能涉及的一些重要术语进行介绍，并明确它们在语言中所处的位置。本文还会讲解 HTML 元素和页面的组织方式，以及其他一些重要的基本语言特性。

### 什么是 HTML?

&emsp;HTML（HyperText Markup Language）不是一门编程语言，而是一种用来告知浏览器如何组织页面的标记语言。HTML 可复杂、可简单，一切取决于开发者。它由一系列的元素（elements）组成，这些元素可以用来包围不同部分的内容，使其以某种方式呈现或者工作。一对标签（tags）可以为一段文字或者一张图片添加超链接，将文字设置为斜体、改变字号等等。例如下面一行内容：

```javascript
我的猫咪脾气爆:)
```

&emsp;可以将这行文字封装成一个段落（Paragraph）`<p>` 元素来使其在单独一行呈现：

```javascript
<p>我的猫咪脾气爆:)</p>
```

### 主动学习：创建第一个 HTML 元素

&emsp;通过使用标签 `<em>` 和 `</em>`（在前面放置 `<em>` 打开元素，在后面放置 `</em>` 关闭元素）—— 这使得行内容变成斜体强调！

#### 嵌套元素

&emsp;也可以把元素放到其它元素之中 —— 这被称作嵌套。如果我们想要表明我们的小猫脾气很暴躁，可以将 "爆" 嵌套在 `<strong>` 中，意味着这个单词被着重强调：

```javascript
<p>我的猫咪脾气<strong>爆</strong></p>
```

#### 块级元素和内联元素

&emsp;在 HTML 中有两种你需要知道的重要元素类别，块级元素和内联元素。

+ 块级元素在页面中以块的形式展现 —— 相对于其前面的内容它会出现在新的一行，其后的内容也会被挤到下一行展现。块级元素通常用于展示页面上结构化的内容，例如段落、列表、导航菜单、页脚等等。一个以 block 形式展现的块级元素不会被嵌套进内联元素中，但可以嵌套在其它块级元素中。
+ 内联元素通常出现在块级元素中并环绕文档内容的一小部分，而不是一整个段落或者一组内容。内联元素不会导致文本换行：它通常出现在一堆文字之间例如超链接元素 `<a>` 或者强调元素 `<em>` 和 `<strong>`。

```javascript
<p>第四</p><p>第五</p><p>第六</p>

<em>第一</em><em>第二</em><em>第三</em>
```

&emsp;`<p>` 是一个块级元素，所以第一行代码中的每个元素分别都另起了新的一行展现，并且每个段落间都有一些间隔（这是因为默认的浏览器有着默认的展示 `<p>` 元素的 CSS styling），而 `<em>` 是一个内联元素，所以就像在下方可以看到的，第二行代码中的三个元素都没有间隙的展示在了同一行。。

> &emsp;note：HTML 5 重新定义了元素的类别：[元素内容分类](https://html.spec.whatwg.org/multipage/indices.html#element-content-categories)。尽管这些新的定义更精确，但却比上述的 "块级元素" 和 "内联元素" 更难理解，在之后的讨论中仍使用旧的定义。

> &emsp;note：文内提到的 "块" 和 "内联"，不应该与 [the types of CSS boxes](https://developer.mozilla.org/zh-CN/docs/Learn/CSS/Building_blocks/The_box_model) 中的同名术语相混淆。尽管它们默认是相关的，但改变 CSS 显示类型并不会改变元素的分类，也不会影响它可以包含和被包含于哪些元素。防止这种混淆也是 HTML 5 摒弃这些术语的原因之一。

> &emsp;note：可以查阅包含了块级元素和内联元素列表的参考页面 -— see [Block-level elements](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Block-level_elements) and [Inline elements](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Inline_elements).

#### 空元素

&emsp;不是所有元素都拥有开始标签、内容、结束标签。一些元素只有一个标签，通常用来在此元素所在位置插入/嵌入一些东西。例如：元素 `<img>` 是用来在元素 `<img>` 所在位置插入一张指定的图片。例子如下：

```javascript
<img src="https://roy-tian.github.io/learning-area/extras/getting-started-web/beginner-html-site/images/firefox-icon.png">
```

> &emsp;note：空元素（Empty elements）有时也被叫作 void elements。

#### 属性

&emsp;元素也可以拥有属性，如下开始标签中的 class 属性：

```javascript
<p class="editor-note">我的猫咪脾气爆:)</p>
```

&emsp;属性包含元素的额外信息，这些信息不会出现在实际的内容中。在上述例子中，这个 class 属性给元素赋了一个识别的名字（id），这个名字此后可以被用来识别此元素的样式信息和其他信息。

&emsp;一个属性必须包含如下内容：

1. 一个空格，在属性和元素名称之间。(如果已经有一个或多个属性，就与前一个属性之间有一个空格。)
2. 属性名称，后面跟着一个等于号。
3. 一个属性值，由一对引号 "" 引起来。

&emsp;为一个元素添加属性：

&emsp;另一个例子是关于元素 `<a>` 的 —— 元素 `<a>` 是锚，它使被标签包裹的内容成为一个超链接。此元素也可以添加大量的属性，其中几个如下：

+ href: 这个属性声明超链接的 web 地址，当这个链接被点击浏览器会跳转至 href 声明的 web 地址。例如：`href="https://www.mozilla.org/"`。
+ title: 标题 title 属性为超链接声明额外的信息，比如你将链接至的那个页面。例如：`title="The Mozilla homepage"`。当鼠标悬停在超链接上面时，这部分信息将以工具提示的形式显示。
+ target: 目标 target 属性用于指定链接如何呈现出来。例如：`target="_blank"` 将在新标签页中显示链接。如果你希望在当前标签页显示链接，忽略这个属性即可。

&emsp;编辑下面的文本框中的内容，使之变成指向任一个你喜欢的 web 地址的链接。首先，添加 `<a>` 元素，然后为它添加 `href` 属性和 `title` 属性。你可以即时的在输出区域看到你修改的内容。你应该可以看到一个链接，当鼠标移上此链接时会显示 `title` 属性值，当点击此链接时会跳转到 `href` 指定的 web 地址。记住：在元素名和属性名之间以及两个属性之间要有一个空格。

&emsp;布尔属性，有时你会看到没有值的属性，它是合法的。这些属性被称为布尔属性，他们只能有跟它的属性名一样的属性值。例如 disabled 属性，他们可以标记表单输入使之变为不可用 (变灰色)，此时用户不能向他们输入任何数据。












## 参考链接
**参考链接:🔗**
+ [HTML 开发者指南](https://developer.mozilla.org/zh-CN/docs/Learn/HTML)
+ [HTML 参考](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Reference)
