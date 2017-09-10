/*******************************************************************************
 *
 * @ File Name  : PrefixFilter.h
 * @ Date       : 2012-10-17
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 
 * @ History    : 2012-10-17：创建
 *
 * ****************************************************************************/
#ifndef _PREFIX_FILTER_H_
#define _PREFIX_FILTER_H_

#include <boost/shared_ptr.hpp>
#include "FileFilter.h"
#define BOOSTPTR        boost::shared_ptr

class PrefixFilter : public FileFilter {
public:
        PrefixFilter(BOOSTPTR<FileFilter> &ptr, const struct _FixFilterArg &arg);
        ~PrefixFilter();

private:
        bool Filter(const string &path);

private:
        BOOSTPTR<FileFilter> m_FilterPtr;
        struct _FixFilterArg m_PrefixArg;
};

#endif  //_PREFIX_FILTER_H_
