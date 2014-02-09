#include "../common.h"

#pragma once

void buslist_init(Stop *stop);
void buslist_deinit(void);
void buslist_in_received_handler(DictionaryIterator *iter);