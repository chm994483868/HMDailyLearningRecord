# Xcode工程相关（一）：project.pbxproj 文件分析

&emsp;.xcodeproj 文件（并不是文件而是一个文件夹）我们大概再熟悉不过，每次创建一个 Xcode 的 App 项目，根目录下面就是一个 **项目名.xcodeprogj** 文件和一个 **项目名文件夹**，项目名文件夹里面是我们的初始几个文件：Assets.xcassets、Main.storybord、LaunchScreen.storyboard、Info.plist、.swift 文件，而 .xcodeproj 文件便是对整个项目工程信息以及项目内所有文件组织架构进行描述，它包含两个最重要的部分：项目内文件的引用和项目的 buildSettings。

&emsp;.xcodeproj 文件并不是一个文件，其实是一个文件夹，而其内部最重要的文件便是：project.pbxproj 文件，我们用 tree 命令看一下它的内部组织：

```javascript
xcodeprojDemo % file xcodeprojDemo.xcodeproj 
xcodeprojDemo.xcodeproj: directory
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

&emsp;








## 参考链接
**参考链接:🔗**
+ [[iOS]XcodeProject的内部结构分析](https://www.jianshu.com/p/50cc564b58ce)
+ [HTML 参考](https://developer.mozilla.org/zh-CN/docs/Web/HTML/Reference)
