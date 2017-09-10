/*************************************************
 * Copyright (C), 2010-2011, Scidata Tech. Co., Ltd.
 *
 * Description: 完成多路径归档任务
 * 
 * Author: gfl
 **************************************************/

#ifndef _TASK_MGR_
#define _TASK_MGR_

#include <map>
#include <string>

using std::map;
using std::string;

class TaskMgr
{
public:
        TaskMgr();
        ~TaskMgr();
public:
	/** 
	 * @note:
	 * @return:
	 */	
        int GetAllTask(const string& configPath, map<string, int64_t> &task);

	/** 
	 * @note:
	 * @return:
	 */	
        int GetDBTask(const string& configPath, map<string, int64_t> &task);

	/** 
	 * @note:
	 * @return:
	 */	
        int GetTaskPath(const string &configPath, string& path);
        
	/** 
	 * @note:
	 * @return:
	 */	
        int RemoveEmptyTaskDir(const string& configPath);

	/** 
	 * @note:
	 * @return:
	 */	
        int GetTaskVolume(const string& path, string &vol);

private:
        /*
        map<string, int64_t> m_Todo;
        map<string, int64_t> m_Doing;
        */
};

#endif //_TASK_MGR
