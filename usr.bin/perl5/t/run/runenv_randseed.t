#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
    require Config;
    Config->import;
}

skip_all_without_config('d_fork');
skip_all("This perl is built with NO_PERL_RAND_SEED")
    if $Config{ccflags} =~ /-DNO_PERL_RAND_SEED\b/;
use strict;
use warnings;

for (1..2) {
    local $ENV{PERL_RAND_SEED} = 1;
    fresh_perl_is("print map { chr(rand(26)+65) } 1..10",
                  "BLVIOAEZTJ", undef, "Test randomness with PERL_RAND_SEED=1");
}

for (1..2) {
    local $ENV{PERL_RAND_SEED} = 2;
    fresh_perl_is("print map { chr(rand(26)+65) } 1..10",
                  "XEOUOFRPQZ", undef, "Test randomness with PERL_RAND_SEED=2");
}

my %got;
for my $try (1..10) {
    local $ENV{PERL_RAND_SEED};
    my ($out,$err)= runperl_and_capture({}, ['-e',"print map { chr(rand(26)+65) } 1..10;"]);
    if ($err) { diag $err }
    $got{$out}++;
}
ok(8 <= keys %got, "Got at least 8 different strings");
for (1..2) {
    local $ENV{PERL_RAND_SEED} = 1;
    my ($out,$err)= runperl_and_capture({}, ['-le',
            <<'EOF_TEST_CODE'
            for my $l ("A".."E") {
                my $pid= fork;
                if ($pid) {
                    push @pids, $pid;
                }
                elsif (!defined $pid) {
                    print "$l:failed fork";
                } elsif (!$pid) {
                    print "$l:", map { chr(rand(26)+65) } 1..10;
                    exit;
                }
            }
            waitpid $_,0 for @pids;
EOF_TEST_CODE
        ]);
    is($err, "", "No exceptions forking.");
    my @parts= sort { $a cmp $b } split /\n/, $out;
    my @want= (
            "A:KNXDITWWJZ",
            "B:WDQJGTBJQS",
            "C:ZGYCCINIHE",
            "D:UGLGAEXFBP",
            "E:MQLTNZGZQB"
    );
    is("@parts","@want","Works as expected with forks.");
}

done_testing();
