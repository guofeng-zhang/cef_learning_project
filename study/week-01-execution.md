# CEF 第1周执行任务单（架构入门）

目标：建立 CEF 源码全局地图，明确进程模型，完成第一次“API 到实现”的最小追踪。  
建议投入：每天 1-1.5 小时，共 5 天。

---

## Day 1：搭建学习工作台 + 目录地图

### 今日任务

1. 确认源码目录可用：
   - `/home/test/code/chromium_git/chromium/src/cef`
   - `/home/test/code/chromium_git/chromium/src/content`
   - `/home/test/code/chromium_git/chromium/src/chrome`
2. 在笔记里写“目录职责草图”：
   - `cef` 做什么
   - `content` 做什么
   - `chrome` 做什么
3. 记录你的构建产物路径：
   - `/home/test/zgf/libcef.so`
   - `/home/test/zgf/cefsimple`

### 今日产出

- 一份 10-20 行的目录职责笔记。

---

## Day 2：理解 CEF 初始化入口

### 今日任务

1. 从 API 文档角度看 `CefInitialize`（先头文件语义）。
2. 在 `cef` 目录中搜索 `CefInitialize` 的实现入口。
3. 写第一张“调用链卡片”（可用 `API_CALLCHAIN_TEMPLATE.md`）：
   - 入口函数
   - 中间转发层
   - 最终落点（先不追很深）

### 今日产出

- `callchain-CefInitialize.md`（最少填写前 4 个章节）。

---

## Day 3：理解 Browser 创建路径

### 今日任务

1. 追踪 `CefBrowserHost::CreateBrowser`。
2. 重点识别：
   - Browser 进程侧入口
   - 与 `content` 的交界点
3. 记录线程/进程信息：
   - 在哪个线程调用
   - 是否触发跨进程行为

### 今日产出

- `callchain-CreateBrowser.md`（至少完成“主路径 + 线程/进程”章节）。

---

## Day 4：多进程模型最小认知

### 今日任务

1. 用你自己的话写清楚三件事：
   - Browser Process 负责什么
   - Renderer Process 负责什么
   - CEF 回调通常在哪一侧触发
2. 运行一次 `cefsimple`，观察并记录：
   - 启动流程关键现象
   - 页面加载时关键行为（导航、渲染）

### 今日产出

- 一张“CEF 进程职责简图”（文字版也可以）。

---

## Day 5：周复盘 + 问题清单

### 今日任务

1. 用 `WEEKLY_STUDY_TEMPLATE.md` 填本周总结：
   - 本周目标是否完成
   - 关键调用链是否跑通
   - 遇到的最大障碍是什么
2. 列出下周要攻克的 3 个问题：
   - 例如 IPC 机制、某回调线程归属、某模块入口定位

### 今日产出

- `week-01.md`（完整一版）
- 下周问题清单（3 条以上）

---

## 本周“合格线”

满足以下 4 条就算第 1 周完成：

- [ ] 你能解释 `cef/content/chrome` 三层职责
- [ ] 你完成 2 张 API 调用链卡片
- [ ] 你写出 Browser/Renderer 的基本分工
- [ ] 你完成一份周总结

---

## 执行建议（很重要）

- 不求一次追到底，先“广度优先”，建立导航能力。
- 每次追踪都记录“文件路径 + 函数名 + 线程/进程”。
- 看不懂的实现先标记，不要卡死在单点。

