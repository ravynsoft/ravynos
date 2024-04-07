#!./perl -w

BEGIN {
    @OrigINC = @INC;
}

use Test::More tests => 13;
use Config;
use File::Spec;
use File::Path;

#set up files and directories
my @lib_dir;
my $Lib_Dir;
my $Arch_Dir;
my $Auto_Dir;
my $Module;
BEGIN {
    # lib.pm is documented to only work with Unix filepaths.
    @lib_dir  = qw(stuff moo);
    $Lib_Dir  = join "/", @lib_dir;
    $Arch_Dir = join "/", @lib_dir, $Config{archname};

    # create the auto/ directory and a module
    $Auto_Dir = File::Spec->catdir(@lib_dir, $Config{archname},'auto');
    $Module   = File::Spec->catfile(@lib_dir, 'Yup.pm');

    mkpath [$Auto_Dir];

    open(MOD, '>', $Module) || DIE $!;
    print MOD <<'MODULE';
package Yup;
$Plan = 9;
return '42';
MODULE

    close MOD;
}

END {
    # rmtree() can indirectly load the XS object for Win32, ensure
    # we have our original sane @INC
    local @INC = @OrigINC;
    # cleanup the auto/ directory we created.
    rmtree([$lib_dir[0]]);
}


use lib $Lib_Dir;
use lib $Lib_Dir;

BEGIN { use_ok('Yup') }

BEGIN {
    is( $INC[1], $Lib_Dir,          'lib adding at end of @INC' );
    print "# \@INC == @INC\n";
    is( $INC[0], $Arch_Dir,        '    auto/ dir in front of that' );
    is( grep(/^\Q$Lib_Dir\E$/, @INC), 1,   '    no duplicates' );

    # Yes, %INC uses Unixy filepaths.
    # Not on Mac OS, it doesn't ... it never has, at least.
    my $path = join("/",$Lib_Dir, 'Yup.pm');
    is( $INC{'Yup.pm'}, $path,    '%INC set properly' );

    is( eval { do 'Yup.pm'  }, 42,  'do() works' );
    ok( eval { require Yup; },      '   require()' );
    ok( eval "use Yup; 1;",         '   use()' );
    is( $@, '', 'last "eval()" parsed and executed correctly' );

    is_deeply(\@OrigINC, \@lib::ORIG_INC,    '@lib::ORIG_INC' );
}

no lib $Lib_Dir;

unlike( do { eval 'use lib $Config{installsitelib};'; $@ || '' },
	qr/::Config is read-only/, 'lib handles readonly stuff' );

BEGIN {
    is( grep(/stuff/, @INC), 0, 'no lib' );
    ok( !do 'Yup.pm',           '   do() effected' );
}
