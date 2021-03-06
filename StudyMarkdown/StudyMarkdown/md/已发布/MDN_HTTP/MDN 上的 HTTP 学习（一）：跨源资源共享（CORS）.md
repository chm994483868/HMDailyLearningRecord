# MDN 上的 HTTP 学习（一）：跨源资源共享（CORS）

&emsp;**跨域资源共享**（英语：Cross-origin resource sharing，缩写：CORS），用于让网页的受限资源能够被其他域名的页面访问的一种机制。通过该机制，页面能够自由地使用不同源（英语：cross-origin）的图片、样式、脚本、iframes 以及视频。一些跨域的请求（特别是 Ajax）常常会被同源策略（英语：Same-origin policy）所禁止。跨源资源共享定义了一种方式，为的是浏览器和服务器之间能互相确认是否足够安全以至于能使用跨源请求（英语：cross-origin requests）。比起纯粹的同源请求，这将更为自由和功能性的（functionality），但比纯粹的跨源请求更为安全。

&emsp;跨域资源共享是一份浏览器技术的规范，提供了 Web 服务从不同网域传来沙盒脚本的方法，以避开浏览器的同源策略。

&emsp;跨域资源共享标准描述了，新的 HTTP 头部在浏览器有权限的时候，应该以如何的形式发送请求到远程 URLs。虽然服务器会有一些校验和认证，但是浏览器有责任去支持这些头部以及增加相关的限制。对于能够修改数据的 Ajax 和 HTTP 请求方法（特别是 GET 以外的 HTTP 请求，或者搭配某些 MIME 类型的 POST 请求），浏览器必须首先使用 OPTIONS 方法发起一个预检请求（preflight request），从而获知服务端是否允许该跨源请求。服务器确认允许之后，才发起实际的 HTTP 请求。在预检请求的返回中，服务器端也可以通知客户端，是否需要携带身份凭证（包括 Cookies 和 HTTP 认证相关数据）。

&emsp;




&emsp;跨源资源共享 (CORS)（或通俗地译为跨域资源共享）是一种基于 HTTP 头的机制，该机制通过允许服务器标示除了它自己以外的其它 origin（域，协议和端口），使得浏览器允许这些 origin 访问加载自己的资源。跨源资源共享还通过一种机制来检查服务器是否会允许要发送的真实请求，该机制通过浏览器发起一个到服务器托管的跨源资源的 "预检" 请求。在预检中，浏览器发送的头中标示有 HTTP 方法和真实请求中会用到的头。

&emsp;






## 参考链接
**参考链接:🔗**
+ [跨源资源共享（CORS）](https://developer.mozilla.org/zh-CN/docs/Web/HTTP/CORS)
