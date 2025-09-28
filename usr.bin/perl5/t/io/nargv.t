#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
}

print "1..7\n";

my $j = 1;
for $i ( 1,2,5,4,3 ) {
    $file = mkfiles($i);
    open(FH, "> $file") || die "can't create $file: $!";
    print FH "not ok " . $j++ . "\n";
    close(FH) || die "Can't close $file: $!";
}


{
    local *ARGV;
    local $^I = '.bak';
    local $_;
    @ARGV = mkfiles(1..3);
    $n = 0;
    while (<>) {
	print STDOUT "# initial \@ARGV: [@ARGV]\n";
	if ($n++ == 2) {
	    other();
	}
	show();
    }
}

$^I = undef;
@ARGV = mkfiles(1..3);
$n = 0;
while (<>) {
    print STDOUT "#final \@ARGV: [@ARGV]\n";
    if ($n++ == 2) {
	other();
    }
    show();
}

# test setuid is preserved (and hopefully setgid)
#
# With nested in-place editing PL_oldname and PL_filemode would
# be overwritten by the values for the last file in the nested
# loop.  This is now all stored as magic in *ARGVOUT{IO}
$^I = "";
@ARGV = mkfiles(1..3);
my $sidfile = $ARGV[1];
chmod(04600, $sidfile);
my $mode = (stat $ARGV[1])[2];
$n = 0;
while (<>) {
    print STDOUT "#final \@ARGV: [@ARGV]\n";
    if ($n++ == 1) {
	other();
    }
    print;
}
my $newmode = (stat $sidfile)[2];
printf "# before %#o after %#o\n", $mode, $newmode;
print +($mode == $newmode ? "" : "not "). "ok 6 # check setuid mode preserved\n";

sub show {
    #warn "$ARGV: $_";
    s/^not //;
    print;
}

sub other {
    no warnings 'once';
    print STDOUT "# Calling other\n";
    local *ARGV;
    local *ARGVOUT;
    local $_;
    @ARGV = mkfiles(5, 4);
    while (<>) {
	print STDOUT "# inner \@ARGV: [@ARGV]\n";
	show();
    }
}

{
    # (perl #133314) directory handle leak
    #
    # We process a significant number of files here to make sure any
    # leaks are significant
    @ARGV = mkfiles(1 .. 10);
    for my $file (@ARGV) {
        open my $f, ">", $file;
        print $f "\n";
        close $f;
    }
    local $^I = ".bak";
    local $_;
    while (<>) {
        s/^/foo/;
    }
}

{
    # (perl #133314) directory handle leak
    # We open three handles here because the file processing opened:
    #  - the original file
    #  - the output file, and finally
    #  - the directory
    # so we need to open the first two to use up the slots used for the original
    # and output files.
    # This test assumes fd are allocated in the typical *nix way - lowest
    # available, which I believe is the case for the Win32 CRTs too.
    # If this turns out not to be the case this test will need to skip on
    # such platforms or only run on a small set of known-good platforms.
    my $tfile = mkfiles(1);
    open my $f, "<", $tfile
      or die "Cannot open temp: $!";
    open my $f2, "<", $tfile
      or die "Cannot open temp: $!";
    open my $f3, "<", $tfile
      or die "Cannot open temp: $!";
    print +(fileno($f3) < 20 ? "ok" : "not ok"), " 7 check fd leak\n";
    close $f;
    close $f2;
    close $f3;
}


my @files;
sub mkfiles {
    foreach (@_) {
	$files[$_] ||= tempfile();
    }
    my @results = @files[@_];
    return wantarray ? @results : @results[-1];
}

END { unlink_all map { ($_, "$_.bak") } @files }
