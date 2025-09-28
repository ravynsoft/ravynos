use Config;
use Test::More;
BEGIN { plan skip_all => 'no threads' unless $Config{useithreads} }

plan tests => 1;

use XS::APItest;

ok test_alloccopstash;
