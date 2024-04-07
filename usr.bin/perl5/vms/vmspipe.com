$! 'f$verify(0)'
$!  ---  protect against nonstandard definitions ---
$ perl_define = "define/nolog"
$ perl_on     = "on error then exit $STATUS"
$ perl_exit   = "exit"
$ perl_del    = "delete"
$ pif         = "if"
$!  --- define i/o redirection (sys$output set by lib$spawn)
$ pif perl_popen_in  .nes. "" then perl_define/user/name_attributes=confine sys$input  'perl_popen_in'
$ pif perl_popen_err .nes. "" then perl_define/user/name_attributes=confine sys$error  'perl_popen_err'
$ pif perl_popen_out .nes. "" then perl_define      sys$output 'perl_popen_out'
$!  --- build command line to get max possible length
$c=perl_popen_cmd0
$c=c+perl_popen_cmd1
$c=c+perl_popen_cmd2
$x=perl_popen_cmd3
$c=c+x
$ perl_on
$ 'c'
$ perl_exit '$STATUS'
