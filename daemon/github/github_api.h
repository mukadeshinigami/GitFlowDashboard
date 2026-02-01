#include <stdbool.h>

typedef struct {
    char* name;
    char* owner;
    char* full_name;
    char* description;
    char* default_branch;
    int stars;
    int forks;
    bool is_private; // 0 - public, 1 - private
} GitHubRepository;

typedef struct {
    // === Идентификация ===
    int id;                  // "id": 1
    int number;              // "number": 1347

    // === Основное содержимое ===
    char* title;             // "title": "Amazing new feature"
    char* body;              // "body": "Please pull these..."
    char* state;             // "state": "open" | "closed"
    bool draft;              // "draft": false (добавь если есть)
    bool locked;             // "locked": true

    // === Автор (из user объекта) ===
    char* user_login;        // "user.login": "octocat"
    char* user_avatar_url;   // "user.avatar_url" (для UI)

    // === Ветки (из head/base объектов) ===
    char* head_ref;          // "head.ref": "new-topic"
    char* head_sha;          // "head.sha": "6dcb09b5..." (для статусов)
    char* base_ref;          // "base.ref": "master"

    // === Временные метки ===
    char* created_at;        // "created_at"
    char* updated_at;        // "updated_at"
    char* closed_at;         // "closed_at" (может быть NULL)
    char* merged_at;         // "merged_at" (может быть NULL)

    // === Ссылки ===
    char* html_url;          // "html_url" — для открытия в браузере

    // === Метки (опционально для MVP) ===
    char** labels;           // массив строк из labels[].name
    int labels_count;
} GitHubPullRequest;

typedef struct {

} GitHubIssue;

int github_api_init(void);

void github_api_cleanup(void);

GitHubRepository* github_get_repository(const char* owner, const char* repo, const char* token); /*
    Получает информацию о репозитории
*/
void github_repository_free(GitHubRepository* repo); /*
    Освобождает память, выделенную для репозитория
*/
GitHubPullRequest* github_get_pull_request(const char* owner, const char* repo, const char* token, int pull_request_id); /*
    Получает информацию о pull request
*/
void github_pull_request_free(GitHubPullRequest* pull_request); /*
    Освобождает память, выделенную для pull request
*/