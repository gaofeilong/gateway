/*******************************************************************************
 *
 * @ File Name  : FileFilter.h
 * @ Date       : 2012-10-17
 * @ Author     : gaofeilong <gaofeilonggmail@163.com>
 * @ Description: 
 * @ History    : 2012-10-17：创建
 *
 * ****************************************************************************/
#ifndef _FILE_FILTER_H_
#define _FILE_FILTER_H_
//#define _DEBUG_
#include "FilterType.h"

class FileFilter {
public:
        FileFilter();
        virtual ~FileFilter();

public:
        virtual bool Filter(const string &path);

};

#endif  //_FILE_FILTER_H_
