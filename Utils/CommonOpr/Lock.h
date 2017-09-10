#ifndef _LOCK_H_
#define _LOCK_H_

#include <string>
using std::string;

#define LOCK_SUCCESS 1
#define LOCK_FAIL    2

class Lock {
public:
        Lock();
        ~Lock();

        /*
         * @note 对指定文件加锁         
         */
        int LockFile(const string& filePath);

        /*
         * @note 试着去锁文件，如果锁成功的话，去掉锁
         */
        int TryLockFile(const string& filePath);

        /*
         * @note 获取目前filePath文件的锁状态
         */
        int GetLockStatus(const string& filePath);
private:
        int UnLockFile(int fd);
        int DoLockFile(const string& filePath, int& fd);
};

#endif
