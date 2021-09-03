PROJECT_NAME=IIW_UDP_GBN
EXEC_DIR=bin
CC=gcc
FLAGS=-g -Wall -Wextra
LIBS=-lpthread
DEFINE=-DTIMEOUT_CONST	# comment if you want adaptive
INCLUDE=-I./logic
OUT_DEBUG_DIR=$(EXEC_DIR)/Debug
OUT_TEST_DIR=$(EXEC_DIR)/Test
OUT_DEBUG=$(OUT_DEBUG_DIR)/$(PROJECT_NAME)
OUT_TEST=$(OUT_TEST_DIR)/$(PROJECT_NAME)
LOGIC=$(shell find ./logic -name "*.c")
TEST=$(filter-out ./logic/main.c, $(LOGIC)) $(shell find ./test -name "*.c")

Debug: $(LOGIC)
	mkdir -p $(OUT_DEBUG_DIR)
	$(CC) $(FLAGS) $(INCLUDE) $(DEFINE) $(LIBS) $(LOGIC) -o $(OUT_DEBUG)

Test: $(TEST)
	mkdir -p $(OUT_TEST_DIR)
	$(CC) $(FLAGS) $(INCLUDE) $(DEFINE) $(LIBS) $(TEST) -o $(OUT_TEST)
