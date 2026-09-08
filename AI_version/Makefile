CC      = gcc
CFLAGS  = -Wall -g
OBJS    = hash.o fs.o cmd.o kmp.o main.o

ezfs: $(OBJS)
	$(CC) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

# 回放官方样例（样例输入在 ../tests/sample_input.txt）
run: ezfs
	./ezfs < ../tests/sample_input.txt

clean:
	rm -f ezfs $(OBJS)

.PHONY: run clean