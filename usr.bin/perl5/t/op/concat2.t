#!./perl

# This file is for concatenation tests that require test.pl.
#
# t/opbasic/concat.t cannot use test.pl as
# it needs to avoid using concatenation in
# its ok() function.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan 4;

# This test is in the file because overload.pm uses concatenation.
{ package o; use overload '""' => sub { $_[0][0] } }
$x = bless[chr 256],o::;
"$x";
$x->[0] = "\xff";
$x.= chr 257;
$x.= chr 257;
is $x, "\xff\x{101}\x{101}", '.= is not confused by changing utf8ness';

# RT #132385
# in multiconcat, each const TEMP used for overloading should be distinct

package RT132385 {
    my @a;
    use overload '.' => sub { push @a, \$_[1]; $_[0] };
    my $o = bless [];
    my $x = $o . "A" . $o . 'B';
    ::is "${$a[0]}${$a[2]}", "AB", "RT #132385";
}



# Ops should not share the same TARG between recursion levels.  This may
# affect other ops, too, but concat seems more susceptible to this than
# others, since it can call itself recursively.  (Where else would I put
# this test, anyway?)
fresh_perl_is <<'end', "tmp\ntmp\n", {},
 sub canonpath {
     my ($path) = @_;
     my $node = '';
     $path =~ s|/\z||;
     return "$node$path";
 }
 
 {
  package Path::Class::Dir;
  use overload q[""] => sub { ::canonpath("tmp") };
 }
 
 print canonpath("tmp"), "\n";
 print canonpath(bless {},"Path::Class::Dir"), "\n";
end
 "recursive concat does not share TARGs";

# don't include the assign as part of the multiconcat if the target
# includes 'local'. This used to screw up on magic vars because the
# 'local $~' was done (thus emptying the var) before multiconcat was
# called.


{
    local $~ = 'FOO';
    my $s;
    {
        local $~ = "$~X";
        $s = $~;
    }
    is($s, 'FOOX', 'local $magic_var = "...."');
}
