#!/bin/bash
#
#author:jyin



usage()
{
    echo "Usage: `basename $0` "
    echo "<option>" 
    echo " --help,-?                    show this help, then exit"
    echo "-g                            This argument is gap    [m]"
    echo "-o                            This argument is offset [n]"
    echo "-d                            This argument is the dir which is the backup destination"
    echo "-f                            This argument is the scan result file"
    exit 0;
}

WRONG_DIR='-1'
NULL='-2'
NOT_NUMERAL='-3'
WRONG_FILE='-4'
MOUNT_FAIL='-5'


result_dir=/var/spool/scigw/ArvTask/`date +%Y%m%d`
log_dir=$result_dir/../../log/`date +%Y%m%d`
common_log=$log_dir/common.log
exception_log=$log_dir/exception.log
work_dir=$result_dir/../../tmp
last_end=$work_dir/last
base_file=$work_dir/base
find_file=$work_dir/find
find_result=$work_dir/find_result

config_file=/etc/scigw/GWconfig
if [ ! -f $config_file ]
then
RET=$WRONG_FILE
echo "Config_file for scan do not exist! " | tee -a $exception_log
exit $RET
fi

if grep "arvDevPath=" $config_file &>/dev/null
then
pathlist_config=`cat $config_file |grep "arvDevPath="|awk -F = '{print $2}'`
else
RET=$NULL
echo "Please configure the config_file" | tee -a $exception_log
exit $RET
fi

if [ ! -f $pathlist_config ]
then
RET=$WRONG_FILE
echo "pathlist_config for scan do not exist! "| tee -a $exception_log
fi

tmp=`cat $pathlist_config`
if [ -z $tmp ] 2>/dev/null
then
RET=$NULL
echo "Please configure then Pathlist_config" |tee -a $exception_log
exit $RET
else
pathlist=`cat $pathlist_config|grep -v "\[*\]" | awk '{print $1}'`
fi

if grep "mountpoint=" $config_file &>/dev/null
then
mount_dest=`cat $config_file |grep "mountpoint="|awk -F = '{print $2}'`
else
RET=$NULL
echo "Please configure the config_file" | tee -a $exception_log
exit $RET
fi

if grep "filter=" $config_file &>/dev/null
then
filter_conf=`cat $config_file |grep "filter="|awk -F = '{print $2}'`
else
RET=$NULL
echo "Please configure the config_file" | tee -a $exception_log
exit $RET
fi


demand_dir=$work_dir/demand_dir
demand_type=$work_dir/demand_type
filter_dir=$work_dir/filter_dir
filter_type=$work_dir/filter_type
result_demand_dir=$work_dir/result_demand_dir
result_demand_type=$work_dir/result_demand_type
result_filter_dir=$work_dir/result_filter_dir
result_filter_type=$work_dir/result_filter_type
filter_result=$find_result


isdir(){
if [ -d $1 ]
then
flag_dir='0'
else
flag_dir='1'
fi
}

isnum(){
expr $1 + 1 &>/dev/null
}


argument_check(){
#dest_dir
#if [ ! -d $dest_dir ]
#then
#RET=$WRONG_DIR
#echo "Wrong dir for backup destination ">>$exception_log
#exit $RET
#fi



#gap
if [ -z $gap ]
then
RET=$NULL
echo "Gap is null">>$exception_log
exit $RET
fi

isnum $gap
if [ $? -ne '0' ]
then
RET=$NOT_NUMERAL
echo "Gap is not numeral">>$exception_log
exit $RET
fi

#offset
if [ -z $offset ]
then
RET=$NULL
echo "Offset is null">>$exception_log
exit $RET
fi

isnum $offset
if [ $? -ne '0' ]
then
RET=$NOT_NUMERAL
echo "Offset is not numeral">>$exception_log
exit $RET
fi
}

select_case(){
if [ $gap -eq '1' ]
then
    [ $offset -eq $gap ] && flag_case="a"
    [ $offset -gt $gap ] && flag_case="b"
fi
if [ $gap -gt '1' ]
then
    [ $offset -le $gap ] && flag_case="c"
    [ $offset -gt $gap ] && flag_case="d"
fi
}

scan(){
case $flag_case in 
    a )
        rm -rf $work_dir/*
        backup_date=`date -d "-$offset day" +%Y-%m-%d`
        let offset+=1
        find $backup_dest -type f -mtime -$offset >$base_file
        
        while read line
        do 
        if ls --full-time $line|grep $backup_date>/dev/null
        then
	         echo `du -b $line|awk '{print $2 , $1}'|sed 's/ /\,\.\?/g'`>>$find_result
        fi
        done < $base_file 
        let offset-=1
    ;;
    b )
        rm -rf $work_dir/*      
        backup_date=`date -d "-$offset day" +%Y-%m-%d`
        let offset+=1
        find $backup_dest -type f -mtime -$offset >$base_file
        
        while read line
        do 
        if ls --full-time $line|grep $backup_date>/dev/null
        then
	         echo `du -b $line|awk '{print $2 , $1}'|sed 's/ /\,\.\?/g'`>>$find_result
        fi
        done < $base_file 
        let offset-=1
    ;;
    c )
        rm -rf $work_dir/*       
        m=$gap
        let m+=1
        find $backup_dest -type f -mtime -$m -exec ls -l -t --full-time {} \;>$base_file
        let m-=1  
        while [ $m -gt '0' ]
        do
        n=`date -d "-$m day" +%Y-%m-%d`
        #echo $n
        let m--
        #echo $m
                grep $n $base_file >>$find_file
        done
        while read line
        do
        p=`awk '{print $9}' $find_file`
        du -b $p|awk '{print $2 , $1}'|sed 's/ /\,\.\?/g' >$find_result
        done<$find_file
        
        
        
        
        #backup_date=`date -d "-$offset day" +%Y-%m-%d`
        #let offset+=1
        #find $backup_dest -type f -mtime -$offset >$base_file
        #let offset-=3
        #find $backup_dest -type f -mtime -$offset >$find_file
        #cat $find_file $base_file | sort | uniq -u >$find_result.tmp
        #while read line
        #do 
        #if ls --full-time $line|grep $backup_date>/dev/null
        #then
	      #   echo `du -b $line|awk '{print $2 , $1}'|sed 's/ /\.\,\?/g'`>>$find_result
        #fi
        #done < $find_result.tmp 
    ;;
    d )
        rm -rf $work_dir/*
        m=$gap
        n=$offset
        let n+=1
        find $backup_dest -type f -mtime -$n -exec ls -t -l --full-time {} \;>$base_file
        let n-=1
        while [ $m -gt '0' ]
        do
        key_word=`date -d "-$n day" +%Y-%m-%d`
        echo $key_word
        let m--
        let n--
        echo $m
        echo $n
                grep $key_word $base_file >>$find_file
        done
        
        while read line
        do
        p=`awk '{print $9}' $find_file`
        du -b $p|awk '{print $2 , $1}'|sed 's/ /\,\.\?/g' >$find_result
        done<$find_file
        #find $backup_dest -type f -mtime $offset 1>$find_file 2>/dev/null
        #let offset-=gap
        #find $backup_dir -type f -mtime $offset 1>$base_file 2>/dev/null
        #cat $find_file $base_file | sort | uniq -u >$find_result
    ;;
    * )
    echo "WRONG ARGUMENT FOR gap or offset"
    ;;    
esac
}

organize_scan_result(){
#filter
if [ -f $filter_conf ]
then
    #1 demand_dir
    sed -n '/demandPath/,/\[*\]/p' $filter_conf |sed '/^$/d'|grep -v "\[">$demand_dir
    while read line
    do
      if [ ! -z $line ]
      then
    	grep $line $filter_result >>$result_demand_dir
    	filter_result=$result_demand_dir
      fi
    done<$demand_dir


    #2 demand_type
    sed -n '/demandType/,/\[*\]/p' $filter_conf |sed '/^$/d'|grep -v "\[">$demand_type
    while read line
    do
    	if [ ! -z $line ]
      then
      grep $line $filter_result>>$result_demand_type
      filter_result=$result_demand_type
      fi
    done<$demand_type

    #3 filter_dir
    sed -n '/filterPath/,/\[*\]/p' $filter_conf |sed '/^$/d'|grep -v "\[">$filter_dir
    while read line
    do
        if [ ! -z $line ]
        then
        grep -v $line $filter_result>>$result_filter_dir
        filter_result=$result_filter_dir
        fi
    done<$filter_dir
    
    #4 filter_type
    sed -n '/filterType/,/\[*\]/p' $filter_conf |sed '/^$/d'|grep -v "\[">$filter_type
    while read line
    do
        if [ ! -z $line ]
        then
        grep -v $line $filter_result>>$result_filter_type
        filter_result=$result_filter_type
        fi
    done<$filter_type
else
echo "Filter file is do not exist!">>$exception_log
#RET=$NULL
#exit $RET
fi

#generate middle result

#if [ -f $result_filter_type ]
#then
#  rm -rf $find_result
#  cp $result_filter_type $find_result
#  elif [ -f $result_filter_dir ]
#  then
#        rm -rf $find_result
#        cp $result_filter_dir $find_result
#        elif [ -f $result_demand_type ]
#        then
#            rm -rf $find_result
#            cp $result_demand_type $find_result
#            elif [ -f $result_demand_dir ]
#            then
#            rm -rf $find_result
#            cp $result_demand_dir $find_result
#else 
#echo "Filter fail !">>$exception_log
##RET=$NULL
##exit $RET
#fi




#add device path
#if [ -f $find_result ]
#then
#rm -rf $result_file
#echo $dest_dir>>$result_file
#cat $find_result>>$result_file
#else
#echo "Scan result is do not exist!">>$exception_log
#RET=$NULL
#exit $RET
#fi

#add device path
if [ -f $filter_result ]
then
rm -rf $result_file
echo $dest_dir>>$result_file
cat $filter_result>>$result_file
else
echo "Scan result is do not exist!">>$exception_log
RET=$NULL
exit $RET
fi
}



