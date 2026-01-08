# Makefile для тестов config_reader

# Пути
PROJECT_ROOT = ../../..
DAEMON_CONFIG_DIR = $(PROJECT_ROOT)/daemon/config
TEST_DIR = .

# Компилятор и флаги
CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
INCLUDES = -I$(DAEMON_CONFIG_DIR) -I$(PROJECT_ROOT)

# Файлы
CONFIG_READER_SRC = $(DAEMON_CONFIG_DIR)/config_reader.c
CONFIG_READER_OBJ = $(TEST_DIR)/config_reader.o
TEST_SRC = $(TEST_DIR)/test_config_reader.c
TEST_BIN = $(TEST_DIR)/test_config_reader

# Цели по умолчанию
.PHONY: all test clean run

all: test

# Компиляция объектного файла config_reader
$(CONFIG_READER_OBJ): $(CONFIG_READER_SRC) $(DAEMON_CONFIG_DIR)/config_reader.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $(CONFIG_READER_SRC) -o $(CONFIG_READER_OBJ)

# Компиляция тестового бинарника
$(TEST_BIN): $(TEST_SRC) $(CONFIG_READER_OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_SRC) $(CONFIG_READER_OBJ) -o $(TEST_BIN)

# Запуск тестов
test: $(TEST_BIN)
	@echo "Запуск тестов..."
	@./$(TEST_BIN)

# Запуск тестов (алиас)
run: test

# Очистка
clean:
	rm -f $(CONFIG_READER_OBJ) $(TEST_BIN)
	@echo "Очистка завершена"

# Помощь
help:
	@echo "Доступные команды:"
	@echo "  make test  - скомпилировать и запустить тесты"
	@echo "  make run   - то же, что и test"
	@echo "  make clean - удалить скомпилированные файлы"
	@echo "  make help  - показать эту справку"





