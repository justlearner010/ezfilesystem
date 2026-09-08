# 简单文件系统 —— 精简版规格（SPEC）

> 本文档把老师提供的《简单的文件系统-题目描述》《简单的文件系统-实验指导用书》两份文档
> 精简为一份**可执行规格**：结构用图、命令用表、细节逐条标注。
> 原文档在仓库根目录，本文件是实现的唯一规格依据。
>
> 阅读顺序建议：`§2 结构图` → `§3 命令规格表` → `§4 状态机` → `§5 细节标注`。

---

## 1. 项目目标

用 C 实现一个**支持多级目录结构的命令行文件系统**：

| 需求 | 说明 |
|---|---|
| 目录树 | 多级嵌套，孩子-兄弟表示法；递归删除子树 |
| 哈希表 | 每个目录两张：子目录名→节点、文件名→文件，链地址法，O(1) 查询 |
| 排序 | 文件/目录按**创建顺序，新→旧**；显示时**文件在前、目录在后** |
| 模糊查找 | `find_file` 按文件名**子串匹配**（KMP），输出相对路径 |
| 文件读写 | 内容追加写入；全局打开状态机 |
| 界面 | 命令行，每次读取命令前输出 `>> ` |

常量（来自实验指导书）：`HASH_SIZE=100`、`CONTENT_SIZE=1000`、`NAME_SIZE=20`。

---

## 2. 结构图

### 2.1 目录树（示例 = 官方样例运行后的状态）

```
根目录
├── File  testFile2.txt          ← 文件在前
└── Dir   testDir                ← 目录在后（同层内按 新→旧 排列）
    ├── File  testFile4.txt      ← 新
    └── File  testFile3.txt      ← 旧
```

### 2.2 数据结构关系（核心）

每个目录节点同时持有**两张哈希表 + 两条链表**，链表管顺序、哈希管查找，增删时必须同步维护。

```
        Directory（目录节点）
┌───────────────────────────────────────────────┐
│ name: "testDir"                               │
│ parent ────────────────► 父目录节点            │
│ firstchild_file ───────► File 链表头（新→旧）   │
│ firstchild_dir  ───────► Directory 链表头(新→旧)│
│ subdirs ──► HashTable{ 目录名 → Directory* }   │
│ files   ──► HashTable{ 文件名 → File*    }     │
└───────────────────────────────────────────────┘
        │                       │
        ▼                       ▼
   File 节点                HashTable（HASH_SIZE=100 个桶）
┌──────────────────┐      ┌────┬────┬────┬─ ─ ─┐
│ name             │      │[0] │[1] │[2] │ ...  │
│ content[1000]    │      └─┬──┴────┴─┬──┴─ ─ ─┘
│ nextbro_file     │        ▼         ▼
└──────────────────┘   HashNode   HashNode       ← 链地址法：同哈希值串成链表
                           │ key │ value │ next → … → NULL
```

### 2.3 模块依赖（实现划分）

```
main（REPL + 状态机拦截） → cmd（命令处理/输出） → fs（树与文件操作） → hash（哈希表）
```

- `hash`：只管键值存取，不管对象生老病死
- `fs`：树的增删查改、遍历、递归删除、路径拼接，**不打印结果**
- `cmd`：把参数翻译成 fs 操作，按规格打印 SUCCESS/ERROR
- `main`：`>> ` 提示、读行拆词、open 状态机拦截、分发

### 2.4 文件打开状态机

```
        ┌─────────────────────────────────────────────┐
        │                                             │
        ▼                                             │
   [关闭状态]          open f                [已打开状态]
        │           ──────────────►                 │
        │                                             │
        │  write_file → ERROR: invalid operation      │ 任一命令(除 close_file/
        │                                             │ write_file) → ERROR: invalid operation
        └─ read_file / 其余命令 照常执行               │  write_file → 追加写入
                                                        │  close_file  → 回到关闭状态
```

---

## 3. 命令规格表

### 3.1 文件与目录管理

| 命令 | 行为 | 成功输出 | 失败输出 |
|---|---|---|---|
| `create_file f` | 当前目录建空文件 | `SUCCESS: created file f` | 已有同名**文件** → `ERROR: file f already exists` |
| `create_dir d` | 当前目录建子目录 | `SUCCESS: created directory d` | 已有同名**目录** → `ERROR: directory d already exists` |
| `delete_file f` | 删除文件 | `SUCCESS: f deleted` | `ERROR: file not found` |
| `delete_dir d` | 递归删除目录及子树 | `SUCCESS: d deleted` | `ERROR: dir not found` |
| `rename_file old new` | 文件改名 | `SUCCESS: renamed file old new` | old 无 → `ERROR: old not found`；new 撞同名文件 → `ERROR: file new already exists` |
| `rename_dir old new` | 目录改名（含子目录改名） | `SUCCESS: renamed dir old new` | old 无 → `ERROR: old not found`；new 撞同名目录 → `ERROR: dir new already exists` |
| `cd d` / `cd ..` | 进入子目录 / 返回上级 | **无输出** | 目录不存在 → `ERROR: dir not found`；根目录下 `cd ..` 静默忽略 |

### 3.2 目录输出与查找

| 命令 | 行为 | 输出 |
|---|---|---|
| `ls` | 列当前目录（不含子目录内容） | 文件在前目录在后、新→旧；`File f` / `Dir  d` |
| `ll_pre` | 先根遍历当前目录子树 | 每项一行 `File path` / `Dir  path`（相对路径） |
| `ll_post` | 后根遍历当前目录子树 | 同上格式 |
| `find_file kw` | 先序遍历当前目录子树，文件名含 kw（KMP）的文件 | 首行 `SEARCH RESULTS:`，之后逐行输出相对路径；无匹配 → `ERROR: kw not found` |

### 3.3 文件操作

| 命令 | 行为 | 成功输出 | 失败输出 |
|---|---|---|---|
| `open f` | 打开文件（进入状态机） | `SUCCESS: opened f` | `ERROR: f not found` |
| `close_file` | 关闭当前打开文件 | `SUCCESS: closed f` | （未打开时题目未定义，见 §6） |
| `read_file f` | 输出文件内容 | `CONTENT: 内容` | `ERROR: f not found` |
| `write_file "content"` | 追加到已打开文件末尾（内容仅小写字母/数字/空格，带引号） | `SUCCESS: successfully written` | 未打开 → `ERROR: invalid operation` |

---

## 4. 遍历与排序规则

1. **同一目录内的显示顺序**：文件链表（新→旧）在前，子目录链表（新→旧）在后。`ls`、`find_file`、`ll_pre`、`ll_post` 都遵守。
2. **先根遍历 `ll_pre` / `find_file`**：对每个目录：先其文件链（新→旧），再各子目录（新→旧）递归。
   - 官方样例 `find_file test` 输出顺序：`testFile2.txt` → `newtestFile.txt` → `testDir/testFile4.txt` → `testDir/testFile3.txt` ✓
3. **后根遍历 `ll_post`**：对每个目录：先其文件（叶子，新→旧），再各子目录递归后根，**最后输出该目录自身**。
   - ⚠️ 题目无 ll_post 样例，此为按"孩子-兄弟树标准后根遍历"推导，实现前建议向 TA 确认。
4. **创建顺序**：新对象**头插**链表；**重命名不改变链表位置**（样例中 rename 后 testFile.txt 仍排在 testFile2.txt 之后）。

---

## 5. 细节标注（逐条从原文/样例确认）

| # | 细节 | 结论 |
|---|---|---|
| 1 | 输出空格 | `Dir  xxx` 是**两个空格**；`File xxx` 是**一个空格**（对照 docx 原文确认） |
| 2 | `SEARCH RESULTS:` 行 | 独立一行输出，后面不带空格直接换行，然后逐行路径 |
| 3 | `cd` 成功 | **无任何输出**；`cd ..` 在根目录同样无输出 |
| 4 | `open` 成功 | 有输出 `SUCCESS: opened f` |
| 5 | 同名冲突只查本类 | 同名文件和目录可共存；`create_file` 只查文件表，不查目录表，反之亦然 |
| 6 | `read_file` | 输出 `CONTENT: ` + 内容；空文件输出 `CONTENT: ` |
| 7 | `write_file` 追加 | 多次 write 依次拼接；新文件内容为空 |
| 8 | find 路径 | 相对**当前目录**：cwd 直属文件无前缀（如 `testFile2.txt`），深层为 `a/b/f` |
| 9 | `rename_dir` 的 old 检查 | 原文写"不存在名为 old 的文件或目录 → ERROR: old not found"，输出与"只查目录表"一致，实现按目录表即可 |
| 10 | 状态机拦截范围 | open 之后**所有**命令（含 open/create/cd/ls/read 等）除非是 `close_file`/`write_file`，一律 `ERROR: invalid operation` |

---

## 6. 未定义行为（题目没写，实现时自行决策，需在 README/Commit 中记录）

| 场景 | 建议决策 |
|---|---|
| 未知命令（未打开状态） | 静默忽略 |
| `close_file` 但未打开 | `ERROR: invalid operation` |
| 文件名超 `NAME_SIZE=20` / 内容超 `CONTENT_SIZE=1000` | 题目保证合法输入，暂不处理（AI 版处理截断） |
| `read_file` 未打开的越权问题 | 允许（规格只禁了写入） |
| 名字超长（≥ NAME_SIZE=20）| AI 版：拒绝创建/重命名，输出 `ERROR: name too long`（Issue #2 决策） |
| 内容超出 CONTENT_SIZE=1000 | AI 版：File.content 动态扩容，无上限（Issue #2 决策） |
| 目录嵌套过深（递归删除栈风险） | AI 版：ASan 实测 800 层无栈溢出，暂不加限制（Issue #2 决策） |
| 路径缓冲溢出 | AI 版：PATH_BUF_SIZE 256→1024 + snprintf 截断（Issue #2 决策） |

---

## 7. 官方样例速查

- 输入：`tests/sample_input.txt`
- 含回显的完整交互：`tests/sample_transcript.txt`（`>> ` 后的命令文字来自终端回显，程序 stdout 只含 `>> ` + 结果）