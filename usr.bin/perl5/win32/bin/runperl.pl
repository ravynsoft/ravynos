#!perl -w
$0 =~ s|\.bat||i;
unless (-f $0) {
    $0 =~ s|.*[/\\]||;
    for (".", split ';', $ENV{PATH}) {
	$_ = "." if $_ eq "";
	$0 = "$_/$0" , goto doit if -f "$_/$0";
    }
    die "'$0' not found.\n";
}
doit: exec "perl", "-x", $0, @ARGV;
die "Failed to exec '$0': $!";
__END__

=head1 NAME

runperl.bat - "universal" batch file to run perl scripts

=head1 SYNOPSIS

	C:\> copy runperl.bat foo.bat
	C:\> foo
	[..runs the perl script 'foo'..]

	C:\> foo.bat
	[..runs the perl script 'foo'..]


=head1 DESCRIPTION

This file can be copied to any file name ending in the ".bat" suffix.
When executed on a DOS-like operating system, it will invoke the perl
script of the same name, but without the ".bat" suffix.  It will
look for the script in the same directory as itself, and then in
the current directory, and then search the directories in your PATH.

It relies on the C<exec()> operator, so you will need to make sure
that works in your perl.

This method of invoking perl scripts has some advantages over
batch-file wrappers like C<pl2bat.bat>:  it avoids duplication
of all the code; it ensures C<$0> contains the same name as the
executing file, without any egregious ".bat" suffix; it allows
you to separate your perl scripts from the wrapper used to
run them; since the wrapper is generic, you can use symbolic
links to simply link to C<runperl.bat>, if you are serving your
files on a filesystem that supports that.

On the other hand, if the batch file is invoked with the ".bat"
suffix, it does an extra C<exec()>.  This may be a performance
issue.  You can avoid this by running it without specifying
the ".bat" suffix.

Perl is invoked with the -x flag, so the script must contain
a C<#!perl> line.  Any flags found on that line will be honored.

=head1 BUGS

Perl is invoked with the -S flag, so it will search the PATH to find
the script.  This may have undesirable effects.

=head1 SEE ALSO

perl, perlwin32, pl2bat.bat

=cut

