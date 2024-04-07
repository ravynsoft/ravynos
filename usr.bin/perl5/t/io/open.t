#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

$|  = 1;
use warnings;
use Config;

plan tests => 188;

sub ok_cloexec {
    SKIP: {
	skip "no fcntl", 1 unless $Config{d_fcntl};
	my $fd = fileno($_[0]);
	fresh_perl_is(qq(
	    print open(F, "+<&=$fd") ? 1 : 0, "\\n";
	), "0\n", {}, "not inherited across exec");
    }
}

my $Perl = which_perl();

my $afile = tempfile();
{
    unlink($afile) if -f $afile;

    $! = 0;  # the -f above will set $! if $afile doesn't exist.
    ok( open(my $f,"+>$afile"),  'open(my $f, "+>...")' );
    ok_cloexec($f);

    binmode $f;
    ok( -f $afile,              '       its a file');
    ok( (print $f "SomeData\n"),  '       we can print to it');
    is( tell($f), 9,            '       tell()' );
    ok( seek($f,0,0),           '       seek set' );

    $b = <$f>;
    is( $b, "SomeData\n",       '       readline' );
    ok( -f $f,                  '       still a file' );

    eval  { die "Message" };
    like( $@, qr/<\$f> line 1/, '       die message correct' );
    
    ok( close($f),              '       close()' );
    ok( unlink($afile),         '       unlink()' );
}

{
    ok( open(my $f,'>', $afile),        "open(my \$f, '>', $afile)" );
    ok_cloexec($f);
    ok( (print $f "a row\n"),           '       print');
    ok( close($f),                      '       close' );
    ok( -s $afile < 10,                 '       -s' );
}

{
    ok( open(my $f,'>>', $afile),       "open(my \$f, '>>', $afile)" );
    ok_cloexec($f);
    ok( (print $f "a row\n"),           '       print' );
    ok( close($f),                      '       close' );
    ok( -s $afile > 10,                 '       -s'    );
}

{
    ok( open(my $f, '<', $afile),       "open(my \$f, '<', $afile)" );
    ok_cloexec($f);
    my @rows = <$f>;
    is( scalar @rows, 2,                '       readline, list context' );
    is( $rows[0], "a row\n",            '       first line read' );
    is( $rows[1], "a row\n",            '       second line' );
    ok( close($f),                      '       close' );
}

{
    ok( -s $afile < 20,                 '-s' );

    ok( open(my $f, '+<', $afile),      'open +<' );
    ok_cloexec($f);
    my @rows = <$f>;
    is( scalar @rows, 2,                '       readline, list context' );
    ok( seek($f, 0, 1),                 '       seek cur' );
    ok( (print $f "yet another row\n"), '       print' );
    ok( close($f),                      '       close' );
    ok( -s $afile > 20,                 '       -s' );

    unlink($afile);
}
{
    ok( open(my $f, '-|', <<EOC),     'open -|' );
    $Perl -e "print qq(a row\\n); print qq(another row\\n)"
EOC

    ok_cloexec($f);
    my @rows = <$f>;
    is( scalar @rows, 2,                '       readline, list context' );
    ok( close($f),                      '       close' );
}
{
    ok( open(my $f, '|-', <<EOC),     'open |-' );
    $Perl -pe "s/^not //"
EOC

    ok_cloexec($f);
    my @rows = <$f>;
    my $test = curr_test;
    print $f "not ok $test - piped in\n";
    next_test;

    $test = curr_test;
    print $f "not ok $test - piped in\n";
    next_test;
    ok( close($f),                      '       close' );
    sleep 1;
    pass('flushing');
}


ok( !eval { open my $f, '<&', $afile; 1; },    '<& on a non-filehandle' );
like( $@, qr/Bad filehandle:\s+$afile/,          '       right error' );

ok( !eval { *some_glob = 1; open my $f, '<&', *some_glob; 1; },    '<& on a non-filehandle glob' );
like( $@, qr/Bad filehandle:\s+some_glob/,          '       right error' );

{
    use utf8;
    use open qw( :utf8 :std );
    ok( !eval { use utf8; *ǡﬁlḛ = 1; open my $f, '<&', *ǡﬁlḛ; 1; },    '<& on a non-filehandle glob' );
    like( $@, qr/Bad filehandle:\s+ǡﬁlḛ/u,          '       right error' );
}

# local $file tests
{
    unlink($afile) if -f $afile;

    ok( open(local $f,"+>$afile"),       'open local $f, "+>", ...' );
    ok_cloexec($f);
    binmode $f;

    ok( -f $afile,                      '       -f' );
    ok( (print $f "SomeData\n"),        '       print' );
    is( tell($f), 9,                    '       tell' );
    ok( seek($f,0,0),                   '       seek set' );

    $b = <$f>;
    is( $b, "SomeData\n",               '       readline' );
    ok( -f $f,                          '       still a file' );

    eval  { die "Message" };
    like( $@, qr/<\$f> line 1/,         '       proper die message' );
    ok( close($f),                      '       close' );

    unlink($afile);
}

{
    ok( open(local $f,'>', $afile),     'open local $f, ">", ...' );
    ok_cloexec($f);
    ok( (print $f "a row\n"),           '       print');
    ok( close($f),                      '       close');
    ok( -s $afile < 10,                 '       -s' );
}

{
    ok( open(local $f,'>>', $afile),    'open local $f, ">>", ...' );
    ok_cloexec($f);
    ok( (print $f "a row\n"),           '       print');
    ok( close($f),                      '       close');
    ok( -s $afile > 10,                 '       -s' );
}

{
    ok( open(local $f, '<', $afile),    'open local $f, "<", ...' );
    ok_cloexec($f);
    my @rows = <$f>;
    is( scalar @rows, 2,                '       readline list context' );
    ok( close($f),                      '       close' );
}

ok( -s $afile < 20,                     '       -s' );

{
    ok( open(local $f, '+<', $afile),  'open local $f, "+<", ...' );
    ok_cloexec($f);
    my @rows = <$f>;
    is( scalar @rows, 2,                '       readline list context' );
    ok( seek($f, 0, 1),                 '       seek cur' );
    ok( (print $f "yet another row\n"), '       print' );
    ok( close($f),                      '       close' );
    ok( -s $afile > 20,                 '       -s' );

    unlink($afile);
}

{
    ok( open(local $f, '-|', <<EOC),  'open local $f, "-|", ...' );
    $Perl -e "print qq(a row\\n); print qq(another row\\n)"
EOC
    ok_cloexec($f);
    my @rows = <$f>;

    is( scalar @rows, 2,                '       readline list context' );
    ok( close($f),                      '       close' );
}

{
    ok( open(local $f, '|-', <<EOC),  'open local $f, "|-", ...' );
    $Perl -pe "s/^not //"
EOC

    ok_cloexec($f);
    my @rows = <$f>;
    my $test = curr_test;
    print $f "not ok $test - piping\n";
    next_test;

    $test = curr_test;
    print $f "not ok $test - piping\n";
    next_test;
    ok( close($f),                      '       close' );
    sleep 1;
    pass("Flush");
}


ok( !eval { open local $f, '<&', $afile; 1 },  'local <& on non-filehandle');
like( $@, qr/Bad filehandle:\s+$afile/,          '       right error' );

{
    local *F;
    for (1..2) {
	ok( open(F, qq{$Perl -le "print 'ok'"|}), 'open to pipe' );
	ok_cloexec(\*F);
	is(scalar <F>, "ok\n",  '       readline');
	ok( close F,            '       close' );
    }

    for (1..2) {
	ok( open(F, "-|", qq{$Perl -le "print 'ok'"}), 'open -|');
	ok_cloexec(\*F);
	is( scalar <F>, "ok\n", '       readline');
	ok( close F,            '       close' );
    }
}


# other dupping techniques
{
    ok( open(my $stdout, ">&", \*STDOUT),       'dup \*STDOUT into lexical fh');
    ok_cloexec($stdout);
    ok( open(STDOUT,     ">&", $stdout),        'restore dupped STDOUT from lexical fh');

    {
	use strict; # the below should not warn
	ok( open(my $stdout, ">&", STDOUT),         'dup STDOUT into lexical fh');
	ok_cloexec($stdout);
    }

    # used to try to open a file [perl #17830]
    ok( open(my $stdin,  "<&", fileno STDIN),   'dup fileno(STDIN) into lexical fh') or _diag $!;
    ok_cloexec($stdin);

    fileno(STDIN) =~ /(.)/;
    ok open($stdin, "<&", $1), 'open ... "<&", $magical_fileno',
	||  _diag $!;
    ok_cloexec($stdin);
}

SKIP: {
    skip "This perl uses perlio", 1 if $Config{useperlio};
    skip_if_miniperl("miniperl can't rely on loading %Errno", 1);
    # Force the reference to %! to be run time by writing ! as {"!"}
    skip "This system doesn't understand EINVAL", 1
	unless exists ${"!"}{EINVAL};

    no warnings 'io';
    ok(!open(F,'>',\my $s) && ${"!"}{EINVAL}, 'open(reference) raises EINVAL');
}

{
    ok( !eval { open F, "BAR", "QUUX" },       'Unknown open() mode' );
    like( $@, qr/\QUnknown open() mode 'BAR'/, '       right error' );
}

{
    local $SIG{__WARN__} = sub { $@ = shift };

    sub gimme {
        my $tmphandle = shift;
	my $line = scalar <$tmphandle>;
	warn "gimme";
	return $line;
    }

    open($fh0[0], "TEST");
    ok_cloexec($fh0[0]);
    gimme($fh0[0]);
    like($@, qr/<\$fh0\[...\]> line 1\./, "autoviv fh package aelem");

    open($fh1{k}, "TEST");
    ok_cloexec($fh1{h});
    gimme($fh1{k});
    like($@, qr/<\$fh1\{...}> line 1\./, "autoviv fh package helem");

    my @fh2;
    open($fh2[0], "TEST");
    ok_cloexec($fh2[0]);
    gimme($fh2[0]);
    like($@, qr/<\$fh2\[...\]> line 1\./, "autoviv fh lexical aelem");

    my %fh3;
    open($fh3{k}, "TEST");
    ok_cloexec($fh3{h});
    gimme($fh3{k});
    like($@, qr/<\$fh3\{...}> line 1\./, "autoviv fh lexical helem");

    local $/ = *F;  # used to cause an assertion failure
    gimme($fh3{k});
    like($@, qr/<\$fh3\{...}> chunk 2\./,
	'<...> line 1 when $/ is set to a glob');
}
    
SKIP: {
    skip("These tests use perlio", 5) unless $Config{useperlio};
    my $w;
    use warnings 'layer';
    local $SIG{__WARN__} = sub { $w = shift };

    eval { open(F, ">>>", $afile) };
    like($w, qr/Invalid separator character '>' in PerlIO layer spec/,
	 "bad open (>>>) warning");
    like($@, qr/Unknown open\(\) mode '>>>'/,
	 "bad open (>>>) failure");

    eval { open(F, ">:u", $afile ) };
    like($w, qr/Unknown PerlIO layer "u"/,
	 'bad layer ">:u" warning');
    eval { open(F, "<:u", $afile ) };
    like($w, qr/Unknown PerlIO layer "u"/,
	 'bad layer "<:u" warning');
    eval { open(F, ":c", $afile ) };
    like($@, qr/Unknown open\(\) mode ':c'/,
	 'bad layer ":c" failure');
}

# [perl #28986] "open m" crashes Perl

fresh_perl_like('open m', qr/^Search pattern not terminated at/,
	{ stderr => 1 }, 'open m test');

fresh_perl_is(
    'sub f { open(my $fh, "xxx"); $fh = "f"; } f; f;print "ok"',
    'ok', { stderr => 1 },
    '#29102: Crash on assignment to lexical filehandle');

# [perl #31767] Using $1 as a filehandle via open $1, "file" doesn't raise
# an exception

eval { open $99, "foo" };
like($@, qr/Modification of a read-only value attempted/, "readonly fh");
# But we do not want that exception applying to close(), since it does not
# modify the fh.
eval {
   no warnings "uninitialized";
   # make sure $+ is undefined
   "a" =~ /(b)?/;
   close $+
};
is($@, '', 'no "Modification of a read-only value" when closing');

# [perl#73626] mg_get wasn't run on the pipe arg

{
    package p73626;
    sub TIESCALAR { bless {} }
    sub FETCH { "$Perl -e 1"}

    tie my $p, 'p73626';

    package main;

    ok( open(my $f, '-|', $p),     'open -| magic');
}

# [perl #77492] Crash when stringifying a glob, a reference to which has
#               been opened and written to.
fresh_perl_is(
    '
      open my $fh, ">", \*STDOUT;
      print $fh "hello";
     "".*STDOUT;
      print "ok";
      close $fh;
      unlink \*STDOUT;
    ',
    'ok', { stderr => 1 },
    '[perl #77492]: open $fh, ">", \*glob causes SEGV');

# [perl #77684] Opening a reference to a glob copy.
SKIP: {
    skip_if_miniperl("no dynamic loading on miniperl, so can't load PerlIO::scalar", 1);
    my $var = *STDOUT;
    open my $fh, ">", \$var;
    print $fh "hello";
    is $var, "hello", '[perl #77684]: open $fh, ">", \$glob_copy'
        # when this fails, it leaves an extra file:
        or unlink \*STDOUT;
}

# check that we can call methods on filehandles auto-magically
# and have IO::File loaded for us
SKIP: {
    skip_if_miniperl("no dynamic loading on miniperl, so can't load IO::File", 3);
    is( $INC{'IO/File.pm'}, undef, "IO::File not loaded" );
    my $var = "";
    open my $fh, ">", \$var;
    ok( eval { $fh->autoflush(1); 1 }, '$fh->autoflush(1) lives' );
    ok( $INC{'IO/File.pm'}, "IO::File now loaded" );
}

sub _117941 { package _117941; open my $a, "TEST" }
delete $::{"_117941::"};
_117941();
pass("no crash when open autovivifies glob in freed package");

# [perl #117265] check for embedded nul in pathnames, allow ending \0 though
{
    my $WARN;
    local $SIG{__WARN__} = sub { $WARN = shift };
    my $temp = tempfile();
    my $temp_match = quotemeta $temp;

    # create the file, so we can check nothing actually touched it
    open my $temp_fh, ">", $temp;
    close $temp_fh;
    ok(utime(time()-10, time(), $temp), "set mtime to a known value");
    ok(chmod(0666, $temp), "set mode to a known value");
    my ($final_mode, $final_mtime) = (stat $temp)[2, 9];

    my $fn = "$temp\0.invalid";
    my $fno = bless \(my $fn2 = "$temp\0.overload"), "OverloadTest";
    is(open(I, $fn), undef, "open with nul in pathnames since 5.18 [perl #117265]");
    like($WARN, qr/^Invalid \\0 character in pathname for open: $temp_match\\0\.invalid/,
         "warn on embedded nul"); $WARN = '';
    is(open(I, $fno), undef, "open with nul in pathnames since 5.18 [perl #117265] (overload)");
    like($WARN, qr/^Invalid \\0 character in pathname for open: $temp_match\\0\.overload/,
         "warn on embedded nul"); $WARN = '';

    is(chmod(0444, $fn), 0, "chmod fails with \\0 in name");
    like($WARN, qr/^Invalid \\0 character in pathname for chmod: $temp_match\\0\.invalid/,
         "also on chmod"); $WARN = '';

    is(chmod(0444, $fno), 0, "chmod fails with \\0 in name (overload)");
    like($WARN, qr/^Invalid \\0 character in pathname for chmod: $temp_match\\0\.overload/,
         "also on chmod"); $WARN = '';

    is (glob($fn), undef, "glob fails with \\0 in name");
    like($WARN, qr/^Invalid \\0 character in pattern for glob: $temp_match\\0\.invalid/,
         "also on glob"); $WARN = '';

    is (glob($fno), undef, "glob fails with \\0 in name (overload)");
    like($WARN, qr/^Invalid \\0 character in pattern for glob: $temp_match\\0\.overload/,
         "also on glob"); $WARN = '';

    {
        no warnings 'syscalls';
        $WARN = '';
        is(open(I, $fn), undef, "open with nul with no warnings syscalls");
        is($WARN, '', "ignore warning on embedded nul with no warnings syscalls");
    }

    SKIP: {
        if (is_miniperl && !eval 'require Errno') {
            skip "Errno not built yet", 8;
        }
        require Errno;
        import Errno 'ENOENT';
        # check handling of multiple arguments, which the original patch
        # mis-handled
        $! = 0;
        is (unlink($fn, $fn), 0, "check multiple arguments to unlink");
        is($!+0, &ENOENT, "check errno");
        $! = 0;
        is (chmod(0644, $fn, $fn), 0, "check multiple arguments to chmod");
        is($!+0, &ENOENT, "check errno");
        $! = 0;
        is (utime(time, time, $fn, $fn), 0, "check multiple arguments to utime");
        is($!+0, &ENOENT, "check errno");
        SKIP: {
            skip "no chown", 2 unless $Config{d_chown};
            $! = 0;
            is(chown(-1, -1, $fn, $fn), 0, "check multiple arguments to chown");
            is($!+0, &ENOENT, "check errno");
        }
    }

    is (unlink($fn), 0, "unlink fails with \\0 in name");
    like($WARN, qr/^Invalid \\0 character in pathname for unlink: $temp_match\\0\.invalid/,
         "also on unlink"); $WARN = '';

    is (unlink($fno), 0, "unlink fails with \\0 in name (overload)");
    like($WARN, qr/^Invalid \\0 character in pathname for unlink: $temp_match\\0\.overload/,
         "also on unlink"); $WARN = '';

    ok(-f $temp, "nothing removed the temp file");
    is((stat $temp)[2], $final_mode, "nothing changed its mode");
    is((stat $temp)[9], $final_mtime, "nothing changes its mtime");
}

# [perl #125115] Dup to closed filehandle creates file named GLOB(0x...)
{
    ok(open(my $fh, "<", "TEST"), "open a handle");
    ok(close $fh, "and close it again");
    ok(!open(my $fh2,  ">&", $fh), "should fail to dup the closed handle");
    # clean up if we failed
    unlink "$fh";
}

{
    package OverloadTest;
    use overload '""' => sub { ${$_[0]} };
}

# [perl #115814] open(${\$x}, ...) creates spurious reference to handle in stash
SKIP: {
    # The bug doesn't depend on perlio, but perlio provides this nice
    # way of discerning when a handle actually closes.
    skip("These tests use perlio", 5) unless $Config{useperlio};
    skip_if_miniperl("miniperl can't load PerlIO::scalar", 5);
    my($a, $b, $s, $t);
    $s = "";
    open($a, ">:scalar:perlio", \$s) or die;
    print {$a} "abc";
    is $s, "", "buffering delays writing to scalar (simple open)";
    $a = undef;
    is $s, "abc", "buffered write happens on dropping handle ref (simple open)";
    $t = "";
    open(${\$b}, ">:scalar:perlio", \$t) or die;
    print {$b} "xyz";
    is $t, "", "buffering delays writing to scalar (complex open)";
    $b = undef;
    is $t, "xyz", "buffered write happens on dropping handle ref (complex open)";
    is scalar(grep { /\A_GEN_/ } keys %::), 0, "no gensym appeared in stash";
}

# [perl #16113] returning handle in localised glob
{
    my $tfile = tempfile();
    open(my $twrite, ">", $tfile) or die $!;
    print {$twrite} "foo\nbar\n" or die $!;
    close $twrite or die $!;
    $twrite = undef;
    my $tread = do {
	local *F;
	open(F, "<", $tfile) or die $!;
	*F;
    };
    is scalar(<$tread>), "foo\n", "IO handle returned in localised glob";
    close $tread;
}
