#pragma once


typedef struct ObjectDesc ObjectDesc;


void objects_db_init();
void objects_db_destroy();
void objects_db_read();
ObjectDesc* objects_db_get(const char* id);
