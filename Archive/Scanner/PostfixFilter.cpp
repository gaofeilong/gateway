/*******************************************************************************
 *
 * @ File Name  : PostfixFilter.cpp
 * @ Date       : 2012-10-17
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 
 * @ History    : 2012-10-17：创建
 *
 * ****************************************************************************/
#include "PostfixFilter.h"
#include "Utils/Log/Log.h"

/* constructor */
PostfixFilter::PostfixFilter(BOOSTPTR<FileFilter> &ptr, 
                        const struct _FixFilterArg &arg): 
        m_FilterPtr(ptr), m_PostfixArg(arg)
{
}

/* destructor */
PostfixFilter::~PostfixFilter()
{
}

/**
 * Filter() -归档文件类型过滤函数，继承自 FileFilter 类
 * @path: 被检查文件路径
 * @arg: 过滤条件结构体
 * @return: true 符合条件
 *          false 不符合条件 
 */
bool PostfixFilter::Filter(const string &path)
{
        size_t pos;
        string file;

        if ((pos = path.rfind("/")) != string::npos) {
                file = path.substr(pos + 1);
        } else {
                file = path; 
        }

        set<string>::const_iterator ssCit = m_PostfixArg._FixSet.begin(); 
        if (m_PostfixArg._Type == _DEMAND) {
                for (; ssCit != m_PostfixArg._FixSet.end(); ++ssCit) {
                        pos = file.rfind(*ssCit);
                        if (pos != string::npos && pos + ssCit->size() == file.size()) {
                                /* 文件名后缀符合被指定的条件 */
#ifdef  _DEBUG_
	                        printf("postfix filter ok: file=%s, postfix=%s, "
					"pos=%ld, ssCit->size()=%ld, file.size()=%ld\n", 
                                        file.c_str(), ssCit->c_str(), pos, 
                                        ssCit->size(), file.size());
#endif  //_DEBUG_
                                return m_FilterPtr->Filter(path);
                        }
#ifdef  _DEBUG_
                        printf("not pass: file=%s, postfix=%s, pos=%ld, "
                                        "ssCit->size()=%ld, file.size()=%ld\n", 
                                        file.c_str(), ssCit->c_str(), pos, 
                                        ssCit->size(), file.size());
#endif  //_DEBUG_
                }
#ifdef  _DEBUG_
                printf("postfix demand error: %s\n", file.c_str());
#endif  //_DEBUG_
                return false;
        } else {
                for (; ssCit != m_PostfixArg._FixSet.end(); ++ssCit) {
                        pos = file.rfind(*ssCit);
                        if (pos != string::npos && pos + ssCit->size() == file.size()) {
                                /* 文件名后缀符合被过滤的条件 */
#ifdef  _DEBUG_
	                        printf("postfix filter ok: file=%s, postfix=%s, "
					"pos=%ld, ssCit->size()=%ld, file.size()=%ld\n", 
                                        file.c_str(), ssCit->c_str(), pos, 
                                        ssCit->size(), file.size());
#endif  //_DEBUG_
                                return false; 
                        }
                }
#ifdef  _DEBUG_
                printf("postfix filter ok: %s\n", file.c_str());
#endif  //_DEBUG_
                return m_FilterPtr->Filter(path);
        }
}
