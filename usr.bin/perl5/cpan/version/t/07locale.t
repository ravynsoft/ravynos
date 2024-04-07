#! /usr/local/perl -w
# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

#########################

use File::Basename;
use File::Temp qw/tempfile/;
use POSIX qw/locale_h/;
use Test::More tests => 8;
use Config;

BEGIN {
    use_ok('version', 0.9929);
}

SKIP: {
	skip 'No locale testing for Perl < 5.6.0', 7 if $] < 5.006;
	skip 'No locale testing without d_setlocale', 7
	    if(!$Config{d_setlocale});
	skip 'No locale testing without LC_NUMERIC', 7
	    if($Config{ccflags}) =~ /-DNO_LOCALE_NUMERIC\b/;

	# test locale handling
	my $warning = '';

	local $SIG{__WARN__} = sub { $warning = $_[0] };

	my $ver = 1.23;  # has to be floating point number
	my $loc;
	my $orig_loc = setlocale(LC_NUMERIC);
	ok ($ver eq "1.23", 'Not using locale yet');  # Don't use is(),
						      # because have to
						      # evaluate in current
						      # scope
	use if $^O !~ /android/, 'locale';

	while (<DATA>) {
	    chomp;
	    $loc = setlocale( LC_ALL, $_);
	    last if $loc && localeconv()->{decimal_point} eq ',';
	}
	skip 'Cannot test locale handling without a comma locale', 6
	    unless $loc and localeconv()->{decimal_point} eq ',';

	setlocale(LC_NUMERIC, $loc);
	$ver = 1.23;  # has to be floating point number
	ok ($ver eq "1,23", "Using locale: $loc");
	$v = 'version'->new($ver);
	unlike($warning, qr/Version string '1,23' contains invalid data/,
	    "Process locale-dependent floating point");
	ok ($v eq "1.23", "Locale doesn't apply to version objects");
	ok ($v == $ver, "Comparison to locale floating point");

        TODO: { # Resolve https://rt.cpan.org/Ticket/Display.html?id=102272
            local $TODO = 'Fails for Perl 5.x.0 < 5.19.0' if $] < 5.019000;
            $ver = 'version'->new($]);
            is "$ver", "$]", 'Use PV for dualvars';
        }
	setlocale( LC_ALL, $orig_loc); # reset this before possible skip
	skip 'Cannot test RT#46921 with Perl < 5.008', 1
	    if ($] < 5.008);
	my ($fh, $filename) = tempfile('tXXXXXXX', SUFFIX => '.pm', UNLINK => 1);
	(my $package = basename($filename)) =~ s/\.pm$//;
	print $fh <<"EOF";
package $package;
use locale;
use POSIX qw(locale_h);
\$^W = 1;
use version;
setlocale (LC_ALL, '$loc');
use version ;
eval "use Socket 1.7";
setlocale( LC_ALL, '$orig_loc');
1;
EOF
	close $fh;

	eval "use lib '.'; use $package;";
	unlike($warning, qr"Version string '1,7' contains invalid data",
	    'Handle locale action-at-a-distance');
}

__DATA__
af_ZA
af_ZA.utf8
af_ZA.UTF-8
an_ES
an_ES.utf8
an_ES.UTF-8
az_AZ.utf8
az_AZ.UTF-8
be_BY
be_BY.utf8
be_BY.UTF-8
bg_BG
bg_BG.utf8
bg_BG.UTF-8
br_FR
br_FR@euro
br_FR.utf8
br_FR.UTF-8
bs_BA
bs_BA.utf8
bs_BA.UTF-8
ca_ES
ca_ES@euro
ca_ES.utf8
ca_ES.UTF-8
cs_CZ
cs_CZ.utf8
cs_CZ.UTF-8
da_DK
da_DK.utf8
da_DK.UTF-8
de_AT
de_AT@euro
de_AT.utf8
de_AT.UTF-8
de_BE
de_BE@euro
de_BE.utf8
de_BE.UTF-8
de_DE
de_DE@euro
de_DE.utf8
de_DE.UTF-8
de_DE.UTF-8
de_LU
de_LU@euro
de_LU.utf8
de_LU.UTF-8
el_GR
el_GR.utf8
el_GR.UTF-8
en_DK
en_DK.utf8
en_DK.UTF-8
es_AR
es_AR.utf8
es_AR.UTF-8
es_BO
es_BO.utf8
es_BO.UTF-8
es_CL
es_CL.utf8
es_CL.UTF-8
es_CO
es_CO.utf8
es_CO.UTF-8
es_EC
es_EC.utf8
es_EC.UTF-8
es_ES
es_ES@euro
es_ES.utf8
es_ES.UTF-8
es_PY
es_PY.utf8
es_PY.UTF-8
es_UY
es_UY.utf8
es_UY.UTF-8
es_VE
es_VE.utf8
es_VE.UTF-8
et_EE
et_EE.iso885915
et_EE.utf8
et_EE.UTF-8
eu_ES
eu_ES@euro
eu_ES.utf8
eu_ES.UTF-8
fi_FI
fi_FI@euro
fi_FI.utf8
fi_FI.UTF-8
fo_FO
fo_FO.utf8
fo_FO.UTF-8
fr_BE
fr_BE@euro
fr_BE.utf8
fr_BE.UTF-8
fr_CA
fr_CA.utf8
fr_CA.UTF-8
fr_CH
fr_CH.utf8
fr_CH.UTF-8
fr_FR
fr_FR@euro
fr_FR.utf8
fr_FR.UTF-8
fr_LU
fr_LU@euro
fr_LU.utf8
fr_LU.UTF-8
gl_ES
gl_ES@euro
gl_ES.utf8
gl_ES.UTF-8
hr_HR
hr_HR.utf8
hr_HR.UTF-8
hu_HU
hu_HU.utf8
hu_HU.UTF-8
id_ID
id_ID.utf8
id_ID.UTF-8
is_IS
is_IS.utf8
is_IS.UTF-8
it_CH
it_CH.utf8
it_CH.UTF-8
it_IT
it_IT@euro
it_IT.utf8
it_IT.UTF-8
ka_GE
ka_GE.utf8
ka_GE.UTF-8
kk_KZ
kk_KZ.utf8
kk_KZ.UTF-8
kl_GL
kl_GL.utf8
kl_GL.UTF-8
lt_LT
lt_LT.utf8
lt_LT.UTF-8
lv_LV
lv_LV.utf8
lv_LV.UTF-8
mk_MK
mk_MK.utf8
mk_MK.UTF-8
mn_MN
mn_MN.utf8
mn_MN.UTF-8
nb_NO
nb_NO.utf8
nb_NO.UTF-8
nl_BE
nl_BE@euro
nl_BE.utf8
nl_BE.UTF-8
nl_NL
nl_NL@euro
nl_NL.utf8
nl_NL.UTF-8
nn_NO
nn_NO.utf8
nn_NO.UTF-8
no_NO
no_NO.utf8
no_NO.UTF-8
oc_FR
oc_FR.utf8
oc_FR.UTF-8
pl_PL
pl_PL.utf8
pl_PL.UTF-8
pt_BR
pt_BR.utf8
pt_BR.UTF-8
pt_PT
pt_PT@euro
pt_PT.utf8
pt_PT.UTF-8
ro_RO
ro_RO.utf8
ro_RO.UTF-8
ru_RU
ru_RU.koi8r
ru_RU.utf8
ru_RU.UTF-8
ru_UA
ru_UA.utf8
ru_UA.UTF-8
se_NO
se_NO.utf8
se_NO.UTF-8
sh_YU
sh_YU.utf8
sh_YU.UTF-8
sk_SK
sk_SK.utf8
sk_SK.UTF-8
sl_SI
sl_SI.utf8
sl_SI.UTF-8
sq_AL
sq_AL.utf8
sq_AL.UTF-8
sr_CS
sr_CS.utf8
sr_CS.UTF-8
sv_FI
sv_FI@euro
sv_FI.utf8
sv_FI.UTF-8
sv_SE
sv_SE.iso885915
sv_SE.utf8
sv_SE.UTF-8
tg_TJ
tg_TJ.utf8
tg_TJ.UTF-8
tr_TR
tr_TR.utf8
tr_TR.UTF-8
tt_RU.utf8
tt_RU.UTF-8
uk_UA
uk_UA.utf8
uk_UA.UTF-8
vi_VN
vi_VN.tcvn
wa_BE
wa_BE@euro
wa_BE.utf8
wa_BE.UTF-8
