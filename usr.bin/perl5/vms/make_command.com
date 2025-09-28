$! MAKE_COMMAND.COM
$! Record MM[SK]/Make parameters in configuration report
$!
$! Author:  Peter Prymmer <pvhp@lns62.lns.cornell.edu>
$! Version: 1.0  18-Jan-1996
$!
$! DCL usage (choose one):
$!      @MAKE_COMMAND                      !or
$!      @MAKE_COMMAND/OUTPUT=MYCONFIG.OUT
$!------------------------------------------------
$ $mms = "'"+p1
$ $makeline = p2+" "+p3+" "+p4+" "+p5+" "+p6+" "+p7+" "+p8
$quotable:
$ if f$locate("""",$makeline).lt.f$length($makeline)
$   then
$   $makeline = $makeline - """"
$   goto quotable
$ endif
$ $makeline = f$edit($makeline,"COMPRESS,TRIM")
$ write sys$output " make_cmd=''$mms'"+" ''$makeline''"
$!------------------------------------------------
