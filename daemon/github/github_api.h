#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Структура для хранения информации о репозитории GitHub
 * 
 * Все строковые поля (char*) выделяются динамически через strdup()
 * и должны быть освобождены через github_repository_free().
 * 
 * @note Поля name, owner, full_name, default_branch обязательны и не могут быть NULL.
 *       Поле description может быть NULL, если репозиторий не имеет описания.
 */
typedef struct {
    char* name;              ///< Название репозитория (например "Hello-World")
    char* owner;             ///< Владелец репозитория (например "octocat")
    char* full_name;         ///< Полное название в формате "owner/repo"
    char* description;       ///< Описание репозитория (может быть NULL)
    char* default_branch;    ///< Ветка по умолчанию (например "main" или "master")
    int stars;               ///< Количество звёзд (stargazers_count)
    int forks;               ///< Количество форков (forks_count)
    bool is_private;         ///< Признак приватности репозитория
} GitHubRepository;

/**
 * @brief Структура для хранения информации о Pull Request GitHub
 * 
 * Все строковые поля (char*) выделяются динамически через strdup()
 * и должны быть освобождены через github_pull_request_free().
 * 
 * @note Поля closed_at и merged_at могут быть NULL, если PR не закрыт/не смержен.
 *       Поле labels может быть NULL, если у PR нет меток.
 */
typedef struct {
    // === Идентификация ===
    int id;                  ///< Уникальный идентификатор PR
    int number;              ///< Номер PR в репозитории

    // === Основное содержимое ===
    char* title;             ///< Заголовок PR
    char* body;              ///< Тело PR (описание)
    char* state;             ///< Состояние: "open", "closed"
    bool draft;              ///< Признак черновика
    bool locked;             ///< Признак блокировки PR

    // === Автор (из user объекта) ===
    char* user_login;        ///< Логин автора PR
    char* user_avatar_url;   ///< URL аватара автора (для UI)

    // === Ветки (из head/base объектов) ===
    char* head_ref;          ///< Ветка, из которой сделан PR
    char* head_sha;          ///< SHA коммита head (для проверки статусов)
    char* base_ref;          ///< Целевая ветка (куда делается PR)

    // === Временные метки ===
    char* created_at;        ///< Дата создания (ISO 8601)
    char* updated_at;        ///< Дата последнего обновления (ISO 8601)
    char* closed_at;         ///< Дата закрытия (может быть NULL)
    char* merged_at;         ///< Дата слияния (может быть NULL)

    // === Ссылки ===
    char* html_url;          ///< URL для открытия PR в браузере

    // === Метки ===
    char** labels;           ///< Массив строк с названиями меток (может быть NULL)
    int labels_count;       ///< Количество меток
} GitHubPullRequest;


/**
 * @brief Структура для накопления данных HTTP ответа
 * 
 * Используется в callback функции write_callback() для libcurl.
 * Данные накапливаются по частям (chunks) по мере получения от сервера.
 */
typedef struct {    
    char* response;         ///< Буфер с данными ответа (выделяется через realloc)
    size_t size;            ///< Текущий размер данных в байтах
} MemoryChunk;

/**
 * @brief Инициализирует библиотеку libcurl
 * @return 0 при успехе, -1 при ошибке
 * @note Должна быть вызвана один раз при старте программы перед использованием API
 */
int github_api_init(void);

/**
 * @brief Освобождает ресурсы библиотеки libcurl
 * @note Должна быть вызвана один раз при завершении программы
 */
void github_api_cleanup(void);

/**
 * @brief Получает информацию о репозитории через GitHub API
 * @param owner Владелец репозитория (например "octocat")
 * @param repo Название репозитория (например "Hello-World")
 * @param token GitHub токен авторизации (Personal Access Token)
 * @return Указатель на GitHubRepository или NULL при ошибке
 * @note Выделенную память необходимо освободить через github_repository_free()
 */
GitHubRepository* github_get_repository(const char* owner, const char* repo, const char* token);

/**
 * @brief Освобождает память, выделенную для структуры GitHubRepository
 * @param repo Указатель на структуру (может быть NULL)
 * @note Безопасно вызывать с NULL указателем
 */
void github_repository_free(GitHubRepository* repo);

/**
 * @brief Получает информацию о Pull Request через GitHub API
 * @param owner Владелец репозитория
 * @param repo Название репозитория
 * @param token GitHub токен авторизации
 * @param pull_request_id Номер PR
 * @return Указатель на GitHubPullRequest или NULL при ошибке
 * @note Выделенную память необходимо освободить через github_pull_request_free()
 * @warning Функция пока не реализована (заглушка)
 */
GitHubPullRequest* github_get_pull_request(const char* owner, const char* repo, const char* token, int pull_request_id);

/**
 * @brief Освобождает память, выделенную для структуры GitHubPullRequest
 * @param pull_request Указатель на структуру (может быть NULL)
 * @note Безопасно вызывать с NULL указателем
 */
void github_pull_request_free(GitHubPullRequest* pull_request);