use strict;
use Test::More tests => 67;
BEGIN {use_ok('I18N::LangTags', ':ALL');}

note("Perl v$], I18N::LangTags v$I18N::LangTags::VERSION");

foreach (['', 0],
	 ['fr', 1],
	 ['fr-ca', 1],
	 ['fr-CA', 1],
	 ['fr-CA-', 0],
	 ['fr_CA', 0],
	 ['fr-ca-joal', 1],
	 ['frca', 0],
	 ['nav', 1, 'not actual tag'],
	 ['nav-shiprock', 1, 'not actual tag'],
	 ['nav-ceremonial', 0, 'subtag too long'],
	 ['x', 0],
	 ['i', 0],
	 ['i-borg', 1, 'fictitious tag'],
	 ['x-borg', 1],
	 ['x-borg-prot5123', 1],
	) {
    my ($tag, $expect, $note) = @$_;
    $note = $note ? " # $note" : '';
    is(is_language_tag($tag), $expect, "is_language_tag('$tag')$note");
}
foreach (
    [ [ 'x-borg-prot5123', 'i-BORG-Prot5123' ], 1],
    [ [ 'en', 'en-us' ], 0],
) {
    my ($tags, $expect, $note) = @$_;
    $note = $note ? " # $note" : '';
    is(same_language_tag(@{$tags}), $expect, "same_language_tag('@{$tags}')$note");
}

foreach (
    [ [ 'en-ca', 'fr-ca' ], 0 ],
    [ [ 'en-ca', 'en-us' ], 1 ],
    [ [ 'en-us-southern', 'en-us-western' ], 2 ],
    [ [ 'en-us-southern', 'en-us' ], 2 ],
) {
    my ($tags, $expect, $note) = @$_;
    $note = $note ? " # $note" : '';
    is(similarity_language_tag(@{$tags}), $expect, "similarity_language_tag('@{$tags}')$note");
}

ok ((grep $_ eq 'hi', panic_languages('kok')),
    "'hi' is a panic language for 'kok'");
ok ((grep $_ eq 'en', panic_languages('x-woozle-wuzzle')),
    "'en' is a panic language for 'x-woozle-wuzzle'");
ok ((! grep $_ eq 'mr', panic_languages('it')),
    "'mr' is not a panic language for 'it'");
ok ((grep $_ eq 'es', panic_languages('it')),
    "'es' is a panic language for 'it'");
ok ((grep $_ eq 'it', panic_languages('es')),
    "'it' is a panic language for 'es'");

note("Now the ::List tests...");
note("# Perl v$], I18N::LangTags::List v$I18N::LangTags::List::VERSION");

use I18N::LangTags::List;
foreach my $lt (qw(
 en
 en-us
 en-kr
 el
 elx
 i-mingo
 i-mingo-tom
 x-mingo-tom
 it
 it-it
 it-IT
 it-FR
 ak
 aka
 jv
 jw
 no
 no-nyn
 nn
 i-lux
 lb
 wa
 yi
 ji
 den-syllabic
 den-syllabic-western
 den-western
 den-latin
 cre-syllabic
 cre-syllabic-western
 cre-western
 cre-latin
 cr-syllabic
 cr-syllabic-western
 cr-western
 cr-latin
 az-latin
)) {
  my $name = I18N::LangTags::List::name($lt);
  isnt($name, undef, "I18N::LangTags::List::name('$lt')");
}

my $correct = 'Azerbaijani in Latin script';
is(I18N::LangTags::List::name('az-Latn'), $correct,
   "Properly recognize 'az-latin' (with script subcomponent): # #16500");
is(I18N::LangTags::List::name('az-latn'), $correct,
   "Properly recognize 'az-latin' (with script subcomponent): # #16500");
