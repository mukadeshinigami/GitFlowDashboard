#!/bin/bash
# Загружаем переменные окружения
source "$(dirname "$0")/load-env.sh"

# Переходим в директорию daemon
cd "$(dirname "$0")/../daemon" || exit 1

# Компилируем (если нужно)
if [ ! -f gitflowd ] || [ main.c -nt gitflowd ] || [ config/config_reader.c -nt gitflowd ]; then
    echo "Компиляция..."
    mkdir -p ../tmp
    
    gcc -std=c11 -D_POSIX_C_SOURCE=200809L \
        -I. -I./config -I../shared/libs/cjson \
        -c config/config_reader.c -o ../tmp/config_reader.o
    
    gcc -std=c11 -D_POSIX_C_SOURCE=200809L \
        -I. -I./config -I../shared/libs/cjson \
        -c ../shared/libs/cjson/cJSON.c -o ../tmp/cjson.o
    
    gcc -std=c11 -D_POSIX_C_SOURCE=200809L \
        -I. -I./config -I../shared/libs/cjson \
        -c main.c -o ../tmp/main.o
    
    gcc ../tmp/main.o ../tmp/config_reader.o ../tmp/cjson.o \
        -o gitflowd
    
    echo "✓ Компиляция завершена"
fi

# Запускаем
./gitflowd