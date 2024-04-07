#!./perl -w

# Tests for the coderef-in-@INC feature

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

use Config;

my $can_fork   = 0;
my $has_perlio = $Config{useperlio};

unless (is_miniperl()) {
    if ($Config{d_fork} && eval 'require POSIX; 1') {
	$can_fork = 1;
    }
}

use strict;

plan(tests => 71 + !is_miniperl() * (4 + 14 * $can_fork));

sub get_temp_fh {
    my $f = tempfile();
    open my $fh, ">$f" or die "Can't create $f: $!";
    print $fh "package ".substr($_[0],0,-3).";\n1;\n";
    print $fh $_[1] if @_ > 1;
    close $fh or die "Couldn't close: $!";
    open $fh, $f or die "Can't open $f: $!";
    return $fh;
}

sub fooinc {
    my ($self, $filename) = @_;
    if (substr($filename,0,3) eq 'Foo') {
	return get_temp_fh($filename);
    }
    else {
        return undef;
    }
}

push @INC, \&fooinc;

my $evalret = eval { require Bar; 1 };
ok( !$evalret,      'Trying non-magic package' );

$evalret = eval { require Foo; 1 };
die $@ if $@;
ok( $evalret,                      'require Foo; magic via code ref'  );
ok( exists $INC{'Foo.pm'},         '  %INC sees Foo.pm' );
is( ref $INC{'Foo.pm'}, 'CODE',    '  val Foo.pm is a coderef in %INC' );
is( $INC{'Foo.pm'}, \&fooinc,	   '  val Foo.pm is correct in %INC' );

$evalret = eval "use Foo1; 1;";
die $@ if $@;
ok( $evalret,                      'use Foo1' );
ok( exists $INC{'Foo1.pm'},        '  %INC sees Foo1.pm' );
is( ref $INC{'Foo1.pm'}, 'CODE',   '  val Foo1.pm is a coderef in %INC' );
is( $INC{'Foo1.pm'}, \&fooinc,     '  val Foo1.pm is correct in %INC' );

$evalret = eval { do 'Foo2.pl'; 1 };
die $@ if $@;
ok( $evalret,                      'do "Foo2.pl"' );
ok( exists $INC{'Foo2.pl'},        '  %INC sees Foo2.pl' );
is( ref $INC{'Foo2.pl'}, 'CODE',   '  val Foo2.pl is a coderef in %INC' );
is( $INC{'Foo2.pl'}, \&fooinc,     '  val Foo2.pl is correct in %INC' );

pop @INC;


sub fooinc2 {
    my ($self, $filename) = @_;
    if (substr($filename, 0, length($self->[1])) eq $self->[1]) {
	return get_temp_fh($filename);
    }
    else {
        return undef;
    }
}

my $arrayref = [ \&fooinc2, 'Bar' ];
push @INC, $arrayref;

$evalret = eval { require Foo; 1; };
die $@ if $@;
ok( $evalret,                     'Originally loaded packages preserved' );
$evalret = eval { require Foo3; 1; };
ok( !$evalret,                    'Original magic INC purged' );

$evalret = eval { require Bar; 1 };
die $@ if $@;
ok( $evalret,                     'require Bar; magic via array ref' );
ok( exists $INC{'Bar.pm'},        '  %INC sees Bar.pm' );
is( ref $INC{'Bar.pm'}, 'ARRAY',  '  val Bar.pm is an arrayref in %INC' );
is( $INC{'Bar.pm'}, $arrayref,    '  val Bar.pm is correct in %INC' );

ok( eval "use Bar1; 1;",          'use Bar1' );
ok( exists $INC{'Bar1.pm'},       '  %INC sees Bar1.pm' );
is( ref $INC{'Bar1.pm'}, 'ARRAY', '  val Bar1.pm is an arrayref in %INC' );
is( $INC{'Bar1.pm'}, $arrayref,   '  val Bar1.pm is correct in %INC' );

ok( eval { do 'Bar2.pl'; 1 },     'do "Bar2.pl"' );
ok( exists $INC{'Bar2.pl'},       '  %INC sees Bar2.pl' );
is( ref $INC{'Bar2.pl'}, 'ARRAY', '  val Bar2.pl is an arrayref in %INC' );
is( $INC{'Bar2.pl'}, $arrayref,   '  val Bar2.pl is correct in %INC' );

pop @INC;

sub FooLoader::INC {
    my ($self, $filename) = @_;
    if (substr($filename,0,4) eq 'Quux') {
	return get_temp_fh($filename);
    }
    else {
        return undef;
    }
}

my $href = bless( {}, 'FooLoader' );
push @INC, $href;

$evalret = eval { require Quux; 1 };
die $@ if $@;
ok( $evalret,                      'require Quux; magic via hash object' );
ok( exists $INC{'Quux.pm'},        '  %INC sees Quux.pm' );
is( ref $INC{'Quux.pm'}, 'FooLoader',
				   '  val Quux.pm is an object in %INC' );
is( $INC{'Quux.pm'}, $href,        '  val Quux.pm is correct in %INC' );

pop @INC;

my $aref = bless( [], 'FooLoader' );
push @INC, $aref;

$evalret = eval { require Quux1; 1 };
die $@ if $@;
ok( $evalret,                      'require Quux1; magic via array object' );
ok( exists $INC{'Quux1.pm'},       '  %INC sees Quux1.pm' );
is( ref $INC{'Quux1.pm'}, 'FooLoader',
				   '  val Quux1.pm is an object in %INC' );
is( $INC{'Quux1.pm'}, $aref,       '  val Quux1.pm  is correct in %INC' );

pop @INC;

my $sref = bless( \(my $x = 1), 'FooLoader' );
push @INC, $sref;

$evalret = eval { require Quux2; 1 };
die $@ if $@;
ok( $evalret,                      'require Quux2; magic via scalar object' );
ok( exists $INC{'Quux2.pm'},       '  %INC sees Quux2.pm' );
is( ref $INC{'Quux2.pm'}, 'FooLoader',
				   '  val Quux2.pm is an object in %INC' );
is( $INC{'Quux2.pm'}, $sref,       '  val Quux2.pm is correct in %INC' );

pop @INC;

push @INC, sub {
    my ($self, $filename) = @_;
    if (substr($filename,0,4) eq 'Toto') {
	$INC{$filename} = 'xyz';
	return get_temp_fh($filename);
    }
    else {
        return undef;
    }
};

$evalret = eval { require Toto; 1 };
die $@ if $@;
ok( $evalret,                      'require Toto; magic via anonymous code ref'  );
ok( exists $INC{'Toto.pm'},        '  %INC sees Toto.pm' );
ok( ! ref $INC{'Toto.pm'},         q/  val Toto.pm isn't a ref in %INC/ );
is( $INC{'Toto.pm'}, 'xyz',	   '  val Toto.pm is correct in %INC' );

pop @INC;

{
    my $autoloaded;
    package AutoInc {
        sub AUTOLOAD {
            my ($self, $filename) = @_;
            $autoloaded = our $AUTOLOAD;
            return ::get_temp_fh($filename);
        }
        sub DESTROY {}
    }

    push @INC, bless {}, "AutoInc";
    $evalret = eval { require Quux3; 1 };
    ok($evalret, "require Quux3 via AUTOLOADed INC");
    ok(exists $INC{"Quux3.pm"}, "Quux3 in %INC");
    is($autoloaded, "AutoInc::INC", "AUTOLOAD was called for INC");

    pop @INC;
}

push @INC, sub {
    my ($self, $filename) = @_;
    if ($filename eq 'abc.pl') {
	return get_temp_fh($filename, qq(return "abc";\n));
    }
    else {
	return undef;
    }
};

my $ret = "";
$ret ||= do 'abc.pl';
is( $ret, 'abc', 'do "abc.pl" sees return value' );

{
    my $got;
    #local @INC; # local fails on tied @INC
    my @old_INC = @INC; # because local doesn't work on tied arrays
    @INC =  ('lib', 'lib/Devel', sub { $got = $_[1]; return undef; });
    foreach my $filename ('/test_require.pm', './test_require.pm',
			  '../test_require.pm') {
	local %INC;
	undef $got;
	undef $test_require::loaded;
	eval { require $filename; };
	is($got, $filename, "the coderef sees the pathname $filename");
	is($test_require::loaded, undef, 'no module is loaded' );
    }

    local %INC;
    undef $got;
    undef $test_require::loaded;

    eval { require 'test_require.pm'; };
    is($got, undef, 'the directory is scanned for test_require.pm');
    is($test_require::loaded, 1, 'the module is loaded');
    @INC = @old_INC;
}

# this will segfault if it fails

sub PVBM () { 'foo' }
{ my $dummy = index 'foo', PVBM }

# I don't know whether these requires should succeed or fail. 5.8 failed
# all of them; 5.10 with an ordinary constant in place of PVBM lets the
# latter two succeed. For now I don't care, as long as they don't
# segfault :).

unshift @INC, sub { PVBM };
eval 'require foo';
ok( 1, 'returning PVBM doesn\'t segfault require' );
eval 'use foo';
ok( 1, 'returning PVBM doesn\'t segfault use' );
shift @INC;
unshift @INC, sub { \PVBM };
eval 'require foo';
ok( 1, 'returning PVBM ref doesn\'t segfault require' );
eval 'use foo';
ok( 1, 'returning PVBM ref doesn\'t segfault use' );
shift @INC;

# [perl #92252]
{
    my $die = sub { die };
    my $data = [];
    unshift @INC, sub { $die, $data };

    # + 1 to account for prototype-defeating &... calling convention
    my $initial_sub_refcnt = &Internals::SvREFCNT($die) + 1;
    my $initial_data_refcnt = &Internals::SvREFCNT($data) + 1;

    do "foo";
    refcount_is $die, $initial_sub_refcnt, "no leaks";
    refcount_is $data, $initial_data_refcnt, "no leaks";

    do "bar";
    refcount_is $die, $initial_sub_refcnt, "no leaks";
    refcount_is $data, $initial_data_refcnt, "no leaks";

    shift @INC;
}

unshift @INC, sub { \(my $tmp = '$_ = "are temps freed prematurely?"') };
eval { require foom };
is $_||$@, "are temps freed prematurely?",
           "are temps freed prematurely when returned from inc filters?";
shift @INC;

# [perl #120657]
sub fake_module {
    my (undef,$module_file) = @_;
    !1
}
{
    local @INC = @INC;
    @INC = (\&fake_module)x2;
    eval { require "${\'bralbalhablah'}" };
    like $@, qr/^Can't locate/,
        'require PADTMP passing freed var when @INC has multiple subs';
}    

SKIP: {
    skip ("Not applicable when run from inccode-tie.t", 6) if tied @INC;
    require Tie::Scalar;
    package INCtie {
        sub TIESCALAR { bless \my $foo }
        sub FETCH { study; our $count++; ${$_[0]} }
    }
    local @INC = undef;
    my $t = tie $INC[0], 'INCtie';
    my $called;
    $$t = sub { $called ++; !1 };
    delete $INC{'foo.pm'}; # in case another test uses foo
    eval { require foo };
    is $INCtie::count, 1,
        'FETCH is called once on undef scalar-tied @INC elem';
    is $called, 1, 'sub in scalar-tied @INC elem is called';
    () = "$INC[0]"; # force a fetch, so the SV is ROK
    $INCtie::count = 0;
    eval { require foo };
    is $INCtie::count, 1,
        'FETCH is called once on scalar-tied @INC elem holding ref';
    is $called, 2, 'sub in scalar-tied @INC elem holding ref is called';
    $$t = [];
    $INCtie::count = 0;
    eval { require foo };
    is $INCtie::count, 1,
       'FETCH called once on scalar-tied @INC elem returning array';
    $$t = "string";
    $INCtie::count = 0;
    eval { require foo };
    is $INCtie::count, 1,
       'FETCH called once on scalar-tied @INC elem returning string';
}


exit if is_miniperl();

SKIP: {
    skip( "No PerlIO available", 3 ) unless $has_perlio;
    pop @INC;

    push @INC, sub {
        my ($cr, $filename) = @_;
        my $module = $filename; $module =~ s,/,::,g; $module =~ s/\.pm$//;
        open my $fh, '<',
             \"package $module; sub complain { warn q() }; \$::file = __FILE__;"
	    or die $!;
        $INC{$filename} = "/custom/path/to/$filename";
        return $fh;
    };

    require Publius::Vergilius::Maro;
    is( $INC{'Publius/Vergilius/Maro.pm'},
        '/custom/path/to/Publius/Vergilius/Maro.pm', '%INC set correctly');
    is( our $file, '/custom/path/to/Publius/Vergilius/Maro.pm',
        '__FILE__ set correctly' );
    {
        my $warning;
        local $SIG{__WARN__} = sub { $warning = shift };
        Publius::Vergilius::Maro::complain();
        like( $warning, qr{something's wrong at /custom/path/to/Publius/Vergilius/Maro.pm}, 'warn() reports correct file source' );
    }
}
pop @INC;

if ($can_fork) {
    require PerlIO::scalar;
    # This little bundle of joy generates n more recursive use statements,
    # with each module chaining the next one down to 0. If it works, then we
    # can safely nest subprocesses
    my $use_filter_too;
    push @INC, sub {
	return unless $_[1] =~ /^BBBLPLAST(\d+)\.pm/;
	my $pid = open my $fh, "-|";
	if ($pid) {
	    # Parent
	    return $fh unless $use_filter_too;
	    # Try filters and state in addition.
	    return ($fh, sub {s/$_[1]/pass/; return}, "die")
	}
	die "Can't fork self: $!" unless defined $pid;

	# Child
	my $count = $1;
	# Lets force some fun with odd sized reads.
	$| = 1;
	print 'push @main::bbblplast, ';
	print "$count;\n";
	if ($count--) {
	    print "use BBBLPLAST$count;\n";
	}
	if ($use_filter_too) {
	    print "die('In $_[1]');";
	} else {
	    print "pass('In $_[1]');";
	}
	print '"Truth"';
	POSIX::_exit(0);
	die "Can't get here: $!";
    };

    @::bbblplast = ();
    require BBBLPLAST5;
    is ("@::bbblplast", "0 1 2 3 4 5", "All ran");

    foreach (keys %INC) {
	delete $INC{$_} if /^BBBLPLAST/;
    }

    @::bbblplast = ();
    $use_filter_too = 1;

    require BBBLPLAST5;

    is ("@::bbblplast", "0 1 2 3 4 5", "All ran with a filter");
}
SKIP:{
    skip "need fork",1 unless $can_fork;
    fresh_perl_like('@INC=("A",bless({},"Hook"),"D"); '
                 .'sub Hook::INCDIR { return "B","C"} '
                 .'eval "require Frobnitz" or print $@;',
                  qr/\(\@INC[\w ]+: A Hook=HASH\(0x[A-Fa-f0-9]+\) B C D\)/,
                  {},
                  "Check if INCDIR hook works as expected");
}
