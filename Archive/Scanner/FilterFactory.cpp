/*******************************************************************************
 *
 * @ File Name  : FilterFactory.cpp
 * @ Date       : 2012-10-24
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 
 * @ History    : 2012-10-24：创建
 *              : 2012-10-31：路径过滤标识敲错了
 *              : 2013-03-07：添加创建目录过滤对象层
 *
 * ****************************************************************************/
#include <stdio.h>

#include "FilterFactory.h"
#include "PostfixFilter.h"
#include "PrefixFilter.h"
#include "SizeFilter.h"
#include "DirFilter.h"

#define BOOSTPTR        boost::shared_ptr
/* constructor */
FilterFactory::FilterFactory()
{
}

/* destructor */
FilterFactory::~FilterFactory()
{
}

/**
 * FilterFactory() -工厂函数，根据过滤条件（结构体）创建过滤对象
 * @return: boost::shared_ptr指针，指向过滤对象
 */
BOOSTPTR<FileFilter> FilterFactory::CreateFilter(const struct _FilterArg &arg)
{
        BOOSTPTR<FileFilter> filterPtr = BOOSTPTR<FileFilter>(new FileFilter());
#ifdef  _DEBUG_
        printf("create FileFilter\n");
#endif  //_DEBUG_

        if (arg._SizeArg._Type != _NOFILTER) {
                filterPtr = BOOSTPTR<FileFilter>(new SizeFilter(filterPtr, arg._SizeArg));
#ifdef  _DEBUG_
                printf("create SizeFilter\n");
#endif  //_DEBUG_
        }
        if (arg._PostfixArg._Type != _NOFILTER) {
                filterPtr = BOOSTPTR<FileFilter>(new PostfixFilter(filterPtr, arg._PostfixArg));
#ifdef  _DEBUG_
                printf("create PostfixFilter\n");
#endif  //_DEBUG_
        }
        if (arg._PrefixArg._Type != _NOFILTER) {
                filterPtr = BOOSTPTR<FileFilter>(new PrefixFilter(filterPtr, arg._PrefixArg));
#ifdef  _DEBUG_
                printf("create PrefixFilter\n");
#endif  //_DEBUG_
        }
        if (arg._DirArg._Type != _NOFILTER) {
                filterPtr = BOOSTPTR<FileFilter>(new DirFilter(filterPtr, arg._DirArg));
#ifdef  _DEBUG_
                printf("create DirFilter\n");
#endif  //_DEBUG_
        }
        return filterPtr;
}
