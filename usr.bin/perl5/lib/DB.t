#!./perl -Tw

BEGIN {
        chdir 't' if -d 't';
        @INC = '../lib';
	require Config;
	if (($Config::Config{'extensions'} !~ m!\bList/Util\b!) ){
		print "1..0 # Skip -- Perl configured without List::Util module\n";
		exit 0;
	}
}

# symbolic references used later
use strict qw( vars subs );

# @DB::dbline values have both integer and string components (Benjamin Goldberg)
use Scalar::Util qw( dualvar );
my $dualfalse = dualvar(0, 'false');
my $dualtrue = dualvar(1, 'true');

use Test::More;

# must happen at compile time for DB:: package variable localizations to work
BEGIN {
        use_ok( 'DB' );
}

# test DB::sub()
{
        my $callflag = 0;
        local $DB::sub = sub {
                $callflag += shift || 1;
                my @vals = (1, 4, 9);
                return @vals;
        };
        my $ret = DB::sub;
        is( $ret, 3, 'DB::sub() should handle scalar context' );
        is( $callflag, 1, '... should call $DB::sub contents' );
        $ret = join(' ', DB::sub(2));
        is( $ret, '1 4 9', '... should handle scalar context' );
        is( $callflag, 3, '... should pass along arguments to the sub' );
        ok( defined($DB::ret),'$DB::ret should be defined after successful return');
        DB::sub;
        ok( !defined($DB::ret), '... should respect void context' );
        $DB::sub = '::DESTROY';
        ok( !defined($DB::ret), '... should return undef for DESTROY()' );
}

# test DB::DB()
{ 
        ok( ! defined DB::DB(), 
                'DB::DB() should return undef if $DB::ready is false');
        is( DB::catch(), 1, 'DB::catch() should work' );
        is( DB->skippkg('foo'), 1, 'DB->skippkg() should push args' );

        # change packages to mess with caller()
        package foo;
        ::ok( ! defined DB::DB(), 'DB::DB() should skip skippable packages' );

        package main;
        is( $DB::filename, $0, '... should set $DB::filename' );
        is( $DB::lineno, __LINE__ - 4, '... should set $DB::lineno' );

        DB::DB();
        # stops at line 94
}

# test DB::save()
{
       no warnings 'uninitialized';

        # assigning a number to $! seems to produce an error message, when read
        local ($@, $,, $/, $\, $^W, $!) = (1 .. 5);
        DB::save();
        is( "$@$!$,$/$\$^W", "1\n0", 'DB::save() should reset punctuation vars' );
}

# test DB::catch()
{
        local $DB::signal;
        DB::catch();
        ok( $DB::signal, 'DB::catch() should set $DB::signal' );
        # add clients and test to see if they are awakened
}

# test DB::_clientname()
is( DB::_clientname('foo=A(1)'), 'foo',
    'DB::_clientname should return refname');
is( DB::_clientname('bar'), undef,
        'DB::_clientname should not return non refname');

# test DB::next() and DB::step()
{
        local $DB::single;
        DB->next();
        is( $DB::single, 2, 'DB->next() should set $DB::single to 2' );
        DB->step();
        is( $DB::single, 1, 'DB->step() should set $DB::single to 1' );
}

# test DB::cont()
{
        # cannot test @stack

        local $DB::single = 1;
        my $fdb = FakeDB->new();
        DB::cont($fdb, 2);
        is( $fdb->{tbreak}, 2, 'DB::cont() should set tbreak in object' );
        is( $DB::single, 0, '... should set $DB::single to 0' );
}

# test DB::ret()
{
        # cannot test @stack

        local $DB::single = 1;
        DB::ret();
        is( $DB::single, 0, 'DB::ret() should set $DB::single to 0' );
}

# test DB::backtrace()
{
        local (@DB::args, $DB::signal);

        my $line = __LINE__ + 1;
        my @ret = eval { DB->backtrace() };
        like( $ret[0], qr/file.+\Q$0\E/, 'DB::backtrace() should report current file');
        like( $ret[0], qr/line $line/, '... should report calling line number' );
        like( $ret[0], qr/eval\Q {...}/, '... should catch eval BLOCK' );

        @ret = eval "one(2)";
        is( scalar @ret, 1, '... should report from provided stack frame number' );
        like( $ret[0], qr/\@ = &eval \'one.+?2\)\'/, #'
                '... should find eval STRING construct');
        $ret[0] = check_context(1);
        like( $ret[0], qr/\$ = &main::check_context/, 
                '... should respect context of calling construct');
        
        $DB::signal = 1;
        @DB::args = (1, 7);
        @ret = three(1);
        is( scalar @ret, 1, '... should end loop if $DB::signal is true' );

        # does not check 'require' or @DB::args mangling
}

sub check_context {
        return (eval "one($_[0])")[-1];
}
sub one { DB->backtrace(@_) }
sub two { one(@_) }
sub three { two(@_) }

# test DB::trace_toggle
{
        local $DB::trace = 0;
        DB->trace_toggle;
        ok( $DB::trace, 'DB::trace_toggle() should toggle $DB::trace' );
        DB->trace_toggle;
        ok( !$DB::trace, '... should toggle $DB::trace (back)' );
}

# test DB::subs()
{
        local %DB::sub;
        my $subs = DB->subs;
        is( $subs, 0, 'DB::subs() should return keys of %DB::subs' );
        %DB::sub = ( foo => 'foo:23-45' , bar => 'ba:r:7-890' );
        $subs = DB->subs;
        is( $subs, 2, '... same song, different key' );
        my @subs = DB->subs( 'foo', 'boo', 'bar' );
        is( scalar @subs, 2, '... should report only for requested subs' );
        my @expected = ( [ 'foo', 23, 45 ], [ 'ba:r', 7, 890 ] );
        is_deeply( \@subs, \@expected, '... find file, start, end for subs' );
}

# test DB::filesubs()
{
        local ($DB::filename, %DB::sub);
        $DB::filename = 'baz';
        %DB::sub = map { $_ => $_ } qw( bazbar bazboo boobar booboo boobaz );
        my @ret = DB->filesubs();
        is( scalar @ret, 2, 'DB::filesubs() should use $DB::filename with no args');
        @ret = grep { /^baz/ } @ret;    
        is( scalar @ret, 2, '... should pick up subs in proper file' );
        @ret = DB->filesubs('boo');
        is( scalar @ret, 3, '... should use argument to find subs' );
        @ret = grep { /^boo/ } @ret;    
        is( scalar @ret, 3, '... should pick up subs in proper file with argument');
}

# test DB::files()
{
        my $dbf = () = DB::files();
        my $main = () = grep ( m!^_<!, keys %main:: );
        is( $dbf, $main, 'DB::files() should pick up filenames from %main::' );
}

# test DB::lines()
{
        local @DB::dbline = ( 'foo' );
        is( DB->lines->[0], 'foo', 'DB::lines() should return ref to @DB::dbline' );
}

# test DB::loadfile()
SKIP: {
        local (*DB::dbline, $DB::filename);
        ok( ! defined DB->loadfile('notafile'),
                'DB::loadfile() should not find unloaded file' );
        my $file = (grep { m|^_<.+\.pm| } keys %main:: )[0];
        skip('cannot find loaded file', 3) unless $file;
        $file =~ s/^_<..//;

        my $db = DB->loadfile($file);
        like( $db, qr!$file\z!, '... should find loaded file from partial name');

        is( *DB::dbline, *{ "_<$db" } , 
                '... should set *DB::dbline to associated glob');
        is( $DB::filename, $db, '... should set $DB::filename to file name' );

        # test clients
}

# test DB::lineevents()
{
        use vars qw( *baz );

        local $DB::filename = 'baz';
        local *baz = *{ "main::_<baz" };
        
        @baz = map { dualvar(1, $_) } qw( one two three four five );
        %baz = (
                1 => "foo\0bar",
                3 => "boo\0far",
                4 => "fazbaz",
        );
        my %ret = DB->lineevents();
        is( scalar keys %ret, 3, 'DB::lineevents() should pick up defined lines' );

        # array access in DB::lineevents() starts at element 1, not 0
        is( join(' ', @{ $ret{1} }), 'two foo bar', '... should stash data in hash');
}

# test DB::set_break()
{
        local ($DB::lineno, *DB::dbline, $DB::package);

        %DB::dbline = (
                1 => "\0",
                2 => undef,
                3 => "123\0\0\0abc",
                4 => "\0abc",
        );

        *DB::dbline = [ $dualfalse, $dualtrue, $dualfalse, $dualfalse, $dualtrue ];

        local %DB::sub = (
                'main::foo'     => 'foo:1-4',
        );
         
        DB->set_break(1, 'foo');
        is( $DB::dbline{1}, "foo\0", 'DB::set_break() should set break condition' );

        $DB::lineno = 1;
        DB->set_break(undef, 'bar');
        is( $DB::dbline{1}, "bar\0", 
                '... should use $DB::lineno without specified line' );

        DB->set_break(4);
        is( $DB::dbline{4}, "1\0abc", '... should use default condition if needed');

        local %DB::sub = (
                'main::foo'     => 'foo:1-4',
        );
        DB->set_break('foo', 'baz');
        is( $DB::dbline{4}, "baz\0abc", 
                '... should use _find_subline() to resolve subname' );

        my $db = FakeDB->new();
        DB::set_break($db, 2);
        like( $db->{output}, qr/2 not break/, '... should respect @DB::dbline' );

        DB::set_break($db, 'nonfoo');
        like( $db->{output}, qr/not found/, '... should warn on unfound sub' );
}

# test DB::set_tbreak()
{
        local ($DB::lineno, *DB::dbline, $DB::package);
        *DB::dbline = [ $dualfalse, $dualtrue, $dualfalse, $dualfalse, $dualtrue ];

        DB->set_tbreak(1);
        is( $DB::dbline{1}, ';9', 'DB::set_tbreak() should set tbreak condition' );

        local %DB::sub = (
                'main::foo'     => 'foo:1-4',
        );
        DB->set_tbreak('foo', 'baz');
        is( $DB::dbline{4}, ';9', 
                '... should use _find_subline() to resolve subname' );

        my $db = FakeDB->new();
        DB::set_tbreak($db, 2);
        like( $db->{output}, qr/2 not break/, '... should respect @DB::dbline' );

        DB::set_break($db, 'nonfoo');
        like( $db->{output}, qr/not found/, '... should warn on unfound sub' );
}

# test DB::_find_subline()
{
        my @foo;
        local *{ "::_<foo" } = \@foo;

        local $DB::package;
        local %DB::sub = (
                'TEST::foo'     => 'foo:10-15',
                'main::foo'     => 'foo:11-12',
                'bar::bar'      => 'foo:10-16',
        );

        $foo[11] = $dualtrue;

        is( DB::_find_subline('TEST::foo'), 11, 
                'DB::_find_subline() should find fully qualified sub' );
        is( DB::_find_subline("TEST'foo"), 11, '... should handle old package sep');
        is( DB::_find_subline('foo'), 11, 
                '... should resolve unqualified package name to main::' );

        $DB::package = 'bar';
        is( DB::_find_subline('bar'), 11, 
                '... should resolve unqualified name with $DB::package, if defined' );
        
        $foo[11] = $dualfalse;

        is( DB::_find_subline('TEST::foo'), 15, 
                '... should increment past lines with no events' );
                
        ok( ! defined DB::_find_subline('sirnotappearinginthisfilm'),
                '... should not find nonexistent sub' );
}

# test DB::clr_breaks()
{
        local *DB::dbline;
        my %lines = (
                1 => "\0",
                2 => undef,
                3 => "123\0\0\0abc",
                4 => "\0\0\0abc",
        );

        %DB::dbline = %lines;
        DB->clr_breaks(1 .. 4);
        is( scalar keys %DB::dbline, 3, 'DB::clr_breaks() should clear breaks' );
        ok( ! exists($DB::dbline{1}), '... should delete empty actions' );
        is( $DB::dbline{3}, "\0\0\0abc", '... should remove break, leaving action');
        is( $DB::dbline{4}, "\0\0\0abc", '... should not remove set actions' );

        local *{ "::_<foo" } = [ 0, 0, 0, 1 ];

        local $DB::package;
        local %DB::sub = (
                'main::foo'     => 'foo:1-3',
        );

        %DB::dbline = %lines;
        DB->clr_breaks('foo');

        is( $DB::dbline{3}, "\0\0\0abc", 
                '... should find lines via _find_subline()' );
        
        my $db = FakeDB->new();
        DB::clr_breaks($db, 'abadsubname');
        is( $db->{output}, "Subroutine not found.\n", 
                '... should output warning if sub cannot be found');

        @DB::dbline = (1 .. 4);
        %DB::dbline = (%lines, 5 => "\0" );

        DB::clr_breaks();

        is( scalar keys %DB::dbline, 4, 
                'Relying on @DB::dbline in DB::clr_breaks() should clear breaks' );
        ok( ! exists($DB::dbline{1}), '... should delete empty actions' );
        is( $DB::dbline{3}, "\0\0\0abc", '... should remove break, leaving action');
        is( $DB::dbline{4}, "\0\0\0abc", '... should not remove set actions' );
        ok( exists($DB::dbline{5}), 
                '... should only go to last index of @DB::dbline' );
}

# test DB::set_action()
{
        local *DB::dbline;

        %DB::dbline = (
                2 => "\0abc",
        );

        *DB::dbline = [ $dualfalse, $dualfalse, $dualtrue, $dualtrue ];

        DB->set_action(2, 'def');
        is( $DB::dbline{2}, "\0def", 
                'DB::set_action() should replace existing action' );
        DB->set_action(3, '');
        is( $DB::dbline{3}, "\0", '... should set new action' );

        my $db = FakeDB->new();
        DB::set_action($db, 'abadsubname');
        is( $db->{output}, "Subroutine not found.\n", 
                '... should output warning if sub cannot be found');

        DB::set_action($db, 1);
        like( $db->{output}, qr/1 not action/, 
                '... should warn if line cannot be actionivated' );
}

# test DB::clr_actions()
{
        local *DB::dbline;
        my %lines = (
                1 => "\0",
                2 => undef,
                3 => "123\0abc",
                4 => "abc\0",
        );

        %DB::dbline = %lines;
        *DB::dbline = [ ($dualtrue) x 4 ];

        DB->clr_actions(1 .. 4);

        is( scalar keys %DB::dbline, 2, 'DB::clr_actions() should clear actions' );
        ok( ! exists($DB::dbline{1}), '... should delete empty actions' );
        is( $DB::dbline{3}, "123", '... should remove action, leaving break');
        is( $DB::dbline{4}, "abc\0", '... should not remove set breaks' );

        local *{ "::_<foo" } = [ 0, 0, 0, 1 ];

        local $DB::package;
        local %DB::sub = (
                'main::foo'     => 'foo:1-3',
        );

        %DB::dbline = %lines;
        DB->clr_actions('foo');

        is( $DB::dbline{3}, "123", '... should find lines via _find_subline()' );
        
        my $db = FakeDB->new();
        DB::clr_actions($db, 'abadsubname');
        is( $db->{output}, "Subroutine not found.\n", 
                '... should output warning if sub cannot be found');

        @DB::dbline = (1 .. 4);
        %DB::dbline = (%lines, 5 => "\0" );

        DB::clr_actions();

        is( scalar keys %DB::dbline, 4, 
                'Relying on @DB::dbline in DB::clr_actions() should clear actions' );
        ok( ! exists($DB::dbline{1}), '... should delete empty actions' );
        is( $DB::dbline{3}, "123", '... should remove action, leaving break');
        is( $DB::dbline{4}, "abc\0", '... should not remove set breaks' );
        ok( exists($DB::dbline{5}), 
                '... should only go to last index of @DB::dbline' );
}

# test DB::prestop()
ok( ! defined DB::prestop('test'),
        'DB::prestop() should return undef for undef value' );
DB::prestop('test', 897);
is( DB::prestop('test'), 897, '... should return value when set' );

# test DB::poststop(), not exactly parallel
ok( ! defined DB::poststop('tset'), 
        'DB::prestop() should return undef for undef value' );
DB::poststop('tset', 987);
is( DB::poststop('tset'), 987, '... should return value when set' );

# test DB::evalcode()
ok( ! defined DB::evalcode('foo'),
        'DB::evalcode() should return undef for undef value' );

DB::evalcode('foo', 'bar');
is( DB::evalcode('foo'), 'bar', '... should return value when set' );

# test DB::_outputall(), must create fake clients first
ok( DB::register( FakeDB->new() ), 'DB::register() should work' );
DB::register( FakeDB->new() ) for ( 1 .. 2);

DB::_outputall(1, 2, 3);
is( $FakeDB::output, '123123123', 
        'DB::_outputall() should call output(@_) on all clients' );

# test virtual methods
for my $method (qw( cprestop cpoststop awaken init stop idle cleanup output )) {
        ok( defined &{ "DB::$method" }, "DB::$method() should be defined" );
}

done_testing();

# DB::skippkg() uses lexical
# DB::ready() uses lexical

package FakeDB;

use vars qw( $output );

sub new {
        bless({}, $_[0]);
}

sub set_tbreak {
        my ($self, $val) = @_;
        $self->{tbreak} = $val;
}

sub output {
        my $self = shift;
        if (ref $self) {
                $self->{output} = join('', @_);
        } else {
                $output .= join('', @_);
        }
}
