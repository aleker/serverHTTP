//
// Created by root on 19.02.17.
//

#ifndef SERWERHTTP_CONFIGFILE_H
#define SERWERHTTP_CONFIGFILE_H

#include <libconfig.h>
#include <string>

using namespace std;

class ConfigFile {
public:
    ConfigFile(const char *file_name);
    ~ConfigFile();

    int readFCGI(const char *ip, int port);

private:
    config_t *cf;
    const char* file_name;

};


#endif //SERWERHTTP_CONFIGFILE_H
