#ifndef _THREAD_PARAM_H_
#define _THREAD_PARAM_H_

#include <map>
#include <string>
#include "Archive.h"

using std::map;
using std::string;

typedef struct _DevThreadParam {
        _DevThreadParam() {
                devTask.clear();
        }
        ~_DevThreadParam(){
                delete ddfsArchive;
                devTask.clear();
        }

        DDFSArchive* ddfsArchive;
        map<string, int64_t> devTask;
} DevThreadParam;

typedef struct _ThreadParam {
        ~_ThreadParam() {
                delete ddfsArchive;
        }

        DDFSArchive* ddfsArchive;
        string       taskPath;
        int64_t      lineNum;
} ThreadParam; 

#endif
