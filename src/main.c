#include <pebble.h>
#include "appmessage.h"
#include "windows/stoplist.h"


static void init() {
    appmessage_init();
    stoplist_init();
}

static void deinit() {
    stoplist_deinit();
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}