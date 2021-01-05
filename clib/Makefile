#CROSS_COMPILE = gcc
LD = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
#TARGET = libseri.so   #目标文件名
TARGET = seri
SRCS = $(wildcard ./src/*.c)
export SRCS
LIBS=-L ./ -lcjson

all: $(TARGET)
$(TARGET): $(SRCS)
	#$(CC) $(SRCS) $(LIBS) -fPIC -shared -o $(TARGET)
	$(CC) $(SRCS) $(LIBS)  -o $(TARGET)
	@echo test

clean:
	-rm $(OBJS) $(TARGET)