# CEF Client Integration & Native Bridge Learning Plan

我是一个长期从事 CEF 相关开发的工程师，主要工作方向是**在客户端嵌入 CEF**，构建稳定的桌面应用浏览器内核能力，并实现**前端与 Native 的高质量双向交互**。

这个仓库用于公开我的 CEF 学习与实践计划，重点不只是“会用 CEF”，而是逐步走向“能读懂源码、能定位问题、能做工程级优化”。

---

## About Me

- CEF 方向从业者，聚焦客户端嵌入式浏览器方案
- 关注 Web UI 在 Native 容器中的工程落地
- 关注前端与 C++/系统能力的桥接、性能与稳定性
- 目标是形成可复用的 CEF 客户端架构与问题排查方法论

---

## Why This Plan

在实际业务中，CEF 不只是“展示网页”，更核心的是：

- 作为客户端内核承载复杂前端业务
- 打通 JavaScript 与 Native 能力（文件、系统、窗口、业务服务）
- 保证跨进程模型下的可调试性、可维护性与可扩展性
- 在性能、包体、启动速度和稳定性之间做工程平衡

因此，这个计划围绕以下目标展开：

1. 建立 CEF 架构与源码认知
2. 掌握 Browser/Renderer 多进程与 IPC 机制
3. 打通 JS Bridge（前端 <-> Native）完整链路
4. 形成可复用的排障与调优流程

---

## Learning Focus

### 1) Client Embedding

- CEF 初始化与生命周期管理
- Browser 创建、窗口集成、进程启动参数
- 资源打包与运行时目录组织

### 2) Frontend-Native Interaction

- JavaScript -> Native 调用通道设计
- Native -> Frontend 回调与事件分发
- 消息协议定义、错误处理、超时与重试策略
- 安全边界与权限控制

### 3) Source-Level Understanding

- 从 CEF API 反查到 `libcef` 实现
- 向下追踪到 Chromium `content` 层
- 识别线程模型、进程上下文与关键调用链

### 4) Engineering Practices

- 增量编译与构建脚本优化
- 包体裁剪与资源管理
- 崩溃定位、日志与最小复现策略

---

## Current Milestones

- [x] 完成 CEF 构建脚本基础整理（面向调试与学习）
- [x] 建立第 1 周学习执行任务单
- [ ] 输出 `CefInitialize` 调用链追踪文档
- [ ] 输出 `CreateBrowser` 调用链追踪文档
- [ ] 完成第一版 JS Bridge 最小可运行示例

---

## Repository Structure

```text
.
├── CEF_SOURCE_LEARNING_PLAN.md       # 总体学习路线（从入门到深入）
├── WEEKLY_STUDY_TEMPLATE.md          # 每周学习打卡模板
├── API_CALLCHAIN_TEMPLATE.md         # API 调用链追踪模板
└── week-01-execution.md              # 第1周执行任务单
```

---

## Long-Term Goal

沉淀一套面向客户端业务的 CEF 开发方法：

- 能快速嵌入、稳定运行
- 能高效打通前端与 Native 交互
- 能在源码层定位复杂问题
- 能在项目中持续优化性能与可维护性

如果你也在做 CEF 客户端、桌面端 Web 容器或 JS/Native 桥接，欢迎交流。
