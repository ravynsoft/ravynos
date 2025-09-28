#!perl -w
$|=1;
BEGIN {
    require Config; import Config;
    if ($Config{'extensions'} !~ /\bOpcode\b/ && $Config{'osname'} ne 'VMS') {
        print "1..0\n";
        exit 0;
    }
}

use Test::More tests => 7;

use Safe 1.00;
use Opcode qw(full_opset);

pass;

my $safe = Safe->new('PLPerl');
$safe->deny_only();

# Expression that triggers require utf8 and call to SWASHNEW.
# Fails with "Undefined subroutine PLPerl::utf8::SWASHNEW called"
# if SWASHNEW is not shared, else returns true if unicode logic is working.
# (For early Perls we don't take into account EBCDIC, so will fail there
my $trigger = q{ my $a = pack('U',0xB6); $a =~ tr/\x{1234}//rd };

ok $safe->reval( $trigger ), 'trigger expression should return true';
is $@, '', 'trigger expression should not die';

# return a closure
my $sub = $safe->reval(q{sub { warn pack('U',0xB6) }});

# define code outside Safe that'll be triggered from inside
my @warns;
$SIG{__WARN__} = sub {
    my $msg = shift;
    # this regex requires a different SWASH digit data for \d)
    # than the one used above and by the trigger code in Safe.pm
    $msg =~ s/\(eval \d+\)/XXX/i; # uses IsDigit SWASH
    push @warns, $msg;
};

is eval { $sub->() }, 1, 'warn should return 1';
is $@, '', '__WARN__ hook should not die';
is @warns, 1, 'should only be 1 warning';
like $warns[0], qr/at XXX line/, 'warning should have been edited';

