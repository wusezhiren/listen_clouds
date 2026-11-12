# listen_clouds

Portable cloud voice protocol and transport adapters. The first adapter implements the Xiaozhi WebSocket v1 JSON messages used by the ESP32 gateway; the application owns local audio, session scheduling and MCP tools.

The protocol encoder uses C11 and has no RTOS or board dependency. Run `CJSON_SOURCE_DIR=/path/to/cJSON tests/test_host.sh` for host tests. The script also detects the cJSON source from the sibling gateway checkout. ESP-IDF TLS/WebSocket, activation and credential storage are the next adapters; no credentials belong in this repository.
