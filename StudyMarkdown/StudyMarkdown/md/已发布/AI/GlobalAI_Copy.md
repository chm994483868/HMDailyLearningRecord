# GlobalAI（免🪜、免💰的ChatGPT 套壳客户端） 开发过程总结（一）：缘起

&emsp;趁着 ChatGPT 爆火，咱也做了一个 ChatGPT 套壳客户端，主 APP 是使用 Flutter 进行开发，然后自定义键盘扩展（Custom Keyboard Extension）、分享扩展（Share Extension）和 Siri/Shortcts 扩展（Intents Extension）部分是 iOS 原生的 Application Extension 开发，emmmm... 虽然用了 Flutter 但是暂时没有适配 android 端，目前仅做了 iOS 版并上架了 App Store，（免费、免🪜、免账号）完全免费使用，大家可以在 App Store 搜索 "globalai" 下载试用！暂时没有完全开源的计划，但是接下来我会写一系列文章把整个开发过程进行讲解以及把核心代码列出来，希望能起个抛砖引玉的作用。我现在的构思是至少 6 篇文章完成这个系列：

1. 如何申请 Azure OpenAI 服务（获得一个一年内无限免费使用的 ChatGPT 账号）。
2. 如何在 Flutter 和 iOS Native 中调用 OpenAI Api 服务。(由于在 iOS 扩展机制中主 App 并没有启动，所以只能写两套请求服务)
3. 如何使用 Flutter 搭建 GlobalAI 主 App 页面。(基础的 Flutter 开发技巧)
4. iOS 原生中如何开发自定义键盘扩展，在有键盘地方随时使用 ChatGPT。
5. iOS 原生中如何开发分享扩展。
6. iOS 原生中如何开发 Siri/Shortcuts 扩展，即在 Siri 中使用 ChatGPT。

<figure class="half">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/097a5e32f93448f0ba7e3b87767f0a1e~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ab83919e1ba74cae8f8700f0c9959126~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;最初我是看了这篇文章：[设计｜怎么在 iOS、Android 的全局层面使用 ChatGPT](https://twitter.com/nishuang/status/1540256151375323137?s=46&t=2pmBCT9eqxZucsai-FwgdA)，看完当时特别激动，感觉 ChatGPT 和 iOS 的键盘扩展简直是绝配！于是报着对 AI 以及 ChatGPT 的极大热情就开始着手也开发一个 ChatGPT 客户端。

## GlobalAI 客户端立意

&emsp;我给这个项目也起了一个：GlobalAI 的名字，立意是在手机的全局层面使用 AI。主要实现方式是利用手机系统的各种扩展机制跳出主 APP，更快捷的使用 AI。

&emsp;在 iOS 的各种扩展机制中自定义键盘无疑是最方便的，但是它的 "侵入性" 也略高，一些害怕隐私泄露的用户估计不太想给键盘扩展的 "允许完全访问" 权限，但其实在 iOS 系统的隔离机制下如果用户不主动切换到某个指定的键盘扩展，那么此键盘扩展是无法读取到其他键盘的输入内容的，所以大可不必紧张，放心点开 "允许完全访问" 即可。如果不打开的话键盘扩展连基本的网络请求都做不到，是完全无法与 GPT 进行交互的，然后为了引导用户打开 "允许完全访问" 权限，GlobalAI 添加了如下提示，并在设置中途放置了一个添加键盘扩展的指导页面。

<figure class="half">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/1fe6dea64c1f42f88cbbc3c8e2596e60~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c553a23fdef94acb86a27b83cc8b0f7c~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;第二种扩展方式便是 Siri 和 Shortcuts，我们可以通过 Siri 直接与 GPT 语音交互，或者添加我制作好的捷径流程与 GPT 进行交互。

<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/8bf258a2d0e34df0bd64728dd9f94970~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/008d2895f5c241d6a010b0e2d5a238d8~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;异或是熟悉捷径操作的小伙伴自己添加捷径流程。例如 ChatGPT 响应完成后直接朗读出来。

<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c1ecdba7fc4f4c05a912a67a1c311ad6~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/4e9b3deb5f4f43069f904cbc8014be50~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;第三种扩展方式是分享扩展，在其他 APP 中当有文字选中时，一般会有个 menu 菜单，最常见的是在 WebView 中选中文字时会弹出，我们可以选中想要与 GPT 交互的文字内容，然后点击 menu 菜单中的分享按钮（没有分享时可点击复制迂回一下），然后选中 GlobalAI 列出的 Prompt 就可以和 GPT 交互了，另外 GPT 也可以访问链接中的内容，我们可以直接抛给它一个链接，让它帮我们总结或翻译链接中的内容，例如在 Safari 中直接点击底部的分享按钮选中 GlobalAI 即可与 GPT 交互。

<figure class="half">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/87da792e0266416fa833f1b99b8434e9~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/6fb3342416e24c6386b92f5ff498d4f7~tplv-k3u1fbpfcp-watermark.image?">
</figure>

## 在 iOS 键盘中使用 ChatGPT（Custom Keyboard Extension）

&emsp;GlobalAI 是把自定义键盘扩展作为核心来开发的，我们可以把输入框或者粘贴板作为输入起点（与 ChatGPT 交互的起点），GlobalAI 读取其中的内容然后与 ChatGPT 交互，例如我们在微信群里直接长按复制群友的问题，然后切换键盘让 GPT 对此问题进行回答，也可先在输入框里面输入自己的问题，然后切换键盘让 GPT 进行回答，GPT 响应的内容会直接插入当前输入框，然后等 GPT 响应完成后我们直接点击发送按钮即可。

&emsp;其它的例如美团、淘宝等需要打字评论时，我们可以先用系统键盘输入商品的名字，然后切换键盘点击评价的 Prompt，即可让 GPT 自动替我们生成评论内容，还有其它的例如：抖音评论回复、微博内容总结、邮件内容回复等等也是同样的操作逻辑。我们可以在主 App 中设置好各种 Prompt，如果对 GPT 生成的内容不满意，也可以慢慢编辑调试 Prompt 的内容直到 GPT 生成的内容满意为止，因为键盘区域过小，我们也可以自由开关是否在键盘区域显示某个 Prompt，同时在键盘扩展中的与 GPT 的聊天记录我们也可以自行决定是否同步到主 App。后续我们就可以随时切换键盘自由使用 ChatGPT 了。

&emsp;在微信/淘宝/微博中使用 ChatGPT:

<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/35c9f5a9a5d144b8b06ed96281bb36e9~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/7f819d4bdbc54fe1be8689ec4a2b086b~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;在 GlobalAI 中配置键盘扩展：

<figure class="half">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e6d11ac62f6b40db8117e8bbe2a79bb4~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/ad544bb90ffc4f1db0239d9459710383~tplv-k3u1fbpfcp-watermark.image?">
</figure>


<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/0ad1b8b373fc499ba79f0ed488c57f38~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/87cace5680e14270892fead594d52ee5~tplv-k3u1fbpfcp-watermark.image?">
</figure>

## 在 Siri 中使用 ChatGPT（Intents Extension）

&emsp;除了在键盘中频繁打字比较累手外，我们还可以在 Siri中使用 ChatGPT，不过暂时 GlobalAI 中 Siri 的使用场景还比较单薄，仅仅支持一问一答的形式，其实开一下脑洞的话我们完全可以把 Siri + ChatGPT + Shortcuts 组合起来，可以做到以语音的形式来控制我们的手机，完成一些 iOS 系统开放的功能，同时我们也可以通过自己手动补充中间步骤来调起第三方 APP 提供的功能。（举一些第三方的例子在这里，例如语音控制给某个好友发微信）

<figure class="half">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/b5809749f6f748efa8edb64a8d8879f1~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/5bb3c50be19649d9b8d4242602e6db33~tplv-k3u1fbpfcp-watermark.image?">
</figure>

## 在分享菜单中使用 ChatGPT（Share Extension）

&emsp;选中文本可以对他们进行翻译、总结、扩展等等，也可以对链接进行翻译和总结等。

<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/60a9cd1c1df64144a1697a585411c224~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/12d4db25c81c45158c7c22b989dbaa35~tplv-k3u1fbpfcp-watermark.image?">
</figure>

## GlobalAI 主 App 的功能总结

&emsp;GlobalAI 主 APP 是一个涵盖所有功能的 ChatGPT 客户端，包括以下主要功能：

1. 完全自定义添加 Api Key 以及代理 URL，方便随时切换 key 来源，例如: OpenAI 的官方账号、Api2d 的账号、Azure OpenAI 账号。
2. 完全自定义的创建会话，所有的参数可定制。
3. 预置会话和预置 Prompt，如果有好的 Prompt 也可发邮件给我，我会添加上去，方便大家使用。
4. 会话详情页面：随时开关上下文、自动复制、点击开启新会话、长按清空聊天记录，控制上下文区间等。
5. ...

<figure class="half">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/0d35f233f8df4da9b89edd598825fd6c~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/4570507034f843658e24e1a70d2908a2~tplv-k3u1fbpfcp-watermark.image?">
</figure>

<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/68040c7db3d440c7ad3e8a5b46116c25~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/3f32d8d2ca244d06ad631528b7dbdb1d~tplv-k3u1fbpfcp-watermark.image?">
</figure>

<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/07bcfedeb0594fc68168427d76a8086b~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/c0924b5e5ad44c169d2f1b928089a582~tplv-k3u1fbpfcp-watermark.image?">
</figure>

&emsp;在 ChatGPT/AI 爆火的当下，网络上真是各种信息满天飞，其中最活跃的便是各种夸大其词卖号的卖课的！我们程序员作为最先吃 ChatGPT 红利的人，一定要有自己的独立判断力不能被那些人蒙蔽了！最直接的方式便是打开 OpenAI 官网与第一手资料进行学习，深入了解 GPT 能干什么不能干什么摸清 GPT 的边界在哪，最直接的方式便是把 OpenAI 官网的介绍文档全部捋一遍，API 的开发文档全部捋一遍，掌握每一个参数的含义，然后再学习下吴恩达老师的 Prompt、LangChain 等的相关课程，所有这些加一起一上午的时间足够了，再然后就是把 GPT 融入到自己的日常工作生活中去，尝试把一些我们熟知结果的任务交给 GPT 来做，然后学习的时候也可把 GPT 作为我们的伴学，有什么模糊的问题时可以先询问它一下，还有就是辅助编程，我日常的开发会涉及到 iOS/SwiftUI、ReactNative、Flutter、JavaScript 等，有什么概念模糊时、有什么 API 记不清时，GPT 第一时间就能直接返回示例代码，真的快速，特别是切换语言时，有时候真的是 "提笔忘字"。当然他有时候也会有幻觉，我也遇到过，最终可能要通过 GPT 加搜索引擎解决问题。总之，拥抱 ChatGPT、拥抱 AI 准没错！

&emsp;本篇介绍先到这里，下篇开始代码干货。  

## 参考链接
**参考链接:🔗**
+ [Introduction](https://platform.openai.com/docs/introduction/overview)
+ [ChatGPT Prompt Engineering for Developers](https://www.youtube.com/watch?v=H4YK_7MAckk)
+ [基于LangChain的大语言模型应用开发](https://www.youtube.com/watch?v=gUcYC0Iuw2g&list=PLiuLMb-dLdWIYYBF3k5JI_6Od593EIuEG)
+ [使用ChatGPT API构建系统](https://www.youtube.com/watch?v=1SZOGp1D17E&list=PLiuLMb-dLdWKjX8ib9PhlCIx1jKMNxMpy)

