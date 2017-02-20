//
// Created by root on 19.02.17.
//

#ifndef SERWERHTTP_CONFIGFILE_H
#define SERWERHTTP_CONFIGFILE_H

#include <libconfig.h>
#include <string>

using namespace std;

class ConfigFile {
private:
    config_t *cf;
    const char* file_name = "config.cfg";

    ConfigFile();
    ConfigFile(const ConfigFile &);

public:
    static ConfigFile & getConfigFile() {
        static ConfigFile config;
        return config;
    }
    ~ConfigFile();

    int readFCGI(string* ip, int* port);

    int readTimeout(int *timeout);

    int readRole(int *role);
};


#endif //SERWERHTTP_CONFIGFILE_H
