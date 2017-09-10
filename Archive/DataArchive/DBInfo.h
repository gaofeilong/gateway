#ifndef _DBINFO_H_
#define _DBINFO_H_

#include <string>
#include <stdlib.h>

typedef enum _IsTaskFinished{
        UNFINISH,
        FINISHED
} IsTaskFinish;

typedef struct _Record{
        std::string Path;
        int LineNum;
        IsTaskFinish IsFinished;
} Record; 

typedef struct _ArvLogInfo {
        std::string startTime;
        std::string endTime;
        std::string srcPath;
        std::string destPath;
        int         arvStatus;
        int64_t     totalSize;
} ArvLogInfo;

typedef struct _ArvErrInfo {
        std::string srcPath;
        std::string time;
        std::string errSubject;
        std::string errInfo;
} ArvErrInfo;

typedef struct _ArvDetailInfo {
        std::string srcPath;
        std::string destPath;
        std::string arvTime;
        int64_t     arvSize;
} ArvDetailInfo;

#endif //_DBINFO_H_
