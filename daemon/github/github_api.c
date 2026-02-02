#include "github_api.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../../shared/libs/cjson/cJSON.h"

/** @brief Базовый URL GitHub API v3 */
#define GITHUB_API_BASE "https://api.github.com"

static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryChunk *mem = (MemoryChunk *)userp;

    char *ptr = realloc( mem->response, mem->size + realsize + 1); 
    // Реаллоцируем память для ответа
    /*Реаллоцируем - это изменение размера памяти, выделенной для массива*/
    
    if (ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        return 0;
    }

    mem->response = ptr;
    /*Копируем данные из data в mem->response + mem->size*/
    memcpy(mem->response + mem->size, data, realsize);
    /*Увеличиваем размер mem->size на realsize*/
    mem->size += realsize;
    /*Добавляем нулевой символ в конец строки*/
    mem->response[mem->size] = '\0';    

    return realsize;
}

int github_api_init(void) {
    CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
    if (code != CURLE_OK) {
        fprintf(stderr, "libcurl global init failed: %s\n", curl_easy_strerror(code));
        return -1;
    }

    return 0;
}

void github_api_cleanup(void) {
    curl_global_cleanup();
}

/**
 * @brief Выполняет GET запрос к GitHub API
 * @param endpoint Путь API (например "/repos/owner/repo")
 * @param token GitHub токен авторизации
 * @return Строка с JSON ответом (caller освобождает через free()) или NULL при ошибке
 */
static char* github_api_request(const char* endpoint, const char* token) {
    /* Инициализируем структуру для хранения ответа */
    MemoryChunk chunk = {.response = NULL, .size = 0};
    
    /* Формируем полный URL */
    char url[512];
    snprintf(url, sizeof(url), "%s%s", GITHUB_API_BASE, endpoint); /**/
    
    /* Создаём CURL handle */
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        fprintf(stderr, "curl_easy_init() failed\n");
        return NULL;
    }
    
    /* Формируем заголовки */
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token);
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "User-Agent: GitFlowDashboard/1.0");
    headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
    /* X-GitHub-Api-Version рекомендуется для стабильности API */
    headers = curl_slist_append(headers, "X-GitHub-Api-Version: 2022-11-28");
    
    /* Настраиваем CURL */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    
    /* Выполняем запрос */
    CURLcode res = curl_easy_perform(curl);
    
    /* Освобождаем ресурсы CURL */
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    /* Проверяем результат */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk.response);  /* Освобождаем частично полученные данные */
        return NULL;
    }
    
    return chunk.response;  /* Caller освобождает через free() */
}

/**
 * @brief Получает информацию о репозитории через GitHub API
 * @param owner Владелец репозитория (например "octocat")
 * @param repo Название репозитория (например "Hello-World")
 * @param token GitHub токен авторизации
 * @return Указатель на GitHubRepository (освобождать через github_repository_free) или NULL при ошибке
 */
GitHubRepository* github_get_repository(const char* owner, const char* repo, const char* token) {
    /* Проверяем входные параметры */
    if (owner == NULL || repo == NULL || token == NULL) {
        fprintf(stderr, "[ERROR] github_get_repository: invalid parameters\n");
        return NULL;
    }
    
    /* Формируем endpoint для GitHub API */
    char endpoint[256];
    snprintf(endpoint, sizeof(endpoint), "/repos/%s/%s", owner, repo);
    
    /* Выполняем HTTP запрос */
    char* json_response = github_api_request(endpoint, token);
    if (json_response == NULL) {
        fprintf(stderr, "[ERROR] github_get_repository: failed to fetch data\n");
        return NULL;
    }
    
    /* Парсим JSON ответ */
    cJSON* json = cJSON_Parse(json_response);
    free(json_response);  /* Освобождаем строку сразу после парсинга */
    
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "[ERROR] github_get_repository: JSON parse error before: %s\n", error_ptr);
        }
        return NULL;
    }
    
    /* Выделяем память для структуры */
    GitHubRepository* repository = malloc(sizeof(GitHubRepository));
    if (repository == NULL) {
        fprintf(stderr, "[ERROR] github_get_repository: failed to allocate memory\n");
        cJSON_Delete(json);
        return NULL;
    }
    
    /* Инициализируем все указатели NULL для безопасного освобождения при ошибке */
    repository->name = NULL;
    repository->owner = NULL;
    repository->full_name = NULL;
    repository->description = NULL;
    repository->default_branch = NULL;
    
    /* Извлекаем поля из JSON */
    cJSON* name_item = cJSON_GetObjectItemCaseSensitive(json, "name");
    cJSON* full_name_item = cJSON_GetObjectItemCaseSensitive(json, "full_name");
    cJSON* owner_obj = cJSON_GetObjectItemCaseSensitive(json, "owner");
    cJSON* description_item = cJSON_GetObjectItemCaseSensitive(json, "description");
    cJSON* default_branch_item = cJSON_GetObjectItemCaseSensitive(json, "default_branch");
    cJSON* stars_item = cJSON_GetObjectItemCaseSensitive(json, "stargazers_count");
    cJSON* forks_item = cJSON_GetObjectItemCaseSensitive(json, "forks_count");
    cJSON* private_item = cJSON_GetObjectItemCaseSensitive(json, "private");
    
    /* Извлекаем owner.login из вложенного объекта */
    const char* owner_login = NULL;
    if (cJSON_IsObject(owner_obj)) {
        cJSON* owner_login_item = cJSON_GetObjectItemCaseSensitive(owner_obj, "login");
        if (cJSON_IsString(owner_login_item)) {
            owner_login = cJSON_GetStringValue(owner_login_item);
        }
    }
    
    /* Копируем строковые поля (используем strdup для выделения памяти) */
    if (cJSON_IsString(name_item)) {
        repository->name = strdup(cJSON_GetStringValue(name_item));
    }
    if (owner_login != NULL) {
        repository->owner = strdup(owner_login);
    }
    if (cJSON_IsString(full_name_item)) {
        repository->full_name = strdup(cJSON_GetStringValue(full_name_item));
    }
    /* description может быть NULL в JSON */
    if (cJSON_IsString(description_item) && description_item->valuestring != NULL) {
        repository->description = strdup(cJSON_GetStringValue(description_item));
    } else {
        repository->description = NULL;  /* Явно устанавливаем NULL если отсутствует */
    }
    if (cJSON_IsString(default_branch_item)) {
        repository->default_branch = strdup(cJSON_GetStringValue(default_branch_item));
    }
    
    /* Извлекаем числовые поля */
    if (cJSON_IsNumber(stars_item)) {
        repository->stars = (int)cJSON_GetNumberValue(stars_item);
    } else {
        repository->stars = 0;
    }
    
    if (cJSON_IsNumber(forks_item)) {
        repository->forks = (int)cJSON_GetNumberValue(forks_item);
    } else {
        repository->forks = 0;
    }
    
    /* Извлекаем булево поле */
    if (cJSON_IsBool(private_item)) {
        repository->is_private = cJSON_IsTrue(private_item);
    } else {
        repository->is_private = false;
    }
    
    /* Проверяем, что обязательные поля были успешно извлечены */
    if (repository->name == NULL || repository->owner == NULL || 
        repository->full_name == NULL || repository->default_branch == NULL) {
        fprintf(stderr, "[ERROR] github_get_repository: missing required fields in JSON\n");
        github_repository_free(repository);  /* Освобождаем частично заполненную структуру */
        cJSON_Delete(json);
        return NULL;
    }
    
    /* Освобождаем JSON объект */
    cJSON_Delete(json);
    
    return repository;
}
void github_repository_free(GitHubRepository* repo) {

    if(repo == NULL) {return;
    }

    free(repo->name);
    free(repo->owner);
    free(repo->full_name);
    free(repo->description);
    free(repo->default_branch);

    free(repo);
}


GitHubPullRequest* github_get_pull_request(const char* owner, const char* repo, const char* token, int pull_request_id) {
    return NULL;
}
void github_pull_request_free(GitHubPullRequest* pull_request) {
    if(pull_request == NULL) { return; 
    }

    free(pull_request->title);
    free(pull_request->body);
    free(pull_request->state);
    free(pull_request->user_login);
    free(pull_request->user_avatar_url);
    free(pull_request->head_ref);
    free(pull_request->head_sha);
    free(pull_request->base_ref);
    free(pull_request->created_at);
    free(pull_request->updated_at);
    free(pull_request->closed_at);
    free(pull_request->merged_at);
    free(pull_request->html_url);
    free(pull_request->labels);

    free(pull_request);
}