#!./perl
#
# Tests for Perl run-time environment variable settings
#
# $PERL5OPT, $PERL5LIB, etc.

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    require './test.pl';
    skip_all_without_config('d_fork');
}

plan tests => 106;

my $STDOUT = tempfile();
my $STDERR = tempfile();
my $PERL = './perl';
my $FAILURE_CODE = 119;

delete $ENV{PERLLIB};
delete $ENV{PERL5LIB};
delete $ENV{PERL5OPT};
delete $ENV{PERL_USE_UNSAFE_INC};

sub try {
  my ($env, $args, $stdout, $stderr) = @_;
  my ($actual_stdout, $actual_stderr) = runperl_and_capture($env, $args);
  local $::Level = $::Level + 1;
  my @envpairs = ();
  for my $k (sort keys %$env) {
    push @envpairs, "$k => $env->{$k}";
  }
  my $label = join(',' => (@envpairs, @$args));
  if (ref $stdout) {
    ok ( $actual_stdout =~/$stdout/, $label . ' stdout' );
  } else {
    is ( $actual_stdout, $stdout, $label . ' stdout' );
  }
  if (ref $stderr) {
    ok ( $actual_stderr =~/$stderr/, $label . ' stderr' );
  } else {
    is ( $actual_stderr, $stderr, $label . ' stderr' );
  }
}

#  PERL5OPT    Command-line options (switches).  Switches in
#                    this variable are taken as if they were on
#                    every Perl command line.  Only the -[DIMUdmtw]
#                    switches are allowed.  When running taint
#                    checks (because the program was running setuid
#                    or setgid, or the -T switch was used), this
#                    variable is ignored.  If PERL5OPT begins with
#                    -T, tainting will be enabled, and any
#                    subsequent options ignored.

try({PERL5OPT => '-w'}, ['-e', 'print $::x'],
    "", 
    qq{Name "main::x" used only once: possible typo at -e line 1.\nUse of uninitialized value \$x in print at -e line 1.\n});

try({PERL5OPT => '-Mstrict'}, ['-I../lib', '-e', 'print $::x'],
    "", "");

try({PERL5OPT => '-Mstrict'}, ['-I../lib', '-e', 'print $x'],
    "", 
    qq{Global symbol "\$x" requires explicit package name (did you forget to declare "my \$x"?) at -e line 1.\nExecution of -e aborted due to compilation errors.\n});

# Fails in 5.6.0
try({PERL5OPT => '-Mstrict -w'}, ['-I../lib', '-e', 'print $x'],
    "", 
    qq{Global symbol "\$x" requires explicit package name (did you forget to declare "my \$x"?) at -e line 1.\nExecution of -e aborted due to compilation errors.\n});

# Fails in 5.6.0
try({PERL5OPT => '-w -Mstrict'}, ['-I../lib', '-e', 'print $::x'],
    "", 
    <<ERROR
Name "main::x" used only once: possible typo at -e line 1.
Use of uninitialized value \$x in print at -e line 1.
ERROR
    );

# Fails in 5.6.0
try({PERL5OPT => '-w -Mstrict'}, ['-I../lib', '-e', 'print $::x'],
    "", 
    <<ERROR
Name "main::x" used only once: possible typo at -e line 1.
Use of uninitialized value \$x in print at -e line 1.
ERROR
    );

try({PERL5OPT => '-MExporter'}, ['-I../lib', '-e0'],
    "", 
    "");

# Fails in 5.6.0
try({PERL5OPT => '-MExporter -MExporter'}, ['-I../lib', '-e0'],
    "", 
    "");

try({PERL5OPT => '-Mstrict -Mwarnings'}, 
    ['-I../lib', '-e', 'print "ok" if $INC{"strict.pm"} and $INC{"warnings.pm"}'],
    "ok",
    "");

open my $fh, ">", "tmpOooof.pm" or die "Can't write tmpOooof.pm: $!";
print $fh "package tmpOooof; 1;\n";
close $fh;
END { 1 while unlink "tmpOooof.pm" }

try({PERL5OPT => '-I. -MtmpOooof'}, 
    ['-e', 'print "ok" if $INC{"tmpOooof.pm"} eq "tmpOooof.pm"'],
    "ok",
    "");

try({PERL5OPT => '-I./ -MtmpOooof'}, 
    ['-e', 'print "ok" if $INC{"tmpOooof.pm"} eq "tmpOooof.pm"'],
    "ok",
    "");

try({PERL5OPT => '-w -w'},
    ['-e', 'print $ENV{PERL5OPT}'],
    '-w -w',
    '');

SKIP: {
    if (exists($Config{taint_support}) && !$Config{taint_support}) {
        skip("built without taint support", 2);
    }
    try({PERL5OPT => '-t'},
        ['-e', 'print ${^TAINT}'],
        '-1',
        '');
}

try({PERL5OPT => '-W'},
    ['-I../lib','-e', 'local $^W = 0;  no warnings;  print $x'],
    '',
    <<ERROR
Name "main::x" used only once: possible typo at -e line 1.
Use of uninitialized value \$x in print at -e line 1.
ERROR
);

try({PERLLIB => "foobar$Config{path_sep}42"},
    ['-e', 'print grep { $_ eq "foobar" } @INC'],
    'foobar',
    '');

try({PERLLIB => "foobar$Config{path_sep}42"},
    ['-e', 'print grep { $_ eq "42" } @INC'],
    '42',
    '');

try({PERL5LIB => "foobar$Config{path_sep}42"},
    ['-e', 'print grep { $_ eq "foobar" } @INC'],
    'foobar',
    '');

try({PERL5LIB => "foobar$Config{path_sep}42"},
    ['-e', 'print grep { $_ eq "42" } @INC'],
    '42',
    '');

try({PERL5LIB => "foo",
     PERLLIB => "bar"},
    ['-e', 'print grep { $_ eq "foo" } @INC'],
    'foo',
    '');

try({PERL5LIB => "foo",
     PERLLIB => "bar"},
    ['-e', 'print grep { $_ eq "bar" } @INC'],
    '',
    '');

SKIP:
{
    skip "NO_PERL_HASH_SEED_DEBUG set", 4
      if $Config{ccflags} =~ /-DNO_PERL_HASH_SEED_DEBUG\b/;

    try({PERL_HASH_SEED_DEBUG => 1},
        ['-e','1'],
        '',
        qr/HASH_FUNCTION =/);

    try({PERL_HASH_SEED_DEBUG => 1},
        ['-e','1'],
        '',
        qr/HASH_SEED =/);
}

SKIP:
{
    skip "NO_PERL_HASH_ENV or NO_PERL_HASH_SEED_DEBUG set", 16
      if $Config{ccflags} =~ /-DNO_PERL_HASH_ENV\b/ ||
         $Config{ccflags} =~ /-DNO_PERL_HASH_SEED_DEBUG\b/;

    # special case, seed "0" implies disabled hash key traversal randomization
    try({PERL_HASH_SEED_DEBUG => 1, PERL_HASH_SEED => "0"},
        ['-e','1'],
        '',
        qr/PERTURB_KEYS = 0/);

    # check that setting it to a different value with the same logical value
    # triggers the normal "deterministic mode".
    try({PERL_HASH_SEED_DEBUG => 1, PERL_HASH_SEED => "0x0"},
        ['-e','1'],
        '',
        qr/PERTURB_KEYS = 2/);

    try({PERL_HASH_SEED_DEBUG => 1, PERL_PERTURB_KEYS => "0"},
        ['-e','1'],
        '',
        qr/PERTURB_KEYS = 0/);

    try({PERL_HASH_SEED_DEBUG => 1, PERL_PERTURB_KEYS => "1"},
        ['-e','1'],
        '',
        qr/PERTURB_KEYS = 1/);

    try({PERL_HASH_SEED_DEBUG => 1, PERL_PERTURB_KEYS => "2"},
        ['-e','1'],
        '',
        qr/PERTURB_KEYS = 2/);

    try({PERL_HASH_SEED_DEBUG => 1, PERL_HASH_SEED => "12345678"},
        ['-e','1'],
        '',
        qr/HASH_SEED = 0x12345678/);

    try({PERL_HASH_SEED_DEBUG => 1, PERL_HASH_SEED => "12"},
        ['-e','1'],
        '',
        qr/HASH_SEED = 0x12000000/);

    try({PERL_HASH_SEED_DEBUG => 1, PERL_HASH_SEED => "123456789"},
        ['-e','1'],
        '',
        qr/HASH_SEED = 0x12345678/);

    # Test that PERL_PERTURB_KEYS works as expected.  We check that we get the same
    # results if we use PERL_PERTURB_KEYS = 0 or 2 and we reuse the seed from previous run.
    my @print_keys = ( '-e', '@_{"A".."Z"}=(); print keys %_');
    for my $mode ( 0,1, 2 ) { # disabled and deterministic respectively
        my %base_opts = ( PERL_PERTURB_KEYS => $mode, PERL_HASH_SEED_DEBUG => 1 ),
          my ($out, $err) = runperl_and_capture( { %base_opts }, [ @print_keys ]);
        if ($err=~/HASH_SEED = (0x[a-f0-9]+)/) {
            my $seed = $1;
            my($out2, $err2) = runperl_and_capture( { %base_opts, PERL_HASH_SEED => $seed }, [ @print_keys ]);
            if ( $mode == 1 ) {
                isnt ($out,$out2,"PERL_PERTURB_KEYS = $mode results in different key order with the same key");
            } else {
                is ($out,$out2,"PERL_PERTURB_KEYS = $mode allows one to recreate a random hash");
            }
            is ($err,$err2,"Got the same debug output when we set PERL_HASH_SEED and PERL_PERTURB_KEYS");
        }
    }
}

# Tests for S_incpush_use_sep():

my @dump_inc = ('-e', 'print "$_\n" foreach @INC');

my ($out, $err) = runperl_and_capture({}, [@dump_inc]);

is ($err, '', 'No errors when determining @INC');

my @default_inc = split /\n/, $out;

SKIP: {
  skip_if_miniperl("under miniperl", 3);
if ($Config{default_inc_excludes_dot}) {
    ok !(grep { $_ eq '.' } @default_inc), '. is not in @INC';
    ($out, $err) = runperl_and_capture({ PERL_USE_UNSAFE_INC => 1 }, [@dump_inc]);

    is ($err, '', 'No errors when determining unsafe @INC');

    my @unsafe_inc = split /\n/, $out;

    ok (eq_array([@unsafe_inc], [@default_inc, '.']), '. last in unsafe @INC')
        or diag 'Unsafe @INC is: ', @unsafe_inc;
}
else {
    is ($default_inc[-1], '.', '. is last in @INC');
    skip('Not testing unsafe @INC when it includes . by default', 2);
}
}

my $sep = $Config{path_sep};
foreach (['nothing', ''],
	 ['something', 'zwapp', 'zwapp'],
	 ['two things', "zwapp${sep}bam", 'zwapp', 'bam'],
	 ['two things, ::', "zwapp${sep}${sep}bam", 'zwapp', 'bam'],
	 [': at start', "${sep}zwapp", 'zwapp'],
	 [': at end', "zwapp${sep}", 'zwapp'],
	 [':: sandwich ::', "${sep}${sep}zwapp${sep}${sep}", 'zwapp'],
	 [':', "${sep}"],
	 ['::', "${sep}${sep}"],
	 [':::', "${sep}${sep}${sep}"],
	 ['two things and :', "zwapp${sep}bam${sep}", 'zwapp', 'bam'],
	 [': and two things', "${sep}zwapp${sep}bam", 'zwapp', 'bam'],
	 [': two things :', "${sep}zwapp${sep}bam${sep}", 'zwapp', 'bam'],
	 ['three things', "zwapp${sep}bam${sep}${sep}owww",
	  'zwapp', 'bam', 'owww'],
	) {
  my ($name, $lib, @expect) = @$_;
  push @expect, @default_inc;

  ($out, $err) = runperl_and_capture({PERL5LIB => $lib}, [@dump_inc]);

  is ($err, '', "No errors when determining \@INC for $name");

  my @inc = split /\n/, $out;

  is (scalar @inc, scalar @expect,
      "expected number of elements in \@INC for $name");

  is ("@inc", "@expect", "expected elements in \@INC for $name");
}

# PERL5LIB tests with included arch directories still missing
