# jQuery 学习笔记

&emsp;jQuery 是一个 JavaScript 库。jQuery 是一个轻量级的 "写的少，做的多" 的 JavaScript 库。

&emsp;先来感受一下 jQuery：

&emsp;给页面中的所有 p 标签添加一个 click 事件，点击后隐藏该 p 标签。

```JavaScript
<script src="https://cdn.staticfile.org/jquery/1.10.2/jquery.min.js"></script>
<script>
    $(document).ready(function() {
        $("p").click(function() {
            $(this).hide();
        });
    });
</script>
```

&emsp;jQuery 库包含以下功能：

+ HTML 元素选取
+ HTML 元素操作
+ HTML 事件函数
+ HTML DOM 遍历和修改
+ CSS 操作
+ JavaScript 特效和动画
+ AJAX 异步请求方式
+ Utilities

&emsp;目前 jQuery 有三个大版本：

+ 1.x：兼容 IE 678，使用最为广泛的，官方只做 BUG 维护，功能不再新增。因此一般项目来说，使用 1.x 版本就可以了，最终版本：1.12.4 (2016 年 5 月 20 日)
+ 2.x：不兼容 IE 678，很少有人使用，官方只做 BUG 维护，功能不再新增。如果不考虑兼容低版本的浏览器可以使用 2.x，最终版本：2.2.4 (2016 年 5 月 20 日)
+ 3.x：不兼容 IE 678，只支持最新的浏览器。除非特殊要求，一般不会使用 3.x 版本的，很多老的 jQuery 插件不支持这个版本。目前该版本是官方主要更新维护的版本。最新版本：3.6.0







## 参考链接
**参考链接:🔗**
+ [jQuery 教程](https://www.runoob.com/jquery/jquery-tutorial.html)

