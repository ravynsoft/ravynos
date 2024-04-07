#!./perl

# This script checks that magic attached to global variables ($!, %SIG,
# etc.) only applies to the globals, and not to similarly-named variables
# in other packages (%Net::DNS::RR::SIG, ${"'Oh no'!"}, etc.).

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

# Hack to allow test counts to be specified piecemeal
BEGIN { ++$INC{'tests.pm'} }
sub tests::VERSION { $tests += pop };
plan (tests => $tests);


use tests 2; # First make sure that %! %- %+ do not load extra modules.
map %{"foo::$_"}, qw< ! - + >;
ok !exists $INC{'Errno.pm'}, '$swext::! does not load Errno';

ok !exists $INC{'Tie/Hash/NamedCapture.pm'},
  '$foo::+ and $foo::- do not load Tie::Hash::NamedCapture';

use tests 1; # ARGV
fresh_perl_is
 '$count=0; ++$count while(<foo::ARGV>); print $count',
 '0',
  { stdin => 'swext\n' },
 '<foo::ARGV> does not iterate through STDIN';

use tests 1; # %SIG
ok !scalar keys %foo::SIG, "%foo::SIG";

use tests 3; # rw ${^LETTERS} variables
for(qw< CHILD_ERROR_NATIVE UTF8CACHE WARNING_BITS >) {
 my $name = s/./"qq|\\c$&|"/ere;
 local $$name = 'swit';

 # Bring it into existence first, as defined() sometimes takes shortcuts
 ${"foo::$name"};

 ok !defined(${"foo::$name"}), "\$foo::^$_";
}

use tests 6; # read-only ${^LETTERS}
for(qw< MATCH PREMATCH POSTMATCH TAINT UNICODE UTF8LOCALE >) {
 ok eval { ${"foo::" . s/./"qq|\\c$&|"/ere} = 'prile' }, "\$foo::^$_";
}

use tests 16; # $<digits> and $<single digit> (regexp only, not $0)
for(qw< 1 2 3 4 5 6 7 8 9 324897 237 635 6780 42 14 >) {
 ok eval { ${"foo::$_"} = 'prile' }, "\$foo::$_";
}

use tests 5; # read-only single-char scalars
for(qw< & ` ' + ] >) {
 ok eval { ${"foo::$_"} = 'twor'}, "\$foo::$_";
}

use tests 14; # rw single-char scalars we can safely modify
{
 # $. doesn’t appear magical from Perl-space until a filehandle has been
 # read, so we’ll do that right now.
 open my $fh, "<", \"freen";
 <$fh>;

 for(qw< : ? ! - | ^ ~ = % . \ / ; 0 >) {
  local $$_ = 'thew';
  ${"foo::$_"}; # touch it
  ok !defined ${"foo::$_"}, "\$foo::$_";
 }
}

use tests 1; # %!
ok scalar keys %{"foo::!"} == 0, '%foo::!';

use tests 4; # [@%][+-]
ok eval { ${"foo::+"}{strat} = 'quin' }, '%foo::+';
ok eval { ${"foo::-"}{strat} = 'quin' }, '%foo::-';
ok eval { ${"foo::+"}[47]    = 'quin' }, '@foo::+';
ok eval { ${"foo::-"}[63]    = 'quin' }, '@foo::-';

use tests 1; # $# - This naughty little thing just warns.
{
 my $w = '';
 local $SIG{__WARN__} = sub { $w = shift };
 eval '${"foo::#"}';
 is $w, '', '$foo::#';
}

use tests 11; # rw $^X scalars
for(qw<  C O I L   H A D   W E P T  >) {
 my $name = eval "qq|\\c$_|";
 local $$name = 'poof'; # we're setting, among other things, $^D, so all
                        # characters in here must be valid -D flags
 ${"foo::$name"}; # touch
 ok !defined ${"foo::$name"}, "\$foo::^$_";
}

use tests 1; # read-only $^X scalars
for(qw< S V >) {
 my $name = eval "qq|\\c$_|";
 ok eval { ${"foo::$name"} = 'twor'}, "\$foo::^$_";
}

use tests 4; # user/group vars
# These are rw, but setting them is obviously going to make the test much
# more complex than necessary. So, again, we check for definition.
for(qw<   < > ( )   >) {
 ${"foo::$_"}; # touch
 ok !defined ${"foo::$_"}, "\$foo::$_";
}

use tests 1; # $^N
# This is a cheeky little blighter. It’s not read-only, but setting it does
# nothing. It is undefined by default.
{
  my $thing;
 "felp" =~ /(.)(?{ $thing = ${"foo::\cN"} })/;
  ok !defined $thing, '$foo::^N';
}

# I think that’s it!
