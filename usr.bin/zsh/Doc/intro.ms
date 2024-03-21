.nr PI 0
.nr LL 6.5i
.if \n(.g \{\
.if "\*(.T"ascii" .ftr C R
.if "\*(.T"latin1" .ftr C R
.nr De \n[.ss]
.\}
.de Ds
.DS I .5i
.ft C
.ps 9
.vs 11
.ss 11
..
.de De
.DE
.ft R
.ps
.vs
.ie \n(.g .ss \n(De
.el .ss
..
.de Sh
.SH
\\$1
.XS
\\$1
.XE
..
.nr HM 4i
.ce 99
.ps 18
.vs 20
.ss 20
\f3An Introduction to the Z Shell\fP

.ps 14
.vs 16
.ss 16
\f2Paul Falstad
pf@software.com

Bas de Bakker
bas@phys.uva.nl\fP
.ce 0
.nr HM 1i
.pn 1
.bp
.\" This blank page on the reverse of the cover.
.sv |1i
.pn 1
.bp
.TL
An Introduction to the Z Shell
.AU
Paul Falstad
pf@software.com
.AU
Bas de Bakker
bas@phys.uva.nl
.PP
.Sh "Introduction"
.PP
\fBzsh\fP is a shell designed for interactive use, although it is also
a powerful scripting language.  Many of the useful features of bash,
ksh, and tcsh were incorporated into \fBzsh\fP; many original features were
added.  This document details some of the unique features of \fBzsh\fP.  It
assumes basic knowledge of the standard UNIX shells; the intent is to
show a reader already familiar with one of the other major shells what
makes \fBzsh\fP more useful or more powerful.  This document is not at all
comprehensive; read the manual entry for a description of the shell
that is complete and concise, although somewhat overwhelming and
devoid of examples.
.PP
The text will frequently mention options that you can set to change
the behaviour of \fBzsh\fP.  You can set these options with the
command
.Ds
%\0setopt\0\fIoptionname\fC
.De
and unset them again with
.Ds
%\0unsetopt\0\fIoptionname\fC
.De
Case is ignored in option names, as are embedded underscores.
.Sh "Filename Generation"
.PP
Otherwise known as \fIglobbing\fP, filename generation
is quite extensive in \fBzsh\fP.  Of course, it has all the
basics:
.Ds
%\0ls
Makefile\0\0\0file.pro\0\0\0foo.o\0\0\0\0\0\0main.o\0\0\0\0\0q.c\0\0\0\0\0\0\0\0run234\0\0\0\0\0stuff
bar.o\0\0\0\0\0\0foo\0\0\0\0\0\0\0\0link\0\0\0\0\0\0\0morestuff\0\0run123\0\0\0\0\0run240\0\0\0\0\0sub
file.h\0\0\0\0\0foo.c\0\0\0\0\0\0main.h\0\0\0\0\0pipe\0\0\0\0\0\0\0run2\0\0\0\0\0\0\0run303
%\0ls\0*.c
foo.c\0\0q.c
%\0ls\0*.[co]
bar.o\0\0\0foo.c\0\0\0foo.o\0\0\0main.o\0\0q.c
%\0ls\0foo.?
foo.c\0\0foo.o
%\0ls\0*.[^c]
bar.o\0\0\0file.h\0\0foo.o\0\0\0main.h\0\0main.o
%\0ls\0*.[^oh]
foo.c\0\0q.c
.De
Also, if the \fIEXTENDEDGLOB\fP option is set,
some new features are activated.
For example, the \fC^\fP character negates the pattern following it:
.Ds
%\0setopt\0extendedglob
%\0ls\0-d\0^*.c
Makefile\0\0\0file.pro\0\0\0link\0\0\0\0\0\0\0morestuff\0\0run2\0\0\0\0\0\0\0run303
bar.o\0\0\0\0\0\0foo\0\0\0\0\0\0\0\0main.h\0\0\0\0\0pipe\0\0\0\0\0\0\0run234\0\0\0\0\0stuff
file.h\0\0\0\0\0foo.o\0\0\0\0\0\0main.o\0\0\0\0\0run123\0\0\0\0\0run240\0\0\0\0\0sub
%\0ls\0-d\0^*.*
Makefile\0\0\0link\0\0\0\0\0\0\0pipe\0\0\0\0\0\0\0run2\0\0\0\0\0\0\0run240\0\0\0\0\0stuff
foo\0\0\0\0\0\0\0\0morestuff\0\0run123\0\0\0\0\0run234\0\0\0\0\0run303\0\0\0\0\0sub
%\0ls\0-d\0^Makefile
bar.o\0\0\0\0\0\0foo\0\0\0\0\0\0\0\0link\0\0\0\0\0\0\0morestuff\0\0run123\0\0\0\0\0run240\0\0\0\0\0sub
file.h\0\0\0\0\0foo.c\0\0\0\0\0\0main.h\0\0\0\0\0pipe\0\0\0\0\0\0\0run2\0\0\0\0\0\0\0run303
file.pro\0\0\0foo.o\0\0\0\0\0\0main.o\0\0\0\0\0q.c\0\0\0\0\0\0\0\0run234\0\0\0\0\0stuff
%\0ls\0-d\0*.^c
\&.rhosts\0\0\0bar.o\0\0\0\0\0file.h\0\0\0\0file.pro\0\0foo.o\0\0\0\0\0main.h\0\0\0\0main.o
.De
An expression of the form
\fC<\fIx\fR\-\fIy\fC>\fR
matches a range of integers:
.Ds
%\0ls\0run<200-300>
run234\0\0run240
%\0ls\0run<300-400>
run303
%\0ls\0run<-200>
run123\0\0run2
%\0ls\0run<300->
run303
%\0ls\0run<->
run123\0\0run2\0\0\0\0run234\0\0run240\0\0run303
.De
The \fINUMERICGLOBSORT\fP option will sort files with numbers
according to the number.  This will not work with \fCls\fP as it
resorts its arguments:
.Ds
%\0setopt\0numericglobsort
%\0echo\0run<->
run2\0run123\0run234\0run240\0run303
.De
Grouping is possible:
.Ds
%\0ls\0(foo|bar).*
bar.o\0\0foo.c\0\0foo.o
%\0ls\0*.(c|o|pro)
bar.o\0\0\0\0\0file.pro\0\0foo.c\0\0\0\0\0foo.o\0\0\0\0\0main.o\0\0\0\0q.c
.De
Also, the string \fC**/\fP forces a recursive search of
subdirectories:
.Ds
%\0ls\0-R
Makefile\0\0\0file.pro\0\0\0foo.o\0\0\0\0\0\0main.o\0\0\0\0\0q.c\0\0\0\0\0\0\0\0run234\0\0\0\0\0stuff
bar.o\0\0\0\0\0\0foo\0\0\0\0\0\0\0\0link\0\0\0\0\0\0\0morestuff\0\0run123\0\0\0\0\0run240\0\0\0\0\0sub
file.h\0\0\0\0\0foo.c\0\0\0\0\0\0main.h\0\0\0\0\0pipe\0\0\0\0\0\0\0run2\0\0\0\0\0\0\0run303

morestuff:

stuff:
file\0\0xxx\0\0\0yyy

stuff/xxx:
foobar

stuff/yyy:
frobar
%\0ls\0**/*bar
stuff/xxx/foobar\0\0stuff/yyy/frobar
%\0ls\0**/f*
file.h\0\0\0\0\0\0\0\0\0\0\0\0foo\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0foo.o\0\0\0\0\0\0\0\0\0\0\0\0\0stuff/xxx/foobar
file.pro\0\0\0\0\0\0\0\0\0\0foo.c\0\0\0\0\0\0\0\0\0\0\0\0\0stuff/file\0\0\0\0\0\0\0\0stuff/yyy/frobar
%\0ls\0*bar*
bar.o
%\0ls\0**/*bar*
bar.o\0\0\0\0\0\0\0\0\0\0\0\0\0stuff/xxx/foobar\0\0stuff/yyy/frobar
%\0ls\0stuff/**/*bar*
stuff/xxx/foobar\0\0stuff/yyy/frobar
.De
.PP
It is possible to exclude certain files from the patterns using
the ~ character.  A pattern of the form \fC*.c~bar.c\fP lists all
files matching \fC*.c\fP, except for the file \fCbar.c\fP.
.Ds
%\0ls\0*.c
foo.c\0\0\0\0foob.c\0\0\0\0bar.c
%\0ls\0*.c~bar.c
foo.c\0\0\0\0foob.c
%\0ls\0*.c~f*
bar.c
.De
.PP
One can add a number of \fIqualifiers\fP to the end of
any of these patterns, to restrict matches to certain
file types.  A qualified pattern is of the form
.DS
\fIpattern\fC(\fR...\fC)\fR
.De
with single-character qualifiers inside the parentheses.
.Ds
%\0alias\0l='ls\0-dF'
%\0l\0*
Makefile\0\0\0\0foo*\0\0\0\0\0\0\0\0main.h\0\0\0\0\0\0q.c\0\0\0\0\0\0\0\0\0run240
bar.o\0\0\0\0\0\0\0foo.c\0\0\0\0\0\0\0main.o\0\0\0\0\0\0run123\0\0\0\0\0\0run303
file.h\0\0\0\0\0\0foo.o\0\0\0\0\0\0\0morestuff/\0\0run2\0\0\0\0\0\0\0\0stuff/
file.pro\0\0\0\0link@\0\0\0\0\0\0\0pipe\0\0\0\0\0\0\0\0run234\0\0\0\0\0\0sub
%\0l\0*(/)
morestuff/\0\0stuff/
%\0l\0*(@)
link@
%\0l\0*(*)
foo*\0\0\0\0\0\0\0\0link@\0\0\0\0\0\0\0morestuff/\0\0stuff/
%\0l\0*(x)
foo*\0\0\0\0\0\0\0\0link@\0\0\0\0\0\0\0morestuff/\0\0stuff/
%\0l\0*(X)
foo*\0\0\0\0\0\0\0\0link@\0\0\0\0\0\0\0morestuff/\0\0stuff/
%\0l\0*(R)
bar.o\0\0\0\0\0\0\0foo*\0\0\0\0\0\0\0\0link@\0\0\0\0\0\0\0morestuff/\0\0run123\0\0\0\0\0\0run240
file.h\0\0\0\0\0\0foo.c\0\0\0\0\0\0\0main.h\0\0\0\0\0\0pipe\0\0\0\0\0\0\0\0run2\0\0\0\0\0\0\0\0run303
file.pro\0\0\0\0foo.o\0\0\0\0\0\0\0main.o\0\0\0\0\0\0q.c\0\0\0\0\0\0\0\0\0run234\0\0\0\0\0\0stuff/
.De
Note that \fC*(x)\fP and \fC*(*)\fP both match executables.
\fC*(X)\fP matches files executable by others, as opposed to
\fC*(x)\fP, which matches files executable by the owner.
\fC*(R)\fP and \fC*(r)\fP match readable files;
\fC*(W)\fP and \fC*(w)\fP, which checks for writable files.
\fC*(W)\fP is especially important, since it checks for world-writable
files:
.Ds
%\0l\0*(w)
bar.o\0\0\0\0\0\0\0foo*\0\0\0\0\0\0\0\0link@\0\0\0\0\0\0\0morestuff/\0\0run123\0\0\0\0\0\0run240
file.h\0\0\0\0\0\0foo.c\0\0\0\0\0\0\0main.h\0\0\0\0\0\0pipe\0\0\0\0\0\0\0\0run2\0\0\0\0\0\0\0\0run303
file.pro\0\0\0\0foo.o\0\0\0\0\0\0\0main.o\0\0\0\0\0\0q.c\0\0\0\0\0\0\0\0\0run234\0\0\0\0\0\0stuff/
%\0l\0*(W)
link@\0\0\0run240
%\0l\0-l\0link\0run240
lrwxrwxrwx\0\01\0pfalstad\0\0\0\0\0\0\010\0May\023\018:12\0link\0->\0/usr/bin/
-rw-rw-rw-\0\01\0pfalstad\0\0\0\0\0\0\0\00\0May\023\018:12\0run240
.De
If you want to have all the files of a certain type as well as all
symbolic links pointing to files of that type, prefix the qualifier
with a \fC-\fP:
.Ds
%\0l\0*(-/)
link@\0\0\0\0\0\0\0morestuff/\0\0stuff/
.De
You can filter out the symbolic links with the \fC^\fP character:
.Ds
%\0l\0*(W^@)
run240
%\0l\0*(x)
foo*\0\0\0\0\0\0\0\0link@\0\0\0\0\0\0\0morestuff/\0\0stuff/
%\0l\0*(x^@/)
foo*
.De
To find all plain files, you can use \fC.\fP:
.Ds
%\0l\0*(.)
Makefile\0\0file.h\0\0\0\0foo*\0\0\0\0\0\0foo.o\0\0\0\0\0main.o\0\0\0\0run123\0\0\0\0run234\0\0\0\0run303
bar.o\0\0\0\0\0file.pro\0\0foo.c\0\0\0\0\0main.h\0\0\0\0q.c\0\0\0\0\0\0\0run2\0\0\0\0\0\0run240\0\0\0\0sub
%\0l\0*(^.)
link@\0\0\0\0\0\0\0morestuff/\0\0pipe\0\0\0\0\0\0\0\0stuff/
%\0l\0s*(.)
stuff/\0\0\0sub
%\0l\0*(p)
pipe
%\0l\0-l\0*(p)
prw-r--r--\0\01\0pfalstad\0\0\0\0\0\0\0\00\0May\023\018:12\0pipe
.De
\fC*(U)\fP matches all files owned by you.
To search for all files not owned by you, use \fC*(^U)\fP:
.Ds
%\0l\0-l\0*(^U)
-rw-------\0\01\0subbarao\0\0\0\0\0\0\029\0May\023\018:13\0sub
.De
This searches for setuid files:
.Ds
%\0l\0-l\0*(s)
-rwsr-xr-x\0\01\0pfalstad\0\0\0\0\0\0\016\0May\023\018:12\0foo*
.De
This checks for a certain user's files:
.Ds
%\0l\0-l\0*(u[subbarao])
-rw-------\0\01\0subbarao\0\0\0\0\0\0\029\0May\023\018:13\0sub
.De
.Sh "Startup Files"
.PP
There are five startup files that \fBzsh\fP will read commands from:
.Ds
$ZDOTDIR/.zshenv
$ZDOTDIR/.zprofile
$ZDOTDIR/.zshrc
$ZDOTDIR/.zlogin
$ZDOTDIR/.zlogout
.De
If \fBZDOTDIR\fP is not set, then the value of \fBHOME\fP is used;
this is the usual case.
.\".KE    <--- missing .KS or .KF above
.PP
\&\fC.zshenv\fP is sourced on all invocations of the shell,
unless the \fC-f\fP option is set.  It should contain commands to set
the command search path, plus other important environment
variables.
\&\fC.zshenv\fP should not contain commands that produce output
or assume the shell is attached to a tty.
.PP
\&\fC.zshrc\fP is sourced in interactive shells.  It should contain
commands to set up aliases, functions, options, key bindings, etc.
.PP
\&\fC.zlogin\fP is sourced in login shells.  It should contain
commands that should be executed only in login shells.
\&\fC.zlogout\fP is sourced when login shells exit.
\&\fC.zprofile\fP is similar to \fC.zlogin\fP, except that it is sourced before
\&\fC.zshrc\fP.
\&\fC.zprofile\fP is meant as an alternative to \fC.zlogin\fP for
ksh fans;
the two are not intended to be used together, although this
could certainly be done if desired.
\&\fC.zlogin\fP is not the place for alias definitions, options, environment
variable settings, etc.;
as a general rule, it should not change the shell environment
at all.  Rather, it should be used to set the terminal type
and run a series of external commands (\fCfortune\fP, \fCmsgs\fP, etc).
.Sh "Shell Functions"
.PP
\fBzsh\fP also allows you to create your own commands by defining shell
functions.  For example:
.Ds
%\0yp\0()\0{
>\0\0\0\0\0\0\0ypmatch\0$1\0passwd.byname
>\0}
%\0yp\0pfalstad
pfalstad:*:3564:35:Paul\0John\0Falstad:/u/pfalstad:/usr/princeton/bin/zsh
.De
This function looks up a user in the NIS password map.
The \fC$1\fP expands to the first argument to \fCyp\fP.
The function could have been equivalently defined in one of the following
ways:
.Ds
%\0function\0yp\0{
>\0\0\0\0\0\0\0ypmatch\0$1\0passwd.byname
>\0}
%\0function\0yp\0()\0{
>\0\0\0\0\0\0\0ypmatch\0$1\0passwd.byname
>\0}
%\0function\0yp\0()\0ypmatch\0$1\0passwd.byname
.De
Note that aliases are expanded when the function definition is
parsed, not when the function is executed.  For example:
.Ds
%\0alias\0ypmatch=echo
%\0yp\0pfalstad
pfalstad:*:3564:35:Paul\0John\0Falstad:/u/pfalstad:/usr/princeton/bin/zsh
.De
Since the alias was defined after the function was parsed, it has
no effect on the function's execution.
However, if we define the function again with the alias in place:
.Ds
%\0function\0yp\0()\0{\0ypmatch\0$1\0passwd.byname\0}
%\0yp\0pfalstad
pfalstad\0passwd.byname
.De
it is parsed with the new alias definition in place.
Therefore, in general you must define aliases before functions.
.\".KE    <--- missing .KS or .KF above
.PP
We can make the function take multiple arguments:
.Ds
%\0unalias\0ypmatch
%\0yp\0()\0{
>\0\0\0\0\0\0\0for\0i
>\0\0\0\0\0\0\0do\0ypmatch\0$i\0passwd.byname
>\0\0\0\0\0\0\0done
>\0}
%\0yp\0pfalstad\0subbarao\0sukthnkr
pfalstad:*:3564:35:Paul\0John\0Falstad:/u/pfalstad:/usr/princeton/bin/zsh
subbarao:*:3338:35:Kartik\0Subbarao:/u/subbarao:/usr/princeton/bin/zsh
sukthnkr:*:1267:35:Rahul\0Sukthankar:/u/sukthnkr:/usr/princeton/bin/tcsh
.De
The \fCfor i\fP loops through each of the function's arguments,
setting \fCi\fP equal to each of them in turn.
We can also make the function do something sensible
if no arguments are given:
.Ds
%\0yp\0()\0{
>\0\0\0\0\0\0\0if\0((\0$#\0==\00\0))
>\0\0\0\0\0\0\0then\0echo\0usage:\0yp\0name\0...;\0fi
>\0\0\0\0\0\0\0for\0i;\0do\0ypmatch\0$i\0passwd.byname;\0done
>\0}
%\0yp
usage:\0yp\0name\0...
%\0yp\0pfalstad\0sukthnkr
pfalstad:*:3564:35:Paul\0John\0Falstad:/u/pfalstad:/usr/princeton/bin/zsh
sukthnkr:*:1267:35:Rahul\0Sukthankar:/u/sukthnkr:/usr/princeton/bin/tcsh
.De
\fC$#\fP is the number of arguments supplied to the function.
If it is equal to zero, we print a usage message; otherwise,
we loop through the arguments, and \fCypmatch\fP all of them.
.\".KE    <--- missing .KS or .KF above
.PP
Here's a function that selects a random line from a file:
.Ds
%\0randline\0()\0{
>\0\0\0\0\0\0\0integer\0z=$(wc\0-l\0<$1)
>\0\0\0\0\0\0\0sed\0-n\0$[RANDOM\0%\0z\0+\01]p\0$1
>\0}
%\0randline\0/etc/motd
PHOENIX\0WILL\0BE\0DOWN\0briefly\0Friday\0morning,\05/24/91\0from\08\0AM\0to
%\0randline\0/etc/motd
SunOS\0Release\04.1.1\0(PHOENIX)\0#19:\0Tue\0May\014\019:03:15\0EDT\01991
%\0randline\0/etc/motd
|\0Please\0use\0the\0"msgs"\0command\0to\0read\0announcements.\0\0Refer\0to\0the\0\0\0|
%\0echo\0$z

%
.De
\fCrandline\fP has a local variable, \fCz\fP, that holds the number of
lines in the file.  \fC$[RANDOM % z + 1]\fP expands to a random number
between 1 and \fCz\fP.  An expression of the form \fC$[\fR...\fC]\fR
expands to the value of the arithmetic expression within the brackets,
and the \fBRANDOM\fP variable returns a random number each time it
is referenced.  \fC%\fP is the modulus operator, as in C.
Therefore, \fCsed -n $[RANDOM%z+1]p\fP picks a random line from its
input, from 1 to \fCz\fP.
.PP
Function definitions can be viewed with the \fCfunctions\fP builtin:
.Ds
%\0functions\0randline
randline\0()\0{
\0\0\0\0\0\0\0\0integer\0z=$(wc\0-l\0<$1)
\0\0\0\0\0\0\0\0sed\0-n\0$[RANDOM\0%\0z\0+\01]p\0$1

}
%\0functions
yp\0()\0{
\0\0\0\0\0\0\0\0if\0let\0$#\0==\00\0
\0\0\0\0\0\0\0\0
\0\0\0\0\0\0\0\0then
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0echo\0usage:\0yp\0name\0...
\0\0\0\0\0\0\0\0
\0\0\0\0\0\0\0\0fi
\0\0\0\0\0\0\0\0for\0i
\0\0\0\0\0\0\0\0do
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0ypmatch\0$i\0passwd.byname
\0\0\0\0\0\0\0\0
\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0done

}
randline\0()\0{
\0\0\0\0\0\0\0\0integer\0z=$(wc\0-l\0<$1)
\0\0\0\0\0\0\0\0sed\0-n\0$[RANDOM\0%\0z\0+\01]p\0$1

}
.De
Here's another one:
.Ds
%\0cx\0()\0{\0chmod\0+x\0$*\0}
%\0ls\0-l\0foo\0bar
-rw-r--r--\0\01\0pfalstad\0\0\0\0\0\0\029\0May\024\004:38\0bar
-rw-r--r--\0\01\0pfalstad\0\0\0\0\0\0\029\0May\024\004:38\0foo
%\0cx\0foo\0bar
%\0ls\0-l\0foo\0bar
-rwxr-xr-x\0\01\0pfalstad\0\0\0\0\0\0\029\0May\024\004:38\0bar
-rwxr-xr-x\0\01\0pfalstad\0\0\0\0\0\0\029\0May\024\004:38\0foo
.De
Note that this could also have been implemented as an alias:
.Ds
%\0chmod\0644\0foo\0bar
%\0alias\0cx='chmod\0+x'
%\0cx\0foo\0bar
%\0ls\0-l\0foo\0bar
-rwxr-xr-x\0\01\0pfalstad\0\0\0\0\0\0\029\0May\024\004:38\0bar
-rwxr-xr-x\0\01\0pfalstad\0\0\0\0\0\0\029\0May\024\004:38\0foo
.De
.PP
Instead of defining a lot of functions in your \fC.zshrc\fP,
all of which you may not use,
it is often better to use the \fCautoload\fP builtin.
The idea is, you create a directory where function
definitions are stored, declare the names in
your \fC.zshrc\fP, and tell the shell where to look for them.
Whenever you reference a function, the shell
will automatically load it into memory.
.Ds
%\0mkdir\0/tmp/funs
%\0cat\0>/tmp/funs/yp
ypmatch\0$1\0passwd.byname
^D
%\0cat\0>/tmp/funs/cx
chmod\0+x\0$*
^D
%\0FPATH=/tmp/funs
%\0autoload\0cx\0yp
%\0functions\0cx\0yp
undefined\0cx\0()
undefined\0yp\0()
%\0chmod\0755\0/tmp/funs/{cx,yp}
%\0yp\0egsirer
egsirer:*:3214:35:Emin\0Gun\0Sirer:/u/egsirer:/bin/sh
%\0functions\0yp
yp\0()\0{
\0\0\0\0\0\0\0\0ypmatch\0$1\0passwd.byname
}
.De
This idea has other benefits.  By adding a \fC#!\fP header
to the files, you can make them double as shell scripts.
(Although it is faster to use them as functions, since a
separate process is not created.)
.Ds
%\0ed\0/tmp/funs/yp
25
i
#!\0/usr/local/bin/zsh
.
w
42
q
%\0</tmp/funs/yp
#!\0/usr/local/bin/zsh
ypmatch\0$1\0passwd.byname
%\0/tmp/funs/yp\0sukthnkr
sukthnkr:*:1267:35:Rahul\0Sukthankar:/u/sukthnkr:/usr/princeton/bin/tcsh
.De
Now other people, who may not use \fBzsh\fP, or who don't want to
copy all of your \fC.zshrc\fP, may use these functions as shell
scripts.
.Sh "Directories"
.PP
One nice feature of \fBzsh\fP is the way it prints directories.
For example, if we set the prompt like this:
.Ds
phoenix%\0PROMPT='%~>\0'
~>\0cd\0src
~/src>
.De
the shell will print the current directory in the prompt,
using the \fC~\fP character.
However, \fBzsh\fP is smarter than most other shells in this respect:
.Ds
~/src>\0cd\0~subbarao
~subbarao>\0cd\0~maruchck
~maruchck>\0cd\0lib
~maruchck/lib>\0cd\0fun
~maruchck/lib/fun>\0foo=/usr/princeton/common/src
~maruchck/lib/fun>\0cd\0~foo
~foo>\0cd\0..
/usr/princeton/common>\0cd\0src
~foo>\0cd\0news/nntp
~foo/news/nntp>\0cd\0inews
~foo/news/nntp/inews>
.De
Note that \fBzsh\fP prints \fIother\fP users' directories 
in the form \fC~user\fP.  Also note that you can
set a parameter and use it as a directory name;
\fBzsh\fP will act as if \fCfoo\fP is a user
with the login directory \fC/usr/princeton/common/src\fP.
This is convenient, especially if you're sick of seeing
prompts like this:
.Ds
phoenix:/usr/princeton/common/src/X.V11R4/contrib/clients/xv/docs>
.De
If you get stuck in this position, you can give the current
directory a short name, like this:
.Ds
/usr/princeton/common/src/news/nntp/inews>\0inews=$PWD
/usr/princeton/common/src/news/nntp/inews>\0echo\0~inews
/usr/princeton/common/src/news/nntp/inews
~inews>
.De
When you reference a directory in the form \fC~inews\fP,
the shell assumes that you want the directory displayed
in this form; thus simply typing \fCecho ~inews\fP or
\fCcd ~inews\fP causes the prompt to be shortened.
You can define a shell function for this purpose:
.Ds
~inews>\0namedir\0()\0{\0$1=$PWD\0;\0\0:\0~$1\0}
~inews>\0cd\0/usr/princeton/bin
/usr/princeton/bin>\0namedir\0pbin
~pbin>\0cd\0/var/spool/mail
/var/spool/mail>\0namedir\0spool
~spool>\0cd\0.msgs
~spool/.msgs>
.De
You may want to add this one-line function to your \fC.zshrc\fP.

\fBzsh\fP can also put the current directory in your title bar,
if you are using a windowing system.
One way to do this is with the \fCchpwd\fP function, which is
automatically executed by the shell whenever you change
directory.  If you are using xterm, this will work:
.Ds
chpwd\0()\0{\0print\0-Pn\0'^[]2;%~^G'\0}
.De
The \fC-P\fP option tells \fCprint\fP to treat its arguments like a prompt
string; otherwise the \fC%~\fP would not be expanded.
The \fC-n\fP option suppresses the terminating newline, as with \fCecho\fP.
.PP
If you are using an IRIS \fCwsh\fP, do this:
.Ds
chpwd\0()\0{\0print\0-Pn\0'\e2201.y%~\e234'\0}
.De
The \fCprint -D\fP command has other uses.  For example, to
print the current directory to standard output in short form,
you can do this:
.Ds
%\0print\0-D\0$PWD
~subbarao/src
.De
and to print each component of the path in short form:
.Ds
%\0print\0-D\0$path
/bin\0/usr/bin\0~locbin\0~locbin/X11\0~/bin
.De
.Sh "Directory Stacks"
.PP
If you use csh, you may know about directory stacks.
The \fCpushd\fP command puts the current directory on the
stack, and changes to a new directory; the \fCpopd\fP command
pops a directory off the stack and changes to it.
.Ds
phoenix%\0cd\0
phoenix%\0PROMPT='Z\0%~>\0'
Z\0~>\0pushd\0/tmp
/tmp\0~
Z\0/tmp>\0pushd\0/usr/etc
/usr/etc\0/tmp\0~
Z\0/usr/etc>\0pushd\0/usr/bin
/usr/bin\0/usr/etc\0/tmp\0~
Z\0/usr/bin>\0popd
/usr/etc\0/tmp\0~
Z\0/usr/etc>\0popd
/tmp\0~
Z\0/tmp>\0pushd\0/etc
/etc\0/tmp\0~
Z\0/etc>\0popd\0
/tmp\0~
.De
\fBzsh\fP's directory stack commands work similarly.  One
difference is the way \fCpushd\fP is handled if no arguments
are given.  As in csh, this exchanges the top two elements
of the directory stack:
.Ds
Z\0/tmp>\0dirs
/tmp\0~
Z\0/tmp>\0pushd
~\0/tmp
.De
unless the stack only has one entry:
.Ds
Z\0~>\0popd
/tmp
Z\0/tmp>\0dirs
/tmp
Z\0/tmp>\0pushd
~\0/tmp
Z\0~>
.De
or unless the \fIPUSHDTOHOME\fP option is set:
.Ds
Z\0~>\0setopt\0pushdtohome
Z\0~>\0pushd
~\0~\0/tmp
.De
.PP
As an alternative to using directory stacks in this manner,
we can get something like a \fIdirectory history\fP
by setting a few more options and parameters:
.Ds
~>\0DIRSTACKSIZE=8
~>\0setopt\0autopushd\0pushdminus\0pushdsilent\0pushdtohome
~>\0alias\0dh='dirs\0-v'
~>\0cd\0/tmp
/tmp>\0cd\0/usr
/usr>\0cd\0bin
/usr/bin>\0cd\0../pub
/usr/pub>\0dh
0\0\0\0\0\0\0\0/usr/pub
1\0\0\0\0\0\0\0/usr/bin
2\0\0\0\0\0\0\0/usr
3\0\0\0\0\0\0\0/tmp
4\0\0\0\0\0\0\0~
/usr/pub>\0cd\0-3
/tmp>\0dh
0\0\0\0\0\0\0\0/tmp
1\0\0\0\0\0\0\0/usr/pub
2\0\0\0\0\0\0\0/usr/bin
3\0\0\0\0\0\0\0/usr
4\0\0\0\0\0\0\0~
/tmp>\0ls\0~2/df
/usr/bin/df
/tmp>\0cd\0-4
~>
.De
Note that \fC~2\fP expanded to the second directory in the
history list, and that \fCcd -3\fP recalled the third
directory in the list.
.PP
You may be wondering what all those options do.
\fIAUTOPUSHD\fP made \fCcd\fP act like \fCpushd\fP.
(\fCalias cd=pushd\fP is not sufficient, for various reasons.)
\fIPUSHDMINUS\fP swapped the meaning of \fCcd +1\fP and
\fCcd -1\fP; we want them to mean the opposite of what they mean in csh,
because it makes more sense in this scheme, and it's easier to type:
.Ds
~>\0dh
0\0\0\0\0\0\0\0~
1\0\0\0\0\0\0\0/tmp
2\0\0\0\0\0\0\0/usr/pub
3\0\0\0\0\0\0\0/usr/bin
4\0\0\0\0\0\0\0/usr
~>\0unsetopt\0pushdminus
~>\0cd\0+1
/tmp>\0dh
0\0\0\0\0\0\0\0/tmp
1\0\0\0\0\0\0\0~
2\0\0\0\0\0\0\0/usr/pub
3\0\0\0\0\0\0\0/usr/bin
4\0\0\0\0\0\0\0/usr
/tmp>\0cd\0+2
/usr/pub>
.De
\fIPUSHDSILENT\fP keeps the shell from printing
the directory stack each time we do a \fCcd\fP,
and \fIPUSHDTOHOME\fP we mentioned earlier:
.Ds
/usr/pub>\0unsetopt\0pushdsilent
/usr/pub>\0cd\0/etc
/etc\0/usr/pub\0/tmp\0~\0/usr/bin\0/usr
/etc>\0cd
~\0/etc\0/usr/pub\0/tmp\0~\0/usr/bin\0/usr
~>\0unsetopt\0pushdtohome
~>\0cd
/etc\0~\0/usr/pub\0/tmp\0~\0/usr/bin\0/usr
/etc>
.De
\fBDIRSTACKSIZE\fP keeps the directory stack
from getting too large, much like \fIHISTSIZE\fP:
.Ds
/etc>\0setopt\0pushdsilent
/etc>\0cd\0/
/>\0cd\0/
/>\0cd\0/
/>\0cd\0/
/>\0cd\0/
/>\0cd\0/
/>\0cd\0/
/>\0cd\0/
/>\0dh
0\0\0\0\0\0\0\0/
1\0\0\0\0\0\0\0/
2\0\0\0\0\0\0\0/
3\0\0\0\0\0\0\0/
4\0\0\0\0\0\0\0/
5\0\0\0\0\0\0\0/
6\0\0\0\0\0\0\0/
7\0\0\0\0\0\0\0/
.De
.Sh "Command/Process Substitution"
.PP
Command substitution in \fBzsh\fP can take two forms.
In the traditional form, a command enclosed in
backquotes (\fC`\fP...\fC`\fP) is replaced on the command line with its output.
This is the form used by the older shells.
Newer shells (like \fBzsh\fP) also provide another form,
\fC$(\fR...\fC)\fR.  This form is much easier to nest.
.Ds
%\0ls\0-l\0`echo\0/vmunix`
-rwxr-xr-x\0\01\0root\0\0\0\0\0\01209702\0May\014\019:04\0/vmunix
%\0ls\0-l\0$(echo\0/vmunix)
-rwxr-xr-x\0\01\0root\0\0\0\0\0\01209702\0May\014\019:04\0/vmunix
%\0who\0|\0grep\0mad
subbarao\0ttyt7\0\0\0May\023\015:02\0\0\0(mad55sx15.Prince)
pfalstad\0ttyu1\0\0\0May\023\016:25\0\0\0(mad55sx14.Prince)
subbarao\0ttyu6\0\0\0May\023\015:04\0\0\0(mad55sx15.Prince)
pfalstad\0ttyv3\0\0\0May\023\016:25\0\0\0(mad55sx14.Prince)
%\0who\0|\0grep\0mad\0|\0awk\0'{print\0$2}'
ttyt7
ttyu1
ttyu6
ttyv3
%\0cd\0/dev;\0ls\0-l\0$(who\0|
>\0grep\0$(echo\0mad)\0|
>\0awk\0'{\0print\0$2\0}')
crwx-w----\0\01\0subbarao\0\020,\0\071\0May\023\018:35\0ttyt7
crw--w----\0\01\0pfalstad\0\020,\0\081\0May\023\018:42\0ttyu1
crwx-w----\0\01\0subbarao\0\020,\0\086\0May\023\018:38\0ttyu6
crw--w----\0\01\0pfalstad\0\020,\0\099\0May\023\018:41\0ttyv3
.De
Many common uses of command substitution, however, are
superseded by other mechanisms of \fBzsh\fP:
.Ds
%\0ls\0-l\0`tty`
crw-rw-rw-\0\01\0root\0\0\0\0\0\020,\0\028\0May\023\018:35\0/dev/ttyqc
%\0ls\0-l\0$TTY
crw-rw-rw-\0\01\0root\0\0\0\0\0\020,\0\028\0May\023\018:35\0/dev/ttyqc
%\0ls\0-l\0`which\0rn`
-rwxr-xr-x\0\01\0root\0\0\0\0\0\0\0172032\0Mar\0\06\018:40\0/usr/princeton/bin/rn
%\0ls\0-l\0=rn
-rwxr-xr-x\0\01\0root\0\0\0\0\0\0\0172032\0Mar\0\06\018:40\0/usr/princeton/bin/rn
.De
A command name with a \fC=\fP prepended is replaced with its full
pathname.  This can be very convenient.  If it's not convenient
for you, you can turn it off:
.Ds
%\0ls
=foo\0\0\0\0=bar
%\0ls\0=foo\0=bar
zsh:\0foo\0not\0found
%\0setopt\0noequals
%\0ls\0=foo\0=bar
=foo\0\0\0\0=bar
.De
.PP
Another nice feature is process substitution:
.Ds
%\0who\0|\0fgrep\0-f\0=(print\0-l\0root\0lemke\0shgchan\0subbarao)
root\0\0\0\0\0console\0May\019\010:41
lemke\0\0\0\0ttyq0\0\0\0May\022\010:05\0\0\0(narnia:0.0)
lemke\0\0\0\0ttyr7\0\0\0May\022\010:05\0\0\0(narnia:0.0)
lemke\0\0\0\0ttyrd\0\0\0May\022\010:05\0\0\0(narnia:0.0)
shgchan\0\0ttys1\0\0\0May\023\016:52\0\0\0(gaudi.Princeton.)
subbarao\0ttyt7\0\0\0May\023\015:02\0\0\0(mad55sx15.Prince)
subbarao\0ttyu6\0\0\0May\023\015:04\0\0\0(mad55sx15.Prince)
shgchan\0\0ttyvb\0\0\0May\023\016:51\0\0\0(gaudi.Princeton.)
.De
A command of the form \fC=(\fR...\fC)\fR is replaced with the name of a \fIfile\fP
containing its output.  (A command substitution, on the other
hand, is replaced with the output itself.)
\fCprint -l\fP is like \fCecho\fP, excepts that it prints its arguments
one per line, the way \fCfgrep\fP expects them:
.Ds
%\0print\0-l\0foo\0bar
foo
bar
.De
We could also have written:
.Ds
%\0who\0|\0fgrep\0-f\0=(echo\0'root
>\0lemke
>\0shgchan
>\0subbarao')
.De
Using\0process\0substitution,
you\0can\0edit\0the\0output\0of\0a\0command:
.Ds
%\0ed\0=(who\0|\0fgrep\0-f\0~/.friends)
355
g/lemke/d
w\0/tmp/filbar
226
q
%\0cat\0/tmp/filbar
root\0\0\0\0\0console\0May\019\010:41
shgchan\0\0ttys1\0\0\0May\023\016:52\0\0\0(gaudi.Princeton.)
subbarao\0ttyt7\0\0\0May\023\015:02\0\0\0(mad55sx15.Prince)
subbarao\0ttyu6\0\0\0May\023\015:04\0\0\0(mad55sx15.Prince)
shgchan\0\0ttyvb\0\0\0May\023\016:51\0\0\0(gaudi.Princeton.)
.De
or easily read archived mail:
.Ds
%\0mail\0-f\0=(zcat\0~/mail/oldzshmail.Z)
"/tmp/zsha06024":\084\0messages,\00\0new,\043\0unread
>\0\01\0\0U\0\0TO:\0pfalstad,\0zsh\0(10)
\0\0\02\0\0U\0\0nytim!tim@uunet.uu.net,\0Re:\0Zsh\0on\0Sparc1\0/SunOS\04.0.3
\0\0\03\0\0U\0\0JAM%TPN@utrcgw.utc.com,\0zsh\0fix\0(15)
\0\0\04\0\0U\0\0djm@eng.umd.edu,\0way\0to\0find\0out\0if\0running\0zsh?\0(25)
\0\0\05\0\0U\0\0djm@eng.umd.edu,\0Re:\0way\0to\0find\0out\0if\0running\0zsh?\0(17)
\0\0\06\0\0\0r\0djm@eng.umd.edu,\0Meta\0.\0(18)
\0\0\07\0\0U\0\0jack@cs.glasgow.ac.uk,\0Re:\0problem\0building\0zsh\0(147)
\0\0\08\0\0U\0\0nytim!tim@uunet.uu.net,\0Re:\0Zsh\0on\0Sparc1\0/SunOS\04.0.3
\0\0\09\0\0\0\0\0ursa!jmd,\0Another\0fix...\0(61)
\0\010\0\0U\0\0pplacewa@bbn.com,\0Re:\0v18i084:\0Zsh\02.00\0-\0A\0small\0complaint\0(36)
\0\011\0\0U\0\0lubkin@cs.rochester.edu,\0POSIX\0job\0control\0(34)
\0\012\0\0U\0\0yale!bronson!tan@uunet.UU.NET
\0\013\0\0U\0\0brett@rpi.edu,\0zsh\0(36)
\0\014\0\0S\0\0subbarao,\0zsh\0sucks!!!!\0(286)
\0\015\0\0U\0\0snibru!d241s008!d241s013!ala@relay.EU.net,\0zsh\0(165)
\0\016\0\0U\0\0nytim!tim@uunet.UU.NET,\0Re:\0Zsh\0on\0Sparc1\0/SunOS\04.0.3
\0\017\0\0U\0\0subbarao,\0zsh\0is\0a\0junk\0shell\0(43)
\0\018\0\0U\0\0amaranth@vela.acs.oakland.edu,\0zsh\0(33)
43u/84\01:\0x
%\0ls\0-l\0/tmp/zsha06024
/tmp/zsha06024\0not\0found
.De
Note that the shell creates a temporary file, and deletes it
when the command is finished.
.Ds
%\0diff\0=(ls)\0=(ls\0-F)
3c3
<\0fortune
---
>\0fortune*
10c10
<\0strfile
---
>\0strfile*
.De
If you read \fBzsh\fP's man page, you may notice that \fC<(\fR...\fC)\fR
is another form of process substitution which is similar to
\fC=(\fR...\fC)\fR.
There is an important difference between the two.
In the \fC<(\fR...\fC)\fR case, the shell creates a named pipe (FIFO)
instead of a file.  This is better, since it does not
fill up the file system; but it does not work in all cases.
In fact, if we had replaced \fC=(\fR...\fC)\fR with \fC<(\fR...\fC)\fR in 
the examples above, all of them would have stopped working
except for \fCfgrep -f <(\fR...\fC)\fR.
You can not edit a pipe, or open it as a mail folder;
\fCfgrep\fP, however, has no problem with reading
a list of words from a pipe.
You may wonder why \fCdiff <(foo) bar\fP doesn't work, since
\fCfoo | diff - bar\fP works; this is because \fCdiff\fP creates
a temporary file if it notices that one of its arguments
is \fC-\fP, and then copies its standard input to the temporary
file.
.PP
\fC>(\fR...\fC)\fR is just like \fC<(\fR...\fC)\fR except that the
command between the parentheses will get its input from the named
pipe.
.Ds
%\0dvips\0-o\0>(lpr)\0zsh.dvi
.De
.Sh "Redirection"
.PP
Apart from all the regular redirections like the Bourne shell has,
\fBzsh\fP can do more.  You can send the output of a command to more
than one file, by specifying more redirections like
.Ds
%\0echo\0Hello\0World\0>file1\0>file2
.De
and the text will end up in both files.  Similarly, you can send the
output to a file and into a pipe:
.Ds
%\0make\0>\0make.log\0|\0grep\0Error
.De
The same goes for input.  You can make the input of a command come
from more than one file.
.Ds
%\0sort\0<file1\0<file2\0<file3
.De
The command will first get the contents of file1 as its standard
input, then those of file2 and finally the contents of file3.  This,
too, works with pipes.
.Ds
%\0cut\0-d:\0-f1\0/etc/passwd\0|\0sort\0<newnames
.De
The sort will get as its standard input first the output of \fCcut\fP
and then the contents of \fCnewnames\fP.
.PP
Suppose you would like to watch the standard output of a command on
your terminal, but want to pipe the standard error to another command.
An easy way to do this in \fBzsh\fP is by redirecting the standard
error using \fC2> >(\fR...\fC)\fR.
.Ds
%\0find\0/\0-name\0games\02>\0>(grep\0-v\0'Permission'\0>\0realerrors)
.De
The above redirection will actually be implemented with a regular
pipe, not a temporary named pipe.
.Sh "Aliasing"
.PP
Often-used commands can be abbreviated with an alias:
.Ds
%\0alias\0uc=uncompress
%\0ls
hanoi.Z
%\0uc\0hanoi
%\0ls
hanoi
.De
or commands with certain desired options:
.Ds
%\0alias\0fm='finger\0-m'
%\0fm\0root
Login\0name:\0root\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0In\0real\0life:\0Operator
Directory:\0/\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0Shell:\0/bin/csh
On\0since\0May\019\010:41:15\0on\0console\0\0\0\0\03\0days\05\0hours\0Idle\0Time
No\0unread\0mail
No\0Plan.

%\0alias\0lock='lock\0-p\0-60000'
%\0lock
lock:\0/dev/ttyr4\0on\0phoenix.\0timeout\0in\060000\0minutes
time\0now\0is\0Fri\0May\024\004:23:18\0EDT\01991
Key:\0

%\0alias\0l='ls\0-AF'
%\0l\0/
\&.bash_history\0\0\0\0\0\0\0\0\0\0\0\0\0\0kadb*
\&.bashrc\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0lib@
\&.cshrc\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0licensed/
\&.exrc\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0lost+found/
\&.login\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0macsyma
\&\fR...
.De
Aliases can also be used to replace old commands:
.Ds
%\0alias\0grep=egrep\0ps=sps\0make=gmake
%\0alias\0whoami='echo\0root'
%\0whoami
root
.De
or to define new ones:
.Ds
%\0cd\0/
%\0alias\0sz='ls\0-l\0|\0sort\0-n\0+3\0|\0tail\0-10'
%\0sz
drwxr-sr-x\0\07\0bin\0\0\0\0\0\0\0\0\0\03072\0May\023\011:59\0etc
drwxrwxrwx\026\0root\0\0\0\0\0\0\0\0\05120\0May\024\004:20\0tmp
drwxr-xr-x\0\02\0root\0\0\0\0\0\0\0\0\08192\0Dec\026\019:34\0lost+found
drwxr-sr-x\0\02\0bin\0\0\0\0\0\0\0\0\014848\0May\023\018:48\0dev
-r--r--r--\0\01\0root\0\0\0\0\0\0\0140520\0Dec\026\020:08\0boot
-rwxr-xr-x\0\01\0root\0\0\0\0\0\0\0311172\0Dec\026\020:08\0kadb
-rwxr-xr-x\0\01\0root\0\0\0\0\0\01209695\0Apr\016\015:33\0vmunix.old
-rwxr-xr-x\0\01\0root\0\0\0\0\0\01209702\0May\014\019:04\0vmunix
-rwxr-xr-x\0\01\0root\0\0\0\0\0\01209758\0May\021\012:23\0vmunix.new.kernelmap.old
-rwxr-xr-x\0\01\0root\0\0\0\0\0\01711848\0Dec\026\020:08\0vmunix.org
%\0cd
%\0alias\0rable='ls\0-AFtrd\0*(R)'\0nrable='ls\0-AFtrd\0*(^R)'
%\0rable
README\0\0\0\0\0\0func/\0\0\0\0\0\0\0bin/\0\0\0\0\0\0\0\0pub/\0\0\0\0\0\0\0\0News/\0\0\0\0\0\0\0src/
nicecolors\0\0etc/\0\0\0\0\0\0\0\0scr/\0\0\0\0\0\0\0\0tmp/\0\0\0\0\0\0\0\0iris/\0\0\0\0\0\0\0zsh*
%\0nrable
Mailboxes/\0\0mail/\0\0\0\0\0\0\0notes
.De
(The pattern \fC*(R)\fP matches all readable files in the current
directory, and \fC*(^R)\fP matches all unreadable files.)
.PP
Most other shells have aliases of this kind (\fIcommand\fP aliases).
However, \fBzsh\fP also has \fIglobal\fP aliases, which are substituted
anywhere on a line.
Global aliases can be used to abbreviate frequently-typed
usernames, hostnames, etc.
.Ds
%\0alias\0-g\0me=pfalstad\0gun=egsirer\0mjm=maruchck
%\0who\0|\0grep\0me
pfalstad\0ttyp0\0\0\0May\024\003:39\0\0\0(mickey.Princeton)
pfalstad\0ttyp5\0\0\0May\024\003:42\0\0\0(mickey.Princeton)
%\0fm\0gun
Login\0name:\0egsirer\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0In\0real\0life:\0Emin\0Gun\0Sirer
Directory:\0/u/egsirer\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0Shell:\0/bin/sh
Last\0login\0Thu\0May\023\019:05\0on\0ttyq3\0from\0bow.Princeton.ED
New\0mail\0received\0Fri\0May\024\002:30:28\01991;
\0\0unread\0since\0Fri\0May\024\002:30:27\01991
%\0alias\0-g\0phx=phoenix.princeton.edu\0warc=wuarchive.wustl.edu
%\0ftp\0warc
Connected\0to\0wuarchive.wustl.edu.
.De
Here are some more interesting uses.
.Ds
%\0alias\0-g\0M='|\0more'\0GF='|\0fgrep\0-f\0~/.friends'
%\0who\0M\0\0\0#\0\fIpipes\0the\0output\0of\0\fCwho\fI\0through\0\fCmore
%\0who\0GF\0\0#\0\fIsee\0if\0your\0friends\0are\0on\fC
%\0w\0GF\0\0\0\0#\0\fIsee\0what\0your\0friends\0are\0doing
.De
Another example makes use of \fBzsh\fP's process substitution.
If you run NIS, and you miss being able to do this:
.Ds
%\0grep\0pfalstad\0/etc/passwd
.De
you can define an alias that will seem more natural
than \fCypmatch pfalstad passwd\fP:
.Ds
%\0alias\0-g\0PASS='<(ypcat\0passwd)'
%\0grep\0pfalstad\0PASS
pfalstad:*:3564:35:Paul\0John\0Falstad:/u/pfalstad:/usr/princeton/bin/zsh
.De
If you're really crazy, you can even call it \fC/etc/passwd\fP:
.Ds
%\0alias\0-g\0/etc/passwd='<(ypcat\0passwd)'
%\0grep\0pfalstad\0/etc/passwd
pfalstad:*:3564:35:Paul\0John\0Falstad:/u/pfalstad:/usr/princeton/bin/zsh
.De
The last example shows one of the perils of global aliases;
they have a lot of potential to cause confusion.
For example, if you defined a global alias called \fC|\fP (which is
possible), \fBzsh\fP would begin to act very strangely; every pipe
symbol would be replaced with the text of your alias.
To some extent, global aliases are like macros in C;
discretion is advised in using them and in choosing names for them.
Using names in all caps is not a bad idea, especially
for aliases which introduce shell metasyntax (like \fCM\fP and \fCGF\fP
above).
.PP
Note that \fBzsh\fP aliases are not like csh aliases.  The syntax for
defining them is different, and they do not have arguments.
All your favorite csh aliases will probably not work under \fBzsh\fP.
For example, if you try:
.Ds
alias\0rm\0mv\0'\e!*\0/tmp/wastebasket'
.De
no aliases will be defined, but \fBzsh\fP will not report an error.
In csh, this line defines an alias that makes \fCrm\fP safe---files
that are \fCrm\fP'd will be moved to a temporary directory instead of
instantly destroyed.  In \fBzsh\fP's syntax, however, this line asks
the shell to print any existing alias definitions for \fCrm\fP,
\fCmv\fP, or \fC!*\ /tmp/wastebasket\fP.  Since there are none, most
likely, the shell will not print anything, although \fCalias\fP will
return a nonzero exit code.  The proper syntax is this:
.Ds
alias\0rm='mv\0\e!*\0/tmp/wastebasket'
.De
However, this won't work either:
.Ds
%\0rm\0foo.dvi
zsh:\0no\0matches\0found:\0!*
.De
While this makes \fCrm\fP safe, it is certainly not what the user
intended.  In \fBzsh\fP, you must use a shell function for this:
.Ds
%\0unalias\0rm
%\0rm\0()\0{\0mv\0$*\0/tmp/wastebasket\0}
%\0rm\0foo.dvi
%\0ls\0/tmp/wastebasket
foo.dvi
.De
While this is much cleaner and easier to read (I hope you will
agree), it is not csh-compatible.  Therefore, a script to convert
csh aliases and variables has been provided.  You should only need to use it
once, to convert all your csh aliases and parameters to \fBzsh\fP format:
.Ds
%\0csh
csh>\0alias
l\0\0\0\0\0\0\0ls\0-AF
more\0\0\0\0less
on\0\0\0\0\0\0last\0-2\0!:1\0;\0who\0|\0grep\0!:1
csh>\0exit
%\0c2z\0>neat_zsh_aliases
%\0cat\0neat_zsh_aliases
alias\0l='ls\0-AF'
alias\0more='less'
on\0()\0{\0last\0-2\0$1\0;\0who\0|\0grep\0$1\0}
\&...
.De
The first two aliases were converted to regular \fBzsh\fP aliases, while
the third, since it needed to handle arguments, was converted to
a function.  \fCc2z\fP can convert most aliases to \fBzsh\fP format without
any problems.  However, if you're using some really arcane csh tricks,
or if you have an alias with a name like \fCdo\fP (which is reserved
in \fBzsh\fP), you may have to fix some of the aliases by hand.
.PP
The \fCc2z\fP script checks your csh setup, and produces a list
of \fBzsh\fP commands which replicate your aliases and parameter settings
as closely as possible.  You could include its output in your
startup file, \fC.zshrc\fP.
.Sh "History"
.PP
There are several ways to manipulate history in \fBzsh\fP.
One way is to use csh-style \fC!\fP history:
.Ds
%\0/usr/local/bin/!:0\0!-2*:s/foo/bar/\0>>!$
.De
If you don't want to use this, you can turn it off
by typing \fCsetopt nobanghist\fP.  If you are afraid of accidentally
executing the wrong command you can set the \fIHISTVERIFY\fP option.
If this option is set, commands that result from history expansion
will not be executed immediately, but will be put back into the editor
buffer for further consideration.
.PP
If you're not familiar with \fC!\fP history, here follows some
explanation.  History substitutions always start with a \fC!\fP,
commonly called \*Qbang\*U.  After the \fC!\fP comes an (optional)
designation of which \*Qevent\*U (command) to use, then a colon, and
then a designation of what word of that command to use.  For example,
\fC!-\fIn\fR refers to the command \fIn\fP commands ago.
.Ds
%\0ls
foo\0\0bar
%\0cd\0foo
%\0!-2
ls
baz\0\0bam
.De
No word designator was used, which means that the whole command
referred to was repeated.  Note that the shell will echo the result of
the history substitution.  The word designator can, among other
things, be a number indicating the argument to use, where \fC0\fP is
the command.
.Ds
%\0/usr/bin/ls\0foo
foo
%\0!:0\0bar
/usr/bin/ls\0bar
bar
.De
In this example, no event designator was used, which tells \fBzsh\fP
to use the previous command.  A \fC$\fP specifies the last argument
.Ds
%\0mkdir\0/usr/local/lib/emacs/site-lisp/calc
%\0cd\0!:$
cd\0/usr/local/lib/emacs/site-lisp/calc
.De
If you use more words of the same command, only the first \fC!\fP
needs an event designator.
.Ds
%\0make\0prig\0>>\0make.log
make:\0***\0No\0rule\0to\0make\0target\0`prig'.\0\0Stop.
%\0cd\0src
%\0!-2:0\0prog\0>>\0!:$
make\0prog\0>>\0make.log
.De
This is different from csh, where a bang with no event designator
always refers to the previous command.  If you actually like this
behaviour, set the \fICSHJUNKIEHISTORY\fP option.
.Ds
%\0setopt\0cshjunkiehistory
%\0!-2:0\0prog2\0>>\0!:$
make\0prog2\0>>\0cshjunkiehistory
.De
Another way to use history is to use the \fCfc\fP command.  For
example, if you type an erroneous command:
.Ds
%\0for\0i\0in\0`cat\0/etc/clients`\0
\0do\0
\0rpu\0$i\0
\0done
zsh:\0command\0not\0found:\0rpu
zsh:\0command\0not\0found:\0rpu
zsh:\0command\0not\0found:\0rpu
\&\fR...
.De
typing \fCfc\fP will execute an editor on this command, allowing
you to fix it.  (The default editor is \fCvi\fP, by the way,
not \fCed\fP).
.Ds
%\0fc
49
/rpu/s//rup/p
\0rup\0$i\0
w
49
q
for\0i\0in\0`cat\0/etc/clients`\0
\0do\0
\0rup\0$i\0
\0done
\0\0\0\0\0\0\0\0beam\0\0\0\0up\0\02\0days,\010:17,\0\0\0\0load\0average:\00.86,\00.80,\00.50
\0\0\0\0\0\0\0\0\0bow\0\0\0\0up\0\04\0days,\0\08:41,\0\0\0\0load\0average:\00.91,\00.80,\00.50
\0\0\0\0\0\0\0\0burn\0\0\0\0up\0\0\0\0\0\0\0\0\0\017:18,\0\0\0\0load\0average:\00.91,\00.80,\00.50
\0\0\0\0\0\0\0burst\0\0\0\0up\0\09\0days,\0\01:49,\0\0\0\0load\0average:\00.95,\00.80,\00.50
\0\0\0\0\0\0\0\0\0tan\0\0\0\0up\0\0\0\0\0\0\0\0\0\011:14,\0\0\0\0load\0average:\00.91,\00.80,\00.50
\0\0\0\0\0\0\0bathe\0\0\0\0up\0\03\0days,\017:49,\0\0\0\0load\0average:\01.84,\01.79,\01.50
\0\0\0\0\0\0\0\0bird\0\0\0\0up\0\01\0day,\0\0\09:13,\0\0\0\0load\0average:\01.95,\01.82,\01.51
\0\0\0\0\0\0bonnet\0\0\0\0up\0\02\0days,\021:18,\0\0\0\0load\0average:\00.93,\00.80,\00.50
\&\fR...
.De
A variant of the \fCfc\fP command is \fCr\fP, which redoes the last
command, with optional changes:
.Ds
%\0echo\0foo
foo
%\0r
echo\0foo
foo

%\0echo\0foo
foo
%\0r\0foo=bar
echo\0bar
bar
.De
.Sh "Command Line Editing"
.PP
\fBzsh\fP's command line editor, \fBZLE\fP, is quite powerful.
It is designed to emulate either emacs or vi; the default
is emacs.  To set the bindings for vi mode, type \fCbindkey -v\fP.  If
your \fBEDITOR\fP or \fBVISUAL\fP environment variable is vi,
\fBzsh\fP will use vi emulation by default.  You can then switch to
emacs mode with \fCbindkey -e\fP.
.PP
In addition to basic editing, the shell allows you to 
recall previous lines in the history.  In emacs mode,
this is done with \fI^P\fP (control-P) or (on many terminals) with the
cursor-up key:
.Ds
%\0ls\0~
-\0\0\0\0\0\0\0\0\0\0\0README\0\0\0\0\0\0file\0\0\0\0\0\0\0\0mail\0\0\0\0\0\0\0\0pub\0\0\0\0\0\0\0\0\0tmp
Mailboxes\0\0\0bin\0\0\0\0\0\0\0\0\0func\0\0\0\0\0\0\0\0nicecolors\0\0scr\0\0\0\0\0\0\0\0\0zsh
News\0\0\0\0\0\0\0\0etc\0\0\0\0\0\0\0\0\0iris\0\0\0\0\0\0\0\0notes\0\0\0\0\0\0\0src
%\0echo\0foobar
foobar
%\0\fI^P\fC
%\0echo\0foobar\fI^P\fC
%\0ls\0~_
.De
Pressing \fI^P\fP once brings up the previous line (\fCecho foobar\fP);
pressing it again brings up the line before that (\fCls ~\fP).
The cursor is left at the end of the line, allowing you to
edit the line if desired before executing it.
In many cases, \fBZLE\fP eliminates the need for the \fCfc\fP command,
since it is powerful enough to handle even multiline commands:
.Ds
%\0for\0i\0in\0a\0b\0c\0d\0e
>\0do
>\0echo\0$i
>\0done
a
b
c
d
e
%\0\fI^P\fC
%\0for\0i\0in\0a\0b\0c\0d\0e\0
\0do\0
\0echo\0$i\0
\0done_
.De
Now you can just move up to the part you want to change...
.Ds
%\0for\0i\0in\0\kxa\l'|\nxu\(ul'\0b\0c\0d\0e
\0do\0
\0echo\0$i\0
\0done
.De
change it, and execute the new command.
.Ds
%\0for\0i\0in\0f\0g\0h\0i\0j
\0do\0
\0echo\0$i\0
\0done
f
g
h
i
j
.De
Also, you can search the history for a certain command using
\fIESC-P\fP, this will look for the last command that started with the
(part of the) word at the beginning of the current line.  Hitting
\fIESC-P\fP another time gets you the command before that, etc.
.Ds
%\0set\0\fIESC-P\fC
%\0setopt\0autolist\0\fIESC-P\fC
%\0setopt\0nocorrect_
.De
Another way is to do an incremental search, emacs-style:
.Ds
%\0\fI^R\fC
%\0_
i-search:

%\0l\kxs\l'|\nxu\(ul'\0/usr/bin
i-search:\0l

%\0date\0>\0foofile\kx.\l'|\nxu\(ul'c
i-search:\0le
.De
Suppose you have retrieved an old history event in one of these ways
and would like to execute several consecutive old commands starting
with this one.  \fC^O\fP will execute the current command and then put
the next command from the history into the editor buffer.  Typing
\fC^O\fP several times will therefore reexecute several consecutive
commands from the history.  Of course, you can edit some of those
commands in between.
.PP
In addition to completion (see below), \fITAB\fP performs expansion if
possible.
.Ds
%\0ls\0*.c\fITAB\fC
%\0ls\0foofile.c\0fortune.c\0rnd.c\0strfile.c\0unstr.c_
.De
For example, suppose you have a bunch of weird files in an important
directory:
.Ds
%\0ls
\0\0*\0*\0*\0\0\0\0\0\0\0;\0&\0%\0$??foo\0\0dspfok\0\0\0\0\0\0\0\0foo.c
\0\0!"foo"!\0\0\0\0\0\0\0`\0\e\0`\0\0\0\0\0\0\0\0\0foo\0\0\0\0\0\0\0\0\0\0\0rrr
.De
You want to remove them, but you don't want to damage \fCfoo.c\fP.
Here is one way to do this:
.Ds
%\0rm\0*\fITAB\fC
%\0rm\0\e\0\e\0\e*\e\0\e*\e\0\e*\e\0\e\0\e\0\0\e!\e"foo\e"\e!\0\e;\e\0\e&\e\0%\e\0\e$'
''
'foo\0\e`\e\0\e\e\e\0\e`\0dspfok\0foo\0foo.c\0rrr_
.De
When you expand \fC*\fP, \fBzsh\fP inserts the names of all the files
into the editing buffer, with proper shell quoting.
Now, just move back and remove \fCfoo.c\fP from the buffer:
.Ds
%\0rm\0\e\0\e\0\e*\e\0\e*\e\0\e*\e\0\e\0\e\0\0\e!\e"foo\e"\e!\0\e;\e\0\e&\e\0%\e\0\e$'
''
'foo\0\e`\e\0\e\e\e\0\e`\0dspfok\0foo\0\kxr\l'|\nxu\(ul'rr
.De
and press return.
Everything except \fCfoo.c\fP will be deleted from the directory.  If
you do not want to actually expand the current word, but would like to
see what the matches are, type \fC^Xg\fP.
.Ds
%\0rm\0f*\fI^Xg\fP
foo\0\0\0\0foo.c
%\0rm\0f*_
.De
Here's another trick; let's say you have typed this command in:
.Ds
%\0gcc\0-o\0x.out\0foob.c\0-g\0-Wpointer-arith\0-Wtrigraphs_
.De
and you forget which library you want.  You need to escape
out for a minute and check by typing
\fCls /usr/lib\fP, or some other such command;
but you don't want to retype the whole command again,
and you can't press return now because the current command
is incomplete.
In \fBzsh\fP, you can put the line on the \fIbuffer stack\fP, using
\fIESC-Q\fP, and type some other commands.  The next time a prompt is printed,
the \fCgcc\fP line will be popped off the stack and put
in the editing buffer automatically; you can then enter the
proper library name and press return (or, \fIESC-Q\fP again and look
for some other libraries whose names you forgot).
.PP
A similar situation: what if you forget the option to gcc that
finds bugs using AI techniques?  You could either use \fIESC-Q\fP
again, and type \fCman gcc\fP, or you could press \fIESC-H\fP, which
essentially does the same thing; it puts the current line on
the buffer stack, and executes the command \fCrun-help gcc\fP,
where \fCrun-help\fP is an alias for \fCman\fP.
.PP
Another interesting command is \fIESC-A\fP.  This executes the
current line, but retains it in the buffer, so that it appears
again when the next prompt is printed.
Also, the cursor stays in the same place.
This is useful for executing a series of similar commands:
.Ds
%\0cc\0grok.c\0-g\0-lc\0-lgl\0-lsun\0-lmalloc\0-Bstatic\0-o\0b.out
%\0cc\0fubar.c\0-g\0-lc\0-lgl\0-lsun\0-lmalloc\0-Bstatic\0-o\0b.out
%\0cc\0fooble.c\0-g\0-lc\0-lgl\0-lsun\0-lmalloc\0-Bstatic\0-o\0b.out
.De
.PP
The \fIESC-'\fP command is useful for managing the shell's quoting
conventions.  Let's say you want to print this string:
.Ds
don't\0do\0that;\0type\0'rm\0-rf\0\e*',\0with\0a\0\e\0before\0the\0*.
.De
All that is necessary is to type it into the editing buffer:
.Ds
%\0don't\0do\0that;\0type\0'rm\0-rf\0\e*',\0with\0a\0\e\0before\0the\0*.
.De
press \fIESC-'\fP (escape-quote):
.Ds
%\0'don'\e''t\0do\0that;\0type\0'\e''rm\0-rf\0\e*'\e'',\0with\0a\0\e\0before\0the\0*.'
.De
then move to the beginning and add the \fCecho\fP command.
.Ds
%\0echo\0'don'\e''t\0do\0that;\0type\0'\e''rm\0-rf\0\e*'\e'',\0with\0a\0\e\0before\0the\0*.'
don't\0do\0that;\0type\0'rm\0-rf\0\e*',\0with\0a\0\e\0before\0the\0*.
.De
Let's say you want to create an alias to do this \fCecho\fP command.
This can be done by recalling the line with \fI^P\fP and pressing
\fIESC-'\fP again:
.Ds
%\0'echo\0'\e''don'\e''\e'\e'''\e''t\0do\0that;\0type\0'\e''\e'\e'''\e''rm\0-rf
\e*'\e''\e'\e'''\e'',\0with\0a\0\e\0before\0the\0*.'\e'''
.De
and then move to the beginning and add the command to create
an alias.
.Ds
%\0alias\0zoof='echo\0'\e''don'\e''\e'\e'''\e''t\0do\0that;\0type\0'\e''\e'\e'''\e''rm
-rf\0\e*'\e''\e'\e'''\e'',\0with\0a\0\e\0before\0the\0*.'\e'''
%\0zoof
don't\0do\0that;\0type\0'rm\0-rf\0\e*',\0with\0a\0\e\0before\0the\0*.
.De
If one of these fancy editor commands changes your command line in a
way you did not intend, you can undo changes with \fC^_\fP, if you can
get it out of your keyboard, or \fC^X^U\fP, otherwise.
.PP
Another use of the editor is to edit the value of variables.
For example, an easy way to change your path is to use
the \fCvared\fP command:
.Ds
%\0vared\0PATH
>\0/u/pfalstad/scr:/u/pfalstad/bin/sun4:/u/maruchck/scr:/u/subbarao/bin:/u/maruc
hck/bin:/u/subbarao/scripts:/usr/princeton/bin:/usr/ucb:/usr/bin:/bin:/usr/host
s:/usr/princeton/bin/X11:/./usr/lang:/./usr/etc:/./etc
.De
You can now edit the path.  When you press return, the contents
of the edit buffer will be assigned to \fBPATH\fP.
.Sh "Completion"
.PP
Another great \fBzsh\fP feature is completion.  If you hit \fITAB\fP, \fBzsh\fP
will complete all kinds of stuff.  Like commands or filenames:
.Ds
%\0comp\fITAB\fC
%\0compress\0_

%\0ls\0nic\fITAB\fC
%\0ls\0nicecolors\0_

%\0ls\0/usr/pr\fITAB\fC
%\0ls\0/usr/princeton/_

%\0ls\0-l\0=com\fITAB\fC
%\0ls\0-l\0=compress\0_
.De
If the completion is ambiguous, the editor will beep.  If you find
this annoying, you can set the \fINOLISTBEEP\fP option.  Completion
can even be done in the middle of words.  To use this, you will have
to set the \fICOMPLETEINWORD\fP option:
.Ds
%\0setopt\0completeinword
%\0ls\0/usr/p\kxt\l'|\nxu\(ul'on\fITAB\fC
%\0ls\0/usr/prince\kxt\l'|\nxu\(ul'on/
%\0setopt\0alwaystoend
%\0ls\0/usr/p\kxt\l'|\nxu\(ul'on\fITAB\fC
%\0ls\0/usr/princeton/_
.De
You can list possible completions by pressing \fI^D\fP:
.Ds
%\0ls\0/vmu\fITAB\0\(embeep\(em\fC
%\0ls\0/vmunix_
%\0ls\0/vmunix\fI^D\fC
vmunix\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0vmunix.old\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0
vmunix.new.kernelmap.old\0\0vmunix.org
.De
Or, you could just set the \fIAUTOLIST\fP option:
.Ds
%\0setopt\0autolist
%\0ls\0/vmu\fITAB\0\(embeep\(em\fC
vmunix\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0vmunix.old\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0
vmunix.new.kernelmap.old\0\0vmunix.org
%\0ls\0/vmunix_
.De
If you like to see the types of the files in these lists, like in
\fCls\ -F\fP, you can set the \fILISTTYPES\fP option.  Together with
\fIAUTOLIST\fP you can use \fILISTAMBIGUOUS\fP.  This will only list
the possibilities if there is no unambiguous part to add:
.Ds
%\0setopt\0listambiguous
%\0ls\0/vmu\fITAB\0\(embeep\(em\fC
%\0ls\0/vmunix_\fITAB\0\(embeep\(em\fC
vmunix\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0vmunix.old\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0
vmunix.new.kernelmap.old\0\0vmunix.org
.De
If you don't want several of these listings to scroll the screen so
much, the \fIALWAYSLASTPROMPT\fP option is useful.  If set, you can
continue to edit the line you were editing, with the completion
listing appearing beneath it.
.PP
Another interesting option is \fIMENUCOMPLETE\fP.  This affects the
way \fITAB\fP works.  Let's look at the \fC/vmunix\fP example again:
.Ds
%\0setopt\0menucomplete
%\0ls\0/vmu\fITAB\fC
%\0ls\0/vmunix\fITAB\fC
%\0ls\0/vmunix.new.kernelmap.old\fITAB\fC
%\0ls\0/vmunix.old_
.De
Each time you press \fITAB\fP, it displays the next possible completion.
In this way, you can cycle through the possible completions until
you find the one you want.
.PP
The \fIAUTOMENU\fP option makes a nice compromise between this method
of completion and the regular method.  If you set this option,
pressing \fITAB\fP once completes the unambiguous part normally,
pressing the \fITAB\fP key repeatedly after an ambiguous completion
will cycle through the possible completions.
.PP
Another option you could set is \fIRECEXACT\fP, which causes
exact matches to be accepted, even if there are other possible
completions:
.Ds
%\0setopt\0recexact
%\0ls\0/vmu\fITAB\0\(embeep\(em\fC
vmunix\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0vmunix.old\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0
vmunix.new.kernelmap.old\0\0vmunix.org
%\0ls\0/vmunix_\fITAB\fC
%\0ls\0/vmunix\0_
.De
To facilitate the typing of pathnames, a slash will be added whenever
a directory is completed.  Some computers don't like the spurious
slashes at the end of directory names.  In that case, the
\fIAUTOREMOVESLASH\fP option comes to rescue.  It will remove these
slashes when you type a space or return after them.
.PP
The \fIfignore\fP variable lists suffixes of files to ignore
during completion.
.Ds
%\0ls\0foo\fITAB\0\(embeep\(em\fC
foofile.c\0\0foofile.o
%\0fignore=(\0.o\0\e~\0.bak\0.junk\0)
%\0ls\0foo\fITAB\fP
%\0ls\0foofile.c\0_
.De
Since \fCfoofile.o\fP has a suffix that is in the \fCfignore\fP list,
it was not considered a possible completion of \fCfoo\fP.
.PP
Username completion is also supported:
.Ds
%\0ls\0~pfal\fITAB\fC
%\0ls\0~pfalstad/_
.De
and parameter name completion:
.Ds
%\0echo\0$ORG\fITAB\fC
%\0echo\0$ORGANIZATION\0_
%\0echo\0${ORG\fITAB\fC
%\0echo\0${ORGANIZATION\0_
.De
Note that in the last example a space is added after the completion as
usual.  But if you want to add a colon or closing brace, you probably
don't want this extra space.  Setting the \fIAUTOPARAMKEYS\fP option
will automatically remove this space if you type a colon or closing
brace after such a completion.
.PP
There is also option completion:
.Ds
%\0setopt\0nocl\fITAB\fC
%\0setopt\0noclobber\0_
.De
and binding completion:
.Ds
%\0bindkey\0'^X^X'\0pu\fITAB\fC
%\0bindkey\0'^X^X'\0push-line\0_
.De
The \fCcompctl\fP command is used to control completion of the
arguments of specific commands.  For example, to specify that certain
commands take other commands as arguments, you use \fCcompctl -c\fP:
.Ds
%\0compctl\0-c\0man\0nohup
%\0man\0upt\fITAB\fC
%\0man\0uptime\0_
.De
To specify that a command should complete filenames, you should use
\fCcompctl -f\fP.  This is the default.  It can be combined with \fC-c\fP,
as well.
.Ds
%\0compctl\0-cf\0echo
%\0echo\0upt\fITAB\fC
%\0echo\0uptime\0_

%\0echo\0fo\fITAB\fC
%\0echo\0foo.c
.De
Similarly, use \fC-o\fP to specify options, \fC-v\fP to specify
variables, and \fC-b\fP to specify bindings.
.Ds
%\0compctl\0-o\0setopt\0unsetopt
%\0compctl\0-v\0typeset\0vared\0unset\0export
%\0compctl\0-b\0bindkey
.De
You can also use \fC-k\fP to specify a custom list of keywords to use
in completion.  After the \fC-k\fP comes either the name of an array
or a literal array to take completions from.
.Ds
%\0ftphosts=(ftp.uu.net\0wuarchive.wustl.edu)
%\0compctl\0-k\0ftphosts\0ftp
%\0ftp\0wu\fITAB\fC
%\0ftp\0wuarchive.wustl.edu\0_

%\0compctl\0-k\0'(cpirazzi\0subbarao\0sukthnkr)'\0mail\0finger
%\0finger\0cp\fITAB\fC
%\0finger\0cpirazzi\0_
.De
To better specify the files to complete for a command, use the
\fC-g\fP option which takes any glob pattern as an argument.  Be sure
to quote the glob patterns as otherwise they will be expanded when the
\fCcompctl\fP command is run.
.Ds
%\0ls
letter.tex\0\0letter.dvi\0\0letter.aux\0\0letter.log\0\0letter.toc
%\0compctl\0-g\0'*.tex'\0latex
%\0compctl\0-g\0'*.dvi'\0xdvi\0dvips
%\0latex\0l\fITAB\fC
%\0latex\0letter.tex\0_
%\0xdvi\0l\fITAB\fC
%\0xdvi\0letter.dvi\0_
.De
Glob patterns can include qualifiers within parentheses.  To rmdir
only directories and cd to directories and symbolic links pointing to
them:
.Ds
%\0compctl\0-g\0'*(-/)'\0cd
%\0compctl\0-g\0'*(/)'\0rmdir
.De
RCS users like to run commands on files which are not in the current
directory, but in the RCS subdirectory where they all get \fC,v\fP
suffixes.  They might like to use
.Ds
%\0compctl\0-g\0'RCS/*(:t:s/\e,v//)'\0co\0rlog\0rcs
%\0ls\0RCS
builtin.c,v\0\0lex.c,v\0\0\0\0\0\0zle_main.c,v
%\0rlog\0bu\fITAB\fC
%\0rlog\0builtin.c\0_
.De
The \fC:t\fP modifier keeps only the last part of the pathname and the
\fC:s/\e,v//\fP will replace any \fC,v\fP by nothing.
.PP
The \fC-s\fP flag is similar to \fC-g\fP, but it uses all expansions,
instead of just globbing, like brace expansion, parameter substitution
and command substitution.
.Ds
%\0compctl\0-s\0'$(setopt)'\0unsetopt
.De
will only complete options which are actually set to be arguments to
\fCunsetopt\fP.
.PP
Sometimes a command takes another command as its argument.  You can
tell \fBzsh\fP to complete commands as the first argument to such a
command and then use the completion method of the second command.  The
\fC-l\fP flag with a null-string argument is used for this.
.Ds
%\0compctl\0-l\0''\0nohup\0exec
%\0nohup\0comp\fITAB\fC
%\0nohup\0compress\0_
%\0nohup\0compress\0fil\fITAB\fC
%\0nohup\0compress\0filename\0_
.De
Sometimes you would like to run really complicated commands to find
out what the possible completions are.  To do this, you can specify a
shell function to be called that will assign the possible completions
to a variable called reply.  Note that this variable must be an array.
Here's another (much slower) way to get the completions for \fCco\fP
and friends:
.Ds
%\0function\0getrcs\0{
>\0reply=()
>\0for\0i\0in\0RCS/*
>\0\0\0do
>\0\0\0reply=($reply[*]\0$(basename\0$i\0,v))
>\0\0\0done
>\0}
%\0compctl\0-K\0getrcs\0co\0rlog\0rcs
.De
Some command arguments use a prefix that is not a part of the things
to complete.  The kill builtin command takes a signal name after a
\fC-\fP.  To make such a prefix be ignored in the completion process,
you can use the \fC-P\fP flag.
.Ds
%\0compctl\0-P\0-\0-k\0signals\0kill
%\0kill\0-H\fITAB\fP
%\0kill\0-HUP\0_
.De
TeX is usually run on files ending in \fC.tex\fP, but also sometimes
on other files.  It is somewhat annoying to specify that the arguments
of TeX should end in \fC.tex\fP and then not be able to complete these
other files.  Therefore you can specify things like \*QComplete to
files ending in \fC.tex\fP if available, otherwise complete to any
filename.\*U.  This is done with \fIxor\fPed completion:
.Ds
%\0compctl\0-g\0'*.tex'\0+\0-f\0tex
.De
The \fC+\fP tells the editor to only take the next thing into account
if the current one doesn't generate any matches.  If you have not
changed the default completion, the above example is in fact
equivalent to
.Ds
%\0compctl\0-g\0'*.tex'\0+\0tex
.De
as a lone \fC+\fP at the end is equivalent to specifying the default
completion after the \fC+\fP.  This form of completion is also
frequently used if you want to run some command only on a certain type
of files, but not necessarily in the current directory.  In this case
you will want to complete both files of this type and directories.
Depending on your preferences you can use either of
.Ds
%\0compctl\0-g\0'*.ps'\0+\0-g\0'*(-/)'\0ghostview
%\0compctl\0-g\0'*.ps\0*(-/)'\0ghostview
.De
where the first one will only complete directories (and symbolic links
pointing to directories) if no postscript file matches the already
typed part of the argument.
.Sh "Extended completion"
.PP
If you play with completion, you will soon notice that you would like
to specify what to complete, depending on what flags you give to the
command and where you are on the command line.  For example, a command
could take any filename argument after a \fC-f\fP flag, a username
after a \fC-u\fP flag and an executable after a \fC-x\fP flag.  This
section will introduce you to the ways to specify these things.  To
many people it seems rather difficult at first, but taking the trouble
to understand it can save you lots of typing in the end.  Even I keep
being surprised when \fBzsh\fP manages to complete a small or even
empty prefix to the right file in a large directory.
.PP
To tell \fBzsh\fP about these kinds of completion, you use \*Qextended
completion\*U by specifying the \fC-x\fP flag to compctl.  The
\fC-x\fP flag takes a list of patterns/flags pairs.  The patterns
specify when to complete and the flags specify what.  The flags are
simply those mentioned above, like \fC-f\fP or \fC-g \fIglob
pattern\fR.
.PP
As an example, the \fCr[\fIstring1\fC,\fIstring2\fC]\fR pattern
matches if the cursor is after something that starts with
\fIstring1\fP and before something that starts with \fIstring2\fP.
The \fIstring2\fP is often something that you do not want to match
anything at all.
.Ds
%\0ls
foo1\0\0\0bar1\0\0\0foo.Z\0\0bar.Z
%\0compctl\0-g\0'^*.Z'\0-x\0'r[-d,---]'\0-g\0'*.Z'\0--\0compress
%\0compress\0f\fITAB\fP
%\0compress\0foo1\0_
%\0compress\0-d\0f\fITAB\fP
%\0compress\0-d\0foo.Z\0_
.De
In the above example, if the cursor is after the \fC-d\fP the pattern
will match and therefore \fBzsh\fP uses the \fC-g *.Z\fP flag that will only
complete files ending in \fC.Z\fP.  Otherwise, if no pattern matches,
it will use the flags before the \fC-x\fP and in this case complete
every file that does not end in \fC.Z\fP.
.PP
The \fCs[\fIstring\fC]\fR pattern matches if the current word starts
with \fIstring\fP.  The \fIstring\fP itself is not considered to be
part of the completion.
.Ds
%\0compctl\0-x\0's[-]'\0-k\0signals\0--\0kill
%\0kill\0-H\fITAB\fP
%\0kill\0-HUP\0_
.De
The \fCtar\fP command takes a tar file as an argument after the
\fC-f\fP option.  The \fCc[\fIoffset\fC,\fIstring\fC]\fR pattern
matches if the word in position \fIoffset\fP relative to the current
word is \fIstring\fP.  More in particular, if \fIoffset\fP is -1, it
matches if the previous word is \fIstring\fP.  This suggests
.Ds
%\0compctl\0-f\0-x\0'c[-1,-f]'\0-g\0'*.tar'\0--\0tar
.De
But this is not enough.  The \fC-f\fP option could be the last of a
longer string of options.  \fCC[\fR...\fC,\fR...\fC]\fR is just like
\fCc[\fR...\fC,\fR...\fC]\fR, except that it uses glob-like pattern
matching for \fIstring\fP.  So
.Ds
%\0compctl\0-f\0-x\0'C[-1,-*f]'\0-g\0'*.tar'\0--\0tar
.De
will complete tar files after any option string ending in an \fCf\fP.
But we'd like even more.  Old versions of tar used all options as the
first argument, but without the minus sign.  This might be
inconsistent with option usage in all other commands, but it is still
supported by newer versions of \fCtar\fP.  So we would also like to
complete tar files if the first argument ends in an \fCf\fP and we're
right behind it.
.PP
We can `and' patterns by putting them next to each other with a space
between them.  We can `or' these sets by putting comma's between them.
We will also need some new patterns.  \fCp[\fInum\fC]\fR will match if
the current argument (the one to be completed) is the \fInum\fPth
argument.  \fCW[\fIindex\fC,\fIpattern\fC]\fR will match if the
argument in place \fIindex\fP matches the \fIpattern\fP.  This gives
us
.Ds
%\0compctl\0-f\0-x\0'C[-1,-*f]\0,\0W[1,*f]\0p[2]'\0-g\0'*.tar'\0--\0tar
.De
In words: If the previous argument is an option string that ends in an
\fCf\fP, or the first argument ended in an \fCf\fP and it is now the
second argument, then complete only filenames ending in \fC.tar\fP.
.PP
All the above examples used only one set of patterns with one
completion flag.  You can use several of these pattern/flag pairs
separated by a \fC-\fP.  The first matching pattern will be used.
Suppose you have a version of \fCtar\fP that supports compressed files
by using a \fC-Z\fP option.  Leaving the old tar syntax aside for a
moment, we would like to complete files ending in \fC.tar.Z\fP if a
\fC-Z\fP option has been used and files ending in \fC.tar\fP
otherwise, all this only after a \fC-f\fP flag.  Again, the \fC-Z\fP
can be alone or it can be part of a longer option string, perhaps the
same as that of the \fC-f\fP flag.  Here's how to do it; note the
backslash and the secondary prompt which are not part of the
\fCcompctl\fP command.
.Ds
%\0compctl\0-f\0-x\0'C[-1,-*Z*f]\0,\0R[-*Z*,---]\0C[-1,-*f]'\0-g\0'*.tar.Z'\0-\0\e
>\0'C[-1,-*f]'\0-g\0'*.tar'\0--\0tar
.De
The first pattern set tells us to match if either the previous
argument was an option string including a \fCZ\fP and ending in an
\fCf\fP or there was an option string with a \fCZ\fP somewhere and the
previous word was any option string ending in an \fCf\fP.  If this is
the case, we need a compressed tar file.  Only if this is not the case
the second pattern set will be considered.  By the way,
\fCR[\fIpattern1\fC,\fIpattern2\fC]\fR is just like
\fCr[\fR...\fC,\fR...\fC]\fR except that it uses pattern matching with
shell metacharacters instead of just strings.
.PP
You will have noticed the \fC--\fP before the command name.  This ends
the list of pattern/flag pairs of \fC-x\fP.  It is usually used just
before the command name, but you can also use an extended completion
as one part of a list of xored completions, in which case the \fC--\fP
appears just before one of the \fC+\fP signs.
.PP
Note the difference between using extended completion as part of a
list of xored completions as in
.Ds
%\0ls
foo\0\0bar
%\0compctl\0-x\0'r[-d,---]'\0-g\0'*.Z'\0--\0+\0-g\0'^*.Z'\0compress
%\0compress\0-d\0f\fITAB\fP
%\0compress\0-d\0foo\0_
.De
and specifying something before the \fC-x\fP as in
.Ds
%\0compctl\0-g\0'^*.Z'\0-x\0'r[-d,---]'\0-g\0'*.Z'\0--\0compress
%\0compress\0-d\0f\fITAB\fP
%\0compress\0-d\0f_
.De
In the first case, the alternative glob pattern (\fC^*.Z\fP) will be
used if the first part does not generate any possible completions,
while in the second case the alternative glob pattern will only be
used if the \fCr[\fR...\fC]\fR pattern doesn't match.
.Sh "Bindings"
.PP
Each of the editor commands we have seen was actually a function bound
by default to a certain key.  The real names of the commands are:
.Ds
\fCexpand-or-complete\0\0\0\fITAB\fR
\fCpush-line\0\0\0\0\0\0\0\0\0\0\0\0\fIESC-Q\fR
\fCrun-help\0\0\0\0\0\0\0\0\0\0\0\0\0\fIESC-H\fR
\fCaccept-and-hold\0\0\0\0\0\0\fIESC-A\fR
\fCquote-line\0\0\0\0\0\0\0\0\0\0\0\fIESC-'\fR
.De
These bindings are arbitrary; you could change them if you want.
For example, to bind \fCaccept-line\fP to \fI^Z\fP:
.Ds
%\0bindkey\0'^Z'\0accept-line
.De
Another idea would be to bind the delete key to \fCdelete-char\fP;
this might be convenient if you use \fI^H\fP for backspace.
.Ds
%\0bindkey\0'^?'\0delete-char
.De
Or, you could bind \fI^X\fP\fI^H\fP to \fCrun-help\fP:
.Ds
%\0bindkey\0'^X^H'\0run-help
.De
Other examples:
.Ds
%\0bindkey\0'^X^Z'\0universal-argument
%\0bindkey\0'\0'\0magic-space
%\0bindkey\0-s\0'^T'\0'uptime
>\0'
%\0bindkey\0'^Q'\0push-line-or-edit
.De
\fCuniversal-argument\fP multiplies the next command by 4.
Thus \fI^X\fP\fI^Z\fP\fI^W\fP might delete the last four words on the line.
If you bind space to \fCmagic-space\fP, then csh-style history
expansion is done on the line whenever you press the space bar.
.PP
Something that often happens is that I am typing a multiline command
and discover an error in one of the previous lines.  In this case,
\fCpush-line-or-edit\fP will put the entire multiline construct into
the editor buffer.  If there is only a single line, it is equivalent
to \fCpush-line\fP.
.PP
The \fC-s\fP flag to \fCbindkey\fP specifies that you are binding the key
to a string, not a command.  Thus \fCbindkey -s '^T' 'uptime\en'\fP
lets you VMS lovers get the load average whenever you press \fI^T\fP.
.PP
If you have a NeXT keyboard, the one with the \fC|\fP and \fC\e\fP keys
very inconveniently placed, the following
bindings may come in handy:
.Ds
%\0bindkey\0-s\0'\ee/'\0'\e\e'
%\0bindkey\0-s\0'\ee='\0'|'
.De
Now you can type \fIALT-/\fP to get a backslash, and \fIALT-=\fP to
get a vertical bar.  This only works inside \fBzsh\fP, of course;
\fCbindkey\fP has no effect on the key mappings inside \fCtalk\fP
or \fCmail\fP, etc.
.PP
Some people like to bind \fC^S\fP and \fC^Q\fP to editor commands.
Just binding these has no effect, as the terminal will catch them and
use them for flow control.  You could unset them as stop and start
characters, but most people like to use these for external commands.
The solution is to set the \fINOFLOWCONTROL\fP option.  This will
allow you to bind the start and stop characters to editor commands,
while retaining their normal use for external commands.
.Sh "Parameter Substitution"
.PP
In \fBzsh\fP, parameters are set like this:
.Ds
%\0foo=bar
%\0echo\0$foo
bar
.De
Spaces before or after the \fC=\fP are frowned upon:
.Ds
%\0foo\0=\0bar
zsh:\0command\0not\0found:\0foo
.De
Also, \fCset\fP doesn't work for setting parameters:
.Ds
%\0set\0foo=bar
%\0set\0foo\0=\0bar
%\0echo\0$foo

%
.De
Note that no error message was printed.  This is because both
of these commands were perfectly valid; the \fCset\fP builtin
assigns its arguments to the \fIpositional parameters\fP
(\fC$1\fP, \fC$2\fP, etc.).
.Ds
%\0set\0foo=bar
%\0echo\0$1
foo=bar
%\0set\0foo\0=\0bar
%\0echo\0$3\0$2
bar\0=
.De
If you're really intent on using the csh syntax, define a
function like this:
.Ds
%\0set\0()\0{
>\0\0\0\0eval\0"$1$2$3"
>\0}
%\0set\0foo\0=\0bar
%\0set\0fuu=brrr
%\0echo\0$foo\0$fuu
bar\0brrr
.De
But then, of course you can't use the form of \fCset\fP with
options, like \fCset -F\fP (which turns off filename generation).
Also, the \fCset\fP command by itself won't list all the parameters
like it should.
To get around that you need a \fCcase\fP statement:
.Ds
%\0set\0()\0{
>\0\0\0\0case\0$1\0in
>\0\0\0\0-*|+*|'')\0builtin\0set\0$*\0;;
>\0\0\0\0*)\0eval\0"$1$2$3"\0;;
>\0\0\0\0esac
>\0}
.De
For the most part, this should make csh users happy.
.PP
The following sh-style operators are supported in \fBzsh\fP:
.Ds
%\0unset\0null
%\0echo\0${foo-xxx}
bar
%\0echo\0${null-xxx}
xxx
%\0unset\0null
%\0echo\0${null=xxx}
xxx
%\0echo\0$null
xxx
%\0echo\0${foo=xxx}
bar
%\0echo\0$foo
bar
%\0unset\0null
%\0echo\0${null+set}

%\0echo\0${foo+set}
set
.De
Also, csh-style \fC:\fP modifiers may be appended to a parameter
substitution.
.Ds
%\0echo\0$PWD
/home/learning/pf/zsh/zsh2.00/src
%\0echo\0$PWD:h
/home/learning/pf/zsh/zsh2.00
%\0echo\0$PWD:h:h
/home/learning/pf/zsh
%\0echo\0$PWD:t
src
%\0name=foo.c
%\0echo\0$name
foo.c
%\0echo\0$name:r
foo
%\0echo\0$name:e
c
.De
The equivalent constructs in ksh (which are also supported in \fBzsh\fP)
are a bit more general and easier to remember.
When the shell expands \fC${foo#\fR\fIpat\fR\fC}\fR,
it checks to see if \fIpat\fP matches a substring at the beginning
of the value
of \fCfoo\fP.  If so, it removes that portion of \fCfoo\fP, using the shortest
possible match.
With \fC${foo##\fR\fIpat\fR\fC}\fR, the longest possible match is removed.
\fC${foo%\fR\fIpat\fR\fC}\fR and \fC${foo%%\fR\fIpat\fR\fC}\fR remove the match
from the end.
Here are the ksh equivalents of the \fC:\fP modifiers:
.Ds
%\0echo\0${PWD%/*}
/home/learning/pf/zsh/zsh2.00
%\0echo\0${PWD%/*/*}
/home/learning/pf/zsh
%\0echo\0${PWD##*/}
src
%\0echo\0${name%.*}
foo
%\0echo\0${name#*.}
c
.De
\fBzsh\fP also has upper/lowercase modifiers:
.Ds
%\0xx=Test
%\0echo\0$xx:u
TEST
%\0echo\0$xx:l
test
.De
and a substitution modifier:
.Ds
%\0echo\0$name:s/foo/bar/
bar.c
%\0ls
foo.c\0\0\0\0foo.h\0\0\0\0foo.o\0\0\0\0foo.pro
%\0for\0i\0in\0foo.*;\0mv\0$i\0$i:s/foo/bar/
%\0ls
bar.c\0\0\0\0bar.h\0\0\0\0bar.o\0\0\0\0bar.pro
.De
There is yet another syntax to modify substituted parameters.  You can
add certain modifiers in parentheses after the opening brace like:
.Ds
${(\fImodifiers\fC)\fIparameter\fC}
.De
For example, \fCo\fP sorts the words resulting from the expansion:
.Ds
%\0echo\0${path}
/usr/bin\0/usr/bin/X11\0/etc
%\0echo\0${(o)path}
/etc\0/usr/bin\0/usr/bin/X11
.De
One possible source of confusion is the fact that in \fBzsh\fP,
the result of parameter substitution is \fInot\fP split into
words.  Thus, this will not work:
.Ds
%\0srcs='glob.c\0exec.c\0init.c'
%\0ls\0$srcs
glob.c\0exec.c\0init.c\0not\0found
.De
This is considered a feature, not a bug.
If splitting were done by default, as it is in most other shells,
functions like this would not work properly:
.Ds
$\0ll\0()\0{\0ls\0-F\0$*\0}
$\0ll\0'fuu\0bar'
fuu\0not\0found
bar\0not\0found

%\0ll\0'fuu\0bar'
fuu\0bar\0not\0found
.De
Of course, a hackish workaround is available in sh (and \fBzsh\fP):
.Ds
%\0setopt\0shwordsplit
%\0ll\0()\0{\0ls\0-F\0"$@"\0}
%\0ll\0'fuu\0bar'
fuu\0bar\0not\0found
.De
If you like the sh behaviour, \fBzsh\fP can accommodate you:
.Ds
%\0ls\0${=srcs}
exec.c\0\0glob.c\0\0init.c
%\0setopt\0shwordsplit
%\0ls\0$srcs
exec.c\0\0glob.c\0\0init.c
.De
Another way to get the \fC$srcs\fP trick to work is to use an array:
.Ds
%\0unset\0srcs
%\0srcs=(\0glob.c\0exec.c\0init.c\0)\0\0
%\0ls\0$srcs
exec.c\0\0glob.c\0\0init.c
.De
or an alias:
.Ds
%\0alias\0-g\0SRCS='exec.c\0glob.c\0init.c'
%\0ls\0SRCS
exec.c\0\0glob.c\0\0init.c
.De
Another option that modifies parameter expansion is
\fIRCEXPANDPARAM\fP:
.Ds
%\0echo\0foo/$srcs
foo/glob.c\0exec.c\0init.c
%\0setopt\0rcexpandparam
%\0echo\0foo/$srcs
foo/glob.c\0foo/exec.c\0foo/init.c
%\0echo\0foo/${^srcs}
foo/glob.c\0foo/exec.c\0foo/init.c
%\0echo\0foo/$^srcs
foo/glob.c\0foo/exec.c\0foo/init.c
.De
.Sh "Shell Parameters"
.PP
The shell has many predefined parameters that may be
accessed.  Here are some examples:
.Ds
%\0sleep\010\0&
[1]\03820
%\0echo\0$!
3820
%\0set\0a\0b\0c
%\0echo\0$#
3
%\0echo\0$ARGC
3
%\0(\0exit\020\0)\0;\0echo\0$?
20
%\0false;\0echo\0$status
1
.De
(\fC$?\fP and \fC$status\fP are equivalent.)
.Ds
%\0echo\0$HOST\0$HOSTTYPE
dendrite\0sun4
%\0echo\0$UID\0$GID
701\060
%\0cd\0/tmp
%\0cd\0/home
%\0echo\0$PWD\0$OLDPWD
/home\0/tmp
%\0ls\0$OLDPWD/.getwd\0
/tmp/.getwd
.De
\fC~+\fP and \fC~-\fP are short for \fC$PWD\fP and \fC$OLDPWD\fP, respectively.
.Ds
%\0ls\0~-/.getwd
/tmp/.getwd
%\0ls\0-d\0~+/learning
/home/learning
%\0echo\0$RANDOM
4880
%\0echo\0$RANDOM
11785
%\0echo\0$RANDOM
2062
%\0echo\0$TTY
/dev/ttyp4
%\0echo\0$VERSION
zsh\0v2.00.03
%\0echo\0$USERNAME
pf
.De
.PP
The \fCcdpath\fP variable sets the search path for the \fCcd\fP command.
If you do not specify \fC.\fP somewhere in the path, it is assumed to
be the first component.
.Ds
%\0cdpath=(\0/usr\0~\0~/zsh\0)
%\0ls\0/usr
5bin\0\0\0\0\0\0\0\0\0dict\0\0\0\0\0\0\0\0\0lang\0\0\0\0\0\0\0\0\0net\0\0\0\0\0\0\0\0\0\0sccs\0\0\0\0\0\0\0\0\0sys
5include\0\0\0\0\0etc\0\0\0\0\0\0\0\0\0\0lector\0\0\0\0\0\0\0nserve\0\0\0\0\0\0\0services\0\0\0\0\0tmp
5lib\0\0\0\0\0\0\0\0\0export\0\0\0\0\0\0\0lib\0\0\0\0\0\0\0\0\0\0oed\0\0\0\0\0\0\0\0\0\0share\0\0\0\0\0\0\0\0ucb
adm\0\0\0\0\0\0\0\0\0\0games\0\0\0\0\0\0\0\0local\0\0\0\0\0\0\0\0old\0\0\0\0\0\0\0\0\0\0skel\0\0\0\0\0\0\0\0\0ucbinclude
bin\0\0\0\0\0\0\0\0\0\0geac\0\0\0\0\0\0\0\0\0lost+found\0\0\0openwin\0\0\0\0\0\0spool\0\0\0\0\0\0\0\0ucblib
boot\0\0\0\0\0\0\0\0\0hosts\0\0\0\0\0\0\0\0macsyma_417\0\0pat\0\0\0\0\0\0\0\0\0\0src\0\0\0\0\0\0\0\0\0\0xpg2bin
demo\0\0\0\0\0\0\0\0\0include\0\0\0\0\0\0man\0\0\0\0\0\0\0\0\0\0princeton\0\0\0\0stand\0\0\0\0\0\0\0\0xpg2include
diag\0\0\0\0\0\0\0\0\0kvm\0\0\0\0\0\0\0\0\0\0mdec\0\0\0\0\0\0\0\0\0pub\0\0\0\0\0\0\0\0\0\0swap\0\0\0\0\0\0\0\0\0xpg2lib
%\0cd\0spool
/usr/spool
%\0cd\0bin
/usr/bin
%\0cd\0func
~/func
%\0cd\0
%\0cd\0pub
%\0pwd
/u/pfalstad/pub
%\0ls\0-d\0/usr/pub
/usr/pub
.De
\fBPATH\fP and \fBpath\fP both set the search path for commands.
These two variables are equivalent, except that one is a string
and one is an array.  If the user modifies \fBPATH\fP, the shell
changes \fBpath\fP as well, and vice versa.
.Ds
%\0PATH=/bin:/usr/bin:/tmp:.
%\0echo\0$path
/bin\0/usr/bin\0/tmp\0.
%\0path=(\0/usr/bin\0.\0/usr/local/bin\0/usr/ucb\0)
%\0echo\0$PATH
/usr/bin:.:/usr/local/bin:/usr/ucb
.De
The same is true of \fBCDPATH\fP and \fBcdpath\fP:
.Ds
%\0echo\0$CDPATH
/usr:/u/pfalstad:/u/pfalstad/zsh
%\0CDPATH=/u/subbarao:/usr/src:/tmp
%\0echo\0$cdpath
/u/subbarao\0/usr/src\0/tmp
.De
In general, predefined parameters with names in all lowercase are
arrays; assignments to them take the form:
.Ds
\fIname\fR\fC=(\fR\0\fIelem\fR\0...\0\fC)\fR
.De
Predefined parameters with names in all uppercase are strings.  If
there is both an array and a string version of the same parameter, the
string version is a colon-separated list, like \fBPATH\fP. You can use
.Ds
%\0typeset\0-T\0FOO\0foo
.De
to create more of these yourself.
.Ds
%\0foo=(\0a\0b\0c\0)
%\0echo\0$FOO
a:b:c
.De
.PP
\fBHISTFILE\fP is the name of the history file, where the history
is saved when a shell exits.
.Ds
%\0zsh
phoenix%\0HISTFILE=/tmp/history
phoenix%\0SAVEHIST=20
phoenix%\0echo\0foo
foo
phoenix%\0date
Fri\0May\024\005:39:35\0EDT\01991
phoenix%\0uptime
\0\05:39am\0\0up\04\0days,\020:02,\0\040\0users,\0\0load\0average:\02.30,\02.20,\02.00
phoenix%\0exit
%\0cat\0/tmp/history
HISTFILE=/tmp/history
SAVEHIST=20
echo\0foo
date
uptime
exit
%\0HISTSIZE=3
%\0history
\0\0\028\0\0rm\0/tmp/history
\0\0\029\0\0HISTSIZE=3
\0\0\030\0\0history
.De
If you have several instances of \fBzsh\fP running at the same
time, like when using the X window system, it might be preferable to
append the history of each shell to a file when a shell exits instead
of overwriting the old contents of the file.  You can get this
behaviour by setting the \fIAPPENDHISTORY\fP option.
.PP
In \fBzsh\fP, if you say
.Ds
%\0>file
.De
the command \fCcat\fP is normally assumed:
.Ds
%\0>file
foo!\0\0\0\0
^D
%\0cat\0file
foo!
.De
Thus, you can view a file simply by typing:
.Ds
%\0<file
foo!
.De
However, this is not csh or sh compatible.  To correct this,
change the value of the parameter \fBNULLCMD\fP,
which is \fCcat\fP by default.
.Ds
%\0NULLCMD=:
%\0>file
%\0ls\0-l\0file
-rw-r--r--\0\01\0pfalstad\0\0\0\0\0\0\0\00\0May\024\005:41\0file
.De
If \fCNULLCMD\fP is unset, the shell reports an error if no
command is specified (like csh).
.Ds
%\0unset\0NULLCMD
%\0>file
zsh:\0redirection\0with\0no\0command
.De
Actually, \fBREADNULLCMD\fP is used whenever you have a null command
reading input from a single file.  Thus, you can set \fBREADNULLCMD\fP
to \fCmore\fP or \fCless\fP rather than \fCcat\fP.  Also, if you
set \fBNULLCMD\fP to \fC:\fP for sh compatibility, you can still read
files with \fC< file\fP if you leave \fBREADNULLCMD\fP set to \fCmore\fP.
.Sh "Prompting"
.PP
The default prompt for \fBzsh\fP is:
.Ds
phoenix%\0echo\0$PROMPT
%m%#\0
.De
The \fC%m\fP stands for the short form of the current hostname,
and the \fC%#\fP stands for a \fC%\fP or a \fC#\fP, depending on whether
the shell is running as root or not.
\fBzsh\fP supports many other control sequences
in the \fBPROMPT\fP variable.
.Ds
%\0PROMPT='%/>\0'
/u/pfalstad/etc/TeX/zsh>

%\0PROMPT='%~>\0'\0\0\0
~/etc/TeX/zsh>\0

%\0PROMPT='%h\0%~>\0'
6\0~/etc/TeX/zsh>\0
.De
\fC%h\fP\0represents\0the\0number\0of\0current\0history\0event.
.Ds
%\0PROMPT='%h\0%~\0%M>\0'
10\0~/etc/TeX/zsh\0apple-gunkies.gnu.ai.mit.edu>\0

%\0PROMPT='%h\0%~\0%m>\0'
11\0~/etc/TeX/zsh\0apple-gunkies>\0

%\0PROMPT='%h\0%t>\0'
12\06:11am>\0

%\0PROMPT='%n\0%w\0tty%l>'
pfalstad\0Fri\024\0ttyp0>
.De
\fBPROMPT2\fP is used in multiline commands, like for-loops.  The
\fC%_\fP escape sequence was made especially for this prompt.  It is
replaced by the kind of command that is being entered.
.Ds
%\0PROMPT2='%_>\0'
%\0for\0i\0in\0foo\0bar
for>

%\0echo\0'hi
quote>
.De
Also available is the \fBRPROMPT\fP parameter.
If this is set, the shell puts a prompt on the \fIright\fP side
of the screen.
.Ds
%\0RPROMPT='%t'
%\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\06:14am

%\0RPROMPT='%~'
%\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0~/etc/TeX/zsh

%\0PROMPT='%l\0%T\0%m[%h]\0'\0RPROMPT='\0%~'
p0\06:15\0phoenix[5]\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0~/etc/TeX/zsh
.De
These special escape sequences can also be used with the
\fC-P\fP option to \fCprint\fP:
.Ds
%\0print\0-P\0%h\0tty%l
15\0ttyp1
.De
.PP
The \fBPOSTEDIT\fP parameter is printed whenever the editor exits.
This can be useful for termcap tricks.  To highlight the prompt
and command line while leaving command output unhighlighted, try this:
.Ds
%\0POSTEDIT=`echotc\0se`
%\0PROMPT='%S%%\0'
.De
This trick is mostly superseded by the zle_highlight array parameter.
.Sh "Login/logout watching"
.PP
You can specify login or logout events to monitor
by setting the \fBwatch\fP variable.
Normally, this is done by specifying a list of usernames.
.Ds
%\0watch=(\0pfalstad\0subbarao\0sukthnkr\0egsirer\0)
.De
The \fClog\fP command reports all people logged in
that you are watching for.
.Ds
%\0log
pfalstad\0has\0logged\0on\0p0\0from\0mickey.
pfalstad\0has\0logged\0on\0p5\0from\0mickey.
%\0\fR...\fC
subbarao\0has\0logged\0on\0p8\0from\0phoenix.
%\0\fR...\fC
subbarao\0has\0logged\0off\0p8\0from\0phoenix.
%\0\fR...\fC
sukthnkr\0has\0logged\0on\0p8\0from\0dew.
%\0\fR...\fC
sukthnkr\0has\0logged\0off\0p8\0from\0dew.
.De
If you specify hostnames with an \fC@\fP prepended,
the shell will watch for all users logging in from
the specified host.
.Ds
%\0watch=(\0@mickey\0@phoenix\0)
%\0log
djthongs\0has\0logged\0on\0q2\0from\0phoenix.
pfalstad\0has\0logged\0on\0p0\0from\0mickey.
pfalstad\0has\0logged\0on\0p5\0from\0mickey.
.De
If you give a tty name with a \fC%\fP prepended, the shell
will watch for all users logging in on that tty.
.Ds
%\0watch=(\0%ttyp0\0%console\0)
%\0log
root\0has\0logged\0on\0console\0from\0.
pfalstad\0has\0logged\0on\0p0\0from\0mickey.
.De
The format of the reports may also be changed.
.Ds
%\0watch=(\0pfalstad\0gettes\0eps\0djthongs\0jcorr\0bdavis\0)
%\0log
jcorr\0has\0logged\0on\0tf\0from\0128.112.176.3:0.
jcorr\0has\0logged\0on\0r0\0from\0128.112.176.3:0.
gettes\0has\0logged\0on\0p4\0from\0yo:0.0.
djthongs\0has\0logged\0on\0pe\0from\0grumpy:0.0.
djthongs\0has\0logged\0on\0q2\0from\0phoenix.
bdavis\0has\0logged\0on\0qd\0from\0BRUNO.
eps\0has\0logged\0on\0p3\0from\0csx30:0.0.
pfalstad\0has\0logged\0on\0p0\0from\0mickey.
pfalstad\0has\0logged\0on\0p5\0from\0mickey.
%\0WATCHFMT='%n\0on\0tty%l\0from\0%M'
%\0log
jcorr\0on\0ttytf\0from\0128.112.176.3:0.
jcorr\0on\0ttyr0\0from\0128.112.176.3:0.
gettes\0on\0ttyp4\0from\0yo:0.0
djthongs\0on\0ttype\0from\0grumpy:0.0
djthongs\0on\0ttyq2\0from\0phoenix.Princeto
bdavis\0on\0ttyqd\0from\0BRUNO.pppl.gov
eps\0on\0ttyp3\0from\0csx30:0.0
pfalstad\0on\0ttyp0\0from\0mickey.Princeton
pfalstad\0on\0ttyp5\0from\0mickey.Princeton
%\0WATCHFMT='%n\0fm\0%m'
%\0log
jcorr\0fm\0128.112.176.3:0
jcorr\0fm\0128.112.176.3:0
gettes\0fm\0yo:0.0
djthongs\0fm\0grumpy:0.0
djthongs\0fm\0phoenix
bdavis\0fm\0BRUNO
eps\0fm\0csx30:0.0
pfalstad\0fm\0mickey
pfalstad\0fm\0mickey
%\0WATCHFMT='%n\0%a\0at\0%t\0%w.'
%\0log
jcorr\0logged\0on\0at\03:15pm\0Mon\020.
jcorr\0logged\0on\0at\03:16pm\0Wed\022.
gettes\0logged\0on\0at\06:54pm\0Wed\022.
djthongs\0logged\0on\0at\07:19am\0Thu\023.
djthongs\0logged\0on\0at\07:20am\0Thu\023.
bdavis\0logged\0on\0at\012:40pm\0Thu\023.
eps\0logged\0on\0at\04:19pm\0Thu\023.
pfalstad\0logged\0on\0at\03:39am\0Fri\024.
pfalstad\0logged\0on\0at\03:42am\0Fri\024.
.De
If you have a \fC.friends\fP file in your home directory,
a convenient way to make \fBzsh\fP watch for all your friends
is to do this:
.Ds
%\0watch=(\0$(<\0~/.friends)\0)
%\0echo\0$watch
subbarao\0maruchck\0root\0sukthnkr\0\fR...
.De
If watch is set to \fCall\fP, then all users logging in or out
will be reported.
.Sh "Options"
.PP
Some options have already been mentioned; here are a few more:
.PP
Using the \fIAUTOCD\fP option, you can simply type the name
of a directory, and it will become the current directory.
.Ds
%\0cd\0/
%\0setopt\0autocd
%\0bin
%\0pwd
/bin
%\0../etc
%\0pwd
/etc
.De
With \fICDABLEVARS\fP, if the argument to \fCcd\fP is the name of a
parameter whose value is a valid directory, it will become
the current directory.
.Ds
%\0setopt\0cdablevars
%\0foo=/tmp
%\0cd\0foo
/tmp
.De
\fICORRECT\fP turns on spelling correction for commands,
and the \fICORRECTALL\fP option turns on spelling correction
for all arguments.
.Ds
%\0setopt\0correct
%\0sl
zsh:\0correct\0`sl'\0to\0`ls'\0[nyae]?\0y
%\0setopt\0correctall
%\0ls\0x.v11r4
zsh:\0correct\0`x.v11r4'\0to\0`X.V11R4'\0[nyae]?\0n
/usr/princton/src/x.v11r4\0not\0found
%\0ls\0/etc/paswd
zsh:\0correct\0to\0`/etc/paswd'\0to\0`/etc/passwd'\0[nyae]?\0y
/etc/passwd
.De
If you press \fCy\fP
when the shell asks you if you want to correct a word, it will
be corrected.  If you press \fCn\fP, it will be left alone.
Pressing \fCa\fP aborts the command, and pressing \fCe\fP brings the line
up for editing again, in case you agree the word is spelled wrong
but you don't like the correction.
.PP
Normally, a quoted expression may contain a newline:
.Ds
%\0echo\0'
>\0foo
>\0'

foo

%
.De
With \fICSHJUNKIEQUOTES\fP set, this is illegal, as it is
in csh.
.Ds
%\0setopt\0cshjunkiequotes
%\0ls\0'foo
zsh:\0unmatched\0'
.De
\fIGLOBDOTS\fP lets files beginning with a \fC.\fP be matched without
explicitly specifying the dot. This can also be specified for a particular
pattern by appending (D) to it.
.Ds
%\0ls\0-d\0*x*
Mailboxes
%\0ls\0-d\0*x*(D)
\&.exrc\0\0\0\0\0\0\0\0\0.pnewsexpert\0\0.xserverrc
\&.mushexpert\0\0\0.xinitrc\0\0\0\0\0\0Mailboxes
%\0setopt\0globdots
%\0ls\0-d\0*x*
\&.exrc\0\0\0\0\0\0\0\0\0.pnewsexpert\0\0.xserverrc
\&.mushexpert\0\0\0.xinitrc\0\0\0\0\0\0Mailboxes
.De
\fIHISTIGNOREDUPS\fP prevents the current line from being
saved in the history if it is the same as the previous one;
\fIHISTIGNORESPACE\fP prevents the current line from being
saved if it begins with a space.
.Ds
%\0PROMPT='%h>\0'
39>\0setopt\0histignoredups
40>\0echo\0foo
foo
41>\0echo\0foo
foo
41>\0echo\0foo
foo
41>\0echo\0bar
bar
42>\0setopt\0histignorespace
43>\0\0echo\0foo
foo
43>\0\0echo\0fubar
fubar
43>\0\0echo\0fubar
fubar
.De
\fIIGNOREBRACES\fP turns off csh-style brace expansion.
.Ds
%\0echo\0x{y{z,a},{b,c}d}e
xyze\0xyae\0xbde\0xcde
%\0setopt\0ignorebraces
%\0echo\0x{y{z,a},{b,c}d}e
x{y{z,a},{b,c}d}e
.De
\fIIGNOREEOF\fP forces the user to type \fCexit\fP or \fClogout\fP,
instead of just pressing \fI^D\fP.
.Ds
%\0setopt\0ignoreeof
%\0^D
zsh:\0use\0'exit'\0to\0exit.
.De
\fIINTERACTIVECOMMENTS\fP turns on interactive comments;
comments begin with a \fC#\fP.
.Ds
%\0setopt\0interactivecomments
%\0date\0#\0this\0is\0a\0comment
Fri\0May\024\006:54:14\0EDT\01991
.De
\fINOBEEP\fP makes sure the shell never beeps.
.PP
\fINOCLOBBER\fP prevents you from accidentally
overwriting an existing file.
.Ds
%\0setopt\0noclobber
%\0cat\0/dev/null\0>~/.zshrc
zsh:\0file\0exists:\0/u/pfalstad/.zshrc
.De
If you really do want to clobber a file, you can use the
\fC>!\fP operator.
To make things easier in this case, the \fC>\fP is stored in
the history list as a \fC>!\fP:
.Ds
%\0cat\0/dev/null\0>!\0~/.zshrc
%\0cat\0/etc/motd\0>\0~/.zshrc
zsh:\0file\0exists:\0/u/pfalstad/.zshrc
%\0!!
cat\0/etc/motd\0>!\0~/.zshrc
%\0\fR...
.De
\fIRCQUOTES\fP lets you use a more elegant method for including
single quotes in a singly quoted string:
.Ds
%\0echo\0'"don'\e''t\0do\0that."'
"don't\0do\0that."
%\0echo\0'"don''t\0do\0that."'
"dont\0do\0that."
%\0setopt\0rcquotes
%\0echo\0'"don''t\0do\0that."'
"don't\0do\0that."
.De
Finally,
\fISUNKEYBOARDHACK\fP wins the award for the strangest option.
If a line ends with \fC`\fP, and there are an odd number of them
on the line, the shell will ignore the trailing \fC`\fP.  This
is provided for keyboards whose RETURN key is too small,
and too close to the \fC`\fP key.
.Ds
%\0setopt\0sunkeyboardhack
%\0date`
Fri\0May\024\006:55:38\0EDT\01991
.De
.Sh "Closing Comments"
.PP
I (Bas de Bakker) would be happy to receive mail if anyone has any
tricks or ideas to add to this document, or if there are some points
that could be made clearer or covered more thoroughly.  Please notify
me of any errors in this document.
.if o \{\
.bp
.sv 1i
.\}
.pn 1
.bp
.PX
