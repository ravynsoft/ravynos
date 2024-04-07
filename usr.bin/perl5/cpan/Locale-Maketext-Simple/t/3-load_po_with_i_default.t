use strict;
use Test::More;
use FindBin qw($Bin);

use Locale::Maketext::Simple (
	Path => "$Bin/po_with_i_default",
	Style => "gettext",
);

eval {
    require Locale::Maketext::Lexicon;
    die unless Locale::Maketext::Lexicon->VERSION(0.20);
    require File::Spec;
};
if ($@) {
    plan skip_all => 'No soft dependencies, i_default will not work';
    exit 0;
}

plan tests => 6;

loc_lang("en");
is(loc("Not a lexicon key"), "Not a lexicon key", "Not a lexicon key");
is(loc("I have got %1 alerts", "hot"), "Maybe it's because I'm a hot Londoner, that I love London tawn", "Got auto key" );
is(loc("I have acknowledged %1 alerts", 83), "Yo dude, I've been working on these 83 alerts", "Got translation still in en");
is(loc("system.messages.arbitrary.unique.lexicon.key"), "This is the default message", "Used i_default because not set in en");

loc_lang("fr");
is(loc("system.messages.arbitrary.unique.lexicon.key"), "This is the default message", "Translated from i_default");
is(loc("I have got %1 alerts", "red"), "Mon Francais red, c'est terrible", "French translated" );

