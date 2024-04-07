use Test::More tests => 9;

BEGIN { use_ok('XS::APItest') };

#########################

my @mpushp = mpushp();
my @mpushn = mpushn();
my @mpushi = mpushi();
my @mpushu = mpushu();
is_deeply(\@mpushp, [qw(one two three)], 'mPUSHp()');
is_deeply(\@mpushn, [0.5, -0.25, 0.125], 'mPUSHn()');
is_deeply(\@mpushi, [-1, 2, -3],         'mPUSHi()');
is_deeply(\@mpushu, [1, 2, 3],           'mPUSHu()');

my @mxpushp = mxpushp();
my @mxpushn = mxpushn();
my @mxpushi = mxpushi();
my @mxpushu = mxpushu();
is_deeply(\@mxpushp, [qw(one two three)], 'mXPUSHp()');
is_deeply(\@mxpushn, [0.5, -0.25, 0.125], 'mXPUSHn()');
is_deeply(\@mxpushi, [-1, 2, -3],         'mXPUSHi()');
is_deeply(\@mxpushu, [1, 2, 3],           'mXPUSHu()');
