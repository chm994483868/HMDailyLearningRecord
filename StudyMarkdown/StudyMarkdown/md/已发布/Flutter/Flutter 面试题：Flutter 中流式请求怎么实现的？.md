# Flutter 面试题：Flutter 中流式网络请求怎么实现的？

&emap;面试被问到了这个问题，先解释一下 stream 概念。

&emsp;在 Flutter 中，Stream 是一种用于处理异步数据流的抽象概念，它表示了一系列异步事件的序列。它可以用来传递和处理连续的数据事件序列，可以是用户输入、网络请求、数据库查询、文件读取等等。

&emsp;Stream 的作用类似于一个管道，数据可以从源头通过流向各个接收者。源头是一个异步生成数据的方法或操作，而接收者可以通过订阅（监听）这个 Stream 来获取和处理数据。Stream 提供了一种非阻塞的方式来处理异步任务，可以在数据准备好时接收和处理，而不需要等待整个数据流程完成。

&emsp;然后简单描述个场景和 API 使用。

&emsp;下面我们看一个使用 Stream 的简单示例，我们有一个 `NumberCreator` 类，通过 `StreamController` 创建一个全新的 Stream，并且指定它异步传递的是 `int` 值，外界可以通过 `stream` 公共属性来订阅这个 Stream。

&emsp;这里我们直接使用一个每秒执行一次的定时器来模拟 Stream 向外传递数据。这个 Stream 便每秒推出一个新的整数。下面看一下外界怎么订阅这个 Stream。

```dart
  StreamSubscription<T> listen(void onData(T event)?, {Function? onError, void onDone()?, bool? cancelOnError});
```

&emsp;当 Stream 构建好后，我们可以使用 `listen` 函数来订阅这个 stream，给它一个 `onData` 函数监听新值，每当 stream 发出一个新值时，这个 `onData` 函数都会被调用并把新值传递过来。同时也可以给它一个 `onError` 函数监听错误，给它一个 `onDone` 监听 stream 完成数据发送。`cancelOnError` 表示发生错误时，是否取消这个 stream，默认值是 `false`。

```dart
import 'dart:async';

class NumberCreator {
  NumberCreator() {
    // 使用一个计时器来进行每秒递增
    Timer.periodic(const Duration(seconds: 1), (t) {
      // 将新数据添加到 stream 中
      _controller.add(_count);
      _count++;
    });
  }

  // 运行计数
  var _count = 1;

  // stream 控制器可以从头开始创建一个全新的 stream，int 表示这个 stream 传递的是 int 值 
  final _controller = StreamController<int>();

  // 控制器的 stream，一个公共属性以便其他对象可以订阅它
  Stream<int> get stream => _controller.stream;
}
```

&emsp;通过 listen 函数设置监听不同的事件。

```dart
void listenStream() async {
  final myStream = NumberCreator().stream;
  final subscription = myStream.listen(
        (data) => debugPrint('Data: $data'),
    onError: (err) {
      debugPrint('err: $err');
    },
    onDone: () {
      debugPrint('stream done!');
    },
  );
}
```

&emsp;看完上面的示例，我们便可简单明白 Stream 的工作模式：通过 listen 函数设置监听，通过 add 函数添加事件。可看出它依然是类似 Future 的 “监听-回调“ 的范畴，不同于 Future 的通过 Timer.run 异步执行 computation。在 Stream 上似乎我们没有看到异步的影子，实则不然，这里可以对比 iOS 中的通知模式：当在指定线程中用 post 发送通知时，然后会以同步的方式还在这个指定线程中执行通知的回调函数。而我们给 Stream 通过 listen 函数添加的监听回调则是依赖 Dart VM 的 event loop 来实现的，回调事件相对于 add 事件是异步的，回调事件通过 event loop 的微任务队列来进行分发。

&emsp;下面引出 Stream 在网络请求中的使用。

&emsp;在日常开发中，相比于使用 Future 进行单次的异步网络请求，我们几乎很难遇到需要使用流式网络请求的场景。但是既然面试官问到了这个问题，那么面试官所在的应该是一家做大模型的公司吧。此时想必大家都想到了，我们每次与大模型对话的场景，大模型恰恰就是流式输出，大模型思考着，预测着下一个词是什么，然后一行一行或者一段一段的给我们响应。

&emsp;这里我们可以以 ChatGPT 的 API 为例，简单描述一下向 GPT 发起流式网络请求的过程。在 Flutter 中网络请求框架可以直接使用 Dio。Dio 支持流式请求 API，首先我们需要在发起请求时指定响应类型是：ResponseType.stream，同时请求 body 中指定 stream 参数为 true，就是告诉 GPT 服务端我们需要流式响应，如果不指定的话，则是 json 响应。如下是一个简单的示例代码：

```dart
  static Future<Stream<T>> postFutureStream<T>({
    required String to,
    required T Function(Map<String, dynamic>) onSuccess,
    required Map<String, dynamic>? body,
  }) async {
  
    // 主要指定是流式请求
    Options options = getCookieHeaderDefaultOptions(isStream: true);
    options.headers?.addAll(HeadersBuilder.build());

    // StreamController 构建 Stream
    StreamController<T> controller = StreamController<T>();
    
    void close() {
      controller.close();
    }

    try {
      final response = await _dio.post(to,
          data: body != null ? convert.jsonEncode(body) : null,
          options: options);

      ResponseBody data = response.data;

      // ⚠️ stream Instance of '_ControllerStream<Uint8List>'
      Stream stream = data.stream;
      
      // 监听 
      stream.listen(
        (rawData) {
          String data = utf8
              .decode(rawData as List<int>, allowMalformed: true)
              .trimRight();
              // ...
              
          try {
            final decoded = jsonDecode(data) as Map<String, dynamic>;
            controller.add(onSuccess(decoded));
          } catch (e) {
            rethrow;
          }
          
        },
        onDone: () {
          OpenAILogger.log("✅ Stream onDone 被执行了！");

          close();
        },
        onError: (error, stackTrace) {
          OpenAILogger.log("✅ Stream onError 被执行了！");

          controller.addError(error, stackTrace);
        },
      );
      
    } on DioError catch (e) {
      rethrow;
    }

    return controller.stream;
  }
```

&emsp;使用 Dio 发起网络请求后，response.data.stream 正是一个 `_ControllerStream<Uint8List>`，它会一段一段返回字符串，类似如下：

```dart
// ...

data: {"choices":[{"delta":{"content":" How"},"finish_reason":null,"index":0}],"created":1715509104,"id":"chatcmpl-9O0Zcn2CcjKpwjKawtqgDXueHnqSp","model":"gpt-35-turbo-16k","object":"chat.completion.chunk","system_fingerprint":null}

data: {"choices":[{"delta":{"content":" can"},"finish_reason":null,"index":0}],"created":1715509104,"id":"chatcmpl-9O0Zcn2CcjKpwjKawtqgDXueHnqSp","model":"gpt-35-turbo-16k","object":"chat.completion.chunk","system_fingerprint":null}

data: {"choices":[{"delta":{"content":" I"},"finish_reason":null,"index":0}],"created":1715509104,"id":"chatcmpl-9O0Zcn2CcjKpwjKawtqgDXueHnqSp","model":"gpt-35-turbo-16k","object":"chat.completion.chunk","system_fingerprint":null}

data: {"choices":[{"delta":{"content":" assist"},"finish_reason":null,"index":0}],"created":1715509104,"id":"chatcmpl-9O0Zcn2CcjKpwjKawtqgDXueHnqSp","model":"gpt-35-turbo-16k","object":"chat.completion.chunk","system_fingerprint":null}

data: {"choices":[{"delta":{"content":" you"},"finish_reason":null,"index":0}],"created":1715509104,"id":"chatcmpl-9O0Zcn2CcjKpwjKawtqgDXueHnqSp","model":"gpt-35-turbo-16k","object":"chat.completion.chunk","system_fingerprint":null}

// ...
```

&emsp;然后进行解析，提取处其中的 content 内容拼接起来，如上：How can I assist you....，把这些字符串拼接起来，就构成了我们与 GPT 的一轮会话。

&emsp;简单描述完 Stream 的概念和使用流程后，下面又引出了两个点：

1. 上面提到的流式请求涉及到的 SSE 协议的知识点，它与 WebSocket 的不同点？
> SSE（Server-Send Events）
> SSE 是一种在基于浏览器的 Web 应用程序中仅从服务器向客户端发送文本消息的技术。SSE 基于 HTTP 协议中的持久连接， 具有由 W3C 标准化的网络协议和 EventSource 客户端接口，作为 HTML5 标准套件的一部分。请求 headers 附带：Content-Type: text/event-stream Cache-Control: no-cache Connection: keep-alive。

2. Stream 异步响应涉及到的 event loop 是怎么实现的?

&emsp;这题可以对比着 JavaScript 和 iOS 中的 runloop 来回答。

&emsp;在无准备的情况下，直接被问趴了...

## 参考链接
**参考链接:🔗**
+ [Dart Streams - Flutter in Focus](https://www.youtube.com/watch?v=nQBpOIHE4eE)
+ [Asynchronous programming: Streams](https://dart.dev/libraries/async/using-streams)
+ [Stream<T> class](https://api.dart.dev/stable/3.3.4/dart-async/Stream-class.html)
