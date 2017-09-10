#ifndef _DIR_FILE_OPR_H_
#define _DIR_FILE_OPR_H_

#include <set>
#include <string>
#include <stdlib.h>

using std::set;
using std::string;

#define CAN_READ      1
#define CAN_NOT_READ  2

#define CAN_WRITE     3
#define CAN_NOT_WRITE 4 

class DirFileOpr
{
public:
        DirFileOpr();
        ~DirFileOpr();

public:
        int  GetDirSet(const string& path, set<string>& dirSet);
        int  GetFileSet(const string& path, set<string>& fileSet);
        int  GetAllFileSet(const string& path, set<string>& fileSet);
        int  DeleteEmptyDir(const string& path);
        int  GetFileSize(const string& path, int64_t& size);

        void GetParPath(const string& path, string& parPath);
        void GetDestPath(const string& refSrcPath, const string& refDestPath, const string& srcPath, string& destFullPath);

        bool IsDir(const string& path);
        bool IsLink(const string& path);
        bool IsBlockDevice(const string& path);
        bool IsMaxRecord(const string& path);
        bool IsDirEmpty(const string& path);
        bool IsUsing(const string& path);
        bool IsChanged(const string& src, const string& dest);
        bool IsMounted(const string& dev, const string& path);

        bool HaveFile(const string& path);
        int  MakeDir(const string& path);
        bool HasPath(const string& path);
        int  GetPathSize(string path, int64_t& size);
        int  GetFileMTime(const string& path, int64_t& sec);

        string ParseFileNameToPath(const string& fileName);

        string GetFileNameByFullPath(const string& path);

        string RebulidPath(const string& srcFullPath, const string& srcPath, const string& destPath);

        int CheckPath(const string& path, const string& type, string& errInfo);

        void AppendBias(string& path);
        void EraseLastBias(string& path);
private:
        int TravelDir(const string& path, set<string>& dirSet, set<string>& fileSet); 
        int IsWrite(const string& path, string& errInfo);
        int IsRead(const string& path, string& errInfo);

};

#endif
