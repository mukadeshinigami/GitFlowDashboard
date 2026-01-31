# 🔄 Потоки данных в GitFlow Dashboard

## Общая схема работы программы

```
┌─────────────────────────────────────────────────────────────┐
│                        main.c                               │
│                                                             │
│  1. Инициализация                                           │
│     └─> github_api_init()                                   │
│                                                             │
│  2. Загрузка конфигурации                                   │
│     └─> load_config("config/config.json")                    │
│         ├─> read_file() ──> читает файл                     │
│         ├─> cJSON_Parse() ──> парсит JSON                    │
│         ├─> parse_github_token() ──> получает токен          │
│         └─> parse_repositories() ──> парсит репозитории     │
│                                                             │
│  3. Запрос к GitHub API                                      │
│     └─> github_get_repository(owner, repo, token)           │
│         ├─> github_api_get(url, token)                      │
│         │   ├─> curl_easy_init()                            │
│         │   ├─> curl_easy_setopt() ──> настройка            │
│         │   ├─> curl_easy_perform() ──> HTTP запрос         │
│         │   └─> write_callback() ──> накопление ответа       │
│         ├─> cJSON_Parse() ──> парсинг JSON                  │
│         └─> заполнение структуры GitHubRepository           │
│                                                             │
│  4. Очистка                                                 │
│     ├─> free_config()                                       │
│     └─> github_api_cleanup()                                │
└─────────────────────────────────────────────────────────────┘
```

## Детальный поток: Загрузка конфигурации

```
config.json (файл)
    │
    ▼
read_file()
    │
    ├─> fopen("config.json", "r")
    ├─> fseek(SEEK_END) ──> узнаем размер
    ├─> malloc(size + 1) ──> выделяем память
    ├─> fread() ──> читаем содержимое
    └─> возвращает char* (JSON строка)
    │
    ▼
cJSON_Parse()
    │
    └─> возвращает cJSON* (дерево объектов)
    │
    ▼
parse_github_token()
    │
    ├─> getenv("GITHUB_TOKEN") ──> проверяем env
    ├─> если нет в env, берем из JSON
    └─> strdup(token) ──> копируем в Config
    │
    ▼
parse_repositories()
    │
    ├─> cJSON_GetArraySize() ──> количество репозиториев
    ├─> malloc(sizeof(Repository) * count) ──> массив структур
    ├─> для каждого репозитория:
    │   ├─> cJSON_GetObjectItem("name")
    │   ├─> strdup(name->valuestring) ──> копируем строку
    │   └─> аналогично для owner, branch, enabled
    └─> возвращает Config*
```

## Детальный поток: Запрос к GitHub API

```
github_get_repository(owner, repo, token)
    │
    ├─> snprintf() ──> формируем URL
    │   "https://api.github.com/repos/{owner}/{repo}"
    │
    ▼
github_api_get(url, token)
    │
    ├─> curl_easy_init() ──> создаем curl handle
    │
    ├─> malloc(1) ──> начальный буфер для ответа
    │
    ├─> curl_easy_setopt(CURLOPT_URL, url)
    ├─> curl_easy_setopt(CURLOPT_WRITEFUNCTION, write_callback)
    ├─> curl_easy_setopt(CURLOPT_WRITEDATA, &response)
    │
    ├─> если token есть:
    │   └─> curl_slist_append("Authorization: Bearer {token}")
    │
    ├─> curl_easy_perform() ──> выполнение HTTP запроса
    │   │
    │   └─> write_callback() вызывается многократно:
    │       ├─> realloc() ──> увеличиваем буфер
    │       ├─> memcpy() ──> копируем данные
    │       └─> накапливаем весь ответ
    │
    ├─> curl_easy_getinfo(CURLINFO_RESPONSE_CODE) ──> проверяем HTTP код
    │
    └─> возвращает char* (JSON строка)
    │
    ▼
cJSON_Parse(json_string)
    │
    └─> возвращает cJSON* (дерево объектов)
    │
    ▼
calloc(1, sizeof(GitHubRepository))
    │
    └─> выделяем и обнуляем структуру
    │
    ▼
Извлечение полей из JSON:
    │
    ├─> cJSON_GetObjectItem("name")
    ├─> strdup(name->valuestring) ──> repository->name
    ├─> cJSON_GetObjectItem("stargazers_count")
    ├─> valueint ──> repository->stars
    └─> аналогично для других полей
    │
    ▼
cJSON_Delete(json) ──> освобождаем JSON объект
    │
    └─> возвращаем GitHubRepository*
```

## Управление памятью: Жизненный цикл Config

```
┌─────────────────────────────────────────────────────────┐
│ 1. Выделение памяти                                     │
│    Config* config = calloc(1, sizeof(Config))          │
│    └─> выделяет структуру, обнуляет поля               │
└─────────────────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────┐
│ 2. Заполнение полей                                     │
│    config->github_token = strdup(token)                │
│    └─> выделяет память для строки                       │
│                                                         │
│    config->repositories = malloc(sizeof(Repository)*N) │
│    └─> выделяет массив структур                        │
│                                                         │
│    for каждого репозитория:                             │
│        repo[i].name = strdup(name)                     │
│        └─> выделяет память для каждой строки           │
└─────────────────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────┐
│ 3. Использование                                        │
│    printf("%s\n", config->github_token)                │
│    printf("%s/%s\n", repo->owner, repo->name)          │
└─────────────────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────┐
│ 4. Освобождение памяти (free_config)                    │
│                                                         │
│    free(config->github_token)  ← строка                │
│                                                         │
│    for каждого репозитория:                             │
│        free(repo[i].name)      ← строки                │
│        free(repo[i].owner)                              │
│        free(repo[i].branch)                             │
│                                                         │
│    free(config->repositories)  ← массив                │
│    free(config->data_dir)      ← строка                │
│    free(config)                ← структура             │
└─────────────────────────────────────────────────────────┘
```

## Управление памятью: Жизненный цикл GitHubRepository

```
┌─────────────────────────────────────────────────────────┐
│ 1. HTTP запрос                                          │
│    char* json_response = github_api_get(url, token)    │
│    └─> выделяет память внутри curl callback             │
└─────────────────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────┐
│ 2. Парсинг JSON                                         │
│    cJSON* json = cJSON_Parse(json_response)            │
│    free(json_response)  ← освобождаем исходную строку   │
└─────────────────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────┐
│ 3. Создание структуры                                   │
│    GitHubRepository* repo = calloc(1, sizeof(...))     │
│    └─> выделяет и обнуляет структуру                    │
└─────────────────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────┐
│ 4. Заполнение полей                                     │
│    repo->name = strdup(name->valuestring)               │
│    repo->full_name = strdup(full_name->valuestring)     │
│    └─> выделяет память для каждой строки               │
└─────────────────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────┐
│ 5. Использование                                        │
│    printf("Name: %s\n", repo->name)                     │
│    printf("Stars: %d\n", repo->stars)                   │
└─────────────────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────────────────┐
│ 6. Освобождение (github_repository_free)                 │
│                                                         │
│    free(repo->name)         ← строка                    │
│    free(repo->full_name)    ← строка                    │
│    free(repo->description)  ← строка                    │
│    free(repo->owner)        ← строка                    │
│    free(repo->default_branch) ← строка                  │
│    free(repo)                ← структура                │
│                                                         │
│    cJSON_Delete(json)       ← JSON объект (если еще не) │
└─────────────────────────────────────────────────────────┘
```

## Поток данных: curl callback

```
HTTP запрос выполняется
    │
    ▼
libcurl получает данные частями (chunks)
    │
    ├─> chunk 1: "{\"name\":\"GitFlowDashboard\""
    ├─> chunk 2: ",\"owner\":\"mukadeshinigami\""
    └─> chunk 3: ",\"stars\":0}"
    │
    ▼
Для каждого chunk вызывается write_callback()
    │
    ├─> realloc(response->data, old_size + chunk_size + 1)
    │   └─> увеличиваем буфер
    │
    ├─> memcpy(&response->data[response->size], chunk, chunk_size)
    │   └─> копируем данные в конец буфера
    │
    └─> response->size += chunk_size
        └─> обновляем размер
    │
    ▼
После всех chunks:
    response->data = "{\"name\":\"GitFlowDashboard\",\"owner\":\"mukadeshinigami\",\"stars\":0}\0"
    response->size = 58
    │
    └─> возвращаем полный JSON как строку
```

## Схема проверок и обработки ошибок

```
┌─────────────────────────────────────────────────────────┐
│ Проверка на каждом этапе                                │
└─────────────────────────────────────────────────────────┘
                    │
        ┌────────────┴────────────┐
        │                         │
        ▼                         ▼
┌───────────────┐        ┌──────────────────┐
│ Успех        │        │ Ошибка           │
│              │        │                  │
│ Продолжаем   │        │ Очистка ресурсов │
│              │        │                  │
│              │        │ Возврат NULL/-1  │
└───────────────┘        └──────────────────┘

Примеры проверок:
├─> if (file == NULL) → ошибка открытия
├─> if (ptr == NULL) → ошибка выделения памяти
├─> if (json == NULL) → ошибка парсинга JSON
├─> if (http_code != 200) → ошибка HTTP
└─> if (config == NULL) → ошибка загрузки конфига
```

## Визуализация структур данных в памяти

```
Config в памяти:
┌─────────────────────────────────────────┐
│ Config* config                          │
│ ┌─────────────────────────────────────┐ │
│ │ github_token ──┐                    │ │
│ │ repositories ──┼─┐                  │ │
│ │ count: 2       │ │                  │ │
│ │ poll_interval: 60                   │ │
│ │ data_dir ──┐   │ │                  │ │
│ └────────────┼───┼─┘                  │ │
│              │   │                    │ │
│              ▼   ▼                    │ │
│         ┌────┐ ┌──────────────────┐  │ │
│         │"gh_│ │ Repository[0]    │  │ │
│         │tok │ │ name ──┐         │  │ │
│         │en" │ │ owner ─┼─┐       │  │ │
│         └────┘ │ branch─┼─┼─┐     │  │ │
│                │ enabled: true    │  │ │
│                └─────────┼─┼─┘     │  │ │
│                          │ │ │     │  │ │
│                          ▼ ▼ ▼     │  │ │
│                    ┌────┐┌──┐┌───┐ │  │ │
│                    │"Git││"m││"ma│ │  │ │
│                    │Flow││uk││in"│ │  │ │
│                    │Dash││ad│└───┘ │  │ │
│                    │boar││es│      │  │ │
│                    │d"  ││hi│      │  │ │
│                    └────┘└──┘      │  │ │
│                                    │  │ │
│                    Repository[1]   │  │ │
│                    ...             │  │ │
└────────────────────────────────────┴──┴─┘
```

---

## Ключевые моменты для понимания

1. **Все данные в куче (heap)** — используем `malloc`/`calloc`/`strdup`
2. **Указатели связывают данные** — `Config.repositories` указывает на массив
3. **Строки копируются** — `strdup()` создает новую копию
4. **Освобождение в обратном порядке** — сначала поля, потом структура
5. **Проверки на каждом шаге** — защита от ошибок

---

**Используйте эту схему для понимания потоков данных в коде!** 🎯




