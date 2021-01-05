LD = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
.PHONY: clib
clib:
	make -C clib