/*******************************************************************************
 *
 * @ File Name  : PostfixFilter.h
 * @ Date       : 2012-10-17
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 
 * @ History    : 2012-10-17：创建
 *
 * ****************************************************************************/
#ifndef _POSTFIX_FILTER_H_
#define _POSTFIX_FILTER_H_

#include <boost/shared_ptr.hpp>

#include "FileFilter.h"
#define BOOSTPTR        boost::shared_ptr

class PostfixFilter : public FileFilter {
public:
        PostfixFilter(BOOSTPTR<FileFilter> &ptr, const struct _FixFilterArg &arg);
        ~PostfixFilter();

public:
        bool Filter(const string &path);

private:
        BOOSTPTR<FileFilter> m_FilterPtr;
        struct _FixFilterArg m_PostfixArg;
};

#endif  //_POSTFIX_FILTER_H_
