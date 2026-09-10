# AI_version —— 升级版文件系统（human_version 的增强版，按 Issue #1-5 演进）
#
# 目录约定：
#   include/  对外头文件（hash.h 哈希表 / fs.h 目录树 / cmd.h 命令处理 / kmp.h 子串匹配）
#   src/      实现（hash.c / fs.c / cmd.c / kmp.c / main.c）
#
# 目标产物 ezfs 生成在本目录（与 ../tests/run_tests.py 的约定一致）。

CC      = gcc
INCDIR  = include
SRCDIR  = src
CFLAGS  = -std=c11 -Wall -Wextra -g -I$(INCDIR)

SRCS    = $(wildcard $(SRCDIR)/*.c)
OBJS    = $(SRCS:.c=.o)
DEPS    = $(OBJS:.o=.d)

ezfs: $(OBJS)
	$(CC) -o $@ $(OBJS)

# -MMD -MP：生成头文件依赖（.d），改 .h 也会触发重编译，避免"改了头文件没生效"
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

# 回放官方样例（样例输入在 ../tests/sample_input.txt）
run: ezfs
	./ezfs < ../tests/sample_input.txt

clean:
	rm -f ezfs $(OBJS) $(DEPS)

.PHONY: run clean
