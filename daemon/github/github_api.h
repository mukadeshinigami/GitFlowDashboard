#ifndef GITHUB_API_H
#define GITHUB_API_H

#include <stdbool.h>

/*
 * Структура для хранения информации о репозитории GitHub
 */
typedef struct {
    char* name;              // Название репозитория
    char* full_name;         // Полное имя (owner/repo)
    char* description;       // Описание
    char* owner;             // Владелец репозитория
    char* default_branch;    // Ветка по умолчанию
    int stars;               // Количество звезд
    int forks;               // Количество форков
    bool is_private;         // Приватный ли репозиторий
} GitHubRepository;

/*
 * Инициализация модуля GitHub API
 * Должна быть вызвана перед использованием других функций
 * Возвращает 0 при успехе, -1 при ошибке
 */
int github_api_init(void);

/*
 * Очистка ресурсов модуля GitHub API
 * Должна быть вызвана в конце работы программы
 */
void github_api_cleanup(void);

/*
 * Выполнить GET-запрос к GitHub API
 * 
 * Параметры:
 *   url - полный URL запроса (например: "https://api.github.com/repos/owner/repo")
 *   token - токен авторизации (может быть NULL для публичных запросов)
 * 
 * Возвращает:
 *   Указатель на строку с JSON-ответом (нужно освободить через free())
 *   NULL при ошибке
 */
char* github_api_get(const char* url, const char* token);

/*
 * Получить информацию о репозитории
 * 
 * Параметры:
 *   owner - владелец репозитория
 *   repo - название репозитория
 *   token - токен авторизации
 * 
 * Возвращает:
 *   Указатель на GitHubRepository (нужно освободить через github_repository_free())
 *   NULL при ошибке
 */
GitHubRepository* github_get_repository(const char* owner, const char* repo, const char* token);

/*
 * Освободить память, выделенную для GitHubRepository
 */
void github_repository_free(GitHubRepository* repo);

#endif // GITHUB_API_H




