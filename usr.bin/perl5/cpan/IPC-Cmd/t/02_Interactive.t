BEGIN { chdir 't' if -d 't'; };
BEGIN { use lib '../lib'; };

use strict;
use File::Spec;

### only run interactive tests when there's someone that can answer them
use Test::More -t STDOUT
                    ? 'no_plan' 
                    : ( skip_all => "No interactive tests from harness" );

my $Class   = 'IPC::Cmd';
my $Child   = File::Spec->catfile( qw[src child.pl] );
my @FDs     = 0..20;
my $IsWin32 = $^O eq 'MSWin32';

use_ok( $Class, 'run' );
$IPC::Cmd::DEBUG = 1;

my $Have_IPC_Run    = $Class->can_use_ipc_run;
my $Have_IPC_Open3  = $Class->can_use_ipc_open3;

### configurations to test IPC::Cmd with
my @Conf = ( 
    [ $Have_IPC_Run, $Have_IPC_Open3 ], 
    [ 0,             $Have_IPC_Open3 ], 
    [ 0,             0 ] 
);




### first, check which FD's are open. they should be open
### /after/ we run our tests as well.
### 0, 1 and 2 should be open, as they are STDOUT, STDERR and STDIN
### XXX 2 are opened by Test::Builder at least.. this is 'whitebox'
### knowledge, so unsafe to test against. around line 1322:
# sub _open_testhandles {
#     return if $Opened_Testhandles;
#     # We dup STDOUT and STDERR so people can change them in their
#     # test suites while still getting normal test output.
#     open(TESTOUT, ">&STDOUT") or die "Can't dup STDOUT:  $!";
#     open(TESTERR, ">&STDERR") or die "Can't dup STDERR:  $!";
#     $Opened_Testhandles = 1;
# }

my @Opened;
{   for ( @FDs ) {
        my $fh;
        my $rv = open $fh, "<&$_";
        push @Opened, $_ if $rv;
    }
    diag( "Opened FDs: @Opened" );
    cmp_ok( scalar(@Opened), '>=', 3,
                                "At least 3 FDs are opened" );
}

for my $aref ( @Conf ) {

    ### stupid warnings
    local $IPC::Cmd::USE_IPC_RUN    = $aref->[0];
    local $IPC::Cmd::USE_IPC_RUN    = $aref->[0];

    local $IPC::Cmd::USE_IPC_OPEN3  = $aref->[1];
    local $IPC::Cmd::USE_IPC_OPEN3  = $aref->[1];

    diag("Config: IPC::Run = $aref->[0] IPC::Open3 = $aref->[1]");
    ok( -t STDIN,               "STDIN attached to a tty" );
    
    for my $cmd ( qq[$^X $Child], qq[$^X $Child | $^X -neprint] ) {
    
        diag("Please enter some input. It will be echo'd back to you");
        my $buffer;
        my $ok = run( command => $cmd, verbose => 1, buffer => \$buffer );
    
        ok( $ok,                    "   Command '$cmd' ran successfully" );
    
        SKIP: {
            skip "No buffers available", 1 unless $Class->can_capture_buffer;
            ok( defined $buffer,    "   Input captured" );
        }
    }
}

### check we didnt leak any FHs
{   ### should be opened
    my %open = map { $_ => 1 } @Opened;
    
    for ( @FDs ) {
        my $fh;
        my $rv = open $fh, "<&=$_";
     
        ### these should be open 
        if( $open{$_} ) {
            ok( $rv,                "FD $_ opened" );
            ok( $fh,                "   FH indeed opened" );
            is( fileno($fh), $_,    "   Opened at the correct fileno($_)" );
        } else {
            ok( !$rv,               "FD $_ not opened" );
            ok( !(fileno($fh)),     "   FH indeed closed" );

            ### extra debug info if tests fail
#             use Devel::Peek;
#             use Data::Dumper;
#             diag( "RV=$rv FH=$fh Fileno=". fileno($fh). Dump($fh) ) if $rv;
#             diag( Dumper( [stat $fh] ) )                            if $rv;

        }
    }
}
