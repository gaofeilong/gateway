#!/bin/bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"/usr/lib/":"/usr/lib64"
CRON=/var/spool/scigw/cron
CMD0=/usr/sbin/scidata_backup.sh
CMD1=/usr/sbin/DDFSArchive
#CMD1=/home/gfl/v3_gateway/DDFSArchive
if [ -e $CRON ] 
then 
        dayNum=`cat $CRON`
        if [ $dayNum -lt $1 ] 
        then
                #touch /var/spool/scigw/up
                day=$[${dayNum}+1]
                echo ${day} > $CRON
        else 
                #touch /var/spool/scigw/reset
                $CMD0
                echo 1 > $CRON
                $CMD1
        fi
else
        #配置文件丢失，直接执行归档，重新开始
        $CMD0
        echo 1 > $CRON
        $CMD1
fi
