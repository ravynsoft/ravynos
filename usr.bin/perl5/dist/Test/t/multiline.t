#!./perl -w

BEGIN { open(STDERR, ">&STDOUT");  }

use strict;
use Test; plan tests => 2, todo => [1,2]; # actually false failure

# perl -Ilib -It/noinck t/multiline.t

ok(
q{
Jojo was a man who thought he was a loner
But he knew it couldn't last
Jojo left his home in Tucson, Arizona
For some California Grass
Get back, get back
Get back to where you once belonged
Get back, get back
Get back to where you once belonged
Get back Jojo Go home
Get back, get back
Back to where you once belonged
Get back, get back
Back to where you once belonged
Get back Jo
}
,
q{
Sweet Loretta Martin thought she was a woman
But she was another man
All the girls around her say she's got it coming
But she gets it while she can
Get back, get back
Get back to where you once belonged
Get back, get back
Get back to where you once belonged
Get back Loretta Go home
Get back, get back
Get back to where you once belonged
Get back, get back
Get back to where you once belonged
Get home Loretta
});

ok "zik\nzak\n  wazaaaaap\ncha ching!\n", "crunk\n\t zonk\nbjork\nchachacha!\n";


