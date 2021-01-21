LD = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
.PHONY: serid
clib:
	make -C serid