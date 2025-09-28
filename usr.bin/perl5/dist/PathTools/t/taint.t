#!./perl -Tw
# Testing Cwd under taint mode.

use strict;

use Cwd;
chdir 't' unless $ENV{PERL_CORE};

use File::Spec;
use lib File::Spec->catdir('t', 'lib');
use Test::More;
BEGIN {
    plan(
	!eval { eval("1".substr($^X,0,0)) }
        ? (tests => 21)
        : (skip_all => "A perl without taint support")
    );
}

use Scalar::Util qw/tainted/;

my @Functions = qw(getcwd cwd fastcwd fastgetcwd
                   abs_path fast_abs_path
                   realpath fast_realpath
                  );

foreach my $func (@Functions) {
    no strict 'refs';
    my $cwd;
    eval { $cwd = &{'Cwd::'.$func} };
    is( $@, '',		"$func() should not explode under taint mode" );
    ok( tainted($cwd),	"its return value should be tainted" );
}

# Previous versions of Cwd tainted $^O
is !tainted($^O), 1, "\$^O should not be tainted";

{
    # [perl #126862] canonpath() loses taint
    my $tainted = substr($ENV{PATH}, 0, 0);
    # yes, getcwd()'s result should be tainted, and is tested above
    # but be sure
    ok tainted(File::Spec->canonpath($tainted . Cwd::getcwd)),
        "canonpath() keeps taint on non-empty string";
    ok tainted(File::Spec->canonpath($tainted)),
        "canonpath() keeps taint on empty string";

    (Cwd::getcwd() =~ /^(.*)/);
    my $untainted = $1;
    ok !tainted($untainted), "make sure our untainted value is untainted";
    ok !tainted(File::Spec->canonpath($untainted)),
        "canonpath() doesn't add taint to untainted string";
}
