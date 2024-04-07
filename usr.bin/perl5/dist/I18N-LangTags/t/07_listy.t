use strict;
use Test::More tests => 16;
BEGIN {use_ok('I18N::LangTags::List');}

note("Perl v$], I18N::LangTags::List v$I18N::LangTags::List::VERSION");

is(I18N::LangTags::List::name('fr'), 'French');
isnt(I18N::LangTags::List::name('fr-fr'), undef);
is(I18N::LangTags::List::name('El Zorcho'), undef);
is(I18N::LangTags::List::name(), undef);

isnt(I18N::LangTags::List::is_decent(), undef);
foreach(['fr', 2],
	['fr-blorch', 2],
	['El Zorcho', 0],
	['sgn', 0],
	['sgn-us', 2],
	['i', 0],
	['i-mingo', 2],
	['i-mingo-tom', 2],
	['cel', 0],
	['cel-gaulish', 2],
       ) {
    my ($tag, $expect) = @$_;
    is(I18N::LangTags::List::is_decent($tag), $expect,
       "I18N::LangTags::List::is_decent('$tag')");
}
