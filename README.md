# 简单文件系统 —— 课程实验仓库

> 这是数据结构/操作系统类课程的实验项目：用 C 语言实现一个**支持多级目录结构的命令行文件系统**。
> 覆盖目录树（孩子-兄弟表示法）、哈希表（链地址法）、KMP 模糊查找、多模式遍历、递归删除、文件读写状态机等知识点。

## 仓库结构

```
ezfilesystem/
├── docx/                              老师原始文档（题目描述 + 实验指导用书，只读参考）
├── SPEC.md                           精简版实现规格（★ 建议最先读，含结构图与细节标注）
├── DESIGN.md                         模块划分与函数接口设计（实现时的接口约定）
├── AGENTS.md                         本仓库工作规范（Commit 规范 / README 维护 / 文档导航 / AI_version 开发模式）
├── README.md                         本文件
├── tests/                            官方样例 + 公共用例 cases/ + AI 专属用例 cases_ai/ + 测试运行器 run_tests.py
├── .github/workflows/ci.yml          CI：human_version 与 AI_version 双版本回归（AI 版额外跑 ASan）
├── run.sh / run.command / run.bat    ★ 一键运行（Linux·macOS 终端 / macOS 双击 / Windows 双击）
│
├── human_version/                    我自己实现的简化版（跑通 SPEC，strstr 替代 KMP，不查越界）
│   ├── include/                      头文件：hash.h（哈希表）/ fs.h（目录树）/ cmd.h（命令）
│   ├── src/                          实现：hash.c / fs.c / cmd.c / main.c
│   └── Makefile                      构建（产物 ezfs 落在本目录）
└── AI_version/                       升级版（基线=human 副本；按 Issue #1-5 逐项增强）
    ├── include/                      头文件：hash.h / fs.h / cmd.h / kmp.h（KMP 子串匹配）
    ├── src/                          实现：hash.c / fs.c / cmd.c / kmp.c / main.c
    └── Makefile                      构建（产物 ezfs 落在本目录）
```

### 源码位置速查

两个版本目录结构完全对应，差异只在同名文件内部（AI 版增强点见 `DESIGN.md §7`）：

| 想找什么 | 文件 | 两版差异 |
|---|---|---|
| 哈希表（链地址法） | `include/hash.h`、`src/hash.c` | 基本一致（AI 版去掉无用的 `CONTENT_SIZE`） |
| 目录树 / 文件实体（孩子-兄弟 + 两张哈希表） | `include/fs.h`、`src/fs.c` | AI 版 content 动态扩容、路径缓冲 1024、退出释放整棵树 |
| 15 条命令的业务实现 | `include/cmd.h`、`src/cmd.c` | AI 版加了名字超长校验、`close_file` 空指针保护、内容动态扩容 |
| 主循环 / 拆词 / 状态机拦截 / 命令分发 | `src/main.c` | human 版定长 `fgets`；AI 版 `getline` + 缺参校验 + 引号校验 |
| 模糊查找算法 | `src/fs.c` 的 `find_walk`（`strstr`） | AI 版换成 `include/kmp.h` + `src/kmp.c`（KMP `nextval`） |
| 编译配置 / 头文件依赖 | `Makefile` | 除源码清单（AI 多 `kmp.c`）外一致 |

## 一键运行（给老师 / 快速体验）

不想敲命令的话，直接运行下面的入口脚本，会给出菜单选择版本与运行方式（编译全自动）：

| 系统 | 怎么运行 |
|---|---|
| **macOS** | 双击 `run.command`（若弹出"无法打开"，右键 → 打开 → 仍然打开） |
| **Windows** | 双击 `run.bat`（需先装 [MinGW-w64](https://www.mingw-w64.org/) 并勾选加入 PATH） |
| **Linux / macOS 终端** | `./run.sh` |

菜单可选：

1. 选版本：`human 版` / `AI 版` / `对比两版输出`
2. 选方式：
   - **回放官方样例**（推荐）—— 输出与报告第 4 节截图一致
   - **交互式体验** —— 手动输入命令，Ctrl+D 退出
   - **运行全部测试用例** —— 自动比对期望输出（human 9 组 / AI 17 组）

只装了编译器也能跑：脚本优先用 `make`，没有 `make` 时退回直接 `gcc -Wall -g -o ezfs *.c`。

等价的手动命令（供参考）：

```bash
make -C human_version run          # 编译并回放官方样例（AI 版同理）
python3 tests/run_tests.py AI_version   # 跑全部用例，退出码 0=全过
```

## 开发流程约定

- 按阶段提交，**每个 commit 只做一件事、信息写详细**（做了什么 + 为什么），与下方「Commit 记录」一一对应。
- **每次 commit 必须同步更新下方「Commit 记录」**（先改 README 再提交），详细规范见 `AGENTS.md`。
- human_version 先跑通，再一起讨论 AI_version 的升级点，升级项记录在 DESIGN.md §7。

## Commit 记录

### 2026-09-10 — 代码整理 + 源码分目录 include/src（行为零变化，CI 全绿）

**做了什么：**
1. **代码整理**（两版，纯格式与等价替换）：统一缩进/花括号/注释风格与声明对齐，清掉行尾空白；修掉 `cmd_create_dir`、`dir_rename`、`file_rename`、`find_walk` 的整段错位缩进与 `cmd_find_file` 挤成一行的语句；无参函数 `()` → `(void)`；human 版硬编码的 `256` 统一成 `PATH_BUF_SIZE`；`malloc`+`memset` → `calloc`；AI 版删掉已无用的 `CONTENT_SIZE`；删 `human_version/.gitkeep`。
2. **源码分目录**（`git mv` 保留历史）：两版统一为 `include/*.h` + `src/*.c`，编译配置、测试运行器、一键运行脚本全部同步到新路径。
3. **构建增强**：两个 Makefile 改用 `wildcard src/*.c`、加 `-Iinclude`、加 `-MMD -MP` 头文件依赖（改 `.h` 会触发重编译）、加 `-std=c11 -Wall -Wextra`。
4. **测试运行器**：`tests/run_tests.py` 的 `--asan` 路径适配新布局；`make` 不可用时自动回退直接 `gcc`，Windows（MinGW 无 `make`）也能跑用例。
5. **一键运行脚本首次入库**（上一轮遗留）：`run.sh` / `run.bat` / `run.command`，含双版本编译、回放官方样例、交互式、跑全部用例、对比两版输出。
6. **README**：重写仓库结构树（补上早已迁移的 `docx/`），新增「源码位置速查」表。

**如何验证：**
- `run_tests.py human_version` 9/9 PASS；`AI_version` 17/17 PASS；`--asan AI_version` 17/17 PASS（无越界/泄漏）。
- **行为零变化**：整理前后各把 34 份用例输入喂给新旧二进制，stdout 逐字节 diff 完全一致。
- `-Wall -Wextra -std=c11` 下零告警；`make` 与「无 make 回退 gcc」两条编译路径均实测通过。
- `run.sh` 四条菜单路径手测：human 回放样例 / AI 跑全部用例 / 交互模式 / 对比两版输出。

**待确认 / 下一步：**
- human_version 按 `AGENTS.md §5` 属冻结版本，本次只做格式与等价替换、**未改任何行为**；两版差异仍等于 AI 版增强能力（KMP / 内存安全 / 输入安全 / 未定义行为 / 健壮性）。
- `REPORT-SUMMARY.md` 与 `docx/202532110113-*.docx(.bak)` 按约定不动，保持未跟踪。
- 尚未 push。

### 2026-09-08 — AI-05 健壮性

1. **平台差异坑（macOS vs Linux）**：AddressSanitizer 的 LeakSanitizer 在 macOS 上不支持（一启动就报错退出），Linux 才可用。本地 macOS 只做越界检测，泄漏检测交 CI。跨平台工具要区分环境。
2. **期望文件生成要按真实遍历顺序**：deep_path 的 ll_pre 期望先写"所有 Dir 再 File"是错的——第 50 层目录 d49 在 d48 的文件之后输出（walk_pre 先文件后子目录）。推导期望时要精确模拟递归顺序，测试失败后对照实际输出校准。
3. **human_version 不是 ASan 的验收对象**：CI 首次给 human 加 ASan 后，Linux 上 deep_delete 探出隐藏内存问题（macOS 不复现）。按约定 human 冻结不修，ASan 只对 AI_version 跑——两版本就不同成熟度，验收要各按其定位。

### 2026-09-08 — AI-05 健壮性增强（Issue #5）：ASan 进 CI + 深度路径/泄漏用例 + 性能冒烟

**做了什么：**
1. run_tests.py 新增 `--asan` 模式：gcc -fsanitize=address 编译 + ASAN_OPTIONS（Linux 下 detect_leaks=1，macOS 仅越界检测——LeakSanitizer 平台不支持）。
2. CI 扩为 4 步：human/AI 常规 + 双版本 ASan（Linux 上自动做泄漏检测）。
3. 新增 2 用例：deep_path（50 层目录：create/cd/find 深层路径/ll_pre 完整路径 104 行）、leak_check（创建文件/目录/写入 → 全删，配 ASan 验证无泄漏）。
4. tests/perf_smoke.py：手动性能冒烟（5000 文件创建 + find + ll + 全删，实测 6.9 万条/秒）。
5. SPEC §6 决策 +1 行。

**待确认 / 下一步：**
- 五个 Issue 全部完成。可选：代码规范化整理、实验报告素材、git 标签打版。

### 2026-09-08 — AI-04 补齐未定义行为（Issue #4，用户确认全部现状/推荐方案）

**做了什么：**
1. 修复必崩点：`cmd_close_file` 在从未 open 时 g_opend_file==NULL，读 `g_opend_file->name` 直接崩溃（human 版同样存在）。AI 版改为输出 `ERROR: invalid operation`。
2. 其余三项维持现状并记录决策：未知命令静默忽略；rename old==new 报 already exists；cd .. 根目录静默忽略（SPEC 已明确）。
3. 新增 1 个 AI 专属用例 undefined_behaviors：close 未打开×2、正常 open/close、old==new、未知命令、根目录 cd ..、收尾 ll_post 验证状态正常。
4. SPEC §6 决策表 +3 行。

**待确认 / 下一步：**
- Issue #5 健壮性增强（边界与对抗场景全面加固，候选清单待用户确认）。

### 2026-09-08 — AI-05 健壮性

1. **平台差异坑（macOS vs Linux）**：AddressSanitizer 的 LeakSanitizer 在 macOS 上不支持（一启动就报错退出），Linux 才可用。本地 macOS 只做越界检测，泄漏检测交 CI。跨平台工具要区分环境。
2. **期望文件生成要按真实遍历顺序**：deep_path 的 ll_pre 期望先写"所有 Dir 再 File"是错的——第 50 层目录 d49 在 d48 的文件之后输出（walk_pre 先文件后子目录）。推导期望时要精确模拟递归顺序，测试失败后对照实际输出校准。

### 2026-09-08 — AI-04 未定义行为

1. **状态机白名单 ≠ 输入校验**：main 的拦截只挡"已打开时的无关命令"；反过来"关闭状态下执行 close_file"会放行到 cmd_close_file——若从未 open，g_opend_file==NULL，`->name` 解引用直接段错误。凡函数内读写指针字段，先判 NULL。

### 2026-09-08 — AI-03 输入安全四项落地：getline 动态读取 + 缺参/引号校验（Issue #3）

**做了什么：**
1. Q1（getline）：main 改用 POSIX getline 动态读取，超长输入行不再截断；EOF 前 free(line)。
2. Q2（缺参）：need_args 表定义每条命令所需参数个数；缺必需参数统一输出 `ERROR: invalid operation`；未知命令返回 -1 保持静默（留给 Issue #4）。
3. Q3（引号校验）：write_file 要求内容被成对英文引号包裹——无左引号/无右引号（残缺）均报 `ERROR: invalid operation`；引号解析放在 strtok 拆词前（strtok 会破坏 line）。
4. Q4（多余参数）：忽略多余参数（如 `rename_dir a b extra` 正常执行）。
5. 新增 3 个 AI 专属用例：missing_args（4 种缺参 + 多余参数忽略）、bad_quotes（无引号/残缺/缺参）、long_content（单行 2000 字符经 getline + 动态扩容完整写入）。
6. SPEC §6 决策表 +4 行。

**待确认 / 下一步：**
- Issue #4 补齐未定义行为（未知命令策略等）。

### 2026-09-08 — AI-02 内存安全四项落地：动态扩容 / 名字拒绝 / 缓冲增强 / 深度实测

**做了什么：**
1. Q1（超长名字拒绝）：cmd 层 create_file/create_dir/rename_file/rename_dir 统一 `name_too_long` 检查（strlen ≥ NAME_SIZE），输出 `ERROR: name too long`。
2. Q2（内容动态扩容）：File.content 从固定数组改为动态分配（初始 64、倍增扩容）；新增 `file_free` 释放 content 与节点；cmd_write_file 改为 memcpy 安全追加（OOM 时输出 ERROR 且不破坏原内容）。
3. Q4（路径缓冲）：PATH_BUF_SIZE 移入 fs.h 并 256→1024，find_walk/ll_*/find_file 统一使用。
4. Q3（深度实测）：ASan 跑 800 层嵌套 create_dir + cd .. 后 delete_dir，无栈溢出、无内存错误，暂不加层数限制。
5. 新增 2 用例：name_too_long（超长拒绝 4 场景）、big_write（8×150=1200 字符动态扩容验证）。
6. SPEC §6 未定义行为决策表新增 4 行（对应 Q1-Q4 决策）。

**待确认 / 下一步：**
- Issue #3 输入安全（无参数命令、超长行、引号残缺、未知命令决策）。

### 2026-09-08 — AI-01 KMP：独立 kmp 模块替换 strstr（nextval 优化），20 万随机用例验证一致

**做了什么：**
1. 新增 `AI_version/kmp.h` / `kmp.c`（Issue #1，与用户确认 3 点：独立模块 / nextval 优化 / 空模式匹配一切）：
   - `kmp_nextval`：0-based nextval 数组（跳过与失配字符相同的自移动），KMP_MAX_PAT 与 fs.h NAME_SIZE 同步；
   - `kmp_match`：包含语义子串匹配；空模式返回 1；模式串长于文件名上限直接返回 0（next 数组固定大小不越界）；
   - 修复实现越界隐患：++i 后 i==m 时不再访问 pat[m]（AI 版内存安全自查）。
2. `fs.c` 的 find_walk 改用 `kmp_match`；Makefile OBJS 加 kmp.o。
3. 验证：AI_version 9 用例全绿；20 万组随机 (text,pat) 与 strstr 结果完全一致；ASan 官方样例无报错。

**待确认 / 下一步：**
- Issue #2 内存安全（write 越界截断策略、递归删除深度）；补充：命令行空关键词依赖 Issue #3 输入安全。

### 2026-09-08 — AI_version 基线建立：Issue 驱动模式启动（AGENTS.md + 双版本测试）

**做了什么：**
1. `AGENTS.md` 新增 §5「AI_version 开发模式」：Issue 驱动、代码只落 AI_version/、用户全程 review 不写码、Pi 写实现跑测试、细节必须先讨论。
2. 创建 5 个 GitHub Issue（#1 KMP / #2 内存安全 / #3 输入安全 / #4 未定义行为 / #5 健壮性）。
3. `AI_version/` 建立基线：从 human_version 复制全部源码（hash/fs/cmd/main + Makefile），之后只在此目录改进，human_version 冻结。
4. `tests/run_tests.py` 支持 `[版本目录]` 参数（默认 human_version）；CI 改为两个版本都跑。
5. 验证：run_tests.py 对 human_version 与 AI_version 均为 passed=9 failed=0。

**待确认 / 下一步：**
- Issue #1：KMP 替换 strstr（实现前与用户确认设计细节）。

### 2026-09-08 — 测试套件 + GitHub Actions CI：9 用例全绿，供 push 后过 CI

**做了什么：**
1. 新增 `tests/cases/`：9 组用例（输入 .in + 期望 .out）——
   - sample（官方样例）；conflict（同名共存/查重只查同类）；deep_delete（多层递归删除）；
   - state_machine（open 状态机拦截/未打开 write）；append（多次追加+引号空格）；
   - find（模糊查找顺序+无结果）；cd_root（根目录 cd .. 忽略）；
   - traversal（ll_pre/ll_post 顺序）；rename_order（重命名不改变创建顺序）。
2. 新增 `tests/run_tests.py`：make 编译 → 逐用例喂输入 → 归一化输出（去 `>> ` 前缀/空行）→ 与期望逐行 diff；退出码 0/1 供 CI 用。
3. 新增 `.github/workflows/ci.yml`：ubuntu-latest 上 checkout → `python3 tests/run_tests.py`。
4. 本地验证：passed=9 failed=0。

**待确认 / 下一步：**
- push 到 GitHub 看 CI 是否过；随后开始 AI_version 升级讨论。

### 2026-09-08 — human_version 完成：main.c 主循环 + Makefile，官方样例逐行一致

**做了什么：**
1. 新增 `main.c`：REPL 主循环——`>> ` 提示、fgets 读行去换行、strtok 拆词、open 状态机拦截（非 write/close 一律 invalid operation）、write_file 引号内容单独解析、15 条命令 if-else 分发、未知命令静默忽略。
2. 新增 `Makefile`：`make` 一键编译（-Wall -g）、`make run` 回放官方样例、`make clean`。
3. 新增 `tests/` 对照脚本思路：程序输出（去 `>> ` 前缀）与 `sample_transcript.txt`（去回显行）**24 行逐行完全一致**。
4. **human_version 至此功能完整**：hash（链地址哈希表）+ fs（目录树/遍历/递归删除/rename）+ cmd（15 命令）+ main（主循环）四模块闭环。

**待确认 / 下一步：**
- AI_version 讨论：KMP 接入、内存安全（越界/泄漏）、未定义行为补齐（close_file 未打开、未知命令）、测试自动化。

### 2026-09-08 — human_version：cmd 层 15 条命令实现并修复，核心链路跑通

**做了什么：**
1. 新增 `cmd.h`（15 个命令函数声明）与 `cmd.c`（实现）：
   - 检查-操作-输出三步：create/delete/rename 查表判重，cd 判 ".."；
   - open/write/close/read 状态机四件套；
   - find_file 两遍遍历（先统计再输出 SEARCH RESULTS）。
2. 新增 `fs.h`/`fs.c` 的 `dir_rename`/`file_rename`（只换哈希 key 不碰链表，保持创建顺序）与 `find_walk`（先根遍历+子串匹配）。
3. 修复 review 发现的 10 个问题（详见踩坑记录 cmd 模块）：
   - create_dir 用 strcpy 当比较/只建不挂/无输出；delete_dir 语义错位 + use-after-free；
   - cd 未处理 ".."、处理完未 return；rename_dir 消息 renamed/rename 拼错；rename_file 拆链重挂乱序；
   - find_walk 的 static 跨文件链接失败；全局变量只有 extern 声明无定义（链接错误）；
   - find_walk 统计趟误打印（SEARCH RESULTS 顺序反）→ 加 print 开关；ls 的 Dir 双空格；cd 错误消息不带名字。
4. 验证：三模块零警告；临时测试驱动（建树/open-write-close-read/rename/find/ls/ll_pre/空目录增删）输出与官方样例逐行一致。

**待确认 / 下一步：**
- 写 main.c（>> 提示、拆命令、状态机拦截、15 条分发），回放 tests/sample_input.txt。

### 2026-09-08 — human_version：修复 fs.c 全部问题（链表维护/清初始化/路径拼接）并通过测试

**做了什么：**
1. 修复 fs.c 的 7 处问题（详见「踩坑记录」fs 模块）：
   - `free(f;)` 语法错误；`snprintf(..., "...", ...)` 大小参数非法；
   - `dir_add_child` 补上漏掉的头插步骤 `d->firstchild_dir = child;`；
   - `dir_remove_child` / `file_remove` 补上摘链表逻辑（含头结点/中间节点），避免 dir_destroy 死循环与悬垂指针；
   - `dir_new` 改 calloc 整体清零（parent/链头不再有垃圾值）；
   - `walk_post` 目录路径重复拼接修复；strncpy 改 NAME_SIZE-1 + 手动补 '\0'。
2. 重写 walk_pre / walk_post 路径语义：path 统一为「末尾带 / 的目录链（根层为空）」——修掉测试暴露的斜杠错误（`/testDirtestFile4.txt` → `testDir/testFile4.txt`）。
3. 验证：gcc -Wall 零警告；临时测试程序（建目录树/遍历/rename 摘链重挂/递归删除）全部通过，路径与顺序符合 SPEC。

**待确认 / 下一步：**
- 写 cmd 层（命令处理/输出）与 main 循环，回放官方样例。

### 2026-09-08 — human_version：完成 hash 模块并通过测试

**做了什么：**
1. 新增 `human_version/hash.h`：常量（HASH_SIZE / NAME_SIZE / CONTENT_SIZE）+ HashNode / HashTable 结构体 + 5 个函数声明，含 include 防护。
2. 新增 `human_version/hash.c`：实现 `hash_key`（ΣASCII % 100）、`hash_insert`（链地址法头插）、`hash_find`、`hash_delete`（前驱指针摘链，支持头结点/中间节点）、`hash_destroy`（全桶释放）。
3. 验证：`gcc -Wall -c` 零警告；临时测试程序插/查/删/销毁全部通过。
4. 期间踩的 4 个坑已记入下方「踩坑记录」：数组不能赋值、结构体按值传参、复制粘贴残留参数、哈希表清零初始化。

**待确认 / 下一步：**
- 开始写 fs.h / fs.c（目录树 + 文件，核心模块）。

### 2026-09-08 — 新增 AGENTS.md：固化 Commit 规范与 README 维护规则

**做了什么：**
1. 新增 `AGENTS.md`：把本仓库的协作契约写成文档 ——
   - 文档导航与冲突裁决（SPEC 为唯一实现依据，docx 只读参考）；
   - **硬性规则：每次 commit 必须做两件事**——commit message 写详细（背景/做了什么/如何验证/待确认）+ 同步更新 README「Commit 记录」；
   - 开发流程约定（讨论先行、human 先跑通、每阶段一 commit）；
   - 实现时易错规格速查（双空格、同名共存、状态机等）。
2. README 同步更新：仓库结构加入 AGENTS.md，本条记录为本规则的首次执行。

### 2026-09-08 — 阅读课程文档，精简为可执行 Spec

**做了什么：**
1. 由 Pi 阅读老师提供的两份 docx 文档（题目描述 / 实验指导用书），逐条提取命令定义与输出格式，并对照原文核实了两处易错格式（`Dir` 后双空格、`SEARCH RESULTS:` 独立成行）。
2. 新增 `SPEC.md`：把冗长的老师文档精简为一页可读规格——
   - 用 4 张图表达：目录树示例、数据结构关系（目录节点 ↔ 双哈希表 ↔ 链表）、模块依赖、打开状态机；
   - 17 条命令收敛为 3 张表格；
   - 用表格标注 10 条从原文/样例中确认的细节（排序规则、重命名不换位、同名文件与目录可共存等）；
   - 明确列出题目**未定义行为**及建议决策，留作实现与升级期的讨论点。
3. 新增 `README.md`：说明项目为课程实验、仓库结构、提交约定，并开始维护本「Commit 记录」。
4. 其余基线：`DESIGN.md`（模块/接口设计）、`tests/`（官方样例输入与回显）、`.gitignore`（编译产物）。

**未解决 / 待确认：**
- `ll_post` 无官方样例，按标准后根遍历推导的输出顺序待 TA 确认；
- `close_file` 未打开、未知命令等未定义行为的最终决策。

## 踩坑记录（学习笔记）## 踩坑记录（学习笔记）

### 2026-09-08 — AI-03 输入安全

1. **strtok 会破坏输入行**：`strtok(line, " ")` 把分隔符替换成 '\0'，之后在 line 上找引号必然失败（strchr 到截断处就停）。修法：引号定位必须在拆词之前完成（先 `strchr` 后 `strtok`）。
2. **getline 需要显式 POSIX 支持**：Linux/glibc 下要 `#define _POSIX_C_SOURCE 200809L`（放文件首部），否则 gcc 可能不暴露 getline 声明。
3. **替代文本前缀陷阱**：README 编辑时锚点"### AI-02 内存安全"同时命中 Commit 记录标题与踩坑记录标题，导致小节误插到 Commit 区。修法：用带完整副标题的锚点 + 复核文件结构。



### 2026-09-08 — 测试架构（AI 专属用例分区）

1. **AI 专属能力不能进公共用例集**：name_too_long/big_write 放入公共 cases/ 后 human_version 回归 2 个 FAIL（human 版本就无这些能力）。修法：AI 专属用例移入 tests/cases_ai/，run_tests.py 按 TARGET 决定是否加载——两版共享公共集，AI 版额外验收增强能力。

### 2026-09-08 — 测试套件

1. **期望文件要覆盖程序全生命周期**：写 `.out` 时漏了创建命令前面的 SUCCESS 行，只写了 find/traversal 目标段——期望文件必须包含从启动到结束的**完整**输出序列（含所有中间 SUCCESS），否则 diff 第 0 行就错位。
2. **归一化要去掉连续提示符**：cd 无输出时可能出现 `>> >> SUCCESS...`，处理 `>> ` 前缀要循环剥除；EOF 前会多一个空 `>> `，要过滤空行。
3. **.gitignore 会吞掉期望文件**：`*.out` 同时匹配编译产物与测试期望文件，导致期望文件没被 `git add` 提交（CI 会全挂）。修法：期望文件改用 `.expected` 后缀，与编译产物 `.out` 区分。

### 2026-09-08 — AI-02 内存安全

1. **结构体字段动态化后释放要跟上**：File.content 改动态分配后，所有 `free(f)` 的地方必须先 `free(f->content)`（cmd_delete_file、dir_destroy）——否则泄漏。封装 `file_free` 统一收口。
2. **字符串替换锚点会被行尾空格坑**：cmd_create_dir 的代码有空混合缩进（行尾多余空格），精确 replace 匹配失败。修法：正则 `\s*` 宽容匹配。给机器改代码要留意肉眼不可见字符。
3. **旧上限测试用例要重新设计**：write 内容单行超过旧 CONTENT_SIZE 无法从命令行构造（main 的 line=256 会截断），改多段写入累积超上限来验证扩容。

### 2026-09-08 — AI-01 KMP 模块

1. **nextval 计算的越界隐患**：`++i` 后 `i == m` 时若再比较 `pat[i]`（即 `pat[m]`）越界一字节。修法：`if (i < m && pat[i] == pat[j])`。规范化实现才能保证安全。
2. **命令行空关键词 ≠ 函数空模式**：`find_file ""` 的 `""` 会被 strtok 当字面 token（两个引号字符）传给 kw；真正的空关键词要靠"无参数命令"，属 Issue #3 输入安全范围。

### 2026-09-08 — cmd 模块

1. **strcpy 当比较用**：`strcpy(g_cwd->name,name)==0` 恒假（strcpy 返回目标指针）。要判断用什么就用什么：查重用 `dir_find_child`，别拿拷贝函数顶替。
2. **只建不挂**：`dir_new(name)` 只创建节点，没 `dir_add_child` 挂到父目录——节点直接丢失。创建 = 建节点 + 挂载两步。
3. **delete_dir 语义错位**：判断「要删的名字 == 当前目录名」不是题目语义；且 `dir_destroy(g_cwd)` 删自己 → g_cwd 悬垂 + 后续 use-after-free。应先 `dir_find_child(g_cwd, name)` 找目标，再「摘链 + 递归销毁」。
4. **cd 忘处理 ".."**：`cd ..` 会去查叫 ".." 的子目录 → 必然报 not found。
5. **处理完没 return**：cd 的 ".." 分支切到父目录后继续往下查——加了 return 才挡住。
6. **static 跨文件不可见**：`find_walk` 声明/定义都加 static，cmd.c 引用时链接失败（undefined reference）。供外部调用的函数不要 static。
7. **extern 声明 ≠ 定义**：全局变量只在 fs.h 声明，fs.c 忘记定义——-c 编译不报，链接才暴露（undefined symbol）。记得 fs.c 里 `Directory *g_root = NULL;` 等。
8. **统计时顺手打印**：find_walk 第一遍遍历统计 found 时也 printf，导致 SEARCH RESULTS 行出现路径之后。修法：遍历函数加 print 开关，第一遍只计数。
9. **消息拼写**：`SUCCESS: rename dir` 应为 `renamed dir`；`Dir ` 单空格应为 `Dir  ` 双空格。
10. **错误消息带不带名字**：cd/delete 的 `dir not found`/`file not found` 固定不带名字；rename/find/open/read 的 `not found` 带名字。

### 2026-09-08 — fs 模块

1. **多余分号**：`free(f;)` 在括号内侧多了一个 `;`，语法错误（`expected ')'`）。多看括号匹配。
2. **把教学占位符照抄进代码**：`snprintf(path + len, "...", ...)` 的 `"..."` 是示意用的占位，真实代码里 size 参数必须是数字（如 `PATH_BUF_SIZE - len`）。占位符是拿来理解思路的，不是拿来粘贴的。
3. **头插只做了一半**：`dir_add_child` 写了 `child->nextbro_dir = d->firstchild_dir` 却漏了 `d->firstchild_dir = child`——指针更新必须成对。后果：目录链表永远不被更新。
4. **只删哈希不摘链表**：`dir_remove_child` / `file_remove` 只调 `hash_delete`，链表节点还挂着 → `dir_destroy` 的 `while(firstchild_*)` 死循环 + free 后悬垂指针。增删必须链表 + 哈希同步维护（指导书反复强调的要点）。
5. **malloc 不初始化就存指针**：`dir_new` 用 malloc 只清了哈希表，`parent`/`firstchild_dir`/`firstchild_file` 是垃圾值——空目录 `ls` 遍历直接野指针崩溃。修法：结构体整体 `calloc`。
6. **递归返回后路径状态没想清楚**：`walk_post` 递归返回时 path 已含目录名（还原点在 len2），又拼了一次 `c->name` → 路径重复。修法：打印 path 自身。
7. **硬编码 + 不补 '\0'（hash 坑重犯）**：`strncpy(d->name, name, 20)` 硬编码 20，且 name 恰好 20 字符时无结尾符。统一 `NAME_SIZE-1` + 手动补 `'\0'`。
8. **路径斜杠语义**：最初 path 表示「无分隔目录链」，递归后用 `printf(path, name)` 出现 `/testDirtestFile4.txt`。重定义为「path 末尾带 '/'（根层为空）」，打印时直接拼接名字。

### 2026-09-08 — hash 模块

1. **数组不能整体赋值**：`node->key = key;` 编译报错 `array type 'char[20]' is not assignable`。即使换个写法，`key` 也只是指针——存地址不存内容，调用方缓冲区一变就悬垂。修法：`strncpy(node->key, key, NAME_SIZE - 1)` + 手动补 `'\0'`。
2. **结构体按值传参会丢修改**：`hash_destroy(HashTable ht)` 参数是值拷贝，函数里 `ht->table[i]` 编译报错；就算改成点号，改的也是副本。修法：改为传指针 `HashTable *ht`。
3. **复制粘贴残留参数**：hash_find / hash_delete 从 insert 复制声明时多带了 `void *value` 参数。修法：声明与定义两处都删。
4. **哈希表必须清零初始化**：`HashTable ht = {{0}};`——不初始化则桶内是野指针，find 直接崩。fs.c 新建目录时两张哈希表同样要清零。