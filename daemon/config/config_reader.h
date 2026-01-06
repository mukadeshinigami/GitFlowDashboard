#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <stddef.h>

typedef struct {
    char* github_token;
} Config;

// Получить GitHub токен с приоритетом (env > json)
// Возвращает указатель на токен или NULL, если не найден
const char* get_github_token(const char* token_from_json);

// Загрузить конфигурацию из файла
// Возвращает указатель на Config или NULL при ошибке
// Память должна быть освобождена через free_config()
Config* load_config(const char* config_path);

// Освободить память, выделенную для конфигурации
void free_config(Config* config);

#endif // CONFIG_READER_H

