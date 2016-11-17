CC=gcc
CPPFLAGS=-I./
CFLAGS=-Wall -g
LIBS= -lpthread

#找到当前目录下所有的.c文件
src = $(wildcard *.c)

#将当前目录下所有的.c  转换成.o给obj
obj = $(patsubst %.c, %.o, $(src))


#main
main=main


target=$(main) $(test)

ALL:$(target)


#生成所有的.o文件
$(obj):%.o:%.c
	$(CC) -c $< -o $@ $(CPPFLAGS) $(CFLAGS) 

#main程序
$(main):main.o ring_buffer.o
	$(CC) $^ -o $@ $(LIBS)
             

#clean指令

clean:
	-rm -rf $(obj) $(target)

distclean:
	-rm -rf $(obj) $(target)

#将clean目标 改成一个虚拟符号
.PHONY: clean ALL distclean
