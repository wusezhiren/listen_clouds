#include <assert.h>
#include <stdio.h>
#include "listen_clouds/frame.h"

int main(void)
{
    assert(lc_ws_frame_complete(1024, 1024, 0, 8192));
    assert(lc_ws_frame_complete(8192, 8192, 0, 8192));
    assert(!lc_ws_frame_complete(0, 0, 0, 8192));
    assert(!lc_ws_frame_complete(-1, -1, 0, 8192));
    assert(!lc_ws_frame_complete(1024, 2048, 0, 8192));
    assert(!lc_ws_frame_complete(1024, 2048, 1024, 8192));
    assert(!lc_ws_frame_complete(8193, 8193, 0, 8192));
    puts("WebSocket frame boundary tests passed");
}
