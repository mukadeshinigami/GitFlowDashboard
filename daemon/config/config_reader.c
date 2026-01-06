#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config_reader.h"


const char* get_github_token(const char* token_from_json) {
    
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

Config* load_config(const char* config_path) {
    (void)config_path;  // Пока не используется, но будет для чтения файла
    Config* config = malloc(sizeof(Config));
    if (config == NULL) {
        fprintf(stderr, "[ERROR] Failed to allocate memory for config\n");
        return NULL;
    }

    const char* token_from_json = "";
    
    const char* token_from_env = get_github_token(token_from_json);
    if (token_from_env) {
        config->github_token = strdup(token_from_env);
        if (config->github_token == NULL) {
            fprintf(stderr, "[ERROR] Failed to allocate memory for token\n");
            free(config);
            return NULL;
        }
    } else {
        fprintf(stderr, "[ERROR] Failed to get GitHub token from environment variable GITHUB_TOKEN or config.json\n");
        free(config);
        return NULL;
    }

    printf("[CONFIG] GitHub token: %s\n", config->github_token);
    return config;
}

void free_config(Config* config) {
    if (config == NULL) {
        return;
    }
    
    if (config->github_token != NULL) {
        free(config->github_token);
        config->github_token = NULL;
    }
    
    free(config);
}
