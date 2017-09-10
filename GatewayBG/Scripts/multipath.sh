#!/bin/sh
#1: 获取存储设备wwid：scsi_id –g –s /block/sda
#2: 插入多路径配置文件的黑名单列表,多路径配置文件(/etc/multipath.conf)
#Author:jyin



#setENV
conf_file="/etc/multipath.conf"
get_wwid_bin="scsi_id –g –s"
key_word='blacklist {'
tmp_file="/etc/multipath.conf.tmp.$$"
line_begin=''
line_end=''
line=''
disk_name=`fdisk -l |grep "^Disk" |awk '{print $2}'|awk -F : '{print $1}'|sed 's/\/dev\///g'`
wwid_file="/etc/wwid_file"
if [ -f $wwid_file ]
then
rm -rf $wwid_file
else
for i in `echo $disk_name`
do 
t=`scsi_id -g -s /block/$i |awk '{print $NF}'`
if [ ! -z $t ]
then
echo $t>>$wwid_file.$$
fi
done
while read line; do line="wwid $line"; echo $line >>$wwid_file; done<$wwid_file.$$
rm -rf $wwid_file.$$
fi




NEED_FILE='-1'

#check ENV
if [ ! -f $conf_file ]
then
echo "Config_file doesn't exist"
exit $NEED_FILE
fi

#Let's go
#case 1 :exist blacklist {}
if grep "^$key_word" $conf_file &>/dev/null
then
line_begin=`grep -n "^$key_word" $conf_file |awk -F : '{print $1}'`
line_end=`grep -n "}" $conf_file |awk -F : '{print $1}'|awk '$1>"$begin_line" {print $1}'|sed -n '1'p`
  #change config 
  sed "$line_begin,$line_end d" $conf_file>$tmp_file
  sed -n "1,$line_begin"p $tmp_file >$tmp_file.tmp
  echo "blacklist {" >>$tmp_file.tmp
  cat $wwid_file >>$tmp_file.tmp
  echo "}" >>$tmp_file.tmp
  let line=line_begin+1
  sed -n "$line,$"p $tmp_file >>$tmp_file.tmp
  sed '/devnode/d' $tmp_file.tmp >$tmp_file
  #backup config file
  mv $conf_file $conf_file.bak
  mv $tmp_file $conf_file
  #clean tmp file
  rm -rf $tmp_file* $wwid_file*
    #case 2 : exist #blacklist{}
    elif grep "^#$key_word" $conf_file &>/dev/null
    then
        line_begin=`grep -n "^#blacklist {" $conf_file |awk -F : '{print $1}' |sed -n '1'p`
        sed -n "1,$line_begin"p $conf_file >$tmp_file.tmp
        echo "blacklist {" >>$tmp_file.tmp
        cat $wwid_file >>$tmp_file.tmp
        echo "}" >>$tmp_file.tmp
        let line=line_begin+1
        sed -n "$line,$"p $conf_file >>$tmp_file.tmp
         sed '/devnode/d' $tmp_file.tmp >$tmp_file
        #backup config file
        mv $conf_file $conf_file.bak
        mv $tmp_file $conf_file
        #clean tmp file
        rm -rf $tmp_file* $wwid_file*
        
fi
