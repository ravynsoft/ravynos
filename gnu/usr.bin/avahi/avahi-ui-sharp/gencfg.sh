#!/bin/sh

. $1
common_dlname=$dlname

exec sed -e "s,@COMMON_DLNAME\@,${common_dlname},g"
