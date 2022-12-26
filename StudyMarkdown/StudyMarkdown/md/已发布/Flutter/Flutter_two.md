# Flutter 学习过程中不容错过的项目进阶实践

&emsp;

## dio

&emsp;然后是网络请求模块，首先是 BaseEntityiAppStore 泛型类中保存数据以及网络请求结果：

```dart
/// 继承自 IEntity 的泛型类
class BaseEntityiAppStore<T> extends IEntity<T> {
  T? data;

  int? errorCode;
  String? errorMsg;

  // 构造函数
  BaseEntityiAppStore(this.errorCode, this.errorMsg, this.data);

  // Map<String, dynamic> 转化为 BaseEntity
  BaseEntityiAppStore.fromJson(Map<String, dynamic> json) {
    errorCode = json[Constant.errorCode] as int?;
    errorMsg = json[Constant.errorMsg] as String?;
    if (json.containsKey(Constant.data)) {
      // generateOBJ 函数来自父类 IEntity
      data = generateOBJ<T>(json[Constant.data] as Object?);
    }
  }

  // 是否请求成功的的 get
  bool get isSuccess => errorCode == 0;
  // 请求响应状态的 get
  ResponseStatus get responseStatus => _responseStatus;

  // 请求响应状态的私有 get
  ResponseStatus get _responseStatus {
    if (errorCode == null) {
      // 正在请求中
      return ResponseStatus.loading;
    } else if (errorCode == 0) {
      // 其他情况的话，如果 data 不是 null 就是响应成功并且有数据，否则就是响应成功并且没数据
      if (data != null) {
        return ResponseStatus.successHasContent;
      } else {
        return ResponseStatus.successNoData;
      }
    } else {
      // 请求失败
      return ResponseStatus.fail;
    }
  }
}
```

&emsp;`Http_util` 文件夹中是对 dio 的封装，

## FlutterJsonBeanFactory

&emsp;Json 转 Model 使用了: [FlutterJsonBeanFactory​(Only Null Safety)​](https://plugins.jetbrains.com/plugin/11415-flutterjsonbeanfactory-only-null-safety-)，它是 Android Studio 的一个插件，对，除了这里，其他我是全程使用了 Visual Studio Code 开发。 

&emsp;FlutterJsonBeanFactory 使用起来超级方便，其中修改服务器返回的 json 字段名也很简单。例如：app_rank_m_entity.dart 文件中的 AppRankMFeedEntryIdAttributes 类，它对应的字段名服务器返回时会在其中加 ":" 号，这里使用 `@JSONField(name: "xxx")` 标注，修改之...

```dart
@JsonSerializable()
class AppRankMFeedEntryIdAttributes {
  @JSONField(name: "im:id")
  String? imid;
  @JSONField(name: "im:bundleId")
  String? imbundleId;
  ...
}
```

&emsp;然后还有 AppRankMFeed 类中的 entry 字段，当时使用 json 数据转 model 时我直接全选了 json 文本，没注意到其中的 entry 字段不是 List 类型，而只是 `AppRankMFeedEntry?` 类型，后来在实际开发中请求接口才发现，这里 entry 的类型跟我们请求数据时传递的 limit 参数有关，如果 limit 参数的值大于 1 则 entry 返回 List 数据，如果等于 1 则仅返回一个 AppRankMFeedEntry 数据。

&emsp;通常情况下我们请求数据时 limit 参数肯定大于 1，所以此时我们直接把 entry 字段修改为 `List<AppRankMFeedEntry>?` 类型。

&emsp;到这里以后我们就要注意一下了，刚刚我们对生成的模型修改了两处，一个是修改已经生成的字段的名字，一个是修改已经生成的字段的类型。FlutterJsonBeanFactory 为我们提供了快捷的操作，当我们发现生成的 model 需要修改时，我们不需要去复制修改原始的 json 文本重新生成 model，我们只需要修改我们已生成的 model 然后按下 option + J 快捷键，那么 model 对应的 generated/json 文件夹中的 xxx.g.dart 文件就会同步更新其中的 FromJson 函数，保证 json 数据转换的正常进行。



## 参考链接
**参考链接:🔗**
+ [get 4.6.5](https://pub.flutter-io.cn/packages/get)
+ [FlutterJsonBeanFactory​(Only Null Safety)​](https://plugins.jetbrains.com/plugin/11415-flutterjsonbeanfactory-only-null-safety-)
