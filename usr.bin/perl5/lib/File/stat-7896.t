#!./perl -w
use strict;

use Test::More;
use File::stat;

# This is possibly a bit black-box, but for now it works.
# If (either) File::stat stops lazy loading Symbol, or Test::More starts, it
# should be revisited
is($INC{'Symbol.pm'}, undef, "Symbol isn't loaded yet");

# ID 20011110.104 (RT #7896)
$! = 0;
is($!, '', '$! is empty');
is(File::stat::stat('/notafile'), undef, 'invalid file should fail');
isnt($!, '', 'should populate $!, given invalid file');
my $e = $!;

isnt($INC{'Symbol.pm'}, undef, "Symbol has been loaded");

# Repeat twice
is(File::stat::stat('/notafile'), undef, 'invalid file should fail again');
is($!, $e, '$! should be consistent for an invalid file');
$e = $!;
is(File::stat::stat('/notafile'), undef, 'invalid file should fail again');
is($!, $e, '$! should be consistent for an invalid file');

done_testing();
