use ExtUtils::MakeMaker;
use ExtUtils::Constant 0.23 'WriteConstants';
use File::Spec;
use strict;
use warnings;

my $core = grep { $_ eq 'PERL_CORE=1' } @ARGV;

my $headerpath;
if ($core) {
    $headerpath = File::Spec->catdir(File::Spec->updir, File::Spec->updir);
} else {
	require Config;
    $headerpath = File::Spec->catdir($Config::Config{archlibexp}, "CORE");
}

my @names = ({ name => 'HEf_SVKEY', macro => 1, type => "IV" },
             qw(SVTYPEMASK SVt_PVGV SVt_PVHV PAD_FAKELEX_ANON
                PAD_FAKELEX_MULTI SVpad_STATE SVpad_TYPED SVpad_OUR));

my @depend;

# First element in each tuple is the file; second is a regex snippet
# giving the prefix to limit the names of symbols to define that come
# from that file.  If none, all symbols will be defined whose values
# match the pattern below.
foreach my $tuple (['cop.h'],
                   ['cv.h', 'CVf'],
                   ['gv.h', 'G[PV]f'],
                   ['op.h'],
                   ['opcode.h', 'OPp'],
                   ['op_reg_common.h','(?:(?:RXf_)?PMf_)'],
                   ['pad.h','PADNAMEf_'],
                   ['regexp.h','RXf_'],
                   ['sv.h', 'SV(?:[fps]|pad)_'],
                  ) {
    my $file = $tuple->[0];
    my $pfx = $tuple->[1] || '';
    my $path = File::Spec->catfile($headerpath, $file);
    push @depend, $path;
    open my $fh, '<', $path or die "Cannot open $path: $!";
    while (<$fh>) {
	push @names, $1 if (/ ^ \s* \# \s* define \s+ ( $pfx \w+ ) \s+
            ( [()|\dx]+ [UuLl]{0,2}            # Parens, '|', digits, 'x',
                                               # followed by optional long,
                                               # unsigned qualifiers
            | 0x[0-9a-fA-F]+                   # hex values
            | \(? \d+ [UuLl]{0,2} \s* << .*?   # digits left shifted by anything
                                               # followed by optional
                                               # long, unsigned qualifiers
            ) \s* (?: $| \/ \* )               # ending at comment or $
        /x);
    }
    close $fh;
}

WriteMakefile(
    NAME	    => "B",
    VERSION_FROM    => "B.pm",
    realclean	    => {FILES=> 'const-c.inc const-xs.inc'},
    depend          => {'Makefile' => "@depend"},
);

# Currently only SVt_PVGV and SVt_PVHV aren't macros, but everything we name
# should exist, so ensure that the C compile breaks if anything does not.
WriteConstants(
    PROXYSUBS => {push => 'EXPORT_OK'},
    NAME => 'B',
    NAMES => [map {ref $_ ? $_ : {name=>$_, macro=>1, type=>"UV"}} @names],
    XS_SUBNAME => undef,
);
