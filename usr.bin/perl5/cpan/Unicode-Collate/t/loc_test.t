
BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir('t') if -d 't';
	@INC = $^O eq 'MacOS' ? qw(::lib) : qw(../lib);
    }
}

use strict;
use warnings;
BEGIN { $| = 1; print "1..134\n"; }
my $count = 0;
sub ok ($;$) {
    my $p = my $r = shift;
    if (@_) {
	my $x = shift;
	$p = !defined $x ? !defined $r : !defined $r ? 0 : $r eq $x;
    }
    print $p ? "ok" : "not ok", ' ', ++$count, "\n";
}

use Unicode::Collate::Locale;

ok(1);

sub _pack_U   { Unicode::Collate::pack_U(@_) }
sub _unpack_U { Unicode::Collate::unpack_U(@_) }

#########################

our (@listEs, @listEsT, @listFr);

@listEs = qw(
    cambio camella camello camelo Camerún
    chico chile Chile CHILE chocolate
    cielo curso espacio espanto español esperanza lama líquido
    llama Llama LLAMA llamar luz nos nueve ñu ojo
);

@listEsT = qw(
    cambio camelo camella camello Camerún cielo curso
    chico chile Chile CHILE chocolate
    espacio espanto español esperanza lama líquido luz
    llama Llama LLAMA llamar nos nueve ñu ojo
);

@listFr = (
  qw(
    cadurcien cæcum cÆCUM CæCUM CÆCUM caennais cæsium cafard
    coercitif cote côte Côte coté Coté côté Côté coter
    élève élevé gène gêne MÂCON maçon
    pèche PÈCHE pêche PÊCHE péché PÉCHÉ pécher pêcher
    relève relevé révèle révélé
    surélévation sûrement suréminent sûreté
    vice-consul vicennal vice-président vice-roi vicésimal),
  "vice versa", "vice-versa",
);

ok(@listEs,  27);
ok(@listEsT, 27);
ok(@listFr,  46);

ok(Unicode::Collate::Locale::_locale('es_MX'), 'es');
ok(Unicode::Collate::Locale::_locale('en_CA'), 'default');

# 6

my $Collator = Unicode::Collate::Locale->
    new(normalization => undef);
ok($Collator->getlocale, 'default');

ok(
  join(':', $Collator->sort(
    qw/ lib strict Carp ExtUtils CGI Time warnings Math overload Pod CPAN /
  ) ),
  join(':',
    qw/ Carp CGI CPAN ExtUtils lib Math overload Pod strict Time warnings /
  ),
);

ok($Collator->cmp("", ""), 0);
ok($Collator->eq("", ""));
ok($Collator->cmp("", "perl"), -1);
ok($Collator->gt("PERL", "perl"));

# 12

$Collator->change(level => 2);

ok($Collator->eq("PERL", "perl"));

my $objEs  = Unicode::Collate::Locale->new
    (normalization => undef, locale => 'ES');
ok($objEs->getlocale, 'es');

my $objEsT = Unicode::Collate::Locale->new
    (normalization => undef, locale => 'es_ES_traditional');
ok($objEsT->getlocale, 'es__traditional');

my $objFr  = Unicode::Collate::Locale->new
    (normalization => undef, locale => 'FR_CA');
ok($objFr->getlocale, 'fr_CA');

# 16

sub randomize { my %hash; @hash{@_} = (); keys %hash; } # ?!

for (my $i = 0; $i < $#listEs; $i++) {
    ok($objEs->lt($listEs[$i], $listEs[$i+1]));
}
# 42

for (my $i = 0; $i < $#listEsT; $i++) {
    ok($objEsT->lt($listEsT[$i], $listEsT[$i+1]));
}
# 68

for (my $i = 0; $i < $#listFr; $i++) {
    ok($objFr->lt($listFr[$i], $listFr[$i+1]));
}
# 113

our @randEs = randomize(@listEs);
our @sortEs = $objEs->sort(@randEs);
ok("@sortEs" eq "@listEs");

our @randEsT = randomize(@listEsT);
our @sortEsT = $objEsT->sort(@randEsT);
ok("@sortEsT" eq "@listEsT");

our @randFr = randomize(@listFr);
our @sortFr = $objFr->sort(@randFr);
ok("@sortFr" eq "@listFr");

# 116

{
    my $keyXS = '__useXS'; # see Unicode::Collate internal
    my $noLoc = Unicode::Collate->new(normalization => undef);
    my $UseXS = ref($noLoc->{$keyXS});
    ok(ref($Collator->{$keyXS}), $UseXS);
    ok(ref($objFr   ->{$keyXS}), $UseXS);
    ok(ref($objEs   ->{$keyXS}), $UseXS);
    ok(ref($objEsT  ->{$keyXS}), $UseXS);
}
# 120

ok(Unicode::Collate::Locale::_locale('sr'),            'sr');
ok(Unicode::Collate::Locale::_locale('sr_Cyrl'),       'sr');
ok(Unicode::Collate::Locale::_locale('sr_Latn'),       'sr_Latn');
ok(Unicode::Collate::Locale::_locale('sr_LATN'),       'sr_Latn');
ok(Unicode::Collate::Locale::_locale('sr_latn'),       'sr_Latn');
ok(Unicode::Collate::Locale::_locale('de'),            'default');
ok(Unicode::Collate::Locale::_locale('de_phone'),      'de__phonebook');
ok(Unicode::Collate::Locale::_locale('de__phonebook'), 'de__phonebook');
ok(Unicode::Collate::Locale::_locale('de-phonebk'),    'de__phonebook');
ok(Unicode::Collate::Locale::_locale('de--phonebk'),   'de__phonebook');

# 130

my $objEs2  = Unicode::Collate::Locale->new
    (normalization => undef, locale => 'ES',
     level => 1,
     entry => << 'ENTRIES',
0000      ; [.FFFE.0020.0005.0000]
00F1      ; [.0010.0020.0002.00F1] # LATIN SMALL LETTER N WITH TILDE
006E 0303 ; [.0010.0020.0002.00F1] # LATIN SMALL LETTER N WITH TILDE
ENTRIES
);

ok($objEs2->lt("abc\x{4E00}", "abc\0"));
ok($objEs2->lt("abc\x{FFFD}", "abc\0"));
ok($objEs2->lt("abc\x{FFFD}", "abc\0"));
ok($objEs2->lt("n\x{303}", "N\x{303}"));

# 134

