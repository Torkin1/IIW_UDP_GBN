PROJECT_NAME=IIW_UDP_GBN
EXEC_DIR=bin
CC=gcc
FLAGS=-g -Wall -Wextra
LIBS=-lpthread
OUT_DEBUG_DIR=$(EXEC_DIR)/Debug
OUT_TEST_DIR=$(EXEC_DIR)/Test
OUT_DEBUG=$(OUT_DEBUG_DIR)/$(PROJECT_NAME)
OUT_TEST=$(OUT_TEST_DIR)/$(PROJECT_NAME)
LOGIC=$(wildcard logic/*.c)
TEST=$(filter-out logic/main.c, $(LOGIC)) $(filter-out test/allTestsLauncher.c, $(wildcard test/*.c))

Debug: $(LOGIC)
	mkdir -p $(OUT_DEBUG_DIR)
	$(CC) $(FLAGS) $(LIBS) $(LOGIC) -o $(OUT_DEBUG)

Test: $(TEST)
	mkdir -p $(OUT_TEST_DIR)
	$(CC) $(FLAGS) $(LIBS) $(TEST) -o $(OUT_TEST)
