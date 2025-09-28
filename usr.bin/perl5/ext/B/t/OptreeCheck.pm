package OptreeCheck;
use parent 'Exporter';
use strict;
use warnings;
our ($TODO, $Level, $using_open);
require "test.pl";

our $VERSION = '0.17';

# now export checkOptree, and those test.pl functions used by tests
our @EXPORT = qw( checkOptree plan skip skip_all pass is like unlike
		  require_ok runperl tempfile);


# The hints flags will differ if ${^OPEN} is set.
# The approach taken is to put the hints-with-open in the golden results, and
# flag that they need to be taken out if ${^OPEN} is set.

if (((caller 0)[10]||{})->{'open<'}) {
    $using_open = 1;
}

=head1 NAME

OptreeCheck - check optrees as rendered by B::Concise

=head1 SYNOPSIS

OptreeCheck supports 'golden-sample' regression testing of perl's
parser, optimizer, bytecode generator, via a single function:
checkOptree(%in).

It invokes B::Concise upon the sample code, checks that the rendering
'agrees' with the golden sample, and reports mismatches.

Additionally, the module processes @ARGV (which is typically unused in
the Core test harness), and thus provides a means to run the tests in
various modes.

=head1 EXAMPLE

  # your test file
  use OptreeCheck;
  plan tests => 1;

  checkOptree (
    name   => "test-name',	# optional, made from others if not given

    # code-under-test: must provide 1 of them
    code   => sub {my $a},	# coderef, or source (wrapped and evald)
    prog   => 'sort @a',	# run in subprocess, aka -MO=Concise
    bcopts => '-exec',		# $opt or \@opts, passed to BC::compile

    errs   => 'Name "main::a" used only once: possible typo at -e line 1.',
				# str, regex, [str+] [regex+],

    # various test options
    # errs   => '.*',		# match against any emitted errs, -w warnings
    # skip => 1,		# skips test
    # todo => 'excuse',		# anticipated failures
    # fail => 1			# force fail (by redirecting result)

    # the 'golden-sample's, (must provide both)

    expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT' );  # start HERE-DOCS
 # 1  <;> nextstate(main 45 optree.t:23) v
 # 2  <0> padsv[$a:45,46] M/LVINTRO
 # 3  <1> leavesub[1 ref] K/REFC,1
 EOT_EOT
 # 1  <;> nextstate(main 45 optree.t:23) v
 # 2  <0> padsv[$a:45,46] M/LVINTRO
 # 3  <1> leavesub[1 ref] K/REFC,1
 EONT_EONT

 __END__

=head2 Failure Reports

 Heres a sample failure, as induced by the following command.
 Note the argument; option=value, after the test-file, more on that later

 $> PERL_CORE=1 ./perl ext/B/t/optree_check.t  testmode=cross
 ...
 ok 19 - canonical example w -basic
 not ok 20 - -exec code: $a=$b+42
 # Failed at test.pl line 249
 #      got '1  <;> nextstate(main 600 optree_check.t:208) v
 # 2  <#> gvsv[*b] s
 # 3  <$> const[IV 42] s
 # 4  <2> add[t3] sK/2
 # 5  <#> gvsv[*a] s
 # 6  <2> sassign sKS/2
 # 7  <1> leavesub[1 ref] K/REFC,1
 # '
 # expected /(?ms-xi:^1  <;> (?:next|db)state(.*?) v
 # 2  <\$> gvsv\(\*b\) s
 # 3  <\$> const\(IV 42\) s
 # 4  <2> add\[t\d+\] sK/2
 # 5  <\$> gvsv\(\*a\) s
 # 6  <2> sassign sKS/2
 # 7  <1> leavesub\[\d+ refs?\] K/REFC,1
 # $)/
 # got:          '2  <#> gvsv[*b] s'
 # want:  (?^:2  <\$> gvsv\(\*b\) s)
 # got:          '3  <$> const[IV 42] s'
 # want:  (?^:3  <\$> const\(IV 42\) s)
 # got:          '5  <#> gvsv[*a] s'
 # want:  (?^:5  <\$> gvsv\(\*a\) s)
 # remainder:
 # 2  <#> gvsv[*b] s
 # 3  <$> const[IV 42] s
 # 5  <#> gvsv[*a] s
 # these lines not matched:
 # 2  <#> gvsv[*b] s
 # 3  <$> const[IV 42] s
 # 5  <#> gvsv[*a] s

Errors are reported 3 different ways;

The 1st form is directly from test.pl's like() and unlike().  Note
that this form is used as input, so you can easily cut-paste results
into test-files you are developing.  Just make sure you recognize
insane results, to avoid canonizing them as golden samples.

The 2nd and 3rd forms show only the unexpected results and opcodes.
This is done because it's blindingly tedious to find a single opcode
causing the failure.  2 different ways are done in case one is
unhelpful.

=head1 TestCase Overview

checkOptree(%tc) constructs a testcase object from %tc, and then calls
methods which eventually call test.pl's like() to produce test
results.

=head2 getRendering

getRendering() runs code or prog or progfile through B::Concise, and
captures its rendering.  Errors emitted during rendering are checked
against expected errors, and are reported as diagnostics by default,
or as failures if 'report=fail' cmdline-option is given.

prog is run in a sub-shell, with $bcopts passed through. This is the way
to run code intended for main.  The code arg in contrast, is always a
CODEREF, either because it starts that way as an arg, or because it's
wrapped and eval'd as $sub = sub {$code};

=head2 mkCheckRex

mkCheckRex() selects the golden-sample for the threaded-ness of the
platform, and produces a regex which matches the expected rendering,
and fails when it doesn't match.

The regex includes 'workarounds' which accommodate expected rendering
variations. These include:

  string constants		# avoid injection
  line numbers, etc		# args of nexstate()
  hexadecimal-numbers

  pad-slot-assignments		# for 5.8 compat, and testmode=cross
  (map|grep)(start|while)	# for 5.8 compat

=head2 mylike

mylike() calls either unlike() or like(), depending on
expectations.  Mismatch reports are massaged, because the actual
difference can easily be lost in the forest of opcodes.

=head1 checkOptree API and Operation

Since the arg is a hash, the api is wide-open, and this really is
about what elements must be or are in the hash, and what they do.  %tc
is passed to newTestCase(), the ctor, which adds in %proto, a global
prototype object.

=head2 name => STRING

If name property is not provided, it is synthesized from these params:
bcopts, note, prog, code.  This is more convenient than trying to do
it manually.

=head2 code or prog or progfile

Either code or prog or progfile must be present.

=head2 prog => $perl_source_string

prog => $src provides a snippet of code, which is run in a sub-process,
via test.pl:runperl, and through B::Concise like so:

    './perl -w -MO=Concise,$bcopts_massaged -e $src'

=head2 progfile => $perl_script

progfile => $file provides a file containing a snippet of code which is
run as per the prog => $src example above.

=head2 code => $perl_source_string || CODEREF

The $code arg is passed to B::Concise::compile(), and run in-process.
If $code is a string, it's first wrapped and eval'd into a $coderef.
In either case, $coderef is then passed to B::Concise::compile():

    $subref = eval "sub{$code}";
    $render = B::Concise::compile($subref)->();

=head2 expect and expect_nt

expect and expect_nt args are the B<golden-sample> renderings, and are
sampled from known-ok threaded and un-threaded bleadperl builds.
They're both required, and the correct one is selected for the platform
being tested, and saved into the synthesized property B<wanted>.

=head2 bcopts => $bcopts || [ @bcopts ]

When getRendering() runs, it passes bcopts into B::Concise::compile().
The bcopts arg can be a single string, or an array of strings.

=head2 errs => $err_str_regex || [ @err_str_regexs ] 

getRendering() processes the code or prog or progfile arg under warnings,
and both parsing and optree-traversal errors are collected.  These are
validated against the one or more errors you specify.

=head1 testcase modifier properties

These properties are set as %tc parameters to change test behavior.

=head2 skip => 'reason'

invokes skip('reason'), causing test to skip.

=head2 todo => 'reason'

invokes todo('reason')

=head2 fail => 1

For code arguments, this option causes getRendering to redirect the
rendering operation to STDERR, which causes the regex match to fail.

=head2 noanchors => 1

If set, this relaxes the regex check, which is normally pretty strict.
It's used primarily to validate checkOptree via tests in optree_check.


=head1 Synthesized object properties

These properties are added into the test object during execution.

=head2 wanted

This stores the chosen expect expect_nt string.  The OptreeCheck
object may in the future delete the raw strings once wanted is set,
thus saving space.

=head2 cross => 1

This tag is added if testmode=cross is passed in as argument.
It causes test-harness to purposely use the wrong string.


=head2 checkErrs

checkErrs() is a getRendering helper that verifies that expected errs
against those found when rendering the code on the platform.  It is
run after rendering, and before mkCheckRex.

=cut

use Config;
use Carp;
use B::Concise qw(walk_output);

BEGIN {
    $SIG{__WARN__} = sub {
	my $err = shift;
	$err =~ m/Subroutine re::(un)?install redefined/ and return;
    };
}

sub import {
    my $pkg = shift;
    $pkg->export_to_level(1,'checkOptree', @EXPORT);
    getCmdLine();	# process @ARGV
}


# %gOpts params comprise a global test-state.  Initial values here are
# HELP strings, they MUST BE REPLACED by runtime values before use, as
# is done by getCmdLine(), via import

our %gOpts = 	# values are replaced at runtime !!
    (
     # scalar values are help string
     selftest	=> 'self-tests mkCheckRex vs the reference rendering',

     fail	=> 'force all test to fail, print to stdout',
     dump	=> 'dump cmdline arg processing',
     noanchors	=> 'dont anchor match rex',

     # array values are one-of selections, with 1st value as default
     #  array: 2nd value is used as help-str, 1st val (still) default
     help	=> [0, 'provides help and exits', 0],
     testmode	=> [qw/ native cross both /],

     # fixup for VMS, cygwin, which don't have stderr b4 stdout
     rxnoorder	=> [1, 'if 1, dont req match on -e lines, and -banner',0],
     strip	=> [1, 'if 1, catch errs and remove from renderings',0],
     stripv	=> 'if strip&&1, be verbose about it',
     errs	=> 'expected compile errs, array if several',
    );


our $threaded = 1 if $Config::Config{usethreads};
our $platform = ($threaded) ? "threaded" : "plain";
our $thrstat = ($threaded)  ? "threaded" : "nonthreaded";

our %modes = (
	      both	=> [ 'expect', 'expect_nt'],
	      native	=> [ ($threaded) ? 'expect' : 'expect_nt'],
	      cross	=> [ !($threaded) ? 'expect' : 'expect_nt'],
	      expect	=> [ 'expect' ],
	      expect_nt	=> [ 'expect_nt' ],
	      );

our %msgs # announce cross-testing.
    = (
       # cross-platform
       'expect_nt-threaded' => " (nT on T) ",
       'expect-nonthreaded' => " (T on nT) ",
       # native - nothing to say (must stay empty - used for $crosstesting)
       'expect_nt-nonthreaded'	=> '',
       'expect-threaded'	=> '',
       );

#######
sub getCmdLine {	# import assistant
    # offer help
    print(qq{\n$0 accepts args to update these state-vars:
	     turn on a flag by typing its name,
	     select a value from list by typing name=val.\n    },
	  mydumper(\%gOpts))
	if grep /help/, @ARGV;

    # replace values for each key !! MUST MARK UP %gOpts
    foreach my $opt (keys %gOpts) {

	# scan ARGV for known params
	if (ref $gOpts{$opt} eq 'ARRAY') {

	    # $opt is a One-Of construct
	    # replace with valid selection from the list

	    # uhh this WORKS. but it's inscrutable
	    # grep s/$opt=(\w+)/grep {$_ eq $1} @ARGV and $gOpts{$opt}=$1/e, @ARGV;
	    my $tval;  # temp
	    if (grep s/$opt=(\w+)/$tval=$1/e, @ARGV) {
		# check val before accepting
		my @allowed = @{$gOpts{$opt}};
		if (grep { $_ eq $tval } @allowed) {
		    $gOpts{$opt} = $tval;
		}
		else {die "invalid value: '$tval' for $opt\n"}
	    }

	    # take 1st val as default
	    $gOpts{$opt} = ${$gOpts{$opt}}[0]
		if ref $gOpts{$opt} eq 'ARRAY';
        }
        else { # handle scalars

	    # if 'opt' is present, true
	    $gOpts{$opt} = (grep /^$opt/, @ARGV) ? 1 : 0;

	    # override with 'foo' if 'opt=foo' appears
	    grep s/$opt=(.*)/$gOpts{$opt}=$1/e, @ARGV;
	}
     }
    print("$0 heres current state:\n", mydumper(\%gOpts))
	if $gOpts{help} or $gOpts{dump};

    exit if $gOpts{help};
}
# the above arg-handling cruft should be replaced by a Getopt call

##############################
# the API (1 function)

sub checkOptree {
    my $tc = newTestCases(@_);	# ctor
    my ($rendering);

    print "checkOptree args: ",mydumper($tc) if $tc->{dump};
    SKIP: {
	if ($tc->{skip}) {
	    skip("$tc->{skip} $tc->{name}",
		    ($gOpts{selftest}
			? 1
			: 1 + @{$modes{$gOpts{testmode}}}
			)
	    );
	}

	return runSelftest($tc) if $gOpts{selftest};

	$tc->getRendering();	# get the actual output
	$tc->checkErrs();

	local $Level = $Level + 2;
      TODO:
	foreach my $want (@{$modes{$gOpts{testmode}}}) {
	    local $TODO = $tc->{todo} if $tc->{todo};

	    $tc->{cross} = $msgs{"$want-$thrstat"};

	    $tc->mkCheckRex($want);
	    $tc->mylike();
	}
    }
    return;
}

sub newTestCases {
    # make test objects (currently 1) from args (passed to checkOptree)
    my $tc = bless { @_ }, __PACKAGE__
	or die "test cases are hashes";

    $tc->label();

    # cpy globals into each test
    foreach my $k (keys %gOpts) {
	if ($gOpts{$k}) {
	    $tc->{$k} = $gOpts{$k} unless defined $tc->{$k};
	}
    }
    if ($tc->{errs}) {
	$tc->{errs} = [$tc->{errs}] unless ref $tc->{errs} eq 'ARRAY';
    }
    return $tc;
}

sub label {
    # may help get/keep test output consistent
    my ($tc) = @_;
    return $tc->{name} if $tc->{name};

    my $buf = (ref $tc->{bcopts}) 
	? join(',', @{$tc->{bcopts}}) : $tc->{bcopts};

    foreach (qw( note prog code )) {
	$buf .= " $_: $tc->{$_}" if $tc->{$_} and not ref $tc->{$_};
    }
    return $tc->{name} = $buf;
}

#################
# render and its helpers

sub getRendering {
    my $tc = shift;
    fail("getRendering: code or prog or progfile is required")
	unless $tc->{code} or $tc->{prog} or $tc->{progfile};

    my @opts = get_bcopts($tc);
    my $rendering = ''; # suppress "Use of uninitialized value in open"
    my @errs;		# collect errs via 


    if ($tc->{prog}) {
	$rendering = runperl( switches => ['-w',join(',',"-MO=Concise",@opts)],
			      prog => $tc->{prog}, stderr => 1,
			      ); # verbose => 1);
    } elsif ($tc->{progfile}) {
	$rendering = runperl( switches => ['-w',join(',',"-MO=Concise",@opts)],
			      progfile => $tc->{progfile}, stderr => 1,
			      ); # verbose => 1);
    } else {
	my $code = $tc->{code};
	unless (ref $code eq 'CODE') {
	    # treat as source, and wrap into subref 
	    #  in caller's package ( to test arg-fixup, comment next line)
	    my $pkg = '{ package '.caller(1) .';';
	    {
		BEGIN { $^H = 0 }
		no warnings;
		$code = eval "$pkg sub { $code } }";
	    }
	    # return errors
	    if ($@) { chomp $@; push @errs, $@ }
	}
	# set walk-output b4 compiling, which writes 'announce' line
	walk_output(\$rendering);

	my $opwalker = B::Concise::compile(@opts, $code);
	die "bad BC::compile retval" unless ref $opwalker eq 'CODE';

      B::Concise::reset_sequence();
	$opwalker->();

	# kludge error into rendering if its empty.
	$rendering = $@ if $@ and ! $rendering;
    }
    # separate banner, other stuff whose printing order isnt guaranteed
    if ($tc->{strip}) {
	$rendering =~ s/(B::Concise::compile.*?\n)//;
	print "stripped from rendering <$1>\n" if $1 and $tc->{stripv};

	#while ($rendering =~ s/^(.*?(-e) line \d+\.)\n//g) {
	while ($rendering =~ s/^(.*?(-e|\(eval \d+\).*?) line \d+\.)\n//g) {
	    print "stripped <$1> $2\n" if $tc->{stripv};
	    push @errs, $1;
	}
	$rendering =~ s/-e syntax OK\n//;
	$rendering =~ s/-e had compilation errors\.\n//;
    }
    $tc->{got}	   = $rendering;
    $tc->{goterrs} = \@errs if @errs;
    return $rendering, @errs;
}

sub get_bcopts {
    # collect concise passthru-options if any
    my ($tc) = shift;
    my @opts = ();
    if ($tc->{bcopts}) {
	@opts = (ref $tc->{bcopts} eq 'ARRAY')
	    ? @{$tc->{bcopts}} : ($tc->{bcopts});
    }
    return @opts;
}

sub checkErrs {
    # check rendering errs against expected errors, reduce and report
    my $tc = shift;

    # check for agreement (order not important)
    my (%goterrs, @missed);
    @goterrs{@{$tc->{goterrs}}} = (1) x scalar @{$tc->{goterrs}}
	if $tc->{goterrs};

    foreach my $want (@{$tc->{errs}}) {
	if (ref $want) {
	    my $seen;
	    foreach my $k (keys %goterrs) {
		next unless $k =~ $want;
		delete $goterrs{$k};
		++$seen;
	    }
	    push @missed, $want unless $seen;
	} else {
	    push @missed, $want unless defined delete $goterrs{$want};
	}
    }

    @missed = sort @missed;
    my @got = sort keys %goterrs;

    if (@{$tc->{errs}}) {
	is(@missed + @got, 0, "Only got expected errors for $tc->{name}")
    } else {
	# @missed must be 0 here.
	is(scalar @got, 0, "Got no errors for $tc->{name}")
    }
    _diag(join "\n", "got unexpected:", @got) if @got;
    _diag(join "\n", "missed expected:", @missed) if @missed;
}

=head1 mkCheckRex ($tc)

It selects the correct golden-sample from the test-case object, and
converts it into a Regexp which should match against the original
golden-sample (used in selftest, see below), and on the renderings
obtained by applying the code on the perl being tested.

The selection is driven by platform mostly, but also by test-mode,
which rather complicates the code.  This is worsened by the potential
need to make platform specific conversions on the reftext.

but is otherwise as strict as possible.  For example, it should *not*
match when opcode flags change, or when optimizations convert an op to
an ex-op.


=head2 match criteria

The selected golden-sample is massaged to eliminate various match
irrelevancies.  This is done so that the tests don't fail just because
you added a line to the top of the test file.  (Recall that the
renderings contain the program's line numbers).  Similar cleanups are
done on "strings", hex-constants, etc.

The need to massage is reflected in the 2 golden-sample approach of
the test-cases; we want the match to be as rigorous as possible, and
thats easier to achieve when matching against 1 input than 2.

Opcode arguments (text within braces) are disregarded for matching
purposes.  This loses some info in 'add[t5]', but greatly simplifies
matching 'nextstate(main 22 (eval 10):1)'.  Besides, we are testing
for regressions, not for complete accuracy.

The regex is anchored by default, but can be suppressed with
'noanchors', allowing 1-liner tests to succeed if opcode is found.

=cut

# needless complexity due to 'too much info' from B::Concise v.60
my $announce = 'B::Concise::compile\(CODE\(0x[0-9a-f]+\)\)';;

sub mkCheckRex {
    # converts expected text into Regexp which should match against
    # unaltered version.  also adjusts threaded => non-threaded
    my ($tc, $want) = @_;

    my $str = $tc->{expect} || $tc->{expect_nt};	# standard bias
    $str = $tc->{$want} if $want && $tc->{$want};	# stated pref

    die("no '$want' golden-sample found: $tc->{name}") unless $str;

    $str =~ s/^\# //mg;	# ease cut-paste testcase authoring

    $tc->{wantstr} = $str;

    # make UNOP_AUX flag type literal
    $str =~ s/<\+>/<\\+>/;
    # make targ args wild
    $str =~ s/\[t\d+\]/[t\\d+]/msg;

    # escape bracing, etc.. manual \Q (doesn't escape '+')
    $str =~ s/([\[\]()*.\$\@\#\|{}])/\\$1/msg;
    # $str =~ s/(?<!\\)([\[\]\(\)*.\$\@\#\|{}])/\\$1/msg;

    # treat dbstate like nextstate (no in-debugger false reports)
    # Note also that there may be 1 level of () nexting, if there's an eval
    # Seems easiest to explicitly match the eval, rather than trying to parse
    # for full balancing and then substitute .*?
    # In which case, we can continue to match for the eval in the rexexp built
    # from the golden result.

    $str =~ s!(?:next|db)state
	      \\\(			# opening literal ( (backslash escaped)
	      [^()]*?			# not ()
	      (\\\(eval\ \d+\\\)	# maybe /eval \d+/ in ()
	       [^()]*?			# which might be followed by something
	      )?
	      \\\)			# closing literal )
	     !'(?:next|db)state\\([^()]*?' .
	      ($1 && '\\(eval \\d+\\)[^()]*')	# Match the eval if present
	      . '\\)'!msgxe;
    # widened for -terse mode
    $str =~ s/(?:next|db)state/(?:next|db)state/msg;
    if (!$using_open && $tc->{strip_open_hints}) {
      $str =~ s[(			# capture
		 \(\?:next\|db\)state	# the regexp matching next/db state
		 .*			# all sorts of things follow it
		 v			# The opening v
		)
		(?:(:>,<,%,\\\{)		# hints when open.pm is in force
		   |(:>,<,%))		# (two variations)
		(\ ->(?:-|[0-9a-z]+))?
		$
	       ]
        [$1 . ($2 && ':\{') . $4]xegm;	# change to the hints without open.pm
    }


    # don't care about:
    $str =~ s/:-?\d+,-?\d+/:-?\\d+,-?\\d+/msg;		# FAKE line numbers
    $str =~ s/match\\\(.*?\\\)/match\(.*?\)/msg;	# match args
    $str =~ s/(0x[0-9A-Fa-f]+)/0x[0-9A-Fa-f]+/msg;	# hexnum values
    $str =~ s/".*?"/".*?"/msg;				# quoted strings
    $str =~ s/FAKE:(\w):\d+/FAKE:$1:\\d+/msg;		# parent pad index

    $str =~ s/(\d refs?)/\\d+ refs?/msg;		# 1 ref, 2+ refs (plural)
    $str =~ s/leavesub \[\d\]/leavesub [\\d]/msg;	# for -terse
    #$str =~ s/(\s*)\n/\n/msg;				# trailing spaces
    
    croak "whitespace only reftext found for '$want': $tc->{name}"
	unless $str =~ /\w+/; # fail unless a real test
    
    # $str = '.*'	if 1;	# sanity test
    # $str .= 'FAIL'	if 1;	# sanity test

    # allow -eval, banner at beginning of anchored matches
    $str = "(-e .*?)?(B::Concise::compile.*?)?\n" . $str
	unless $tc->{noanchors} or $tc->{rxnoorder};
    
    my $qr = ($tc->{noanchors})	? qr/$str/ms : qr/^$str$/ms ;

    $tc->{rex}		= $qr;
    $tc->{rexstr}	= $str;
    $tc;
}

##############
# compare and report

sub mylike {
    # reworked mylike to use hash-obj
    my $tc	= shift;
    my $got	= $tc->{got};
    my $want	= $tc->{rex};
    my $cmnt	= $tc->{name};
    my $cross	= $tc->{cross};

    # bad is anticipated failure
    my $bad = ($cross && $tc->{crossfail}) || (!$cross && $tc->{fail});

    my $ok = $bad ? unlike ($got, $want, $cmnt) : like ($got, $want, $cmnt);

    reduceDiffs ($tc) if not $ok;

    return $ok;
}

sub reduceDiffs {
    # isolate the real diffs and report them.
    # i.e. these kinds of errs:
    # 1. missing or extra ops.  this skews all following op-sequences
    # 2. single op diff, the rest of the chain is unaltered
    # in either case, std err report is inadequate;

    my $tc	= shift;
    my $got	= $tc->{got};
    my @got	= split(/\n/, $got);
    my $want	= $tc->{wantstr};
    my @want	= split(/\n/, $want);

    # split rexstr into units that should eat leading lines.
    my @rexs = map qr/$_/, split (/\n/, $tc->{rexstr});

    foreach my $rex (@rexs) {
        my $exp = shift @want;
        my $line = shift @got;
        # remove matches, and report
        unless ($got =~ s/^($rex\n)//ms) {
            _diag("got:\t\t'$line'\nwant:\t $rex\n");
            last;
        }
    }
    _diag("remainder:\n$got");
    _diag("these lines not matched:\n$got\n");
}

=head1 Global modes

Unusually, this module also processes @ARGV for command-line arguments
which set global modes.  These 'options' change the way the tests run,
essentially reusing the tests for different purposes.



Additionally, there's an experimental control-arg interface (i.e.
subject to change) which allows the user to set global modes.


=head1 Testing Method

At 1st, optreeCheck used one reference-text, but the differences
between Threaded and Non-threaded renderings meant that a single
reference (sampled from say, threaded) would be tricky and iterative
to convert for testing on a non-threaded build.  Worse, this conflicts
with making tests both strict and precise.

We now use 2 reference texts, the right one is used based upon the
build's threaded-ness.  This has several benefits:

 1. native reference data allows closer/easier matching by regex.
 2. samples can be eyeballed to grok T-nT differences.
 3. data can help to validate mkCheckRex() operation.
 4. can develop regexes which accommodate T-nT differences.
 5. can test with both native and cross-converted regexes.

Cross-testing (expect_nt on threaded, expect on non-threaded) exposes
differences in B::Concise output, so mkCheckRex has code to do some
cross-test manipulations.  This area needs more work.

=head1 Test Modes

One consequence of a single-function API is difficulty controlling
test-mode.  I've chosen for now to use a package hash, %gOpts, to store
test-state.  These properties alter checkOptree() function, either
short-circuiting to selftest, or running a loop that runs the testcase
2^N times, varying conditions each time.  (current N is 2 only).

So Test-mode is controlled with cmdline args, also called options below.
Run with 'help' to see the test-state, and how to change it.

=head2  selftest

This argument invokes runSelftest(), which tests a regex against the
reference renderings that they're made from.  Failure of a regex match
its 'mold' is a strong indicator that mkCheckRex is buggy.

That said, selftest mode currently runs a cross-test too, they're not
completely orthogonal yet.  See below.

=head2 testmode=cross

Cross-testing is purposely creating a T-NT mismatch, looking at the
fallout, which helps to understand the T-NT differences.

The tweaking appears contrary to the 2-refs philosophy, but the tweaks
will be made in conversion-specific code, which (will) handles T->NT
and NT->T separately.  The tweaking is incomplete.

A reasonable 1st step is to add tags to indicate when TonNT or NTonT
is known to fail.  This needs an option to force failure, so the
test.pl reporting mechanics show results to aid the user.

=head2 testmode=native

This is normal mode.  Other valid values are: native, cross, both.

=head2 checkOptree Notes

Accepts test code, renders its optree using B::Concise, and matches
that rendering against a regex built from one of 2 reference
renderings %tc data.

The regex is built by mkCheckRex(\%tc), which scrubs %tc data to
remove match-irrelevancies, such as (args) and [args].  For example,
it strips leading '# ', making it easy to cut-paste new tests into
your test-file, run it, and cut-paste actual results into place.  You
then retest and reedit until all 'errors' are gone.  (now make sure you
haven't 'enshrined' a bug).

name: The test name.  May be augmented by a label, which is built from
important params, and which helps keep names in sync with whats being
tested.

=cut

sub runSelftest {
    # tests the regex produced by mkCheckRex()
    # by using on the expect* text it was created with
    # failures indicate a code bug, 
    # OR regexs plugged into the expect* text (which defeat conversions)
    my $tc = shift;

    for my $provenance (qw/ expect expect_nt /) {
	#next unless $tc->{$provenance};

	$tc->mkCheckRex($provenance);
	$tc->{got} = $tc->{wantstr};	# fake the rendering
	$tc->mylike();
    }
}

my $dumploaded = 0;

sub mydumper {

    do { Dumper(@_); return } if $dumploaded;

    eval "require Data::Dumper"
	or do{
	    print "Sorry, Data::Dumper is not available\n";
	    print "half hearted attempt:\n";
	    foreach my $it (@_) {
		if (ref $it eq 'HASH') {
		    print " $_ => $it->{$_}\n" foreach sort keys %$it;
		}
	    }
	    return;
	};

    Data::Dumper->import;
    $Data::Dumper::Sortkeys = 1;
    $dumploaded++;
    Dumper(@_);
}

############################
# support for test writing

sub preamble {
    my $testct = shift || 1;
    return <<EO_HEADER;
#!perl

BEGIN {
    chdir q(t);
    \@INC = qw(../lib ../ext/B/t);
    require q(./test.pl);
}
use OptreeCheck;
plan tests => $testct;

EO_HEADER

}

sub OptreeCheck::wrap {
    my $code = shift;
    $code =~ s/(?:(\#.*?)\n)//gsm;
    $code =~ s/\s+/ /mgs;	       
    chomp $code;
    return unless $code =~ /\S/;
    my $comment = $1;
    
    my $testcode = qq{
	
checkOptree(note   => q{$comment},
	    bcopts => q{-exec},
	    code   => q{$code},
	    expect => <<EOT_EOT, expect_nt => <<EONT_EONT);
ThreadedRef
    paste your 'golden-example' here, then retest
EOT_EOT
NonThreadedRef
    paste your 'golden-example' here, then retest
EONT_EONT
    
};
    return $testcode;
}

sub OptreeCheck::gentest {
    my ($code,$opts) = @_;
    my $rendering = getRendering({code => $code});
    my $testcode = OptreeCheck::wrap($code);
    return unless $testcode;

    # run the prog, capture 'reference' concise output
    my $preamble = preamble(1);
    my $got = runperl( prog => "$preamble $testcode", stderr => 1,
		       #switches => ["-I../ext/B/t", "-MOptreeCheck"], 
		       );  #verbose => 1);
    
    # extract the 'reftext' ie the got 'block'
    if ($got =~ m/got \'.*?\n(.*)\n\# \'\n\# expected/s) {
	my $goldentxt = $1;
	#and plug it into the test-src
	if ($threaded) {
	    $testcode =~ s/ThreadedRef/$goldentxt/;
	} else {
	    $testcode =~ s/NonThreadRef/$goldentxt/;
	}
	my $b4 = q{expect => <<EOT_EOT, expect_nt => <<EONT_EONT};
	my $af = q{expect => <<'EOT_EOT', expect_nt => <<'EONT_EONT'};
	$testcode =~ s/$b4/$af/;
	
	return $testcode;
    }
    return '';
}


sub OptreeCheck::processExamples {
    my @files = @_;

    # gets array of paragraphs, which should be code-samples.  They're
    # turned into optreeCheck tests,

    foreach my $file (@files) {
	open (my $fh, '<', $file) or die "cant open $file: $!\n";
	$/ = "";
	my @chunks = <$fh>;
	print preamble (scalar @chunks);
	foreach my $t (@chunks) {
	    print "\n\n=for gentest\n\n# chunk: $t=cut\n\n";
	    print OptreeCheck::gentest ($t);
	}
    }
}

# OK - now for the final insult to your good taste...  

if ($0 =~ /OptreeCheck\.pm/) {

    #use lib 't';
    require './t/test.pl';

    # invoked as program.  Work like former gentest.pl,
    # ie read files given as cmdline args,
    # convert them to usable test files.

    require Getopt::Std;
    Getopt::Std::getopts('') or 
	die qq{ $0 sample-files*    # no options

	  expecting filenames as args.  Each should have paragraphs,
	  these are converted to checkOptree() tests, and printed to
	  stdout.  Redirect to file then edit for test. \n};

  OptreeCheck::processExamples(@ARGV);
}

1;

__END__

=head1 TEST DEVELOPMENT SUPPORT

This optree regression testing framework needs tests in order to find
bugs.  To that end, OptreeCheck has support for developing new tests,
according to the following model:

 1. write a set of sample code into a single file, one per
    paragraph.  Add <=for gentest> blocks if you care to, or just look at
    f_map and f_sort in ext/B/t/ for examples.

 2. run OptreeCheck as a program on the file

   ./perl -Ilib ext/B/t/OptreeCheck.pm -w ext/B/t/f_map
   ./perl -Ilib ext/B/t/OptreeCheck.pm -w ext/B/t/f_sort

   gentest reads the sample code, runs each to generate a reference
   rendering, folds this rendering into an optreeCheck() statement,
   and prints it to stdout.

 3. run the output file as above, redirect to files, then rerun on
    same build (for sanity check), and on thread-opposite build.  With
    editor in 1 window, and cmd in other, it's fairly easy to cut-paste
    the gots into the expects, easier than running step 2 on both
    builds then trying to sdiff them together.

=head1 CAVEATS

This code is purely for testing core. While checkOptree feels flexible
enough to be stable, the whole selftest framework is subject to change
w/o notice.

=cut
