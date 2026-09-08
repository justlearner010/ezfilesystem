# 简单文件系统 —— 设计与实现方案

> 目标：用 C/C++ 实现一个支持多级目录的命令行文件系统。
> `human_version/` 是我（用户）自己实现的简化版；`AI_version/` 待简化版完成后一起讨论升级点。

## 0. 任务概述

- 树状多级目录（孩子-兄弟表示法），目录下有文件，文件有内容（追加写入）。
- 每个目录用两张哈希表加速「子目录名 → 目录节点」「文件名 → 文件对象」查询。
- 命令行交互：每次读取命令前输出 `>> `。
- 目录/文件排序：按照**创建顺序，新 → 旧**（后创建的在前）。文件在前、目录在后（ls / 遍历的显示约定）。
- 文件查找：`find_file` 用 **KMP** 子串匹配，输出相对路径。

### 命令清单与输出格式（已逐条核对 docx 原文）

| 命令 | 成功输出 | 失败输出 |
|---|---|---|
| `create_file f` | `SUCCESS: created file f` | 同名文件已存在 → `ERROR: file f already exists` |
| `create_dir d` | `SUCCESS: created directory d` | 同名目录已存在 → `ERROR: directory d already exists` |
| `delete_file f` | `SUCCESS: f deleted` | 不存在 → `ERROR: file not found` |
| `delete_dir d` | `SUCCESS: d deleted` | 不存在 → `ERROR: dir not found` |
| `rename_file old new` | `SUCCESS: renamed file old new` | old 不存在 → `ERROR: old not found`；new 已有同名文件 → `ERROR: file new already exists` |
| `rename_dir old new` | `SUCCESS: renamed dir old new` | old 不存在 → `ERROR: old not found`；new 已有同名目录 → `ERROR: dir new already exists` |
| `find_file kw` | `SEARCH RESULTS:` 换行后逐行输出相对路径 | 无匹配 → `ERROR: kw not found` |
| `ls` | `File 文件名` / `Dir  目录名`（注意 Dir 后是**两个**空格） | — |
| `ll_pre` / `ll_post` | `File 相对路径` / `Dir  相对路径` | — |
| `cd d` / `cd ..` | （无输出） | 目录不存在 → `ERROR: dir not found`；根目录下 `cd ..` 忽略 |
| `open f` | `SUCCESS: opened f` | 不存在 → `ERROR: f not found` |
| `close_file` | `SUCCESS: closed f` | 未打开时行为未定义（建议 `ERROR: invalid operation`，待确认） |
| `read_file f` | `CONTENT: 文件内容` | 不存在 → `ERROR: f not found` |
| `write_file "content"` | `SUCCESS: successfully written`（追加到已打开文件末尾） | 未处于打开状态 → `ERROR: invalid operation` |

**状态机（重要）**：文件被 open 后，除 `close_file`、`write_file` 之外的**所有**命令一律输出 `ERROR: invalid operation`（包括 open、create、cd、ls、read_file 等）。

## 1. 总体结构（模块划分）

建议拆 4 个模块，依赖方向：`main → cmd → fs → hash`。

```
ezfilesystem/
├── DESIGN.md
├── tests/
│   ├── sample_input.txt       # 官方样例输入（可直接回放）
│   └── sample_transcript.txt  # 官方样例完整回显（对照用）
├── human_version/             # 我自己实现的简化版
│   ├── hash.h / hash.c        # 哈希表（链地址法）
│   ├── fs.h   / fs.c          # 目录树 + 文件对象 + 遍历/删除/查找
│   ├── cmd.h  / cmd.c         # 每条命令的处理（含输出）
│   ├── main.c                 # REPL 主循环 + 命令解析 + 状态机
│   └── Makefile
└── AI_version/                # 升级版（后续讨论）
```

模块职责：
- **hash**：只负责「键 → 值」的插入/查找/删除，不关心 File/Directory 生命周期。
- **fs**：数据结构定义、树的增删查、遍历、递归删除、路径拼接。不打印结果。
- **cmd**：把命令参数翻译成 fs 操作，并按题目要求的格式打印 SUCCESS/ERROR。
- **main**：`>> ` 提示、读行、拆词、open 状态机拦截、分发。

## 2. 数据结构（沿用实验指导书的定义）

```c
#define HASH_SIZE    100
#define CONTENT_SIZE 1000
#define NAME_SIZE    20

typedef struct File File;
typedef struct Directory Directory;

/* 哈希节点：链地址法，桶内单向链表 */
typedef struct HashNode {
    char key[NAME_SIZE];          // 键的拷贝（重命名时需同步更新）
    void *value;                  // File* 或 Directory*
    struct HashNode *next;
} HashNode;

typedef struct HashTable {
    HashNode *table[HASH_SIZE];
} HashTable;

typedef struct Directory {
    char name[NAME_SIZE];
    struct Directory *parent;              // 父目录
    struct Directory *firstchild_dir;      // 子目录链表头（头插 = 最新在前）
    struct Directory *nextbro_dir;         // 兄弟目录
    struct File *firstchild_file;          // 文件链表头（头插 = 最新在前）
    HashTable *subdirs;                    // 直接子目录：名 → Directory*
    HashTable *files;                      // 直接文件：名 → File*
} Directory;

typedef struct File {
    char name[NAME_SIZE];
    char content[CONTENT_SIZE];            // 文件内容（新文件为空）
    struct File *nextbro_file;             // 兄弟文件
} File;
```

**全局状态（main/fs 模块共享）**：

```c
Directory *g_root;      // 根目录（常在）
Directory *g_cwd;       // 当前目录
int  g_is_open;         // 0 / 1
File *g_opend_file;     // 当前打开的文件
```

### 设计要点 / 为什么

1. **链表 + 哈希表双结构**：同目录下文件/子目录各一条链表（头插维护"新→旧"顺序，供 ls/遍历用），两张哈希表保证 O(1) 查重与定位。增删必须**同步维护两个结构**，这是最容易漏 bug 的地方。
2. **文件与目录严格区分**：同名文件和目录可以共存（两张哈希表互不干扰），错误提示也分别判断。
3. **重命名不改变创建顺序**：只改 `name` 字段 + 更新哈希表（删除旧键、插入新键），链表位置不动。

## 3. 函数接口设计

### 3.1 哈希表 hash.h

```c
unsigned hash_key(const char *key);                 // sum(ASCII) % HASH_SIZE
void    hash_insert(HashTable *ht, const char *key, void *value);
void   *hash_find(HashTable *ht, const char *key);  // 未找到返回 NULL
int     hash_delete(HashTable *ht, const char *key);// 只删 HashNode，不释放 value
void    hash_destroy(HashTable *ht);                // 释放全部 HashNode，不释放 value
```

### 3.2 文件系统核心 fs.h

```c
/* 目录 */
Directory *dir_new(const char *name);                   // 含 parent=NULL、两张哈希表
void       dir_add_child(Directory *d, Directory *child);   // 头插 + 写哈希
Directory *dir_find_child(Directory *d, const char *name);  // 查哈希
void       dir_remove_child(Directory *d, Directory *child);// 摘链表 + 删哈希，不释放
void       dir_destroy(Directory *d);                   // 后序递归删除整棵子树并释放

/* 文件 */
File *file_new(const char *name);                       // 内容为空
void  file_add(Directory *d, File *f);                  // 头插 + 写哈希
File *file_find(Directory *d, const char *name);        // 查哈希
void  file_remove(Directory *d, File *f);               // 摘链表 + 删哈希，不释放

/* 遍历（find_file / ll_pre / ll_post 共用） */
void walk_pre (Directory *cur, char *path, int len);    // 先根：先文件链后子目录链
void walk_post(Directory *cur, char *path, int len);    // 后根：见 §5.3
```

> `path` 是相对 `g_cwd` 的路径缓冲，递归时 `path+len` 处追加 `/name`。
> `find_file` 不单独写遍历，用 `walk_pre` + 对文件名做 KMP 匹配即可。

### 3.3 命令处理 cmd.h

```c
void cmd_create_file(const char *name);
void cmd_create_dir (const char *name);
void cmd_delete_file(const char *name);
void cmd_delete_dir (const char *name);
void cmd_rename_file(const char *old, const char *neu);
void cmd_rename_dir (const char *old, const char *neu);
void cmd_find_file  (const char *kw);       // 内部 KMP
void cmd_ls(void);
void cmd_ll_pre(void);
void cmd_ll_post(void);
void cmd_cd(const char *name);              // ".." 特判
void cmd_open(const char *name);
void cmd_close_file(void);
void cmd_read_file(const char *name);
void cmd_write_file(const char *content);   // content 不带引号
```

### 3.4 主循环 main.c 伪代码

```
while (1) {
    printf(">> ");
    fgets(buf, sizeof buf, stdin); 去掉末尾 '\n'
    if (EOF) break;
    解析出命令名 + 参数（write_file 特殊：取引号内内容，可能含空格）
    if (g_is_open && 命令不是 write_file 也不是 close_file) {
        printf("ERROR: invalid operation\n"); continue;
    }
    if (命令 == write_file && !g_is_open) { printf("ERROR: invalid operation\n"); continue; }
    switch / if-else 分发到 cmd_*
}
```

## 4. 关键算法

### 4.1 哈希：链地址法
- `hash_key(key) = Σ ASCII(key[i]) % HASH_SIZE`
- 桶内**头插**（插入更快；不要求桶内有序）。
- 查重只需 `hash_find` 判空。

### 4.2 创建顺序（新 → 旧）
- 所有文件/子目录**头插**链表。头结点即最新创建的。
- `ls`：先遍历文件链（新→旧），再遍历子目录链（新→旧），文件在前目录在后。

### 4.3 递归删除（后序）
```
dir_destroy(d):
    while (d->firstchild_file) { f = 头; file_remove(d, f); free(f); }   // 先删文件
    while (d->firstchild_dir)  { c = 头; dir_remove_child(d, c); dir_destroy(c); } // 再递归删子目录
    hash_destroy(d->subdirs); hash_destroy(d->files);
    free(d);
```
- 要点：**先摘链 + 删哈希，再递归释放**，避免悬空指针；哈希节点在 `dir_remove_child` 里已删。

### 4.4 KMP（find_file 用）
- 对关键词预处理 `nextval[]`，`kmp_match(text, kw)` 判断文件名**包含**关键词。
- 场景是"文件名里找子串"，文本短（≤ NAME_SIZE），但题目要求用 KMP 体现算法，简化版可先用 `strstr`，AI 版再换 KMP。

### 4.5 遍历顺序定义
- **先根 `ll_pre`**：对每个目录：先输出其文件链（新→旧），再对每个子目录（新→旧）递归。样例吻合：
  ```
  File testFile2.txt
  Dir  testDir
  File testDir/testFile4.txt
  File testDir/testFile3.txt
  ```
- **后根 `ll_post`**：对每个目录：先输出其文件（叶子，新→旧），再对每个子目录递归后根，**最后输出该目录自身**。即样例状态应输出：
  ```
  File testFile2.txt
  File testDir/testFile4.txt
  File testDir/testFile3.txt
  Dir  testDir
  ```
  > ⚠️ 题目没有给出 ll_post 的样例输出，上面的定义是「孩子-兄弟树的标准后根遍历」推导，实现时以 TA 要求为准，需确认。

### 4.6 find_file 输出顺序
- 与 `ll_pre` 同一遍历：当前目录文件（新→旧）→ 各子目录（新→旧）递归。
- 匹配到就输出相对 `g_cwd` 的路径（cwd 下的文件无前缀，直接文件名）。
- 样例校验：`testFile2.txt, newtestFile.txt, testDir/testFile4.txt, testDir/testFile3.txt` ✓

## 5. 已确认的格式细节 / 易踩坑清单（重要）

1. `Dir  目录名` 是**两个空格**，`File 文件名` 是**一个空格**（对照 docx 原文确认）。
2. `SEARCH RESULTS:` 单独一行，之后每行一个路径；无匹配输出 `ERROR: kw not found`。
3. `cd` 成功**无输出**；`cd ..` 在根目录**无输出且忽略**；`open` 成功输出 `SUCCESS: opened f`。
4. `read_file` 输出 `CONTENT: ` + 内容；空文件即 `CONTENT: `。
5. `write_file` 的 content 由 `"` 包裹，可含空格；**追加**写入，多次 write 依次拼接。
6. `g_is_open` 状态下一切非 write/close 命令 → `ERROR: invalid operation`（含再次 open、cd、ls、read_file、find_file…）。
7. 未打开时 `write_file` → `ERROR: invalid operation`。
8. 同名文件 + 目录可共存；`rename_file` 撞上同名**目录**不算冲突（只查文件哈希表），反之亦然。
9. 重命名（rename_file/rename_dir）不改变链表中的创建顺序位置。
10. 未知命令：题目未定义（打开状态下会统一被 invalid operation 拦截；未打开时建议忽略，待确认）。
11. `close_file` 在未打开时行为题目未定义（建议输出 `ERROR: invalid operation`，待确认）。

## 6. 验证方案

1. 交互式回放 `tests/sample_input.txt`，对照 `tests/sample_transcript.txt`。
   > 注意：`>> ` 提示符后面跟的"回显命令"来自终端，程序 stdout 只含 `>> ` + 结果；管道喂入时输出形如 `>> SUCCESS: ...`，无回显。
2. 边界用例（human 版做完后叠加）：
   - 同名冲突：create_file 撞同名文件 / 撞同名目录 / rename 撞名
   - 递归删除多层目录后 ls / find
   - `cd ..` 到根目录再 `cd ..`
   - 空文件 read、连续多次 write_file 追加
   - find_file 无结果、find 关键词命中 cwd 与深层
   - open 状态机全命令拦截、未 open 时 write/close
   - 长名/长内容越界（NAME_SIZE/CONTENT_SIZE 的截断策略，AI 版处理）

## 7. human_version（简化版）与 AI_version（升级版）分工建议

### human_version 可先简化的点
- find_file 先用 `strstr`，KMP 放到 AI 版。
- 不处理未知命令、close_file 未打开等「题目未定义」分支（能跑通样例即可）。
- 名字/内容长度不校验（题目保证合法输入），NAME_SIZE/CONTENT_SIZE 直接按宏用。
- 内存释放只保证 `delete_dir` 递归正确，程序退出时可不做全局清理。
- 单文件 main.c 起步也可以；拆 4 模块是为了后面升级省事。

### AI_version 升级方向（待 human 完成后细化）
- KMP 正式接入。
- 内存安全：越界截断策略、所有路径的释放检查、哈希节点与对象生命周期梳理。
- 「未定义行为」决策补齐（未知命令、未打开 close 等）。
- 测试框架：样例 diff 自动化 + 边界用例脚本。
- 代码规范、注释、实验报告素材（复杂度表、算法说明）。