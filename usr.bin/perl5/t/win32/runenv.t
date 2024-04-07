#!./perl
#
# Tests for Perl run-time environment variable settings
# Clone of t/run/runenv.t but without the forking, and with cmd.exe-friendly -e syntax.
#
# $PERL5OPT, $PERL5LIB, etc.

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    require File::Temp; import File::Temp qw/:POSIX/;

    require Win32;
    ($::os_id, $::os_major) = ( Win32::GetOSVersion() )[ 4, 1 ];
    if ($::os_id == 2 and $::os_major == 6) {    # Vista, Server 2008 (incl R2), 7
	$::tests = 45;
    }
    else {
	$::tests = 42;
    }

    require './test.pl';
}

skip_all "requires compilation with PERL_IMPLICIT_SYS"
  unless $Config{ccflags} =~/(?:\A|\s)-DPERL_IMPLICIT_SYS\b/;

plan tests => $::tests;

my $PERL = '.\perl';
my $NL = $/;

delete $ENV{PERLLIB};
delete $ENV{PERL5LIB};
delete $ENV{PERL5OPT};


# Run perl with specified environment and arguments, return (STDOUT, STDERR)
sub runperl_and_capture {
  my ($env, $args) = @_;

  # Clear out old env
  local %ENV = %ENV;
  delete $ENV{PERLLIB};
  delete $ENV{PERL5LIB};
  delete $ENV{PERL5OPT};

  # Populate with our desired env
  for my $k (keys %$env) {
     $ENV{$k} = $env->{$k};
  }

  # This is slightly expensive, but this is more reliable than
  # trying to emulate fork(), and we still get STDERR and STDOUT individually.
  my $stderr_cache = tmpnam();
  my $stdout = `$PERL @$args 2>$stderr_cache`;
  my $stderr = '';
  if (-s $stderr_cache) {
    open(my $stderr_cache_fh, "<", $stderr_cache)
      or die "Could not retrieve STDERR output: $!";
    while ( defined(my $s_line = <$stderr_cache_fh>) ) {
      $stderr .= $s_line;
    }
    close $stderr_cache_fh;
    unlink $stderr_cache;
  }
  
  return ($stdout, $stderr);
}

sub try {
  my ($env, $args, $stdout, $stderr, $name) = @_;
  my ($actual_stdout, $actual_stderr) = runperl_and_capture($env, $args);
  $name ||= "";
  local $::Level = $::Level + 1;
  is $actual_stdout, $stdout, "$name - stdout";
  is $actual_stderr, $stderr, "$name - stderr";
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

try({PERL5OPT => '-w'}, ['-e', '"print $::x"'],
    "", 
    qq(Name "main::x" used only once: possible typo at -e line 1.${NL}Use of uninitialized value \$x in print at -e line 1.${NL}));

try({PERL5OPT => '-Mstrict'}, ['-I..\lib', '-e', '"print $::x"'],
    "", "");

try({PERL5OPT => '-Mstrict'}, ['-I..\lib', '-e', '"print $x"'],
    "", 
    qq(Global symbol "\$x" requires explicit package name (did you forget to declare "my \$x"?) at -e line 1.${NL}Execution of -e aborted due to compilation errors.${NL}));

# Fails in 5.6.0
try({PERL5OPT => '-Mstrict -w'}, ['-I..\lib', '-e', '"print $x"'],
    "", 
    qq(Global symbol "\$x" requires explicit package name (did you forget to declare "my \$x"?) at -e line 1.${NL}Execution of -e aborted due to compilation errors.${NL}));

# Fails in 5.6.0
try({PERL5OPT => '-w -Mstrict'}, ['-I..\lib', '-e', '"print $::x"'],
    "", 
    <<ERROR
Name "main::x" used only once: possible typo at -e line 1.
Use of uninitialized value \$x in print at -e line 1.
ERROR
    );

# Fails in 5.6.0
try({PERL5OPT => '-w -Mstrict'}, ['-I..\lib', '-e', '"print $::x"'],
    "", 
    <<ERROR
Name "main::x" used only once: possible typo at -e line 1.
Use of uninitialized value \$x in print at -e line 1.
ERROR
    );

try({PERL5OPT => '-MExporter'}, ['-I..\lib', '-e0'],
    "", 
    "");

# Fails in 5.6.0
try({PERL5OPT => '-MExporter -MExporter'}, ['-I..\lib', '-e0'],
    "", 
    "");

try({PERL5OPT => '-Mstrict -Mwarnings'}, 
    ['-I..\lib', '-e', '"print \"ok\" if $INC{\"strict.pm\"} and $INC{\"warnings.pm\"}"'],
    "ok",
    "");

open my $fh, ">", "Oooof.pm" or die "Can't write Oooof.pm: $!";
print $fh "package Oooof; 1;\n";
close $fh;
END { 1 while unlink "Oooof.pm" }

try({PERL5OPT => '-I. -MOooof'}, 
    ['-e', '"print \"ok\" if $INC{\"Oooof.pm\"} eq \"Oooof.pm\""'],
    "ok",
    "");

try({PERL5OPT => '-w -w'},
    ['-e', '"print $ENV{PERL5OPT}"'],
    '-w -w',
    '');

try({PERL5OPT => '-t'},
    ['-e', '"print ${^TAINT}"'],
    '-1',
    '');

try({PERL5OPT => '-W'},
    ['-I..\lib','-e', '"local $^W = 0;  no warnings;  print $x"'],
    '',
    <<ERROR
Name "main::x" used only once: possible typo at -e line 1.
Use of uninitialized value \$x in print at -e line 1.
ERROR
);

try({PERLLIB => "foobar$Config{path_sep}42"},
    ['-e', '"print grep { $_ eq \"foobar\" } @INC"'],
    'foobar',
    '');

try({PERLLIB => "foobar$Config{path_sep}42"},
    ['-e', '"print grep { $_ eq \"42\" } @INC"'],
    '42',
    '');

try({PERL5LIB => "foobar$Config{path_sep}42"},
    ['-e', '"print grep { $_ eq \"foobar\" } @INC"'],
    'foobar',
    '');

try({PERL5LIB => "foobar$Config{path_sep}42"},
    ['-e', '"print grep { $_ eq \"42\" } @INC"'],
    '42',
    '');

try({PERL5LIB => "foo",
     PERLLIB => "bar"},
    ['-e', '"print grep { $_ eq \"foo\" } @INC"'],
    'foo',
    '');

try({PERL5LIB => "foo",
     PERLLIB => "bar"},
    ['-e', '"print grep { $_ eq \"bar\" } @INC"'],
    '',
    '');

{
    # 131665
    # crashes without the fix
    my $longname = "X" x 2048;
    try({ $longname => 1 },
        [ '-e', '"print q/ok/"' ],
        'ok', '',
        'very long env var names' );
}

# Tests for S_incpush_use_sep():

my @dump_inc = ('-e', '"print \"$_\n\" foreach @INC"');

my ($out, $err) = runperl_and_capture({}, [@dump_inc]);

is ($err, '', 'No errors when determining @INC');

my @default_inc = split /\n/, $out;

if ($Config{default_inc_excludes_dot}) {
    ok !(grep { $_ eq '.' } @default_inc), '. is not in @INC';
}
else {
    is ($default_inc[-1], '.', '. is last in @INC');
}

my $sep = $Config{path_sep};
my @test_cases = (
	 ['nothing', ''],
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
);

# This block added to verify fix for RT #87322
if ($::os_id == 2 and $::os_major == 6) {    # Vista, Server 2008 (incl R2), 7
  my @big_perl5lib = ('z' x 16) x 2049;
    push @testcases, [
        'enough items so PERL5LIB val is longer than 32k',
        join($sep, @big_perl5lib), @big_perl5lib,
    ];
}

foreach ( @testcases ) {
  my ($name, $lib, @expect) = @$_;
  push @expect, @default_inc;

  ($out, $err) = runperl_and_capture({PERL5LIB => $lib}, [@dump_inc]);

  is ($err, '', "No errors when determining \@INC for $name");

  my @inc = split /\n/, $out;

  is (scalar @inc, scalar @expect,
      "expected number of elements in \@INC for $name");

  is ("@inc", "@expect", "expected elements in \@INC for $name");
}
