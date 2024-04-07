#!./perl -T

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bFile\/Glob\b/i) {
        print "1..0\n";
        exit 0;
    }
}

use Test::More;
BEGIN {
    plan(
        ${^TAINT}
        ? (tests => 2)
        : (skip_all => "Appear to running a perl without taint support")
    );
}

BEGIN {
    use_ok('File::Glob');
}

@a = File::Glob::bsd_glob("*");
eval { $a = join("",@a), kill 0; 1 };
like($@, qr/Insecure dependency/, 'all filenames should be tainted');
