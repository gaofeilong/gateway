/*******************************************************************************
 *
 * @ File Name  : DirFilter.h
 * @ Date       : 2012-10-17
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 过滤文件目录
 * @ History    : 2012-10-17：创建
 *
 * ****************************************************************************/
#ifndef _DIR_FILTER_H_
#define _DIR_FILTER_H_

#include <boost/shared_ptr.hpp>
#include "FileFilter.h"
#define BOOSTPTR        boost::shared_ptr

class DirFilter : public FileFilter {
public:
        DirFilter(BOOSTPTR<FileFilter> &ptr, const struct _DirFilterArg &arg);
        ~DirFilter();

private:
        bool Filter(const string &path);

private:
        BOOSTPTR<FileFilter> m_FilterPtr;
        struct _DirFilterArg m_DirArg;
};

#endif  //_DIR_FILTER_H_
