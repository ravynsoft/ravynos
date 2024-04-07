#!perl -w

BEGIN {
    unshift @INC, 't/compat' if $] < 5.006002;
};

use strict;
use Test::More;
use Storable qw(store nstore);
use Config qw(%Config);

# The @tests array below was create by the following program
my $dummy = <<'EOT';
use Storable;
use Data::Dump qw(dump);

print "my \@tests = (\n";
for my $f (<data_*>) {
    print "    [\n";
    print "        " . dump(substr(`cat $f`, 0, 32) . "...") , ",\n";

    my $x = dump(Storable::file_magic($f));
    $x =~ s/^/        /gm;
    print "$x,\n";

    print "    ],\n";
}
print ");\n";
EOT

my @tests = (
    [
        "perl-store\x041234\4\4\4\xD4\xC2\32\b\3\13\0\0\0v\b\xC5\32\b...",
        {
          byteorder  => 1234,
          file       => "data_perl-5.006001_i686-linux-thread-multi_Storable-0.1.le32",
          hdrsize    => 18,
          intsize    => 4,
          longsize   => 4,
          netorder   => 0,
          ptrsize    => 4,
          version    => -1,
          version_nv => -1,
        },
    ],
    [
        "perl-store\0\x041234\4\4\4\x8Co\34\b\3\13\0\0\0v\x94v\34...",
        {
          byteorder  => 1234,
          file       => "data_perl-5.006001_i686-linux-thread-multi_Storable-0.4_07.le32",
          hdrsize    => 19,
          intsize    => 4,
          longsize   => 4,
          major      => 0,
          netorder   => 0,
          ptrsize    => 4,
          version    => 0,
          version_nv => 0,
        },
    ],
    [
        "perl-store\1\x8Co\34\b\3\0\0\0\13v\x94v\34\b\1\0\0\4\0\0\0...",
        {
          file       => "data_perl-5.006001_i686-linux-thread-multi_Storable-0.4_07.neutral",
          hdrsize    => 11,
          major      => 0,
          netorder   => 1,
          version    => 0,
          version_nv => 0,
        },
    ],
    [
        "pst0\2\x041234\4\4\4\3\13\0\0\0\1\0\4\0\0\0\0\0\0\0\0\0\0\0...",
        {
          byteorder  => 1234,
          file       => "data_perl-5.006001_i686-linux-thread-multi_Storable-0.604.le32",
          hdrsize    => 13,
          intsize    => 4,
          longsize   => 4,
          major      => 1,
          netorder   => 0,
          ptrsize    => 4,
          version    => 1,
          version_nv => 1,
        },
    ],
    [
        "pst0\3\3\0\0\0\13\1\0\0\4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0...",
        {
          file       => "data_perl-5.006001_i686-linux-thread-multi_Storable-0.604.neutral",
          hdrsize    => 5,
          major      => 1,
          netorder   => 1,
          version    => 1,
          version_nv => 1,
        },
    ],
    [
        "pst0\4\0\x041234\4\4\4\3\13\0\0\0\1\0\4\0\0\0\0\0\0\0\0\0\0...",
        {
          byteorder  => 1234,
          file       => "data_perl-5.006001_i686-linux-thread-multi_Storable-0.700.le32",
          hdrsize    => 14,
          intsize    => 4,
          longsize   => 4,
          major      => 2,
          minor      => 0,
          netorder   => 0,
          ptrsize    => 4,
          version    => "2.0",
          version_nv => "2.000",
        },
    ],
    [
        "pst0\5\0\3\0\0\0\13\1\0\0\4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0...",
        {
          file       => "data_perl-5.006001_i686-linux-thread-multi_Storable-0.700.neutral",
          hdrsize    => 6,
          major      => 2,
          minor      => 0,
          netorder   => 1,
          version    => "2.0",
          version_nv => "2.000",
        },
    ],
    [
        "pst0\4\4\x041234\4\4\4\x08\3\13\0\0\0\1\0\4\0\0\0\0\0\0\0\0\0...",
        {
          byteorder  => 1234,
          file       => "data_perl-5.006001_i686-linux-thread-multi_Storable-1.012.le32",
          hdrsize    => 15,
          intsize    => 4,
          longsize   => 4,
          major      => 2,
          minor      => 4,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 4,
          version    => "2.4",
          version_nv => "2.004",
        },
    ],
    [
        "pst0\4\3\x044321\4\4\4\x08\3\0\0\0\13\1\0\0\4\0\0\0\0\0\0\0\0...",
        {
          byteorder  => 4321,
          file       => "data_perl-5.006001_IA64.ARCHREV_0-thread-multi_Storable-1.006.be32",
          hdrsize    => 15,
          intsize    => 4,
          longsize   => 4,
          major      => 2,
          minor      => 3,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 4,
          version    => "2.3",
          version_nv => "2.003",
        },
    ],
    [
        "pst0\5\3\3\0\0\0\13\1\0\0\4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0...",
        {
          file       => "data_perl-5.006001_IA64.ARCHREV_0-thread-multi_Storable-1.006.neutral",
          hdrsize    => 6,
          major      => 2,
          minor      => 3,
          netorder   => 1,
          version    => "2.3",
          version_nv => "2.003",
        },
    ],
    [
        "pst0\4\4\x044321\4\4\4\x08\3\0\0\0\13\1\0\0\4\0\0\0\0\0\0\0\0...",
        {
          byteorder  => 4321,
          file       => "data_perl-5.006001_IA64.ARCHREV_0-thread-multi_Storable-1.012.be32",
          hdrsize    => 15,
          intsize    => 4,
          longsize   => 4,
          major      => 2,
          minor      => 4,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 4,
          version    => "2.4",
          version_nv => "2.004",
        },
    ],
    [
        "pst0\5\4\3\0\0\0\13\1\0\0\4\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0...",
        {
          file       => "data_perl-5.006001_IA64.ARCHREV_0-thread-multi_Storable-1.012.neutral",
          hdrsize    => 6,
          major      => 2,
          minor      => 4,
          netorder   => 1,
          version    => "2.4",
          version_nv => "2.004",
        },
    ],
    [
        "pst0\4\6\x044321\4\4\4\x08\3\0\0\0\13\n\n4294967296...",
        {
          byteorder  => 4321,
          file       => "data_perl-5.008001_darwin-thread-multi-2level_Storable-2.08.be32",
          hdrsize    => 15,
          intsize    => 4,
          longsize   => 4,
          major      => 2,
          minor      => 6,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 4,
          version    => "2.6",
          version_nv => "2.006",
        },
    ],
    [
        "pst0\5\6\3\0\0\0\13\n\n4294967296\0\0\0\bfour_...",
        {
          file       => "data_perl-5.008001_darwin-thread-multi-2level_Storable-2.08.neutral",
          hdrsize    => 6,
          major      => 2,
          minor      => 6,
          netorder   => 1,
          version    => "2.6",
          version_nv => "2.006",
        },
    ],
    [
        "pst0\4\6\x044321\4\4\4\x08\3\0\0\0\13\4\3\0\0\0\0\0\0\0\nem...",
        {
          byteorder  => 4321,
          file       => "data_perl-5.008003_PA-RISC1.1-thread-multi_Storable-2.09.be32",
          hdrsize    => 15,
          intsize    => 4,
          longsize   => 4,
          major      => 2,
          minor      => 6,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 4,
          version    => "2.6",
          version_nv => "2.006",
        },
    ],
    [
        "pst0\5\6\3\0\0\0\13\4\3\0\0\0\0\0\0\0\nempty_hash\n...",
        {
          file       => "data_perl-5.008003_PA-RISC1.1-thread-multi_Storable-2.09.neutral",
          hdrsize    => 6,
          major      => 2,
          minor      => 6,
          netorder   => 1,
          version    => "2.6",
          version_nv => "2.006",
        },
    ],
    [
        "pst0\4\6\x0812345678\4\4\4\x08\3\13\0\0\0\4\3\0\0\0\0\n\0...",
        {
          byteorder  => 12_345_678,
          file       => "data_perl-5.008004_i86pc-solaris-64int_Storable-2.12.le64",
          hdrsize    => 19,
          intsize    => 4,
          longsize   => 4,
          major      => 2,
          minor      => 6,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 4,
          version    => "2.6",
          version_nv => "2.006",
        },
    ],
    [
        "pst0\4\6\x041234\4\4\4\x08\3\13\0\0\0\4\3\0\0\0\0\n\0\0\0em...",
        {
          byteorder  => 1234,
          file       => "data_perl-5.008006_i686-linux-thread-multi_Storable-2.13.le32",
          hdrsize    => 15,
          intsize    => 4,
          longsize   => 4,
          major      => 2,
          minor      => 6,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 4,
          version    => "2.6",
          version_nv => "2.006",
        },
    ],
    [
        "pst0\4\6\x0887654321\4\x08\x08\x08\3\0\0\0\13\4\3\0\0\0\0\0\0...",
        {
          byteorder  => 87_654_321,
          file       => "data_perl-5.008007_IA64.ARCHREV_0-thread-multi-LP64_Storable-2.13.be64",
          hdrsize    => 19,
          intsize    => 4,
          longsize   => 8,
          major      => 2,
          minor      => 6,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 8,
          version    => "2.6",
          version_nv => "2.006",
        },
    ],
    [
        "pst0\4\x07\x0812345678\4\x08\x08\x08\3\13\0\0\0\4\3\0\0\0\0\n\0...",
        {
          byteorder  => 12_345_678,
          file       => "data_perl-5.008007_x86-solaris-thread-multi-64_Storable-2.15.le64",
          hdrsize    => 19,
          intsize    => 4,
          longsize   => 8,
          major      => 2,
          minor      => 7,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 8,
          version    => "2.7",
          version_nv => "2.007",
        },
    ],
    [
        "pst0\5\x07\3\0\0\0\13\4\3\0\0\0\0\0\0\0\nempty_hash\n...",
        {
          file       => "data_perl-5.008007_x86-solaris-thread-multi-64_Storable-2.15.neutral",
          hdrsize    => 6,
          major      => 2,
          minor      => 7,
          netorder   => 1,
          version    => "2.7",
          version_nv => "2.007",
        },
    ],
    [
        "pst0\4\5\x041234\4\4\4\x08\3\13\0\0\0\4\3\0\0\0\0\n\0\0\0em...",
        {
          byteorder  => 1234,
          file       => "data_perl-5.008_i686-linux-thread-multi_Storable-2.04.le32",
          hdrsize    => 15,
          intsize    => 4,
          longsize   => 4,
          major      => 2,
          minor      => 5,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 4,
          version    => "2.5",
          version_nv => "2.005",
        },
    ],
    [
        "pst0\5\5\3\0\0\0\13\4\3\0\0\0\0\0\0\0\nempty_hash\n...",
        {
          file       => "data_perl-5.008_i686-linux-thread-multi_Storable-2.04.neutral",
          hdrsize    => 6,
          major      => 2,
          minor      => 5,
          netorder   => 1,
          version    => "2.5",
          version_nv => "2.005",
        },
    ],
    [
        "pst0\4\x07\x041234\4\4\4\x08\3\13\0\0\0\4\3\0\0\0\0\n\0\0\0em...",
        {
          byteorder  => 1234,
          file       => "data_perl-5.009003_i686-linux_Storable-2.15.le32",
          hdrsize    => 15,
          intsize    => 4,
          longsize   => 4,
          major      => 2,
          minor      => 7,
          netorder   => 0,
          nvsize     => 8,
          ptrsize    => 4,
          version    => "2.7",
          version_nv => "2.007",
        },
    ],
);

plan tests => 31 + 2 * @tests;

my $file = "xx-$$.pst";

is(eval { Storable::file_magic($file) }, undef, "empty file give undef");
like($@, qq{/^Can't open '\Q$file\E':/}, "...and croaks");
is(Storable::file_magic(__FILE__), undef, "not an image");

store({}, $file);
{
    my $info = Storable::file_magic($file);
    unlink($file);
    ok($info, "got info");
    is($info->{file}, $file, "file set");
    is($info->{hdrsize}, 11 + length($Config{byteorder}), "hdrsize");
    like($info->{version}, q{/^2\.\d+$/}, "sane version");
    is($info->{version_nv}, Storable::BIN_WRITE_VERSION_NV, "version_nv match");
    is($info->{major}, 2, "sane major");
    ok($info->{minor}, "have minor");
    ok($info->{minor} >= Storable::BIN_WRITE_MINOR, "large enough minor");

    ok(!$info->{netorder}, "no netorder");

    my %attrs = (
        nvsize  => 5.006, 
        ptrsize => 5.005, 
        map {$_ => 5.004} qw(byteorder intsize longsize)
    );
    for my $attr (keys %attrs) {
        SKIP: {
            skip "attribute $attr not available on this version of Perl", 1 if $attrs{$attr} > $];
            is($info->{$attr}, $Config{$attr}, "$attr match Config");
        }
    }
}

nstore({}, $file);
{
    my $info = Storable::file_magic($file);
    unlink($file);
    ok($info, "got info");
    is($info->{file}, $file, "file set");
    is($info->{hdrsize}, 6, "hdrsize");
    like($info->{version}, q{/^2\.\d+$/}, "sane version");
    is($info->{version_nv}, Storable::BIN_WRITE_VERSION_NV, "version_nv match");
    is($info->{major}, 2, "sane major");
    ok($info->{minor}, "have minor");
    ok($info->{minor} >= Storable::BIN_WRITE_MINOR, "large enough minor");

    ok($info->{netorder}, "no netorder");
    for (qw(byteorder intsize longsize ptrsize nvsize)) {
	ok(!exists $info->{$_}, "no $_");
    }
}

for my $test (@tests) {
    my($data, $expected) = @$test;
    open(FH, '>', $file) || die "Can't create $file: $!";
    binmode(FH);
    print FH $data;
    close(FH) || die "Can't write $file: $!";

    my $name = $expected->{file};
    $expected->{file} = $file;

    my $info = Storable::file_magic($file);
    unlink($file);

    is_deeply($info, $expected, "file_magic $name");

    $expected->{file} = 1;
    is_deeply(Storable::read_magic($data), $expected, "read magic $name");
}
