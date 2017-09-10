/*******************************************************************************
 *
 * @ File Name  : PrefixFilter.cpp
 * @ Date       : 2012-10-17
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 
 * @ History    : 2012-10-17：创建
 *
 * ****************************************************************************/
#include "PrefixFilter.h"
#include "Utils/Log/Log.h"

/* constructor */
PrefixFilter::PrefixFilter(BOOSTPTR<FileFilter> &ptr, 
                        const struct _FixFilterArg &arg): 
        m_FilterPtr(ptr), m_PrefixArg(arg)
{
}

/* destructor */
PrefixFilter::~PrefixFilter()
{
}

/**
 * Filter() -归档子路径过滤函数，继承自 FileFilter 类
 * @path: 被检查文件路径
 * @arg: 过滤条件结构体
 * @return: true 符合条件
 *          false 不符合条件 
 */
bool PrefixFilter::Filter(const string &path)
{
        size_t pos;
        string file;

        if ((pos = path.rfind("/")) != string::npos) {
                file = path.substr(pos + 1);
        } else {
                file = path; 
        }

        set<string>::const_iterator ssCit = m_PrefixArg._FixSet.begin(); 

        if (m_PrefixArg._Type == _DEMAND) {
                for (; ssCit != m_PrefixArg._FixSet.end(); ++ssCit) {
                        if (file.find(*ssCit) == 0) {
#ifdef  _DEBUG_
                                printf("prefix filter ok: %s\n", path.c_str());
#endif  //_DEBUG_
                                return m_FilterPtr->Filter(path);
                        }
                }
#ifdef  _DEBUG_
                                printf("prefix filter error: %s\n", path.c_str());
#endif  //_DEBUG_
                return false;
        } else {
                for (; ssCit != m_PrefixArg._FixSet.end(); ++ssCit) {
                        if (file.find(*ssCit) == 0) {
#ifdef  _DEBUG_
                                printf("prefix filter error: %s\n", path.c_str());
#endif  //_DEBUG_
                                return false;
                        }
                }
#ifdef  _DEBUG_
                printf("prefix filter ok: %s\n", path.c_str());
#endif  //_DEBUG_
                return m_FilterPtr->Filter(path);
        }
        
}
