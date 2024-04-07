#!/usr/bin/perl -Tw

use strict;
use Test::More tests => 6;

BEGIN {
    use_ok( 'Locale::Maketext' );
}

use utf8;

{
    package My::Localize;
    our @ISA = ('Locale::Maketext');
}
{
    package My::Localize::cs_cz;
    our @ISA = ('My::Localize');
    our %Lexicon = (
        '[_1]foo1\n' => '[_1]bar\n',
        '[_1]foo2\n' => '[_1]běr\n',
        'foo2\n' => 'aěa\n',
        "[_1]foo\\n\n" => "[_1]bar\\n\n",
    );
    keys %Lexicon; # dodges the 'used only once' warning
}

my $lh = My::Localize->get_handle('cs_cz');
isa_ok( $lh, 'My::Localize::cs_cz' );
is( $lh->maketext('[_1]foo1\n', 'arg'), 'argbar\n', 'Safe parameterized' );
is( $lh->maketext('[_1]foo2\n', 'arg'), 'argběr\n', 'Unicode parameterized' );
is( $lh->maketext('foo2\n'), 'aěa\n', 'Unicode literal' );
is( $lh->maketext("[_1]foo\\n\n", 'arg'), "argbar\\n\n", 'new line parameterized' );
