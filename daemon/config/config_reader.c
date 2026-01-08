#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config_reader.h"
#include "../../shared/libs/cjson/cJSON.h"


static char* read_file(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (file == NULL) {
        fprintf(stderr, "[ERROR] Failed to open file %s\n", filepath);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(file);
        return NULL;
    }

    char* file_content = malloc(file_size + 1); 
    if (file_content == NULL) {
        fclose(file);
        return NULL;
    }

    size_t read_bytes = fread(file_content, 1, file_size, file);
    file_content[read_bytes] = '\0';
    fclose(file);
    
    return file_content;
}

// Парсинг массива репозиториев
static int parse_repositories(cJSON* repos_json, Config* config) {
    if (!cJSON_IsArray(repos_json)) {
        fprintf(stderr, "[ERROR] 'repositories' must be an array\n");
        return -1;
    }

    int count = cJSON_GetArraySize(repos_json);
    if (count == 0) {
        config->repositories = NULL;
        config->repositories_count = 0;
        return 0;
    }

    config->repositories = malloc(sizeof(Repository) * count);
    if (config->repositories == NULL) {
        fprintf(stderr, "[ERROR] Failed to allocate memory for repositories\n");
        return -1;
    }
    config->repositories_count = count;

    // Инициализируем все поля NULL
    for (int i = 0; i < count; i++) {
        config->repositories[i].name = NULL;
        config->repositories[i].owner = NULL;
        config->repositories[i].branch = NULL;
        config->repositories[i].enabled = false;
    }

    // Парсим каждый репозиторий
    for (int i = 0; i < count; i++) {
        cJSON* repo_item = cJSON_GetArrayItem(repos_json, i);
        if (!cJSON_IsObject(repo_item)) {
            fprintf(stderr, "[ERROR] Repository[%d] is not an object\n", i);
            goto error;
        }

        // Парсим поля
        cJSON* name = cJSON_GetObjectItem(repo_item, "name");
        cJSON* owner = cJSON_GetObjectItem(repo_item, "owner");
        cJSON* branch = cJSON_GetObjectItem(repo_item, "branch");
        cJSON* enabled = cJSON_GetObjectItem(repo_item, "enabled");

        if (!cJSON_IsString(name) || !cJSON_IsString(owner) || 
            !cJSON_IsString(branch) || !cJSON_IsBool(enabled)) {
            fprintf(stderr, "[ERROR] Repository[%d] has invalid fields\n", i);
            goto error;
        }

        config->repositories[i].name = strdup(name->valuestring);
        config->repositories[i].owner = strdup(owner->valuestring);
        config->repositories[i].branch = strdup(branch->valuestring);
        config->repositories[i].enabled = cJSON_IsTrue(enabled);

        if (!config->repositories[i].name || 
            !config->repositories[i].owner || 
            !config->repositories[i].branch) {
            fprintf(stderr, "[ERROR] Failed to allocate memory for repository[%d]\n", i);
            goto error;
        }
    }

    return 0;

error:
    // Освобождаем уже выделенную память
    for (int i = 0; i < config->repositories_count; i++) {
        free(config->repositories[i].name);
        free(config->repositories[i].owner);
        free(config->repositories[i].branch);
    }
    free(config->repositories);
    config->repositories = NULL;
    config->repositories_count = 0;
    return -1;
}

const char* get_github_token(const char* token_from_json) {
    // Получаем токен из переменной окружения
    const char* env_token = getenv("GITHUB_TOKEN");
    
    
    if (env_token && strlen(env_token) > 0) {
        printf("[CONFIG] Token loaded from environment variable GITHUB_TOKEN\n");
        return env_token;
    }
    if (token_from_json && strlen(token_from_json) > 0) {
        printf("[CONFIG] Token loaded from config.json\n");
        return token_from_json;
    }   
    fprintf(stderr, "[ERROR] GitHub token not found in environment variable GITHUB_TOKEN or config.json\n");
    return NULL;
}

static int parse_github_token(cJSON* json, Config* config) {
    cJSON* github = cJSON_GetObjectItem(json, "github");
    const char* token_from_json = "";
    
    if (cJSON_IsObject(github)) {
        cJSON* token_json = cJSON_GetObjectItem(github, "token");
        if (cJSON_IsString(token_json)) {
            token_from_json = token_json->valuestring;
        }
    }

    const char* token = get_github_token(token_from_json);
    if (!token) {
        return -1;
    }

    config->github_token = strdup(token);
    if (!config->github_token) {
        fprintf(stderr, "[ERROR] Failed to allocate memory for token\n");
        return -1;
    }
    return 0;
}

static void parse_poll_interval(cJSON* json, Config* config) {
    cJSON* poll_interval_json = cJSON_GetObjectItem(json, "poll_interval");
    if (cJSON_IsNumber(poll_interval_json)) {
        config->poll_interval = poll_interval_json->valueint;
    }
}

static int parse_storage_data_dir(cJSON* json, Config* config) {
    cJSON* storage = cJSON_GetObjectItem(json, "storage");
    if (!cJSON_IsObject(storage)) {
        return 0;
    }

    cJSON* data_dir_json = cJSON_GetObjectItem(storage, "data_dir");
    if (!cJSON_IsString(data_dir_json)) {
        return 0;
    }

    config->data_dir = strdup(data_dir_json->valuestring);
    if (!config->data_dir) {
        fprintf(stderr, "[ERROR] Failed to allocate memory for data_dir\n");
        return -1;
    }
    return 0;
}

static int parse_github_repositories(cJSON* json, Config* config) {
    cJSON* github = cJSON_GetObjectItem(json, "github");
    if (!cJSON_IsObject(github)) {
        return 0;
    }

    cJSON* repos_json = cJSON_GetObjectItem(github, "repositories");
    if (!repos_json) {
        return 0;
    }

    if (parse_repositories(repos_json, config) != 0) {
        fprintf(stderr, "[ERROR] Failed to parse repositories\n");
        return -1;
    }
    return 0;
}

static void print_config_info(const Config* config) {
    printf("[CONFIG] GitHub token: %s\n", config->github_token);
    printf("[CONFIG] Poll interval: %d seconds\n", config->poll_interval);
    printf("[CONFIG] Storage data dir: %s\n", 
           config->data_dir ? config->data_dir : "(not set)");
    printf("[CONFIG] Repositories count: %d\n", config->repositories_count);
    for (int i = 0; i < config->repositories_count; i++) {
        printf("[CONFIG]   [%d] %s/%s (branch: %s, enabled: %s)\n",
               i,
               config->repositories[i].owner,
               config->repositories[i].name,
               config->repositories[i].branch,
               config->repositories[i].enabled ? "yes" : "no");
    }
}

Config* load_config(const char* config_path) {
    char* file_contents = read_file(config_path);
    if (!file_contents) {
        return NULL;
    }

    cJSON* json = cJSON_Parse(file_contents);
    free(file_contents);

    if (!json) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr) {
            fprintf(stderr, "[ERROR] JSON parse error: %s\n", error_ptr);
        }
        return NULL;
    }

    Config* config = calloc(1, sizeof(Config));
    if (!config) {
        fprintf(stderr, "[ERROR] Failed to allocate memory for config\n");
        cJSON_Delete(json);
        return NULL;
    }

    config->repositories = NULL;
    config->repositories_count = 0;
    config->poll_interval = 60;
    config->data_dir = NULL;

    if (parse_github_token(json, config) != 0) {
        cJSON_Delete(json);
        free_config(config);
        return NULL;
    }

    parse_poll_interval(json, config);

    if (parse_storage_data_dir(json, config) != 0) {
        cJSON_Delete(json);
        free_config(config);
        return NULL;
    }

    if (parse_github_repositories(json, config) != 0) {
        cJSON_Delete(json);
        free_config(config);
        return NULL;
    }

    print_config_info(config);
    cJSON_Delete(json);
    return config;
}

void free_config(Config* config) {
    /*
    Освобождаем память, выделенную для конфигурации
    */
    if (config == NULL) {
        return;
    }

    free(config->github_token);

    if (config->repositories != NULL) {
        for (int i = 0; i < config->repositories_count; i++) {
            free(config->repositories[i].name);
            free(config->repositories[i].owner);
            free(config->repositories[i].branch);
        }
        free(config->repositories);
    }

    free(config->data_dir);

    free(config);
}
