#!/bin/sh

. $1
client_dlname=$dlname
. $2
common_dlname=$dlname
. $3
glib_dlname=$dlname

exec sed -e "s,@CLIENT_DLNAME\@,${client_dlname},g" \
         -e "s,@COMMON_DLNAME\@,${common_dlname},g" \
         -e "s,@GLIB_DLNAME\@,${glib_dlname},g"
