#include <pebble.h>
#include "buslist.h"
#include "stoplist.h"
#include "../common.h"
#include "../libs/pebble_assist.h"

#define  MAX_BUSES 20

static Bus buses[MAX_BUSES];

static int nb_buses = 0;

static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context);
static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context);
static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context);
static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);
static void send_request(Stop *stop);
static void window_load(Window *window);
static void refresh_list();

static Window *window;
static MenuLayer *menu_layer;
static Stop *currentStop;

void buslist_init(Stop *stop) {
    currentStop = stop;
    window = window_create();

    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load
    });

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
    send_request(currentStop);
}

void buslist_deinit(void) {
    window_destroy_safe(window);
}

void buslist_in_received_handler(DictionaryIterator *iter) {
    Tuple *index_tuple = dict_find(iter, KEY_INDEX);
    Tuple *busLine_tuple = dict_find(iter, KEY_BUSLINE);
    Tuple *busTimeLeft_tuple = dict_find(iter, KEY_BUSTIMELEFT);
    Tuple *busDest_tuple = dict_find(iter, KEY_BUSDEST);

    if (index_tuple && busLine_tuple && busTimeLeft_tuple && busDest_tuple) {
        Bus bus;
        bus.index = index_tuple->value->int16;
        strncpy(bus.line, busLine_tuple->value->cstring, sizeof(bus.line)-1);
        strncpy(bus.timeleft, busTimeLeft_tuple->value->cstring, sizeof(bus.timeleft)-1);
        strncpy(bus.dest, busDest_tuple->value->cstring, sizeof(bus.dest)-1);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "bus received in %s", bus.timeleft);
        buses[bus.index] = bus;
        nb_buses ++;
        menu_layer_reload_data_and_mark_dirty(menu_layer);
    }
}

static void refresh_list() {
    memset(buses, 0x0, sizeof(buses));
    nb_buses = 0;
    menu_layer_set_selected_index(menu_layer, (MenuIndex) { .row = 0, .section = 0 }, MenuRowAlignBottom, false);
    menu_layer_reload_data(menu_layer);
    send_request(currentStop);
    menu_layer_reload_data(menu_layer);
}

static void send_request(Stop *stop) {
    Tuplet stopId_tubple = TupletCString(KEY_STOPID, stop->id);

    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    if (iter == NULL) {
        return;
    }

    dict_write_tuplet(iter, &stopId_tubple);
    dict_write_end(iter);

    app_message_outbox_send();
}




static uint16_t menu_get_num_sections_callback(struct MenuLayer *menu_layer, void *callback_context) {
    return 1;
}

static uint16_t menu_get_num_rows_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return (nb_buses) ? nb_buses : 1;
}

static int16_t menu_get_header_height_callback(struct MenuLayer *menu_layer, uint16_t section_index, void *callback_context) {
    return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static int16_t menu_get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    return 48;
}

static void menu_draw_header_callback(GContext *ctx, const Layer *cell_layer, uint16_t section_index, void *callback_context) {
    menu_cell_basic_header_draw(ctx, cell_layer, "Buses");
}

static void menu_draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    if (nb_buses == 0) {
        menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, NULL);
    } else {
        graphics_context_set_text_color(ctx, GColorBlack);
        char destAndLine[50];
        strncpy(destAndLine, buses[cell_index->row].dest, sizeof(destAndLine)-1);
        strcat(destAndLine, " (");
        strcat(destAndLine, buses[cell_index->row].line);
        strcat(destAndLine, ")");
        graphics_draw_text(ctx, destAndLine, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), (GRect) { .origin = { 4, -4 }, .size = { PEBBLE_WIDTH - 8, 28 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
        graphics_draw_text(ctx, buses[cell_index->row].timeleft, fonts_get_system_font(FONT_KEY_GOTHIC_18), (GRect) { .origin = { 4, 24 }, .size = { 100, 20 } }, GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }
}

static void menu_select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    if (nb_buses == 0) {
        return;
    }
}

static void menu_select_long_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    refresh_list();
}

static void window_load(Window *window) {
    refresh_list();
}