#!/bin/sh
#run error warning check
#Author:jyin

. /usr/sbin/sci_setenv.sh
RET='66'
if [ -z $mail_type ]
then
echo "Read $mail_conf error!"
else
  case $mail_type in
  1)
  error_check
  if [ $? -eq '0' ]
  then
  echo "run error_check ok"
  exit $RET
  else
  echo "run error_check fail"
  RET='1'
  exit $RET
  fi
  ;;
  2)
  warning_check
  if [ $? -eq '0' ]
  then
  echo "run warning_check ok"
  exit $RET
  else
  echo "run warning_check fail"
  RET='2'
  exit $RET
  fi
  ;;
  3)
  error_check
  if [ $? -eq '0' ]
  then
  echo "run error_check ok"
  else
  echo "run error_check fail"
  RET='3'
  fi
  
  warning_check
  if [ $? -eq '0' ]
  then
  echo "run warning_check ok"
  exit $RET
  else
  echo "run warning_check fail"
  RET='3'
  exit $RET
  fi
  ;;
  *)
  RET='4'
  exit $RET  
  ;;
  esac    
fi
