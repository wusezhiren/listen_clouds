#pragma once

#include "esp_err.h"
#include "listen_clouds/protocol.h"

esp_err_t lc_activation_request(const char *endpoint, const char *device_id,
    const char *client_id, const char *user_agent, bool confirm,
    lc_activation_t *out, int *http_status);
