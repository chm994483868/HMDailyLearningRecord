# GlobalAI（免🪜、免💰的ChatGPT 套壳客户端） 开发过程总结（一）：缘起

&emsp;趁着 ChatGPT 爆火，咱也做了一个 ChatGPT 套壳客户端，主 APP 是使用 Flutter 进行开发，然后自定义键盘扩展（Custom Keyboard Extension）、分享扩展（Share Extension）和 Siri/Shortcts 扩展（Intents Extension）部分是 iOS 原生的 Application Extension 开发，emmmm... 虽然用了 Flutter 但是暂时没有适配 android 端，目前仅做了 iOS 版并上架了 App Store，（免费、免🪜、免账号）完全免费使用，大家可以在 App Store 搜索 "globalai" 下载试用！暂时没有完全开源的计划，但是接下来我会写一系列文章把整个开发过程进行讲解以及把核心代码列出来，希望能起个抛砖引玉的作用。我现在的构思是至少 6 篇文章完成这个系列：

1. 如何申请 Azure OpenAI 服务（获得一个一年内无限免费使用的 ChatGPT 账号）。
2. 如何在 Flutter 和 iOS Native 中调用 OpenAI Api 服务。(由于在 iOS 扩展机制中主 App 并没有启动，所以只能写两套请求服务)
3. 如何使用 Flutter 搭建 GlobalAI 主 App 页面。
4. iOS 原生中如何开发自定义键盘扩展。
5. iOS 原生中如何开发分享扩展。
6. iOS 原生中如何开发 Siri/Shortcuts 扩展，即在 Siri 中使用 ChatGPT。

// 这里用两张 GlobalAI：App store 里面黑白两张截图。（为了对称）

<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/3b43d74db8ad4499bb25bc3d9462fe84~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/3b43d74db8ad4499bb25bc3d9462fe84~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;最初我是看了这篇文章：[设计｜怎么在 iOS、Android 的全局层面使用 ChatGPT](https://twitter.com/nishuang/status/1540256151375323137?s=46&t=2pmBCT9eqxZucsai-FwgdA)，看完当时特别激动，感觉 ChatGPT 和 iOS 的键盘扩展简直是绝配！于是报着对 AI 以及 ChatGPT 的极大热情就开始着手也开发一个 ChatGPT 客户端。

## GlobalAI 客户端立意

&emsp;我给这个项目也起了一个：GlobalAI 的名字，立意是在手机的全局层面使用 AI。主要实现方式是利用手机系统的各种扩展机制跳出主 APP，更快捷的使用 AI。

&emsp;在 iOS 的各种扩展机制中自定义键盘扩展无疑是最方便的，但是它的 "侵入性" 也略高，一些害怕隐私泄露的用户估计不太想给键盘扩展的 "允许完全访问" 权限，但其实在 iOS 系统的隔离机制下如果用户不主动切换到某个指定的键盘扩展，那么此键盘扩展是无法读取到其他键盘的输入内容的，所以大可不必紧张，放心点开 "允许完全访问" 即可。如果不打开的话键盘扩展连基本的网络请求都做不到，是完全无法与 GPT 进行交互的，然后为了引导用户打开 "允许完全访问" 权限，GlobalAI 添加了如下提示。

// 这里使用 GlobalAI 设置页面的图和设置键盘扩展的引导网页
![IMG_0998.PNG](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/7f439279b9a543bfb4b72e6d3cd09c5f~tplv-k3u1fbpfcp-watermark.image?)

&emsp;第二种扩展方式便是 Siri 和 Shortcuts，我们可以通过 Siri 直接与 GPT 语音交互，或者添加我制作好的捷径流程与 GPT 进行交互。

// 这里使用系统 Shortcuts 中 GlobalAI 的捷径列表的截图/和 GlobalAI 中点击添加捷径的截图
![IMG_0999.PNG](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e228b4275ae9451aaaeba7cfb504eb01~tplv-k3u1fbpfcp-watermark.image?)

&emsp;第三种扩展方式是分享扩展，在其他 APP 中当有文字选中时，一般会有个 menu 菜单，最常见的是在 WebView 中选中文字时会弹出，我们可以选中想要与 GPT 交互的文字内容，然后点击 menu 菜单中的分享按钮（没有分享时可点击复制迂回一下），然后选中 GlobalAI 列出的 Prompt 就可以和 GPT 交互了，另外 GPT 也可以访问链接中的内容，我们可以直接抛给它一个链接，让它帮我们总结或翻译链接中的内容，例如在 Safari 中直接点击底部的分享按钮选中 GlobalAI 即可与 GPT 交互。

// 这里截图
![IMG_1004.PNG](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ae59790acdb044dd8d2e5a49cbd4efce~tplv-k3u1fbpfcp-watermark.image?)

![IMG_1002.PNG](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/461acf243a204580a5f19e477c08df03~tplv-k3u1fbpfcp-watermark.image?)

![IMG_1003.PNG](https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/553b698903ad4d0d905eca80840affcd~tplv-k3u1fbpfcp-watermark.image?)

## 在 iOS 键盘中使用 ChatGPT（Custom Keyboard Extension）

&emsp;GlobalAI 是把自定义键盘扩展作为核心来开发的，我们可以把输入框或者粘贴板作为输入起点（与 ChatGPT 交互的起点），GlobalAI 读取其中的内容然后与 ChatGPT 交互，例如我们在微信群里直接长按复制群友的问题，然后切换键盘让 GPT 对此问题进行回答，也可先在输入框里面输入自己的问题，然后切换键盘让 GPT 进行回答，GPT 响应的内容会直接插入当前输入框，然后等 GPT 响应完成后我们直接点击发送按钮即可。其它的例如美团、淘宝等需要打字评论时，我们可以先用系统键盘输入商品的名字，然后切换键盘点击评价的 Prompt，即可让 GPT 自动替我们生成评论内容，还有其它的例如：抖音评论回复、微博内容总结、邮件内容回复等等也是同样的操作逻辑。我们可以在主 App 中设置好各种 Prompt，如果对 GPT 生成的内容不满意，也可以慢慢编辑调试 Prompt 的内容直到 GPT 生成的内容满意为止，然后后续我们就可以切换随时切换键盘自由使用了。

// 下面截图介绍 键盘扩展 的功能点，大概能截出好多张图

## 在 Siri 中使用 ChatGPT（Intents Extension）

&emsp;除了在键盘中频繁打字比较累手外，我们还可以在 Siri中使用 ChatGPT，不过暂时 GlobalAI 中 Siri 的使用场景还比较单薄，仅仅支持一问一答的形式，其实开一下脑洞的话我们完全可以把 Siri + ChatGPT + Shortcuts 组合起来，可以做到以语音的形式来控制我们的手机，完成一些 iOS 系统开放的功能，同时我们也可以通过自己手动补充中间步骤来调起第三方 APP 提供的功能。（举一些第三方的例子在这里，例如语音控制给某个好友发微信）


## 在分享菜单中使用 ChatGPT（Share Extension）


&emsp;首先选中文本可以对他们进行翻译、总结、扩展等等，也可以对链接进行翻译和总结。


// 使用分享扩展的截图


## GlobalAI 主 App 的功能总结

&emsp;完全自定义的创建会话，所有的参数可定制。（也可以浅浅介绍下各个参数）

&emsp;预置会话和预置 Prompt，如果用好的 Prompt 也可发邮件，我会添加上去，方便大家使用。

&emsp;会话详情页面：随时开关上下文、自动复制、点击开启新会话、长按清空聊天记录，控制上下文区间。


&emsp;然后来个总结，并展开，下篇开始代码内容。






```
```

// 这个作为文末的总结用吧！

&emsp;在 ChatGPT/AI 爆火的当下，网络上真是各种信息满天飞，其中最活跃的便是各种夸大其词卖号的卖课的！我们程序员作为最先吃 ChatGPT 红利的人，一定要有自己的独立判断力不能被那些人蒙蔽了！最直接的方式便是打开 OpenAI 官网与第一手资料进行学习，深入了解 GPT 能干什么不能干什么摸清 GPT 的边界在哪，最直接的方式便是把 OpenAI 官网的介绍文档全部捋一遍，API 的开发文档全部捋一遍，掌握每一个参数的含义，然后再学习下吴恩达老师的 Prompt、Langchain 等的相关课程，所有这些加一起一上午的时间足够了，再然后就是把 GPT 融入到自己的日常工作生活中去，尝试把一些我们熟知结果的任务交给 GPT 来做，然后学习的时候也可把 GPT 作为我们的伴学，有什么模糊的问题询问它一下，还有就是辅助编程，我日常的开发会涉及到 iOS/SwiftUI、ReactNative、Flutter，有什么概念模糊时、有什么 API 记不清时，GPT 第一时间就能直接返回示例代码，真的超快速，当然他有时候会有幻觉，我也遇到过，最终 GPT 加搜索引擎解决问题。  


## 参考链接
**参考链接:🔗**
+ [casatwy/CTMediator](https://github.com/casatwy/CTMediator)

