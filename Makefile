# Makefile of sample for a project

#OS=MAC
OS=LINUX

DAEMON=n
DEBUG=y

INC_DIR=./
SRC_DIR=./

ifeq ($(OS), MAC)
CFLAGS= -g -Wall -D OS_MAC -I$(INC_DIR)
else
CFLAGS= -g -Wall -D OS_LINUX
endif

ifeq ($(OS), MAC)
LDFLAGS=
else
LDFLAGS= -liconv
endif

SRC_FILES=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(SRC_DIR)/dot-matrix.o
TARGET=dot-matrix

all:$(TARGET)

$(TARGET):$(OBJS)
	$(CC) -o $@ $(LDFLAGS) $?
$(SRC_DIR)/%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

.PHONY:clean
clean:
	rm -rf $(OBJS) $(TARGET)
