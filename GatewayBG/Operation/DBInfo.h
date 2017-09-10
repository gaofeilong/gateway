#ifndef _DB_INFO_H_
#define _DB_INFO_H_

#include <string>
using std::string;

typedef struct _ArchiveInfo {
        int    id;
        string ip;
        string configPath;
} ArchiveInfo;

#endif
