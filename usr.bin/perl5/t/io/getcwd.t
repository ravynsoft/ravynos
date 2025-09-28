#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
}

defined &Internals::getcwd
  or plan skip_all => "no Internals::getcwd";

my $cwd = Internals::getcwd();

if (ok(defined $cwd, "Internals::getcwd() returned a defined result")) {
    isnt($cwd, "", "Internals::getcwd() returned a non-empty result");
    ok(-d $cwd, "Internals::getcwd() result is a directory");
}

done_testing();
