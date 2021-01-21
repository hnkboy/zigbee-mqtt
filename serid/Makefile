#CROSS_COMPILE = gcc
LD = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
#TARGETLIB = libseri.so   #目标文件名
TARGET = serid
SRCS = $(wildcard ./src/*.c)
export SRCS
LIBS=-L ./ -lcjson

all: $(TARGET)
$(TARGET): $(SRCS)
	#$(CC) $(SRCS) $(LIBS) -fPIC -shared -o $(TARGETLIB)
	$(CC) $(SRCS) $(LIBS)  -o $(TARGET)
	@echo test

clean:
	-rm $(OBJS) $(TARGET)