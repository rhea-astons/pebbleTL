#include "../common.h"

#pragma once

void stoplist_init(void);
void stoplist_deinit(void);
void stoplist_in_received_handler(DictionaryIterator *iter);