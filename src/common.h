#pragma once

typedef struct {
    int index;
    char id[25];
    char name[50];
    char distance[10];
} Stop;

typedef struct {
    int index;
    char line[5];
    char timeleft[10];
    char dest[50];
} Bus;

enum {
    KEY_INDEX,
    KEY_STOPID,
    KEY_STOPNAME,
    KEY_STOPDISTANCE,
    KEY_BUSLINE,
    KEY_BUSTIMELEFT,
    KEY_BUSDEST
};