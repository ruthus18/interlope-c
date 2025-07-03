#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toml.h"

#include "config.h"
#include "core/log.h"



_Config Config;
toml_table_t* toml_conf = NULL;

static
void _read_int(char* table, char* key, int* dest) {
    toml_table_t* t = toml_table_in(toml_conf, table);
    toml_datum_t datum = toml_int_in(t, key);
    if (!datum.ok)
        log_exit("Unable to read config value: %s.%s (int)", table, key);
    
    *dest = (int)datum.u.i;
}

static
void _read_bool(char* table, char* key, bool* dest) {
    toml_table_t* t = toml_table_in(toml_conf, table);
    toml_datum_t datum = toml_bool_in(t, key);
    if (!datum.ok)
        log_exit("Unable to read config value: %s.%s (bool)", table, key);

    *dest = datum.u.b;
}

static
void _read_double(char* table, char* key, double* dest) {
    toml_table_t* t = toml_table_in(toml_conf, table);
    toml_datum_t datum = toml_double_in(t, key);
    if (!datum.ok)
        log_exit("Unable to read config value: %s.%s (dobule)", table, key);
    
    *dest = datum.u.d;
}

static
void _read_string(char* table, char* key, char* dest) {
    toml_table_t* t = toml_table_in(toml_conf, table);
    toml_datum_t datum = toml_string_in(t, key);
    if (!datum.ok)
        log_exit("Unable to read config value: %s.%s (string)", table, key);
    
    strcpy(dest, datum.u.s);
}

void config_load(const char* config_path) {
    FILE* fp = fopen(config_path, "r");
    if (!fp)  log_exit("Unable to open config file: %s", config_path);
    
    char errbuf[200];
    toml_conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);
    
    if (!toml_conf)  log_exit("Unable to parse config file: %s", errbuf);

    _read_string("window", "title", Config.WINDOW_TITLE);
    _read_int("window", "width", &Config.WINDOW_WIDTH);
    _read_int("window", "height", &Config.WINDOW_HEIGHT);
    _read_bool("window", "fullsc", &Config.WINDOW_FULLSC);
    _read_int("window", "xpos", &Config.WINDOW_XPOS);
    _read_int("window", "ypos", &Config.WINDOW_YPOS);
    _read_bool("window", "border", &Config.WINDOW_BORDER);
    _read_bool("window", "vsync", &Config.WINDOW_VSYNC);
    _read_double("window", "max_framerate", &Config.WINDOW_MAX_FRAMERATE);

    _read_bool("graphics", "wireframe", &Config.GRAPHICS_WIREFRAME);

    _read_string("path", "shaders", Config.DIR_SHADERS);
    _read_string("path", "meshes", Config.DIR_MESHES);
    _read_string("path", "textures", Config.DIR_TEXTURES);
    _read_string("path", "objects_data", Config.PATH_OBJECTS_DATA);
    _read_string("path", "scenes_data", Config.PATH_SCENES_DATA);

    toml_free(toml_conf);
}