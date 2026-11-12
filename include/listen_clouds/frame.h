#pragma once

#include <stddef.h>
#include <stdbool.h>

/* Reject incomplete or oversized WebSocket messages before invoking application callbacks. */
bool lc_ws_frame_complete(int data_len, int payload_len, int payload_offset, size_t limit);
