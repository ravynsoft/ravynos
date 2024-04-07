#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require Config; import Config;
    if (not $Config{'d_readdir'}) {
	print "1..0\n";
	exit 0;
    }
}

use DirHandle;
use Test::More;

# Fetching the list of files in two different ways and expecting them 
# to be the same is a race condition when tests are running in parallel.
# So go somewhere quieter.
my $chdir;
if ($ENV{PERL_CORE} && -d 'uni') {
  chdir 'uni';
  push @INC, '../../lib';
  $chdir++;
};

$dot = DirHandle->new('.');

ok(defined $dot, "DirHandle->new returns defined value");
isa_ok($dot, 'DirHandle');

@a = sort <*>;
do { $first = $dot->read } while defined($first) && $first =~ /^\./;
ok(+(grep { $_ eq $first } @a),
    "Scalar context: First non-dot entry returned by 'read' is found in glob");

@b = sort($first, (grep {/^[^.]/} $dot->read));
ok(+(join("\0", @a) eq join("\0", @b)),
    "List context: Remaining entries returned by 'read' match glob");

ok($dot->rewind, "'rewind' method returns true value");
@c = sort grep {/^[^.]/} $dot->read;
cmp_ok(join("\0", @b), 'eq', join("\0", @c),
    "After 'rewind', directory re-read as expected");

ok($dot->close, "'close' method returns true value");
$dot->rewind;
ok(! defined $dot->read,
    "Having closed the directory handle -- and notwithstanding invocation of 'rewind' -- 'read' returns undefined value");

{
    local $@;
    eval { $redot = DirHandle->new( '.', '..' ); };
    like($@, qr/^usage/,
        "DirHandle constructor with too many arguments fails as expected");
}

# Now let's test with directory argument provided to 'open' rather than 'new'

$redot = DirHandle->new();
ok(defined $redot, "DirHandle->new returns defined value even without provided argument");
isa_ok($redot, 'DirHandle');
ok($redot->open('.'), "Explicit call of 'open' method returns true value");
do { $first = $redot->read } while defined($first) && $first =~ /^\./;
ok(+(grep { $_ eq $first } @a),
    "Scalar context: First non-dot entry returned by 'read' is found in glob");

@b = sort($first, (grep {/^[^.]/} $redot->read));
ok(+(join("\0", @a) eq join("\0", @b)),
    "List context: Remaining entries returned by 'read' match glob");

ok($redot->rewind, "'rewind' method returns true value");
@c = sort grep {/^[^.]/} $redot->read;
cmp_ok(join("\0", @b), 'eq', join("\0", @c),
    "After 'rewind', directory re-read as expected");

ok($redot->close, "'close' method returns true value");
$redot->rewind;
ok(! defined $redot->read,
    "Having closed the directory handle -- and notwithstanding invocation of 'rewind' -- 'read' returns undefined value");

$undot = DirHandle->new('foobar');
ok(! defined $undot,
    "Constructor called with non-existent directory returns undefined value");

# Test error conditions for various methods

$aadot = DirHandle->new();
ok(defined $aadot, "DirHandle->new returns defined value even without provided argument");
isa_ok($aadot, 'DirHandle');
{
    local $@;
    eval { $aadot->open('.', '..'); };
    like($@, qr/^usage/,
        "'open' called with too many arguments fails as expected");
}
ok($aadot->open('.'), "Explicit call of 'open' method returns true value");
{
    local $@;
    eval { $aadot->read('foobar'); };
    like($@, qr/^usage/,
        "'read' called with argument fails as expected");
}
{
    local $@;
    eval { $aadot->close('foobar'); };
    like($@, qr/^usage/,
        "'close' called with argument fails as expected");
}
{
    local $@;
    eval { $aadot->rewind('foobar'); };
    like($@, qr/^usage/,
        "'rewind' called with argument fails as expected");
}

{
    local $@;
    eval { $bbdot = DirHandle::new(); };
    like($@, qr/^usage/,
        "DirHandle called as function but with no arguments fails as expected");
}

$bbdot = DirHandle->new();
ok(! $bbdot->open('foobar'),
    "Calling open method on nonexistent directory returns false value");
ok(! $bbdot->read(),
    "Calling read method after failed open method returns false value");
ok(! $bbdot->rewind(),
    "Calling rewind method after failed open method returns false value");
ok(! $bbdot->close(),
    "Calling close method after failed open method returns false value");

if ($chdir) {
  chdir "..";
}

done_testing();
