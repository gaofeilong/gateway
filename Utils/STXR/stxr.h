/*
 * stxr.h
 *
 *  Created on: Dec 29, 2011
 *      Author: root
 */

#ifndef STXR_H_
#define STXR_H_

/*
 * STGetConnInfo 解析配置文件获取连接信息
 *
 *      param
 *      1.	Configpath：对应的数据库连接信息的配置文件路径
 *      2.	connInfo:预分配的连接信息内存[in/out]，建议大小待定
 *      3.	Buflen:预分配的内存大小
 *
 *      return
 *                   0：成功 1：失败
 */
extern int STGetConnInfo(char *configpath, void *connInfo, int buflen);

/*
 * STGetTablespaceName 获取表空间名
 *
 *      param
 *      1.	connInfo：数据库连接信息
 *      2.	datafilename:要消冗的数据文件名
 *      3.	Tbsname:预分配的表空间名内存[in/out]
 *      4.	Buflen:预分配的内存大小
 *
 *      return
 *                   0：成功 1：失败
 */
extern int STGetTablespaceName(void *connInfo, char * datafilename, char *tbsname, int buflen);

/*
 * STSetTablespaceReadOnly 表空间只读
 *
 *      param
 *      1.	connInfo：数据库连接信息
 *      2.	tbsname:表空间名
 *
 *      return
 *                   0：成功 1：失败
 */
extern int STSetTablespaceReadOnly(void *connInfo, char * tbsname);

/*
 * STSetTablespaceOffline 表空间Offline
 *
 *      param
 *      1.	connInfo：数据库连接信息
 *      2.	tbsname:表空间名
 *
 *      return
 *                   0：成功 1：失败
 */
extern int STSetTablespaceOffline(void *connInfo, char * tbsname);

/*
 * STSetTablespaceOnline 表空间Online
 *
 *      param
 *      1.	connInfo：数据库连接信息
 *      2.	tbsname:表空间名
 *
 *      return
 *                   0：成功 1：失败
 */
extern int STSetTablespaceOnline(void *connInfo, char * tbsname);




#endif /* STXR_H_ */
