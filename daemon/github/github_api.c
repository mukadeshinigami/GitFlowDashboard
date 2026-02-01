#include "github_api.h"
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

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

GitHubRepository* github_get_repository(const char* owner, const char* repo, const char* token) {
    return 0;
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