/*******************************************************************************
 *
 * @ File Name  : SizeFilter.h
 * @ Date       : 2012-10-17
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 
 * @ History    : 2012-10-17：创建
 *
 * ****************************************************************************/
#ifndef _SIZE_FILTER_H_
#define _SIZE_FILTER_H_

#include <boost/shared_ptr.hpp>

#include "FileFilter.h"
#define BOOSTPTR        boost::shared_ptr

class SizeFilter : public FileFilter {
public:
        SizeFilter(BOOSTPTR<FileFilter> &ptr, const struct _SizeFilterArg &arg);
        ~SizeFilter();

public:
        bool Filter(const string &path);

private:
        inline bool InRange(size_t size);

private:
        BOOSTPTR<FileFilter> m_FilterPtr;
        struct _SizeFilterArg m_SizeArg;
};

#endif  //_SIZE_FILTER_H_
