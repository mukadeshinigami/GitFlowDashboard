#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "github_api.h"
#include "../../shared/libs/cjson/cJSON.h"

/*
 * Структура для хранения данных ответа от curl
 */
typedef struct {
    char* data;
    size_t size;
} CurlResponse;

/*
 * Callback функция для записи данных от curl
 */
static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    CurlResponse* response = (CurlResponse*)userp;

    char* ptr = realloc(response->data, response->size + realsize + 1);
    if (ptr == NULL) {
        fprintf(stderr, "[GITHUB] Failed to allocate memory for response\n");
        return 0;
    }

    response->data = ptr;
    memcpy(&(response->data[response->size]), contents, realsize);
    response->size += realsize;
    response->data[response->size] = '\0';

    return realsize;
}

int github_api_init(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    return 0;
}

void github_api_cleanup(void) {
    curl_global_cleanup();
}

char* github_api_get(const char* url, const char* token) {
    if (url == NULL) {
        fprintf(stderr, "[GITHUB] URL is NULL\n");
        return NULL;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "[GITHUB] Failed to initialize curl\n");
        return NULL;
    }

    CurlResponse response = {0};
    response.data = malloc(1);
    response.size = 0;

    if (response.data == NULL) {
        fprintf(stderr, "[GITHUB] Failed to allocate memory for response\n");
        curl_easy_cleanup(curl);
        return NULL;
    }

    // Устанавливаем URL
    curl_easy_setopt(curl, CURLOPT_URL, url);

    // Устанавливаем callback для записи данных
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

    // Устанавливаем User-Agent (обязательно для GitHub API)
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "GitFlowDashboard/1.0");

    // Добавляем токен авторизации, если он есть
    struct curl_slist* headers = NULL;
    if (token != NULL && strlen(token) > 0) {
        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token);
        headers = curl_slist_append(headers, auth_header);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    // Выполняем запрос
    CURLcode res = curl_easy_perform(curl);

    // Проверяем HTTP код
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    // Освобождаем заголовки
    if (headers != NULL) {
        curl_slist_free_all(headers);
    }

    if (res != CURLE_OK) {
        fprintf(stderr, "[GITHUB] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(response.data);
        curl_easy_cleanup(curl);
        return NULL;
    }

    if (http_code != 200) {
        fprintf(stderr, "[GITHUB] HTTP error: %ld\n", http_code);
        free(response.data);
        curl_easy_cleanup(curl);
        return NULL;
    }

    curl_easy_cleanup(curl);

    printf("[GITHUB] Successfully fetched data from: %s\n", url);
    return response.data;
}

GitHubRepository* github_get_repository(const char* owner, const char* repo, const char* token) {
    if (owner == NULL || repo == NULL) {
        fprintf(stderr, "[GITHUB] owner or repo is NULL\n");
        return NULL;
    }

    // Формируем URL
    char url[512];
    snprintf(url, sizeof(url), "https://api.github.com/repos/%s/%s", owner, repo);

    // Выполняем запрос
    char* json_response = github_api_get(url, token);
    if (json_response == NULL) {
        return NULL;
    }

    // Парсим JSON
    cJSON* json = cJSON_Parse(json_response);
    free(json_response);

    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "[GITHUB] JSON parse error: %s\n", error_ptr);
        }
        return NULL;
    }

    // Выделяем память для структуры
    GitHubRepository* repository = calloc(1, sizeof(GitHubRepository));
    if (repository == NULL) {
        fprintf(stderr, "[GITHUB] Failed to allocate memory for repository\n");
        cJSON_Delete(json);
        return NULL;
    }

    // Парсим поля JSON
    cJSON* name = cJSON_GetObjectItemCaseSensitive(json, "name");
    cJSON* full_name = cJSON_GetObjectItemCaseSensitive(json, "full_name");
    cJSON* description = cJSON_GetObjectItemCaseSensitive(json, "description");
    cJSON* owner_json = cJSON_GetObjectItemCaseSensitive(json, "owner");
    cJSON* default_branch = cJSON_GetObjectItemCaseSensitive(json, "default_branch");
    cJSON* stargazers_count = cJSON_GetObjectItemCaseSensitive(json, "stargazers_count");
    cJSON* forks_count = cJSON_GetObjectItemCaseSensitive(json, "forks_count");
    cJSON* is_private = cJSON_GetObjectItemCaseSensitive(json, "private");

    if (cJSON_IsString(name)) {
        repository->name = strdup(name->valuestring);
    }
    if (cJSON_IsString(full_name)) {
        repository->full_name = strdup(full_name->valuestring);
    }
    if (cJSON_IsString(description) && description->valuestring != NULL) {
        repository->description = strdup(description->valuestring);
    }
    if (cJSON_IsObject(owner_json)) {
        cJSON* owner_login = cJSON_GetObjectItemCaseSensitive(owner_json, "login");
        if (cJSON_IsString(owner_login)) {
            repository->owner = strdup(owner_login->valuestring);
        }
    }
    if (cJSON_IsString(default_branch)) {
        repository->default_branch = strdup(default_branch->valuestring);
    }
    if (cJSON_IsNumber(stargazers_count)) {
        repository->stars = stargazers_count->valueint;
    }
    if (cJSON_IsNumber(forks_count)) {
        repository->forks = forks_count->valueint;
    }
    if (cJSON_IsBool(is_private)) {
        repository->is_private = cJSON_IsTrue(is_private);
    }

    cJSON_Delete(json);

    return repository;
}

void github_repository_free(GitHubRepository* repo) {
    if (repo == NULL) {
        return;
    }

    free(repo->name);
    free(repo->full_name);
    free(repo->description);
    free(repo->owner);
    free(repo->default_branch);
    free(repo);
}

