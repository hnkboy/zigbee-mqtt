#CROSS_COMPILE = gcc
LD = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
#TARGETLIB = libseri.so   #目标文件名
TARGET = serid
SRCS = $(wildcard ./src/*.c)
export SRCS
LIBS=-L ./ -lcjson
#编译出lib
#$(CC) $(SRCS) $(LIBS) -fPIC -shared -o $(TARGETLIB)
all: $(TARGET)
$(TARGET): $(SRCS)

	$(CC) $(SRCS) $(LIBS)  -o $(TARGET)
	@echo build done

clean:
	-rm $(OBJS) $(TARGET)