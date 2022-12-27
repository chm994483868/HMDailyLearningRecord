# Flutter 学习过程中不容错过的项目进阶实践

&emsp;上一篇 Flutter 项目中我们没有使用任何状态管理框架，直接 setState 和 StatefulWidget 一把梭完成了整个项目。本项目呢我们也来学习 Flutter 的状态管理以及 Flutter 项目的分层。

&emsp;这里首先要感谢两位大佬：[用 SwiftUI 实现一个开源的 App Store](https://juejin.cn/post/7051512478630412301) 和 [season_zhu](https://juejin.cn/user/4353721778057997/posts) 和 [seasonZhu/GetXStudy](https://github.com/seasonZhu/GetXStudy)。

&emsp;本项目我是以 iAppStore-SwiftUI 为原型然后也直接使用里面的 Apple 的接口，然后参考着 GetXStudy 项目使用 GetX 为状态管理完成的。我大概给它起了一个：iAppStore-Flutter 的名字：[chm994483868/iAppStore_Flutter](https://github.com/chm994483868/iAppStore_Flutter)。比起前一个项目的 setState 和 StatefulWidget 一把梭，本项目我全部自己手打完成，内部几乎写满了注释，整个开发过程下来，对我而言也加深了很多对 Flutter 的理解，特别是状态管理以及 Widget 的 rebuild。这样一路下来自己的 Flutter 开发技术也算是大概上了一个台阶，继续进步，加油！

<figure class="half">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/acec99bfcdaa432f9b69e852f29a131b~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p1-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/17da5e4da42e45e8aacf955c4f0804bb~tplv-k3u1fbpfcp-watermark.image?">
</figure>

<figure class="half">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/97a40254562e49ac9d908771dfd85508~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/4ce6da21488948f69e0269bd8bbfc319~tplv-k3u1fbpfcp-watermark.image?">
</figure>

<figure class="half">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/e2446d45993a45baa4109c75c806cd60~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/171a9719e3c34671b22b36167e509064~tplv-k3u1fbpfcp-watermark.image?">
</figure>

<figure class="half">
    <img src="https://p6-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/833dc4279f76439da6d3a0f01b16041c~tplv-k3u1fbpfcp-watermark.image?">
    <img src="https://p3-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/dac02bad9ff04905833e1ff6fb025ea9~tplv-k3u1fbpfcp-watermark.image?">
</figure>

## FlutterJsonBeanFactory（Json 模型转换模块）

&emsp;Json 转 Model 使用了: [FlutterJsonBeanFactory​(Only Null Safety)​](https://plugins.jetbrains.com/plugin/11415-flutterjsonbeanfactory-only-null-safety-)，它是 Android Studio 的一个插件，对，除了这里，其他我是全程使用了 Visual Studio Code 开发。 

&emsp;在本项目中主要使用到了两个模型，一个是表示 App 详情信息的模型：app_detail_m_entity.dart，一个是表示 App 列表的数据模型：app_rank_m_entity.dart（app_rank_m_entity.dart 内部模型嵌套过多，看起来眼花缭乱，所以可以从 app_detail_m_entity.dart 内部看起，它内部比较精简），首先他们的内部都很规律，先是模型自己自定义的字段名，然后是它们所需要的 fromJson 和 toJson 函数分别指向了对应的 app_detail_m_entity.g.dart/app_rank_m_entity.g.dart 文件中自动生成的 fromJson 和 toJson 函数。

&emsp;FlutterJsonBeanFactory 插件会自动生成的对应的 app_detail_m_entity.g.dart 和 app_rank_m_entity.g.dart 文件，它们内部是自动生成的所有模型 class 与 json 互转时使用到的 fromJson 和 toJson 函数。

&emsp;看 app_detail_m_entity.g.dart 文件时我们注意到内部使用到了一个 JsonConvert 类，它也是 FlutterJsonBeanFactory 插件自动生成的，它统筹起了整个 Json 转模型的工作，首先它定义了一个 `static final Map<String, JsonConvertFunction> _convertFuncMap = {...};`，把项目中所有需要 Json 转换的模型类的 FromJson 函数收集起来方便后续直接读取使用，然后是 `T? asT<T extends Object?>(dynamic value) {...}` 函数完成所有基础类型直接转换以及如果是我们的自定义模型类型则从 `_convertFuncMap` 中读取对应的 fromJson 函数进行转换。

&emsp;这几乎就是 Json 转 model 的全部内容了，FlutterJsonBeanFactory 直接帮助我们省掉了枯燥的手写 fromJson 和 toJson 函数的全部过程，效率拉满！

&emsp;看完 FlutterJsonBeanFactory 相关的 Json 模型转换过程后，我们便可以看懂 `IEntity` 抽象类的作用了：**定义抽象泛型类 IEntity 作为 BaseEntity/BaseEntityiAppStore 的基类，为它们提供一个 generateOBJ 函数，完成 Json 数据到 T 的模型转换。**。

```dart
/// 定义抽象泛型类 IEntity 作为 BaseEntity/BaseEntityiAppStore 的基类，为它们提供一个 generateOBJ 函数，完成 Json 数据到 T 的模型转换。
abstract class IEntity<T> {
  T? generateOBJ<O>(Object? json) {
    if (json == null) {
      return null;
    }

    if (typeName(T) == 'String') {
      return json.toString() as T;
    } else if (typeName(T) == 'Map<dynamic, dynamic>') {
      return json as T;
    } else {
      /// List 类型数据由 fromJsonAsT 判断处理
      return JsonConvert.fromJsonAsT<T>(json);
    }
  }
}
```

&emsp;`IEntity` 为其子类 `BaseEntityiAppStore` 提供了一个 `generateOBJ` 函数，用于当网络请求数据返回后，把返回的 Json 数据转换为对应的 T 模型，而 `generateOBJ` 函数内部调用的便是 JsonConvert 的 `fromJsonAsT` 函数，而 `fromJsonAsT` 内部便是调用的 `jsonConvert.asT<M>(json)` 函数。

&emsp;到这里看懂了 Json 转模型的全过程，我们就可以去看网络请求模块了，此时便能看懂网络请求数据返回后，对数据的处理和转换过程了。

### 修改模型中自定义字段名

&emsp;FlutterJsonBeanFactory 使用起来超级方便，其中修改服务器返回的 json 字段名也很简单。例如：app_rank_m_entity.dart 文件中的 AppRankMFeedEntryIdAttributes 类，它的原始字段名服务器返回时会在其中加 ":" 号，这里使用 `@JSONField(name: "xxx")` 标注，修改之...，例如下面的示例，服务器返回了一个 `im:id` 的字段名，我们把它修改为 `imid`，方便我们使用。

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

### 修改模型中字段类型

&emsp;然后还有 AppRankMFeed 类中的 entry 字段，当时使用 json 数据转 model 时我直接全选了 json 文本，没注意到其中的 entry 字段不是 List 类型，而只是 `AppRankMFeedEntry?` 类型，后来在实际开发中请求接口才发现，这里 entry 的类型跟我们请求数据时传递的 limit 参数有关，如果 limit 参数的值大于 1 则 entry 返回 List 数据，如果等于 1 则仅返回一个 AppRankMFeedEntry 数据。

&emsp;通常情况下我们请求数据时 limit 参数肯定大于 1，所以此时我们直接把 entry 字段修改为 `List<AppRankMFeedEntry>?` 类型。

&emsp;到这里以后我们就要注意一下了，刚刚我们对生成的模型修改了两处，一个是修改已经生成的字段的名字，一个是修改已经生成的字段的类型。那么我们直接改了模型，那么模型对应的 g.dart 文件中的的 fromJson 函数就失效了，这里我们也不用担心，FlutterJsonBeanFactory 为我们提供了快捷的操作，当我们发现已生成的 model 需要修改时，我们不需要去复制修改原始的 json 文本重新生成 model，我们只需要修改我们已生成的 model 然后按下 option + J 快捷键，那么 model 对应的 generated/json 文件夹中的 xxx.g.dart 文件就会同步更新其中的 FromJson 函数，保证 json 数据转换模型的正常进行。

## dio（网络请求模块）

&emsp;看完了上面数据转模型的过程，然后便是和数据最紧密相关的网络请求模块了。

### BaseEntityiAppStore 泛型类承载网络请求结果

&emsp;首先是 BaseEntityiAppStore 泛型类中：

+ `T? data;`：保存返回的数据
+ `int? errorCode`：网络请求响应的 code
+ `String? errorMsg`：网络请求错误信息描述

&emsp;在本项目中 Apple 的数据接口只返回数据，不返回 code 之类的，所以这里的 `errorCode` 和 `errorMsg` 是我们自己添加的字段，在本项目中它们仅表示两种状态：

+ 当网络请求成功时 `errorCode` 的值是 0，`errorMsg` 值为空，`data` 是请求返回的数据转换为 T 类型。
+ 当网络请求以任何原因请求失败时 `errorCode` 的值是 -1，`errorMsg` 值为错误原因，`data` 为 null。

&emsp;大概还有第三种：

+ 当网络请求成功时 `errorCode` 的值是 0，`errorMsg` 值为空，`data` 因为返回的的数据为空或者异常导致模型转换失败，`data` 值为 null。

&emsp;然后是 `BaseEntityiAppStore.fromJson(Map<String, dynamic> json) {...}` 函数，是我们自己手动编写的，首先取出 json 数据中的 `errorCode` 和 `errorMsg`，然后如果有 `data` 数据的话，调用 `IEntity` 中的 `T? generateOBJ<O>(Object? json){...}` 函数，完成数据到模型 T 的转换。

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

&emsp;看完 `base_entity_iappstore.dart` 和 `i_entity.dart` 文件的内容后，我们对网络请求返回的数据向 T 模型转换的过程有一定的了解了。那么我们还有两个方向需要学习：如何使用它们呢？和网络数据如何请求呢？

&emsp;下面我们看：网络数据如何请求呢？本项目中网络数据的请求使用了 dio package，并对它进行了一个简单的封装。

### HttpUtils 类封装 dio 网络请求

&emsp;`http_util.dart` 文件是对 dio 的简单封装，其中主要封装了 get/post 请求。

```dart
abstract class HttpUtils {
  // 超时时间 1 min，dio 中是以毫秒计算的
  static const timeout = 60000000;

  /// 初始化方法私有化
  HttpUtils._();

  static final _dio = Dio(
    BaseOptions(
      baseUrl: Api.baseUrl,
      connectTimeout: timeout,
      receiveTimeout: timeout,
      headers: {},
    ),
  ).addPrettyPrint;

  static Options getCookieHeaderOptions() {
    // iAppStore 暂时没有 header
    String value = "";
    Options options = Options(headers: {HttpHeaders.cookieHeader: value});
    return options;
  }

  // get 请求
  static Future<Map<String, dynamic>> get({
    required String api,
    Map<String, dynamic> params = const {},
    Map<String, dynamic> headers = const {},
  }) async {
    Options options = getCookieHeaderOptions();
    options.headers?.addAll(headers);

    try {
      Response response = await _dio.get(api, queryParameters: params, options: options);
      if (response.data != null) {
        // ❌❌❌ 注意：itunes.apple.com 返回的数据是 String
        Map<String, dynamic> json;
        if (response.data.runtimeType == String) {
          json = convert.jsonDecode(response.data);
        } else {
          json = response.data;
        }
        return {
          Constant.errorCode: 0,
          Constant.errorMsg: "",
          Constant.data: json,
        };
      } else {
        // response.data 数据为 null，说明请求成功了，但是没有返回数据，那么这是什么情况呢？
        return {
          Constant.errorCode: 0,
          Constant.errorMsg: "",
          Constant.data: Null,
        };
      }
    } on DioError catch (e) {
      debugPrint("❌❌❌ post 请求发生错误: $e");

      return {
        Constant.errorCode: -1,
        Constant.errorMsg: e.toString(),
        Constant.data: Null,
      };
    }
  }

  // post 请求
  static Future<Map<String, dynamic>> post({
    required String api,
    Map<String, dynamic> params = const {},
    Map<String, dynamic> headers = const {},
  }) async {
    debugPrint("🌍🌍🌍 URL: $api");
    Options options = getCookieHeaderOptions();
    options.headers?.addAll(headers);
    try {
      Response response = await _dio.post(api, queryParameters: params, options: options);
      if (response.data != null) {
        // ❌❌❌ 注意：itunes.apple.com 返回的数据是 String
        Map<String, dynamic> json;
        if (response.data.runtimeType == String) {
          json = convert.jsonDecode(response.data);
        } else {
          json = response.data;
        }
        return {
          Constant.errorCode: 0,
          Constant.errorMsg: "",
          Constant.data: json,
        };
      } else {
        // response.data 数据为 null，说明请求成功了，但是没有返回数据，那么这是什么情况呢？
        return {
          Constant.errorCode: 0,
          Constant.errorMsg: "",
          Constant.data: Null,
        };
      }
    } on DioError catch (e) {
      debugPrint("❌❌❌ post 请求发生错误: $e");

      return {
        Constant.errorCode: -1,
        Constant.errorMsg: e.toString(),
        Constant.data: Null,
      };
    }
  }

  // request
  Future<Response<T>> request<T>(
    String api, {
    required HTTPMethod method,
    dynamic data,
    Map<String, dynamic>? queryParameters,
    Map<String, dynamic> headers = const {},
  }) async {
    Response response = await _dio.request(api,
        data: data,
        queryParameters: queryParameters,
        options: Options(headers: headers, method: method.string));
    return response.data;
  }
}

/// 延展 Dio，给它添加一个名为 addPrettyPrint 的 get，自定义 Dio log 输出
extension AddPrettyPrint on Dio {
  Dio get addPrettyPrint {
    interceptors.add(PrettyDioLogger(
      requestHeader: false,
      requestBody: true,
      responseBody: true,
      responseHeader: false,
      compact: false,
    ));

    return this;
  }
}

/// 定义 HTTP 请求方式的枚举
enum HTTPMethod {
  get("GET"),
  post("POST"),
  delete("DELETE"),
  put("PUT"),
  patch("PATCH"),
  head("HEAD");

  final String string;
  const HTTPMethod(this.string);
}

/// 延展 Response 给它添加一个名为 status 的 get，根据响应的 code，从 HttpStatus.mappingTable map 中取一个对应的枚举值
extension EnumStatus on Response {
  season.HttpStatus get status =>
      season.HttpStatus.mappingTable[statusCode] ?? season.HttpStatus.connectionError;
}
```

&emsp;其中的请求返回以后，不管成功与失败都返回：

```dart
        return {
          Constant.errorCode: 0,
          Constant.errorMsg: "",
          Constant.data: json,
        };
```

&emsp;形式的 Map，看着有点 low。看完 HttpUtils 类的内容也没有什么东西，很简单，就是 dio 的最基础用法。然后我们看下 request.dart 文件中对 HttpUtils 类的一个名为 Request 的扩展，外界所有的 get/pos 网络请求都是走的这里的 getiAppStore/postiAppStore 函数。

### HttpUtils 的延展：Request 的使用

```dart
/// 延展 HttpUtils 添加 get<T> 和 post<T> 函数
extension Request on HttpUtils {
  /// for iAppStore，iAppStore 和 GetXStudy 的接口数据结构完全不同，这里针对 iAppStore 单独再进行封装
  /// Get
  static Future<BaseEntityiAppStore<T>> getiAppStore<T>({required String api, Map<String, dynamic> params = const {}}) async {
    final data = await HttpUtils.get(api: api, params: params);
    final model = BaseEntityiAppStore<T>.fromJson(data);
    return model;
  }

  /// Post
  static Future<BaseEntityiAppStore<T>> postiAppStore<T>({required String api, Map<String, dynamic> params = const {}}) async {
    final data = await HttpUtils.post(api: api, params: params);
    final model = BaseEntityiAppStore<T>.fromJson(data);
    return model;
  }
}
```

&emsp;看到这里的网络请求，我们便可以和我们上面看的 BaseEntityiAppStore 泛型类联系起来了：`final model = BaseEntityiAppStore<T>.fromJson(data);` 直接把请求返回的 `data` 数据转换为 `BaseEntityiAppStore<T>`。这里我们先看一个使用实例，例如在 App 详情页面，我们需要根据 AppID 和当前 App 所处的区域 ID 请求 App 的详细信息，这里要发起一个网络请求，此时我们便可以这样：

```dart
import 'package:iappstore_flutter/base/interface.dart';
import 'package:iappstore_flutter/entity/app_detail_m_entity.dart';
import 'package:iappstore_flutter/entity/base_entity_iappstore.dart';
import 'package:iappstore_flutter/http_util/request.dart' as http;

class DetailRepository extends IRepository {
  Future<BaseEntityiAppStore<AppDetailMEntity>> appDetailData({required String appID, required String regionID}) => http.Request.postiAppStore(api: "$regionID/lookup?id=$appID");
}
```

&emsp;关于 DetailRepository 这个类名，等后面我们讲项目分层的时候再来细看。这里我们主要把目光聚焦在 `appDetailData` 函数上。`appDetailData` 函数带着 `appID` 和 `regionID` 两个参数发起一个 post 请求（注意这两个参数只需要拼接在 url 里面就可以了，例如一个完整的请求接口是：`https://itunes.apple.com/us/lookup?id=544007664`），然后请求成功后返回一个 `BaseEntityiAppStore<AppDetailMEntity>` 类的实例对象，其中 T 类型是 `AppDetailMEntity` 是我们之前老早就定义好的 App 详情信息的 model。

&emsp;至此，本项目中的网络请求、json 数据转模型我们就看完了。下面我们把目光扩大，聚在整个项目中，看下项目的架构。

## GetX 架构分层

&emsp;首先是整个项目每个页面（模块）对应四个文件（夹）：（这里我们以 rank_home 模块为例来分析）

+ binding：继承自 `abstract class Bindings {...}` 的子类，重载其 `void dependencies();` 函数，在其中添加的 `Get.lazyPut(...);`，例如在 `RankHomeBinding` 中，把 `RankHomeRepository` 和 `RankHomeController` 实例进行 `lazyPut`，保证在需要使用的地方可以直接 Get.find 找到。  

```dart
class RankHomeBinding extends Bindings {
  @override
  void dependencies() {
    Get.lazyPut(() => RankHomeRepository());
    Get.lazyPut(() => RankHomeController());

    Get.lazyPut(tag: RankHomeController.className, () => RefreshController(initialRefresh: true));
  }
}
```

+ controller：继承自 `abstract class GetxController extends DisposableInterface with ListenableMixin, ListNotifierMixin {...}` 的子类，这里主要放置页面所需要使用到的数据，以及各种操作逻辑。作用有点类似 MVVM 中的 VM。例如在 `RankHomeController` 中，所有的数据变量都放在其中，以及网络请求、下拉刷新、错误重试等逻辑。

```dart
class RankHomeController extends BaseRefreshControlleriAppStore<RankHomeRepository, AppRankMEntity> implements IClassName {
  // 实现 IClassName 抽象类中定义的 className
  static String? get className => (RankHomeController).toString();

  // 取得请求得到的 App 排行榜数据列表
  List<AppRankMFeedEntry> get dataSource => response?.data?.feed?.entry ?? [];

  // 导航栏中的标题，因为要动态更新，所以这里使用 RxString 类型
  final rankTitle = "排行榜".obs;
  // 导航栏底部的更新时间，因为要动态更新，同样使用 RxString 类型
  final updateTimeString = DateTime.now().toLocal().toString().obs;

  // 筛选页面默认三个筛选项：都用数据源中第一个筛选项：热门免费榜-所有 App-中国
  String rankName = Constant.rankingTypeLists.first;
  String categoryName = Constant.categoryTypeLists.first;
  String regionName = Constant.regionTypeLists.first;

  @override
  void onInit() {
    super.onInit();

    // 在 init 中直接 find 到 RankHomeBinding 中 dependencies 函数中添加的：Get.lazyPut(tag: RankHomeController.className, () => RefreshController(initialRefresh: true));
    refreshController = Get.find(tag: RankHomeController.className);
  }

  // 刷新函数直接调用 fetchRankData，传 false 不显示加载 loading
  @override
  Future<void> onRefresh() async {
    fetchRankData(false);
  }

  // rank_home 不需要加载更多，这里直接 loadComplete 完成，并 update
  @override
  Future<void> onLoadMore() async {
    refreshController.loadComplete();

    update();
  }

  // 根据当前的筛选类型搜索排行榜的数据
  void fetchRankData(bool isShowLoading) async {
    // 根据排行榜的名字，找到对应的排行榜的 ID
    final rankID = Constant.rankingTypeListIds[rankName] ?? "topFreeApplications";
    // 根据类型的名字，找到对应的类型的 ID
    final categoryID = Constant.categoryTypeListIds[categoryName] ?? "0";
    // 根据地区的名字，找到对应的地区的 ID
    final regionID = Constant.regionTypeListIds[regionName] ?? "cn";
    // 根据排行榜的 ID 找到对应的枚举类型
    final rankingType = RankingType.convert(rankID);

    // 根据入参判断是否需要展示 loading 动画
    if (isShowLoading == true) {
      status = ResponseStatus.loading;
      update();
    }

    // 请求排行榜的 App 数据列表
    response = await request.applications(url: rankingType.url(categoryID, regionID, 200));

    // 请求响应以后根据响应的状态更新 status 的值，此值决定了 rank_home 页面的显示内容：loading 页面、空页面、App 列表页面、请求失败的重试页面
    status = response?.responseStatus ?? ResponseStatus.successHasContent;

    // 如果响应成功后返回的 App 列表为空表示，数据为空
    if ((response?.data?.feed?.entry?.length ?? 0) <= 0) {
      status = ResponseStatus.successNoData;
    }
    
    // 根据响应的数据更新导航栏的标题
    rankTitle.value = (response?.data?.feed?.title?.label ?? "").split("：").last;
    // 更新更新的时间
    updateTimeString.value = DateTime.now().toLocal().toString();
    // 如果是下拉刷新的话，结束刷新动画
    refreshController.refreshCompleted();

    // 更新 RankHome 中 RefreshStatusView 中 contentBuilder 中的内容
    update();
  }

  // 重写 onRetry 函数，当网络请求失败时，点击重试按钮，重新请求数据
  @override
  void onRetry() {
    super.onRetry();

    debugPrint("🧶🧶🧶 ⛑⛑⛑ 重试被点击");
    fetchRankData(true);
  }
}
```

+ repository：存放页面需要使用到的各个网络请求。例如在 `RankHomeRepository` 中，把请求 App 排行榜的网络请求放在里面：

```dart
class RankHomeRepository extends IRepository {
  Future<BaseEntityiAppStore<AppRankMEntity>> applications({required String url}) => http.Request.postiAppStore(api: url);
}
```

+ view：继承自 `abstract class GetView<T> extends StatelessWidget {...}` 的子类。`GetView` 主要添加了 `tag` 和 `controlelr` 两个字段，`controller` 作为 `GetView` 的一个 get 函数：`T get controller => GetInstance().find<T>(tag: tag)!;`，可以在 `GetView` 的任何位置找到并使用 `controlelr`，例如在 `class RankHomePage extends GetView<RankHomeController> {...}` 中，`RankHomePage` 的 T 正是 `RankHomeController`，在上面的 `RankHomeController` 示例代码中我们已经看到其内部所有逻辑，而且在 `RankHomeBinding` 中 `Get.lazyPut(() => RankHomeController());` 这样也保证了 `GetInstance().find<RankHomeController>` 必定能找到已经 `put` 的 `RankHomeController` 使用。 

&emsp;同时在 `RankHomePage` 中，我们也把 `RefreshStatusView` 的 `contentBuilder` 控制在了涵盖范围最小，这样也保证了在 `RankHomeController` 中调用 `update` 函数进行 Widget 重建的性能消耗最小。

&emsp;这里可以认真研习一下 `RefreshStatusView` 的封装，由于时间原因，这里就不展开细说了。

&emsp;GetX 如果继续展开的话，还有很多内容要学习，由于时间原因，这里就不展开了，作为我下一个阶段的学习目标。当前仅涉及到 GetX 的基本使用。

&emsp;当前分析就先到这里把，所有的代码和注释都在：[chm994483868/iAppStore_Flutter](https://github.com/chm994483868/iAppStore_Flutter)，欢迎大家阅读并提出宝贵的修改意见。

## 参考链接
**参考链接:🔗**
+ [get 4.6.5](https://pub.flutter-io.cn/packages/get)
+ [FlutterJsonBeanFactory​(Only Null Safety)​](https://plugins.jetbrains.com/plugin/11415-flutterjsonbeanfactory-only-null-safety-)
+ [用 SwiftUI 实现一个开源的 App Store](https://juejin.cn/post/7051512478630412301)
+ [season_zhu](https://juejin.cn/user/4353721778057997/posts)
+ [seasonZhu/GetXStudy](https://github.com/seasonZhu/GetXStudy)
+ [FlutterJsonBeanFactory​(Only Null Safety)​](https://plugins.jetbrains.com/plugin/11415-flutterjsonbeanfactory-only-null-safety-)


