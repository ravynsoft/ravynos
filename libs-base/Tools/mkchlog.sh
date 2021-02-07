#!/bin/sh
svn log -rHEAD --xml --verbose | xsltproc /usr/local/share/svn2cl/svn2cl.xsl - > ChangeLog.new
svn up ChangeLog
cat ChangeLog >> ChangeLog.new
mv ChangeLog.new ChangeLog
$EDITOR ChangeLog
svn commit -m 'Added ChangeLog entry from last commit' ChangeLog
