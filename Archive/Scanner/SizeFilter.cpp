/*******************************************************************************
 *
 * @ File Name  : SizeFilter.cpp
 * @ Date       : 2012-10-17
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 
 * @ History    : 2012-10-17：创建
 *
 * ****************************************************************************/
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "SizeFilter.h"
#include "Utils/Log/Log.h"

/* constructor */
SizeFilter::SizeFilter(BOOSTPTR<FileFilter> &ptr, 
                        const struct _SizeFilterArg &arg): 
        m_FilterPtr(ptr), m_SizeArg(arg)
{
}

/* destructor */
SizeFilter::~SizeFilter()
{
}

/**
 * Filter() -归档文件大小过滤函数，继承自 FileFilter 类
 * @path: 被检查文件路径
 * @arg: 过滤条件结构体
 * @return: true 符合条件
 *          false 不符合条件 
 */
bool SizeFilter::Filter(const string &path)
{
        struct stat st;
        stat(path.c_str(), &st);

        if (m_SizeArg._Type == _DEMAND) {                 /* 指定 */
                if (!InRange(st.st_size)) {                     /* 不符合被指定 */
#ifdef  _DEBUG_
                        printf("size filter error: %s\n", path.c_str());
#endif  //_DEBUG_
                        return false;
                }
        } else {
                if(InRange(st.st_size)) {                       /* 符合被过滤 */
#ifdef  _DEBUG_
                        printf("size filter error: %s\n", path.c_str());
#endif  //_DEBUG_
                        return false;
                }
        }
        //printf("size filter ok: %s\n", path.c_str());
        
        /* 属于被指定的或者没有被过滤掉，进入下一次过滤条件判断 */
#ifdef  _DEBUG_
        printf("size filter ok: %s\n", path.c_str());
#endif  //_DEBUG_
        return m_FilterPtr->Filter(path);
}

/**
 * InRange() -判断文件大小是否符合要求
 * @size: 被检查文件大小
 * @sizeArg: 文件大小过滤条件结构体
 * @return: true 符合条件
 *          false 不符合条件 
 */
bool SizeFilter::InRange(size_t size)
{
        if (m_SizeArg._Min == 0) {                /* 小于某个值 */
                return size <= m_SizeArg._Max;
        } else if (m_SizeArg._Max == 0) {         /* 大于某个值 */
                return size >= m_SizeArg._Min;
        } else {                                /* 介于某两个值之间 */
                return size <= m_SizeArg._Max && size >= m_SizeArg._Min;
        }
}
