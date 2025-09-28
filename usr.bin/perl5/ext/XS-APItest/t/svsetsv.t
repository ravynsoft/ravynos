use strict;
use warnings;

use Test::More tests => 7;

BEGIN { use_ok('XS::APItest') };

# I can't see a good way to easily get back perl-space diagnostics for these
# I hope that this isn't a problem.
ok(sv_setsv_cow_hashkey_core,
   "With PERL_CORE sv_setsv does COW for shared hash key scalars");

ok(!sv_setsv_cow_hashkey_notcore,
   "Without PERL_CORE sv_setsv doesn't COW for shared hash key scalars");

*AUTOLOAD = \&XS::APItest::AutoLoader::AUTOLOADp;
foo(\1); sv_set_deref(\&AUTOLOAD, '$', 0);
is prototype(\&AUTOLOAD), '$', 'sv_setsv(cv,...) sets prototype';
foo(\1); sv_set_deref(\&AUTOLOAD, '$', 1);
is prototype(\&AUTOLOAD), '$', 'sv_setpv(cv,...) sets prototype';
foo(\1); sv_set_deref(\&AUTOLOAD, '$', 2);
is prototype(\&AUTOLOAD), '$', 'sv_setpvn(cv,...) sets prototype';

# Perhaps this does not belong here?  But it is at least testing that
# sv_mortalcopy uses sv_setsv in an unsurprising way.
ok !SvIsCOW(sv_mortalcopy(__PACKAGE__)),
  'sv_mortalcopy does not COW for extensions [perl #79824]';
