#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
/*
    Структура для хранения информации о репозитории
*/
    char* name;
    char* owner;
    char* branch;
    bool enabled;
 } Repository;

typedef struct {
    /*
    Структура для хранения конфигурации
    */
    char* github_token;
    Repository* repositories; // массив репозиториев
    int repositories_count; // количество репозиториев
    int poll_interval; // интервал опроса
    char* data_dir; // директория для хранения данных
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
