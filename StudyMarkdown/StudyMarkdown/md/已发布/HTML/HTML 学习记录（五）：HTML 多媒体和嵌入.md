# HTML 学习记录（五）：HTML 多媒体和嵌入

&emsp;这个模块带你探索如何使用 HTML 在你的网页里如何包含多媒体信息，例如用各种方法包含图片，以及嵌入视频、音频，甚至是嵌入其他完整的网页。

&emsp;在这份教程中，到目前为止我们已经看到了许多的文字了。真的很多文字。但是网页除了文本之外什么都没有，真的非常无聊，所以，让我们开始看看怎样让网页动起来。用更多有趣的内容！本模块要探索怎样用 HTML 来让你的网页包含多媒体，包括可以包含图像的不同方式，以及怎样嵌入视频，甚至是整个其他的网页。

&emsp;还有其他类型的多媒体要考虑，但是从低调的 `<img>` 元素开始是符合逻辑的，它常常被用来在网页中嵌入一个简单的图片。在这篇文章中，我们要看看怎样更深入的使用它，包括基础，用标题注解 `<figure>`，以及怎样把它关联到 CSS 背景图片。

&emsp;在一开始时，Web 仅有文本，那真的是很无趣。幸运的是，没过多久网页上就能嵌入图片和其他有趣的内容了。虽然还有许多其他类型的多媒体，但是从地位比较低的 `<img>` 元素开始是符合逻辑的，它常常被用来在网页中嵌入一张简单的图片。在这篇文章中，我们将看到怎样深入的使用它，包括基本的用 `<figure>` 来添加说明文字，以及怎样把它和 CSS 背景图片链接起来。

&emsp;我们可以用 `<img>` 元素来把图片放到网页上。它是一个空元素（它不需要包含文本内容或闭合标签），最少只需要一个 src （一般读作其全称 source）来使其生效。src 属性包含了指向我们想要引入的图片的路径，可以是相对路径或绝对 URL，就像 `<a>` 元素的 href 属性一样。

&emsp;举个例子来看，如果你有一幅文件名为 dinosaur.jpg 的图片，且它与你的 HTML 页面存放在相同路径下，那么你可以这样嵌入它：

```javascript
<img src="dinosaur.jpg">
```

&emsp;如果这张图片存储在和 HTML 页面同路径的 images 文件夹下（这也是 Google 推荐的做法，利于 SEO/索引），那么你可以采用如下形式：

```javascript
<img src="images/dinosaur.jpg">
```

&emsp;以此类推。

> &emsp;note：搜索引擎也读取图像的文件名并把它们计入 SEO。因此你应该给你的图片取一个描述性的文件名：dinosaur.jpg 比 img835.png 要好。

&emsp;你也可以像下面这样使用绝对路径：

```javascript
<img src="https://www.example.com/images/dinosaur.jpg">
```

&emsp;但是这种方式是不被推荐的，这样做只会使浏览器做更多的工作，例如重新通过 DNS 再去寻找 IP 地址。通常我们都会把图片和 HTML 放在同一个服务器上。

&emsp;像 `<img>` 和 `<video>` 这样的元素有时被称之为替换元素，因为这样的元素的内容和尺寸由外部资源（像是一个图片或视频文件）所定义，而不是元素自身。

```javascript
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Images in HTML</title>
  </head>
  <body>
    <h1>Images in HTML</h1>

    <img src="dinosaur.jpg"
         alt="The head and torso of a dinosaur skeleton; it has a large head with long sharp teeth"
         width="400"
         height="341"
         title="A T-Rex on display in the Manchester University Museum">
  </body>
</html>
```

## 备选文本

&emsp;下一个我们讨论的属性是 alt，它的值应该是对图片的文字描述，用于在图片无法显示或不能被看到的情况。例如，上面的例子可以做如下改进：

```javascript
<img src="images/dinosaur.jpg"
     alt="The head and torso of a dinosaur skeleton;
          it has a large head with long sharp teeth">
```

&emsp;测试 alt 属性最简单的方式就是故意拼错图片文件名，这样浏览器就无法找到该图片从而显示备选的文本。如果我们将上例的图片文件名改为 dinosooooor.jpg，浏览器就不能显示图片，而显示 alt 属性中的文本内容。

&emsp;那么，为什么我们需要备选文本呢？它可以派上用场的原因有很多：

+ 用户有视力障碍，通过屏幕阅读器来浏览网页。事实上，给图片一个备选的描述文本对大多数用户都是很有用的。
+ 就像上面所说的，你也许会把图片的路径或文件名拼错。
+ 浏览器不支持该图片类型。某些用户仍在使用纯文本的浏览器，例如 Lynx，这些浏览器会把图片替换为描述文本。
+ 你会想提供一些文字描述来给搜索引擎使用，例如搜索引擎可能会将图片的文字描述和查询条件进行匹配。
+ 用户关闭的图片显示以减少数据的传输，这在手机上是十分普遍的，并且在一些国家带宽有限且昂贵。

&emsp;你到底应该在 alt 里写点什么呢？这首先取决于为什么这张图片会在这儿，换句话说，如果这张图片没显示出来，会少了什么：

+ 装饰：如果图片只是用于装饰，而不是内容的一部分，可以写一个空的 alt=""。例如，屏幕阅读器不会浪费时间对用户读出不是核心需要的内容。实际上装饰性图片就不应该放在 HTML 文件里，CSS 背景图片才应该用于插入装饰图片，但如果这种情况无法避免，alt="" 会是最佳处理方案。
+ 内容：如果你的图片提供了重要的信息，就要在 alt 文本中简要的提供相同的信息，甚至更近一步，把这些信息写在主要的文本内容里，这样所有人都能看见。不要写冗余的备选文本（如果在主要文本中将所有的段落都重复两遍，对于没有失明的用户来说多烦啊！），如果在主要文本中已经对图片进行了充分的描述，写 alt="" 就好。
+ 链接：如果你把图片嵌套在 `<a>` 标签里，来把图片变成链接，那你还必须提供无障碍的链接文本。在这种情况下，你可以写在同一个 `<a>` 元素里，或者写在图片的 `alt` 属性里，随你喜欢。
+ 文本：你不应该将文本放到图像里。例如，如果你的主标题需要有阴影，你可以用 CSS 来达到这个目的，而不是把文本放到图片里。如果真的必须这么做，那就把文本也放到 alt 里。

&emsp;本质上，关键在于在图片无法被看见时也提供一个可用的体验，这确保了所有人都不会错失一部分内容。尝试在浏览器中使图片不可见然后看看网页变成什么样了，你会很快意识到在图片无法显示时备选文本能帮上多大忙。

## 宽度和高度

&emsp;你可以用宽度和高度属性来指定你的图片的高度和宽度（你可以用多种方式找到你的图片的宽度和高度，例如在 Mac 上，你可以用 Cmd + I 来得到显示的图片文件的信息）回到我们的例子，你可以这样做：

```javascript
<img src="images/dinosaur.jpg"
     alt="一只恐龙头部和躯干的骨架，它有一个巨大的头，长着锋利的牙齿。"
     width="400"
     height="341">
```

&emsp;在正常的情况下，这不会对显示产生很大的影响，但是如果图片没有显示（例如用户刚刚开始浏览网页，但是图片还没有加载完成），你会注意到浏览器会为要显示的图片留下一定的空间：

&emsp;这是一件好事情 —— 这使得页面加载的更快速更流畅。

&emsp;然而，你不应该使用 HTML 属性来改变图片的大小。如果你把尺寸设定的太大，最终图片看起来会模糊；如果太小，会在下载远远大于你需要的图片时浪费带宽。如果你没有保持正确的宽高比，图片可能看起来会扭曲。在把图片放到你的网站页面之前，你应该使用图形编辑器使图片的尺寸正确。

&emsp;如果你需要改变图片的尺寸，你应该使用 CSS 而不是 HTML。

## Image titles 图片标题

&emsp;类似于超链接，你可以给图片增加 title 属性来提供需要更进一步的支持信息。在我们的例子中，可以这样做：

```javascript
<img src="images/dinosaur.jpg"
     alt="一只恐龙头部和躯干的骨架，它有一个巨大的头，长着锋利的牙齿。"
     width="400"
     height="341"
     title="A T-Rex on display in the Manchester University Museum">
```

&emsp;这会给我们一个鼠标悬停提示，看起来就像链接标题：

&emsp;图片标题并不必须要包含有意义的信息，通常来说，将这样的支持信息放到主要文本中而不是附着于图片会更好。不过，在有些环境中这样做更有用，比如当没有空间显示提示时，也就是在图片栏中。

&emsp;然而，这并不是推荐的 —— title 有很多易访问性问题，主要是基于这样一个事实，即屏幕阅读器的支持是不可预测的，大多数浏览器都不会显示它，除非你在鼠标悬停时（例如：title 无法访问键盘用户）。

## 通过为图片搭配说明文字的方式来解说图片

&emsp;说到说明文字，这里有很多种方法让你添加一段说明文字来搭配图片。比如，没有人会阻止你这么做：

```javascript
<div class="figure">
  <img src="/images/dinosaur_small.jpg"
     alt="一只恐龙头部和躯干的骨架，它有一个巨大的头，长着锋利的牙齿。"
     width="400"
     height="341">
  <p>曼彻斯特大学博物馆展出的一只霸王龙的化石</p>
</div>
```

&emsp;这是可以的， `<p>` 中包含了你需要的内容，以及方便使用 CSS 的一种很好的风格。但是这里有一个问题，从语义的角度上来讲，`<img>` 和 `<p>` 并没有什么联系，这会给使用屏幕阅读的人造成问题，比如当你有 50 张图片和其搭配的 50 段说明文字，那么一段说明文字是和哪张图片有关联的呢？

&emsp;有一个更好的做法是使用 HTML5 的 `<figure>` 和 `<figcaption>` 元素，它正是为此而被创造出来的：为图片提供一个语义容器，在标题和图片之间建立清晰的关联。我们之前的例子可以重写为：

```javascript
<figure>
  <img src="https://raw.githubusercontent.com/mdn/learning-area/master/html/multimedia-and-embedding/images-in-html/dinosaur_small.jpg"
     alt="一只恐龙头部和躯干的骨架，它有一个巨大的头，长着锋利的牙齿。"
     width="400"
     height="341">
  <figcaption>曼彻斯特大学博物馆展出的一只霸王龙的化石</figcaption>
</figure>
```

&emsp;这个 `<figcaption>` 元素告诉浏览器和其他辅助的技术工具这段说明文字描述了 `<figure>` 元素的内容。

&emsp;从无障碍的角度来说，说明文字和 alt 文本扮演着不同的角色。看得见图片的人们同样可以受益于说明文字，而 alt 文字只有在图片无法显示时才这样。所以，说明文字和 alt 的内容不应该一样，因为当图片无法显示时，它们会同时出现。尝试让你的图片不显示，看看效果如何。

&emsp;注意 `<figure>` 里不一定要是一张图片，只要是一个这样的独立内容单元：

+ 用简洁、易懂的方式表达意图。
+ 可以置于页面线性流的某处。
+ 为主要内容提供重要的补充说明。

&emsp;`<figure>` 可以是几张图片、一段代码、音视频、方程、表格或别的。

## CSS 背景图片

&emsp;你也可以使用 CSS 把图片嵌入网站中（JavaScript 也行，不过那是另外一个故事了），这个 CSS 属性 `background-image` 和另其他 `background-*` 属性是用来放置背景图片的。比如，为页面中的所有段落设置一个背景图片，你可以这样做：

```javascript
p {
  background-image: url("images/dinosaur.jpg");
}
```

&emsp;按理说，这种做法相对于 HTML 中插入图片的做法，可以更好地控制图片和设置图片的位置，那么为什么我们还要使用 HTML 图片呢？如上所述，CSS 背景图片只是为了装饰 — 如果你只是想要在你的页面上添加一些漂亮的东西，来提升视觉效果，那 CSS 的做法是可以的。但是这样插入的图片完全没有语义上的意义，它们不能有任何备选文本，也不能被屏幕阅读器识别。这就是 HTML 图片有用的地方了。

&emsp;总而言之，如果图像对你的内容里有意义，则应使用 HTML 图像。如果图像纯粹是装饰，则应使用 CSS 背景图片。

## 视频和音频内容

&emsp;接下来，我们将看看怎样在我们的页面上用 HTML5 的 `<video>` 和 `<audio>` 元素来嵌入视频和音频；包括基础，提供向不同的浏览器提供不同文件格式的访问方式，增加标题和副标题，以及增加对过时的浏览器的兼容。

&emsp;现在我们可以轻松的为一个网页添加简单的图像，下一步我们开始为 HTML 文档添加音频和视频播放器。在这篇文章中，我们会使用 `<video>` 和 `<audio>` 元素来做到这件事；然后我们还会看看如何为你的视频添加字幕。

## web 中的音频和视频

&emsp;web 开发者们一直以来想在 Web 中使用音频和视频，自 21 世纪初以来，我们的带宽开始能够支持任意类型的视频（视频文件比文本和图片要大的多）。在早些时候，传统的 web 技术（如 HTML）不能够在 Web 中嵌入音频和视频，所以一些像 Flash (后来有 Silverlight) 的专利技术在处理这些内容上变得很受欢迎。这些技术能够正常的工作，但是却有着一系列的问题，包括无法很好的支持 HTML/CSS 特性、安全问题，以及可行性问题。

&emsp;传统的解决方案能够解决许多这样的问题，前提是它能够正确的工作。幸运的是，几年之后 HTML 5 标准提出，其中有许多的新特性，包括 `<video>` 和 `<audio>` 标签，以及一些 JavaScript 和 APIs 用于对其进行控制。在这里，我们不讨论有关 JavaScript 的问题，仅仅讲解有关 HTML 的基础。

## `<video>` 元素

&emsp;`<video>` 允许你轻松地嵌入一段视频。一个简单的例子如下：

```javascript
<video src="rabbit320.webm" controls>
  <p>你的浏览器不支持 HTML 5 视频。可点击<a href="rabbit320.mp4">此链接</a>观看</p>
</video>
```

&emsp;当中的一些属性如下：

+ `src`：同 `<img>` 标签使用方式相同，src 属性指向你想要嵌入网页当中的视频资源，他们的使用方式完全相同。
+ `controls`：用户必须能够控制视频和音频的回放功能。你可以使用 controls 来包含浏览器提供的控件界面，同时你也可以使用合适的 JavaScript API 创建自己的界面。界面中至少要包含开始、停止以及调整音量的功能。

&emsp;`<video>` 标签内的内容：这个叫做后备内容 — 当浏览器不支持 `<video>` 标签的时候，就会显示这段内容，这使得我们能够对旧的浏览器提供回退内容。你可以添加任何后备内容，在这个例子中我们提供了一个指向这个视频文件的链接，从而使用户至少可以访问到这个文件，而不会局限于浏览器的支持。

```javascript
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Simple video example</title>
  </head>
  <body>
    <h1>Below is a simple video example</h1>

    <video src="rabbit320.webm" controls>
      <p>Your browser doesn't support HTML5 video. Here is a <a href="rabbit320.webm">link to the video</a> instead.</p> 
    </video>
  </body>
</html>
```

## 使用多个播放源以提高兼容性

&emsp;以上的例子中有一个问题，你可能已经注意到了，如果你尝试使用像 Safari 或者 Internet Explorer 这些浏览器来访问上面的链接。视频并不会播放，这是因为不同的浏览器对视频格式的支持不同。幸运的是，你有办法防止这个问题发生。

### 媒体文件的内容

&emsp;我们先来快速的了解一下术语。像 MP3、MP4、WebM 这些术语叫做容器格式。他们定义了构成媒体文件的音频轨道和视频轨道的储存结构，其中还包含描述这个媒体文件的元数据，以及用于编码的编码译码器等等。

&emsp;一个格式为 WebM 的电影包含视频轨道、音频轨道和文本轨道，其中视频轨道包含一个主视频轨道和一个可选的 Angle 轨道；音频轨道包含英语和西班牙语的音频轨道，还有一个英语评论的音频轨道；文字轨道包含英语和西班牙语的字幕轨道，如下图所示：

![截屏2022-06-28 22.38.03.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ad903f576f734171810367585280bfc1~tplv-k3u1fbpfcp-watermark.image?)

&emsp;为了编解码器（codec）编码媒体，容器中的音频和视频轨道以适合的格式保存。音频轨道和视频轨道使用不同的格式。每个音频轨道都使用音频编解码器进行编码，而视频轨道则使用视频编解码器进行编码。如前所述，不同的浏览器支持不同的视频和音频格式，以及不同的容器格式（如 MP3、MP4 和 WebM，这些格式又可以包含不同类型的视频和音频）。

&emsp;例如：

&emsp;WebM 容器通常包括了 Opus 或 Vorbis 音频和 VP8/VP9 视频。这在所有的现代浏览器中都支持，除了他们的老版本。

&emsp;MP4 容器通常包括 AAC 以及 MP3 音频和 H.264 视频。这在所有的现代浏览器中都支持，还有 Internet Explorer。

&emsp;老式的 Ogg 容器往往支持 Ogg Vorbis 音频和 Ogg Theora 视频。主要在 Firefox 和 Chrome 当中支持，不过这个容器已经被更强大的 WebM 容器所取代。

&emsp;有一些特殊情况。例如，对于某些类型的音频，通常编解码器的数据存储没有容器或简化容器。其中一个例子就是 FLAC 编解码器，它通常存储在 FLAC 文件中，FLAC 文件只是 FLAC 的原始轨迹。

&emsp;另一种情况是一直流行的 MP3 文件。"MP3 文件" 实际上是存储在 MPEG 或 MPEG-2 容器中的 MPEG-1 音频层 III（MPEG-1 Audio Layer III，MP3）音频轨道。这一点特别有趣，因为尽管大多数浏览器不支持在 `<video>` 和 `<audio>` 元素中使用 MPEG 媒体，但由于 MP3 的流行，它们可能仍然支持 MP3。

&emsp;音频播放器将会直接播放音频文件，例如 MP3 和 Ogg 文件。这些不需要容器。

### 浏览器所支持的媒体文件

> &emsp;note：你也许会疑惑为什么会有这样的情况存在。MP3 (音频格式) 和 MP4/H.264 (视频格式) 是被广泛支持的两种格式，并且质量良好。然而，他们却有专利的阻碍 — MP3 的专利会持续到 2017 年，而 H.264 会持续到 2027 年早期。意思也就是说浏览器若想要支持这些格式，就得支付高额的费用。此外，许多人反对软件技术垄断，支持开放的格式。这就是为什么我们需要准备不同的格式来兼容不同的浏览器。

&emsp;刚刚所说的格式主要用于将音频和视频压缩成可管理的文件（原始的音频和视频文件非常大）。浏览器包含了不同的 Codecs（编解码器（从 "coder-decoder" 派生的混合词）是对数据流进行编码或解码的程序、算法或设备。给定的编解码器知道如何处理特定的编码或压缩技术），如 Vorbis 和 H.264，它们用来将已压缩的音频和视频转化成二进制数字。不同的编码器和不同的容器都有各自的优缺点，在你更了解它们后，你可以自己选择使用哪个编码器和容器。

&emsp;浏览器并不全支持相同的 Codecs，所以你得使用几个不同格式的文件来兼容不同的浏览器。如果你使用的格式都得不到浏览器的支持，那么媒体文件将不会播放。

&emsp;要使你的媒体文件在不同平台，不同设备的浏览器上都可观看，这需要多种编码器组合使用，但是这是一种非常麻烦的事。

&emsp;需要记住的另一件事：同一款浏览器，移动版与桌面版支持的格式可能会有不同。最重要的是，它们都可以减轻媒体播放的处理负担（对于所有媒体或仅针对其内部无法处理的特定类型）。这意味着设备的媒体支持还部分取决于用户安装了什么软件。

> &emsp;note：这并没有那么简单，你可以从这里看到 [音视频编码兼容表](https://developer.mozilla.org/zh-CN/docs/Web/Media/Formats)。此外，许多移动平台的浏览器能够播放一些不支持的格式，但是它们用的却是底层系统的媒体播放器。但这也仅是现在支持。

&emsp;我们该怎么做呢？请看如下例子：

```javascript
<video controls>
  <source src="rabbit320.mp4" type="video/mp4">
  <source src="rabbit320.webm" type="video/webm">
  <p>你的浏览器不支持 HTML5 视频。可点击<a href="rabbit320.mp4">此链接</a>观看</p>
</video>
```

&emsp;现在我们将 src 属性从 `<video>` 标签中移除，转而将它放在几个单独的标签 `<source>` 当中。在这个例子当中，浏览器将会检查 `<source>` 标签，并且播放第一个与其自身 codec 相匹配的媒体。你的视频应当包括 WebM 和 MP4 两种格式，这两种在目前已经足够支持大多数平台和浏览器。

&emsp;每个 `<source>` 标签页含有一个 type 属性，这个属性是可选的，但是建议你添加上这个属性 — 它包含了视频文件的 MIME types ，同时浏览器也会通过检查这个属性来迅速的跳过那些不支持的格式。如果你没有添加 type 属性，浏览器会尝试加载每一个文件，直到找到一个能正确播放的格式，这样会消耗掉大量的时间和资源。

&emsp;MIME type（现在称为 "媒体类型 (media type)"，但有时也是 "内容类型 (content type)"）是指示文件类型的字符串，与文件一起发送（例如，一个声音文件可能被标记为 audio/ogg ，一个图像文件可能是 image/png）。它与传统 Windows 上的文件扩展名有相同目的。

### 其他 `<video>` 特性

&emsp;这里有许多你可以用在 HTML5 `<video>` 上的特性，请看第三个例子：

```c++
<video controls width="400" height="400"
       autoplay loop muted
       poster="poster.png">
  <source src="rabbit320.mp4" type="video/mp4">
  <source src="rabbit320.webm" type="video/webm">
  <p>你的浏览器不支持 HTML5 视频。可点击<a href="rabbit320.mp4">此链接</a>观看</p>
</video>
```

+ width 和 height：可以用属性控制视频的尺寸，也可以用 CSS 来控制视频尺寸。 无论使用哪种方式，视频都会保持它原始的长宽比 — 也叫做纵横比。如果你设置的尺寸没有保持视频原始长宽比，那么视频边框将会拉伸，而未被视频内容填充的部分，将会显示默认的背景颜色。
+ autoplay：这个属性会使音频和视频内容立即播放，即使页面的其他部分还没有加载完全。建议不要应用这个属性在你的网站上，因为用户们会比较反感自动播放的媒体文件。
+ loop：这个属性可以让音频或者视频文件循环播放。同样不建议使用，除非有必要。
+ muted：这个属性会导致媒体播放时，默认关闭声音。
+ poster：这个属性指向了一个图像的 URL，这个图像会在视频播放前显示。通常用于粗略的预览或者广告。
+ preload：这个属性被用来缓冲较大的文件，有 3 个值可选：
  + "none" ：不缓冲
  + "auto" ：页面加载后缓存媒体文件
  + "metadata" ：仅缓冲文件的元数据

&emsp;查看下面一个例子：

```javascript
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Extra video features example</title>
    <style>
      video {
        background: black;
      }
    </style>
  </head>
  <body>
    <h1>Extra video features example</h1>

    <video controls width="400" height="400"
           loop muted preload="auto"
           poster="poster.png">
      <source src="rabbit320.mp4" type="video/mp4">
      <source src="rabbit320.webm" type="video/webm">
      <p>Your browser doesn't support HTML5 video. Here is a <a href="rabbit320.mp4">link to the video</a> instead.</p>
    </video>
  </body>
</html>
```

&emsp;注意示例中并没有使用 autoplay 属性在这个版本的例子中 — 如果当页面一加载就开始播放视频的话，就不会看到 poster 属性的效果了。

### `<audio>` 标签

&emsp;`<audio>` 标签与 `<video>` 标签的使用方式几乎完全相同，有一些细微的差别比如下面的边框不同，一个典型的例子如下：

```javascript
<audio controls>
  <source src="viper.mp3" type="audio/mp3">
  <source src="viper.ogg" type="audio/ogg">
  <p>你的浏览器不支持 HTML5 音频，可点击<a href="viper.mp3">此链接</a>收听。</p>
</audio>
```

&emsp;音频播放器所占用的空间比视频播放器要小，由于它没有视觉部件 — 你只需要显示出能控制音频播放的控件。一些与 HTML 5 `<video>` 的差异如下：

+ `<audio>` 标签不支持 width/height 属性 — 由于其并没有视觉部件，也就没有可以设置 width/height 的内容。
+ 同时也不支持 poster 属性 — 同样，没有视觉部件。

&emsp;除此之外，`<audio>` 标签支持所有 `<video>` 标签拥有的特性 — 你可以回顾上面的章节来了解更多的有关信息。

### 重新播放媒体

&emsp;任何时候，你都可以在 Javascript 中调用 load() 方法来重置媒体。如果有多个由 `<source>` 标签指定的媒体来源，浏览器会从选择媒体来源开始重新加载媒体。

```javascript
const mediaElem = document.getElementById("my-media-element");
mediaElem.load();
```

### 音轨增删事件

&emsp;你可以监控媒体元素中的音频轨道，当音轨被添加或删除时，你可以通过监听相关事件来侦测到。具体来说，通过监听 AudioTrackList 对象的 addtrack 事件（即 HTMLMediaElement.audioTracks 对象），你可以及时对音轨的增加做出响应。

```javascript
const mediaElem = document.querySelector("video");
mediaElem.audioTracks.onaddtrack = function(event) {
  audioTrackAdded(event.track);
}
```

## 显示音轨文本

&emsp;现在，我们将讨论一个略微先进的概念，这个概念将会十分的有用。许多人不想（或者不能）听到 Web 上的音频/视频内容，至少在某些情况下是这样的，比如：

+ 许多人患有听觉障碍（通常来说是很难听清声音的人，或者聋人），所以他们不能听见音频中的声音。
+ 另外的情况可能是由于人们并不能听音频，可能是因为他们在一个非常嘈杂的环境当中（比如在一个拥挤的酒吧内恰好赶上了球赛），也可能是由于他们并不想打扰到其他人（比如在一个十分安静的地方，例如图书馆）。
+ 有一些人他们不说音频当中的语言，所以他们听不懂，因此他们想要一个副本或者是翻译来帮助他们理解媒体内容。

&emsp;给那些听不懂音频语言的人们提供一个音频内容的副本岂不是一件很棒的事情吗？所以，感谢 HTML5 `<video>` 使之成为可能，有了 WebVTT 格式，你可以使用 `<track>` 标签。

&emsp;WebVTT 是一个格式，用来编写文本文件，这个文本文件包含了众多的字符串，这些字符串会带有一些元数据，它们可以用来描述这个字符串将会在视频中显示的时间，甚至可以用来描述这些字符串的样式以及定位信息。这些字符串叫做 cues ，你可以根据不同的需求来显示不同的样式，最常见的如下：

+ subtitles：通过添加翻译字幕，来帮助那些听不懂外国语言的人们理解音频当中的内容。
+ captions：同步翻译对白，或是描述一些有重要信息的声音，来帮助那些不能听音频的人们理解音频中的内容。
+ timed descriptions：将文字转换为音频，用于服务那些有视觉障碍的人。

&emsp;一个典型的 WebVTT 文件如下：

```javascript
WEBVTT

1
00:00:22.230 --> 00:00:24.606
第一段字幕

2
00:00:30.739 --> 00:00:34.074
第二段

  ...
```

&emsp;让其与 HTML 媒体一起显示，你需要做如下工作：

1. 以 .vtt 后缀名保存文件。
2. 用 `<track>` 标签链接 .vtt 文件，`<track>` 标签需放在 `<audio>` 或 `<video>` 标签当中，同时需要放在所有 `<source>` 标签之后。使用 kind 属性来指明是哪一种类型，如 subtitles、captions、descriptions。然后，使用 srclang 来告诉浏览器你是用什么语言来编写的 subtitles。

&emsp;如下：

```javascript
<video controls>
    <source src="example.mp4" type="video/mp4">
    <source src="example.webm" type="video/webm">
    <track kind="subtitles" src="subtitles_en.vtt" srclang="en">
</video>
```

> &emsp;note：文本轨道会使你的网站更容易被搜索引擎抓取到（SEO），由于搜索引擎的文本抓取能力非常强大，使用文本轨道甚至可以让搜索引擎通过视频的内容直接链接。

## 从 `<object>` 到 `<iframe>` — 其他嵌入技术

&emsp;在这一节，我们将来了解一些另辟蹊径的内容，看一组元素，它们可以让你在页面中嵌入许多不同类型的内容：`<iframe>`、`<embed>` 和 `<object>` 元素。`<iframe>` 用来嵌入其他网页，而另外两者可以帮助你嵌入 PDF、SVG 甚至是 Flash — 一种逐渐退出历史舞台的技术，不过也许你还是能时不时的看到它。

## 嵌入的简史

&emsp;很久以前，很流行在网络上使用 frames 创建网站 — 网站的一小部分存储于单独的 HTML 页面中。这些被嵌入在一个称为 frameset 的主文档中，它允许你指定每个 frame 能够填充在屏幕上的区域，非常像调整表格的列和行的大小。这些做法在 90 年代中期至 90 年代后期被认为是比较酷的，有证据表明，将网页分解成较小的块，这样有利于下载速度 — 尤其是在那时网络连接速度太慢的情况下更为明显。然而，这些技术有很多问题，随着网络速度越来越快，这些技术带来的问题远超过它们带来的积极因素，所以你再也看不到它们被使用了。

&emsp;一小段时间之后（20 世纪 90 年代末，21 世纪初），插件技术变得非常受欢迎，例如 Java Applet 和 Flash — 这些技术允许网络开发者将丰富的内容嵌入到网页中，例如视频和动画等，这些内容不能通过 HTML 单独实现。嵌入这些技术是通过诸如元素表示外部资源，可以将其视为图像，嵌套浏览上下文或要由插件处理的资源。`<object>` 和较少使用 `</cke:embed>` 元素表示外部应用程序或交互式内容（换句话说，插件）的集成点。`<embed>` 的元素来实现的，当时它们非常有用。由于许多问题，包括可访问性、安全性、文件大小等，它们已经过时了; 如今，大多数移动设备不再支持这些插件，桌面端也逐渐不再支持。

&emsp;最后，元素表示嵌套的浏览上下文，有效地将另一个 HTML 页面嵌入到当前页面中。 在 HTML 4.01 中，文档可能包含一个头部和一个主体或头部和 frameset，但不包括主体和 frameset。但是，一个 `<iframe>` 可以在普通文档正文中使用。 每个浏览上下文都有自己的会话历史和活动文档。包含嵌入内容的浏览上下文称为父浏览上下文。顶级浏览上下文（没有父级）通常是浏览器窗口。`<iframe>` 元素出现了（连同其他嵌入内容的方式，如元素与 canvas 脚本 API 来绘制图形和动画。`<canvas>` 元素将视频内容嵌入到文档中 `<video>` 等），它提供了一种将整个 web 页嵌入到另一个网页的方法，看起来就像那个 web 页是另一个网页的一个 `<img>` 或其他元素一样。`<iframe>` 现在经常被使用。

## 自主学习：嵌入类型的使用

1. 首先，去 Youtube 找一个喜欢的视频。
2. 在视频下方，你会看到一个共享按钮 - 点击查看共享选项。
3. 选择 "嵌入" 选项卡，你将得到一些 `<iframe>` 代码 - 复制一下。

```javascript
<iframe
  width="560"
  height="315"
  src="https://www.youtube.com/embed/01iz9kmvbxg"
  title="YouTube video player"
  frameborder="0"
  allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture"
  allowfullscreen
></iframe>;
```

## Iframe 详解

&emsp;是不是很简单又有趣呢？`<iframe>: The Inline Frame element` 元素旨在允许你将其他 Web 文档嵌入到当前文档中。这很适合将第三方内容嵌入你的网站，你可能无法直接控制，也不希望实现自己的版本 - 例如来自在线视频提供商的视频，Disqus 等评论系统、在线地图提供商、广告横幅等。你通过本课程使用的实时可编辑示例就是使用 `<iframe>` 实现的。

&emsp;我们会在后面提到，关于 `<iframe>` 有一些严重的安全隐患需要考虑，但这并不意味着你不应该在你的网站上使用它们 — 它只需要一些知识和仔细地思考。让我们更详细地探索这些代码。假设你想在其中一个网页上加入 MDN 词汇表，你可以尝试以下方式：

```javascript
<iframe
  src="https://developer.mozilla.org/en-US/docs/Glossary"
  width="100%"
  height="500"
  frameborder="0"
  allowfullscreen
  sandbox
>
  <p>
    {" "}
    <a href="https://developer.mozilla.org/en-US/docs/Glossary">
      Fallback link for browsers that don't support iframes
    </a>{" "}
  </p>
</iframe>;
```

&emsp;此示例包括使用以下所需的 `<iframe>` 基本要素：

+ allowfullscreen：如果设置，`<iframe>` 则可以通过全屏 API 设置为全屏模式（稍微超出本文的范围）。
+ frameborder：如果设置为 1，则会告诉浏览器在此 `<iframe>` 和其他 `<iframe>` 之间绘制边框，这是默认行为。0 删除边框。不推荐这样设置，因为在 CSS 中可以更好地实现相同的效果。border: none;
+ src：此属性与 `<video>/<img>` 一样，包含指向要嵌入文档的 URL 的路径。
+ width 和 height：这些属性指定你想要的 iframe 的宽度和高度。

&emsp;备选内容：与 `<video>` 等其他类似元素相同，你可以在 `<iframe></iframe>` 标签之间包含备选内容，如果浏览器不支持 `<iframe>`，将会显示备选内容，这种情况下，我们已经添加了一个到该页面的链接。现在你几乎不可能遇到任何不支持 `<iframe>` 的浏览器。

+ sandbox：该属性需要在已经支持其他 `<iframe>` 功能（例如 IE 10 及更高版本）但稍微更现代的浏览器上才能工作，该属性可以提高安全性设置。

> &emsp;为了提高速度，在主内容完成加载后，使用 JavaScript 设置 iframe 的 src 属性是个好主意。这使你的页面可以更快地被使用，并减少你的官方页面加载时间（重要的 SEO 指标）。

## 安全隐患

&emsp;以上我们提到了安全问题 - 现在我们来详细介绍一下这一点。我们并不期望你第一次就能完全理解所有内容; 我们只想让你意识到这一问题，在你更有经验并开始考虑在你的实验和工作中使用 `<iframe>` 时为你提供参考。此外，没有必要害怕和不使用 `<iframe>` — 你只需要谨慎一点。继续看下去吧...

&emsp;浏览器制造商和 Web 开发人员了解到网络上的坏人（通常被称为黑客，或更准确地说是破解者），如果他们试图恶意修改你的网页或欺骗人们进行不想做的事情时常把 iframe 作为共同的攻击目标（官方术语：攻击向量），例如显示用户名和密码等敏感信息。因此，规范工程师和浏览器开发人员已经开发了各种安全机制，使 `<iframe>` 更加安全，这有些最佳方案值得我们考虑 - 我们将在下面介绍其中的一些。

> &emsp;单击劫持是一种常见的 iframe 攻击，黑客将隐藏的 iframe 嵌入到你的文档中（或将你的文档嵌入到他们自己的恶意网站），并使用它来捕获用户的交互。这是误导用户或窃取敏感数据的常见方式。

&emsp;一个快速的例子 — 尝试在浏览器中加载下面的示例 HTML 内容。你将不会看到任何内容，但如果你点击浏览器开发者工具中的控制台，你会看到一条消息，告诉你为什么没有显示内容。在 Chrome 中，会被告知："X-Frame-Options 拒绝加载 `https://developer.mozilla.org/en-US/docs/Glossary`"。这是因为构建 MDN 的开发人员已经在网站页面的服务器上设置了一个不允许被嵌入到 `<iframe>` 的设置（请参阅配置 CSP 指令）这是有必要的 — 整个 MDN 页面被嵌入在其他页面中没有多大意义，除非你想要将其嵌入到你的网站上并将其声称为自己的内容，或尝试通过单击劫持来窃取数据，这都是非常糟糕的事情。此外，如果每个人都这样做，所有额外的带宽将花费 Mozilla 很多资金。

```javascript
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8" />
    <title>Iframe with basic details</title>
  </head>
  <body>
    <h1>Iframe with basic details</h1>

    <iframe
      src="https://developer.mozilla.org/en-US/docs/Glossary"
      width="100%"
      height="500"
      frameborder="0"
      allowfullscreen
      sandbox
    >
      <p>
        <a href="https://developer.mozilla.org/en-US/docs/Glossary">
          Fallback link for browsers that don't support iframes
        </a>
      </p>
    </iframe>
  </body>
</html>
```

### 只有在必要时嵌入

&emsp;有时嵌入第三方内容（例如 YouTube 视频和地图）是有意义的，但如果你只在完全需要时嵌入第三方内容，你可以省去很多麻烦。网络安全的一个很好的经验法则是 "你怎么谨慎都不为过，如果你决定要做这件事，多检查一遍；如果是别人做的，在被证明是安全的之前，都假设这是危险的。"。

&emsp;除了安全问题，你还应该意识到知识产权问题。无论在线内容还是离线内容，绝大部分内容都是有版权的，甚至是一些你没想到有版权的内容（例如，Wikimedia Commons 上的大多数图片）。不要在网页上展示一些不属于你的内容，除非你是所有者或所有者给了你明确的、书面的许可。对于侵犯版权的惩罚是严厉的。再说一次，你再小心也不为过。

&emsp;如果内容获得许可，你必须遵守许可条款。例如，MDN 上的内容是在 CC-BY-SA 下许可的，这意味着，如果你要引用我们的内容，就必须用适当的方式注明来源，即使你对内容做了实质性的修改。

### 使用 HTTPS

&emsp;HTTPS 是 HTTP 的加密版本。你应该尽可能使用 HTTPS 为你的网站提供服务：

1. HTTPS 减少了远程内容在传输过程中被篡改的机会。
2. HTTPS 防止嵌入式内容访问你的父文档中的内容，反之亦然。

&emsp;使用 HTTPS 需要一个安全证书，这可能是昂贵的（尽管 Let's Encrypt 让这件事变得更容易），如果你没有，可以使用 HTTP 来为你的父文档提供服务。但是，由于 HTTPS 的第二个好处，无论成本如何，你绝对不能使用 HTTP 嵌入第三方内容（在最好的情况下，你的用户的 Web 浏览器会给他们一个可怕的警告）。所有有声望的公司，例如 Google Maps 或 Youtube，当你嵌入内容时，`<iframe>` 将通过 HTTPS 提供 - 查看 `<iframe>` src 属性内的 URL。

### 始终使用 sandbox 属性

&emsp;想尽可能减少攻击者在你的网站上做坏事的机会，那么你应该给嵌入的内容仅能完成自己工作的权限。当然，这也适用于你自己的内容。一个允许包含在其里的代码以适当的方式执行或者用于测试，但不能对其他代码库（意外或恶意）造成任何损害的容器称为沙盒。

&emsp;未沙盒化（Unsandboxed）内容可以做得太多（执行 JavaScript，提交表单，弹出窗口等）默认情况下，你应该使用没有参数的 sandbox 属性来强制执行所有可用的限制，如我们前面的示例所示。

&emsp;如果绝对需要，你可以逐个添加权限（sandbox="" 属性值内）—— 请参阅 sandbox 所有可用选项的参考条目。其中重要的一点是，你永远不应该同时添加 allow-scripts 和 allow-same-origin 到你的 sandbox 属性中 —— 在这种情况下，嵌入式内容可以绕过阻止站点执行脚本的同源安全策略，并使用 JavaScript 完全关闭沙盒。

> &emsp;note：如果攻击者可以欺骗人们直接访问恶意内容（在 iframe 之外），则沙盒无法提供保护。如果某些内容可能是恶意的（例如，用户生成的内容），请保证其是从不同的域向你的主站点提供的。

### 配置 CSP 指令

&emsp;CSP 代表内容安全策略，它提供一组 HTTP 标头（由 web 服务器发送时与元数据一起发送的元数据），旨在提高 HTML 文档的安全性。在 `<iframe>` 的安全性方面，你可以将服务器配置为发送适当的 X-Frame-Options  标题。这样做可以防止其他网站在其网页中嵌入你的内容（这将导致点击和一系列其他攻击），正如我们之前看到的那样，MDN 开发人员已经做了这些工作。

## `<embed>` 和 `<object>` 元素

&emsp;`<embed>` 和 `<object>` 元素的功能不同于 `<iframe>` —— 这些元素是用来嵌入多种类型的外部内容的通用嵌入工具，其中包括像 Java 小程序和 Flash，PDF（可在浏览器中显示为一个 PDF 插件）这样的插件技术，甚至像视频，SVG 和图像的内容！

&emsp;然而，你不太可能使用这些元素 —— Applet 几年来一直没有被使用；由于许多原因，Flash 不再受欢迎（见下面的插件案例）；PDF 更倾向于被链接而不是被嵌入；其他内容，如图像和视频都有更优秀、更容易元素来处理。插件和这些嵌入方法真的是一种传统技术，我们提及它们主要是为了以防你在某些情况下遇到问题，比如内部网或企业项目等。

&emsp;下面是一个使用该 `<embed>` 元素嵌入 Flash 影片的示例：

```javascript
<embed
  src="whoosh.swf"
  quality="medium"
  bgcolor="#ffffff"
  width="550"
  height="400"
  name="whoosh"
  align="middle"
  allowScriptAccess="sameDomain"
  allowFullScreen="false"
  type="application/x-shockwave-flash"
  pluginspage="http://www.macromedia.com/go/getflashplayer"
></embed>;
```

&emsp;很可怕，不是吗。Adobe Flash 工具生成的 HTML 往往更糟糕，使用嵌入 `<object>` 元素的 `<embed>` 元素来覆盖所有的基础。甚至有一段时间，Flash 被成功地用作 HTML5 视频的备用内容，但是这种情况越来越被认为是不必要的。

&emsp;现在来看一个 `<object>` 将 PDF 嵌入一个页面的例子：

```javascript
<object
  data="mypdf.pdf"
  type="application/pdf"
  width="800"
  height="1200"
  typemustmatch
>
  <p>
    You don't have a PDF plugin, but you can{" "}
    <a href="myfile.pdf">download the PDF file.</a>
  </p>
</object>;
```

&emsp;PDF 是纸与数据之间重要的阶梯，但它们在可访问性上有些问题，并且可能难以在小屏幕上阅读。它们在一些圈子中仍然受欢迎，我们最好是用链接指向它们，而不是将其嵌入到网页中，以便它们可以在单独的页面上被下载或被阅读。

### 针对插件的情况

&emsp;以前，插件在网络上是不可或缺的。还记得你必须安装 Adobe Flash Player 才能在线观看电影的日子吗？并且你还会不断地收到关于更新 Flash Player 和 Java 运行环境的烦人警报。Web 技术已经变得更加强大，那些日子已经结束了。对于大多数应用程序，现在是停止依赖插件传播内容，开始利用 Web 技术的时候了。

+ 扩大你对大家的影响力。每个人都有一个浏览器，但插件越来越少，特别是在移动用户中。由于 Web 在很大程度上不需要依赖插件而运行，所以人们宁愿只是去竞争对手的网站而不是安装插件。
+ 从 Flash 和其他插件附带的额外的可访问性问题中摆脱。
+ 避免额外的安全隐患。即使经过无数次补丁，Adobe Flash 也是非常不安全的。2015 年，Facebook 的首席安全官 Alex Stamos 甚至要求 Adobe 停止 Flash。

&emsp;那你该怎么办？如果你需要交互性，HTML 和 JavaScript 可以轻松地为你完成工作，而不需要 Java 小程序或过时的 ActiveX/BHO 技术。你可以使用 HTML5 视频来满足媒体需求，矢量图形 SVG，以及复杂图像和动画画布。彼得·埃尔斯特（Peter Elst）几年前已经提到，对于工作 Adobe Flash 极少是正确的工具，除了专门的游戏和商业应用。对于 ActiveX，即使微软的 Edge 浏览器也不再支持。

&emsp;在 Web 文档中嵌入其他内容这一主题可以很快变得非常复杂，因此在本文中，我们尝试以一种简单而熟悉的方式来介绍它，这种介绍方式将立即显示出相关性，同时仍暗示了一些涉及更高级功能的技术。刚开始，除了嵌入第三方内容（如地图和视频），你不太可能在网页上使用到嵌入技术。当你变得更有经验时，你可能会开始为他们找到更多的用途。

&emsp;除了我们在这里讨论的那些外，还有许多涉及嵌入外部内容的技术。我们看到了一些在前面的文章中出现的，如 `<video>`，`<audio>` 和 `<img>`，但还有其它的有待关注，如 `<canvas>` 用于 JavaScript 生成的 2D 和 3D 图形，`<svg>` 用于嵌入矢量图形。我们将在此学习模块的下一篇文章中学习 SVG。

## 在页面中添加矢量图像

&emsp;矢量图像在一些特定场景中非常有用。不同于常见的格式，比如 PNG/JPG，它们不会在放大的时候变得扭曲或者显示出像素格 —— 它们可以在缩放时保持光滑。本文将为你介绍什么是矢量图像，以及如何在网页中添加流行的 SVG 格式图像。

&emsp;矢量图形在很多情况下非常有用 — 它们拥有较小的文件尺寸，却高度可缩放，所以它们不会在镜头拉近或者放大图像时像素化。在这篇文章中，我们将为你呈现如何在网页中添加矢量图形。

## 什么是矢量图形？

&emsp;在网上，你会和两种类型的图片打交道 — 位图和矢量图：

+ 位图使用像素网格来定义 — 一个位图文件精确得包含了每个像素的位置和它的色彩信息。流行的位图格式包括 Bitmap (.bmp), PNG (.png), JPEG (.jpg), and GIF (.gif.)
+ 矢量图使用算法来定义 — 一个矢量图文件包含了图形和路径的定义，电脑可以根据这些定义计算出当它们在屏幕上渲染时应该呈现的样子。SVG 格式可以让我们创造用于 Web 的精彩的矢量图形。

&emsp;为了让你清楚的认识到两者的区别，我们来一个例子。查看这个例子：[vector-versus-raster.html](https://mdn.github.io/learning-area/html/multimedia-and-embedding/adding-vector-graphics-to-the-web/vector-versus-raster.html) — 它并排展示了两个看起来一致的图像，一个红色的五角星以及黑色的阴影。不同的是，左边的是 PNG，而右边的是 SVG 图像。

&emsp;当你放大网页的时候，区别就会变得明显起来 — 随着你的放大，PNG 图片变得像素化了，因为它存储是每个像素的颜色和位置信息 — 当它被放大时，每个像素就被放大以填满屏幕上更多的像素，所以图像就会开始变得马赛克感觉。矢量图像看起来仍然效果很好且清晰，因为无论它的尺寸如何，都使用算法来计算出图像的形状，仅仅是根据放大的倍数来调整算法中的值。

&emsp;此外，矢量图形相较于同样的位图，通常拥有更小的体积，因为它们仅需储存少量的算法，而不是逐个储存每个像素的信息。

## SVG 是什么？

&emsp;SVG 是用于描述矢量图像的 XML 语言。它基本上是像 HTML 一样的标记，只是你有许多不同的元素来定义要显示在图像中的形状，以及要应用于这些形状的效果。SVG 用于标记图形，而不是内容。非常简单，你有一些元素来创建简单图形，如 `<circle>` 和 `<rect>`。更高级的 SVG 功能包括 `<feColorMatrix>`（使用变换矩阵转换颜色）`<animate>` （矢量图形的动画部分）和 `<mask>`（在图像顶部应用模板）。

> &emsp;note：可扩展标记语言（XML）是由 W3C 指定的一种通用标记语言。信息技术（IT）行业使用许多基于 XML 的语言作为数据描述性语言。
  XML 标签 类似 HTML 标签，但由于 XML 允许用户定义他们自己的标签，所以 XML 更加灵活。从这个层面来看，XML 表现的像一种元语言 —— 也就是说，它可以被用来定义其他语言，例如 RSS。不仅如此，HTML 是陈述性语言，而 XML 是数据描述性语言。这意味着 XML 除了 Web 之外有更远更广的应用。例如，Web 服务可以利用 XML 去交换请求和响应。

> &emsp;note：RSS（简易信息聚合）指的是用于发布站点更新的数种 XML 文档格式。当你订阅一个网站的 RSS 摘要时，网站会将 RSS 信息和更新信息发送到一个称为资讯（feed）的 RSS 文档中，因此你无需手动查看所有喜爱的网站就可以获取这些网站的更新动态。

&emsp;作为一个简单的例子，以下代码创建一个圆和一个矩形：

```javascript
<svg version="1.1"
     baseProfile="full"
     width="300" height="200"
     xmlns="http://www.w3.org/2000/svg">
  <rect width="100%" height="100%" fill="black" />
  <circle cx="150" cy="100" r="90" fill="blue" />
</svg>
```

&emsp;从上面的例子可以看出，SVG 很容易手工编码。是的，你可以在文本编辑器中手动编写简单的 SVG，但是对于复杂的图像，这很快就开始变得非常困难。为了创建 SVG 图像，大多数人使用矢量图形编辑器，如 Inkscape 或 Illustrator。这些软件包允许你使用各种图形工具创建各种插图，并创建照片的近似值（例如 Inkscape 的跟踪位图功能）。

&emsp;SVG 除了迄今为止所描述的以外还有其他优点：

+ 矢量图像中的文本仍然可访问（这也有利于 SEO）。
+ SVG 可以很好地适应样式/脚本，因为图像的每个组件都是可以通过 CSS 或通过 JavaScript 编写的样式的元素。

&emsp;那么为什么会有人想使用光栅图形而不是 SVG？其实 SVG 确实有一些缺点：

+ SVG 非常容易变得复杂，这意味着文件大小会增加; 复杂的 SVG 也会在浏览器中占用很长的处理时间。
+ SVG 可能比栅格图像更难创建，具体取决于你尝试创建哪种图像。
+ 旧版浏览器不支持 SVG，因此如果你需要在网站上支持旧版本的 IE，则可能不适合（SVG 从 IE9 开始得到支持）。

&emsp;由于上述原因，光栅图形更适合照片那样复杂精密的图像。

> &emsp;note：在 Inkscape 中，将文件保存为纯 SVG 以节省空间。

## 将 SVG 添加到页面

&emsp;在本节中，我们将介绍将 SVG 矢量图形添加到网页的不同方式。

### 快捷方式：`<img>`

&emsp;要通过 `<img>` 元素嵌入 SVG，你只需要按照预期的方式在 src 属性中引用它。你将需要一个 height 或 width 属性（或者如果你的 SVG 没有固有的宽高比）。

```javascript
<img
    src="equilateral.svg"
    alt="triangle with all three sides equal"
    height="87px"
    width="100px" />
```

### 优点

+ 快速，熟悉的图像语法与 alt 属性中提供的内置文本等效。
+ 可以通过在 `<a>` 元素嵌套 `<img>`，使图像轻松地成为超链接。

### 缺点

+ 无法使用 JavaScript 操作图像。
+ 如果要使用 CSS 控制 SVG 内容，则必须在 SVG 代码中包含内联 CSS 样式。（从 SVG 文件调用的外部 CSS 不起作用）
+ 不能用 CSS 伪类来重设图像样式（如：focus）。

### 疑难解答和跨浏览器支持

&emsp;对于不支持 SVG（IE 8 及更低版本，Android 2.3 及更低版本）的浏览器，你可以从 src 属性引用 PNG 或 JPG，并使用 srcset 属性 只有最近的浏览器才能识别）来引用 SVG。在这种情况下，仅支持浏览器将加载 SVG - 较旧的浏览器将加载 PNG：

```javascript
<img src="equilateral.png" alt="triangle with equal sides" srcset="equilateral.svg">
```

&emsp;你还可以使用 SVG 作为 CSS 背景图像，如下所示。在下面的代码中，旧版浏览器会坚持他们理解的 PNG，而较新的浏览器将加载 SVG：

```javascript
background: url("fallback.png") no-repeat center;
background-image: url("image.svg");
background-size: contain;
```

&emsp;像上面描述的 `<img>` 方法一样，使用 CSS 背景图像插入 SVG 意味着它不能被 JavaScript 操作，并且也受到相同的 CSS 限制。

&emsp;如果 SVG 根本没有显示，可能是因为你的服务器设置不正确。如果是这个问题，[这篇文章](https://developer.mozilla.org/zh-CN/docs/Web/SVG/Tutorial/Getting_Started#a_word_on_webservers) 将告诉你正确方向。

## 如何在 HTML 中引入 SVG 代码

&emsp;你还可以在文本编辑器中打开 SVG 文件，复制 SVG 代码，并将其粘贴到 HTML 文档中 - 这有时称为将 SVG 内联或内联 SVG。确保你的 SVG 代码在 `<svg></svg>` 标签中（不要在外面添加任何内容）。这是一个非常简单的示例，你可以粘贴到文档中：

```javascript
<svg width="300" height="200">
    <rect width="100%" height="100%" fill="green" />
</svg>
```

### 优点

+ 将 SVG 内联减少 HTTP 请求，可以减少加载时间。
+ 你可以为 SVG 元素分配 class 和 id，并使用 CSS 修改样式，无论是在 SVG 中，还是 HTML 文档中的 CSS 样式规则。实际上，你可以使用任何 SVG 外观属性 作为 CSS 属性。
+ 内联 SVG 是唯一可以让你在 SVG 图像上使用 CSS 交互（如：focus）和 CSS 动画的方法（即使在常规样式表中）。
+ 你可以通过将 SVG 标记包在 `<a>` 元素中，使其成为超链接。

### 缺点

+ 这种方法只适用于在一个地方使用的 SVG。多次使用会导致资源密集型维护（resource-intensive maintenance）。
+ 额外的 SVG 代码会增加 HTML 文件的大小。
+ 浏览器不能像缓存普通图片一样缓存内联 SVG。
+ 你可能会在 `<foreignObject>` 元素中包含回退，但支持 SVG 的浏览器仍然会下载任何后备图像。你需要考虑仅仅为支持过时的浏览器，而增加额外开销是否真的值得。

## 如何使用 `<iframe>` 嵌入 SVG

&emsp;你可以在浏览器中打开 SVG 图像，就像网页一样。因此，使用 `<iframe>` 嵌入 SVG 文档就像我们在从对象到 iframe - 其他嵌入技术中进行研究一样。

&emsp;这是一个快速回顾：

```javascript
<iframe src="triangle.svg" width="500" height="500" sandbox>
    <img src="triangle.png" alt="Triangle with three unequal sides" />
</iframe>
```

&emsp;这绝对不是最好的方法：

### 缺点

+ 如你所知， iframe 有一个回退机制，如果浏览器不支持 iframe，则只会显示回退。
+ 此外，除非 SVG 和你当前的网页具有相同的 origin，否则你不能在主页面上使用 JavaScript 来操纵 SVG。

> &emsp;note：Web 内容的源由用于访问它的 URL 的方案（协议），主机 (域名) 和端口定义。只有当方案、主机和端口都匹配时，两个对象具有相同的起源。某些操作仅限于同源内容，而可以使用 CORS 解除这个限制。

> &emsp;note：CORS（Cross-Origin Resource Sharing 跨域资源共享）是一个系统，它由一系列传输的 HTTP 头组成，这些 HTTP 头决定浏览器是否阻止前端 JavaScript 代码获取跨域请求的响应。同源安全策略默认阻止 "跨域" 获取资源。但是 CORS 给了 web 服务器这样的权限，即服务器可以选择，允许跨域请求访问到它们的资源。

## 动手学习：使用 SVG

&emsp;在这个动手学习部分中，我们希望你能够体验一下 SVG 的乐趣。在下面的 "input" 部分，你将看到我们已经提供了一些样例来开始使用。你还可以访问 SVG 元素参考，了解更多关于 SVG 可以使用的其他玩具的细节，也可以尝试一下。这部分都是为了锻炼你的研究技巧，并且有一些乐趣。

```javascript
  <svg width="100%" height="100%">
    <rect width="100%" height="100%" fill="red" />
    <circle cx="100%" cy="100%" r="150" fill="blue" stroke="black" />
    <polygon points="120,0 240,225 0,225" fill="green"/>
    <text x="50" y="100" font-family="Verdana" font-size="55"
          fill="white" stroke="black" stroke-width="2">
            Hello!
    </text>
  </svg>
```

## 总结

&emsp;本文提供了一个矢量图形和 SVG 的快速浏览，让你了解他们的作用，以及如何在网页中引入 SVG。

## 响应式图片

&emsp;现在有许多不同的设备类型能够浏览网络 - 从手机到台式电脑 - 在现代网络世界中掌握的一个基本概念就是响应式设计。这是指创建可以自动更改其功能以适应不同屏幕尺寸、分辨率等的网页。稍后将在 CSS 模块中详细介绍这一点，但是现在我们将看看 HTML 可用于创建响应式图像的工具，包括 `<picture>` 元素。响应式图片仅仅只是响应式 web 设计的一部分，奠定了响应式 web 设计的良好基础。

## 为什么要用自适应的图片？

&emsp;让我们来看一个典型的场景。一个典型的网站可能会有一张页首图片，这让访问者看起来感到愉快。图片下面可能会添加一些内容图像。页首图像的跨度可能是整个页首的宽度。而内容图像会适应内容纵列的某处。此处有个简单的例子：

```javascript
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Not responsive demo</title>
    <style>
      html {
        font-family: sans-serif;
        background-color: gray;
      }

      body {
        width: 100%;
        max-width: 1200px;
        margin: 0 auto;
        background-color: white;
      }

      header {
        background: url(header.jpg) no-repeat center;
        height: 200px;
      }

      section {
        padding: 20px;
      }

      p {
        line-height: 1.5;
      }

      img {
        display: block;
        margin: 0 auto;
        max-width: 100%;
      }
    </style>
  </head>
  <body>
    <header>
      
    </header>

    <main>
      <section>
        <h1>My website</h1>

        <p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris eget venenatis ligula. Ut lacinia at dolor vitae pulvinar. Aliquam pretium dignissim eros. Integer faucibus, dui non aliquet bibendum, lectus orci lobortis odio, ornare porttitor est tellus eget velit. Nulla eros elit, malesuada id neque vel, viverra vehicula neque. Nullam auctor turpis non leo iaculis finibus. Quisque blandit arcu venenatis libero tempor, ac pulvinar ligula dapibus.</p>

        <img src="elva-800w.jpg" alt="Chris standing up holding his daughter Elva">

        <p>Suspendisse potenti. Ut in luctus eros. Mauris pulvinar vehicula aliquet. Etiam imperdiet eleifend luctus. Duis ut justo nec eros ornare consectetur. Vestibulum convallis condimentum varius. Maecenas rutrum porta varius. Phasellus volutpat sem id sagittis luctus. Morbi vitae quam vitae nisi iaculis dignissim.</p>

        <img src="elva-fairy-800w.jpg" alt="Elva dressed as a fairy">

        <p>Header image originally by <a href="https://www.flickr.com/photos/miwok/17086751527/">Miwok</a>.</p>
      </section>
    </main>
  </body>
</html>
```

&emsp;这个网页在宽屏设备上表现良好，例如笔记本电脑或台式机。我们不会在这一节课中讨论 CSS，除了下面提到的那些：

+ 正文内容被设置的最大宽度为 1200 像素 —— 在高于该宽度的可视窗口中，正文保持在 1200 像素，并将其本身置于可用空间的中间。在该宽度以下的可视窗口中，正文将保持在可视窗口宽度的 100%。
+ 页眉图像被设置为使其中心始终位于页眉的中心，无论页眉的宽度是多少。所以如果网站被显示在窄屏上，图片中心的重要细节（里面的人）仍然可以看到，而两边超出的部分都消失了。它的高度是 200px。
+ 内容图片已经被设置为如果 body 元素比图像更小，图像就开始缩小，这样图像总是在正文里，而不是溢出正文。

&emsp;然而，当你尝试在一个狭小的屏幕设备上查看本页面时，问题就会产生。网页的页眉看起来还可以，但是页眉这张图片占据了屏幕的一大部分的高度，在这个尺寸下，你很难看到在第一张图片内容里的人。

&emsp;一个改进的方法是，当网站在狭窄的屏幕上观看时，显示一幅图片的包含了重要细节的裁剪版本，第二个被裁剪的图片会在像平板电脑这样的中等宽度的屏幕设备上显示，这就是众所周知的美术设计问题（art direction problem）。

&emsp;另外，如果是在小屏手机屏幕上显示网页，那么没有必要在网页上嵌入这样大的图片。这被称之为分辨率切换问题（resolution switching problem）。位图有固定数量的像素宽，固定数量的像素高，与 矢量图 外观相同，但本质不同。如果显示尺寸大于原始尺寸，一张自身较小的位图看起来会有颗粒感（矢量图则不会）。

&emsp;相反，没有必要在比图片实际尺寸小的屏幕上显示一张大图，这样做会浪费带宽 —— 当可以在设备上使用小图像时，手机用户尤其不愿意因为下载用于桌面的大图像而浪费带宽。理想的情况是当访问网站时依靠不同的设备来提供不同的分辨率图片和不同尺寸的图片。

&emsp;让事情变得复杂的是，有些设备有很高的分辨率，为了显示的更出色，可能需要超出你预料的更大的图像。这从本质上是一样的问题，但在环境上有一些不同。

&emsp;你可能会认为矢量图形能解决这些问题，在某种程度上是这样的 —— 它们无论是文件大小还是比例都合适，无论在哪里你都应该尽可能的使用它们。然而，它们并不适合所有的图片类型，虽然在简单图形、图案、界面元素等方面较好，但如果是有大量的细节的照片，创建矢量图像会变得非常复杂。像 JPEG 格式这样的位图会更适合上面例子中的图像。

&emsp;当 web 第一次出现时，这样的问题并不存在，在上世纪 90 年代中期，仅仅可以通过笔记本电脑和台式机来浏览 web 页面，所以浏览器开发者和规范制定者甚至没有想到要实现这种解决方式（响应式开发）。最近应用的响应式图像技术，通过让浏览器提供多个图像文件来解决上述问题，比如使用相同显示效果的图片但包含多个不同的分辨率（分辨率切换），或者使用不同的图片以适应不同的空间分配（美术设计）。

> &emsp;note：在这篇文章中讨论的新特性 — `srcset/sizes/<picture>` — 都已经被新版本的现代浏览器和移动浏览器所支持（包括 Edge，而不是 IE）。

## 怎样创建自适应的图片？

&emsp;在这一部分中，我们将看看上面说明的两个问题，并且展示怎样用 HTML 的响应式图片来解决这些问题。需要注意的是，如以上示例所示，在本节中我们将专注于 HTML 的 `<img>`，但网站页眉的图片仅是装饰性的，实际上应该要用 CSS 的背景图片来实现。CSS 是比 HTML 更好的响应式设计的工具，我们会在未来的 CSS 模块中讨论。

### 分辨率切换：不同的尺寸

&emsp;那么，我们想要用分辨率切换解决什么问题呢？我们想要显示相同的图片内容，仅仅依据设备来显示更大或更小的图片 —— 这是我们在示例中使用第二个内容图像的情况。标准的 `<img>` 元素传统上仅仅让你给浏览器指定唯一的资源文件。

```javascript
<img src="elva-fairy-800w.jpg" alt="Elva dressed as a fairy">
```

&emsp;我们可以使用两个新的属性 —— srcset 和 sizes —— 来提供更多额外的资源图像和提示，帮助浏览器选择正确的一个资源。你可以看到如下示例代码：

```javascript
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width">
    <title>Responsive HTML images demo</title>
    <style>
      html {
        font-family: sans-serif;
        background-color: gray;
      }

      body {
        max-width: 1200px;
        margin: 0 auto;
        background-color: white;
      }

      header {
        background: url(header.jpg) no-repeat center;
        height: 200px;
      }

      section {
        padding: 20px;
      }

      p {
        line-height: 1.5;
      }

      img {
        display: block;
        margin: 0 auto;
        max-width: 100%;
      }
    </style>
  </head>
  <body>
    <header>

    </header>

    <main>
      <section>
        <h1>My website</h1>

        <p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris eget venenatis ligula. Ut lacinia at dolor vitae pulvinar. Aliquam pretium dignissim eros. Integer faucibus, dui non aliquet bibendum, lectus orci lobortis odio, ornare porttitor est tellus eget velit. Nulla eros elit, malesuada id neque vel, viverra vehicula neque. Nullam auctor turpis non leo iaculis finibus. Quisque blandit arcu venenatis libero tempor, ac pulvinar ligula dapibus.</p>

        <picture>
          <source media="(max-width: 799px)" srcset="elva-480w-close-portrait.jpg">
          <source media="(min-width: 800px)" srcset="elva-800w.jpg">
          <img src="elva-800w.jpg" alt="Chris standing up holding his daughter Elva">
        </picture>

        <p>Suspendisse potenti. Ut in luctus eros. Mauris pulvinar vehicula aliquet. Etiam imperdiet eleifend luctus. Duis ut justo nec eros ornare consectetur. Vestibulum convallis condimentum varius. Maecenas rutrum porta varius. Phasellus volutpat sem id sagittis luctus. Morbi vitae quam vitae nisi iaculis dignissim.</p>

        <img srcset="elva-fairy-480w.jpg 480w,
                     elva-fairy-800w.jpg 800w"
             sizes="(max-width: 600px) 480px,
                    800px"
             src="elva-fairy-800w.jpg" alt="Elva dressed as a fairy">

        <p>Header image originally by <a href="https://www.flickr.com/photos/miwok/17086751527/">Miwok</a>.</p>
      </section>
    </main>
  </body>
</html>
```

```javascript
<img srcset="elva-fairy-320w.jpg 320w,
             elva-fairy-480w.jpg 480w,
             elva-fairy-800w.jpg 800w"
     sizes="(max-width: 320px) 280px,
            (max-width: 480px) 440px,
            800px"
     src="elva-fairy-800w.jpg" alt="Elva dressed as a fairy">
```

&emsp;srcset 和 sizes 属性看起来很复杂，但是如果你按照上图所示进行格式化，那么他们并不是很难理解，每一行有不同的属性值。每个值都包含逗号分隔的列表。列表的每一部分由三个子部分组成。让我们来看看现在的每一个内容：

&emsp;srcset 定义了我们允许浏览器选择的图像集，以及每个图像的大小。在每个逗号之前，我们写：

1. 一个文件名 (elva-fairy-480w.jpg.)
2. 一个空格
3. 图像的固有宽度（以像素为单位）（480w）。注意，这里使用宽度描述符 w，而非你可能期望的 px。这是图像的真实大小。

&emsp;sizes 定义了一组媒体条件（例如屏幕宽度）并且指明当某些媒体条件为真时，什么样的图片尺寸是最佳选择 — 我们在之前已经讨论了一些提示。在这种情况下，在每个逗号之前，我们写：

1. 一个媒体条件（(max-width:480px)）—— 你会在 CSS topic中学到更多的。但是现在我们仅仅讨论的是媒体条件描述了屏幕可能处于的状态。在这里，我们说 "当可视窗口的宽度是 480 像素或更少"。
2. 一个空格。
3. 当媒体条件为真时，图像将填充的槽的宽度（440px）。

&emsp;对于槽的宽度，你也许会提供一个固定值 (px, em) 或者是一个相对于视口的长度 (vw)，但不是百分比。你也许已经注意到最后一个槽的宽度是没有媒体条件的，它是默认的，当没有任何一个媒体条件为真时，它就会生效。当浏览器成功匹配第一个媒体条件的时候，剩下所有的东西都会被忽略，所以要注意媒体条件的顺序。

&emsp;所以，有了这些属性，浏览器会：

1. 查看设备宽度
2. 检查 sizes 列表中哪个媒体条件是第一个为真
3. 查看给予该媒体查询的槽大小
4. 加载 srcset 列表中引用的最接近所选的槽大小的图像

&emsp;就是这样！所以在这里，如果支持浏览器以视窗宽度为 480px 来加载页面，那么 (max-width: 480px) 的媒体条件为真，因此 440px 的槽会被选择，所以 elva-fairy-480w.jpg 将加载，因为它的的固定宽度（480w）最接近于 440px。800px 的照片大小为 128KB 而 480px 版本仅有 63KB 大小 — 节省了 65KB。现在想象一下，如果这是一个有很多图片的页面。使用这种技术会节省移动端用户的大量带宽。

&emsp;老旧的浏览器不支持这些特性，它会忽略这些特征。并继续正常加载 src 属性引用的图像文件。

> &emsp;note：在 HTML 文件中的 `<head>` 标签里，你将会找到这一行代码 `<meta name="viewport" content="width=device-width">`: 这行代码会强制地让手机浏览器采用它们真实可视窗口的宽度来加载网页（有些手机浏览器会提供不真实的可视窗口宽度，然后加载比浏览器真实可视窗口的宽度大的宽度的网页，然后再缩小加载的页面，这样的做法对响应式图片或其他设计，没有任何帮助。我们会在未来的模块教给你更多关于这方面的知识）。

### 分辨率切换：相同的尺寸，不同的分辨率

&emsp;如果你支持多种分辨率显示，但希望每个人在屏幕上看到的图片的实际尺寸是相同的，你可以让浏览器通过 srcset 和 x 语法结合 —— 一种更简单的语法 —— 而不用 sizes，来选择适当分辨率的图片。你可以看一个例子：

```javascript
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width">
    <title>Responsive HTML images demo</title>
    <style>

      img {
        width: 320px;
      }

    </style>
  </head>
  <body>
    

        <img srcset="elva-fairy-320w.jpg,
                     elva-fairy-480w.jpg 1.5x,
                     elva-fairy-640w.jpg 2x"
             src="elva-fairy-640w.jpg" alt="Elva dressed as a fairy"> 

        
  </body>
</html>
```

&emsp;在这个例子中，下面的 CSS 会应用在图片上，所以它的宽度在屏幕上是 320 像素（也称作 CSS 像素）：

&emsp;在这种情况下，sizes 并不需要 —— 浏览器只是计算出正在显示的显示器的分辨率，然后提供 srcset 引用的最适合的图像。因此，如果访问页面的设备具有标准/低分辨率显示，一个设备像素表示一个 CSS 像素，elva-fairy-320w.jpg 会被加载（1x 是默认值，所以你不需要写出来）。如果设备有高分辨率，两个或更多的设备像素表示一个 CSS 像素，elva-fairy-640w.jpg 会被加载。640px 的图像大小为 93KB，320px 的图像的大小仅仅有 39KB。

### 美术设计

&emsp;回顾一下，美术设计问题涉及要更改显示的图像以适应不同的图像显示尺寸。例如，如果在桌面浏览器上的一个网站上显示一张大的、横向的照片，照片中央有个人，然后当在移动端浏览器上浏览这个网站时，照片会缩小，这时照片上的人会变得非常小，看起来会很糟糕。这种情况可能在移动端显示一个更小的肖像图会更好，这样人物的大小看起来更合适。`<picture>` 元素允许我们这样实现。

&emsp;我们有一张图片需要美术设计：

```javascript
<img src="elva-800w.jpg" alt="Chris standing up holding his daughter Elva">
```

&emsp;让我们改用 `<picture>`！就像 `<video>` 和 `<audio>`，`<picture>` 素包含了一些 `<source>` 元素，它使浏览器在不同资源间做出选择，紧跟着的是最重要的 `<img>` 元素。

```javascript
<picture>
  <source media="(max-width: 799px)" srcset="elva-480w-close-portrait.jpg">
  <source media="(min-width: 800px)" srcset="elva-800w.jpg">
  <img src="elva-800w.jpg" alt="Chris standing up holding his daughter Elva">
</picture>
```

+ `<source>` 元素包含一个 media 属性，这一属性包含一个媒体条件 —— 就像第一个 srcset 例子，这些条件来决定哪张图片会显示 —— 第一个条件返回真，那么就会显示这张图片。在这种情况下，如果视窗的宽度为 799px 或更少，第一个 `<source>` 元素的图片就会显示。如果视窗的宽度是 800px 或更大，就显示第二张图片。
+ srcset 属性包含要显示图片的路径。请注意，正如我们在 `<img>` 上面看到的那样，`<source>` 可以使用引用多个图像的 srcset 属性，还有 sizes 属性。所以你可以通过一个 `<picture>` 元素提供多个图片，不过也可以给每个图片提供多分辨率的图片。实际上，你可能不想经常做这样的事情。
+ 在任何情况下，你都必须在 `</picture>` 之前正确提供一个 `<img>` 元素以及它的 src 和 alt 属性，否则不会有图片显示。当媒体条件都不返回真的时候（你可以在这个例子中删除第二个 `<source>` 元素），它会提供图片；如果浏览器不支持 `<picture>` 元素时，它可以作为后备方案。

> &emsp;note：你应该仅仅当在美术设计场景下使用 media 属性；当你使用 media 时，不要在 sizes 属性中也提供媒体条件。

### 为什么我们不能使用 CSS 或 JavaScript 来做到这一效果？

&emsp;当浏览器开始加载一个页面，它会在主解析器开始加载和解析页面的 CSS 和 JavaScript 之前先下载 (预加载) 任意的图片。这是一个非常有用的技巧，平均下来减少了页面加载时间的 20%。但是，这对响应式图片一点帮助都没有，所以需要类似 srcset 的实现方法。因为你不能先加载好 `<img>` 元素后，再用 JavaScript 检测可视窗口的宽度，如果觉得大小不合适，再动态地加载小的图片替换已经加载好的图片，这样的话，原始的图像已经被加载了，然后你又加载了小的图像，这样的做法对于响应式图像的理念来说，是很糟糕的。

### 大胆的使用现代图像格式

&emsp;有很多令人激动的新图像格式（例如 WebP 和 JPEG-2000）可以在有高质量的同时有较低的文件大小。然而，浏览器对其的支持参差不齐。

&emsp;`<picture>` 让我们能继续满足老式浏览器的需要。你可以在 type 属性中提供 MIME 类型，这样浏览器就能立即拒绝其不支持的文件类型：

```javascript
<picture>
  <source type="image/svg+xml" srcset="pyramid.svg">
  <source type="image/webp" srcset="pyramid.webp">
  <img src="pyramid.png" alt="regular pyramid built from four equilateral triangles">
</picture>
```

+ 不要使用 media 属性，除非你也需要美术设计。
+ 在 `<source>` 元素中，你只可以引用在 `type` 中声明的文件类型。
+ 像之前一样，如果必要，你可以在 `srcset` 和 `sizes` 中使用逗号分割的列表。

&emsp;美术设计：当你想为不同布局提供不同剪裁的图片——比如在桌面布局上显示完整的、横向图片，而在手机布局上显示一张剪裁过的、突出重点的纵向图片，可以用 `<picture>` 元素来实现。

&emsp;分辨率切换：当你想要为窄屏提供更小的图片时，因为小屏幕不需要像桌面端显示那么大的图片；以及你想为高/低分辨率屏幕提供不同分辨率的图片时，都可以通过 vector graphics (SVG images)、 srcset 以及 sizes 属性来实现。

## 参考链接
**参考链接:🔗**
+ [HTML 开发者指南](https://developer.mozilla.org/zh-CN/docs/Learn/HTML)
+ [HTML 参考](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Reference)
