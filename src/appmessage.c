#include <pebble.h>
#include "common.h"
#include "windows/stoplist.h"
#include "windows/buslist.h"


static void in_received_handler(DictionaryIterator *iter, void *context);
static void in_dropped_handler(AppMessageResult reason, void *context);
static void out_sent_handler(DictionaryIterator *sent, void *context);
static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context);

void appmessage_init(void) {
    app_message_register_inbox_received(in_received_handler);
    app_message_register_inbox_dropped(in_dropped_handler);
    app_message_register_outbox_sent(out_sent_handler);
    app_message_register_outbox_failed(out_failed_handler);
    app_message_open(512 /* inbound_size */, 64 /* outbound_size */);
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
    Tuple *index_tuple = dict_find(iter, KEY_INDEX);
    Tuple *stopId_tuple = dict_find(iter, KEY_STOPID);
    Tuple *stopName_tuple = dict_find(iter, KEY_STOPNAME);
    Tuple *stopDistance_tuple = dict_find(iter, KEY_STOPDISTANCE);
    Tuple *busLine_tuple = dict_find(iter, KEY_BUSLINE);
    Tuple *busTimeLeft_tuple = dict_find(iter, KEY_BUSTIMELEFT);
    Tuple *busDest_tuple = dict_find(iter, KEY_BUSDEST);

    if (index_tuple && stopId_tuple && stopName_tuple && stopDistance_tuple) {
        stoplist_in_received_handler(iter);
        return;
    }

    if (index_tuple && busLine_tuple && busTimeLeft_tuple && busDest_tuple) {
        buslist_in_received_handler(iter);
        return;
    }

}

static void in_dropped_handler(AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Incoming AppMessage from Pebble dropped, %d", reason);
}

static void out_sent_handler(DictionaryIterator *sent, void *context) {
    // outgoing message was delivered
}

static void out_failed_handler(DictionaryIterator *failed, AppMessageResult reason, void *context) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Failed to send AppMessage to Pebble");
}