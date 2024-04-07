    eval 'exec perl -x -S "$0" ${1+"$@"}'
	if 0;	# In case running under some shell

require 5;
use ExtUtils::PL2Bat;
use Getopt::Std;

$0 =~ s|.*[/\\]||;

my $usage = <<EOT;
Usage:  $0 [-h]
   or:  $0 [-w] [-u] [-a argstring] [-s stripsuffix] [files]
   or:  $0 [-w] [-u] [-n ntargs] [-o otherargs] [-s stripsuffix] [files]
        -n ntargs       arguments to invoke perl with in generated file
                            when run from Windows NT.  Defaults to
                            '-x -S %0 %*'.
        -o otherargs    arguments to invoke perl with in generated file
                            other than when run from Windows NT.  Defaults
                            to '-x -S "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9'.
        -u              update files that may have already been processed
                            by (some version of) pl2bat.
        -w              include "-w" on the /^#!.*perl/ line (unless
                            a /^#!.*perl/ line was already present).
        -s stripsuffix  strip this suffix from file before appending ".bat"
                            Not case-sensitive
                            Can be a regex if it begins with '/'
                            Defaults to "/\.plx?/"
        -h              show this help
EOT

my %OPT = ();
warn($usage), exit(0) if !getopts('whun:o:a:s:',\%OPT) or $OPT{'h'};
die '-a option has been removed' if $OPT{a};

my %key_for = (
	n => 'ntargs',
	o => 'otherargs',
	a => 'argstring',
	u => 'update',
	w => 'usewarnings'
);

my %args;
for my $old_key (keys %key_for) {
	if (exists $OPT{$old_key}) {
		$args{$key_for{$old_key}} = $OPT{$old_key};
	}
}
if (exists $OPT{s}) {
	$args{strip_suffix} = $OPT{'s'} =~ m#^/([^/]*[^/\$]|)\$?/?$# ? qr/$1/ : qr/\Q$OPT{'s'}\E/;
}

for my $file (@ARGV) {
	pl2bat(%args, in => $file);
}

__END__

=head1 NAME

pl2bat - wrap perl code into a batch file

=head1 SYNOPSIS

B<pl2bat> B<-h>

B<pl2bat> [B<-w>] S<[B<-a> I<argstring>]> S<[B<-s> I<stripsuffix>]> [files]

B<pl2bat> [B<-w>] S<[B<-n> I<ntargs>]> S<[B<-o> I<otherargs>]> S<[B<-s> I<stripsuffix>]> [files]

=head1 DESCRIPTION

This utility converts a perl script into a batch file that can be
executed on DOS-like operating systems.  This is intended to allow
you to use a Perl script like regular programs and batch files where
you just enter the name of the script [probably minus the extension]
plus any command-line arguments and the script is found in your B<PATH>
and run.

=head2 ADVANTAGES

There are several alternatives to this method of running a Perl script. 
They each have disadvantages that help you understand the motivation
for using B<pl2bat>.

=over

=item 1

    C:> perl x:/path/to/script.pl [args]

=item 2

    C:> perl -S script.pl [args]

=item 3

    C:> perl -S script [args]

=item 4

    C:> ftype Perl=perl.exe "%1" %*
    C:> assoc .pl=Perl
    then
    C:> script.pl [args]

=item 5

    C:> ftype Perl=perl.exe "%1" %*
    C:> assoc .pl=Perl
    C:> set PathExt=%PathExt%;.PL
    then
    C:> script [args]

=back

B<1> and B<2> are the most basic invocation methods that should work on
any system [DOS-like or not].  They require extra typing and require
that the script user know that the script is written in Perl.  This
is a pain when you have lots of scripts, some written in Perl and some
not.  It can be quite difficult to keep track of which scripts need to
be run through Perl and which do not.  Even worse, scripts often get
rewritten from simple batch files into more powerful Perl scripts in
which case these methods would require all existing users of the scripts
be updated.

B<3> works on modern Win32 versions of Perl.  It allows the user to
omit the ".pl" or ".bat" file extension, which is a minor improvement.

B<4> and B<5> work on some Win32 operating systems with some command
shells.  One major disadvantage with both is that you can't use them
in pipelines nor with file redirection.  For example, none of the
following will work properly if you used method B<4> or B<5>:

    C:> script.pl <infile
    C:> script.pl >outfile
    C:> echo y | script.pl
    C:> script.pl | more

This is due to a Win32 bug which Perl has no control over.  This bug
is the major motivation for B<pl2bat> [which was originally written
for DOS] being used on Win32 systems.

Note also that B<5> works on a smaller range of combinations of Win32
systems and command shells while B<4> requires that the user know
that the script is a Perl script [because the ".pl" extension must
be entered].  This makes it hard to standardize on either of these
methods.

=head2 DISADVANTAGES

There are several potential traps you should be aware of when you
use B<pl2bat>.

The generated batch file is initially processed as a batch file each
time it is run.  This means that, to use it from within another batch
file you should precede it with C<call> or else the calling batch
file will not run any commands after the script:

    call script [args]

Except under Windows NT, if you specify more than 9 arguments to
the generated batch file then the 10th and subsequent arguments
are silently ignored.

Except when using F<CMD.EXE> under Windows NT, if F<perl.exe> is not
in your B<PATH>, then trying to run the script will give you a generic
"Command not found"-type of error message that will probably make you
think that the script itself is not in your B<PATH>.  When using
F<CMD.EXE> under Windows NT, the generic error message is followed by
"You do not have Perl in your PATH", to make this clearer.

On most DOS-like operating systems, the only way to exit a batch file
is to "fall off the end" of the file.  B<pl2bat> implements this by
doing C<goto :endofperl> and adding C<__END__> and C<:endofperl> as
the last two lines of the generated batch file.  This means:

=over

=item No line of your script should start with a colon.

In particular, for this version of B<pl2bat>, C<:endofperl>,
C<:WinNT>, and C<:script_failed_so_exit_with_non_zero_val> should not
be used.

=item Care must be taken when using C<__END__> and the C<DATA> file handle.

One approach is:

    .  #!perl
    .  while( <DATA> ) {
    .	  last   if  /^__END__$/;
    .	  [...]
    .  }
    .  __END__
    .  lines of data
    .  to be processed
    .  __END__
    .  :endofperl

The dots in the first column are only there to prevent F<cmd.exe> to interpret
the C<:endofperl> line in this documentation.  Otherwise F<pl2bat.bat> itself
wouldn't work.  See the previous item. :-)

=item The batch file always "succeeds"

The following commands illustrate the problem:

    C:> echo exit(99); >fail.pl
    C:> pl2bat fail.pl
    C:> perl -e "print system('perl fail.pl')"
    99
    C:> perl -e "print system('fail.bat')"
    0

So F<fail.bat> always reports that it completed successfully.  Actually,
under Windows NT, we have:

    C:> perl -e "print system('fail.bat')"
    1

So, for Windows NT, F<fail.bat> fails when the Perl script fails, but
the return code is always C<1>, not the return code from the Perl script.

=back

=head2 FUNCTION

By default, the ".pl" suffix will be stripped before adding a ".bat" suffix
to the supplied file names.  This can be controlled with the C<-s> option.

The default behavior is to have the batch file compare the C<OS>
environment variable against C<"Windows_NT">.  If they match, it
uses the C<%*> construct to refer to all the command line arguments
that were given to it, so you'll need to make sure that works on your
variant of the command shell.  It is known to work in the F<CMD.EXE> shell
under Windows NT.  4DOS/NT users will want to put a C<ParameterChar = *>
line in their initialization file, or execute C<setdos /p*> in
the shell startup file.

On Windows95 and other platforms a nine-argument limit is imposed
on command-line arguments given to the generated batch file, since
they may not support C<%*> in batch files.

These can be overridden using the C<-n> and C<-o> options or the
deprecated C<-a> option.

=head1 OPTIONS

=over 8

=item B<-n> I<ntargs>

Arguments to invoke perl with in generated batch file when run from
Windows NT (or Windows 98, probably).  Defaults to S<'-x -S %0 %*'>.

=item B<-o> I<otherargs>

Arguments to invoke perl with in generated batch file except when
run from Windows NT (ie. when run from DOS, Windows 3.1, or Windows 95).
Defaults to S<'-x -S "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9'>.

=item B<-a> I<argstring>

Arguments to invoke perl with in generated batch file.  Specifying
B<-a> prevents the batch file from checking the C<OS> environment
variable to determine which operating system it is being run from.

=item B<-s> I<stripsuffix>

Strip a suffix string from file name before appending a ".bat"
suffix.  The suffix is not case-sensitive.  It can be a regex if
it begins with '/' (the trailing '/' is optional and a trailing
C<$> is always assumed).  Defaults to C</.plx?/>.

=item B<-w>

If no line matching C</^#!.*perl/> is found in the script, then such
a line is inserted just after the new preamble.  The exact line
depends on C<$Config{startperl}> [see L<Config>].  With the B<-w>
option, C<" -w"> is added after the value of C<$Config{startperl}>.
If a line matching C</^#!.*perl/> already exists in the script,
then it is not changed and the B<-w> option is ignored.

=item B<-u>

If the script appears to have already been processed by B<pl2bat>,
then the script is skipped and not processed unless B<-u> was
specified.  If B<-u> is specified, the existing preamble is replaced.

=item B<-h>

Show command line usage.

=back

=head1 EXAMPLES

	C:\> pl2bat foo.pl bar.PM 
	[..creates foo.bat, bar.PM.bat..]

	C:\> pl2bat -s "/\.pl|\.pm/" foo.pl bar.PM
	[..creates foo.bat, bar.bat..]

	C:\> pl2bat < somefile > another.bat

	C:\> pl2bat > another.bat
	print scalar reverse "rekcah lrep rehtona tsuj\n";
	^Z
	[..another.bat is now a certified japh application..]

	C:\> ren *.bat *.pl
	C:\> pl2bat -u *.pl
	[..updates the wrapping of some previously wrapped scripts..]

	C:\> pl2bat -u -s .bat *.bat
	[..same as previous example except more dangerous..]

=head1 BUGS

C<$0> will contain the full name, including the ".bat" suffix
when the generated batch file runs.  If you don't like this,
see runperl.bat for an alternative way to invoke perl scripts.

Default behavior is to invoke Perl with the B<-S> flag, so Perl will
search the B<PATH> to find the script.   This may have undesirable
effects.

On really old versions of Win32 Perl, you can't run the script
via

    C:> script.bat [args]

and must use

    C:> script [args]

A loop should be used to build up the argument list when not on
Windows NT so more than 9 arguments can be processed.

See also L</DISADVANTAGES>.

=head1 SEE ALSO

perl, perlwin32, runperl.bat

=cut

