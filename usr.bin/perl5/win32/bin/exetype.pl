#!perl -w
use strict;

# All the IMAGE_* structures are defined in the WINNT.H file
# of the Microsoft Platform SDK.

my %subsys = (NATIVE    => 1,
              WINDOWS   => 2,
              CONSOLE   => 3,
              POSIX     => 7,
              WINDOWSCE => 9);

unless (0 < @ARGV && @ARGV < 3) {
    printf "Usage: $0 exefile [%s]\n", join '|', sort keys %subsys;
    exit;
}

$ARGV[1] = uc $ARGV[1] if $ARGV[1];
unless (@ARGV == 1 || defined $subsys{$ARGV[1]}) {
    (my $subsys = join(', ', sort keys %subsys)) =~ s/, (\w+)$/ or $1/;
    print "Invalid subsystem $ARGV[1], please use $subsys\n";
    exit;
}

my ($record,$magic,$signature,$offset,$size);
open EXE, '+<', $ARGV[0] or die "Cannot open $ARGV[0]: $!\n";
binmode EXE;

# read IMAGE_DOS_HEADER structure
read EXE, $record, 64;
($magic,$offset) = unpack "Sx58L", $record;

die "$ARGV[0] is not an MSDOS executable file.\n"
    unless $magic == 0x5a4d; # "MZ"

# read signature, IMAGE_FILE_HEADER and first WORD of IMAGE_OPTIONAL_HEADER
seek EXE, $offset, 0;
read EXE, $record, 4+20+2;
($signature,$size,$magic) = unpack "Lx16Sx2S", $record;

die "PE header not found" unless $signature == 0x4550; # "PE\0\0"

die "Optional header is neither in NT32 nor in NT64 format"
    unless ($size == 224 && $magic == 0x10b) || # IMAGE_NT_OPTIONAL_HDR32_MAGIC
           ($size == 240 && $magic == 0x20b);   # IMAGE_NT_OPTIONAL_HDR64_MAGIC

# Offset 68 in the IMAGE_OPTIONAL_HEADER(32|64) is the 16 bit subsystem code
seek EXE, $offset+4+20+68, 0;
if (@ARGV == 1) {
    read EXE, $record, 2;
    my ($subsys) = unpack "S", $record;
    $subsys = {reverse %subsys}->{$subsys} || "UNKNOWN($subsys)";
    print "$ARGV[0] uses the $subsys subsystem.\n";
}
else {
    print EXE pack "S", $subsys{$ARGV[1]};
}
close EXE;
__END__

=head1 NAME

exetype - Change executable subsystem type between "Console" and "Windows"

=head1 SYNOPSIS

	C:\perl\bin> copy perl.exe guiperl.exe
	C:\perl\bin> exetype guiperl.exe windows

=head1 DESCRIPTION

This program edits an executable file to indicate which subsystem the
operating system must invoke for execution.

You can specify any of the following subsystems:

=over

=item CONSOLE

The CONSOLE subsystem handles a Win32 character-mode application that
use a console supplied by the operating system.

=item WINDOWS

The WINDOWS subsystem handles an application that does not require a
console and creates its own windows, if required.

=item NATIVE

The NATIVE subsystem handles a Windows NT device driver.

=item WINDOWSCE

The WINDOWSCE subsystem handles Windows CE consumer electronics
applications.

=item POSIX

The POSIX subsystem handles a POSIX application in Windows NT.

=back

=head1 AUTHOR

Jan Dubois <jand@activestate.com>

=cut
