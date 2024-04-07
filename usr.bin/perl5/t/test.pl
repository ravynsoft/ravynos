#
# t/test.pl - most of Test::More functionality without the fuss


# NOTE:
#
# Do not rely on features found only in more modern Perls here, as some CPAN
# distributions copy this file and must operate on older Perls. Similarly, keep
# things, simple as this may be run under fairly broken circumstances. For
# example, increment ($x++) has a certain amount of cleverness for things like
#
#   $x = 'zz';
#   $x++; # $x eq 'aaa';
#
# This stands more chance of breaking than just a simple
#
#   $x = $x + 1
#
# In this file, we use the latter "Baby Perl" approach, and increment
# will be worked over by t/op/inc.t

$| = 1;
our $Level = 1;
my $test = 1;
my $planned;
my $noplan;
my $Perl;       # Safer version of $^X set by which_perl()

# This defines ASCII/UTF-8 vs EBCDIC/UTF-EBCDIC
$::IS_ASCII  = ord 'A' ==  65;
$::IS_EBCDIC = ord 'A' == 193;

# This is 'our' to enable harness to account for TODO-ed tests in
# overall grade of PASS or FAIL
our $TODO = 0;
our $NO_ENDING = 0;
our $Tests_Are_Passing = 1;

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
	$plan{skip_all} and skip_all($plan{skip_all});
	$n = $plan{tests};
    }
    _print "1..$n\n" unless $noplan;
    $planned = $n;
}


# Set the plan at the end.  See Test::More::done_testing.
sub done_testing {
    my $n = $test - 1;
    $n = shift if @_;

    _print "1..$n\n";
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

sub _diag {
    return unless @_;
    my @mess = _comment(@_);
    $TODO ? _print(@mess) : _print_stderr(@mess);
}

# Use this instead of "print STDERR" when outputting failure diagnostic
# messages
sub diag {
    _diag(@_);
}

# Use this instead of "print" when outputting informational messages
sub note {
    return unless @_;
    _print( _comment(@_) );
}

sub is_miniperl {
    return !defined &DynaLoader::boot_DynaLoader;
}

sub set_up_inc {
    # Don’t clobber @INC under miniperl
    @INC = () unless is_miniperl;
    unshift @INC, @_;
}

sub _comment {
    return map { /^#/ ? "$_\n" : "# $_\n" }
           map { split /\n/ } @_;
}

sub _have_dynamic_extension {
    my $extension = shift;
    unless (eval {require Config; 1}) {
	warn "test.pl had problems loading Config: $@";
	return 1;
    }
    $extension =~ s!::!/!g;
    return 1 if ($Config::Config{extensions} =~ /\b$extension\b/);
}

sub skip_all {
    if (@_) {
        _print "1..0 # Skip @_\n";
    } else {
	_print "1..0\n";
    }
    exit(0);
}

sub skip_all_if_miniperl {
    skip_all(@_) if is_miniperl();
}

sub skip_all_without_dynamic_extension {
    my ($extension) = @_;
    skip_all("no dynamic loading on miniperl, no $extension") if is_miniperl();
    return if &_have_dynamic_extension;
    skip_all("$extension was not built");
}

sub skip_all_without_perlio {
    skip_all('no PerlIO') unless PerlIO::Layer->find('perlio');
}

sub skip_all_without_config {
    unless (eval {require Config; 1}) {
	warn "test.pl had problems loading Config: $@";
	return;
    }
    foreach (@_) {
	next if $Config::Config{$_};
	my $key = $_; # Need to copy, before trying to modify.
	$key =~ s/^use//;
	$key =~ s/^d_//;
	skip_all("no $key");
    }
}

sub skip_all_without_unicode_tables { # (but only under miniperl)
    if (is_miniperl()) {
        skip_all_if_miniperl("Unicode tables not built yet")
            unless eval 'require "unicore/UCD.pl"';
    }
}

sub find_git_or_skip {
    my ($source_dir, $reason);

    if ( $ENV{CONTINUOUS_INTEGRATION} && $ENV{WORKSPACE} ) {
        $source_dir = $ENV{WORKSPACE};
        if ( -d "${source_dir}/.git" ) {
            $ENV{GIT_DIR} = "${source_dir}/.git";
            return $source_dir;
        }
    }

    if (-d '.git') {
	$source_dir = '.';
    } elsif (-l 'MANIFEST' && -l 'AUTHORS') {
	my $where = readlink 'MANIFEST';
	die "Can't readlink MANIFEST: $!" unless defined $where;
	die "Confusing symlink target for MANIFEST, '$where'"
	    unless $where =~ s!/MANIFEST\z!!;
	if (-d "$where/.git") {
	    # Looks like we are in a symlink tree
	    if (exists $ENV{GIT_DIR}) {
		diag("Found source tree at $where, but \$ENV{GIT_DIR} is $ENV{GIT_DIR}. Not changing it");
	    } else {
		note("Found source tree at $where, setting \$ENV{GIT_DIR}");
		$ENV{GIT_DIR} = "$where/.git";
	    }
	    $source_dir = $where;
	}
    } elsif (exists $ENV{GIT_DIR} || -f '.git') {
	my $commit = '8d063cd8450e59ea1c611a2f4f5a21059a2804f1';
	my $out = `git rev-parse --verify --quiet '$commit^{commit}'`;
	chomp $out;
	if($out eq $commit) {
	    $source_dir = '.'
	}
    }
    if ($ENV{'PERL_BUILD_PACKAGING'}) {
	$reason = 'PERL_BUILD_PACKAGING is set';
    } elsif ($source_dir) {
	my $version_string = `git --version`;
	if (defined $version_string
	      && $version_string =~ /\Agit version (\d+\.\d+\.\d+)(.*)/) {
	    return $source_dir if eval "v$1 ge v1.5.0";
	    # If you have earlier than 1.5.0 and it works, change this test
	    $reason = "in git checkout, but git version '$1$2' too old";
	} else {
	    $reason = "in git checkout, but cannot run git";
	}
    } else {
	$reason = 'not being run from a git checkout';
    }
    skip_all($reason) if $_[0] && $_[0] eq 'all';
    skip($reason, @_);
}

sub BAIL_OUT {
    my ($reason) = @_;
    _print("Bail out!  $reason\n");
    exit 255;
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
	$out = $pass ? "ok $test - [$where]" : "not ok $test - [$where]";
    }

    if ($TODO) {
	$out = $out . " # TODO $TODO";
    } else {
	$Tests_Are_Passing = 0 unless $pass;
    }

    _print "$out\n";

    if ($pass) {
	note @mess; # Ensure that the message is properly escaped.
    }
    else {
	my $msg = "# Failed test $test - ";
	$msg.= "$name " if $name;
	$msg .= "$where\n";
	_diag $msg;
	_diag @mess;
    }

    $test = $test + 1; # don't use ++

    return $pass;
}

sub _where {
    my (undef, $filename, $lineno) = caller($Level);
    return "at $filename line $lineno";
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

# Support pre-5.10 Perls, for the benefit of CPAN dists that copy this file.
# Note that chr(90) exists in both ASCII ("Z") and EBCDIC ("!").
my $chars_template = defined(eval { pack "W*", 90 }) ? "W*" : "U*";
eval 'sub re::is_regexp { ref($_[0]) eq "Regexp" }'
    if !defined &re::is_regexp;

# keys are the codes \n etc map to, values are 2 char strings such as \n
my %backslash_escape;
foreach my $x (split //, 'enrtfa\\\'"') {
    $backslash_escape{ord eval "\"\\$x\""} = "\\$x";
}
# A way to display scalars containing control characters and Unicode.
# Trying to avoid setting $_, or relying on local $_ to work.
sub display {
    my @result;
    foreach my $x (@_) {
        if (defined $x and not ref $x) {
            my $y = '';
            foreach my $c (unpack($chars_template, $x)) {
                if ($c > 255) {
                    $y = $y . sprintf "\\x{%x}", $c;
                } elsif ($backslash_escape{$c}) {
                    $y = $y . $backslash_escape{$c};
                } elsif ($c < ord " ") {
                    # Use octal for characters with small ordinals that are
                    # traditionally expressed as octal: the controls below
                    # space, which on EBCDIC are almost all the controls, but
                    # on ASCII don't include DEL nor the C1 controls.
                    $y = $y . sprintf "\\%03o", $c;
                } elsif (chr $c =~ /[[:print:]]/a) {
                    $y = $y . chr $c;
                }
                else {
                    $y = $y . sprintf "\\x%02X", $c;
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
	unshift(@mess, "#      got "._qq($got)."\n",
		       "# expected "._qq($expected)."\n");
        if (defined $got and defined $expected and
            (length($got)>20 or length($expected)>20))
        {
            my $p = 0;
            $p++ while substr($got,$p,1) eq substr($expected,$p,1);
            push @mess,"#  diff at $p\n";
            push @mess,"#    after "._qq(substr($got,$p-40<0 ? 0 : $p-40,40))."\n";
            push @mess,"#     have "._qq(substr($got,$p,40))."\n";
            push @mess,"#     want "._qq(substr($expected,$p,40))."\n";
        }
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
        unshift(@mess, "# it should not be "._qq($got)."\n",
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
        # that stringify to the same thing but are actually numerically
        # different. Display the numbers if $type isn't a string operator,
        # and the numbers are stringwise the same.
        # (all string operators have alphabetic names, so tr/a-z// is true)
        # This will also show numbers for some unneeded cases, but will
        # definitely be helpful for things such as == and <= that fail
        if ($got eq $expected and $type !~ tr/a-z//) {
            unshift @mess, "# $got - $expected = " . ($got - $expected) . "\n";
        }
        unshift(@mess, "#      got "._qq($got)."\n",
                       "# expected $type "._qq($expected)."\n");
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
	unshift@mess, "#      got "._qq($got)."\n",
		      "# expected "._qq($expected)." (within "._qq($range).")\n";
    }
    _ok($pass, _where(), $name, @mess);
}

# Note: this isn't quite as fancy as Test::More::like().

sub like   ($$@) { like_yn (0,@_) }; # 0 for -
sub unlike ($$@) { like_yn (1,@_) }; # 1 for un-

sub like_yn ($$$@) {
    my ($flip, undef, $expected, $name, @mess) = @_;

    # We just accept like(..., qr/.../), not like(..., '...'), and
    # definitely not like(..., '/.../') like
    # Test::Builder::maybe_regex() does.
    unless (re::is_regexp($expected)) {
	die "PANIC: The value '$expected' isn't a regexp. The like() function needs a qr// pattern, not a string";
    }

    my $pass;
    $pass = $_[1] =~ /$expected/ if !$flip;
    $pass = $_[1] !~ /$expected/ if $flip;
    my $display_got = $_[1];
    $display_got = display($display_got);
    my $display_expected = $expected;
    $display_expected = display($display_expected);
    unless ($pass) {
	unshift(@mess, "#      got '$display_got'\n",
		$flip
		? "# expected !~ /$display_expected/\n"
                : "# expected /$display_expected/\n");
    }
    local $Level = $Level + 1;
    _ok($pass, _where(), $name, @mess);
}

sub refcount_is {
    # Don't unpack first arg; access it directly via $_[0] to avoid creating
    # another reference and upsetting the refcount
    my (undef, $expected, $name, @mess) = @_;
    my $got = &Internals::SvREFCNT($_[0]) + 1; # +1 to account for the & calling style
    my $pass = $got == $expected;
    unless ($pass) {
        unshift @mess, "#      got $got references\n" .
                       "# expected $expected\n";
    }
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
    my $n   = @_ ? shift : 1;
    my $bad_swap;
    my $both_zero;
    {
      local $^W = 0;
      $bad_swap = $why > 0 && $n == 0;
      $both_zero = $why == 0 && $n == 0;
    }
    if ($bad_swap || $both_zero || @_) {
      my $arg = "'$why', '$n'";
      if (@_) {
        $arg .= join(", ", '', map { qq['$_'] } @_);
      }
      die qq[$0: expected skip(why, count), got skip($arg)\n];
    }
    for (1..$n) {
        _print "ok $test # skip $why\n";
        $test = $test + 1;
    }
    local $^W = 0;
    last SKIP;
}

sub skip_if_miniperl {
    skip(@_) if is_miniperl();
}

sub skip_without_dynamic_extension {
    my $extension = shift;
    skip("no dynamic loading on miniperl, no extension $extension", @_)
	if is_miniperl();
    return if &_have_dynamic_extension($extension);
    skip("extension $extension was not built", @_);
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
      if (
        defined $orig->{$key} != defined $value
        || (defined $value && $orig->{$key} ne $value)
      ) {
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

# We only provide a subset of the Test::More functionality.
sub require_ok ($) {
    my ($require) = @_;
    if ($require =~ tr/[A-Za-z0-9:.]//c) {
	fail("Invalid character in \"$require\", passed to require_ok");
    } else {
	eval <<REQUIRE_OK;
require $require;
REQUIRE_OK
	is($@, '', _where(), "require $require");
    }
}

sub use_ok ($) {
    my ($use) = @_;
    if ($use =~ tr/[A-Za-z0-9:.]//c) {
	fail("Invalid character in \"$use\", passed to use");
    } else {
	eval <<USE_OK;
use $use;
USE_OK
	is($@, '', _where(), "use $use");
    }
}

# runperl, run_perl - Runs a separate perl interpreter and returns its output.
# Arguments :
#   switches => [ command-line switches ]
#   nolib    => 1 # don't use -I../lib (included by default)
#   non_portable => Don't warn if a one liner contains quotes
#   prog     => one-liner (avoid quotes)
#   progs    => [ multi-liner (avoid quotes) ]
#   progfile => perl script
#   stdin    => string to feed the stdin (or undef to redirect from /dev/null)
#   stderr   => If 'devnull' suppresses stderr, if other TRUE value redirect
#               stderr to stdout
#   args     => [ command-line arguments to the perl program ]
#   verbose  => print the command line

my $is_mswin    = $^O eq 'MSWin32';
my $is_vms      = $^O eq 'VMS';
my $is_cygwin   = $^O eq 'cygwin';

sub _quote_args {
    my ($runperl, $args) = @_;

    foreach (@$args) {
	# In VMS protect with doublequotes because otherwise
	# DCL will lowercase -- unless already doublequoted.
       $_ = q(").$_.q(") if $is_vms && !/^\"/ && length($_) > 0;
       $runperl = $runperl . ' ' . $_;
    }
    return $runperl;
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
	$runperl = $runperl . ' "-I../lib" "-I." '; # doublequotes because of VMS
    }
    if ($args{switches}) {
	local $Level = 2;
	die "test.pl:runperl(): 'switches' must be an ARRAYREF " . _where()
	    unless ref $args{switches} eq "ARRAY";
	$runperl = _quote_args($runperl, $args{switches});
    }
    if (defined $args{prog}) {
	die "test.pl:runperl(): both 'prog' and 'progs' cannot be used " . _where()
	    if defined $args{progs};
        $args{progs} = [split /\n/, $args{prog}, -1]
    }
    if (defined $args{progs}) {
	die "test.pl:runperl(): 'progs' must be an ARRAYREF " . _where()
	    unless ref $args{progs} eq "ARRAY";
        foreach my $prog (@{$args{progs}}) {
	    if (!$args{non_portable}) {
		if ($prog =~ tr/'"//) {
		    warn "quotes in prog >>$prog<< are not portable";
		}
		if ($prog =~ /^([<>|]|2>)/) {
		    warn "Initial $1 in prog >>$prog<< is not portable";
		}
		if ($prog =~ /&\z/) {
		    warn "Trailing & in prog >>$prog<< is not portable";
		}
	    }
            if ($is_mswin || $is_vms) {
                $runperl = $runperl . qq ( -e "$prog" );
            }
            else {
                $runperl = $runperl . qq ( -e '$prog' );
            }
        }
    } elsif (defined $args{progfile}) {
	$runperl = $runperl . qq( "$args{progfile}");
    } else {
	# You probably didn't want to be sucking in from the upstream stdin
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

	if ($is_mswin || $is_vms) {
	    $runperl = qq{$Perl -e "print qq(} .
		$args{stdin} . q{)" | } . $runperl;
	}
	else {
	    $runperl = qq{$Perl -e 'print qq(} .
		$args{stdin} . q{)' | } . $runperl;
	}
    } elsif (exists $args{stdin}) {
        # Using the pipe construction above can cause fun on systems which use
        # ksh as /bin/sh, as ksh does pipes differently (with one less process)
        # With sh, for the command line 'perl -e 'print qq()' | perl -e ...'
        # the sh process forks two children, which use exec to start the two
        # perl processes. The parent shell process persists for the duration of
        # the pipeline, and the second perl process starts with no children.
        # With ksh (and zsh), the shell saves a process by forking a child for
        # just the first perl process, and execing itself to start the second.
        # This means that the second perl process starts with one child which
        # it didn't create. This causes "fun" when if the tests assume that
        # wait (or waitpid) will only return information about processes
        # started within the test.
        # They also cause fun on VMS, where the pipe implementation returns
        # the exit code of the process at the front of the pipeline, not the
        # end. This messes up any test using OPTION FATAL.
        # Hence it's useful to have a way to make STDIN be at eof without
        # needing a pipeline, so that the fork tests have a sane environment
        # without these surprises.

        # /dev/null appears to be surprisingly portable.
        $runperl = $runperl . ($is_mswin ? ' <nul' : ' </dev/null');
    }
    if (defined $args{args}) {
	$runperl = _quote_args($runperl, $args{args});
    }
    if (exists $args{stderr} && $args{stderr} eq 'devnull') {
        $runperl = $runperl . ($is_mswin ? ' 2>nul' : ' 2>/dev/null');
    }
    elsif ($args{stderr}) {
        $runperl = $runperl . ' 2>&1';
    }
    if ($args{verbose}) {
	my $runperldisplay = $runperl;
	$runperldisplay =~ s/\n/\n\#/g;
	_print_stderr "# $runperldisplay\n";
    }
    return $runperl;
}

# usage:
#  $ENV{PATH} =~ /(.*)/s;
#  local $ENV{PATH} = untaint_path($1);
sub untaint_path {
    my $path = shift;
    my $sep;

    if (! eval {require Config; 1}) {
        warn "test.pl had problems loading Config: $@";
        $sep = ':';
    } else {
        $sep = $Config::Config{path_sep};
    }

    $path =
        join $sep, grep { $_ ne "" and $_ ne "." and -d $_ and
              ($is_mswin or $is_vms or !(stat && (stat _)[2]&0022)) }
        split quotemeta ($sep), $1;
    if ($is_cygwin) {   # Must have /bin under Cygwin
        if (length $path) {
            $path = $path . $sep;
        }
        $path = $path . '/bin';
    } elsif (!$is_vms and !length $path) {
        # empty PATH is the same as a path of "." on *nix so to prevent
        # tests from dieing under taint we need to return something
        # absolute. Perhaps "/" would be better? Anything absolute will do.
        $path = "/usr/bin";
    }

    $path;
}

# sub run_perl {} is alias to below
# Since this uses backticks to run, it is subject to the rules of the shell.
# Locale settings may pose a problem, depending on the program being run.
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
	my @keys = grep {exists $ENV{$_}} qw(CDPATH IFS ENV BASH_ENV);
	local @ENV{@keys} = ();
	# Untaint, plus take out . and empty string:
	local $ENV{'DCL$PATH'} = $1 if $is_vms && exists($ENV{'DCL$PATH'}) && ($ENV{'DCL$PATH'} =~ /(.*)/s);
        $ENV{PATH} =~ /(.*)/s;
        local $ENV{PATH} = untaint_path($1);
	$runperl =~ /(.*)/s;
	$runperl = $1;

	$result = `$runperl`;
    } else {
	$result = `$runperl`;
    }
    $result =~ s/\n\n/\n/g if $is_vms; # XXX pipes sometimes double these
    return $result;
}

# Nice alias
*run_perl = *run_perl = \&runperl; # shut up "used only once" warning

# Run perl with specified environment and arguments, return (STDOUT, STDERR)
# set DEBUG_RUNENV=1 in the environment to debug.
sub runperl_and_capture {
  my ($env, $args) = @_;

  my $STDOUT = tempfile();
  my $STDERR = tempfile();
  my $PERL   = $^X;
  my $FAILURE_CODE = 119;

  local %ENV = %ENV;
  delete $ENV{PERLLIB};
  delete $ENV{PERL5LIB};
  delete $ENV{PERL5OPT};
  delete $ENV{PERL_USE_UNSAFE_INC};
  my $pid = fork;
  return (0, "Couldn't fork: $!") unless defined $pid;   # failure
  if ($pid) {                   # parent
    waitpid $pid,0;
    my $exit_code = $? ? $? >> 8 : 0;
    my ($out, $err)= ("", "");
    local $/;
    if (open my $stdout, '<', $STDOUT) {
        $out .= <$stdout>;
    } else {
        $err .= "Could not read STDOUT '$STDOUT' file: $!\n";
    }
    if (open my $stderr, '<', $STDERR) {
        $err .= <$stderr>;
    } else {
        $err .= "Could not read STDERR '$STDERR' file: $!\n";
    }
    if ($exit_code == $FAILURE_CODE) {
        $err .= "Something went wrong. Received FAILURE_CODE as exit code.\n";
    }
    if ($ENV{DEBUG_RUNENV}) {
        print "OUT: $out\n";
        print "ERR: $err\n";
    }
    return ($out, $err);
  } elsif (defined $pid) {                      # child
    # Just in case the order we update the environment changes how
    # the environment is set up we sort the keys here for consistency.
    for my $k (sort keys %$env) {
      $ENV{$k} = $env->{$k};
    }
    if ($ENV{DEBUG_RUNENV}) {
        print "Child Process $$ Executing:\n$PERL @$args\n";
    }
    open STDOUT, '>', $STDOUT
        or do {
            print "Failed to dup STDOUT to '$STDOUT': $!";
            exit $FAILURE_CODE;
        };
    open STDERR, '>', $STDERR
        or do {
            print "Failed to dup STDERR to '$STDERR': $!";
            exit $FAILURE_CODE;
        };
    exec $PERL, @$args
        or print STDERR "Failed to exec: ",
                  join(" ",map { "'$_'" } $^X, @$args),
                  ": $!\n";
    exit $FAILURE_CODE;
  }
}

sub DIE {
    _print_stderr "# @_\n";
    exit 1;
}

# A somewhat safer version of the sometimes wrong $^X.
sub which_perl {
    unless (defined $Perl) {
	$Perl = $^X;

	# VMS should have 'perl' aliased properly
	return $Perl if $is_vms;

	my $exe;
	if (! eval {require Config; 1}) {
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
	    if (! eval {require File::Spec; 1}) {
		warn "test.pl had problems loading File::Spec: $@";
		$Perl = "./$perl";
	    } else {
		$Perl = File::Spec->catfile(File::Spec->curdir(), $perl);
	    }
	}

	# Build up the name of the executable file from the name of
	# the command.

	if ($Perl !~ /\Q$exe\E$/i) {
	    $Perl = $Perl . $exe;
	}

	warn "which_perl: cannot find $Perl from $^X" unless -f $Perl;

	# For subcommands to use.
	$ENV{PERLEXE} = $Perl;
    }
    return $Perl;
}

sub unlink_all {
    my $count = 0;
    foreach my $file (@_) {
        1 while unlink $file;
	if( -f $file ){
	    _print_stderr "# Couldn't unlink '$file': $!\n";
	}else{
	    $count = $count + 1; # don't use ++
	}
    }
    $count;
}

# _num_to_alpha - Returns a string of letters representing a positive integer.
# Arguments :
#   number to convert
#   maximum number of letters

# returns undef if the number is negative
# returns undef if the number of letters is greater than the maximum wanted

# _num_to_alpha( 0) eq 'A';
# _num_to_alpha( 1) eq 'B';
# _num_to_alpha(25) eq 'Z';
# _num_to_alpha(26) eq 'AA';
# _num_to_alpha(27) eq 'AB';

my @letters = qw(A B C D E F G H I J K L M N O P Q R S T U V W X Y Z);

# Avoid ++ -- ranges split negative numbers
sub _num_to_alpha {
    my($num,$max_char) = @_;
    return unless $num >= 0;
    my $alpha = '';
    my $char_count = 0;
    $max_char = 0 if !defined($max_char) or $max_char < 0;

    while( 1 ){
        $alpha = $letters[ $num % @letters ] . $alpha;
        $num = int( $num / @letters );
        last if $num == 0;
        $num = $num - 1;

        # char limit
        next unless $max_char;
        $char_count = $char_count + 1;
        return if $char_count == $max_char;
    }
    return $alpha;
}

my %tmpfiles;
sub unlink_tempfiles {
    unlink_all keys %tmpfiles;
    %tempfiles = ();
}

END { unlink_tempfiles(); }


# NOTE: tempfile() may be used as a module names in our tests
# so the result must be restricted to only legal characters for a module
# name.

# A regexp that matches the tempfile names
$::tempfile_regexp = 'tmp_[A-Z]+_[A-Z]+';

# Avoid ++, avoid ranges, avoid split //
my $tempfile_count = 0;
my $max_file_chars = 3;
# Note that the max number of is NOT 26**3, it is 26**3 + 26**2 + 26,
# as 3 character files are distinct from 2 character files, from 1 characters
# files, etc.
sub tempfile {
    # if you change the format returned by tempfile() you MUST change
    # the $::tempfile_regex define above.
    my $try_prefix = (-d "t" ? "t/" : "")."tmp_"._num_to_alpha($$);
    while (1) {
        my $alpha = _num_to_alpha($tempfile_count,$max_file_chars);
        last unless defined $alpha;
        my $try = $try_prefix . "_" . $alpha;
        $tempfile_count = $tempfile_count + 1;

        # Need to note all the file names we allocated, as a second request
        # may come before the first is created. Also we are avoiding ++ here
        # so we aren't using the normal idiom for this kind of test.
	if (!$tmpfiles{$try} && !-e $try) {
	    # We have a winner
	    $tmpfiles{$try} = 1;
	    return $try;
	}
    }
    die sprintf
        'panic: Too many tempfile()s with prefix "%s", limit of %d reached',
        $try_prefix, 26 ** $max_file_chars;
}

# register_tempfile - Adds a list of files to be removed at the end of the current test file
# Arguments :
#   a list of files to be removed later

# returns a count of how many file names were actually added

# Reuses %tmpfiles so that tempfile() will also skip any files added here
# even if the file doesn't exist yet.

sub register_tempfile {
    my $count = 0;
    for( @_ ){
	if( $tmpfiles{$_} ){
	    _print_stderr "# Temporary file '$_' already added\n";
	}else{
	    $tmpfiles{$_} = 1;
	    $count = $count + 1;
	}
    }
    return $count;
}

# This is the temporary file for fresh_perl
my $tmpfile = tempfile();

sub fresh_perl {
    my($prog, $runperl_args) = @_;

    # Run 'runperl' with the complete perl program contained in '$prog', and
    # arguments in the hash referred to by '$runperl_args'.  The results are
    # returned, with $? set to the exit code.  Unless overridden, stderr is
    # redirected to stdout.
    #
    # Placing the program in a file bypasses various sh vagaries

    die sprintf "Second argument to fresh_perl_.* must be hashref of args to fresh_perl (or {})"
        unless !(defined $runperl_args) || ref($runperl_args) eq 'HASH';

    # Given the choice of the mis-parsable {}
    # (we want an anon hash, but a borked lexer might think that it's a block)
    # or relying on taking a reference to a lexical
    # (\ might be mis-parsed, and the reference counting on the pad may go
    #  awry)
    # it feels like the least-worse thing is to assume that auto-vivification
    # works. At least, this is only going to be a run-time failure, so won't
    # affect tests using this file but not this function.
    my $trim= delete $runperl_args->{rtrim_result}; # hide from runperl
    $runperl_args->{progfile} ||= $tmpfile;
    $runperl_args->{stderr}     = 1 unless exists $runperl_args->{stderr};

    open TEST, '>', $tmpfile or die "Cannot open $tmpfile: $!";
    binmode TEST, ':utf8' if $runperl_args->{wide_chars};
    print TEST $prog;
    close TEST or die "Cannot close $tmpfile: $!";

    my $results = runperl(%$runperl_args);
    my $status = $?;    # Not necessary to save this, but it makes it clear to
                        # future maintainers.
    $results=~s/[ \t]+\n/\n/g if $trim;
    # Clean up the results into something a bit more predictable.
    $results  =~ s/\n+$//;
    $results =~ s/at\s+$::tempfile_regexp\s+line/at - line/g;
    $results =~ s/of\s+$::tempfile_regexp\s+aborted/of - aborted/g;

    # bison says 'parse error' instead of 'syntax error',
    # various yaccs may or may not capitalize 'syntax'.
    $results =~ s/^(syntax|parse) error/syntax error/mig;

    if ($is_vms) {
        # some tests will trigger VMS messages that won't be expected
        $results =~ s/\n?%[A-Z]+-[SIWEF]-[A-Z]+,.*//;

        # pipes double these sometimes
        $results =~ s/\n\n/\n/g;
    }

    $? = $status;
    return $results;
}


sub _fresh_perl {
    my($prog, $action, $expect, $runperl_args, $name) = @_;

    local $Level = $Level + 1;

    # strip trailing whitespace if requested - makes some tests easier
    $expect=~s/[[:blank:]]+\n/\n/g if $runperl_args->{rtrim_result};

    my $results = fresh_perl($prog, $runperl_args);
    my $status = $?;

    # Use the first line of the program as a name if none was given
    unless( $name ) {
        (my $first_line, $name) = $prog =~ /^((.{1,50}).*)/;
        $name = $name . '...' if length $first_line > length $name;
    }

    # Historically this was implemented using a closure, but then that means
    # that the tests for closures avoid using this code. Given that there
    # are exactly two callers, doing exactly two things, the simpler approach
    # feels like a better trade off.
    my $pass;
    if ($action eq 'eq') {
	$pass = is($results, $expect, $name);
    } elsif ($action eq '=~') {
	$pass = like($results, $expect, $name);
    } else {
	die "_fresh_perl can't process action '$action'";
    }
	
    unless ($pass) {
        _diag "# PROG: \n$prog\n";
        _diag "# STATUS: $status\n";
    }

    return $pass;
}

#
# fresh_perl_is
#
# Combination of run_perl() and is().
#

sub fresh_perl_is {
    my($prog, $expected, $runperl_args, $name) = @_;

    # _fresh_perl() is going to clip the trailing newlines off the result.
    # This will make it so the test author doesn't have to know that.
    $expected =~ s/\n+$//;

    local $Level = $Level + 1;
    _fresh_perl($prog, 'eq', $expected, $runperl_args, $name);
}

#
# fresh_perl_like
#
# Combination of run_perl() and like().
#

sub fresh_perl_like {
    my($prog, $expected, $runperl_args, $name) = @_;
    local $Level = $Level + 1;
    _fresh_perl($prog, '=~', $expected, $runperl_args, $name);
}

# Many tests use the same format in __DATA__ or external files to specify a
# sequence of (fresh) tests to run, extra files they may temporarily need, and
# what the expected output is.  Putting it here allows common code to serve
# these multiple tests.
#
# Each program is source code to run followed by an "EXPECT" line, followed
# by the expected output.
#
# The first line of the code to run may be a command line switch such as -wE
# or -0777 (alphanumerics only; only one cluster, beginning with a minus is
# allowed).  Later lines may contain (note the '# ' on each):
#   # TODO reason for todo
#   # SKIP reason for skip
#   # SKIP ?code to test if this should be skipped
#   # NAME name of the test (as with ok($ok, $name))
#
# The expected output may contain:
#   OPTION list of options
#   OPTIONS list of options
#
# The possible options for OPTION may be:
#   regex - the expected output is a regular expression
#   random - all lines match but in any order
#   fatal - the code will fail fatally (croak, die)
#   nonfatal - the code is not expected to fail fatally
#
# If the actual output contains a line "SKIPPED" the test will be
# skipped.
#
# If the actual output contains a line "PREFIX", any output starting with that
# line will be ignored when comparing with the expected output
#
# If the global variable $FATAL is true then OPTION fatal is the
# default.

our $FATAL;
sub _setup_one_file {
    my $fh = shift;
    # Store the filename as a program that started at line 0.
    # Real files count lines starting at line 1.
    my @these = (0, shift);
    my ($lineno, $current);
    while (<$fh>) {
        if ($_ eq "########\n") {
            if (defined $current) {
                push @these, $lineno, $current;
            }
            undef $current;
        } else {
            if (!defined $current) {
                $lineno = $.;
            }
            $current .= $_;
        }
    }
    if (defined $current) {
        push @these, $lineno, $current;
    }
    ((scalar @these) / 2 - 1, @these);
}

sub setup_multiple_progs {
    my ($tests, @prgs);
    foreach my $file (@_) {
        next if $file =~ /(?:~|\.orig|,v)$/;
        next if $file =~ /perlio$/ && !PerlIO::Layer->find('perlio');
        next if -d $file;

        open my $fh, '<', $file or die "Cannot open $file: $!\n" ;
        my $found;
        while (<$fh>) {
            if (/^__END__/) {
                $found = $found + 1; # don't use ++
                last;
            }
        }
        # This is an internal error, and should never happen. All bar one of
        # the files had an __END__ marker to signal the end of their preamble,
        # although for some it wasn't technically necessary as they have no
        # tests. It might be possible to process files without an __END__ by
        # seeking back to the start and treating the whole file as tests, but
        # it's simpler and more reliable just to make the rule that all files
        # must have __END__ in. This should never fail - a file without an
        # __END__ should not have been checked in, because the regression tests
        # would not have passed.
        die "Could not find '__END__' in $file"
            unless $found;

        my ($t, @p) = _setup_one_file($fh, $file);
        $tests += $t;
        push @prgs, @p;

        close $fh
            or die "Cannot close $file: $!\n";
    }
    return ($tests, @prgs);
}

sub run_multiple_progs {
    my $up = shift;
    my @prgs;
    if ($up) {
	# The tests in lib run in a temporary subdirectory of t, and always
	# pass in a list of "programs" to run
	@prgs = @_;
    } else {
        # The tests below t run in t and pass in a file handle. In theory we
        # can pass (caller)[1] as the second argument to report errors with
        # the filename of our caller, as the handle is always DATA. However,
        # line numbers in DATA count from the __END__ token, so will be wrong.
        # Which is more confusing than not providing line numbers. So, for now,
        # don't provide line numbers. No obvious clean solution - one hack
        # would be to seek DATA back to the start and read to the __END__ token,
        # but that feels almost like we should just open $0 instead.

        # Not going to rely on undef in list assignment.
        my $dummy;
        ($dummy, @prgs) = _setup_one_file(shift);
    }
    my $taint_disabled;
    if (! eval {require Config; 1}) {
        warn "test.pl had problems loading Config: $@";
        $taint_disabled = '';
    } else {
        $taint_disabled = $Config::Config{taint_disabled};
    }

    my $tmpfile = tempfile();

    my $count_failures = 0;
    my ($file, $line);
  PROGRAM:
    while (defined ($line = shift @prgs)) {
        $_ = shift @prgs;
        unless ($line) {
            $file = $_;
            if (defined $file) {
                print "# From $file\n";
            }
	    next;
	}
	my $switch = "";
	my @temps ;
	my @temp_path;
	if (s/^(\s*-\w+)//) {
	    $switch = $1;
	}

        s/^# NOTE.*\n//mg; # remove any NOTE comments in the content

        # unhide conflict markers - we hide them so that naive
        # conflict marker detection logic doesn't get upset with our
        # tests.
        s/([<=>])CONFLICT\1/$1 x 7/ge;

	my ($prog, $expected) = split(/\nEXPECT(?:\n|$)/, $_, 2);

	my %reason;
	foreach my $what (qw(skip todo)) {
	    $prog =~ s/^#\s*\U$what\E\s*(.*)\n//m and $reason{$what} = $1;
	    # If the SKIP reason starts ? then it's taken as a code snippet to
	    # evaluate. This provides the flexibility to have conditional SKIPs
	    if ($reason{$what} && $reason{$what} =~ s/^\?//) {
		my $temp = eval $reason{$what};
		if ($@) {
		    die "# In \U$what\E code reason:\n# $reason{$what}\n$@";
		}
		$reason{$what} = $temp;
	    }
	}

    my $name = '';
    if ($prog =~ s/^#\s*NAME\s+(.+)\n//m) {
        $name = $1;
    } elsif (defined $file) {
        $name = "test from $file at line $line";
    }

        if ($switch=~/[Tt]/ and $taint_disabled eq "define") {
            $reason{skip} ||= "This perl does not support taint";
        }

	if ($reason{skip}) {
	SKIP:
	  {
	    skip($name ? "$name - $reason{skip}" : $reason{skip}, 1);
	  }
	  next PROGRAM;
	}

	if ($prog =~ /--FILE--/) {
	    my @files = split(/\n?--FILE--\s*([^\s\n]*)\s*\n/, $prog) ;
	    shift @files ;
	    die "Internal error: test $_ didn't split into pairs, got " .
		scalar(@files) . "[" . join("%%%%", @files) ."]\n"
		    if @files % 2;
	    while (@files > 2) {
		my $filename = shift @files;
		my $code = shift @files;
		push @temps, $filename;
		if ($filename =~ m#(.*)/# && $filename !~ m#^\.\./#) {
		    require File::Path;
		    File::Path::mkpath($1);
		    push(@temp_path, $1);
		}
		open my $fh, '>', $filename or die "Cannot open $filename: $!\n";
		print $fh $code;
		close $fh or die "Cannot close $filename: $!\n";
	    }
	    shift @files;
	    $prog = shift @files;
	}

	open my $fh, '>', $tmpfile or die "Cannot open >$tmpfile: $!";
	print $fh q{
        BEGIN {
            push @INC, '.';
            open STDERR, '>&', STDOUT
              or die "Can't dup STDOUT->STDERR: $!;";
        }
	};
	print $fh "\n#line 1\n";  # So the line numbers don't get messed up.
	print $fh $prog,"\n";
	close $fh or die "Cannot close $tmpfile: $!";
	my $results = runperl( stderr => 1, progfile => $tmpfile,
			       stdin => undef, $up
			       ? (switches => ["-I$up/lib", $switch], nolib => 1)
			       : (switches => [$switch])
			        );
	my $status = $?;
	$results =~ s/\n+$//;
	# allow expected output to be written as if $prog is on STDIN
	$results =~ s/$::tempfile_regexp/-/g;
	if ($^O eq 'VMS') {
	    # some tests will trigger VMS messages that won't be expected
	    $results =~ s/\n?%[A-Z]+-[SIWEF]-[A-Z]+,.*//;

	    # pipes double these sometimes
	    $results =~ s/\n\n/\n/g;
	}
	# bison says 'parse error' instead of 'syntax error',
	# various yaccs may or may not capitalize 'syntax'.
	$results =~ s/^(syntax|parse) error/syntax error/mig;
	# allow all tests to run when there are leaks
	$results =~ s/Scalars leaked: \d+\n//g;

	$expected =~ s/\n+$//;
	my $prefix = ($results =~ s#^PREFIX(\n|$)##) ;
	# any special options? (OPTIONS foo bar zap)
	my $option_regex = 0;
	my $option_random = 0;
	my $fatal = $FATAL;
	if ($expected =~ s/^OPTIONS? (.+)(?:\n|\Z)//) {
	    foreach my $option (split(' ', $1)) {
		if ($option eq 'regex') { # allow regular expressions
		    $option_regex = 1;
		}
		elsif ($option eq 'random') { # all lines match, but in any order
		    $option_random = 1;
		}
		elsif ($option eq 'fatal') { # perl should fail
		    $fatal = 1;
		}
                elsif ($option eq 'nonfatal') {
                    # used to turn off default fatal
                    $fatal = 0;
                }
		else {
		    die "$0: Unknown OPTION '$option'\n";
		}
	    }
	}
	die "$0: can't have OPTION regex and random\n"
	    if $option_regex + $option_random > 1;
	my $ok = 0;
	if ($results =~ s/^SKIPPED\n//) {
	    print "$results\n" ;
	    $ok = 1;
	}
	else {
	    if ($option_random) {
	        my @got = sort split "\n", $results;
	        my @expected = sort split "\n", $expected;

	        $ok = "@got" eq "@expected";
	    }
	    elsif ($option_regex) {
	        $ok = $results =~ /^$expected/;
	    }
	    elsif ($prefix) {
	        $ok = $results =~ /^\Q$expected/;
	    }
	    else {
	        $ok = $results eq $expected;
	    }

	    if ($ok && $fatal && !($status >> 8)) {
		$ok = 0;
	    }
	}

	local $::TODO = $reason{todo};

	unless ($ok) {
        my $err_line = '';
        $err_line   .= "FILE: $file ; line $line\n" if defined $file;
        $err_line   .= "PROG: $switch\n$prog\n" .
			           "EXPECTED:\n$expected\n";
        $err_line   .= "EXIT STATUS: != 0\n" if $fatal;
        $err_line   .= "GOT:\n$results\n";
        $err_line   .= "EXIT STATUS: " . ($status >> 8) . "\n" if $fatal;
        if ($::TODO) {
            $err_line =~ s/^/# /mg;
            print $err_line;  # Harness can't filter it out from STDERR.
        }
        else {
            print STDERR $err_line;
            ++$count_failures;
            die "PERL_TEST_ABORT_FIRST_FAILURE set Test Failure"
                if $ENV{PERL_TEST_ABORT_FIRST_FAILURE};
        }
    }

        if (defined $file) {
            _ok($ok, "at $file line $line", $name);
        } else {
            # We don't have file and line number data for the test, so report
            # errors as coming from our caller.
            local $Level = $Level + 1;
            ok($ok, $name);
        }

	foreach (@temps) {
	    unlink $_ if $_;
	}
	foreach (@temp_path) {
	    File::Path::rmtree $_ if -d $_;
	}
    }

    if ( $count_failures ) {
        print STDERR <<'EOS';
#
# Note: 'run_multiple_progs' run has one or more failures
#        you can consider setting the environment variable
#        PERL_TEST_ABORT_FIRST_FAILURE=1 before running the test
#        to stop on the first error.
#
EOS
    }


    return;
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


# Call $class->new( @$args ); and run the result through object_ok.
# See Test::More::new_ok
sub new_ok {
    my($class, $args, $obj_name) = @_;
    $args ||= [];
    $obj_name = "The object" unless defined $obj_name;

    local $Level = $Level + 1;

    my $obj;
    my $ok = eval { $obj = $class->new(@$args); 1 };
    my $error = $@;

    if($ok) {
        object_ok($obj, $class, $obj_name);
    }
    else {
        ok( 0, "new() died" );
        diag("Error was:  $@");
    }

    return $obj;

}


sub isa_ok ($$;$) {
    my($object, $class, $obj_name) = @_;

    my $diag;
    $obj_name = 'The object' unless defined $obj_name;
    my $name = "$obj_name isa $class";
    if( !defined $object ) {
        $diag = "$obj_name isn't defined";
    }
    else {
        my $whatami = ref $object ? 'object' : 'class';

        # We can't use UNIVERSAL::isa because we want to honor isa() overrides
        local($@, $!);  # eval sometimes resets $!
        my $rslt = eval { $object->isa($class) };
        my $error = $@;  # in case something else blows away $@

        if( $error ) {
            if( $error =~ /^Can't call method "isa" on unblessed reference/ ) {
                # It's an unblessed reference
                $obj_name = 'The reference' unless defined $obj_name;
                if( !UNIVERSAL::isa($object, $class) ) {
                    my $ref = ref $object;
                    $diag = "$obj_name isn't a '$class' it's a '$ref'";
                }
            }
            elsif( $error =~ /Can't call method "isa" without a package/ ) {
                # It's something that can't even be a class
                $obj_name = 'The thing' unless defined $obj_name;
                $diag = "$obj_name isn't a class or reference";
            }
            else {
                die <<WHOA;
WHOA! I tried to call ->isa on your object and got some weird error.
This should never happen.  Please contact the author immediately.
Here's the error.
$@
WHOA
            }
        }
        elsif( !$rslt ) {
            $obj_name = "The $whatami" unless defined $obj_name;
            my $ref = ref $object;
            $diag = "$obj_name isn't a '$class' it's a '$ref'";
        }
    }

    _ok( !$diag, _where(), $name );
}


sub class_ok {
    my($class, $isa, $class_name) = @_;

    # Written so as to count as one test
    local $Level = $Level + 1;
    if( ref $class ) {
        ok( 0, "$class is a reference, not a class name" );
    }
    else {
        isa_ok($class, $isa, $class_name);
    }
}


sub object_ok {
    my($obj, $isa, $obj_name) = @_;

    local $Level = $Level + 1;
    if( !ref $obj ) {
        ok( 0, "$obj is not a reference" );
    }
    else {
        isa_ok($obj, $isa, $obj_name);
    }
}


# Purposefully avoiding a closure.
sub __capture {
    push @::__capture, join "", @_;
}
    
sub capture_warnings {
    my $code = shift;

    local @::__capture;
    local $SIG {__WARN__} = \&__capture;
    local $Level = 1;
    &$code;
    return @::__capture;
}

# This will generate a variable number of tests.
# Use done_testing() instead of a fixed plan.
sub warnings_like {
    my ($code, $expect, $name) = @_;
    local $Level = $Level + 1;

    my @w = capture_warnings($code);

    cmp_ok(scalar @w, '==', scalar @$expect, $name);
    foreach my $e (@$expect) {
	if (ref $e) {
	    like(shift @w, $e, $name);
	} else {
	    is(shift @w, $e, $name);
	}
    }
    if (@w) {
	diag("Saw these additional warnings:");
	diag($_) foreach @w;
    }
}

sub _fail_excess_warnings {
    my($expect, $got, $name) = @_;
    local $Level = $Level + 1;
    # This will fail, and produce diagnostics
    is($expect, scalar @$got, $name);
    diag("Saw these warnings:");
    diag($_) foreach @$got;
}

sub warning_is {
    my ($code, $expect, $name) = @_;
    die sprintf "Expect must be a string or undef, not a %s reference", ref $expect
	if ref $expect;
    local $Level = $Level + 1;
    my @w = capture_warnings($code);
    if (@w > 1) {
	_fail_excess_warnings(0 + defined $expect, \@w, $name);
    } else {
	is($w[0], $expect, $name);
    }
}

sub warning_like {
    my ($code, $expect, $name) = @_;
    die sprintf "Expect must be a regexp object"
	unless ref $expect eq 'Regexp';
    local $Level = $Level + 1;
    my @w = capture_warnings($code);
    if (@w > 1) {
	_fail_excess_warnings(0 + defined $expect, \@w, $name);
    } else {
	like($w[0], $expect, $name);
    }
}

# Set a watchdog to timeout the entire test file.  The input seconds is
# multiplied by $ENV{PERL_TEST_TIME_OUT_FACTOR} (default 1; minimum 1).
# Set this in your profile for slow boxes, or use it to override the timeout
# temporarily for debugging.
#
# NOTE:  If the test file uses 'threads', then call the watchdog() function
#        _AFTER_ the 'threads' module is loaded.
{ # Closure
    my $watchdog;
    my $watchdog_thread;

sub watchdog ($;$)
{
    my $timeout = shift;

    # If cancelling, use the state variables to know which method was used to
    # create the watchdog.
    if ($timeout == 0) {
        if ($watchdog_thread) {
            $watchdog_thread->kill('KILL');
            undef $watch_dog_thread;
        }
        elsif ($watchdog) {
            kill('KILL', $watchdog);
            undef $watch_dog;
        }
        else {
            alarm(0);
        }

        return;
    }

    # Make sure these aren't defined.
    undef $watchdog;
    undef $watchdog_thread;

    my $method = shift || "";

    my $timeout_msg = 'Test process timed out - terminating';

    # Accept either spelling
    my $timeout_factor = $ENV{PERL_TEST_TIME_OUT_FACTOR}
                      || $ENV{PERL_TEST_TIMEOUT_FACTOR}
                      || 1;
    $timeout_factor = 1 if $timeout_factor < 1;
    $timeout_factor = $1 if $timeout_factor =~ /^(\d+)$/;

    # Valgrind slows perl way down so give it more time before dying.
    $timeout_factor = 10 if $timeout_factor < 10 && $ENV{PERL_VALGRIND};

    $timeout *= $timeout_factor;

    my $pid_to_kill = $$;   # PID for this process

    if ($method eq "alarm") {
        goto WATCHDOG_VIA_ALARM;
    }

    # shut up use only once warning
    my $threads_on = $threads::threads && $threads::threads;

    # Don't use a watchdog process if 'threads' is loaded -
    #   use a watchdog thread instead
    if (!$threads_on || $method eq "process") {

        # On Windows and VMS, try launching a watchdog process
        #   using system(1, ...) (see perlport.pod).  system() returns
        #   immediately on these platforms with effectively a pid of the new
        #   process
        if ($is_mswin || $is_vms) {
            # On Windows, try to get the 'real' PID
            if ($is_mswin) {
                eval { require Win32; };
                if (defined(&Win32::GetCurrentProcessId)) {
                    $pid_to_kill = Win32::GetCurrentProcessId();
                }
            }

            # If we still have a fake PID, we can't use this method at all
            return if ($pid_to_kill <= 0);

            # Launch watchdog process
            undef $watchdog;
            eval {
                local $SIG{'__WARN__'} = sub {
                    _diag("Watchdog warning: $_[0]");
                };
                my $sig = $is_vms ? 'TERM' : 'KILL';
                my $prog = "sleep($timeout);" .
                           "warn qq/# $timeout_msg" . '\n/;' .
                           "kill(q/$sig/, $pid_to_kill);";

                # If we're in taint mode PATH will be tainted
                $ENV{PATH} =~ /(.*)/s;
                local $ENV{PATH} = untaint_path($1);

                # On Windows use the indirect object plus LIST form to guarantee
                # that perl is launched directly rather than via the shell (see
                # perlfunc.pod), and ensure that the LIST has multiple elements
                # since the indirect object plus COMMANDSTRING form seems to
                # hang (see perl #121283). Don't do this on VMS, which doesn't
                # support the LIST form at all.
                if ($is_mswin) {
                    my $runperl = which_perl();
                    $runperl =~ /(.*)/;
                    $runperl = $1;
                    if ($runperl =~ m/\s/) {
                        $runperl = qq{"$runperl"};
                    }
                    $watchdog = system({ $runperl } 1, $runperl, '-e', $prog);
                }
                else {
                    my $cmd = _create_runperl(prog => $prog);
                    $watchdog = system(1, $cmd);
                }
            };
            if ($@ || ($watchdog <= 0)) {
                _diag('Failed to start watchdog');
                _diag($@) if $@;
                undef($watchdog);
                return;
            }

            # Add END block to parent to terminate and
            #   clean up watchdog process
            eval("END { local \$! = 0; local \$? = 0;
                        wait() if kill('KILL', $watchdog); };");
            return;
        }

        # Try using fork() to generate a watchdog process
        undef $watchdog;
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
		if ($is_cygwin) {
		    # sometimes the above isn't enough on cygwin
		    sleep 1; # wait a little, it might have worked after all
		    system("/bin/kill -f $pid_to_kill") if kill(0, $pid_to_kill);
		}
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
    if (eval {require threads; 1}) {
        $watchdog_thread = 'threads'->create(sub {
                # Load POSIX if available
                eval { require POSIX; };

                $SIG{'KILL'} = sub { threads->exit(); };

                # Detach after the signal handler is set up; the parent knows
                # not to signal until detached.
                'threads'->detach();

                # Execute the timeout
                my $time_left = $timeout;
                do {
                    $time_left = $time_left - sleep($time_left);
                } while ($time_left > 0);

                # Kill the parent (and ourself)
                select(STDERR); $| = 1;
                _diag($timeout_msg);
                POSIX::_exit(1) if (defined(&POSIX::_exit));
                my $sig = $is_vms ? 'TERM' : 'KILL';
                kill($sig, $pid_to_kill);
        });

        # Don't proceed until the watchdog has set up its signal handler.
        # (Otherwise there is a possibility that we will exit with threads
        # running.)  The watchdog tells us the handler is set by detaching
        # itself.  (The 'is_running()' is a fail-safe.)
        while (     $watchdog_thread->is_running()
               && ! $watchdog_thread->is_detached())
        {
            'threads'->yield();
        }

        return;
    }

    # If everything above fails, then just use an alarm timeout
WATCHDOG_VIA_ALARM:
    if (eval { alarm($timeout); 1; }) {
        # Load POSIX if available
        eval { require POSIX; };

        # Alarm handler will do the actual 'killing'
        $SIG{'ALRM'} = sub {
            select(STDERR); $| = 1;
            _diag($timeout_msg);
            POSIX::_exit(1) if (defined(&POSIX::_exit));
            my $sig = $is_vms ? 'TERM' : 'KILL';
            kill($sig, $pid_to_kill);
        };
    }
}
} # End closure

# Orphaned Docker or Linux containers do not necessarily attach to PID 1. They might attach to 0 instead.
sub is_linux_container {

    if ($^O eq 'linux' && open my $fh, '<', '/proc/1/cgroup') {
        while(<$fh>) {
            if (m{^\d+:pids:(.*)} && $1 ne '/init.scope') {
                return 1;
            }
        }
    }

    return 0;
}

1;
