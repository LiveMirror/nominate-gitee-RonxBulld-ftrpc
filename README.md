# ftrpc 简介
ftrpc 全名 Fast Tiny Remote Procedure Call，是我在创作另一个项目（Hyper App）的时候产生的附加项目。
因为苦于用各种各样的语言去编写前端以及后端的通信协议，以至于无法忍受每次 API 的修改都会导致重写大量代码。所以用了2天时间编写了基础功能，又用了2天的时间完善自定义结构体，最终形成了v1.0.0版的 ftrpc。
ftrpc 是真正的模块化运作，它只关心从 idl 中生成目标代码，而异于 Thrift 等自带通信组件的 RPC 框架。用户可以通过调用自己的 socket 或者 websocket 又或者别的什么组件去完成整个 RPC 动作。
ftrpc 是一个异步框架，这意味着你必须在调用的时候设置回调，并且在回调中处理返回的数据。

# 为什么不用 Thrift 或者 gRPC
为什么不用 Thrift ？因为 Thrift 是单向连接，只能 custom 主动询问 provide ，反过来却不行。
gRPC 则过于庞大，而且无法为我生成前端代码。

# 如何编译 ftrpc
ftrpc 使用 C++ 编写，并且指定了应用标准为C++17，低版本的 gcc 编译器建议升级到 7.3.0。
压缩包中也提供了 MinGW-w64 版的二进制文件以供 windows 用户使用。
源码文档中提供了 cmake 源文件，只需在合适的目录调用 cmake 指令然后 make 即可，例如：
mkdir build && cd build && cmake .. && make

# 如何使用 ftrpc
一套完整的 ftrpc 系统包含两个部分：ftrpc 执行文件和 template 模板目录。
ftrpc 在生成代码的时候将参考外部模板，将内部逻辑生成的代码片段插入到指定位置，最终输出为目标代码。
使用 ftrpc 首先需要编写 idl 文件，语法问题将在下一节阐述。然后调用程序去处理它，例如：
ftrpc -o c++,ts test.idl --no-version --builtin-json
如你所见，ftrpc 提供了丰富的参数帮助用户生成想要的代码。
输入 ftrpc -h 可以在终端打印出完整的帮助信息：
Usage: ftrpc [--help|-h] [--no-version|-n] [--output|-o [c++,python,js]] <IDL File>

--help|-h            Show this message.（打印本帮助信息）
--output|-o          Who will choose to generate.（选择目标生成语言，目前仅支持 C++ 和 typescript）
--no-version-n|-n    No version infomation in output.（输出的文件名中不包含版本信息）
--builtin-json|-j    Use built-in json module with C++.（使用内嵌的 jsoncpp 模块）

Created by Rexfield.
Allow with GPLv3.
BUG report: https://gitee.com/RonxBulld/ftrpc/issues

· 当前版本中仅能生成 C++ 的前后端以及 typescript 的前端；
· 程序默认在导出文件名中标注 ftrpc 协议版本号，例如：V2版本生成的 ftrpc.provider.cpp 将会被重命名为 ftrpc.provider.v2.cpp；
· 使用内嵌的 jsoncpp 模块将会在输出目录产生 json.cpp json/json.h json/json-forwards.h 文件，用户需要在工程中包含这些项目。

# ftrpc 语法结构
version = NUMBER;
module NAME:
{
    struct NAME:
    {
        TYPE NAME;
        ...
    };
    TYPE NAME(ARGS...);
};
首先在文件开头必须声明当前 IDL 的版本，这个版本是用户自己定义的而非 ftrpc 内部封装协议的版本号。
然后定义 module，module 会在 C++ 中生成类，在 typescript 中生成 module。
接下来，在 module 的内部用户可以任意的进行 struct 或者 api 的定义了。定义有些类似于 C 语言，但是要记住 struct NAME 后面的冒号，以及右花括号后面的分号。
基础类型目前仅支持 int float string bool，后续会提供更多的支持。