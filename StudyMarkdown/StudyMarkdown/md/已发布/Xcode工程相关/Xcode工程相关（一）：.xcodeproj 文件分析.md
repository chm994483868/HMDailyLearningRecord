# Xcode工程相关（一）：project.pbxproj 文件分析

## .xcodeproj 文件夹概述

&emsp;.xcodeproj 文件（并不是文件其实是一个文件夹）我们大概再熟悉不过，每次创建一个 Xcode 的 App 项目，根目录下面默认就是一个 **项目名.xcodeprogj** 文件和一个 **项目名文件夹**，项目名文件夹里面是我们的初始几个文件：Assets.xcassets、Main.storybord、LaunchScreen.storyboard、Info.plist、.swift 文件，而 .xcodeproj 文件（内部的 project.pbxproj 文件）便是对整个项目工程信息以及项目内所有文件组织结构进行描述，它包含两个最重要的部分：项目内文件的引用和项目的 BuildSettings、BuildPhase 等。

&emsp;.xcodeproj 文件并不是一个文件，而是一个文件夹，而其内部最重要的文件便是：project.pbxproj 文件。默认情况下 .xcodeproj 文件夹内部还有一个 xcuserdata 文件夹和 project.xcworkspace 文件夹，它们内部没什么重要信息，暂时忽略，我们把目光主要集中在 project.pbxproj 文件中，合并代码时我们大概遇到过很多次 project.pbxproj 文件冲突，特别是需要手动处理时，当我们的项目大起来以后打开 project.pbxproj 看到其内部成千上万的行数差不多要当场裂开，乍一眼看上去它内部结构极其复杂，仔细看下的话也可以发现清晰的规律，每个区域的划分都遵循一个规则，苹果在每个区域加了类似 `/* Begin xxx section */ ... /* End xxx section */` 的注释说明供我们参考。

&emsp;project.pbxproj 文件本质是一个 ASCII text 文件，但是看下来觉得更像是一个类似 JSON 的结构。（The Xcode project file is an old-style plist (Next style) based on braces to delimit the hierarchy. 也被称为是一个旧式的 plist）

&emsp;使用 file 命令看到 project.pbxproj 文件是一个 ASCII text 文件：

```c++
xcodeprojDemo.xcodeproj % file project.pbxproj 
project.pbxproj: ASCII text
```

&emsp;附带使用 tree 命令我们看一下 .xcodeproj 文件夹的内部组织：

```javascript
xcodeprojDemo % file xcodeprojDemo.xcodeproj 
xcodeprojDemo.xcodeproj: directory // xcodeproj 是一个文件夹
xcodeprojDemo % cd xcodeprojDemo.xcodeproj 
xcodeprojDemo.xcodeproj % ls
project.pbxproj        project.xcworkspace    xcuserdata
xcodeprojDemo.xcodeproj % tree  
.
├── project.pbxproj
├── project.xcworkspace
│   ├── contents.xcworkspacedata
│   ├── xcshareddata
│   │   └── IDEWorkspaceChecks.plist
│   └── xcuserdata
│       └── hmc.xcuserdatad
│           └── UserInterfaceState.xcuserstate
└── xcuserdata
    └── hmc.xcuserdatad
        └── xcschemes
            └── xcschememanagement.plist

7 directories, 5 files
```

&emsp;.xcodeproj 文件夹内部 7 个文件夹，5 个文件，下面我们把目光都集中到 project.pbxproj 文件上，看下它内部包含的内容都代表了什么含义。

## project.pbxproj 文件

&emsp;project.pbxproj 文件是一个旧式的 plist（Next style），基于大括号来分隔层次结构。该文件以显式编码信息开头，通常是 UTF-8 编码信息。这意味着文件在开始时不得不带有 BOM（Byte Ordering Mark），否则解析将失败。

&emsp;project.pbxproj 文件更类似于 JSON 结构，重要的内容都包含在 objects 属性中，objects 嵌套其中，它其中的每个属性都由 24 位十六进制（96 位二进制位）表示形式的标识符作为唯一标识。此唯一标识符在整个文档中是唯一的。

&emsp;把 objects 中的内容摘出来后，project.pbxproj 文件内容如下，其中 rootObject 指向 PBXProject section。PBXProject 为根节点，代表着整个工程。PBXProject 中的 targets 属性是一个数组，可以包含多个 PBXNativeTarget，每个 PBXNativeTarget 代表着工程中的一个 target（这里需要理解 Project 和 Target 的包含关系），它维护着自己的需要编译的代码源文件（PBXSourcesBuildPhase）、storyboard 和 Assets.xcassets 等资源文件（PBXResourcesBuildPhase）以及依赖的库（PBXFrameworksBuildPhase，如：系统 framework、系统库、非系统 framework、非系统静态库）。




            buildPhases = (
                
                /* 指向 PBXSourcesBuildPhase section，对应 Build Phases 中的 Compile Sources，表示那些需要编译的代码源文件 */
                8E8A672E2863E745003DB257 /* Sources */,
                
                /* 指向 PBXFrameworksBuildPhase section，对应 Build Phases 中的 Link Binary With Libraries，表示引入的 framework 等库 */
                8E8A672F2863E745003DB257 /* Frameworks */,
                
                /* 指向 PBXResourcesBuildPhase section，对应 Build Phases 中的 Copy Bundle Resources，表示项目资源文件，例如：LaunchScreen 和 Main Storyboard 以及 Assets.xcassets 文件 */
                8E8A67302863E745003DB257 /* Resources */,
            );
            
            
            
            
```c++
// !$*UTF8*$!
{
    archiveVersion = 1;
    classes = {
    };
    objectVersion = 55;
    objects = {
        ......
    };
    rootObject = 8E8A672A2863E745003DB257 /* Project object */;
}
```

![截屏2022-06-30 09.25.16.png](https://p9-juejin.byteimg.com/tos-cn-i-k3u1fbpfcp/674134c4124b4e29ade7b8c0eed541ad~tplv-k3u1fbpfcp-watermark.image?)

&emsp;objects 中的内容被使用 `/* Begin xxx section */ ... /* End xxx section */` 格式的注释化作了一个一个区域，每个区域的几个元素加在一起表示工程的一些局部信息，下面我们一起来看看都有哪些内容。

&emsp;下面我们集中目光看 objects 中的内容。

### PBXBuildFile

&emsp;构建所需的代码源文件、资源文件、引入库等，平时 git 发生冲突也主要是在这个区域内冲突，每新建一个 .h/.m 文件，就会修改这个区域，各个 branch 都在创建的时候，容易冲突。

```c++
/* Begin PBXBuildFile section */
        8E8A67362863E745003DB257 /* AppDelegate.swift in Sources */ = {isa = PBXBuildFile; fileRef = 8E8A67352863E745003DB257 /* AppDelegate.swift */; };
        8E8A67382863E745003DB257 /* SceneDelegate.swift in Sources */ = {isa = PBXBuildFile; fileRef = 8E8A67372863E745003DB257 /* SceneDelegate.swift */; };
        8E8A673A2863E745003DB257 /* ViewController.swift in Sources */ = {isa = PBXBuildFile; fileRef = 8E8A67392863E745003DB257 /* ViewController.swift */; };
        8E8A673D2863E745003DB257 /* Main.storyboard in Resources */ = {isa = PBXBuildFile; fileRef = 8E8A673B2863E745003DB257 /* Main.storyboard */; };
        8E8A673F2863E746003DB257 /* Assets.xcassets in Resources */ = {isa = PBXBuildFile; fileRef = 8E8A673E2863E746003DB257 /* Assets.xcassets */; };
        8E8A67422863E746003DB257 /* LaunchScreen.storyboard in Resources */ = {isa = PBXBuildFile; fileRef = 8E8A67402863E746003DB257 /* LaunchScreen.storyboard */; };
/* End PBXBuildFile section */
```

### PBXFileReference

&emsp;记录了每个代码文件的文件类型、路径 path、sourceTree，不论引入文件的时候是 create group 还是 create reference，都会在这里添加一条记录。

```c++
/* Begin PBXFileReference section */
        8E8A67322863E745003DB257 /* xcodeprojDemo.app */ = {isa = PBXFileReference; explicitFileType = wrapper.application; includeInIndex = 0; path = xcodeprojDemo.app; sourceTree = BUILT_PRODUCTS_DIR; };
        8E8A67352863E745003DB257 /* AppDelegate.swift */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.swift; path = AppDelegate.swift; sourceTree = "<group>"; };
        8E8A67372863E745003DB257 /* SceneDelegate.swift */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.swift; path = SceneDelegate.swift; sourceTree = "<group>"; };
        8E8A67392863E745003DB257 /* ViewController.swift */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.swift; path = ViewController.swift; sourceTree = "<group>"; };
        8E8A673C2863E745003DB257 /* Base */ = {isa = PBXFileReference; lastKnownFileType = file.storyboard; name = Base; path = Base.lproj/Main.storyboard; sourceTree = "<group>"; };
        8E8A673E2863E746003DB257 /* Assets.xcassets */ = {isa = PBXFileReference; lastKnownFileType = folder.assetcatalog; path = Assets.xcassets; sourceTree = "<group>"; };
        8E8A67412863E746003DB257 /* Base */ = {isa = PBXFileReference; lastKnownFileType = file.storyboard; name = Base; path = Base.lproj/LaunchScreen.storyboard; sourceTree = "<group>"; };
        8E8A67432863E746003DB257 /* Info.plist */ = {isa = PBXFileReference; lastKnownFileType = text.plist.xml; path = Info.plist; sourceTree = "<group>"; };
/* End PBXFileReference section */
```

### PBXFrameworksBuildPhase

&emsp;在这个 section 中配置的是工程依赖的系统 framework、系统库、非系统 framework、非系统静态库，对应 Build Phases 中的 `Link Binary With Libraries`。可以在工程配置 Build Phases 的 Link Binary With Libraries 中配置。

```c++
/* Begin PBXFrameworksBuildPhase section */
        8E8A672F2863E745003DB257 /* Frameworks */ = {
            isa = PBXFrameworksBuildPhase;
            buildActionMask = 2147483647;
            files = (
            );
            runOnlyForDeploymentPostprocessing = 0;
        };
/* End PBXFrameworksBuildPhase section */
```

### PBXGroup

&emsp;工程中所有文件的 group 信息，这个和 xcode 文件目录是对应的，每一层的文件目录有唯一的 UUID，同一层 group 下的子 group 会和上一层的 group 的 UUID 有很高的重合度(基本只有 1-2 位不同)，这个 PBXGroup section 中，子 group 没有用树的方式，而是采用类似列表的方式呈现了所有的 group 目录，可以脑补：打开 xcode 左侧目录，然后让所有目录和文件 "左对齐"，然后就会生成如下的结构。

```c++
/* Begin PBXGroup section */
        8E8A67292863E745003DB257 = {
            isa = PBXGroup;
            children = (
                8E8A67342863E745003DB257 /* xcodeprojDemo */,
                8E8A67332863E745003DB257 /* Products */,
            );
            sourceTree = "<group>";
        };
        8E8A67332863E745003DB257 /* Products */ = {
            isa = PBXGroup;
            children = (
                8E8A67322863E745003DB257 /* xcodeprojDemo.app */,
            );
            name = Products;
            sourceTree = "<group>";
        };
        8E8A67342863E745003DB257 /* xcodeprojDemo */ = {
            isa = PBXGroup;
            children = (
                8E8A67352863E745003DB257 /* AppDelegate.swift */,
                8E8A67372863E745003DB257 /* SceneDelegate.swift */,
                8E8A67392863E745003DB257 /* ViewController.swift */,
                8E8A673B2863E745003DB257 /* Main.storyboard */,
                8E8A673E2863E746003DB257 /* Assets.xcassets */,
                8E8A67402863E746003DB257 /* LaunchScreen.storyboard */,
                8E8A67432863E746003DB257 /* Info.plist */,
            );
            path = xcodeprojDemo;
            sourceTree = "<group>";
        };
/* End PBXGroup section */
```

### PBXNativeTarget

&emsp;每个 Target 的 Build Settings 和 Build Phases（Sources/Frameworks/Resources 等）的信息。

```c++
/* Begin PBXNativeTarget section */
        8E8A67312863E745003DB257 /* xcodeprojDemo */ = {
            isa = PBXNativeTarget;
            
            /* buildConfigurationList 属性指向了 Target 的 Configurations 列表，默认的：Debug 和 Release */
            buildConfigurationList = 8E8A67462863E746003DB257 /* Build configuration list for PBXNativeTarget "xcodeprojDemo" */;
            
            buildPhases = (
                
                /* 指向 PBXSourcesBuildPhase section，对应 Build Phases 中的 Compile Sources，表示那些需要编译的代码源文件 */
                8E8A672E2863E745003DB257 /* Sources */,
                
                /* 指向 PBXFrameworksBuildPhase section，对应 Build Phases 中的 Link Binary With Libraries，表示引入的系统 framework、系统库、非系统 framework、非系统静态库 */
                8E8A672F2863E745003DB257 /* Frameworks */,
                
                /* 指向 PBXResourcesBuildPhase section，对应 Build Phases 中的 Copy Bundle Resources，表示 Target 使用的资源文件，例如：LaunchScreen 和 Main Storyboard、Assets.xcassets 文件、图片、音频、视频文件等 */
                8E8A67302863E745003DB257 /* Resources */,
            );
            buildRules = (
            );
            dependencies = (
            );
            name = xcodeprojDemo;
            productName = xcodeprojDemo;
            productReference = 8E8A67322863E745003DB257 /* xcodeprojDemo.app */;
            productType = "com.apple.product-type.application";
        };
/* End PBXNativeTarget section */
```

### PBXProject

&emsp;整个项目工程 Project 的信息，包括项目路径、Config 信息，相关版本号，所有的 Target 等信息。

```c++
/* Begin PBXProject section */
        8E8A672A2863E745003DB257 /* Project object */ = {
            isa = PBXProject;
            attributes = {
                BuildIndependentTargetsInParallel = 1;
                LastSwiftUpdateCheck = 1310;
                LastUpgradeCheck = 1310;
                TargetAttributes = {
                    8E8A67312863E745003DB257 = {
                        CreatedOnToolsVersion = 13.1;
                    };
                };
            };
            
            /* buildConfigurationList 属性指向了 Project 的 Configuration 列表，默认的：Debug 和 Release */
            buildConfigurationList = 8E8A672D2863E745003DB257 /* Build configuration list for PBXProject "xcodeprojDemo" */;
            
            compatibilityVersion = "Xcode 13.0";
            developmentRegion = en;
            hasScannedForEncodings = 0;
            knownRegions = (
                en,
                Base,
            );
            mainGroup = 8E8A67292863E745003DB257;
            productRefGroup = 8E8A67332863E745003DB257 /* Products */;
            projectDirPath = "";
            projectRoot = "";
            
            /* 当前 Project 下的 target 列表，默认情况下只有一个和 Project 同名的 Target */
            targets = (
                8E8A67312863E745003DB257 /* xcodeprojDemo */,
            );
        };
/* End PBXProject section */
```

### PBXResourcesBuildPhase

&emsp;列举了项目中每个 Resources 的信息，对应 Build Phase 下的 `Copy Bundle Resources`，例如：LaunchScreen、Main Storyboaryd、Assets.xcassets 等资源文件。

```c++
/* Begin PBXResourcesBuildPhase section */
        8E8A67302863E745003DB257 /* Resources */ = {
            isa = PBXResourcesBuildPhase;
            buildActionMask = 2147483647;
            files = (
                8E8A67422863E746003DB257 /* LaunchScreen.storyboard in Resources */,
                8E8A673F2863E746003DB257 /* Assets.xcassets in Resources */,
                8E8A673D2863E745003DB257 /* Main.storyboard in Resources */,
            );
            runOnlyForDeploymentPostprocessing = 0;
        };
/* End PBXResourcesBuildPhase section */
```

### PBXSourcesBuildPhase

&emsp;对应 Xcode 中 Build Phases 的 Complie Sources 的代码文件。

```c++
/* Begin PBXSourcesBuildPhase section */
        8E8A672E2863E745003DB257 /* Sources */ = {
            isa = PBXSourcesBuildPhase;
            buildActionMask = 2147483647;
            files = (
                8E8A673A2863E745003DB257 /* ViewController.swift in Sources */,
                8E8A67362863E745003DB257 /* AppDelegate.swift in Sources */,
                8E8A67382863E745003DB257 /* SceneDelegate.swift in Sources */,
            );
            runOnlyForDeploymentPostprocessing = 0;
        };
/* End PBXSourcesBuildPhase section */
```

### PBXVariantGroup

&emsp;不同地区的资源文件的引用信息，如果你项目使用了国际化，相关的 xxx.string 就在这个 section 中。

```c++
/* Begin PBXVariantGroup section */
        8E8A673B2863E745003DB257 /* Main.storyboard */ = {
            isa = PBXVariantGroup;
            children = (
                8E8A673C2863E745003DB257 /* Base */,
            );
            name = Main.storyboard;
            sourceTree = "<group>";
        };
        8E8A67402863E746003DB257 /* LaunchScreen.storyboard */ = {
            isa = PBXVariantGroup;
            children = (
                8E8A67412863E746003DB257 /* Base */,
            );
            name = LaunchScreen.storyboard;
            sourceTree = "<group>";
        };
/* End PBXVariantGroup section */
```

### XCBuildConfiguration

&emsp;在不同的 Configuration 下对应 Xcode 中 Build Settings 中的配置信息，默认的是：Debug 和 Release 两个 Configuration。

```c++
/* Begin XCBuildConfiguration section */
        8E8A67442863E746003DB257 /* Debug */ = {
            isa = XCBuildConfiguration;
            buildSettings = {
                ALWAYS_SEARCH_USER_PATHS = NO;
                CLANG_ANALYZER_NONNULL = YES;
                CLANG_ANALYZER_NUMBER_OBJECT_CONVERSION = YES_AGGRESSIVE;
                CLANG_CXX_LANGUAGE_STANDARD = "gnu++17";
                CLANG_CXX_LIBRARY = "libc++";
                CLANG_ENABLE_MODULES = YES;
                CLANG_ENABLE_OBJC_ARC = YES;
                CLANG_ENABLE_OBJC_WEAK = YES;
                CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING = YES;
                CLANG_WARN_BOOL_CONVERSION = YES;
                CLANG_WARN_COMMA = YES;
                CLANG_WARN_CONSTANT_CONVERSION = YES;
                CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS = YES;
                CLANG_WARN_DIRECT_OBJC_ISA_USAGE = YES_ERROR;
                CLANG_WARN_DOCUMENTATION_COMMENTS = YES;
                CLANG_WARN_EMPTY_BODY = YES;
                CLANG_WARN_ENUM_CONVERSION = YES;
                CLANG_WARN_INFINITE_RECURSION = YES;
                CLANG_WARN_INT_CONVERSION = YES;
                CLANG_WARN_NON_LITERAL_NULL_CONVERSION = YES;
                CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF = YES;
                CLANG_WARN_OBJC_LITERAL_CONVERSION = YES;
                CLANG_WARN_OBJC_ROOT_CLASS = YES_ERROR;
                CLANG_WARN_QUOTED_INCLUDE_IN_FRAMEWORK_HEADER = YES;
                CLANG_WARN_RANGE_LOOP_ANALYSIS = YES;
                CLANG_WARN_STRICT_PROTOTYPES = YES;
                CLANG_WARN_SUSPICIOUS_MOVE = YES;
                CLANG_WARN_UNGUARDED_AVAILABILITY = YES_AGGRESSIVE;
                CLANG_WARN_UNREACHABLE_CODE = YES;
                CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;
                COPY_PHASE_STRIP = NO;
                DEBUG_INFORMATION_FORMAT = dwarf;
                ENABLE_STRICT_OBJC_MSGSEND = YES;
                ENABLE_TESTABILITY = YES;
                GCC_C_LANGUAGE_STANDARD = gnu11;
                GCC_DYNAMIC_NO_PIC = NO;
                GCC_NO_COMMON_BLOCKS = YES;
                GCC_OPTIMIZATION_LEVEL = 0;
                GCC_PREPROCESSOR_DEFINITIONS = (
                    "DEBUG=1",
                    "$(inherited)",
                );
                GCC_WARN_64_TO_32_BIT_CONVERSION = YES;
                GCC_WARN_ABOUT_RETURN_TYPE = YES_ERROR;
                GCC_WARN_UNDECLARED_SELECTOR = YES;
                GCC_WARN_UNINITIALIZED_AUTOS = YES_AGGRESSIVE;
                GCC_WARN_UNUSED_FUNCTION = YES;
                GCC_WARN_UNUSED_VARIABLE = YES;
                IPHONEOS_DEPLOYMENT_TARGET = 15.0;
                MTL_ENABLE_DEBUG_INFO = INCLUDE_SOURCE;
                MTL_FAST_MATH = YES;
                ONLY_ACTIVE_ARCH = YES;
                SDKROOT = iphoneos;
                SWIFT_ACTIVE_COMPILATION_CONDITIONS = DEBUG;
                SWIFT_OPTIMIZATION_LEVEL = "-Onone";
            };
            name = Debug;
        };
        8E8A67452863E746003DB257 /* Release */ = {
            isa = XCBuildConfiguration;
            buildSettings = {
                ALWAYS_SEARCH_USER_PATHS = NO;
                CLANG_ANALYZER_NONNULL = YES;
                CLANG_ANALYZER_NUMBER_OBJECT_CONVERSION = YES_AGGRESSIVE;
                CLANG_CXX_LANGUAGE_STANDARD = "gnu++17";
                CLANG_CXX_LIBRARY = "libc++";
                CLANG_ENABLE_MODULES = YES;
                CLANG_ENABLE_OBJC_ARC = YES;
                CLANG_ENABLE_OBJC_WEAK = YES;
                CLANG_WARN_BLOCK_CAPTURE_AUTORELEASING = YES;
                CLANG_WARN_BOOL_CONVERSION = YES;
                CLANG_WARN_COMMA = YES;
                CLANG_WARN_CONSTANT_CONVERSION = YES;
                CLANG_WARN_DEPRECATED_OBJC_IMPLEMENTATIONS = YES;
                CLANG_WARN_DIRECT_OBJC_ISA_USAGE = YES_ERROR;
                CLANG_WARN_DOCUMENTATION_COMMENTS = YES;
                CLANG_WARN_EMPTY_BODY = YES;
                CLANG_WARN_ENUM_CONVERSION = YES;
                CLANG_WARN_INFINITE_RECURSION = YES;
                CLANG_WARN_INT_CONVERSION = YES;
                CLANG_WARN_NON_LITERAL_NULL_CONVERSION = YES;
                CLANG_WARN_OBJC_IMPLICIT_RETAIN_SELF = YES;
                CLANG_WARN_OBJC_LITERAL_CONVERSION = YES;
                CLANG_WARN_OBJC_ROOT_CLASS = YES_ERROR;
                CLANG_WARN_QUOTED_INCLUDE_IN_FRAMEWORK_HEADER = YES;
                CLANG_WARN_RANGE_LOOP_ANALYSIS = YES;
                CLANG_WARN_STRICT_PROTOTYPES = YES;
                CLANG_WARN_SUSPICIOUS_MOVE = YES;
                CLANG_WARN_UNGUARDED_AVAILABILITY = YES_AGGRESSIVE;
                CLANG_WARN_UNREACHABLE_CODE = YES;
                CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;
                COPY_PHASE_STRIP = NO;
                DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";
                ENABLE_NS_ASSERTIONS = NO;
                ENABLE_STRICT_OBJC_MSGSEND = YES;
                GCC_C_LANGUAGE_STANDARD = gnu11;
                GCC_NO_COMMON_BLOCKS = YES;
                GCC_WARN_64_TO_32_BIT_CONVERSION = YES;
                GCC_WARN_ABOUT_RETURN_TYPE = YES_ERROR;
                GCC_WARN_UNDECLARED_SELECTOR = YES;
                GCC_WARN_UNINITIALIZED_AUTOS = YES_AGGRESSIVE;
                GCC_WARN_UNUSED_FUNCTION = YES;
                GCC_WARN_UNUSED_VARIABLE = YES;
                IPHONEOS_DEPLOYMENT_TARGET = 15.0;
                MTL_ENABLE_DEBUG_INFO = NO;
                MTL_FAST_MATH = YES;
                SDKROOT = iphoneos;
                SWIFT_COMPILATION_MODE = wholemodule;
                SWIFT_OPTIMIZATION_LEVEL = "-O";
                VALIDATE_PRODUCT = YES;
            };
            name = Release;
        };
        8E8A67472863E746003DB257 /* Debug */ = {
            isa = XCBuildConfiguration;
            buildSettings = {
                ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon;
                ASSETCATALOG_COMPILER_GLOBAL_ACCENT_COLOR_NAME = AccentColor;
                CODE_SIGN_STYLE = Automatic;
                CURRENT_PROJECT_VERSION = 1;
                GENERATE_INFOPLIST_FILE = YES;
                INFOPLIST_FILE = xcodeprojDemo/Info.plist;
                INFOPLIST_KEY_UIApplicationSupportsIndirectInputEvents = YES;
                INFOPLIST_KEY_UILaunchStoryboardName = LaunchScreen;
                INFOPLIST_KEY_UIMainStoryboardFile = Main;
                INFOPLIST_KEY_UISupportedInterfaceOrientations_iPad = "UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight";
                INFOPLIST_KEY_UISupportedInterfaceOrientations_iPhone = "UIInterfaceOrientationPortrait UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight";
                LD_RUNPATH_SEARCH_PATHS = (
                    "$(inherited)",
                    "@executable_path/Frameworks",
                );
                MARKETING_VERSION = 1.0;
                PRODUCT_BUNDLE_IDENTIFIER = com.chm.xcodeprojDemo;
                PRODUCT_NAME = "$(TARGET_NAME)";
                SWIFT_EMIT_LOC_STRINGS = YES;
                SWIFT_VERSION = 5.0;
                TARGETED_DEVICE_FAMILY = "1,2";
            };
            name = Debug;
        };
        8E8A67482863E746003DB257 /* Release */ = {
            isa = XCBuildConfiguration;
            buildSettings = {
                ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon;
                ASSETCATALOG_COMPILER_GLOBAL_ACCENT_COLOR_NAME = AccentColor;
                CODE_SIGN_STYLE = Automatic;
                CURRENT_PROJECT_VERSION = 1;
                GENERATE_INFOPLIST_FILE = YES;
                INFOPLIST_FILE = xcodeprojDemo/Info.plist;
                INFOPLIST_KEY_UIApplicationSupportsIndirectInputEvents = YES;
                INFOPLIST_KEY_UILaunchStoryboardName = LaunchScreen;
                INFOPLIST_KEY_UIMainStoryboardFile = Main;
                INFOPLIST_KEY_UISupportedInterfaceOrientations_iPad = "UIInterfaceOrientationPortrait UIInterfaceOrientationPortraitUpsideDown UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight";
                INFOPLIST_KEY_UISupportedInterfaceOrientations_iPhone = "UIInterfaceOrientationPortrait UIInterfaceOrientationLandscapeLeft UIInterfaceOrientationLandscapeRight";
                LD_RUNPATH_SEARCH_PATHS = (
                    "$(inherited)",
                    "@executable_path/Frameworks",
                );
                MARKETING_VERSION = 1.0;
                PRODUCT_BUNDLE_IDENTIFIER = com.chm.xcodeprojDemo;
                PRODUCT_NAME = "$(TARGET_NAME)";
                SWIFT_EMIT_LOC_STRINGS = YES;
                SWIFT_VERSION = 5.0;
                TARGETED_DEVICE_FAMILY = "1,2";
            };
            name = Release;
        };
/* End XCBuildConfiguration section */
```

### XCConfigurationList

&emsp;XCBuildConfiguration 只是列举了所有 Target 的所有 Setting 项，下面这个文件区分，不同 Target 在 Debug 时使用哪个 Setting 项，在 Release 时使用哪个 Setting 项。

```c++
/* Begin XCConfigurationList section */
        8E8A672D2863E745003DB257 /* Build configuration list for PBXProject "xcodeprojDemo" */ = {
            isa = XCConfigurationList;
            buildConfigurations = (
                8E8A67442863E746003DB257 /* Debug */,
                8E8A67452863E746003DB257 /* Release */,
            );
            defaultConfigurationIsVisible = 0;
            defaultConfigurationName = Release;
        };
        8E8A67462863E746003DB257 /* Build configuration list for PBXNativeTarget "xcodeprojDemo" */ = {
            isa = XCConfigurationList;
            buildConfigurations = (
                8E8A67472863E746003DB257 /* Debug */,
                8E8A67482863E746003DB257 /* Release */,
            );
            defaultConfigurationIsVisible = 0;
            defaultConfigurationName = Release;
        };
/* End XCConfigurationList section */
```







## 参考链接
**参考链接:🔗**
+ [[iOS]XcodeProject的内部结构分析](https://www.jianshu.com/p/50cc564b58ce)
+ [iOS 开发 xcode中的project.pbxproj--深入剖析](https://blog.csdn.net/kuangdacaikuang/article/details/52987132)
+ [iOS 开发：深入理解 Xcode 工程结构（一）](https://blog.csdn.net/y4x5M0nivSrJaY3X92c/article/details/84851561?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1-84851561-blog-52987132.pc_relevant_aa&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7ECTRLIST%7Edefault-1-84851561-blog-52987132.pc_relevant_aa&utm_relevant_index=2)
+ [XCode工程文件结构及Xcodeproj框架的使用( 二 )](https://blog.csdn.net/ehyubewb/article/details/79954255?spm=1001.2101.3001.6650.1&utm_medium=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-1-79954255-blog-84851561.pc_relevant_aa&depth_1-utm_source=distribute.pc_relevant.none-task-blog-2%7Edefault%7EBlogCommendFromBaidu%7Edefault-1-79954255-blog-84851561.pc_relevant_aa&utm_relevant_index=2)

   
