#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../daemon/config/config_reader.h"

// Вспомогательная функция для проверки теста
static int test_count = 0;
static int pass_count = 0;

#define TEST_ASSERT(condition, message) \
    do { \
        test_count++; \
        if (condition) { \
            printf("  ✓ %s\n", message); \
            pass_count++; \
        } else { \
            printf("  ✗ %s\n", message); \
        } \
    } while(0)

// Тест функции get_github_token с переменной окружения
void test_get_github_token_from_env() {
    printf("\n=== Тест: get_github_token() из переменной окружения ===\n");
    
    // Устанавливаем переменную окружения
    setenv("GITHUB_TOKEN", "test_token_env_123", 1);
    
    const char* token = get_github_token("");
    
    TEST_ASSERT(token != NULL, "Токен не NULL");
    TEST_ASSERT(strcmp(token, "test_token_env_123") == 0, "Токен совпадает с установленным");
    
    // Очищаем
    unsetenv("GITHUB_TOKEN");
}

// Тест функции get_github_token из JSON (когда env нет)
void test_get_github_token_from_json() {
    printf("\n=== Тест: get_github_token() из JSON (env отсутствует) ===\n");
    
    // Убеждаемся, что переменной окружения нет
    unsetenv("GITHUB_TOKEN");
    
    const char* token = get_github_token("test_token_json_456");
    
    TEST_ASSERT(token != NULL, "Токен не NULL");
    TEST_ASSERT(strcmp(token, "test_token_json_456") == 0, "Токен совпадает с JSON");
}

// Тест приоритета: env > json
void test_token_priority_env_over_json() {
    printf("\n=== Тест: Приоритет env над json ===\n");
    
    // Устанавливаем оба источника
    setenv("GITHUB_TOKEN", "env_token", 1);
    
    const char* token = get_github_token("json_token");
    
    TEST_ASSERT(token != NULL, "Токен не NULL");
    TEST_ASSERT(strcmp(token, "env_token") == 0, "Используется токен из env, а не из json");
    
    unsetenv("GITHUB_TOKEN");
}

// Тест ошибки: токен не найден
void test_token_not_found() {
    printf("\n=== Тест: Токен не найден ===\n");
    
    unsetenv("GITHUB_TOKEN");
    
    const char* token = get_github_token("");
    
    TEST_ASSERT(token == NULL, "Токен NULL при отсутствии источников");
}

// Тест функции load_config с переменной окружения
void test_load_config_with_env() {
    printf("\n=== Тест: load_config() с переменной окружения ===\n");
    
    setenv("GITHUB_TOKEN", "load_test_token", 1);
    
    Config* config = load_config("daemon/config/config.json");
    
    TEST_ASSERT(config != NULL, "Конфиг загружен успешно");
    if (config != NULL) {
        TEST_ASSERT(config->github_token != NULL, "Токен в конфиге не NULL");
        TEST_ASSERT(strcmp(config->github_token, "load_test_token") == 0, "Токен совпадает");
        free_config(config);
    }
    
    unsetenv("GITHUB_TOKEN");
}

// Тест функции load_config без переменной окружения
void test_load_config_without_env() {
    printf("\n=== Тест: load_config() без переменной окружения ===\n");
    
    unsetenv("GITHUB_TOKEN");
    
    Config* config = load_config("daemon/config/config.json");
    
    // Сейчас это должно вернуть NULL, т.к. файл не читается
    TEST_ASSERT(config == NULL, "Конфиг NULL при отсутствии токена");
}

// Тест функции free_config
void test_free_config() {
    printf("\n=== Тест: free_config() ===\n");
    
    setenv("GITHUB_TOKEN", "free_test_token", 1);
    
    Config* config = load_config("daemon/config/config.json");
    
    if (config != NULL) {
        free_config(config);
        TEST_ASSERT(1, "free_config() выполнен без ошибок (проверка на segfault)");
    }
    
    // Тест с NULL
    free_config(NULL);
    TEST_ASSERT(1, "free_config(NULL) безопасен");
    
    unsetenv("GITHUB_TOKEN");
}

// Тест управления памятью
void test_memory_management() {
    printf("\n=== Тест: Управление памятью ===\n");
    
    setenv("GITHUB_TOKEN", "memory_test", 1);
    
    Config* config1 = load_config("daemon/config/config.json");
    Config* config2 = load_config("daemon/config/config.json");
    
    TEST_ASSERT(config1 != NULL && config2 != NULL, "Два конфига загружены");
    
    if (config1 && config2) {
        // Проверяем, что это разные копии в памяти
        TEST_ASSERT(config1 != config2, "Конфиги в разной памяти");
        TEST_ASSERT(config1->github_token != config2->github_token, "Токены в разной памяти");
        
        free_config(config1);
        free_config(config2);
        TEST_ASSERT(1, "Память освобождена без ошибок");
    }
    
    unsetenv("GITHUB_TOKEN");
}

int main() {
    printf("========================================\n");
    printf("Тесты для config_reader.c\n");
    printf("========================================\n");
    
    // Запускаем все тесты
    test_get_github_token_from_env();
    test_get_github_token_from_json();
    test_token_priority_env_over_json();
    test_token_not_found();
    test_load_config_with_env();
    test_load_config_without_env();
    test_free_config();
    test_memory_management();
    
    // Итоги
    printf("\n========================================\n");
    printf("Результаты тестов:\n");
    printf("  Всего тестов: %d\n", test_count);
    printf("  Пройдено:     %d\n", pass_count);
    printf("  Провалено:   %d\n", test_count - pass_count);
    printf("========================================\n");
    
    if (pass_count == test_count) {
        printf("✓ Все тесты пройдены!\n");
        return 0;
    } else {
        printf("✗ Некоторые тесты провалены\n");
        return 1;
    }
}

