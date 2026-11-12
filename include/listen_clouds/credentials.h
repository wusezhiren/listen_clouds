#pragma once

#include "esp_err.h"

typedef struct {
    char url[256];
    char token[192];
    char client_id[37];
} lc_credentials_t;

/* The namespace and keys match deployed gateways, preserving NVS across migration. */
esp_err_t lc_credentials_load(lc_credentials_t *out);
esp_err_t lc_credentials_save(lc_credentials_t *credentials, const char *url, const char *token);
