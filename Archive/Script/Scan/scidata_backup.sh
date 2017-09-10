#!/bin/bash

#author:jyin

export LANG=zh_EN.UTF-8 ||export LANG=en_US.UTF-8
. /usr/sbin/scidata_backup_setenv.sh


if [ ! -d $mount_dest ]
then
mkdir -p $mount_dest
fi

#config_file_check




rm -rf $work_dir/*
#config file check

if grep "intervalTime=" $config_file &>/dev/null
then
gap=`cat $config_file |grep "intervalTime="|awk -F = '{print $2}'`
else
RET=$NULL
echo "Please configure the config_file" | tee -a $exception_log
exit $RET
fi

if grep "modifyTime=" $config_file &>/dev/null
then
offset=`cat $config_file |grep "modifyTime="|awk -F = '{print $2}'`
else
RET=$NULL
echo "Please configure the config_file" | tee -a $exception_log
exit $RET
fi




#if [ $# -eq '0' ]
#then
#usage
#fi

#while [ "$#" -gt '0' ]
#do
#        case $1 in
#        -g )
#                gap=$2
#                shift
#                ;;
#        -g*)
#                gap=`echo $1 | sed 's/-g//'`
#        
#                ;;
#        -o )
#                offset=$2
#                shift
#                ;;
#        -o*)
#                offset=`echo $1 | sed 's/-o//'`
#        
#                ;;
#        
#        -d )
#                dest_dir=$2
#               shift
#                ;;
#        -d*)
#                dest_dir=`echo $1 | sed 's/-d//'`
#                
#               ;;
#        -f )
#                result_file=$2
#                shift
#                ;;
#        -f*)
#                result_file=`echo $1 | sed 's/-f//'`
#                ;;
#                
#        --help|-\?)
#                usage
#                break
#                ;;
#       *)
#                usage
#                ;;
#        esac
#        shift
#done



#echo "gap="$gap
#echo "offset="$offset
#echo "dest="$dest_dir
#echo "file="$result_file

#mkdir for result_file
if [  -d $result_dir ]
then
rm -rf $result_dir
mkdir -p $result_dir
else
mkdir -p $result_dir
fi

if [  -d $log_dir ]
then
rm -rf $log_dir
mkdir -p $log_dir
else
mkdir -p $log_dir
fi

#argument check
argument_check





#Let's go
#mkdir for scan
if [ ! -d $work_dir ]
then
mkdir -p $work_dir
fi



#¸ù¾ÝÊÇ·ñmount
count='1'
for i in $pathlist
do
        result_file=$result_dir/arv_path_$count
        dest_dir=$i
        #echo $i
        isdir $i
        #echo $flag_dir
        case $flag_dir in
        0)
        echo "$i is dir ,do not neet to mount" >>$common_log
        #do scan
        backup_dest=$i
        
        #select with gap
        select_case
        scan
        organize_scan_result
        let count+=1
        ;;
        1)
        #mount dest_dir
        echo "mount $i" >>$common_log
        mount $i $mount_dest
        if [ $? -ne '0' ]
        then
            umount $mount_dest
            if [ $? -eq '0' ]
            then
            mount $i $mount_dest
                if [ $? -ne '0' ]
                then
                echo "$i mount fail" >>$exception_log
                RET=$MOUNT_FAIL
                exit $RET
                fi
            fi
        fi
        echo "list mount" >>$common_log
        mount >>$common_log
        
        #do scan
        #select with gap
        backup_dest=$mount_dest
        select_case
        scan
        organize_scan_result
        let count+=1
        #umount dest_dir
        echo "umount " >>$common_log
        umount $mount_dest
        echo "list umount" >>$common_log
        mount >>$common_log
        ;;
        esac
done










