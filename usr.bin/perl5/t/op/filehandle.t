#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
    skip_all_if_miniperl("no dynamic loading on miniperl, no IO, hence no FileHandle");
}

plan 4;
use FileHandle;

my $str = "foo";
open my $fh, "<", \$str;
is <$fh>, "foo", "open fh to reference to string: got expected content";

eval {
   $fh->seek(0, 0);
   is $fh->tell, 0, "after 'seek' and 'tell', got expected current fh position in bytes";
   is <$fh>, "foo", "after 'seek' and 'tell', still got expected content";
};

is $@, '', "no errors after 'seek' or 'tell'";
