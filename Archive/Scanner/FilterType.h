/*******************************************************************************
 *
 * @ File Name  : FilterType.h
 * @ Date       : 2012-10-17
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 
 * @ History    : 2012-10-17：创建
 *                2013-03-07: 添加目录过滤结构体
 *
 * ****************************************************************************/

#ifndef _FILTER_TYPE_H_
#define _FILTER_TYPE_H_

#include <set>
#include <string>
using std::set;
using std::string;

/* _NOFILTER：不指定或过滤。_FILTER: 过滤，_DEMAND：指定 */
enum _FilterType { _NOFILTER, _FILTER, _DEMAND };

/* 过滤前缀、后缀 */
struct _FixFilterArg {
        enum _FilterType _Type;         /* 如果指定了，指向必须是 0 */
        set<string> _FixSet;            /* 要指定、过滤的文件类型集合 */
};

/* 过滤目录 */
struct _DirFilterArg {
        enum _FilterType _Type;         /* 如果指定了，指向必须是 0 */
        set<string> _DirSet;            /* 要指定、过滤的文件目录集合 */
};

/* 用 Max 和 Min 是否为 0 来标识三种过滤：最大、最小、介于 */
struct _SizeFilterArg {
        enum _FilterType _Type;         /* -1：无，0: 过滤，1：指定 */
        size_t _Max;                    /* 指定、过滤文件大小的最大值 */
        size_t _Min;                    /* 指定、过滤文件大小的最小值 */
};

/* 指定、过滤所有条件的综合 */
struct _FilterArg {
        struct _DirFilterArg _DirArg;
        struct _SizeFilterArg _SizeArg;
        struct _FixFilterArg _PrefixArg;
        struct _FixFilterArg _PostfixArg;
};

#endif  //_FILTER_TYPE_H_
