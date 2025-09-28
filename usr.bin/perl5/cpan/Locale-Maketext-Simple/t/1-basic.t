use strict;
use Test;

BEGIN {
    plan tests => 9;
    $INC{'Locale/Maketext/Lexicon.pm'} = __FILE__;
    $Locale::Maketext::Lexicon::VERSION = 0;
}

use Locale::Maketext::Simple;
ok(Locale::Maketext::Simple->VERSION);
ok(loc("Just [_1] Perl [_2]", qw(another hacker)), "Just another Perl hacker");

{
    local $^W; # shuts up 'redefined' warnings
    Locale::Maketext::Simple->reload_loc;
    Locale::Maketext::Simple->import(Style => 'gettext');
}

ok(loc("Just %1 Perl %2", qw(another hacker)), "Just another Perl hacker");
ok(loc_lang('fr'));
ok(loc("Just %quant(%1,Perl hacker)", 1), "Just 1 Perl hacker");
ok(loc("Just %quant(%1,Perl hacker)", 2), "Just 2 Perl hackers");
ok(loc("Just %quant(%1,Mad skill,Mad skillz)", 3), "Just 3 Mad skillz");
ok(loc("Error %tense(%1,present)", 'uninstall'), "Error uninstalling");
ok(loc("Error %tense(uninstall,present)"), "Error uninstalling");
