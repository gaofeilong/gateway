#!/bin/sh


. /usr/sbin/sci_setenv.sh

#cleanup env
if [ -f $mail_content ]
then
rm -rf $mail_content
fi
if [ -f $mail ]
then
rm -rf $mail
fi

#check used space for $partition
for i in `echo $partition`
do
used_space=`df |sed -n '/\//p'|grep "$i" | awk '{print $5}' | sed 's/%//'`
if [ "$used_space" -ge '85' ]
then
echo "$i Have Used $used_space % Space ">>$mail_content.tmp
time=`date +%Y-%m-%d`" `date|awk '{print $4}'`"
space_warning_info="The $i space is over usage rates"
$mysql_bin "insert into $table_statement values ('2','$time','$space_warning_info');"
elif [ $used_space -eq '0' ]
then
echo "please check your input partition!"
fi
done


#check cpu
top -n 1 -i -b >cpu_log
used_cpu=`awk '
$1~/^[0-9]/ {
  a[$1]=$9;
}
END{
  for (i in a){
    sum+=a[i];
  }
  printf("%d\n", sum);
}
' cpu_log `

if [ "$used_cpu" -ge '90' ]
then
send_mail_cpu="yes"
echo "CPU have used $used_cpu % ">>$mail_content
time=`date +%Y-%m-%d`" `date|awk '{print $4}'`"
$mysql_bin "insert into $table_statement values ('2','$time','$cpu_warning_info');"
elif [ x"$used_cpu" == x"error" ]
then
echo "CPU check fail!"
fi

organize_mail_title
organize_mail_cpu
organize_mail_space
organize_mail_detail_warning
organize_mail_end
send_mail_warning $mail
cleanup_warning
