cmake_minimum_required(VERSION 3.16)
idf_component_register(SRCS "src/protocol.c" "src/transport_espidf.c"
                       INCLUDE_DIRS "include" REQUIRES esp_websocket_client PRIV_REQUIRES cjson mbedtls)
