#
# $Id: piconv.t,v 0.4 2013/02/18 02:23:56 dankogai Exp $
#

BEGIN {
    if ( $ENV{'PERL_CORE'} && $] >= 5.011) {
        print "1..0 # Skip: Don't know how to test this within perl's core\n";
        exit 0;
    }
}

use strict;
use FindBin;
use File::Spec;
use IPC::Open3 qw(open3);
use IO::Select;
use Test::More;

my $WIN = $^O eq 'MSWin32';

if ($WIN) {
    eval { require IPC::Run; IPC::Run->VERSION(0.83); 1; } or 
        plan skip_all => 'Win32 environments require IPC::Run 0.83 to complete this test';
}

sub run_cmd (;$$);

my $blib =
  File::Spec->rel2abs(
    File::Spec->catdir( $FindBin::RealBin, File::Spec->updir ) );
my $script = File::Spec->catdir($blib, 'bin', 'piconv');
my @base_cmd = ( $^X, "-Mblib=$blib", $script );

plan tests => 5;

{
    my ( $st, $out, $err ) = run_cmd;
    is( $st, 0, 'status for usage call' );
    is( $out, $WIN ? undef : '' );
    like( $err, qr{^piconv}, 'usage' );
}

{
    my($st, $out, $err) = run_cmd [qw(-S foobar -f utf-8 -t ascii), $script];
    like($err, qr{unknown scheme.*fallback}i, 'warning for unknown scheme');
}

{
    my ( $st, $out, $err ) = run_cmd [qw(-f utf-8 -t ascii ./non-existing/file)];
    like( $err, qr{can't open}i );
}

sub run_cmd (;$$) {
    my ( $args, $in ) = @_;
    
    my $out = "x" x 10_000;
    $out = "";
    my $err = "x" x 10_000;
    $err = "";
        
    if ($WIN) {
		IPC::Run->import(qw(run timeout));
		my @cmd;
		if (defined $args) {
			@cmd = (@base_cmd, @$args);
		} else {
			@cmd = @base_cmd;
		}
        run(\@cmd, \$in, \$out, \$err, timeout(10));
        my $st = $?;
		$out = undef if ($out eq '');
        ( $st, $out, $err );
    } else {
		$in ||= '';
        my ( $in_fh, $out_fh, $err_fh );
        use Symbol 'gensym';
        $err_fh =
          gensym;    # sigh... otherwise stderr gets just to $out_fh, not to $err_fh
        my $pid = open3( $in_fh, $out_fh, $err_fh, @base_cmd, @$args )
          or die "Can't run @base_cmd @$args: $!";
        print $in_fh $in;
        my $sel = IO::Select->new( $out_fh, $err_fh );

        while ( my @ready = $sel->can_read ) {
            for my $fh (@ready) {
                if ( eof($fh) ) {
                    $sel->remove($fh);
                    last if !$sel->handles;
                }
                elsif ( $out_fh == $fh ) {
                    my $line = <$fh>;
                    $out .= $line;
                }
                elsif ( $err_fh == $fh ) {
                    my $line = <$fh>;
                    $err .= $line;
                }
            }
        }
        my $st = $?;
        ( $st, $out, $err );
    }
}
