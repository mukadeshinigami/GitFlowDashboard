# Makefile для GitFlow Dashboard Daemon

# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11 -D_POSIX_C_SOURCE=200809L
INCLUDES = -I. -I../shared/libs/cjson

# Директории
DAEMON_DIR = .
CONFIG_DIR = $(DAEMON_DIR)/config
GITHUB_DIR = $(DAEMON_DIR)/github
CJSON_DIR = ../shared/libs/cjson

# Исходные файлы
MAIN_SRC = $(DAEMON_DIR)/main.c
CONFIG_READER_SRC = $(CONFIG_DIR)/config_reader.c
GITHUB_API_SRC = $(GITHUB_DIR)/github_api.c
CJSON_SRC = $(CJSON_DIR)/cJSON.c

# Объектные файлы
MAIN_OBJ = $(DAEMON_DIR)/main.o
CONFIG_READER_OBJ = $(DAEMON_DIR)/config_reader.o
GITHUB_API_OBJ = $(DAEMON_DIR)/github_api.o
CJSON_OBJ = $(DAEMON_DIR)/cJSON.o

# Исполняемый файл
TARGET = $(DAEMON_DIR)/gitflowd

# Все объектные файлы
OBJS = $(MAIN_OBJ) $(CONFIG_READER_OBJ) $(GITHUB_API_OBJ) $(CJSON_OBJ)

# Библиотеки для линковки
LIBS = -lcurl

# Цели по умолчанию
.PHONY: all clean help

all: $(TARGET)

# Сборка исполняемого файла
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS)
	@echo "Сборка завершена: $(TARGET)"

# Компиляция main.c
$(MAIN_OBJ): $(MAIN_SRC) $(CONFIG_DIR)/config_reader.h $(GITHUB_DIR)/github_api.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $(MAIN_SRC) -o $(MAIN_OBJ)

# Компиляция config_reader.c
$(CONFIG_READER_OBJ): $(CONFIG_READER_SRC) $(CONFIG_DIR)/config_reader.h $(CJSON_DIR)/cJSON.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $(CONFIG_READER_SRC) -o $(CONFIG_READER_OBJ)

# Компиляция github_api.c
$(GITHUB_API_OBJ): $(GITHUB_API_SRC) $(GITHUB_DIR)/github_api.h $(CJSON_DIR)/cJSON.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $(GITHUB_API_SRC) -o $(GITHUB_API_OBJ)

# Компиляция cJSON.c
$(CJSON_OBJ): $(CJSON_SRC) $(CJSON_DIR)/cJSON.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $(CJSON_SRC) -o $(CJSON_OBJ)

# Очистка
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Очистка завершена"

# Помощь
help:
	@echo "Доступные команды:"
	@echo "  make       - скомпилировать daemon (gitflowd)"
	@echo "  make clean - удалить скомпилированные файлы"
	@echo "  make help  - показать эту справку"
