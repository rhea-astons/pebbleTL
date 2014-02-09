#include <pebble.h>
#include "stoplist.h"
#include "buslist.h"
#include "../common.h"
#include "../libs/pebble_assist.h"

#define  MAX_STOPS 20

static Stop stops[MAX_STOPS];

static int nb_stops = 0;

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void send_request();
static void refresh_list();

static Window *window;
static MenuLayer *menu_layer;


void stoplist_init(void) {
    window = window_create();

    menu_layer = menu_layer_create_fullscreen(window);
    menu_layer_set_callbacks(menu_layer, NULL, (MenuLayerCallbacks) {
        .get_num_sections = menu_get_num_sections_callback,
        .get_num_rows = menu_get_num_rows_callback,
        .get_header_height = menu_get_header_height_callback,
        .get_cell_height = menu_get_cell_height_callback,
        .draw_header = menu_draw_header_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
        .select_long_click = menu_select_long_callback,
    });
    menu_layer_set_click_config_onto_window(menu_layer, window);
    menu_layer_add_to_window(menu_layer, window);

    window_stack_push(window, true);
}

static void refresh_list() {
    memset(stops, 0x0, sizeof(stops));
    nb_stops = 0;
    menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
    menu_layer_reload_data(menu_layer);
    send_request();
    menu_layer_reload_data(menu_layer);
}

static void send_request() {
    Tuplet index_tuple = TupletInteger(KEY_INDEX, 0);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
    if (iter == NULL) {
        return;
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "refreshing");
    dict_write_tuplet(iter, &index_tuple);
    dict_write_end(iter);

    app_message_outbox_send();
}

void stoplist_deinit(void) {
    window_destroy_safe(window);
}


void stoplist_in_received_handler(DictionaryIterator *iter) {
    Tuple *index_tuple = dict_find(iter, KEY_INDEX);
    Tuple *stopName_tuple = dict_find(iter, KEY_STOPNAME);
    Tuple *stopID_tuple = dict_find(iter, KEY_STOPID);
    Tuple *stopDistance_tuple = dict_find(iter, KEY_STOPDISTANCE);

    if (index_tuple && stopName_tuple && stopID_tuple && stopDistance_tuple) {
        Stop stop;
        stop.index = index_tuple->value->int16;
        strncpy(stop.id, stopID_tuple->value->cstring, sizeof(stop.id) - 1);
        strncpy(stop.name, stopName_tuple->value->cstring, sizeof(stop.name) - 1);
        strncpy(stop.distance, stopDistance_tuple->value->cstring, sizeof(stop.distance) - 1);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "stop received dist: %s)", stop.distance);
        stops[stop.index] = stop;
        nb_stops++;
        menu_layer_reload_data_and_mark_dirty(menu_layer);
    }
}

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
    return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return (nb_stops) ? nb_stops : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return 48;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
    menu_cell_basic_header_draw(ctx, cell_layer, "Stops");
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    if (nb_stops == 0) {
        menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
    } else {
        graphics_context_set_text_color(ctx, GColorBlack);
        graphics_draw_text(ctx, stops[cell_index->row].name, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), (GRect) { .origin = { 4, -4 }, .size = { PEBBLE_WIDTH - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
        graphics_draw_text(ctx, stops[cell_index->row].distance, fonts_get_system_font(FONT_KEY_GOTHIC_18), (GRect) { .origin = { 4, 24 }, .size = { 100, 20 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    if (nb_stops == 0) {
        return;
    }
    buslist_init(&stops[cell_index->row]);
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    refresh_list();
}