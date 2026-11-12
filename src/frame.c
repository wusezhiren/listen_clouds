#include "listen_clouds/frame.h"

bool lc_ws_frame_complete(int data_len, int payload_len, int payload_offset, size_t limit)
{
    return data_len > 0 && payload_offset == 0 && payload_len == data_len &&
           (size_t)data_len <= limit;
}
