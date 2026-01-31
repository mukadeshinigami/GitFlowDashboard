#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "config/config_reader.h"
#include "github/github_api.h"

#define CONFIG_PATH "config/config.json"

int main(void) {
    printf("Starting GitFlow Dashboard Daemon...\n");
    printf("Config path: %s\n", CONFIG_PATH);

    // Инициализируем GitHub API
    if (github_api_init() != 0) {
        fprintf(stderr, "[ERROR] Failed to initialize GitHub API\n");
        return 1;
    }

    // Загружаем конфигурацию
    Config* config = load_config(CONFIG_PATH);
    if (config == NULL) {
        fprintf(stderr, "[ERROR] Failed to load config\n");
        github_api_cleanup();
        return 1;
    }

    // Тестируем GitHub API: получаем информацию о первом репозитории
    if (config->repositories_count > 0 && config->repositories[0].enabled) {
        Repository* repo = &config->repositories[0];
        printf("\n[TEST] Получаем информацию о репозитории: %s/%s\n", 
               repo->owner, repo->name);

        GitHubRepository* github_repo = github_get_repository(
            repo->owner, 
            repo->name, 
            config->github_token
        );

        if (github_repo != NULL) {
            printf("\n[RESULT] Информация о репозитории:\n");
            printf("  Название: %s\n", github_repo->name ? github_repo->name : "(не указано)");
            printf("  Полное имя: %s\n", github_repo->full_name ? github_repo->full_name : "(не указано)");
            printf("  Владелец: %s\n", github_repo->owner ? github_repo->owner : "(не указано)");
            printf("  Описание: %s\n", github_repo->description ? github_repo->description : "(нет описания)");
            printf("  Ветка по умолчанию: %s\n", github_repo->default_branch ? github_repo->default_branch : "(не указано)");
            printf("  Звезд: %d\n", github_repo->stars);
            printf("  Форков: %d\n", github_repo->forks);
            printf("  Приватный: %s\n", github_repo->is_private ? "да" : "нет");

            github_repository_free(github_repo);
        } else {
            fprintf(stderr, "[ERROR] Не удалось получить информацию о репозитории\n");
        }
    } else {
        printf("[INFO] Нет активных репозиториев для тестирования\n");
    }

    // Освобождаем ресурсы
    free_config(config);
    github_api_cleanup();

    printf("\nGitFlow Dashboard Daemon завершен\n");
    return 0;
}

