#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan 6;

# Need to run this in a quiet private directory as it assumes that it can
# reliably delete fixed file names.
my $tempdir = tempfile;

mkdir $tempdir, 0700 or die "Can't mkdir '$tempdir': $!";
chdir $tempdir or die die "Can't chdir '$tempdir': $!";

sub make_file {
  my $file = shift;
  open my $fh, ">", $file or die "Can't open $file: $!";
  close $fh or die "Can't close $file: $!";
}

make_file('aaa');
is unlink('aaa'), 1, 'retval of unlink with one file name';
ok (!-e 'aaa', 'unlink unlinked it');
make_file($_) for 'aaa', 'bbb';
is unlink('aaa','bbb','ccc'), 2,
    'retval of unlink with list that includes nonexistent file';
ok (!-e 'aaa' && !-e 'bbb', 'unlink unlank the files it claims it unlank');
$_ = 'zzz';
make_file 'zzz';
is unlink, 1, 'retval of unlink with no args';
ok !-e 'zzz', 'unlink with no arg unlinked $_';


chdir '..' or die "Couldn't chdir .. for cleanup: $!";
rmdir $tempdir or die "Couldn't unlink tempdir '$tempdir': $!";
