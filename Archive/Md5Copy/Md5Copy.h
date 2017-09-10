/*************************************************
 * Copyright (C), 2010-2011, Scidata Tech. Co., Ltd.
 *
 * Description: 拷贝文件，校验文件MD5
 * 
 * Author: gfl
 **************************************************/
#ifndef MD5_COPY_H_
#define MD5_COPY_H_

#include <string>

class Md5Copy
{
public:
        Md5Copy();
        ~Md5Copy();
public:

        /**
         * @note 拷贝源数据到指定位置
         */
        int CopyData(int chkSig, const string& srcPath, const string& destPath);
};

#endif
