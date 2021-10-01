PROJECT_NAME=IIW_UDP_GBN
EXEC_DIR=bin
CC=gcc
FLAGS=-g -Wall -Wextra
LIBS=-lpthread -lm
DEFINE=-DTIMEOUT_ADAPTIVE# if not specified, a default constant timeout will be used
LOGIC_DIR=./logic
TEST_DIR=./test
INCLUDE=-I$(LOGIC_DIR)
OUT_DEBUG_DIR=$(EXEC_DIR)/Debug
OUT_DEBUG_DIR_CLIENT=$(OUT_DEBUG_DIR)/client
OUT_DEBUG_DIR_SERVER=$(OUT_DEBUG_DIR)/server
OUT_DEBUG_CLIENT=$(OUT_DEBUG_DIR_CLIENT)/client
OUT_DEBUG_SERVER=$(OUT_DEBUG_DIR_SERVER)/server
OUT_TEST_DIR=$(EXEC_DIR)/Test
OUT_TEST_DIR_CLIENT=$(OUT_TEST_DIR)/client
OUT_TEST_DIR_SERVER=$(OUT_TEST_DIR)/server
OUT_TEST=$(OUT_TEST_DIR)/$(PROJECT_NAME)_TestCommon
OUT_TEST_CLIENT=$(OUT_TEST_DIR_CLIENT)/$(PROJECT_NAME)_TestClient
OUT_TEST_SERVER=$(OUT_TEST_DIR_SERVER)/$(PROJECT_NAME)_TestServer
LOGIC=$(shell find $(LOGIC_DIR) -name "*.c")
LOGIC_COMMON=$(shell find $(LOGIC_DIR) \( -path $(LOGIC_DIR)/client -o -path $(LOGIC_DIR)/server \) -prune -false -o -name "*.c")
LOGIC_CLIENT=$(LOGIC_COMMON) $(shell find $(LOGIC_DIR)/client -name "*.c")
LOGIC_SERVER=$(LOGIC_COMMON) $(shell find $(LOGIC_DIR)/server -name "*.c")
TEST_MAIN=$(TEST_DIR)/testMain.c
TEST_COMMON=$(shell find $(TEST_DIR) \( -path $(TEST_DIR)/client -o -path $(TEST_DIR)/server \) -prune -false -o -name "*.c")
TEST_CLIENT=$(TEST_COMMON) $(filter-out $(LOGIC_DIR)/client/main.c, $(LOGIC_CLIENT)) $(shell find $(TEST_DIR)/client -name "*.c")
TEST_SERVER=$(TEST_COMMON) $(filter-out $(LOGIC_DIR)/server/main.c, $(LOGIC_SERVER)) $(shell find $(TEST_DIR)/server -name "*.c")

testCommon: $(LOGIC_COMMON) $(TEST_COMMON)
	mkdir -p $(OUT_TEST_DIR)
	$(CC) $(FLAGS) $(INCLUDE) $(DEFINE) $(LOGIC_COMMON) $(TEST_COMMON) -o $(OUT_TEST) $(LIBS)

testClient: $(TEST_CLIENT)
	mkdir -p $(OUT_TEST_DIR_CLIENT)
	$(CC) $(FLAGS) $(INCLUDE) $(DEFINE) $(TEST_CLIENT) -o $(OUT_TEST_CLIENT) $(LIBS)

testServer: $(TEST_SERVER)
	mkdir -p $(OUT_TEST_DIR_SERVER)
	$(CC) $(FLAGS) $(INCLUDE) $(DEFINE) $(TEST_SERVER) -o $(OUT_TEST_SERVER) $(LIBS)

testClientAndServer: TestClient TestServer

client: $(LOGIC_CLIENT)
	mkdir -p $(OUT_DEBUG_DIR_CLIENT)
	$(CC) $(FLAGS) $(INCLUDE) $(DEFINE) $(LOGIC_CLIENT) -o $(OUT_DEBUG_CLIENT) $(LIBS)

server: $(LOGIC_SERVER)
	mkdir -p $(OUT_DEBUG_DIR_SERVER)
	$(CC) $(FLAGS) $(INCLUDE) $(DEFINE) $(LOGIC_SERVER) -o $(OUT_DEBUG_SERVER) $(LIBS)
	
all: client server
