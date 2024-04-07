use Test2::Tools::Tiny;
use Test2::Util qw/get_tid USE_THREADS try ipc_separator/;
use File::Temp qw/tempfile/;
use File::Spec;
use List::Util qw/shuffle/;
use strict;
use warnings;

if ($] lt "5.008") {
    print "1..0 # SKIP Test cannot run on perls below 5.8.0\n";
    exit 0;
}

sub simple_capture(&) {
    my $code = shift;

    my ($err, $out) = ("", "");

    my ($ok, $e);
    {
        local *STDOUT;
        local *STDERR;

        ($ok, $e) = try {
            open(STDOUT, '>', \$out) or die "Failed to open a temporary STDOUT: $!";
            open(STDERR, '>', \$err) or die "Failed to open a temporary STDERR: $!";

            $code->();
        };
    }

    die $e unless $ok;

    return {
        STDOUT => $out,
        STDERR => $err,
    };
}

require Test2::IPC::Driver::Files;
ok(my $ipc = Test2::IPC::Driver::Files->new, "Created an IPC instance");
ok($ipc->isa('Test2::IPC::Driver::Files'), "Correct type");
ok($ipc->isa('Test2::IPC::Driver'), "inheritence");

ok(-d $ipc->tempdir, "created temp dir");
is($ipc->pid, $$, "stored pid");
is($ipc->tid, get_tid(), "stored the tid");

my $hid = join ipc_separator, qw'12345 1 1 1';

$ipc->add_hub($hid);
my $hubfile = File::Spec->catfile($ipc->tempdir, "HUB" . ipc_separator . $hid);
ok(-f $hubfile, "wrote hub file");
if(ok(open(my $fh, '<', $hubfile), "opened hub file")) {
    my @lines = <$fh>;
    close($fh);
    is_deeply(
        \@lines,
        [ "$$\n", get_tid() . "\n" ],
        "Wrote pid and tid to hub file"
    );
}

{
    package Foo;
    use base 'Test2::Event';
}

$ipc->send($hid, bless({ foo => 1 }, 'Foo'));
$ipc->send($hid, bless({ bar => 1 }, 'Foo'));

my $sep = ipc_separator;
opendir(my $dh, $ipc->tempdir) || die "Could not open tempdir: !?";
my @files = grep { $_ !~ m/^\.+$/ && $_ !~ m/^HUB${sep}$hid/ } readdir($dh);
closedir($dh);
is(@files, 2, "2 files added to the IPC directory");

my @events = $ipc->cull($hid);
is_deeply(
    \@events,
    [{ foo => 1 }, { bar => 1 }],
    "Culled both events"
);

opendir($dh, $ipc->tempdir) || die "Could not open tempdir: !?";
@files = grep { $_ !~ m/^\.+$/ && $_ !~ m/^HUB$sep$hid/ } readdir($dh);
closedir($dh);
is(@files, 0, "All files collected");

$ipc->drop_hub($hid);
ok(!-f $ipc->tempdir . '/' . $hid, "removed hub file");

$ipc->send($hid, bless({global => 1}, 'Foo'), 'GLOBAL');
my @got = $ipc->cull($hid);
ok(@got == 0, "did not get our own global event");

my $tmpdir = $ipc->tempdir;
ok(-d $tmpdir, "still have temp dir");
$ipc = undef;
ok(!-d $tmpdir, "cleaned up temp dir");

{
    my $ipc = Test2::IPC::Driver::Files->new();

    my $tmpdir = $ipc->tempdir;

    my $ipc_thread_clone = bless {%$ipc}, 'Test2::IPC::Driver::Files';
    $ipc_thread_clone->set_tid(100);
    $ipc_thread_clone = undef;
    ok(-d $tmpdir, "Directory not removed (different thread)");

    my $ipc_fork_clone = bless {%$ipc}, 'Test2::IPC::Driver::Files';
    $ipc_fork_clone->set_pid($$ + 10);
    $ipc_fork_clone = undef;
    ok(-d $tmpdir, "Directory not removed (different proc)");


    $ipc_thread_clone = bless {%$ipc}, 'Test2::IPC::Driver::Files';
    $ipc_thread_clone->set_tid(undef);
    $ipc_thread_clone = undef;
    ok(-d $tmpdir, "Directory not removed (no thread)");

    $ipc_fork_clone = bless {%$ipc}, 'Test2::IPC::Driver::Files';
    $ipc_fork_clone->set_pid(undef);
    $ipc_fork_clone = undef;
    ok(-d $tmpdir, "Directory not removed (no proc)");

    $ipc = undef;
    ok(!-d $tmpdir, "Directory removed");
}

{
    no warnings qw/once redefine/;
    local *Test2::IPC::Driver::Files::driver_abort = sub {};
    local *Test2::IPC::Driver::Files::abort = sub {
        my $self = shift;
        local $self->{no_fatal} = 1;
        local $self->{no_bail} = 1;
        $self->Test2::IPC::Driver::abort(@_);
        die 255;
    };

    my $tmpdir;
    my @lines;
    my $file = __FILE__;

    my $out = simple_capture {
        local $ENV{T2_KEEP_TEMPDIR} = 1;

        my $ipc = Test2::IPC::Driver::Files->new();
        $tmpdir = $ipc->tempdir;
        $ipc->add_hub($hid);
        eval { $ipc->add_hub($hid) }; push @lines => __LINE__;
        $ipc->send($hid, bless({ foo => 1 }, 'Foo'));
        $ipc->cull($hid);
        $ipc->drop_hub($hid);
        eval { $ipc->drop_hub($hid) }; push @lines => __LINE__;

        # Make sure having a hub file sitting around does not throw things off
        # in T2_KEEP_TEMPDIR
        $ipc->add_hub($hid);
        $ipc = undef;
        1;
    };

    my $cleanup = sub {
        if (opendir(my $d, $tmpdir)) {
            for my $f (readdir($d)) {
                next if $f =~ m/^\.+$/;
                my $file = File::Spec->catfile($tmpdir, $f);
                next unless -f $file;
                1 while unlink $file;
            }
            closedir($d);
            rmdir($tmpdir) or warn "Could not remove temp dir '$tmpdir': $!";
        }
    };
    $cleanup->();

    like($out->{STDERR}, qr/IPC Temp Dir: \Q$tmpdir\E/m, "Got temp dir path");
    like($out->{STDERR}, qr/^# Not removing temp dir: \Q$tmpdir\E$/m, "Notice about not closing tempdir");

    like($out->{STDERR}, qr/^IPC Fatal Error: File for hub '$hid' already exists/m, "Got message for duplicate hub");
    like($out->{STDERR}, qr/^IPC Fatal Error: File for hub '$hid' does not exist/m, "Cannot remove hub twice");

    $out = simple_capture {
        my $ipc = Test2::IPC::Driver::Files->new();
        $ipc->add_hub($hid);
        my $trace = Test2::EventFacet::Trace->new(frame => [__PACKAGE__, __FILE__, __LINE__, 'foo']);
        my $e = eval { $ipc->send($hid, bless({glob => \*ok, trace => $trace}, 'Foo')); 1 };
        print STDERR $@ unless $e || $@ =~ m/^255/;
        $ipc->drop_hub($hid);
    };

    like($out->{STDERR}, qr/IPC Fatal Error:/, "Got fatal error");
    like($out->{STDERR}, qr/There was an error writing an event/, "Explanation");
    like($out->{STDERR}, qr/Destination: $hid/, "Got dest");
    like($out->{STDERR}, qr/Origin PID:\s+$$/, "Got pid");
    like($out->{STDERR}, qr/Error: Can't store GLOB items/, "Got cause");

    $out = simple_capture {
        my $ipc = Test2::IPC::Driver::Files->new();
        local $@;
        eval { $ipc->send($hid, bless({ foo => 1 }, 'Foo')) };
        print STDERR $@ unless $@ =~ m/^255/;
        $ipc = undef;
    };
    like($out->{STDERR}, qr/IPC Fatal Error: hub '$hid' is not available, failed to send event!/, "Cannot send to missing hub");

    $out = simple_capture {
        my $ipc = Test2::IPC::Driver::Files->new();
        $tmpdir = $ipc->tempdir;
        $ipc->add_hub($hid);
        $ipc->send($hid, bless({ foo => 1 }, 'Foo'));
        local $@;
        eval { $ipc->drop_hub($hid) };
        print STDERR $@ unless $@ =~ m/^255/;
    };
    $cleanup->();
    like($out->{STDERR}, qr/IPC Fatal Error: Not all files from hub '$hid' have been collected/, "Leftover files");
    like($out->{STDERR}, qr/IPC Fatal Error: Leftover files in the directory \(.*\.ready\)/, "What file");

    $out = simple_capture {
        my $ipc = Test2::IPC::Driver::Files->new();
        $ipc->add_hub($hid);

        eval { $ipc->send($hid, { foo => 1 }) };
        print STDERR $@ unless $@ =~ m/^255/;

        eval { $ipc->send($hid, bless({ foo => 1 }, 'xxx')) };
        print STDERR $@ unless $@ =~ m/^255/;
    };
    like($out->{STDERR}, qr/IPC Fatal Error: 'HASH\(.*\)' is not a blessed object/, "Cannot send unblessed objects");
    like($out->{STDERR}, qr/IPC Fatal Error: 'xxx=HASH\(.*\)' is not an event object!/, "Cannot send non-event objects");


    $ipc = Test2::IPC::Driver::Files->new();

    my ($fh, $fn) = tempfile();
    print $fh "\n";
    close($fh);

    Storable::store({}, $fn);
    $out = simple_capture { eval { $ipc->read_event_file($fn) } };
    like(
        $out->{STDERR},
        qr/IPC Fatal Error: Got an unblessed object: 'HASH\(.*\)'/,
        "Events must actually be events (must be blessed)"
    );

    Storable::store(bless({}, 'Test2::Event::FakeEvent'), $fn);
    $out = simple_capture { eval { $ipc->read_event_file($fn) } };
    like(
        $out->{STDERR},
        qr{IPC Fatal Error: Event has unknown type \(Test2::Event::FakeEvent\), tried to load 'Test2/Event/FakeEvent\.pm' but failed: Can't locate Test2/Event/FakeEvent\.pm},
        "Events must actually be events (not a real module)"
    );

    Storable::store(bless({}, 'Test2::API'), $fn);
    $out = simple_capture { eval { $ipc->read_event_file($fn) } };
    like(
        $out->{STDERR},
        qr{'Test2::API=HASH\(.*\)' is not a 'Test2::Event' object},
        "Events must actually be events (not an event type)"
    );

    Storable::store(bless({}, 'Foo'), $fn);
    $out = simple_capture {
        local @INC;
        push @INC => ('t/lib', 'lib');
        eval { $ipc->read_event_file($fn) };
    };
    ok(!$out->{STDERR}, "no problem", $out->{STDERR});
    ok(!$out->{STDOUT}, "no problem", $out->{STDOUT});

    unlink($fn);
}

{
    my $ipc = Test2::IPC::Driver::Files->new();
    $ipc->add_hub($hid);
    $ipc->send($hid, bless({global => 1}, 'Foo'), 'GLOBAL');
    $ipc->set_globals({});
    my @events = $ipc->cull($hid);
    is_deeply(
        \@events,
        [ {global => 1} ],
        "Got global event"
    );

    @events = $ipc->cull($hid);
    ok(!@events, "Did not grab it again");

    $ipc->set_globals({});
    @events = $ipc->cull($hid);
    is_deeply(
        \@events,
        [ {global => 1} ],
        "Still there"
    );

    $ipc->drop_hub($hid);
    $ipc = undef;
}

{
    my @list = shuffle (
        {global => 0, pid => 2, tid => 1, eid => 1},
        {global => 0, pid => 2, tid => 1, eid => 2},
        {global => 0, pid => 2, tid => 1, eid => 3},

        {global => 1, pid => 1,  tid => 1, eid => 1},
        {global => 1, pid => 12, tid => 1, eid => 3},
        {global => 1, pid => 11, tid => 1, eid => 2},

        {global => 0, pid => 2, tid => 3, eid => 1},
        {global => 0, pid => 2, tid => 3, eid => 10},
        {global => 0, pid => 2, tid => 3, eid => 100},

        {global => 0, pid => 5, tid => 3, eid => 2},
        {global => 0, pid => 5, tid => 3, eid => 20},
        {global => 0, pid => 5, tid => 3, eid => 200},
    );

    my @sorted;
    {
        package Test2::IPC::Driver::Files;
        @sorted = sort cmp_events @list;
    }

    is_deeply(
        \@sorted,
        [
            {global => 1, pid => 1,  tid => 1, eid => 1},
            {global => 1, pid => 11, tid => 1, eid => 2},
            {global => 1, pid => 12, tid => 1, eid => 3},

            {global => 0, pid => 2, tid => 1, eid => 1},
            {global => 0, pid => 2, tid => 1, eid => 2},
            {global => 0, pid => 2, tid => 1, eid => 3},

            {global => 0, pid => 2, tid => 3, eid => 1},
            {global => 0, pid => 2, tid => 3, eid => 10},
            {global => 0, pid => 2, tid => 3, eid => 100},

            {global => 0, pid => 5, tid => 3, eid => 2},
            {global => 0, pid => 5, tid => 3, eid => 20},
            {global => 0, pid => 5, tid => 3, eid => 200},
        ],
        "Sort by global, pid, tid and then eid"
    );
}

{
    my $ipc = 'Test2::IPC::Driver::Files';

    is_deeply(
        $ipc->parse_event_filename(join ipc_separator, qw'GLOBAL 123 456 789 Event Type Foo.ready.complete'),
        {
            ready    => !!1,
            complete => !!1,
            global   => 1,
            type     => "Event::Type::Foo",
            hid      => "GLOBAL",
            pid      => "123",
            tid      => "456",
            eid      => "789",
            file     => join ipc_separator, qw'GLOBAL 123 456 789 Event Type Foo',
        },
        "Parsed global complete"
    );

    is_deeply(
        $ipc->parse_event_filename(join ipc_separator, qw'GLOBAL 123 456 789 Event Type Foo.ready'),
        {
            ready    => !!1,
            complete => !!0,
            global   => 1,
            type     => "Event::Type::Foo",
            hid      => "GLOBAL",
            pid      => "123",
            tid      => "456",
            eid      => "789",
            file     => join ipc_separator, qw'GLOBAL 123 456 789 Event Type Foo',
        },
        "Parsed global ready"
    );

    is_deeply(
        $ipc->parse_event_filename(join ipc_separator, qw'GLOBAL 123 456 789 Event Type Foo'),
        {
            ready    => !!0,
            complete => !!0,
            global   => 1,
            type     => "Event::Type::Foo",
            hid      => "GLOBAL",
            pid      => "123",
            tid      => "456",
            eid      => "789",
            file     => join ipc_separator, qw'GLOBAL 123 456 789 Event Type Foo',
        },
        "Parsed global not ready"
    );

    is_deeply(
        $ipc->parse_event_filename(join ipc_separator, qw'1 1 1 1 123 456 789 Event Type Foo.ready.complete'),
        {
            ready    => !!1,
            complete => !!1,
            global   => 0,
            type     => "Event::Type::Foo",
            hid      => "1${sep}1${sep}1${sep}1",
            pid      => "123",
            tid      => "456",
            eid      => "789",
            file     => join ipc_separator, qw'1 1 1 1 123 456 789 Event Type Foo',
        },
        "Parsed event complete"
    );

    is_deeply(
        $ipc->parse_event_filename(join ipc_separator, qw'1 2 3 4 123 456 789 Event Type Foo.ready'),
        {
            ready    => !!1,
            complete => !!0,
            global   => 0,
            type     => "Event::Type::Foo",
            hid      => "1${sep}2${sep}3${sep}4",
            pid      => "123",
            tid      => "456",
            eid      => "789",
            file     => join ipc_separator, qw'1 2 3 4 123 456 789 Event Type Foo',
        },
        "Parsed event ready"
    );

    is_deeply(
        $ipc->parse_event_filename(join ipc_separator, qw'3 2 11 12 123 456 789 Event'),
        {
            ready    => !!0,
            complete => !!0,
            global   => 0,
            type     => "Event",
            hid      => "3${sep}2${sep}11${sep}12",
            pid      => "123",
            tid      => "456",
            eid      => "789",
            file     => join ipc_separator, qw'3 2 11 12 123 456 789 Event',
        },
        "Parsed event not ready"
    );
}

{
    my $ipc = Test2::IPC::Driver::Files->new();

    my $hid = join ipc_separator, qw"1 1 1 1";

    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, qw"GLOBAL 123 456 789 Event Type Foo.ready.complete") ? 1 : 0,
        0,
        "Do not read complete global"
    );

    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, qw"GLOBAL 123 456 789 Event Type Foo.ready") ? 1 : 0,
        1,
        "Should read ready global the first time"
    );
    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, qw"GLOBAL 123 456 789 Event Type Foo.ready") ? 1 : 0,
        0,
        "Should not read ready global again"
    );

    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, qw"GLOBAL 123 456 789 Event Type Foo") ? 1 : 0,
        0,
        "Should not read un-ready global"
    );

    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, $hid, qw"123 456 789 Event Type Foo.ready.complete") ? 1 : 0,
        0,
        "Do not read complete our hid"
    );

    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, $hid, qw"123 456 789 Event Type Foo.ready") ? 1 : 0,
        1,
        "Should read ready our hid"
    );

    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, $hid, qw"123 456 789 Event Type Foo.ready") ? 1 : 0,
        1,
        "Should read ready our hid (again, no duplicate checking)"
    );

    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, $hid, qw"123 456 789 Event Type Foo") ? 1 : 0,
        0,
        "Should not read un-ready our hid"
    );

    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, qw"1 2 3 123 456 789 Event Type Foo.ready.complete") ? 1 : 0,
        0,
        "Not ours - complete"
    );

    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, qw"1 2 3 123 456 789 Event Type Foo.ready") ? 1 : 0,
        0,
        "Not ours - ready"
    );

    is_deeply(
        $ipc->should_read_event($hid, join ipc_separator, qw"1 2 3 123 456 789 Event Type Foo") ? 1 : 0,
        0,
        "Not ours - unready"
    );

    my @got = $ipc->should_read_event($hid, join ipc_separator, $hid, qw"123 456 789 Event Type Foo");
    ok(!@got, "return empty list for false");

    @got = $ipc->should_read_event($hid, join ipc_separator, $hid, qw"123 456 789 Event Type Foo.ready");
    is(@got, 1, "got 1 item on true");

    like(delete $got[0]->{full_path}, qr{^.+\Q$hid\E${sep}123${sep}456${sep}789${sep}Event${sep}Type${sep}Foo\.ready$}, "Got full path");
    is_deeply(
        $got[0],
        $ipc->parse_event_filename(join ipc_separator, $hid, qw"123 456 789 Event Type Foo.ready"),
        "Apart from full_path we get entire parsed filename"
    );

    $ipc = undef;
}

done_testing;
