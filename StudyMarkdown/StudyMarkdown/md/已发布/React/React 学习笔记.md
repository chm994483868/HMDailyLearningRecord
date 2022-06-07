# React 学习笔记

&emsp;React 是一个用于构建用户界面的 JavaScript 库。React 主要用于构建 UI，很多人认为 React 是 MVC 中的 V（视图）。React 起源于 Facebook 的内部项目，用来架设 Instagram 的网站，并于 2013 年 5 月开源。React 拥有较高的性能，代码逻辑非常简单，越来越多的人已开始关注和使用它。

&emsp;React 特点：

1. 声明式设计 − React 采用声明范式，可以轻松描述应用。
2. 高效 − React 通过对 DOM 的模拟，最大限度地减少与 DOM 的交互。
3. 灵活 − React 可以与已知的库或框架很好地配合。
4. JSX − JSX 是 JavaScript 语法的扩展。React 开发不一定使用 JSX ，但建议使用它。
5. 组件 − 通过 React 构建组件，使得代码更加容易得到复用，能够很好的应用在大项目的开发中。
6. 单向响应的数据流 − React 实现了单向响应的数据流，从而减少了重复代码，这也是它为什么比传统数据绑定更简单。

&emsp;React 的一个实例：

```javascript
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Document</title>
</head>

<body>
    <script src="https://cdn.staticfile.org/react/16.4.0/umd/react.development.js"></script>
    <script src="https://cdn.staticfile.org/react-dom/16.4.0/umd/react-dom.development.js"></script>
    <script src="https://cdn.staticfile.org/babel-standalone/6.26.0/babel.min.js"></script>

    <div id="example"></div>
    <script type="text/babel">
        ReactDOM.render(
            <h1>Hello, React!</h1>,
            document.getElementById('example')
        );
    </script>
</body>

</html>
```

&emsp;或者使用 `create-react-app` 工具（下一章节会介绍）创建的 react 开发环境：

```javascript
import React from "react";
import ReactDOM from "react-dom";

function Hello(props) {
  return <h1>Hello React!</h1>;
}

ReactDOM.render(<Hello />, document.getElementById("root"));
```

&emsp;这时候浏览器打开 `http://localhost:3000/` 就会输出：Hello React!

## React 安装

&emsp;









## 参考链接
**参考链接:🔗**
+ [React 官方文档](https://zh-hans.reactjs.org/docs/getting-started.html)
+ [React 教程](https://www.runoob.com/react/react-tutorial.html)

+ [入门教程: 认识 React](https://zh-hans.reactjs.org/tutorial/tutorial.html)

