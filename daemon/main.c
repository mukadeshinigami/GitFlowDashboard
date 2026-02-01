#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "config/config_reader.h"
#include "github/github_api.h"

#define CONFIG_PATH "config/config.json"
#define STR_OR_DEFAULT(s, def) ((s) ? (s) : (def))

static void print_repository_info(const GitHubRepository* repo) {
    printf("\n[RESULT] Информация о репозитории:\n");
    printf("  Название: %s\n", STR_OR_DEFAULT(repo->name, "(не указано)"));
    printf("  Полное имя: %s\n", STR_OR_DEFAULT(repo->full_name, "(не указано)"));
    printf("  Владелец: %s\n", STR_OR_DEFAULT(repo->owner, "(не указано)"));
    printf("  Описание: %s\n", STR_OR_DEFAULT(repo->description, "(нет описания)"));
    printf("  Ветка по умолчанию: %s\n", STR_OR_DEFAULT(repo->default_branch, "(не указано)"));
    printf("  Звезд: %d\n", repo->stars);
    printf("  Форков: %d\n", repo->forks);
    printf("  Приватный: %s\n", repo->is_private ? "да" : "нет");
}

static void test_github_repository(const Config* config) {
    if (config->repositories_count == 0 || !config->repositories[0].enabled) {
        printf("[INFO] Нет активных репозиториев для тестирования\n");
        return;
    }

    const Repository* repo = &config->repositories[0];
    printf("\n[TEST] Получаем информацию о репозитории: %s/%s\n", 
           repo->owner, repo->name);

    GitHubRepository* github_repo = github_get_repository(
        repo->owner, 
        repo->name, 
        config->github_token
    );

    if (github_repo == NULL) {
        fprintf(stderr, "[ERROR] Не удалось получить информацию о репозитории\n");
        return;
    }

    print_repository_info(github_repo);
    github_repository_free(github_repo);
}

int main(void) {
    printf("Starting GitFlow Dashboard Daemon...\n");
    printf("Config path: %s\n", CONFIG_PATH);

    if (github_api_init() != 0) {
        fprintf(stderr, "[ERROR] Failed to initialize GitHub API\n");
        return 1;
    }

    Config* config = load_config(CONFIG_PATH);
    if (config == NULL) {
        fprintf(stderr, "[ERROR] Failed to load config\n");
        github_api_cleanup();
        return 1;
    }

    test_github_repository(config);

    free_config(config);
    github_api_cleanup();

    printf("\nGitFlow Dashboard Daemon завершен\n");
    return 0;
}

