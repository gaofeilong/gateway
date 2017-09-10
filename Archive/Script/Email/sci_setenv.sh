#!/bin/sh
#set env for check
#Author:jyin


#set env
scigw_dir=/etc/scigw
warning_dir=$scigw_dir/Log/LogWarning
error_dir=$scigw_dir/Log/LogError
warning_file=$warning_dir/*
error_file=$error_dir/*
mail_content=$scigw_dir/mail_warning_content.log
mail_content_error=$scigw_dir/mail_error_content.log
send_mail="no"
send_mail_cpu="no"
send_mail_space="no"
send_mail_file="no"
send_mail_warn="no"
send_mail_error="no"

config_dir=$scigw_dir/Log/Config
config_file=$config_dir/scigw.ini
#error_conf=$config_dir/LogErrConfig.dat
#warning_conf=$config_dir/LogWarConfig.dat
mail_conf=$config_dir/LogEmailConfig.dat
if [ -f $config_file ]
then
partition=`cat $config_file|xargs`
else
echo "Please check you have config $config_file!"
exit
fi
used_space='0'
used_cpu='0'
time=`date`



#set user for sendmail


send_to=`grep "SendToAddr=" $mail_conf |awk -F "=" '{print $2}'`
send_from=`grep "SendFromAddr=" $mail_conf |awk -F "=" '{print $2}'`
passwd=`grep "Password=" $mail_conf |awk -F "=" '{print $2}'`
smtp=`echo $send_from |awk -F "@" '{print $2}'`
mail=$scigw_dir/mail

mail_type=`grep "MailType=" $mail_conf |awk -F "=" '{print $2}'`
subject_warning="`date +%Y-%m-%d` : warning_mail"
subject_error="`date +%Y-%m-%d` : error_mail"

database=scidata

mysql_bin="mysql -D $database -e"
table=sci_email
table_statement="$table(type,date,info)"


cpu_warning_info="The CPU is over usage rates"

sign_warning='11'
sign_error='11'

#warning
organize_mail_title(){
echo "################### Check Warning Begin  #####################" >$mail_content

}
organize_mail_cpu(){
echo "">>$mail_content
echo "--------------------- CPU Check Begin ------------------------" >>$mail_content
echo "">>$mail_content
echo "  CPU Have Used: $used_cpu %">>$mail_content 
echo "" >>$mail_content
echo "---------------------- CPU Check End -------------------------" >>$mail_content
}
organize_mail_space(){
if [ -f $mail_content.tmp ]
then
  echo "">>$mail_content
  echo "------------------ Disk Space Check Begin --------------------" >>$mail_content
  echo "">>$mail_content
  cat $mail_content.tmp>>$mail_content
  rm -rf $mail_content.tmp
  echo "" >>$mail_content
  echo "------------------- Disk Space Check End --------------------- ">>$mail_content
fi
}
organize_mail_detail_warning(){
count_warning=`ls -al $warning_dir |wc -l`
if [ $count_warning -gt '3' ]
then
echo "">>$mail_content
echo '      Time                IP              Message' >>$mail_content
echo "">>$mail_content
for i in `ls $warning_dir`
do
cat $warning_dir/$i >>$mail_content
echo "">>$mail_content
done
fi
}

organize_mail_end(){
echo "" >>$mail_content
echo "###################### Check Warning End #####################">>$mail_content
}



#error
organize_mail_error(){
echo "################### Check Error Begin  #####################" >$mail_content_error
echo '      Time                IP              Message'>>$mail_content_error
echo "">>$mail_content_error
for i in `ls $error_dir`
do
cat $error_dir/$i >>$mail_content_error
echo "">>$mail_content_error
done



echo "" >>$mail_content_error
echo "###################### Check Error End #####################">>$mail_content_error
} 

send_mail_error(){
echo "Sending Error Mail ..."
mailing_content=`cat $mail_content_error`
cat <<EOF | tee -a $1 | perl >> $1 2>&1
use Mail::Sender;
use strict;

my \$sender = new Mail::Sender
        {smtp => '$smtp',
        from => '$send_from',
        auth => 'LOGIN',
        authid => '$send_from',
        authpwd => '$passwd'};

\$sender->MailFile({
        to => '$send_to',
        subject => '$subject_error',
        msg => '$mailing_content',
        file =>'$mail_content_error',
        skip_bad_recipients => 'ture',
        charset => 'GBK'}) or die "send mail failed";
EOF



if [ $? -eq '0' ]
then
sign_error='0'
else
sign_error='1'
fi
}

send_mail_warning(){
echo "Sending Warning Mail ..."
mailing_content=`cat $mail_content`
cat <<EOF | tee -a $1 | perl >> $1 2>&1
use Mail::Sender;
use strict;

my \$sender = new Mail::Sender
        {smtp => '$smtp',
        from => '$send_from',
        auth => 'LOGIN',
        authid => '$send_from',
        authpwd => '$passwd'};

\$sender->MailFile({
        to => '$send_to',
        subject => '$subject_warning',
        msg => '$mailing_content',
        file =>'$mail_content',
        skip_bad_recipients => 'ture',
        charset => 'GBK'}) or die "send mail failed";
EOF


if [ $? -eq '0' ]
then
sign_warning='0'
else
sign_warning='1'
fi
}

cleanup_warning(){
rm -rf $warning_dir/*
}

cleanup_error(){
rm -rf $error_dir/*
}

cleanup_log(){
rm -rf $scigw_dir/*.log
}

error_check(){
#cleanup env
if [ -f $mail_content_error ]
then
rm -rf $mail_content_error
fi

if [ -f $mail ]
then
rm -rf $mail
fi


#check whether send mail
count_error=`ls -al $error_dir |wc -l`
if [ $count_error -gt '3' ]
then
send_mail_error="yes"
fi



if [ x"$send_mail_error" == x"yes" ]
then
organize_mail_error
send_mail_error $mail
fi


#clean error & warning
if [ $sign_error -eq '0' ]
then
cleanup_error
fi
}


warning_check(){
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
echo "Partition $i Have Used $used_space % Space ">>$mail_content.tmp
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



#check whether send mail
count_warning=`ls -al $warning_dir |wc -l`

if [ $count_warning -gt '3' ]
then
send_mail_file="yes"
fi

if [ -f $mail_content.tmp ]
then
  if grep "Space" $mail_content.tmp
  then
  send_mail_space="yes"
  fi
fi

if [ x"$send_mail_cpu" == x"yes" ] || [ x"$send_mail_space" == x"yes" ] || [ x"$send_mail_file" == x"yes" ]
then
send_mail="yes"
fi

if [ x"$send_mail" == x"yes" ]
then
organize_mail_title
  
  if [ x"$send_mail_cpu" == x"yes" ]
  then
  organize_mail_cpu
  fi

  if [ x"$send_mail_space" == x"yes" ]
  then
  organize_mail_space
  fi
  

  if [ x"$send_mail_file" == x"yes" ]
  then
  organize_mail_detail_warning
  fi
  
organize_mail_end
send_mail_warning $mail

fi



#clean  warning

if [ $sign_warning -eq '0' ]
then
cleanup_warning
fi
}
