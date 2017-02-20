//
// Created by root on 19.02.17.
//

#include "ConfigFile.h"
#include <unistd.h>

ConfigFile::ConfigFile() {
//    char cwd[1024];
//    if (getcwd(cwd, sizeof(cwd)) != NULL)
//        fprintf(stdout, "Current working dir: %s\n", cwd);
//    else {
//        perror("getcwd() error");
//        return;
//    }
    // TODO usunąć komentarze
    cf = (config_t*)malloc(sizeof(config_t));
    config_init(cf);
    if (!config_read_file(cf, file_name)) {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(cf), config_error_line(cf), config_error_text(cf));
        config_destroy(cf);
        perror("Error constructing config file!");
        return;
    }
}

ConfigFile::~ConfigFile() {
    free(cf);
    config_destroy(cf);
}

int ConfigFile::readFCGI(const char *ip, int* port) {

    if (config_lookup_string(cf, "FCGI.ip", &ip) != 1) {
        perror("Error reading ip from config file");
        return -1;
    };
    if (config_lookup_int(cf, "FCGI.port", port) != 1) {
        perror("Error reading port from config file");
        return -1;
    };
    return 0;
}

int ConfigFile::readTimeout(int* timeout) {
    if (config_lookup_int(cf, "timeout", timeout) != 1) {
        perror("Error reading timeout from config file");
        return -1;
    };
    return 0;
}

int ConfigFile::readRole(int* role) {
    if (config_lookup_int(cf, "FCGI.role", role) != 1) {
        perror("Error reading role from config file");
        return -1;
    };
    return 0;
}

