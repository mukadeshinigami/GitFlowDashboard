#include "github_api.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "../../shared/libs/cjson/cJSON.h"

/** @brief Базовый URL GitHub API v3 */
#define GITHUB_API_BASE "https://api.github.com"

/**
 * @brief Callback функция для записи HTTP ответа в память
 * 
 * Вызывается библиотекой libcurl для каждой части (chunk) полученных данных.
 * Накапливает данные в структуре MemoryChunk, динамически увеличивая буфер.
 * 
 * @param data Указатель на полученные данные от сервера
 * @param size Размер одного элемента данных
 * @param nmemb Количество элементов данных
 * @param userp Указатель на структуру MemoryChunk для накопления данных
 * @return Количество обработанных байт (realsize) или 0 при ошибке
 * @note При ошибке realloc() возвращает 0, что останавливает загрузку в libcurl
 */
static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryChunk *mem = (MemoryChunk *)userp;

    /* Реаллоцируем память для ответа (увеличиваем буфер) */
    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    
    if (ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        return 0;  /* Возвращаем 0, чтобы libcurl остановил загрузку */
    }

    mem->response = ptr;
    /* Копируем новые данные в конец существующего буфера */
    memcpy(mem->response + mem->size, data, realsize);
    /* Обновляем размер накопленных данных */
    mem->size += realsize;
    /* Добавляем нулевой символ для корректной работы со строками */
    mem->response[mem->size] = '\0';    

    return realsize;
}

/**
 * @brief Инициализирует библиотеку libcurl для работы с GitHub API
 * 
 * Выполняет глобальную инициализацию libcurl. Должна быть вызвана
 * один раз при старте программы перед любыми запросами к API.
 * 
 * @return 0 при успехе, -1 при ошибке инициализации
 * @note Должна быть вызвана перед использованием любых функций API
 * @see github_api_cleanup()
 */
int github_api_init(void) {
    CURLcode code = curl_global_init(CURL_GLOBAL_ALL);
    if (code != CURLE_OK) {
        fprintf(stderr, "libcurl global init failed: %s\n", curl_easy_strerror(code));
        return -1;
    }

    return 0;
}

/**
 * @brief Освобождает ресурсы библиотеки libcurl
 * 
 * Выполняет глобальную очистку libcurl. Должна быть вызвана
 * один раз при завершении программы после всех запросов к API.
 * 
 * @note Должна быть вызвана после завершения работы с API
 * @see github_api_init()
 */
void github_api_cleanup(void) {
    curl_global_cleanup();
}

/**
 * @brief Выполняет HTTP GET запрос к GitHub API
 * 
 * Формирует полный URL, настраивает заголовки авторизации и выполняет
 * HTTP GET запрос через libcurl. Ответ накапливается в памяти через callback.
 * 
 * @param endpoint Путь API endpoint (например "/repos/owner/repo")
 * @param token GitHub Personal Access Token для авторизации
 * @return Указатель на строку с JSON ответом или NULL при ошибке
 * @note Выделенную память необходимо освободить через free()
 * @note Проверяет HTTP статус-код (200-299 считаются успешными)
 * @note Безопасно вызывать с NULL параметрами (вернёт NULL)
 */
static char* github_api_request(const char* endpoint, const char* token) {
    /* Инициализируем структуру для накопления HTTP ответа */
    MemoryChunk chunk = {.response = NULL, .size = 0};
    
    /* Формируем полный URL из базового адреса и endpoint */
    char url[512];
    int url_len = snprintf(url, sizeof(url), "%s%s", GITHUB_API_BASE, endpoint);
    if (url_len < 0 || url_len >= (int)sizeof(url)) {
        fprintf(stderr, "[ERROR] github_api_request: URL too long\n");
        return NULL;
    }
    
    /* Создаём CURL handle для выполнения HTTP запроса */
    CURL *curl = curl_easy_init();
    if (curl == NULL) {
        fprintf(stderr, "curl_easy_init() failed\n");
        return NULL;
    }
    
    /* Формируем заголовок авторизации с токеном */
    char auth_header[256];
    int auth_len = snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", token);
    if (auth_len < 0 || auth_len >= (int)sizeof(auth_header)) {
        fprintf(stderr, "[ERROR] github_api_request: auth header too long\n");
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    /* Создаём список HTTP заголовков для запроса */
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "User-Agent: GitFlowDashboard/1.0");
    headers = curl_slist_append(headers, "Accept: application/vnd.github+json");
    /* X-GitHub-Api-Version рекомендуется для стабильности API */
    headers = curl_slist_append(headers, "X-GitHub-Api-Version: 2022-11-28");
    
    /* Настраиваем параметры CURL запроса */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &chunk);
    
    /* Выполняем HTTP GET запрос */
    CURLcode res = curl_easy_perform(curl);
    
    /* Получаем HTTP статус-код ответа */
    long http_code = 0;
    if (res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }
    
    /* Освобождаем ресурсы CURL (заголовки и handle) */
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    /* Проверяем результат выполнения запроса */
    if (res != CURLE_OK) {
        fprintf(stderr, "[ERROR] curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk.response);  /* Освобождаем частично полученные данные */
        return NULL;
    }
    
    /* Проверяем HTTP статус-код (200-299 считаются успешными) */
    if (http_code < 200 || http_code >= 300) {
        fprintf(stderr, "[ERROR] GitHub API returned HTTP %ld\n", http_code);
        free(chunk.response);
        return NULL;
    }
    
    /* Возвращаем накопленный ответ (caller должен освободить через free()) */
    return chunk.response;
}

/**
 * @brief Получает информацию о репозитории через GitHub API
 * 
 * Выполняет HTTP GET запрос к GitHub API endpoint /repos/{owner}/{repo},
 * парсит JSON ответ и создаёт структуру GitHubRepository с извлечёнными данными.
 * 
 * @param owner Владелец репозитория (например "octocat")
 * @param repo Название репозитория (например "Hello-World")
 * @param token GitHub Personal Access Token для авторизации
 * @return Указатель на GitHubRepository или NULL при ошибке
 * @note Выделенную память необходимо освободить через github_repository_free()
 * @note Безопасно вызывать с NULL параметрами (вернёт NULL)
 * @note Обязательные поля: name, owner, full_name, default_branch
 * @note Поле description может быть NULL, если репозиторий не имеет описания
 */
GitHubRepository* github_get_repository(const char* owner, const char* repo, const char* token) {
    /* Валидация входных параметров */
    if (owner == NULL || repo == NULL || token == NULL) {
        fprintf(stderr, "[ERROR] github_get_repository: invalid parameters\n");
        return NULL;
    }
    
    /* Формируем endpoint для GitHub API в формате /repos/{owner}/{repo} */
    char endpoint[256];
    int endpoint_len = snprintf(endpoint, sizeof(endpoint), "/repos/%s/%s", owner, repo);
    if (endpoint_len < 0 || endpoint_len >= (int)sizeof(endpoint)) {
        fprintf(stderr, "[ERROR] github_get_repository: endpoint too long\n");
        return NULL;
    }
    
    /* Выполняем HTTP GET запрос к GitHub API */
    char* json_response = github_api_request(endpoint, token);
    if (json_response == NULL) {
        fprintf(stderr, "[ERROR] github_get_repository: failed to fetch data\n");
        return NULL;
    }
    
    /* Парсим JSON ответ от GitHub API */
    cJSON* json = cJSON_Parse(json_response);
    free(json_response);  /* Освобождаем строку сразу после парсинга */
    
    if (json == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "[ERROR] github_get_repository: JSON parse error before: %s\n", error_ptr);
        } else {
            fprintf(stderr, "[ERROR] github_get_repository: JSON parse error\n");
        }
        return NULL;
    }
    
    /* Выделяем память для структуры GitHubRepository */
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
    
    /* Извлекаем JSON элементы для каждого поля структуры */
    cJSON* name_item = cJSON_GetObjectItemCaseSensitive(json, "name");
    cJSON* full_name_item = cJSON_GetObjectItemCaseSensitive(json, "full_name");
    cJSON* owner_obj = cJSON_GetObjectItemCaseSensitive(json, "owner");
    cJSON* description_item = cJSON_GetObjectItemCaseSensitive(json, "description");
    cJSON* default_branch_item = cJSON_GetObjectItemCaseSensitive(json, "default_branch");
    cJSON* stars_item = cJSON_GetObjectItemCaseSensitive(json, "stargazers_count");
    cJSON* forks_item = cJSON_GetObjectItemCaseSensitive(json, "forks_count");
    cJSON* private_item = cJSON_GetObjectItemCaseSensitive(json, "private");
    
    /* Извлекаем owner.login из вложенного объекта owner */
    const char* owner_login = NULL;
    if (cJSON_IsObject(owner_obj)) {
        cJSON* owner_login_item = cJSON_GetObjectItemCaseSensitive(owner_obj, "login");
        if (cJSON_IsString(owner_login_item)) {
            owner_login = cJSON_GetStringValue(owner_login_item);
        }
    }
    
    /* Копируем строковые поля в структуру (strdup выделяет память) */
    if (cJSON_IsString(name_item)) {
        repository->name = strdup(cJSON_GetStringValue(name_item));
        if (repository->name == NULL) {
            fprintf(stderr, "[ERROR] github_get_repository: strdup failed for name\n");
            github_repository_free(repository);
            cJSON_Delete(json);
            return NULL;
        }
    }
    if (owner_login != NULL) {
        repository->owner = strdup(owner_login);
        if (repository->owner == NULL) {
            fprintf(stderr, "[ERROR] github_get_repository: strdup failed for owner\n");
            github_repository_free(repository);
            cJSON_Delete(json);
            return NULL;
        }
    }
    if (cJSON_IsString(full_name_item)) {
        repository->full_name = strdup(cJSON_GetStringValue(full_name_item));
        if (repository->full_name == NULL) {
            fprintf(stderr, "[ERROR] github_get_repository: strdup failed for full_name\n");
            github_repository_free(repository);
            cJSON_Delete(json);
            return NULL;
        }
    }
    /* description может быть NULL в JSON (опциональное поле) */
    if (cJSON_IsString(description_item) && description_item->valuestring != NULL) {
        repository->description = strdup(cJSON_GetStringValue(description_item));
        if (repository->description == NULL) {
            fprintf(stderr, "[ERROR] github_get_repository: strdup failed for description\n");
            github_repository_free(repository);
            cJSON_Delete(json);
            return NULL;
        }
    } else {
        repository->description = NULL;  /* Явно устанавливаем NULL если отсутствует */
    }
    if (cJSON_IsString(default_branch_item)) {
        repository->default_branch = strdup(cJSON_GetStringValue(default_branch_item));
        if (repository->default_branch == NULL) {
            fprintf(stderr, "[ERROR] github_get_repository: strdup failed for default_branch\n");
            github_repository_free(repository);
            cJSON_Delete(json);
            return NULL;
        }
    }
    
    /* Извлекаем числовые поля из JSON */
    if (cJSON_IsNumber(stars_item)) {
        repository->stars = (int)cJSON_GetNumberValue(stars_item);
    } else {
        repository->stars = 0;  /* Значение по умолчанию */
    }
    
    if (cJSON_IsNumber(forks_item)) {
        repository->forks = (int)cJSON_GetNumberValue(forks_item);
    } else {
        repository->forks = 0;  /* Значение по умолчанию */
    }
    
    /* Извлекаем булево поле is_private */
    if (cJSON_IsBool(private_item)) {
        repository->is_private = cJSON_IsTrue(private_item);
    } else {
        repository->is_private = false;  /* Значение по умолчанию */
    }
    
    /* Валидация: проверяем наличие всех обязательных полей */
    if (repository->name == NULL || repository->owner == NULL || 
        repository->full_name == NULL || repository->default_branch == NULL) {
        fprintf(stderr, "[ERROR] github_get_repository: missing required fields in JSON\n");
        github_repository_free(repository);  /* Освобождаем частично заполненную структуру */
        cJSON_Delete(json);
        return NULL;
    }
    
    /* Освобождаем JSON объект (больше не нужен) */
    cJSON_Delete(json);
    
    return repository;
}
/**
 * @brief Освобождает память, выделенную для структуры GitHubRepository
 * 
 * Освобождает все строковые поля (выделенные через strdup()) и саму структуру.
 * Безопасно вызывать с NULL указателем.
 * 
 * @param repo Указатель на структуру GitHubRepository (может быть NULL)
 * @note Безопасно вызывать с NULL указателем
 * @note Освобождает все строковые поля и саму структуру
 */
void github_repository_free(GitHubRepository* repo) {
    if (repo == NULL) {
        return;
    }

    free(repo->name);
    free(repo->owner);
    free(repo->full_name);
    free(repo->description);
    free(repo->default_branch);

    free(repo);
}


/**
 * @brief Получает информацию о Pull Request через GitHub API
 * 
 * Выполняет HTTP GET запрос к GitHub API endpoint /repos/{owner}/{repo}/pulls/{id},
 * парсит JSON ответ и создаёт структуру GitHubPullRequest.
 * 
 * @param owner Владелец репозитория
 * @param repo Название репозитория
 * @param token GitHub Personal Access Token для авторизации
 * @param pull_request_id Номер Pull Request
 * @return Указатель на GitHubPullRequest или NULL при ошибке
 * @note Выделенную память необходимо освободить через github_pull_request_free()
 * @warning Функция пока не реализована (заглушка, возвращает NULL)
 */
GitHubPullRequest* github_get_pull_request(const char* owner, const char* repo, const char* token, int pull_request_id) {
    return NULL;
}
/**
 * @brief Освобождает память, выделенную для структуры GitHubPullRequest
 * 
 * Освобождает все строковые поля (выделенные через strdup()), массив labels
 * (если присутствует) и саму структуру. Безопасно вызывать с NULL указателем.
 * 
 * @param pull_request Указатель на структуру GitHubPullRequest (может быть NULL)
 * @note Безопасно вызывать с NULL указателем
 * @note Освобождает все строковые поля, массив labels и саму структуру
 * @note Корректно обрабатывает массив labels (освобождает каждую строку, затем массив)
 */
void github_pull_request_free(GitHubPullRequest* pull_request) {
    if (pull_request == NULL) {
        return;
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
    
    /* Освобождаем массив labels */
    if (pull_request->labels != NULL) {
        for (int i = 0; i < pull_request->labels_count; i++) {
            free(pull_request->labels[i]);
        }
        free(pull_request->labels);
    }

    free(pull_request);
}