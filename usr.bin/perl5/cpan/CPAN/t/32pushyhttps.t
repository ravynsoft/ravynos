use strict;
use warnings;
use Test::More;

unless ($ENV{AUTHOR_TEST}) {
    my $msg = 'running MinimumVersion test only run when AUTHOR_TEST set';
    plan( skip_all => $msg );
}

unshift @INC, './lib', './t';
require local_utils;
local_utils::cleanup_dot_cpan();
local_utils::prepare_dot_cpan();
local_utils::read_myconfig();
require CPAN::MyConfig;
require CPAN;
CPAN::HandleConfig->load;

for (qw(HTTP::Tiny Net::SSLeay IO::Socket::SSL)) {
    my $has_it = eval "require $_; 1";
    ok $has_it, "found $_" or plan( skip_all => "$_ not available" );
}

{
    package CPAN::Shell::tacet;
    my $output;
    sub init {
        $output = "";
    }
    sub myprint {
        shift;
        $output .= shift;
    }
    sub mydie {
        shift;
        die shift;
    }
    *mywarn = *mywarn = \&myprint;
    sub output {
        $output;
    }
}

sub collect_output {
    CPAN::Shell::tacet->init();
    my $command = shift @_;
    CPAN::Shell->$command(@_);
    return CPAN::Shell::tacet->output();
}

$CPAN::Frontend = $CPAN::Frontend = "CPAN::Shell::tacet";

require File::Which;
my %HAVE;
for (qw(curl wget)) {
    if (my $which = File::Which::which($_)) {
        pass $which;
        my $o = collect_output ( o => conf => $_ => $which );
        like $o, qr(Please use), "'Please use' on $_" or diag ">>>>$o<<<<";
        $HAVE{$_} = $which;
    } else {
        plan( skip_all => "$_ not found" );
    }
}

like collect_output ( o => conf => pushy_https => 1 ), qr(Please use), "'Please use' on pushy_https";
like collect_output ( o => conf => tar_verbosity => "none" ), qr(Please use), "'Please use' on tar_verbosity";

# | HTTP::Tiny | Net::SSLeay+IO::Socket::SSL | curl | wget | proto |
# |------------+-----------------------------+------+------+-------|
# |          1 |                           1 |    1 |    1 | https | (1)
# |          1 |                           1 |    0 |    0 | https | (2)
# |          1 |                           0 |    0 |    0 | http  | (3)
# |          0 |                           0 |    0 |    0 | -     | (4)
# |          0 |                           0 |    1 |    0 | https | (5)
# |          0 |                           0 |    0 |    1 | https | (6)

my $getmodule = "CPAN::Test::Dummy::Perl5::Make";
my $getdistro_qr = qr(CPAN-Test-Dummy-Perl5-Make);

# 1
like collect_output ( force => get => $getmodule ),
    qr((?s)Fetching with HTTP::Tiny.*https://.+/$getdistro_qr), "saw (1) https with all parties ON";
ok unlink(CPAN::Shell->expand("Module", $getmodule)->distribution->{localfile}), "unlink tarball";

# 2
like collect_output ( o => conf => curl => "" ), qr(Please use), "'Please use' on curl OFF";
like collect_output ( o => conf => wget => "" ), qr(Please use), "'Please use' on wget OFF";
like collect_output ( force => get => $getmodule ),
    qr((?s)Fetching with HTTP::Tiny.*https://.+/$getdistro_qr), "saw (2) https with HTTP::Tiny+SSL ON";
ok unlink(CPAN::Shell->expand("Module", $getmodule)->distribution->{localfile}), "unlink tarball";

# 3
ok delete $CPAN::HAS_USABLE->{"Net::SSLeay"}, "delete Net::SSLeay from %HAS_USABLE";
ok delete $INC{"Net/SSLeay.pm"}, "delete Net::SSLeay from %INC";
like collect_output ( o => conf => dontload_list => "Net::SSLeay" ),
    qr(Please use), "'Please use' on Net::SSLeay OFF";
like collect_output ( force => get => $getmodule ),
    qr((?si)fall back to http.*fetching with HTTP::Tiny.*http://.+/$getdistro_qr),
    "saw (3) http:// with HTTP::Tiny without SSL";
ok unlink(CPAN::Shell->expand("Module", $getmodule)->distribution->{localfile}), "unlink tarball";

# 4
ok delete $CPAN::HAS_USABLE->{"HTTP::Tiny"}, "delete HTTP::Tiny from %HAS_USABLE";
ok delete $INC{"HTTP/Tiny.pm"}, "delete HTTP::Tiny from %INC";
like collect_output ( o => conf => dontload_list => push => "HTTP::Tiny" ),
    qr(Please use), "'Please use' on HTTP::Tiny OFF";
eval { collect_output ( force => get => $getmodule ) };
my $output = CPAN::Shell::tacet->output;
like $@, qr(Giving up), "saw error 'Giving up'";
like $output, qr(Missing or unusable module), "saw (4) 'unusable module'";

# 5
like collect_output ( o => conf => curl => $HAVE{curl} ), qr(Please use), "'Please use' on curl ON";
ok $CPAN::Config->{curl}, "set curl to $CPAN::Config->{curl}";
like collect_output ( force => get => $getmodule ),
    qr((?s)Trying with.*curl.*to get.*https://.+/$getdistro_qr), "saw (5) https with curl ON";
ok unlink(CPAN::Shell->expand("Module", $getmodule)->distribution->{localfile}), "unlink tarball";

# 6
like collect_output ( o => conf => curl => "" ), qr(Please use), "'Please use' on curl OFF";
ok !$CPAN::Config->{curl}, "set curl to '$CPAN::Config->{curl}'";
like collect_output ( o => conf => wget => $HAVE{wget} ), qr(Please use), "'Please use' on wget ON";
ok $CPAN::Config->{wget}, "set wget to $CPAN::Config->{wget}";
like collect_output ( force => get => $getmodule ),
    qr((?s)Trying with.*wget.*to get.*https://.+/$getdistro_qr), "saw (6) https with wget ON";
ok unlink(CPAN::Shell->expand("Module", $getmodule)->distribution->{localfile}), "unlink tarball";

done_testing;
