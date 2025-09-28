#!./perl -w
BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
    skip_all_if_miniperl();
}

use Config;

my $perlio_log = "perlio$$.txt";

skip_all "DEBUGGING build required"
  unless $::Config{ccflags} =~ /(?<!\S)-DDEBUGGING(?!\S)/
         or $^O eq 'VMS' && $::Config{usedebugging_perl} eq 'Y';

plan tests => 9;

END {
    unlink $perlio_log;
}
{
    unlink $perlio_log;
    local $ENV{PERLIO_DEBUG} = $perlio_log;
    fresh_perl_is("print qq(hello\n)", "hello\n",
                  { stderr => 1 },
                  "No perlio debug file without -Di...");
    ok(!-e $perlio_log, "...no perlio.txt found");
    fresh_perl_like("print qq(hello\n)", qr/\nEXECUTING...\n{1,2}hello\n?/,
                  { stderr => 1, switches => [ "-Di" ] },
                  "Perlio debug file with both -Di and PERLIO_DEBUG...");
    ok(-e $perlio_log, "... perlio debugging file found with -Di and PERLIO_DEBUG");

    unlink $perlio_log;
    SKIP: {
        if (not $Config{taint_support}) {
            skip("Your perl was built without taint support", 2);
        }
        fresh_perl_like("print qq(hello\n)", qr/define raw/,
                      { stderr => 1, switches => [ "-TDi" ] },
                      "Perlio debug output to stderr with -TDi (with PERLIO_DEBUG)...");
        ok(!-e $perlio_log, "...no perlio debugging file found");
    }
}

{
    local $ENV{PERLIO_DEBUG};
    fresh_perl_like("print qq(hello)", qr/define raw/,
                    { stderr => 1, switches => [ '-Di' ] },
                   "-Di defaults to stderr");
    SKIP: {
        skip("Your perl was built without taint support", 1)
            unless $Config{taint_support};

        fresh_perl_like("print qq(hello)", qr/define raw/,
                    { stderr => 1, switches => [ '-TDi' ] },
                   "Perlio debug output to STDERR with -TDi (no PERLIO_DEBUG)");
    }
}
{
    # -DXv tests
    fresh_perl_like('{ my $n=1; *foo= sub () { $n }; }',
                    qr/To: CV=0x[a-f0-9]+ \(ANON\), OUTSIDE=0x0 \(null\)/,
                    { stderr => 1, switches => [ '-DXv' ] },
                    "-DXv does not assert when dumping anonymous constant sub");
}
