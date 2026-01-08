#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include "config/config_reader.h"

#define CONFIG_PATH "config/config.json"

int main(void) {
    printf("Starting GitFlow Dashboard Daemon...%s\n", CONFIG_PATH);

    Config* config = load_config(CONFIG_PATH);

    if (config == NULL) {
        fprintf(stderr, "[ERROR] Failed to load config\n");
        return 1;
    }

    free_config(config);

    printf("GitFlow Dashboard Daemon started successfully\n");
    return 0;
}

