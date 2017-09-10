/*******************************************************************************
 *
 * @ File Name  : FileFilter.cpp
 * @ Date       : 2012-10-17
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 
 * @ History    : 2012-10-17：创建
 *
 * ****************************************************************************/
#include "FileFilter.h"
#include "Utils/Log/Log.h"

/* constructor */
FileFilter::FileFilter()
{
}

/* destructor */
FileFilter::~FileFilter()
{
}

/**
 * Filter() -虚函数，被各过滤子类继承和重写，用于判断文件是否符合要求
 * @path: 被检查文件路径
 * @arg: 过滤条件结构体
 * @return: true 符合条件
 *          false 不符合条件 
 */
bool FileFilter::Filter(const string &path)
{
#ifdef  _DEBUG_
        printf("file filter ok: %s\n", path.c_str());
#endif  //_DEBUG_
        return true;
}
