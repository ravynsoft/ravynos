use Test::More tests => 11;

BEGIN {
    use_ok('Locale::Maketext');
};

package MyTestLocale;

@MyTestLocale::ISA = qw(Locale::Maketext);
%MyTestLocale::Lexicon = ();
%MyTestLocale::Lexicon = (); # to avoid warnings

package MyTestLocale::fr;

@MyTestLocale::fr::ISA = qw(MyTestLocale);

%MyTestLocale::fr::Lexicon = (
    '_AUTO' => 1,
    'Hello World' => 'Bonjour Monde',
);

package main;

my $lh = MyTestLocale->get_handle('fr');
$lh->{'use_external_lex_cache'} = 1;
ok(exists $MyTestLocale::fr::Lexicon{'Hello World'} && !ref $MyTestLocale::fr::Lexicon{'Hello World'}, 'lex value not a ref');

is($lh->maketext('Hello World'), 'Bonjour Monde', 'renders correctly first time');
ok(exists $lh->{'_external_lex_cache'}{'Hello World'} && ref $lh->{'_external_lex_cache'}{'Hello World'}, 'compiled into lex_cache');
ok(exists $MyTestLocale::fr::Lexicon{'Hello World'} && !ref $MyTestLocale::fr::Lexicon{'Hello World'}, 'lex value still not a ref');

is($lh->maketext('Hello World'), 'Bonjour Monde', 'renders correctly second time time');
ok(exists $lh->{'_external_lex_cache'}{'Hello World'} && ref $lh->{'_external_lex_cache'}{'Hello World'}, 'still compiled into lex_cache');
ok(exists $MyTestLocale::fr::Lexicon{'Hello World'} && !ref $MyTestLocale::fr::Lexicon{'Hello World'}, 'lex value still not a ref');

is($lh->maketext('This is not a key'), 'This is not a key', '_AUTO renders correctly first time');
ok(exists $lh->{'_external_lex_cache'}{'This is not a key'} && ref $lh->{'_external_lex_cache'}{'This is not a key'}, '_AUTO compiled into lex_cache');
ok(!exists $MyTestLocale::fr::Lexicon{'This is not a key'}, '_AUTO lex value not added to lex');
