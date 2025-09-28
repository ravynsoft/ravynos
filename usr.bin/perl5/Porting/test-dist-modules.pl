#!perl
# this should be perl 5.8 compatible, since it will be used
# with old perls while testing dist modules on those perls
use strict;
use warnings;
use File::Temp "tempdir";
use ExtUtils::Manifest "maniread";
use Cwd "getcwd";
use Getopt::Long;
use Config;

my $continue;
my $separate;
GetOptions("c|continue" => \$continue,
           "s|separate" => \$separate,
           "h|help"     => \&usage)
  or die "Unknown options\n";

$|++;

-f "Configure"
  or die "Expected to be run from a perl checkout";

my $github_ci = $ENV{'GITHUB_SHA'} ? 1 : 0;

my $manifest = maniread();
my @failures = ();

my @config;
my $install_path;
if ($separate) {
    # require EU::MM 6.31 or later
    my $install_base = tempdir( CLEANUP => 1 );
    push @config, "INSTALL_BASE=$install_base";
    $ENV{PERL5LIB} .= $Config{path_sep} if $ENV{PERL5LIB};
    $ENV{PERL5LIB} .= join $Config{path_sep},
      "$install_base/lib/perl5/$Config{archname}",
      "$install_base/lib/perl5";
}

my %dist_config = (
    # these are defined by the modules as distributed on CPAN
    # I don't know why their Makefile.PLs aren't in core
    "threads"        => [ "DEFINE=-DHAS_PPPORT_H" ],
    "threads-shared" => [ "DEFINE=-DHAS_PPPORT_H" ],
   );

my $start = getcwd()
  or die "Cannot fetch current directory: $!\n";

# get ppport.h
my $pppdir = test_dist("Devel-PPPort");

if (@failures) {
    if ($github_ci) {
        # GitHub may show STDERR before STDOUT.. despite autoflush
        # being enabled.. Make sure it detects the 'endgroup' before
        # the `die` statement.
        print STDERR "::endgroup::\n";
    }
    die "Devel-PPPort failed, aborting other tests.\n";
}

my $pppfile = "$pppdir/ppport.h";

-f $pppfile
  or die "No ppport.h found in $pppdir\n";

# Devel-PPPort is manually processed before anything else to ensure we
# have an up to date ppport.h
my @dists = @ARGV;
if (@dists) {
    for my $dist (@dists) {
        -d "dist/$dist" or die "dist/$dist not a directory\n";
    }
}
else {
    opendir my $distdir, "dist"
      or die "Cannot opendir 'dist': $!\n";
    @dists = sort { lc $a cmp lc $b } grep { /^\w/ && $_ ne "Devel-PPPort" } readdir $distdir;
    closedir $distdir;
}

# These may end up being included if their problems are resolved
{
    # https://github.com/Perl/version.pm claims CPAN is upstream
    @dists = grep { $_ ne "version" } @dists;

    # Safe is tied pretty heavily to core
    # in any case it didn't seem simple to fix
    @dists = grep { $_ ne "Safe" } @dists;
}

for my $dist (@dists) {
    test_dist($dist);
}

if (@failures) {
    if ($github_ci) {
        # GitHub may show STDERR before STDOUT.. despite autoflush
        # being enabled.. Make sure it detects the 'endgroup' before
        # the `die` statement.
        print STDERR "::endgroup::\n";
    }
    my $msg = join("\n", map { "\t'$_->[0]' failed at $_->[1]" } @failures);
    die "Following dists had failures:\n$msg\n";
}

sub test_dist {
    my ($name) = @_;

    print "::group::Testing $name\n" if $github_ci;
    print "*** Testing $name ***\n";
    my $dir = tempdir( CLEANUP => 1);
    run("cp", "-a", "dist/$name/.", "$dir/.")
      or die "Cannot copy dist files to working directory\n";
    chdir $dir
      or die "Cannot chdir to dist working directory '$dir': $!\n";
    if ($pppfile) {
        run("cp", $pppfile, ".")
          or die "Cannot copy $pppfile to .\n";
    }
    if ($name eq "IO" || $name eq "threads" || $name eq "threads-shared") {
        write_testpl();
    }
    if ($name eq "threads" || $name eq "threads-shared") {
        write_threads_h();
    }
    if ($name eq "threads-shared") {
        write_shared_h();
    }
    unless (-f "Makefile.PL") {
        print "  Creating Makefile.PL for $name\n";
        my $key = "ABSTRACT_FROM";
        my @parts = split /-/, $name;
        my $last = $parts[-1];
        my $module = join "::", @parts;
        my $fromname;
        for my $check ("$last.pm", join("/", "lib", @parts) . ".pm") {
            if (-f $check) {
                $fromname = $check;
                last;
            }
        }
        $fromname
          or die "Cannot find ABSTRACT_FROM for $name\n";
        my $value = $fromname;
        open my $fh, ">", "Makefile.PL"
          or die "Cannot create Makefile.PL: $!\n";
        # adapted from make_ext.pl
        printf $fh <<'EOM', $module, $fromname, $key, $value;
use strict;
use ExtUtils::MakeMaker;

# This is what the .PL extracts to. Not the ultimate file that is installed.
# (ie Win32 runs pl2bat after this)

# Doing this here avoids all sort of quoting issues that would come from
# attempting to write out perl source with literals to generate the arrays and
# hash.
my @temps = 'Makefile.PL';
foreach (glob('scripts/pod*.PL')) {
    # The various pod*.PL extractors change directory. Doing that with relative
    # paths in @INC breaks. It seems the lesser of two evils to copy (to avoid)
    # the chdir doing anything, than to attempt to convert lib paths to
    # absolute, and potentially run into problems with quoting special
    # characters in the path to our build dir (such as spaces)
    require File::Copy;

    my $temp = $_;
    $temp =~ s!scripts/!!;
    File::Copy::copy($_, $temp) or die "Can't copy $temp to $_: $!";
    push @temps, $temp;
}

my $script_ext = $^O eq 'VMS' ? '.com' : '';
my %%pod_scripts;
foreach (glob('pod*.PL')) {
    my $script = $_;
    s/.PL$/$script_ext/i;
    $pod_scripts{$script} = $_;
}
my @exe_files = values %%pod_scripts;

WriteMakefile(
    NAME          => '%s',
    VERSION_FROM  => '%s',
    %-13s => '%s',
    realclean     => { FILES => "@temps" },
    (%%pod_scripts ? (
        PL_FILES  => \%%pod_scripts,
        EXE_FILES => \@exe_files,
        clean     => { FILES => "@exe_files" },
    ) : ()),
);

EOM
        close $fh;
    }

    my $verbose = $github_ci && $ENV{'RUNNER_DEBUG'} ? 1 : 0;
    my $failed = "";
    my @my_config = @config;
    if (my $cfg = $dist_config{$name}) {
        push @my_config, @$cfg;
    }
    if (!run($^X, "Makefile.PL", @my_config)) {
        $failed = "Makefile.PL";
        die "$name: Makefile.PL failed\n" unless $continue;
    }
    elsif (!run("make", "test", "TEST_VERBOSE=$verbose")) {
        $failed = "make test";
        die "$name: make test failed\n" unless $continue;
    }
    elsif (!run("make", "install")) {
        $failed = "make install";
        die "$name: make install failed\n" unless $continue;
    }

    chdir $start
      or die "Cannot return to $start: $!\n";

    if ($github_ci) {
        print "::endgroup::\n";
    }
    if ($continue && $failed) {
        print "::error ::$name failed at $failed\n" if $github_ci;
        push @failures, [ $name, $failed ];
    }

    $dir;
}

# IO, threads and threads-shared use the blead t/test.pl when tested in core
# and bundle their own test.pl when distributed on CPAN.
# The test.pl source below is from the IO distribution but so far seems sufficient
# for threads and threads-shared.
sub write_testpl {
    _write_from_data("t/test.pl");
}

# threads and threads-shared bundle this file, which isn't needed in core
sub write_threads_h {
    _write_from_data("threads.h");
}

# threads-shared bundles this file, which isn't needed in core
sub write_shared_h {
    _write_from_data("shared.h");
}

# file data read from <DATA>
my %file_data;

sub _write_from_data {
    my ($want_name) = @_;

    unless (keys %file_data) {
        my $name;
        while (<DATA>) {
            if (/^-- (\S+) --/) {
                $name = $1;
            }
            else {
                $file_data{$name} .= $_;
            }
        }
        close DATA;
    }

    my $data = $file_data{$want_name} or die "No data found for $want_name";
    open my $fh, ">", $want_name
      or die "Cannot create $want_name: $!\n";
    print $fh $data;
    close $fh
      or die "Cannot close $want_name: $!\n";
}

sub run {
    my (@cmd) = @_;

    print "\$ @cmd\n";
    my $result = system(@cmd);
    if ($result < 0) {
        print "Failed: $!\n";
    }
    elsif ($result) {
        printf "Failed: %d (%#x)\n", $result, $?;
    }
    return $result == 0;
}

sub usage {
    print <<EOS;
Usage: $^X $0 [options] [distnames]
 -c | -continue
     Continue processing after failures
     Devel::PPPort must successfully build to continue.
 -s | -separate
     Install to a work path, not to perl's site_perl.
 -h | -help
     Display this message.

Optional distnames should be names of the distributions under dist/ to
test.  If omitted all of the distributions under dist/ are tested.
Devel-PPPort is always tested.

Test all of the distributions, stop on the first failure:

   $^X $0 -s

Test the various threads distributions, continue on failure:

   $^X $0 -s -c threads threads-shared Thread-Queue Thread-Semaphore
EOS
}

__DATA__
-- t/test.pl --
#
# t/test.pl - most of Test::More functionality without the fuss
 
 
# NOTE:
#
# Increment ($x++) has a certain amount of cleverness for things like
#
#   $x = 'zz';
#   $x++; # $x eq 'aaa';
#
# stands more chance of breaking than just a simple
#
#   $x = $x + 1
#
# In this file, we use the latter "Baby Perl" approach, and increment
# will be worked over by t/op/inc.t
 
$Level = 1;
my $test = 1;
my $planned;
my $noplan;
my $Perl;       # Safer version of $^X set by which_perl()
 
$TODO = 0;
$NO_ENDING = 0;
 
# Use this instead of print to avoid interference while testing globals.
sub _print {
    local($\, $", $,) = (undef, ' ', '');
    print STDOUT @_;
}
 
sub _print_stderr {
    local($\, $", $,) = (undef, ' ', '');
    print STDERR @_;
}
 
sub plan {
    my $n;
    if (@_ == 1) {
        $n = shift;
        if ($n eq 'no_plan') {
          undef $n;
          $noplan = 1;
        }
    } else {
        my %plan = @_;
        $n = $plan{tests};
    }
    _print "1..$n\n" unless $noplan;
    $planned = $n;
}
 
END {
    my $ran = $test - 1;
    if (!$NO_ENDING) {
        if (defined $planned && $planned != $ran) {
            _print_stderr
                "# Looks like you planned $planned tests but ran $ran.\n";
        } elsif ($noplan) {
            _print "1..$ran\n";
        }
    }
}
 
# Use this instead of "print STDERR" when outputing failure diagnostic
# messages
sub _diag {
    return unless @_;
    my @mess = map { /^#/ ? "$_\n" : "# $_\n" }
               map { split /\n/ } @_;
    $TODO ? _print(@mess) : _print_stderr(@mess);
}
 
sub diag {
    _diag(@_);
}
 
sub skip_all {
    if (@_) {
        _print "1..0 # Skip @_\n";
    } else {
        _print "1..0\n";
    }
    exit(0);
}
 
sub _ok {
    my ($pass, $where, $name, @mess) = @_;
    # Do not try to microoptimize by factoring out the "not ".
    # VMS will avenge.
    my $out;
    if ($name) {
        # escape out '#' or it will interfere with '# skip' and such
        $name =~ s/#/\\#/g;
        $out = $pass ? "ok $test - $name" : "not ok $test - $name";
    } else {
        $out = $pass ? "ok $test" : "not ok $test";
    }
 
    $out .= " # TODO $TODO" if $TODO;
    _print "$out\n";
 
    unless ($pass) {
        _diag "# Failed $where\n";
    }
 
    # Ensure that the message is properly escaped.
    _diag @mess;
 
    $test = $test + 1; # don't use ++
 
    return $pass;
}
 
sub _where {
    my @caller = caller($Level);
    return "at $caller[1] line $caller[2]";
}
 
# DON'T use this for matches. Use like() instead.
sub ok ($@) {
    my ($pass, $name, @mess) = @_;
    _ok($pass, _where(), $name, @mess);
}
 
sub _q {
    my $x = shift;
    return 'undef' unless defined $x;
    my $q = $x;
    $q =~ s/\\/\\\\/g;
    $q =~ s/'/\\'/g;
    return "'$q'";
}
 
sub _qq {
    my $x = shift;
    return defined $x ? '"' . display ($x) . '"' : 'undef';
};
 
# keys are the codes \n etc map to, values are 2 char strings such as \n
my %backslash_escape;
foreach my $x (split //, 'nrtfa\\\'"') {
    $backslash_escape{ord eval "\"\\$x\""} = "\\$x";
}
# A way to display scalars containing control characters and Unicode.
# Trying to avoid setting $_, or relying on local $_ to work.
sub display {
    my @result;
    foreach my $x (@_) {
        if (defined $x and not ref $x) {
            my $y = '';
            foreach my $c (unpack("U*", $x)) {
                if ($c > 255) {
                    $y .= sprintf "\\x{%x}", $c;
                } elsif ($backslash_escape{$c}) {
                    $y .= $backslash_escape{$c};
                } else {
                    my $z = chr $c; # Maybe we can get away with a literal...
                    $z = sprintf "\\%03o", $c if $z =~ /[[:^print:]]/;
                    $y .= $z;
                }
            }
            $x = $y;
        }
        return $x unless wantarray;
        push @result, $x;
    }
    return @result;
}
 
sub is ($$@) {
    my ($got, $expected, $name, @mess) = @_;
 
    my $pass;
    if( !defined $got || !defined $expected ) {
        # undef only matches undef
        $pass = !defined $got && !defined $expected;
    }
    else {
        $pass = $got eq $expected;
    }
 
    unless ($pass) {
        unshift(@mess, "#      got "._q($got)."\n",
                       "# expected "._q($expected)."\n");
    }
    _ok($pass, _where(), $name, @mess);
}
 
sub isnt ($$@) {
    my ($got, $isnt, $name, @mess) = @_;
 
    my $pass;
    if( !defined $got || !defined $isnt ) {
        # undef only matches undef
        $pass = defined $got || defined $isnt;
    }
    else {
        $pass = $got ne $isnt;
    }
 
    unless( $pass ) {
        unshift(@mess, "# it should not be "._q($got)."\n",
                       "# but it is.\n");
    }
    _ok($pass, _where(), $name, @mess);
}
 
sub cmp_ok ($$$@) {
    my($got, $type, $expected, $name, @mess) = @_;
 
    my $pass;
    {
        local $^W = 0;
        local($@,$!);   # don't interfere with $@
                        # eval() sometimes resets $!
        $pass = eval "\$got $type \$expected";
    }
    unless ($pass) {
        # It seems Irix long doubles can have 2147483648 and 2147483648
        # that stringify to the same thing but are acutally numerically
        # different. Display the numbers if $type isn't a string operator,
        # and the numbers are stringwise the same.
        # (all string operators have alphabetic names, so tr/a-z// is true)
        # This will also show numbers for some uneeded cases, but will
        # definately be helpful for things such as == and <= that fail
        if ($got eq $expected and $type !~ tr/a-z//) {
            unshift @mess, "# $got - $expected = " . ($got - $expected) . "\n";
        }
        unshift(@mess, "#      got "._q($got)."\n",
                       "# expected $type "._q($expected)."\n");
    }
    _ok($pass, _where(), $name, @mess);
}
 
# Check that $got is within $range of $expected
# if $range is 0, then check it's exact
# else if $expected is 0, then $range is an absolute value
# otherwise $range is a fractional error.
# Here $range must be numeric, >= 0
# Non numeric ranges might be a useful future extension. (eg %)
sub within ($$$@) {
    my ($got, $expected, $range, $name, @mess) = @_;
    my $pass;
    if (!defined $got or !defined $expected or !defined $range) {
        # This is a fail, but doesn't need extra diagnostics
    } elsif ($got !~ tr/0-9// or $expected !~ tr/0-9// or $range !~ tr/0-9//) {
        # This is a fail
        unshift @mess, "# got, expected and range must be numeric\n";
    } elsif ($range < 0) {
        # This is also a fail
        unshift @mess, "# range must not be negative\n";
    } elsif ($range == 0) {
        # Within 0 is ==
        $pass = $got == $expected;
    } elsif ($expected == 0) {
        # If expected is 0, treat range as absolute
        $pass = ($got <= $range) && ($got >= - $range);
    } else {
        my $diff = $got - $expected;
        $pass = abs ($diff / $expected) < $range;
    }
    unless ($pass) {
        if ($got eq $expected) {
            unshift @mess, "# $got - $expected = " . ($got - $expected) . "\n";
        }
        unshift@mess, "#      got "._q($got)."\n",
                      "# expected "._q($expected)." (within "._q($range).")\n";
    }
    _ok($pass, _where(), $name, @mess);
}
 
# Note: this isn't quite as fancy as Test::More::like().
 
sub like   ($$@) { like_yn (0,@_) }; # 0 for -
sub unlike ($$@) { like_yn (1,@_) }; # 1 for un-
 
sub like_yn ($$$@) {
    my ($flip, $got, $expected, $name, @mess) = @_;
    my $pass;
    $pass = $got =~ /$expected/ if !$flip;
    $pass = $got !~ /$expected/ if $flip;
    unless ($pass) {
        unshift(@mess, "#      got '$got'\n",
                $flip
                ? "# expected !~ /$expected/\n" : "# expected /$expected/\n");
    }
    local $Level = $Level + 1;
    _ok($pass, _where(), $name, @mess);
}
 
sub pass {
    _ok(1, '', @_);
}
 
sub fail {
    _ok(0, _where(), @_);
}
 
sub curr_test {
    $test = shift if @_;
    return $test;
}
 
sub next_test {
  my $retval = $test;
  $test = $test + 1; # don't use ++
  $retval;
}
 
# Note: can't pass multipart messages since we try to
# be compatible with Test::More::skip().
sub skip {
    my $why = shift;
    my $n    = @_ ? shift : 1;
    for (1..$n) {
        _print "ok $test # skip $why\n";
        $test = $test + 1;
    }
    local $^W = 0;
    last SKIP;
}
 
sub todo_skip {
    my $why = shift;
    my $n   = @_ ? shift : 1;
 
    for (1..$n) {
        _print "not ok $test # TODO & SKIP $why\n";
        $test = $test + 1;
    }
    local $^W = 0;
    last TODO;
}
 
sub eq_array {
    my ($ra, $rb) = @_;
    return 0 unless $#$ra == $#$rb;
    for my $i (0..$#$ra) {
        next     if !defined $ra->[$i] && !defined $rb->[$i];
        return 0 if !defined $ra->[$i];
        return 0 if !defined $rb->[$i];
        return 0 unless $ra->[$i] eq $rb->[$i];
    }
    return 1;
}
 
sub eq_hash {
  my ($orig, $suspect) = @_;
  my $fail;
  while (my ($key, $value) = each %$suspect) {
    # Force a hash recompute if this perl's internals can cache the hash key.
    $key = "" . $key;
    if (exists $orig->{$key}) {
      if ($orig->{$key} ne $value) {
        _print "# key ", _qq($key), " was ", _qq($orig->{$key}),
                     " now ", _qq($value), "\n";
        $fail = 1;
      }
    } else {
      _print "# key ", _qq($key), " is ", _qq($value),
                   ", not in original.\n";
      $fail = 1;
    }
  }
  foreach (keys %$orig) {
    # Force a hash recompute if this perl's internals can cache the hash key.
    $_ = "" . $_;
    next if (exists $suspect->{$_});
    _print "# key ", _qq($_), " was ", _qq($orig->{$_}), " now missing.\n";
    $fail = 1;
  }
  !$fail;
}
 
sub require_ok ($) {
    my ($require) = @_;
    eval <<REQUIRE_OK;
require $require;
REQUIRE_OK
    _ok(!$@, _where(), "require $require");
}
 
sub use_ok ($) {
    my ($use) = @_;
    eval <<USE_OK;
use $use;
USE_OK
    _ok(!$@, _where(), "use $use");
}
 
# runperl - Runs a separate perl interpreter.
# Arguments :
#   switches => [ command-line switches ]
#   nolib    => 1 # don't use -I../lib (included by default)
#   prog     => one-liner (avoid quotes)
#   progs    => [ multi-liner (avoid quotes) ]
#   progfile => perl script
#   stdin    => string to feed the stdin
#   stderr   => redirect stderr to stdout
#   args     => [ command-line arguments to the perl program ]
#   verbose  => print the command line
 
my $is_mswin    = $^O eq 'MSWin32';
my $is_netware  = $^O eq 'NetWare';
my $is_macos    = $^O eq 'MacOS';
my $is_vms      = $^O eq 'VMS';
my $is_cygwin   = $^O eq 'cygwin';
 
sub _quote_args {
    my ($runperl, $args) = @_;
 
    foreach (@$args) {
        # In VMS protect with doublequotes because otherwise
        # DCL will lowercase -- unless already doublequoted.
       $_ = q(").$_.q(") if $is_vms && !/^\"/ && length($_) > 0;
        $$runperl .= ' ' . $_;
    }
}
 
sub _create_runperl { # Create the string to qx in runperl().
    my %args = @_;
    my $runperl = which_perl();
    if ($runperl =~ m/\s/) {
        $runperl = qq{"$runperl"};
    }
    #- this allows, for example, to set PERL_RUNPERL_DEBUG=/usr/bin/valgrind
    if ($ENV{PERL_RUNPERL_DEBUG}) {
        $runperl = "$ENV{PERL_RUNPERL_DEBUG} $runperl";
    }
    unless ($args{nolib}) {
        if ($is_macos) {
            $runperl .= ' -I::lib';
            # Use UNIX style error messages instead of MPW style.
            $runperl .= ' -MMac::err=unix' if $args{stderr};
        }
        else {
            $runperl .= ' "-I../lib"'; # doublequotes because of VMS
        }
    }
    if ($args{switches}) {
        local $Level = 2;
        die "test.pl:runperl(): 'switches' must be an ARRAYREF " . _where()
            unless ref $args{switches} eq "ARRAY";
        _quote_args(\$runperl, $args{switches});
    }
    if (defined $args{prog}) {
        die "test.pl:runperl(): both 'prog' and 'progs' cannot be used " . _where()
            if defined $args{progs};
        $args{progs} = [$args{prog}]
    }
    if (defined $args{progs}) {
        die "test.pl:runperl(): 'progs' must be an ARRAYREF " . _where()
            unless ref $args{progs} eq "ARRAY";
        foreach my $prog (@{$args{progs}}) {
            if ($is_mswin || $is_netware || $is_vms) {
                $runperl .= qq ( -e "$prog" );
            }
            else {
                $runperl .= qq ( -e '$prog' );
            }
        }
    } elsif (defined $args{progfile}) {
        $runperl .= qq( "$args{progfile}");
    } else {
        # You probaby didn't want to be sucking in from the upstream stdin
        die "test.pl:runperl(): none of prog, progs, progfile, args, "
            . " switches or stdin specified"
            unless defined $args{args} or defined $args{switches}
                or defined $args{stdin};
    }
    if (defined $args{stdin}) {
        # so we don't try to put literal newlines and crs onto the
        # command line.
        $args{stdin} =~ s/\n/\\n/g;
        $args{stdin} =~ s/\r/\\r/g;
 
        if ($is_mswin || $is_netware || $is_vms) {
            $runperl = qq{$Perl -e "print qq(} .
                $args{stdin} . q{)" | } . $runperl;
        }
        elsif ($is_macos) {
            # MacOS can only do two processes under MPW at once;
            # the test itself is one; we can't do two more, so
            # write to temp file
            my $stdin = qq{$Perl -e 'print qq(} . $args{stdin} . qq{)' > teststdin; };
            if ($args{verbose}) {
                my $stdindisplay = $stdin;
                $stdindisplay =~ s/\n/\n\#/g;
                _print_stderr "# $stdindisplay\n";
            }
            `$stdin`;
            $runperl .= q{ < teststdin };
        }
        else {
            $runperl = qq{$Perl -e 'print qq(} .
                $args{stdin} . q{)' | } . $runperl;
        }
    }
    if (defined $args{args}) {
        _quote_args(\$runperl, $args{args});
    }
    $runperl .= ' 2>&1'          if  $args{stderr} && !$is_macos;
    $runperl .= " \xB3 Dev:Null" if !$args{stderr} &&  $is_macos;
    if ($args{verbose}) {
        my $runperldisplay = $runperl;
        $runperldisplay =~ s/\n/\n\#/g;
        _print_stderr "# $runperldisplay\n";
    }
    return $runperl;
}
 
sub runperl {
    die "test.pl:runperl() does not take a hashref"
        if ref $_[0] and ref $_[0] eq 'HASH';
    my $runperl = &_create_runperl;
    my $result;
 
    my $tainted = ${^TAINT};
    my %args = @_;
    exists $args{switches} && grep m/^-T$/, @{$args{switches}} and $tainted = $tainted + 1;
 
    if ($tainted) {
        # We will assume that if you're running under -T, you really mean to
        # run a fresh perl, so we'll brute force launder everything for you
        my $sep;
 
        if (! eval 'require Config; 1') {
            warn "test.pl had problems loading Config: $@";
            $sep = ':';
        } else {
            $sep = $Config::Config{path_sep};
        }
 
        my @keys = grep {exists $ENV{$_}} qw(CDPATH IFS ENV BASH_ENV);
        local @ENV{@keys} = ();
        # Untaint, plus take out . and empty string:
        local $ENV{'DCL$PATH'} = $1 if $is_vms && ($ENV{'DCL$PATH'} =~ /(.*)/s);
        $ENV{PATH} =~ /(.*)/s;
        local $ENV{PATH} =
            join $sep, grep { $_ ne "" and $_ ne "." and -d $_ and
                ($is_mswin or $is_vms or !(stat && (stat _)[2]&0022)) }
                    split quotemeta ($sep), $1;
        $ENV{PATH} .= "$sep/bin" if $is_cygwin;  # Must have /bin under Cygwin
 
        $runperl =~ /(.*)/s;
        $runperl = $1;
 
        $result = `$runperl`;
    } else {
        $result = `$runperl`;
    }
    $result =~ s/\n\n/\n/ if $is_vms; # XXX pipes sometimes double these
    return $result;
}
 
*run_perl = \&runperl; # Nice alias.
 
sub DIE {
    _print_stderr "# @_\n";
    exit 1;
}
 
# A somewhat safer version of the sometimes wrong $^X.
sub which_perl {
    unless (defined $Perl) {
        $Perl = $^X;
 
        # VMS should have 'perl' aliased properly
        return $Perl if $^O eq 'VMS';
 
        my $exe;
        if (! eval 'require Config; 1') {
            warn "test.pl had problems loading Config: $@";
            $exe = '';
        } else {
            $exe = $Config::Config{_exe};
        }
       $exe = '' unless defined $exe;
 
        # This doesn't absolutize the path: beware of future chdirs().
        # We could do File::Spec->abs2rel() but that does getcwd()s,
        # which is a bit heavyweight to do here.
 
        if ($Perl =~ /^perl\Q$exe\E$/i) {
            my $perl = "perl$exe";
            if (! eval 'require File::Spec; 1') {
                warn "test.pl had problems loading File::Spec: $@";
                $Perl = "./$perl";
            } else {
                $Perl = File::Spec->catfile(File::Spec->curdir(), $perl);
            }
        }
 
        # Build up the name of the executable file from the name of
        # the command.
 
        if ($Perl !~ /\Q$exe\E$/i) {
            $Perl .= $exe;
        }
 
        warn "which_perl: cannot find $Perl from $^X" unless -f $Perl;
 
        # For subcommands to use.
        $ENV{PERLEXE} = $Perl;
    }
    return $Perl;
}
 
sub unlink_all {
    foreach my $file (@_) {
        1 while unlink $file;
        _print_stderr "# Couldn't unlink '$file': $!\n" if -f $file;
    }
}
 
my %tmpfiles;
END { unlink_all keys %tmpfiles }
 
# A regexp that matches the tempfile names
$::tempfile_regexp = 'tmp\d+[A-Z][A-Z]?';
 
# Avoid ++, avoid ranges, avoid split //
my @letters = qw(A B C D E F G H I J K L M N O P Q R S T U V W X Y Z);
sub tempfile {
    my $count = 0;
    do {
        my $temp = $count;
        my $try = "tmp$$";
        do {
            $try .= $letters[$temp % 26];
            $temp = int ($temp / 26);
        } while $temp;
        # Need to note all the file names we allocated, as a second request may
        # come before the first is created.
        if (!-e $try && !$tmpfiles{$try}) {
            # We have a winner
            $tmpfiles{$try}++;
            return $try;
        }
        $count = $count + 1;
    } while $count < 26 * 26;
    die "Can't find temporary file name starting 'tmp$$'";
}
 
# This is the temporary file for _fresh_perl
my $tmpfile = tempfile();
 
#
# _fresh_perl
#
# The $resolve must be a subref that tests the first argument
# for success, or returns the definition of success (e.g. the
# expected scalar) if given no arguments.
#
 
sub _fresh_perl {
    my($prog, $resolve, $runperl_args, $name) = @_;
 
    $runperl_args ||= {};
    $runperl_args->{progfile} = $tmpfile;
    $runperl_args->{stderr} = 1;
 
    open TEST, ">$tmpfile" or die "Cannot open $tmpfile: $!";
 
    # VMS adjustments
    if( $^O eq 'VMS' ) {
        $prog =~ s#/dev/null#NL:#;
 
        # VMS file locking
        $prog =~ s{if \(-e _ and -f _ and -r _\)}
                  {if (-e _ and -f _)}
    }
 
    print TEST $prog;
    close TEST or die "Cannot close $tmpfile: $!";
 
    my $results = runperl(%$runperl_args);
    my $status = $?;
 
    # Clean up the results into something a bit more predictable.
    $results =~ s/\n+$//;
    $results =~ s/at\s+$::tempfile_regexp\s+line/at - line/g;
    $results =~ s/of\s+$::tempfile_regexp\s+aborted/of - aborted/g;
 
    # bison says 'parse error' instead of 'syntax error',
    # various yaccs may or may not capitalize 'syntax'.
    $results =~ s/^(syntax|parse) error/syntax error/mig;
 
    if ($^O eq 'VMS') {
        # some tests will trigger VMS messages that won't be expected
        $results =~ s/\n?%[A-Z]+-[SIWEF]-[A-Z]+,.*//;
 
        # pipes double these sometimes
        $results =~ s/\n\n/\n/g;
    }
 
    my $pass = $resolve->($results);
    unless ($pass) {
        _diag "# PROG: \n$prog\n";
        _diag "# EXPECTED:\n", $resolve->(), "\n";
        _diag "# GOT:\n$results\n";
        _diag "# STATUS: $status\n";
    }
 
    # Use the first line of the program as a name if none was given
    unless( $name ) {
        ($first_line, $name) = $prog =~ /^((.{1,50}).*)/;
        $name .= '...' if length $first_line > length $name;
    }
 
    _ok($pass, _where(), "fresh_perl - $name");
}
 
#
# fresh_perl_is
#
# Combination of run_perl() and is().
#
 
sub fresh_perl_is {
    my($prog, $expected, $runperl_args, $name) = @_;
    local $Level = 2;
    _fresh_perl($prog,
                sub { @_ ? $_[0] eq $expected : $expected },
                $runperl_args, $name);
}
 
#
# fresh_perl_like
#
# Combination of run_perl() and like().
#
 
sub fresh_perl_like {
    my($prog, $expected, $runperl_args, $name) = @_;
    local $Level = 2;
    _fresh_perl($prog,
                sub { @_ ?
                          $_[0] =~ (ref $expected ? $expected : /$expected/) :
                          $expected },
                $runperl_args, $name);
}
 
sub can_ok ($@) {
    my($proto, @methods) = @_;
    my $class = ref $proto || $proto;
 
    unless( @methods ) {
        return _ok( 0, _where(), "$class->can(...)" );
    }
 
    my @nok = ();
    foreach my $method (@methods) {
        local($!, $@);  # don't interfere with caller's $@
                        # eval sometimes resets $!
        eval { $proto->can($method) } || push @nok, $method;
    }
 
    my $name;
    $name = @methods == 1 ? "$class->can('$methods[0]')"
                          : "$class->can(...)";
 
    _ok( !@nok, _where(), $name );
}
 
sub isa_ok ($$;$) {
    my($object, $class, $obj_name) = @_;
 
    my $diag;
    $obj_name = 'The object' unless defined $obj_name;
    my $name = "$obj_name isa $class";
    if( !defined $object ) {
        $diag = "$obj_name isn't defined";
    }
    elsif( !ref $object ) {
        $diag = "$obj_name isn't a reference";
    }
    else {
        # We can't use UNIVERSAL::isa because we want to honor isa() overrides
        local($@, $!);  # eval sometimes resets $!
        my $rslt = eval { $object->isa($class) };
        if( $@ ) {
            if( $@ =~ /^Can't call method "isa" on unblessed reference/ ) {
                if( !UNIVERSAL::isa($object, $class) ) {
                    my $ref = ref $object;
                    $diag = "$obj_name isn't a '$class' it's a '$ref'";
                }
            } else {
                die <<WHOA;
WHOA! I tried to call ->isa on your object and got some weird error.
This should never happen.  Please contact the author immediately.
Here's the error.
$@
WHOA
            }
        }
        elsif( !$rslt ) {
            my $ref = ref $object;
            $diag = "$obj_name isn't a '$class' it's a '$ref'";
        }
    }
 
    _ok( !$diag, _where(), $name );
}
 
# Set a watchdog to timeout the entire test file
# NOTE:  If the test file uses 'threads', then call the watchdog() function
#        _AFTER_ the 'threads' module is loaded.
sub watchdog ($)
{
    my $timeout = shift;
    my $timeout_msg = 'Test process timed out - terminating';
 
    my $pid_to_kill = $$;   # PID for this process
 
    # Don't use a watchdog process if 'threads' is loaded -
    #   use a watchdog thread instead
    if (! $threads::threads) {
 
        # On Windows and VMS, try launching a watchdog process
        #   using system(1, ...) (see perlport.pod)
        if (($^O eq 'MSWin32') || ($^O eq 'VMS')) {
            # On Windows, try to get the 'real' PID
            if ($^O eq 'MSWin32') {
                eval { require Win32; };
                if (defined(&Win32::GetCurrentProcessId)) {
                    $pid_to_kill = Win32::GetCurrentProcessId();
                }
            }
 
            # If we still have a fake PID, we can't use this method at all
            return if ($pid_to_kill <= 0);
 
            # Launch watchdog process
            my $watchdog;
            eval {
                local $SIG{'__WARN__'} = sub {
                    _diag("Watchdog warning: $_[0]");
                };
                my $sig = $^O eq 'VMS' ? 'TERM' : 'KILL';
                $watchdog = system(1, which_perl(), '-e',
                                                    "sleep($timeout);" .
                                                    "warn('# $timeout_msg\n');" .
                                                    "kill($sig, $pid_to_kill);");
            };
            if ($@ || ($watchdog <= 0)) {
                _diag('Failed to start watchdog');
                _diag($@) if $@;
                undef($watchdog);
                return;
            }
 
            # Add END block to parent to terminate and
            #   clean up watchdog process
            eval "END { local \$! = 0; local \$? = 0;
                        wait() if kill('KILL', $watchdog); };";
            return;
        }
 
        # Try using fork() to generate a watchdog process
        my $watchdog;
        eval { $watchdog = fork() };
        if (defined($watchdog)) {
            if ($watchdog) {   # Parent process
                # Add END block to parent to terminate and
                #   clean up watchdog process
                eval "END { local \$! = 0; local \$? = 0;
                            wait() if kill('KILL', $watchdog); };";
                return;
            }
 
            ### Watchdog process code
 
            # Load POSIX if available
            eval { require POSIX; };
 
            # Execute the timeout
            sleep($timeout - 2) if ($timeout > 2);   # Workaround for perlbug #49073
            sleep(2);
 
            # Kill test process if still running
            if (kill(0, $pid_to_kill)) {
                _diag($timeout_msg);
                kill('KILL', $pid_to_kill);
            }
 
            # Don't execute END block (added at beginning of this file)
            $NO_ENDING = 1;
 
            # Terminate ourself (i.e., the watchdog)
            POSIX::_exit(1) if (defined(&POSIX::_exit));
            exit(1);
        }
 
        # fork() failed - fall through and try using a thread
    }
 
    # Use a watchdog thread because either 'threads' is loaded,
    #   or fork() failed
    if (eval 'require threads; 1') {
        threads->create(sub {
                # Load POSIX if available
                eval { require POSIX; };
 
                # Execute the timeout
                my $time_left = $timeout;
                do {
                    $time_left -= sleep($time_left);
                } while ($time_left > 0);
 
                # Kill the parent (and ourself)
                select(STDERR); $| = 1;
                _diag($timeout_msg);
                POSIX::_exit(1) if (defined(&POSIX::_exit));
                my $sig = $^O eq 'VMS' ? 'TERM' : 'KILL';
                kill($sig, $pid_to_kill);
            })->detach();
        return;
    }
 
    # If everything above fails, then just use an alarm timeout
    if (eval { alarm($timeout); 1; }) {
        # Load POSIX if available
        eval { require POSIX; };
 
        # Alarm handler will do the actual 'killing'
        $SIG{'ALRM'} = sub {
            select(STDERR); $| = 1;
            _diag($timeout_msg);
            POSIX::_exit(1) if (defined(&POSIX::_exit));
            my $sig = $^O eq 'VMS' ? 'TERM' : 'KILL';
            kill($sig, $pid_to_kill);
        };
    }
}
 
1;
-- threads.h --
#ifndef _THREADS_H_
#define _THREADS_H_

/* Needed for 5.8.0 */
#ifndef CLONEf_JOIN_IN
#  define CLONEf_JOIN_IN        8
#endif
#ifndef SAVEBOOL
#  define SAVEBOOL(a)
#endif

/* Added in 5.11.x */
#ifndef G_WANT
#  define G_WANT                (128|1)
#endif

/* Added in 5.24.x */
#ifndef PERL_TSA_RELEASE
#  define PERL_TSA_RELEASE(x)
#endif
#ifndef PERL_TSA_EXCLUDES
#  define PERL_TSA_EXCLUDES(x)
#endif
#ifndef CLANG_DIAG_IGNORE
#  define CLANG_DIAG_IGNORE(x)
#endif
#ifndef CLANG_DIAG_RESTORE
#  define CLANG_DIAG_RESTORE
#endif

/* Added in 5.38 */
#ifndef PERL_SRAND_OVERRIDE_NEXT_PARENT
#  define PERL_SRAND_OVERRIDE_NEXT_PARENT()
#endif

#endif
-- shared.h --
#ifndef _SHARED_H_
#define _SHARED_H_

#include "ppport.h"

#ifndef HvNAME_get
#  define HvNAME_get(hv)        (0 + ((XPVHV*)SvANY(hv))->xhv_name)
#endif

#endif
