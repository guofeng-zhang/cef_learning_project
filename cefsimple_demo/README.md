# cefsimple demo (VS Code)

一个最小 CEF 示例工程，依赖你**自己用 `build_cef.sh` 从 chromium 源码编出来的 CEF
发行包**（位于 `/home/test/zgf/my_cef/`），按 CEF 官方 `README.md`（Example 2）的
组织方式来写，在 VS Code 里可以直接编译 + 调试。

## 目录结构

```
zgf/
├── my_cef/                        <-- build_cef.sh 装配出的自建 CEF 发行包
│   ├── cmake/                     FindCEF.cmake + cef_variables + cef_macros
│   ├── include/                   CEF 公共头 + 生成头
│   ├── libcef_dll/                wrapper 源码 + 自动生成的 CMakeLists.txt
│   ├── Release/                   libcef.so / chrome-sandbox / libEGL.so ...
│   ├── Resources/                 *.pak / icudtl.dat / locales/
│   └── Debug -> Release           (FindCEF 需要 ${CEF_ROOT}/${CMAKE_BUILD_TYPE})
│
└── cefsimple_demo/
    ├── CMakeLists.txt             顶层：find_package(CEF) + add_subdirectory
    ├── cefsimple/
    │   ├── CMakeLists.txt         本应用 target（add_executable + libcef_lib）
    │   ├── browser_app.{cc,h}     只在 Browser 进程生效的 CefApp（传给 CefInitialize）
    │   ├── sub_process_app.{cc,h} 只在子进程生效的 CefApp（传给 CefExecuteProcess）
    │   ├── simple_handler.{cc,h}
    │   ├── cefsimple_linux.cc
    │   └── simple_handler_linux.cc
    ├── .vscode/
    │   ├── settings.json          CMake Tools + IntelliSense 默认项
    │   ├── tasks.json             configure / build / clean
    │   ├── launch.json            F5 gdb 调试
    │   └── c_cpp_properties.json  IntelliSense 兜底路径
    └── README.md
```

## 依赖

1. 先跑一遍 `build_cef.sh`（位于 `~/Downloads/`），它会：
   - 从 chromium 源码编出 `libcef.so` + `cefsimple` 的所有产物；
   - 把它们装配成标准 CEF 发行包布局，输出到 `/home/test/zgf/my_cef/`。
2. 系统工具：

   ```bash
   sudo apt install -y build-essential cmake ninja-build gdb
   ```

3. VS Code 扩展建议：`ms-vscode.cpptools`、`ms-vscode.cmake-tools`。

## 使用方式

### 方式 A：VS Code（推荐）

1. `File → Open Folder…` 打开 `/home/test/zgf/cefsimple_demo`。
2. `Ctrl+Shift+B` → 自动 configure + build `cefsimple`。
3. `F5` → gdb 启动，断点停在 `cefsimple/*.cc`。

### 方式 B：命令行

```bash
cd /home/test/zgf/cefsimple_demo
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target cefsimple -j
./build/cefsimple/Debug/cefsimple --no-sandbox
```

产物在 `build/cefsimple/Debug/`：`cefsimple` + `libcef.so`、`chrome-sandbox`、所有
`*.pak`、`icudtl.dat`、`locales/` 等由 CEF 自带的 `COPY_FILES` 宏自动从
`my_cef/Release/` 和 `my_cef/Resources/` 拷来。

## 切换到别的 CEF 发行包

顶层 `CMakeLists.txt` 默认用 `../my_cef`。要换成官方下载的或别的版本，通过
`-DCEF_ROOT` 或环境变量传：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
      -DCEF_ROOT=/path/to/cef_binary_146.0.11+...
```

## 学习观察：两个 CefApp 的进程归属

本 demo 被改造成**两个独立的 `CefApp` 子类**，分别演示
`CefExecuteProcess` 和 `CefInitialize` 的互斥职责：

| 类 | 文件 | 传给谁 | 实际生效的进程 |
|---|---|---|---|
| `BrowserApp` | `browser_app.{h,cc}` | `CefInitialize` | 仅 Browser（主）进程 |
| `SubProcessApp` | `sub_process_app.{h,cc}` | `CefExecuteProcess` | 仅 Renderer / GPU / Utility / Zygote |

运行时 stderr 会打印带前缀的观察日志，过滤一下就能看到：

```bash
./build/cefsimple/Debug/cefsimple --no-sandbox 2>&1 \
  | grep -E '^\[(main|BrowserApp|SubProcessApp)\]'
```

预期结论：

- `[BrowserApp]` 日志只出自 **一个** pid（Browser 主进程）。
- `[SubProcessApp]` 日志出自**多个** pid（每个子进程一个），`cmdline`
  里能看到 `--type=renderer` / `--type=gpu-process` / `--type=zygote`
  / `--type=utility` 区分身份。
- `OnBrowserCreated` / `OnContextCreated` 只在 **`--type=renderer`** 的
  子进程里出现；其它子进程里虽然 `SubProcessApp` 也被构造，但这两个
  回调不会触发。
- `[main] CefExecuteProcess returned ...`：Browser 收到 `-1`，子进程
  收到 `>=0`。

如果你看到 `[BrowserApp]` 日志出现在带 `--type=...` 的子进程里，或者
`[SubProcessApp] OnBrowserCreated` 出现在 Browser 进程 pid 下，说明对
两个函数的理解有偏差——去对照 `libcef/browser/main_runner.cc:186` 的
`if (!command_line.HasSwitch(kProcessType)) return -1;` 再读一遍。

## 常见问题

### GDB 单步进不去 libcef 内部

`build_cef.sh` 默认用 `symbol_level=0` 编译 CEF，`libcef.so` 是 stripped 的，
无法单步到 CEF 内部。如果要看 libcef 内部，把 `build_cef.sh` 里的
`symbol_level=0` 改成 `symbol_level=1` 或 `2` 重编即可（体积会涨几百 MB）。
应用层的 `browser_app.cc` / `sub_process_app.cc` / `simple_handler.cc`
断点不受影响。

### chrome-sandbox 权限

发行包里的 `chrome-sandbox` 要 setuid-root 才能启用沙盒。本地偷懒可以加
`--no-sandbox` 运行；要开沙盒：

```bash
cd build/cefsimple/Debug
sudo chown root:root chrome-sandbox
sudo chmod 4755 chrome-sandbox
./cefsimple
```

### IntelliSense 红波浪

CMake Tools 先 configure 一次生成 `build/compile_commands.json`，C/C++ 扩展就能
100% 精确。`c_cpp_properties.json` 里的 `includePath` 只是 fallback。
