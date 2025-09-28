use strict;
use warnings;

BEGIN {
    if( $ENV{PERL_CORE} ) {
        chdir 't' if -d 't';
        unshift @INC, '../lib';
    }
    else {
        unshift @INC, 't/lib';
    }
    $ENV{PERL_MM_MANIFEST_VERBOSE}=1;
}
chdir 't';

use Test::More tests => 98;
use Cwd;

use File::Spec;
use File::Path;
use File::Find;
use Config;

my $Is_VMS = $^O eq 'VMS';
my $Is_VMS_noefs = $Is_VMS;
if ($Is_VMS) {
    my $vms_efs = 0;
    if (eval 'require VMS::Feature') {
        $vms_efs = VMS::Feature::current("efs_charset");
    } else {
        my $efs_charset = $ENV{'DECC$EFS_CHARSET'} || '';
        $vms_efs = $efs_charset =~ /^[ET1]/i;
    }
    $Is_VMS_noefs = 0 if $vms_efs;
}

# We're going to be chdir'ing and modules are sometimes loaded on the
# fly in this test, so we need an absolute @INC.
@INC = map File::Spec->rel2abs($_), @INC;

# keep track of everything added so it can all be deleted
my %Files;
sub add_file {
    my ($file, $data) = @_;
    $data ||= 'foo';
    $file =~ s/ /^_/g if $Is_VMS_noefs; # escape spaces
    1 while unlink $file;  # or else we'll get multiple versions on VMS
    open( T, '> '.$file) or return;
    binmode T, ':raw'; # no CRLFs please
    print T $data;
    close T;
    return 0 unless -e $file;  # exists under the name we gave it ?
    ++$Files{$file};
}

sub read_manifest {
    open( M, 'MANIFEST' ) or return;
    chomp( my @files = <M> );
    close M;
    return @files;
}

sub catch_warning {
    my $warn = '';
    local $SIG{__WARN__} = sub { $warn .= $_[0] };
    return join('', $_[0]->() ), $warn;
}

sub remove_dir {
    ok( rmdir( $_ ), "remove $_ directory" ) for @_;
}

# use module, import functions
BEGIN {
    use_ok( 'ExtUtils::Manifest',
            qw( mkmanifest manicheck filecheck fullcheck
                maniread manicopy skipcheck maniadd maniskip) );
}

my $cwd = Cwd::getcwd();

# Just in case any old files were lying around.
rmtree('mantest');

ok( mkdir( 'mantest', 0777 ), 'make mantest directory' );
ok( chdir( 'mantest' ), 'chdir() to mantest' );
ok( add_file('foo'), 'add a temporary file' );

# This ensures the -x check for manicopy means something
# Some platforms don't have chmod or an executable bit, in which case
# this call will do nothing or fail, but on the platforms where chmod()
# works, we test the executable bit is copied
chmod( 0744, 'foo') if $Config{'chmod'};

# there shouldn't be a MANIFEST there
my ($res, $warn) = catch_warning( \&mkmanifest );
# Canonize the order.
$warn = join("", map "$_|",
                 sort { lc($a) cmp lc($b) } split /\r?\n/, $warn);
is( $warn, "Added to MANIFEST: foo|Added to MANIFEST: MANIFEST|",
    "mkmanifest() displayed its additions" );

# and now you see it
ok( -e 'MANIFEST', 'create MANIFEST file' );

my @list = read_manifest();
is( @list, 2, 'check files in MANIFEST' );
ok( ! ExtUtils::Manifest::filecheck(), 'no additional files in directory' );

# after adding bar, the MANIFEST is out of date
ok( add_file( 'bar' ), 'add another file' );
ok( ! manicheck(), 'MANIFEST now out of sync' );

# it reports that bar has been added and throws a warning
($res, $warn) = catch_warning( \&filecheck );

like( $warn, qr/^Not in MANIFEST: bar/, 'warning that bar has been added' );
is( $res, 'bar', 'bar reported as new' );

# now quiet the warning that bar was added and test again
($res, $warn) = do { local $ExtUtils::Manifest::Quiet = 1;
                     catch_warning( \&skipcheck )
                };
is( $warn, '', 'disabled warnings' );

# add a skip file with a rule to skip itself (and the nonexistent glob '*baz*')
add_file( 'MANIFEST.SKIP', "baz\n.SKIP" );

# this'll skip the new file
($res, $warn) = catch_warning( \&skipcheck );
like( $warn, qr/^Skipping MANIFEST\.SKIP/i, 'got skipping warning' );

my @skipped;
catch_warning( sub {
	@skipped = skipcheck()
});

is( join( ' ', @skipped ), 'MANIFEST.SKIP', 'listed skipped files' );

{
	local $ExtUtils::Manifest::Quiet = 1;
	is( join(' ', filecheck() ), 'bar', 'listing skipped with filecheck()' );
}

# add a subdirectory and a file there that should be found
ok( mkdir( 'moretest', 0777 ), 'created moretest directory' );
add_file( File::Spec->catfile('moretest', 'quux'), 'quux' );
ok( exists( ExtUtils::Manifest::manifind()->{'moretest/quux'} ),
                                        "manifind found moretest/quux" );

# only MANIFEST and foo are in the manifest
$_ = 'foo';
my $files = maniread();
is( keys %$files, 2, 'two files found' );
is( join(' ', sort { lc($a) cmp lc($b) } keys %$files), 'foo MANIFEST',
                                        'both files found' );
is( $_, 'foo', q{maniread() doesn't clobber $_} );

ok( mkdir( 'copy', 0777 ), 'made copy directory' );

# Check that manicopy copies files.
manicopy( $files, 'copy', 'cp' );
my @copies = ();
find( sub { push @copies, $_ if -f }, 'copy' );
@copies = map { s/\.$//; $_ } @copies if $Is_VMS;  # VMS likes to put dots on
                                                   # the end of files.
# Have to compare insensitively for non-case preserving VMS
is_deeply( [sort map lc, @copies], [sort map lc, keys %$files] );

# cp would leave files readonly, so check permissions.
foreach my $orig (@copies) {
    my $copy = "copy/$orig";
    ok( -r $copy,               "$copy: must be readable" );
    is( -w $copy, -w $orig,     "       writable if original was" );
    is( -x $copy, -x $orig,     "       executable if original was" );
}
rmtree('copy');


# poison the manifest, and add a comment that should be reported
add_file( 'MANIFEST', 'none #none' );
is( ExtUtils::Manifest::maniread()->{none}, '#none',
                                        'maniread found comment' );

ok( mkdir( 'copy', 0777 ), 'made copy directory' );
$files = maniread();
eval { (undef, $warn) = catch_warning( sub {
		manicopy( $files, 'copy', 'cp' ) })
};

# a newline comes through, so get rid of it
chomp($warn);
# the copy should have given a warning
like($warn, qr/^none not found/, 'carped about none' );
($res, $warn) = catch_warning( \&skipcheck );
like($warn, qr/^Skipping MANIFEST.SKIP/i, 'warned about MANIFEST.SKIP' );

# tell ExtUtils::Manifest to use a different file
{
	local $ExtUtils::Manifest::MANIFEST = 'albatross';
	($res, $warn) = catch_warning( \&mkmanifest );
	like( $warn, qr/Added to albatross: /, 'using a new manifest file' );

	# add the new file to the list of files to be deleted
	$Files{'albatross'}++;
}


# Make sure MANIFEST.SKIP is using complete relative paths
add_file( 'MANIFEST.SKIP' => "^moretest/q\n" );

# This'll skip moretest/quux
($res, $warn) = catch_warning( \&skipcheck );
like( $warn, qr{^Skipping moretest/quux$}i, 'got skipping warning again' );


# There was a bug where entries in MANIFEST would be blotted out
# by MANIFEST.SKIP rules.
add_file( 'MANIFEST.SKIP' => 'foo' );
add_file( 'MANIFEST'      => "foobar\n"   );
add_file( 'foobar'        => '123' );
($res, $warn) = catch_warning( \&manicheck );
is( $res,  '',      'MANIFEST overrides MANIFEST.SKIP' );
is( $warn, '',   'MANIFEST overrides MANIFEST.SKIP, no warnings' );

$files = maniread;
ok( !$files->{wibble},     'MANIFEST in good state' );
maniadd({ wibble => undef });
maniadd({ yarrow => "hock" });
$files = maniread;
is( $files->{wibble}, '',    'maniadd() with undef comment' );
is( $files->{yarrow}, 'hock','          with comment' );
is( $files->{foobar}, '',    '          preserved old entries' );

my $manicontents = do {
  local $/;
  open my $fh, "MANIFEST" or die;
  binmode $fh, ':raw';
  <$fh>
};
is index($manicontents, "\015\012"), -1, 'MANIFEST no CRLF';

{
    # EOL normalization in maniadd()

    # move manifest away:
    rename "MANIFEST", "MANIFEST.bak" or die "Could not rename MANIFEST to MANIFEST.bak: $!";
    my $prev_maniaddresult;
    my @eol = ("\012","\015","\015\012");
    # for all line-endings:
    for my $i (0..$#eol) {
        my $eol = $eol[$i];
        #   cp the backup of the manifest to MANIFEST, line-endings adjusted
        my $content = do { local $/; open my $fh, "MANIFEST.bak" or die; <$fh> };
    SPLITTER: for my $eol2 (@eol) {
            if ( index($content, $eol2) > -1 ) {
                my @lines = split /$eol2/, $content;
                pop @lines while $lines[-1] eq "";
                open my $fh, ">", "MANIFEST" or die "Could not open >MANIFEST: $!";
                print $fh map "$_$eol", @lines;
                close $fh or die "Could not close: $!";
                last SPLITTER;
            }
        }
        #   try maniadd
        maniadd({eoltest => "end of line normalization test"});
        #   slurp result and compare to previous result
        my $maniaddresult = do { local $/; open my $fh, "MANIFEST" or die; <$fh> };
        if ($prev_maniaddresult) {
            if ( $maniaddresult eq $prev_maniaddresult ) {
                pass "normalization success with i=$i";
            } else {
                require Data::Dumper;
                no warnings "once";
                local $Data::Dumper::Useqq = 1;
                local $Data::Dumper::Terse = 1;
                is Data::Dumper::Dumper($maniaddresult), Data::Dumper::Dumper($prev_maniaddresult), "eol normalization failed with i=$i";
            }
        }
        $prev_maniaddresult = $maniaddresult;
    }
    # move backup over MANIFEST
    rename "MANIFEST.bak", "MANIFEST" or die "Could not rename MANIFEST.bak to MANIFEST: $!";
}

my %funky_files;
# test including a filename with a space
SKIP: {
    add_file( 'foo bar' => "space" )
        or skip "couldn't create spaced test file", 2;
    local $ExtUtils::Manifest::MANIFEST = "albatross";
    maniadd({ 'foo bar' => "contains space"});
    is( maniread()->{'foo bar'}, "contains space",
	'spaced manifest filename' );
    add_file( 'albatross.bak', '' );
    ($res, $warn) = catch_warning( \&mkmanifest );
    like( $warn, qr/\A(Added to.*\n)+\z/m,
	  'no warnings about funky filename' );
    $funky_files{'space'} = 'foo bar';
}

# test including a filename with a space and a quote
SKIP: {
    add_file( 'foo\' baz\'quux' => "quote" )
        or skip "couldn't create quoted test file", 1;
    local $ExtUtils::Manifest::MANIFEST = "albatross";
    maniadd({ 'foo\' baz\'quux' => "contains quote"});
    is( maniread()->{'foo\' baz\'quux'}, "contains quote",
	'quoted manifest filename' );
    $funky_files{'space_quote'} = 'foo\' baz\'quux';
}

# test including a filename with a space and a backslash
SKIP: {
    add_file( 'foo bar\\baz' => "backslash" )
        or skip "couldn't create backslash test file", 1;
    local $ExtUtils::Manifest::MANIFEST = "albatross";
    maniadd({ 'foo bar\\baz' => "contains backslash"});
    is( maniread()->{'foo bar\\baz'}, "contains backslash",
	'backslashed manifest filename' );
    $funky_files{'space_backslash'} = 'foo bar\\baz';
}

# test including a filename with a space, quote, and a backslash
SKIP: {
    add_file( 'foo bar\\baz\'quux' => "backslash/quote" )
        or skip "couldn't create backslash/quote test file", 1;
    local $ExtUtils::Manifest::MANIFEST = "albatross";
    maniadd({ 'foo bar\\baz\'quux' => "backslash and quote"});
    is( maniread()->{'foo bar\\baz\'quux'}, "backslash and quote",
	'backslashed and quoted manifest filename' );
    $funky_files{'space_quote_backslash'} = 'foo bar\\baz\'quux';
}

# test including a filename which is itself a quoted string
# https://rt.perl.org/Ticket/Display.html?id=122415
SKIP: {
    my $quoted_filename = q{'quoted name.txt'};
    my $description     = "quoted string";
    add_file( $quoted_filename  => $description )
        or skip "couldn't create $description test file", 1;
    local $ExtUtils::Manifest::MANIFEST = "albatross";
    maniadd({ $quoted_filename => $description });
    is( maniread()->{$quoted_filename}, $description,
     'file whose name starts and ends with quotes' );
    $funky_files{$description} = $quoted_filename;
}

my @funky_keys = qw(space space_quote space_backslash space_quote_backslash);
# test including an external manifest.skip file in MANIFEST.SKIP
{
    maniadd({ foo => undef , albatross => undef,
              'mymanifest.skip' => undef, 'mydefault.skip' => undef});
    for (@funky_keys) {
        maniadd( {$funky_files{$_} => $_} ) if defined $funky_files{$_};
    }

    add_file('mymanifest.skip' => "^foo\n");
    add_file('mydefault.skip'  => "^my\n");
    local $ExtUtils::Manifest::DEFAULT_MSKIP =
         File::Spec->catfile($cwd, qw(mantest mydefault.skip));
    my $skip = File::Spec->catfile($cwd, qw(mantest mymanifest.skip));
    add_file('MANIFEST.SKIP' =>
             "albatross\n#!include $skip\n#!include_default");
    my ($res, $warn) = catch_warning( \&skipcheck );
    for (qw(albatross foo foobar mymanifest.skip mydefault.skip)) {
        like( $warn, qr/Skipping \b$_\b/,
              "Skipping $_" );
    }
    for my $funky_key (@funky_keys) {
        SKIP: {
            my $funky_file = $funky_files{$funky_key};
	    skip "'$funky_key' not created", 1 unless $funky_file;
	    like( $warn, qr/Skipping \b\Q$funky_file\E\b/,
	      "Skipping $funky_file");
	}
    }
    ($res, $warn) = catch_warning( \&mkmanifest );
    for (qw(albatross foo foobar mymanifest.skip mydefault.skip)) {
        like( $warn, qr/Removed from MANIFEST: \b$_\b/,
              "Removed $_ from MANIFEST" );
    }
    for my $funky_key (@funky_keys) {
        SKIP: {
            my $funky_file = $funky_files{$funky_key};
	    skip "'$funky_key' not created", 1 unless $funky_file;
	    like( $warn, qr/Removed from MANIFEST: \b\Q$funky_file\E\b/,
	      "Removed $funky_file from MANIFEST");
	}
    }
    my $files = maniread;
    ok( ! exists $files->{albatross}, 'albatross excluded via MANIFEST.SKIP' );
    ok( exists $files->{yarrow},      'yarrow included in MANIFEST' );
    ok( exists $files->{bar},         'bar included in MANIFEST' );
    ok( ! exists $files->{foobar},    'foobar excluded via mymanifest.skip' );
    ok( ! exists $files->{foo},       'foo excluded via mymanifest.skip' );
    ok( ! exists $files->{'mymanifest.skip'},
        'mymanifest.skip excluded via mydefault.skip' );
    ok( ! exists $files->{'mydefault.skip'},
        'mydefault.skip excluded via mydefault.skip' );

    # test exclusion of funky files
    for my $funky_key (@funky_keys) {
        SKIP: {
            my $funky_file = $funky_files{$funky_key};
	    skip "'$funky_key' not created", 1 unless $funky_file;
	    ok( ! exists $files->{$funky_file},
		  "'$funky_file' excluded via mymanifest.skip" );
	}
    }

    # tests for maniskip
    my $skipchk = maniskip();
    is ( $skipchk->('albatross'), 1,
	'albatross excluded via MANIFEST.SKIP' );
    is( $skipchk->('yarrow'), '',
	'yarrow included in MANIFEST' );
    is( $skipchk->('bar'), '',
	'bar included in MANIFEST' );
    $skipchk = maniskip('mymanifest.skip');
    is( $skipchk->('foobar'), 1,
	'foobar excluded via mymanifest.skip' );
    is( $skipchk->('foo'), 1,
	'foo excluded via mymanifest.skip' );
    is( $skipchk->('mymanifest.skip'), '',
        'mymanifest.skip included via mydefault.skip' );
    is( $skipchk->('mydefault.skip'), '',
        'mydefault.skip included via mydefault.skip' );
    $skipchk = maniskip('mydefault.skip');
    is( $skipchk->('foobar'), '',
	'foobar included via mydefault.skip' );
    is( $skipchk->('foo'), '',
	'foo included via mydefault.skip' );
    is( $skipchk->('mymanifest.skip'), 1,
        'mymanifest.skip excluded via mydefault.skip' );
    is( $skipchk->('mydefault.skip'), 1,
        'mydefault.skip excluded via mydefault.skip' );

    my $extsep = $Is_VMS_noefs ? '_' : '.';
    $Files{"$_.bak"}++ for ('MANIFEST', "MANIFEST${extsep}SKIP");
}

add_file('MANIFEST'   => 'Makefile.PL');
maniadd({ foo  => 'bar' });
$files = maniread;
# VMS downcases the MANIFEST.  We normalize it here to match.
%$files = map +(lc $_ => $files->{$_}), keys %$files;
my %expect = ( 'makefile.pl' => '',
               'foo'    => 'bar'
             );
is_deeply( $files, \%expect, 'maniadd() vs MANIFEST without trailing newline');

#add_file('MANIFEST'   => 'Makefile.PL');
#maniadd({ foo => 'bar' });

SKIP: {
    chmod( 0400, 'MANIFEST' );
    skip "Can't make MANIFEST read-only", 2 if -w 'MANIFEST' or $Config{osname} eq 'cygwin';

    eval {
        maniadd({ 'foo' => 'bar' });
    };
    is( $@, '',  "maniadd() won't open MANIFEST if it doesn't need to" );

    eval {
        maniadd({ 'grrrwoof' => 'yippie' });
    };
    like( $@, qr/^\Qmaniadd() could not open MANIFEST:\E/,
                 "maniadd() dies if it can't open the MANIFEST" );

    chmod( 0600, 'MANIFEST' );
}


END {
	is( unlink( keys %Files ), keys %Files, 'remove all added files' );
	for my $file ( keys %Files ) { 1 while unlink $file; } # all versions
	remove_dir( 'moretest', 'copy' );

	# now get rid of the parent directory
	ok( chdir( $cwd ), 'return to parent directory' );
	remove_dir( 'mantest' );
}
