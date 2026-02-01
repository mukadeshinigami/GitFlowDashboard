#include "github_api.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

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
    /* Валидация входных параметров */
    if (endpoint == NULL || token == NULL) {
        fprintf(stderr, "github_api_request: endpoint or token is NULL\n");
        return NULL;
    }

    /* Формируем полный URL: https://api.github.com + endpoint */
    char url[512];
    int written = snprintf(url, sizeof(url), "https://api.github.com%s", endpoint);
    if (written < 0 || written >= (int)sizeof(url)) {
        fprintf(stderr, "github_api_request: URL too long\n");
        return NULL;
    }

    /* Инициализируем CURL handle */
    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        fprintf(stderr, "github_api_request: curl_easy_init() failed\n");
        return NULL;
    }

    /* Подготавливаем буфер для ответа */
    MemoryChunk chunk = {
        .response = NULL,
        .size = 0
    };

    /* Формируем заголовок Authorization */
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token);

    /* Собираем список заголовков */
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "User-Agent: GitFlowDashboard");
    headers = curl_slist_append(headers, "Accept: application/vnd.github+json");

    /* Настраиваем CURL */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);

    /* Выполняем запрос */
    CURLcode res = curl_easy_perform(curl);

    /* Проверяем результат выполнения */
    if (res != CURLE_OK) {
        fprintf(stderr, "github_api_request: curl_easy_perform() failed: %s\n", 
                curl_easy_strerror(res));
        free(chunk.response);  /* Освобождаем если что-то успело записаться */
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return NULL;
    }

    /* Проверяем HTTP код ответа */
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        fprintf(stderr, "github_api_request: HTTP %ld for %s\n", http_code, endpoint);
        free(chunk.response);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return NULL;
    }

    /* Очистка CURL ресурсов */
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    /* Возвращаем JSON строку — caller должен вызвать free() */
    return chunk.response;
}

GitHubRepository* github_get_repository(const char* owner, const char* repo, const char* token) {
    return NULL;
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