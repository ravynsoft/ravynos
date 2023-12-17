.de SE	\" start example
.sp .5
.RS
.ft CR
.nf
..
.de EE	\" end example
.fi
.sp .5
.RE
.ft R
..
.TL
Bash \- The GNU shell*
.AU
Chet Ramey
Case Western Reserve University
chet@po.cwru.edu
.FS
*An earlier version of this article appeared in The Linux Journal.
.FE
.NH 1
Introduction
.PP
.B Bash
is the shell, or command language interpreter,
that will appear in the GNU operating system.
The name is an acronym for
the \*QBourne-Again SHell\*U, a pun on Steve Bourne, the author
of the direct ancestor of the current
.UX
shell \fI/bin/sh\fP,
which appeared in the Seventh Edition Bell Labs Research version
of \s-1UNIX\s+1.
.PP
Bash is an \fBsh\fP\-compatible shell that incorporates useful
features from the Korn shell (\fBksh\fP) and the C shell (\fBcsh\fP),
described later in this article.  It is ultimately intended to be a
conformant implementation of the IEEE POSIX Shell and Utilities
specification (IEEE Working Group 1003.2).  It offers functional
improvements over sh for both interactive and programming use.
.PP
While the GNU operating system will most likely include a version
of the Berkeley shell csh, Bash will be the default shell.
Like other GNU software, Bash is quite portable.  It currently runs
on nearly every version of
.UX
and a few other operating systems \- an independently-supported
port exists for OS/2, and there are rumors of ports to DOS and
Windows NT.  Ports to \s-1UNIX\s+1-like systems such as QNX and Minix
are part of the distribution.
.PP
The original author of Bash
was Brian Fox, an employee of the Free Software Foundation.  The
current developer and maintainer is Chet Ramey, a volunteer who
works at Case Western Reserve University.
.NH 1
What's POSIX, anyway?
.PP
.I POSIX
is a name originally coined by Richard Stallman for a family of open
system standards based on \s-1UNIX\s+1.  There are a number of aspects of \s-1UNIX\s+1
under consideration for standardization, from the basic system services
at the system call and C library level to applications and tools to system
administration and management.  Each area of standardization is
assigned to a working group in the 1003 series.
.PP
The POSIX Shell and Utilities standard has been developed by IEEE Working
Group 1003.2 (POSIX.2).\(dd
.FS
\(ddIEEE, \fIIEEE Standard for Information Technology -- Portable
Operating System Interface (POSIX) Part 2: Shell and Utilities\fP,
1992.
.FE
It concentrates on the command interpreter
interface and utility programs
commonly executed from the command line or by other programs.
An initial version of the standard has been
approved and published by the IEEE, and work is currently underway to
update it.
There are four primary areas of work in the 1003.2 standard:
.IP \(bu
Aspects of the shell's syntax and command language.
A number of special builtins such as
.B cd
and
.B exec
are being specified as part of the shell, since their
functionality usually cannot be implemented by a separate executable;
.IP \(bu
A set of utilities to be called by shell scripts and applications.
Examples are programs like
.I sed,
.I tr,
and
.I awk.
Utilities commonly implemented as shell builtins
are described in this section, such as
.B test
and
.B kill .
An expansion of this section's scope, termed the User Portability
Extension, or UPE, has standardized interactive programs such as
.I vi
and
.I mailx;
.IP \(bu
A group of functional interfaces to services provided by the
shell, such as the traditional \f(CRsystem()\fP
C library function.  There are functions to perform shell word
expansions, perform filename expansion (\fIglobbing\fP), obtain values
of POSIX.2 system configuration variables, retrieve values of
environment variables (\f(CRgetenv()\fP\^), and other services;
.IP \(bu
A suite of \*Qdevelopment\*U utilities such as
.I c89
(the POSIX.2 version of \fIcc\fP),
and
.I yacc.
.PP
Bash is concerned with the aspects of the shell's behavior
defined by POSIX.2.  The shell command language has of
course been standardized, including the basic flow control
and program execution constructs, I/O redirection and
pipelining, argument handling, variable expansion, and quoting.
The
.I special
builtins, which must be implemented as part of the shell to
provide the desired functionality, are specified as being
part of the shell; examples of these are
.B eval
and
.B export .
Other utilities appear in the sections of POSIX.2 not
devoted to the shell which are commonly (and in some
cases must be) implemented as builtin commands, such as
.B read
and
.B test .
POSIX.2 also specifies aspects of the shell's
interactive behavior as part of
the UPE, including job control and command line editing.
Interestingly enough, only \fIvi\fP-style line editing commands
have been standardized; \fIemacs\fP editing commands were left
out due to objections.
.PP
While POSIX.2 includes much of what the shell has traditionally
provided, some important things have been omitted as being
\*Qbeyond its scope.\*U  There is, for instance, no mention of
a difference between a
.I login
shell and any other interactive shell (since POSIX.2 does not
specify a login program).  No fixed startup files are defined,
either \- the standard does not mention
.I .profile .
.NH 1
Basic Bash features
.PP
Since the Bourne shell
provides Bash with most of its philosophical underpinnings,
Bash inherits most of its features and functionality from sh.
Bash implements all of the traditional sh flow
control constructs (\fIfor\fP, \fIif\fP, \fIwhile\fP, etc.).
All of the Bourne shell builtins, including those not specified in
the POSIX.2 standard, appear in Bash.  Shell \fIfunctions\fP,
introduced in the SVR2 version of the Bourne shell,
are similar to shell scripts, but are defined using a special
syntax and are executed in the same process as the calling shell.
Bash has shell functions
which behave in a fashion upward-compatible with sh functions.
There are certain shell
variables that Bash interprets in the same way as sh, such as
.B PS1 ,
.B IFS ,
and
.B PATH .
Bash implements essentially the same grammar, parameter and
variable expansion semantics, redirection, and quoting as the
Bourne shell.  Where differences appear between the POSIX.2
standard and traditional sh behavior, Bash follows POSIX.
.PP
The Korn Shell (\fBksh\fP) is a descendant of the Bourne shell written
at AT&T Bell Laboratories by David Korn\(dg.  It provides a number of
useful features that POSIX and Bash have adopted.  Many of the
interactive facilities in POSIX.2 have their roots in the ksh:
for example, the POSIX and ksh job control facilities are nearly
identical. Bash includes features from the Korn Shell for both
interactive use and shell programming.  For programming, Bash provides
variables such as
.B RANDOM
and
.B REPLY ,
the
.B typeset
builtin,
the ability to remove substrings from variables based on patterns,
and shell arithmetic.
.FS
\(dgMorris Bolsky and David Korn, \fIThe KornShell Command and
Programming Language\fP, Prentice Hall, 1989.
.FE
.B RANDOM
expands to a random number each time it is referenced; assigning a
value to
.B RANDOM
seeds the random number generator.
.B REPLY
is the default variable used by the
.B read
builtin when no variable names are supplied as arguments.
The
.B typeset
builtin is used to define variables and give them attributes
such as \fBreadonly\fP.
Bash arithmetic allows the evaluation of an expression and the
substitution of the result.  Shell variables may be used as operands,
and the result of an expression may be assigned to a variable.
Nearly all of the operators from the C language are available,
with the same precedence rules:
.SE
$ echo $((3 + 5 * 32))
163
.EE
.LP
For interactive use, Bash implements ksh-style aliases and builtins
such as
.B fc
(discussed below) and
.B jobs .
Bash aliases allow a string to be substituted for a command name.
They can be used to create a mnemonic for a \s-1UNIX\s+1 command
name (\f(CRalias del=rm\fP), to expand a single word to a complex command
(\f(CRalias news='xterm -g 80x45 -title trn -e trn -e -S1 -N &'\fP), or to
ensure that a command is invoked with a basic set of options
(\f(CRalias ls="/bin/ls -F"\fP).
.PP
The C shell (\fBcsh\fP)\(dg, originally written by Bill Joy while at
Berkeley, is widely used and quite popular for its interactive
facilities.  Bash includes a csh-compatible history expansion
mechanism (\*Q! history\*U), brace expansion, access to a stack
of directories via the
.B pushd ,
.B popd ,
and
.B dirs
builtins, and tilde expansion, to generate users' home directories.
Tilde expansion has also been adopted by both the Korn Shell and
POSIX.2.
.FS
\(dgBill Joy, An Introduction to the C Shell, \fIUNIX User's Supplementary
Documents\fP, University of California at Berkeley, 1986.
.FE
.PP
There were certain areas in which POSIX.2 felt standardization
was necessary, but no existing implementation provided the proper
behavior.  The working group invented and standardized functionality
in these areas, which Bash implements.  The
.B command
builtin was invented so that shell functions could be written to
replace builtins; it makes the capabilities of the builtin
available to the function.  The reserved word \*Q!\*U was added
to negate the return value of a command or pipeline; it was nearly
impossible to express \*Qif not x\*U cleanly using the sh language.
There exist multiple incompatible implementations of the 
.B test
builtin, which tests files for type and other attributes and performs
arithmetic and string comparisons.
POSIX considered none of these correct, so the standard
behavior was specified in terms of the number of arguments to the
command.  POSIX.2 dictates exactly what will happen when four or
fewer arguments are given to
.B test ,
and leaves the behavior undefined when more arguments are supplied.
Bash uses the POSIX.2 algorithm, which was conceived by David Korn.
.NH 2
Features not in the Bourne Shell
.PP
There are a number of minor differences between Bash and the
version of sh present on most other versions of \s-1UNIX\s+1.  The majority
of these are due to the POSIX standard, but some are the result of
Bash adopting features from other shells.  For instance, Bash
includes the new \*Q!\*U reserved word, the
.B command
builtin, the ability of the
.B read
builtin to correctly return a line ending with a backslash, symbolic
arguments to the
.B umask
builtin, variable substring removal, a way to get the length of a variable,
and the new algorithm for the
.B test
builtin from the POSIX.2 standard, none of which appear in sh.
.PP
Bash also implements the \*Q$(...)\*U command substitution syntax,
which supersedes the sh `...` construct.
The \*Q$(...)\*U construct expands to the output of the command
contained within the
parentheses, with trailing newlines removed.  The sh syntax is
accepted for backwards compatibility, but the \*Q$(...)\*U form
is preferred because its quoting rules are much simpler and it
is easier to nest.
.PP
The Bourne shell does not provide such features as brace expansion,
the ability
to define a variable and a function with the same name, local variables
in shell functions, the ability to enable and disable individual
builtins or write a function to replace a builtin, or a means to
export a shell function to a child process.
.PP
Bash has closed
a long-standing shell security hole by not using the
.B $IFS
variable to split each word read by the shell, but splitting only
the results of expansion (ksh and the 4.4 BSD sh have fixed this
as well).  Useful behavior such as a means to abort
execution of a script read with the \*Q.\*U command using the
\fBreturn\fP builtin or automatically
exporting variables in the shell's environment to children is also
not present in the Bourne shell.  Bash provides a much more powerful
environment for both interactive use and programming.
.NH 1
Bash-specific Features
.PP
This section details a few of the features which make Bash unique.
Most of them provide improved interactive use, but a few programming
improvements are present as well.  Full descriptions of these
features can be found in the Bash documentation.
.NH 2
Startup Files
.PP
Bash executes startup files differently than other shells.  The Bash
behavior is a compromise between the csh principle of startup files
with fixed names executed for each shell and the sh
\*Qminimalist\*U behavior.  An interactive instance of Bash started
as a login shell reads and executes
.I ~/.bash_profile
(the file .bash_profile in the user's home directory), if it exists.
An interactive non-login shell reads and executes
.I ~/.bashrc .
A non-interactive shell (one begun to execute a shell script, for
example) reads no fixed startup file, but uses the value of the variable
.B $ENV ,
if set, as the name of a startup file.  The ksh practice of reading
.B $ENV
for every shell, with the accompanying difficulty of defining the
proper variables and functions for interactive and non-interactive
shells or having the file read only for interactive shells, was
considered too complex.  Ease of use won out here.  Interestingly,
the next release of ksh will change to reading
.B $ENV
only for interactive shells.
.NH 2
New Builtin Commands
.PP
There are a few builtins which are new or have been extended in Bash.
The
.B enable
builtin allows builtin commands to be turned on and off arbitrarily.
To use the version of
.I echo
found in a user's search path rather than the Bash builtin,
\f(CRenable -n echo\fP suffices.  The
.B help
builtin provides
quick synopses of the shell facilities without requiring
access to a manual page.
.B Builtin
is similar to
.B command
in that it bypasses shell functions and directly executes builtin
commands.  Access to a csh-style stack of directories is provided
via the
.B pushd ,
.B popd ,
and
.B dirs
builtins.
.B Pushd
and
.B popd
insert and remove directories from the stack, respectively, and
.B dirs
lists the stack contents.  On systems that allow fine-grained control
of resources, the
.B ulimit
builtin can be used to tune these settings.
.B Ulimit
allows a user to control,
among other things, whether core dumps are to be generated,
how much memory the shell or a child process is allowed to allocate,
and how large a file created by a child process can grow.  The
.B suspend
command will stop the shell process when job control is active; most
other shells do not allow themselves to be stopped like that.
.B Type,
the Bash answer to
.B which
and 
.B whence,
shows what will happen when a word is typed as a command:
.SE
$ type export
export is a shell builtin
$ type -t export
builtin
$ type bash
bash is /bin/bash
$ type cd
cd is a function
cd ()
{
    builtin cd ${1+"$@"} && xtitle $HOST: $PWD
}
.EE
.LP
Various
modes tell what a command word is (reserved word, alias, function, builtin,
or file) or which version of a command will be executed based on
a user's search path.  Some of this functionality has been adopted
by POSIX.2 and folded into the
.B command
utility.
.NH 2
Editing and Completion
.PP
One area in which Bash shines is command line editing.  Bash uses the
.I readline
library to read and edit lines when interactive.  Readline is a
powerful and flexible input facility that a user can configure to
individual tastes.  It allows lines to be edited using either emacs
or vi commands, where those commands are appropriate.  The full
capability of emacs is not present \- there is no way to execute
a named command with M-x, for instance \- but the existing commands
are more than adequate.  The vi mode is compliant with
the command line editing standardized by POSIX.2.
.PP
Readline is fully customizable.  In addition to the basic commands
and key bindings, the library allows users to define additional
key bindings using a startup file.  The 
.I inputrc
file, which defaults to the file
.I ~/.inputrc ,
is read each time readline initializes, permitting users to
maintain a consistent interface across a set of programs.  Readline
includes an extensible interface, so each program using the
library can add its own bindable commands and program-specific
key bindings.  Bash uses this facility to add bindings
that perform history expansion or shell word expansions on the current
input line.
.PP
Readline interprets a number of
variables which further tune its behavior.  Variables
exist to control whether or not eight-bit characters are directly
read as input or converted to meta-prefixed key sequences (a
meta-prefixed key sequence consists of the character with the
eighth bit zeroed, preceded by the
.I meta-prefix
character, usually escape, which selects an alternate keymap), to
decide whether to output characters with the eighth bit set
directly or as a meta-prefixed key sequence, whether or not to
wrap to a new screen line when a line being edited is longer than
the screen width, the keymap to which subsequent key bindings should
apply, or even what happens when readline wants to
ring the terminal's bell.  All of these variables can be set in
the inputrc file.
.PP
The startup file understands a set of C
preprocessor-like conditional constructs which allow variables or
key bindings to be assigned based on the application using readline,
the terminal currently being used, or the editing mode.  Users can
add program-specific bindings to make their lives easier:  I have
bindings that let me edit the value of
.B $PATH
and double-quote the current or previous word:
.SE
# Macros that are convenient for shell interaction
$if Bash
# edit the path
"\eC-xp": "PATH=${PATH}\ee\eC-e\eC-a\eef\eC-f"
# prepare to type a quoted word -- insert open and close double
# quotes and move to just after the open quote
"\eC-x\e"": "\e"\e"\eC-b"
# Quote the current or previous word
"\eC-xq": "\eeb\e"\eef\e""
$endif
.EE
.LP
There is a readline
command to re-read the file, so users can edit the file, change
some bindings, and begin to use them almost immediately.
.PP
Bash implements the
.B bind
builtin for more dynamic control of readline than the startup file
permits.
.B Bind
is used in several ways.  In
.I list
mode, it can display the current key bindings, list all the
readline editing directives available for binding, list which keys
invoke a given directive, or output the current set of key
bindings in a format that can be incorporated directly into an inputrc
file.  In
.I batch
mode, it reads a series of key bindings directly from a file and
passes them to readline.  In its most common usage,
.B bind
takes a single string and passes it directly to readline, which
interprets the line as if it had just been read from the inputrc file.
Both key bindings and variable assignments may appear in the
string given to
.B bind .
.PP
The readline library also provides an interface for \fIword completion\fP.
When the
.I completion
character (usually TAB) is typed, readline looks at the word currently
being entered and computes the set of filenames of which the current
word is a valid prefix.
If there is only one possible completion, the
rest of the characters are inserted directly, otherwise the 
common prefix of the set of filenames is added to the current word.
A second TAB character entered immediately after a non-unique
completion causes readline to list the possible completions; there is
an option to have the list displayed immediately.
Readline provides hooks so that applications can provide specific types
of completion before the default filename completion is attempted.
This is quite flexible, though it is not completely user-programmable.
Bash, for example, can complete filenames, command names (including aliases,
builtins, shell reserved words, shell functions, and executables found
in the file system), shell variables, usernames, and hostnames.  It
uses a set of heuristics that, while not perfect, is generally quite
good at determining what type of completion to attempt.
.NH 2
History
.PP
Access to the list of commands previously entered (the \fIcommand history\fP)
is provided jointly by Bash and the readline library.  Bash provides
variables (\fB$HISTFILE\fP, \fB$HISTSIZE\fP, and \fB$HISTCONTROL\fP)
and the
.B history
and
.B fc
builtins to manipulate the history list.
The value of
.B $HISTFILE
specifies the file where Bash writes the command history on exit and
reads it on startup.
.B $HISTSIZE
is used to limit the number of commands saved in the history.
.B $HISTCONTROL
provides a crude form of control over which commands are saved on
the history list: a value of
.I ignorespace
means to not save commands which begin with a space; a value of
.I ignoredups
means to not save commands identical to the last command saved.
\fB$HISTCONTROL\fP was named \fB$history_control\fP in earlier
versions of Bash; the old name is still accepted for backwards
compatibility.  The
.B history
command can read or write files containing the history list
and display the current list contents.  The
.B fc
builtin, adopted from POSIX.2 and the Korn Shell, allows display
and re-execution, with optional editing,
of commands from the history list.  The readline
library offers a set of commands to search the history list for
a portion of the current input line or a string typed by the user.
Finally, the
.I history
library, generally incorporated directly into the readline library,
implements a facility for history recall, expansion, and re-execution
of previous commands very similar to csh
(\*Qbang history\*U, so called because the exclamation point
introduces a history substitution):
.SE
$ echo a b c d e
a b c d e
$ !! f g h i
echo a b c d e f g h i
a b c d e f g h i
$ !-2
echo a b c d e
a b c d e
$ echo !-2:1-4
echo a b c d
a b c d
.EE
.LP
The command history is only
saved when the shell is interactive, so it is not available for use
by shell scripts.
.NH 2
New Shell Variables
.PP
There are a number of convenience variables that Bash interprets
to make life easier.  These include
.B FIGNORE ,
which is a set of filename suffixes identifying files to exclude when
completing filenames;
.B HOSTTYPE ,
which is automatically set to a string describing the type of
hardware on which Bash is currently executing;
.B command_oriented_history ,
which directs Bash to save all lines of a multiple-line
command such as a \fIwhile\fP or \fIfor\fP loop in a single
history entry, allowing easy re-editing; and
.B IGNOREEOF ,
whose value indicates the number of consecutive EOF characters that
an interactive shell will read before exiting \- an easy way to keep
yourself from being logged out accidentally.  The
.B auto_resume
variable alters the way the shell treats simple command names:
if job control is active, and this variable is set, single-word
simple commands without redirections cause the shell to first
look for and restart a suspended job with that name before
starting a new process.
.NH 2
Brace Expansion
.PP
Since sh offers no convenient way to generate arbitrary strings that
share a common prefix or suffix (filename expansion requires that
the filenames exist), Bash implements \fIbrace expansion\fP, a
capability picked up from csh.
Brace expansion is similar to filename expansion, but the strings
generated need not correspond to existing files.  A brace expression
consists of an optional
.I preamble ,
followed by a pair of braces enclosing a series of comma-separated
strings, and an optional
.I postamble .
The preamble is prepended to each string within the braces, and the
postamble is then appended to each resulting string:
.SE
$ echo a{d,c,b}e
ade ace abe
.EE
.LP
As this example demonstrates, the results of brace expansion are not
sorted, as they are by filename expansion.
.NH 2
Process Substitution
.PP
On systems that can support it, Bash provides a facility known as
\fIprocess substitution\fP.  Process substitution is similar to command
substitution in that its specification includes a command to execute,
but the shell does not collect the command's output and insert it into
the command line.  Rather, Bash opens a pipe to the command, which
is run in the background.  The shell uses named pipes (FIFOs) or the
.I /dev/fd
method of naming open files to expand the process
substitution to a filename which connects to the pipe when opened.
This filename becomes the result of the expansion.  Process substitution
can be used to compare the outputs of two different versions of an
application as part of a regression test:
.SE
$ cmp <(old_prog) <(new_prog)
.EE
.NH 2
Prompt Customization
.PP
One of the more popular interactive features that Bash provides is
the ability to customize the prompt.  Both
.B $PS1
and
.B $PS2,
the primary and secondary prompts, are expanded before being
displayed.  Parameter and variable expansion is performed when
the prompt string is expanded, so any shell variable can be
put into the prompt (e.g., 
.B $SHLVL ,
which indicates how deeply the current shell is nested).
Bash specially interprets characters in the prompt string
preceded by a backslash.  Some of these backslash escapes are
replaced with 
the current time, the date, the current working directory,
the username, and the command number or history number of the command
being entered.  There is even a backslash escape to cause the shell
to change its prompt when running as root after an \fIsu\fP.
Before printing each primary prompt, Bash expands the variable 
.B $PROMPT_COMMAND 
and, if it has a value, executes the expanded value as a command,
allowing additional prompt customization.  For example, this assignment
causes the current user, the current host, the time, the last
component of the current working directory, the level of shell
nesting, and the history number of the current command to be embedded
into the primary prompt:
.SE
$ PS1='\eu@\eh [\et] \eW($SHLVL:\e!)\e$ '
chet@odin [21:03:44] documentation(2:636)$ cd ..
chet@odin [21:03:54] src(2:637)$
.EE
.LP
The string being assigned is surrounded by single quotes so that if
it is exported, the value of
.B $SHLVL
will be updated by a child shell:
.SE
chet@odin [21:17:35] src(2:638)$ export PS1
chet@odin [21:17:40] src(2:639)$ bash
chet@odin [21:17:46] src(3:696)$
.EE
.LP
The \fP\e$\fP escape is displayed
as \*Q\fB$\fP\*U when running as a normal user, but as \*Q\fB#\fP\*U when
running as root.
.NH 2
File System Views
.PP
Since Berkeley introduced symbolic links in 4.2 BSD, one of their most
annoying properties has been the \*Qwarping\*U to a completely
different area of the file system when using
.B cd ,
and the resultant non-intuitive behavior of \*Q\fBcd ..\fP\*U.
The \s-1UNIX\s+1 kernel treats symbolic links
.I physically .
When the kernel is translating a pathname
in which one component is a symbolic link, it replaces all or part
of the pathname while processing the link.  If the contents of the symbolic
link begin with a slash, the kernel replaces the
pathname entirely; if not, the link contents replace
the current component.  In either case, the symbolic link
is visible.  If the link value is an absolute pathname,
the user finds himself in a completely different part of the file
system.
.PP
Bash provides a
.I logical
view of the file system.  In this default mode, command and filename
completion and builtin commands such as
.B cd
and
.B pushd
which change the current working directory transparently follow
symbolic links as if they were directories.
The
.B $PWD
variable, which holds the shell's idea of the current working directory,
depends on the path used to reach the directory rather than its
physical location in the local file system hierarchy.  For example:
.SE
$ cd /usr/local/bin
$ echo $PWD
/usr/local/bin
$ pwd
/usr/local/bin
$ /bin/pwd
/net/share/sun4/local/bin
$ cd ..
$ pwd
/usr/local
$ /bin/pwd
/net/share/sun4/local
$ cd ..
$ pwd
/usr
$ /bin/pwd
/usr
.EE
.LP
One problem with this, of
course, arises when programs that do not understand the shell's logical
notion of the file system interpret \*Q..\*U differently.  This generally
happens when Bash completes filenames containing \*Q..\*U according to a
logical hierarchy which does not correspond to their physical location. 
For users who find this troublesome, a corresponding
.I physical
view of the file system is available:
.SE
$ cd /usr/local/bin
$ pwd
/usr/local/bin
$ set -o physical
$ pwd
/net/share/sun4/local/bin
.EE
.NH 2
Internationalization
.PP
One of the most significant improvements in version 1.13 of Bash was the
change to \*Qeight-bit cleanliness\*U.  Previous versions used the
eighth bit of characters to mark whether or not they were
quoted when performing word expansions.  While this did not affect
the majority of users, most of whom used only seven-bit ASCII characters,
some found it confining.  Beginning with version 1.13, Bash
implemented a different quoting mechanism that did not alter the
eighth bit of characters.  This allowed Bash
to manipulate files with \*Qodd\*U characters in their names, but
did nothing to help users enter those names, so
version 1.13 introduced changes to readline that
made it eight-bit clean as well.  Options exist that force readline to
attach no special significance to characters with the eighth bit set
(the default behavior is to convert these characters to meta-prefixed
key sequences) and to output these characters without conversion to
meta-prefixed sequences.  These changes, along with the expansion of
keymaps to a full eight bits, enable readline to work with most of the
ISO-8859 family of character sets, used by many European countries.
.NH 2
POSIX Mode
.PP
Although Bash is intended to be POSIX.2 conformant, there are areas in
which the default behavior is not compatible with the standard.  For
users who wish to operate in a strict POSIX.2 environment, Bash
implements a \fIPOSIX mode\fP.  When this mode is active, Bash modifies
its default operation where it differs from POSIX.2 to match the
standard.  POSIX mode is entered when Bash is started with the
.B -posix
option.  This feature is also available as an option to the
\fBset\fP builtin, \fBset -o posix\fP.
For compatibility with other GNU software that attempts to be POSIX.2
compliant, Bash also enters POSIX mode if the variable
.B $POSIXLY_CORRECT
is set when Bash is started or assigned a value during execution.
.B $POSIX_PEDANTIC
is accepted as well, to be compatible with some older GNU utilities.
When Bash is started in POSIX mode, for example, it sources the
file named by the value of 
.B $ENV
rather than the \*Qnormal\*U startup files, and does not allow
reserved words to be aliased.
.NH 1
New Features and Future Plans
.PP
There are several features introduced in the current
version of Bash, version 1.14, and a number under consideration
for future releases.  This section will briefly detail the new
features in version 1.14 and describe several features
that may appear in later versions.
.NH 2
New Features in Bash-1.14
.PP
The new features available in Bash-1.14 answer several of
the most common requests for enhancements.  Most notably, there
is a mechanism
for including non-visible character sequences in prompts, such as
those which cause a terminal to print characters in different
colors or in standout mode.  There was nothing preventing the use
of these sequences in earlier
versions, but the readline redisplay algorithm assumed each
character occupied physical screen space and would wrap lines
prematurely.
.PP
Readline has a few new
variables, several new bindable commands, and some additional
emacs mode default key bindings.  A new history search
mode has been implemented: in this mode, readline searches the
history for lines beginning with the characters between the
beginning of the current line and the cursor.  The existing readline
incremental search commands no longer match identical lines more
than once.
Filename completion now expands variables in directory names.
The history expansion facilities are now nearly
completely csh-compatible: missing modifiers have been added and
history substitution has been extended.
.PP
Several of the features described earlier, such as
.B "set -o posix"
and
.B $POSIX_PEDANTIC ,
are new in version 1.14.
There is a new shell variable,
.B OSTYPE ,
to which Bash assigns a value that identifies the
version of \s-1UNIX\s+1 it's
running on (great for putting architecture-specific binary directories
into the \fB$PATH\fP).
Two variables have been renamed:
.B $HISTCONTROL
replaces
.B $history_control ,
and
.B $HOSTFILE
replaces
.B $hostname_completion_file .
In both cases, the old names are accepted for backwards
compatibility.  The ksh
.I select
construct, which allows the generation of simple menus,
has been implemented.  New capabilities have been added
to existing variables:
.B $auto_resume
can now take values of
.I exact
or
.I substring ,
and
.B $HISTCONTROL
understands the value
.I ignoreboth ,
which combines the two previously acceptable values.  The
.B dirs
builtin has acquired options to print out specific members of the
directory stack.  The
.B $nolinks
variable, which forces a physical view of the file system,
has been superseded by the
.B \-P
option to the
.B set
builtin (equivalent to \fBset -o physical\fP); the variable is retained
for backwards compatibility.  The version string contained in
.B $BASH_VERSION
now includes an indication of the patch level as well as the
\*Qbuild version\*U.
Some little-used features have
been removed:  the
.B bye
synonym for
.B exit
and the
.B $NO_PROMPT_VARS
variable are gone.  There is now an organized test suite that can be
run as a regression test when building a new version of Bash.
.PP
The documentation has been thoroughly overhauled:
there is a new manual page on the readline library and the \fIinfo\fP
file has been updated to reflect the current version.
As always, as many bugs as possible have been fixed, although some
surely remain.
.NH 2
Other Features
.PP
There are a few features that I hope to include in later Bash releases.
Some are based on work already done in other shells.
.PP
In addition to simple variables, a future release of Bash will include
one-dimensional arrays, using the ksh
implementation of arrays as a model.  Additions to the ksh syntax,
such as \fIvarname\fP=( ... ) to assign a list of words directly to
an array and a mechanism to allow
the
.B read
builtin to read a list of values directly into an array, would be
desirable.  Given those extensions, the ksh
.B "set \-A"
syntax may not be worth supporting (the
.B \-A
option assigns a list of values to an array, but is a rather
peculiar special case).
.PP
Some shells include a means of \fIprogrammable\fP word
completion, where the user specifies on a per-command basis how the
arguments of the command are to be treated when completion is attempted:
as filenames, hostnames, executable files, and so on.  The other
aspects of the current Bash implementation could remain as-is; the
existing heuristics would still be valid.  Only when completing the
arguments to a simple command would the programmable completion be
in effect.
.PP
It would also be nice to give the user finer-grained
control over which commands are saved onto the history list.  One
proposal is for a variable, tentatively named
.B HISTIGNORE ,
which would contain a colon-separated list of commands.  Lines beginning
with these commands, after the restrictions of
.B $HISTCONTROL
have been applied, would not be placed onto the history list.  The
shell pattern-matching capabilities could also be available when
specifying the contents of
.B $HISTIGNORE .
.PP
One thing that newer shells such as
.B wksh
(also known as
.B dtksh )
provide is a command to dynamically load code
implementing additional builtin commands into a running shell.
This new builtin would take an object file or shared library
implementing the \*Qbody\*U of the
builtin (\fIxxx_builtin()\fP for those familiar with Bash internals)
and a structure containing the name of the new command, the function
to call when the new builtin is invoked (presumably defined in the
shared object specified as an argument), and the documentation to be
printed by the
.B help
command (possibly present in the shared object as well).  It would
manage the details of extending the internal table of builtins. 
.PP
A few other builtins would also be desirable: two are the POSIX.2
.B getconf
command, which prints the values of system configuration variables
defined by POSIX.2, and a
.B disown
builtin, which causes a shell running
with job control active to \*Qforget about\*U one or more
background jobs in its internal jobs table.  Using
.B getconf ,
for example, a user could retrieve a value for
.B $PATH
guaranteed to find all of the POSIX standard utilities, or
find out how long filenames may be in the file system containing
a specified directory.
.PP
There are no implementation timetables for any of these features, nor
are there concrete plans to include them.  If anyone has comments on
these proposals, feel free to send me electronic mail.
.NH 1
Reflections and Lessons Learned
.PP
The lesson that has been repeated most often during Bash
development is that there are dark corners in the Bourne shell,
and people use all of them.  In the original description of the
Bourne shell, quoting and the shell grammar are both poorly
specified and incomplete; subsequent descriptions have not helped
much.  The grammar presented in Bourne's paper describing
the shell distributed with the Seventh Edition of \s-1UNIX\s+1\(dg
is so far off that it does not allow the command \f(CWwho|wc\fP.
In fact, as Tom Duff states:
.QP
Nobody really knows what the
Bourne shell's grammar is.  Even examination of the source code is
little help.\(dd
.FS
\(dgS. R. Bourne, \*QUNIX Time-Sharing System:  The UNIX Shell\*U,
\fIBell System Technical Journal\fP, 57(6), July-August, 1978, pp. 1971-1990.
.FE
.FS
\(ddTom Duff, \*QRc \- A Shell for Plan 9 and \s-1UNIX\s+1 systems\*U,
\fIProc. of the Summer 1990 EUUG Conference\fP, London, July, 1990,
pp. 21-33.
.FE
.LP
The POSIX.2 standard includes a \fIyacc\fP grammar that comes close
to capturing the Bourne shell's behavior, but it disallows some
constructs which sh accepts without complaint \- and there are
scripts out there that use them.  It took a few versions and
several bug reports before Bash implemented sh-compatible quoting,
and there are still some \*Qlegal\*U sh constructs which Bash flags as
syntax errors.  Complete sh compatibility is a tough nut.
.PP
The shell is bigger and slower than I would like, though the current
version is substantially faster than previously.  The readline library
could stand a substantial rewrite.  A hand-written parser to replace
the current \fIyacc\fP-generated one would probably result in a speedup,
and would solve one glaring problem:  the shell could parse
commands in \*Q$(...)\*U constructs
as they are entered, rather than reporting errors when the construct
is expanded.
.PP
As always, there is some chaff to go with the wheat.
Areas of duplicated functionality need to be cleaned
up.  There are several cases where Bash treats a variable specially to
enable functionality available another way (\fB$notify\fP vs.
\fBset -o notify\fP and \fB$nolinks\fP vs. \fBset -o physical\fP, for
instance); the special treatment of the variable name should probably
be removed.  A few more things could stand removal; the
.B $allow_null_glob_expansion
and
.B $glob_dot_filenames
variables are of particularly questionable value.
The \fB$[...]\fP arithmetic evaluation syntax is redundant now that
the POSIX-mandated \fB$((...))\fP construct has been implemented,
and could be deleted.
It would be nice if the text output by the
.B help
builtin were external to the shell rather than compiled into it.
The behavior enabled by
.B $command_oriented_history ,
which causes the shell to attempt to save all lines of a multi-line
command in a single history entry, should be made the default and
the variable removed.
.NH 1
Availability
.PP
As with all other
GNU software, Bash is available for anonymous FTP from
.I prep.ai.mit.edu:/pub/gnu
and from other GNU software mirror sites.  The current version is in
.I bash-1.14.1.tar.gz
in that directory.  Use
.I archie
to find the nearest archive site.  The
latest version is always available for FTP from
.I bash.CWRU.Edu:/pub/dist.
Bash documentation is available for FTP from
.I bash.CWRU.Edu:/pub/bash.
.PP
The Free Software Foundation sells tapes and CD-ROMs
containing Bash; send electronic mail to
\f(CRgnu@prep.ai.mit.edu\fP or call \f(CR+1-617-876-3296\fP
for more information.
.PP
Bash is also distributed with several versions of \s-1UNIX\s+1-compatible
systems.  It is included as /bin/sh and /bin/bash on several Linux
distributions (more about the difference in a moment), and as contributed
software in BSDI's BSD/386* and FreeBSD.
.FS
*BSD/386 is a trademark of Berkeley Software Design, Inc.
.FE
.PP
The Linux distribution deserves special mention.  There are two
configurations included in the standard Bash distribution: a
\*Qnormal\*U configuration, in which all of the standard features
are included, and a \*Qminimal\*U configuration, which omits job
control, aliases, history and command line editing, the directory
stack and
.B pushd/popd/dirs,
process substitution, prompt string special character decoding, and the
.I select
construct.  This minimal version is designed to be a drop-in replacement
for the traditional \s-1UNIX\s+1 /bin/sh, and is included as the Linux
/bin/sh in several packagings.
.NH 1
Conclusion
.PP
Bash is a worthy successor to sh.
It is sufficiently portable
to run on nearly every version of \s-1UNIX\s+1 from
4.3 BSD to SVR4.2, and several \s-1UNIX\s+1 workalikes.
It is robust enough to replace sh on most of those systems,
and provides more functionality.  It has several thousand regular users,
and their feedback has helped to make it as good as it is today \- a
testament to the benefits of free software.
