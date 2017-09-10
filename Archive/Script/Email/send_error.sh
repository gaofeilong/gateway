#!/bin/sh
#force send error mail
. /usr/sbin/sci_setenv.sh

if [ -f $mail ]
then
rm -rf $mail
fi

if [ -f $mail_content_error ]
then
rm -rf $mail_content_error
fi


organize_mail_error
send_mail_error $mail
cleanup_error
