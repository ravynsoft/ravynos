#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    push @INC, '../lib';
}

use Test::More;

BEGIN {
    use_ok( 'less' );

    package less::again;
    sub stash_name {'less'}
    @ISA = 'less';
    $INC{'less/again.pm'} = 1;
}

is_deeply([less->of], [], 'more please');
use less;
is_deeply([less->of], ['please'],'less please');
is_deeply([less::again->of], ['please'], 'less::again please');
no less;
is_deeply([less->of],[],'more please');
is_deeply([less::again->of], [], 'no less::again please');
use less::again;
is_deeply([less->of], ['please'],'less please');
is_deeply([less::again->of], ['please'], 'less::again please');
no less::again;
is_deeply([less->of],[],'more please');
is_deeply([less::again->of], [], 'no less::again please');

use less 'random acts';
is_deeply([sort less->of],[sort qw(random acts)],'less random acts');

is(scalar less->of('random'),1,'less random');

done_testing();
