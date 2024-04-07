#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( qw(. ../lib) );
}

use strict;
use warnings;

plan( tests => 12 );

our (@warnings, $sub, $warn);

BEGIN {
    $warn = 'Illegal character in prototype';
}

sub one_warning_ok {
    cmp_ok(scalar(@warnings), '==', 1, 'One warning');
    cmp_ok(substr($warnings[0],0,length($warn)),'eq',$warn,'warning message');
    @warnings = ();
}

sub no_warnings_ok {
    cmp_ok(scalar(@warnings), '==', 0, 'No warnings');
    @warnings = ();
}

BEGIN {
    $SIG{'__WARN__'} = sub { push @warnings, @_ };
    $| = 1;
}

BEGIN { @warnings = () }

$sub = sub (x) { };

BEGIN {
    one_warning_ok;
}

{
    no warnings 'syntax';
    $sub = sub (x) { };
}

BEGIN {
    no_warnings_ok;
}

{
    no warnings 'illegalproto';
    $sub = sub (x) { };
}

BEGIN {
    no_warnings_ok;
}

{
    no warnings 'syntax';
    use warnings 'illegalproto';
    $sub = sub (x) { };
}

BEGIN {
    one_warning_ok;
}

BEGIN {
    $warn = q{Prototype after '@' for};
}

$sub = sub (@$) { };

BEGIN {
    one_warning_ok;
}

{
    no warnings 'syntax';
    $sub = sub (@$) { };
}

BEGIN {
    no_warnings_ok;
}

{
    no warnings 'illegalproto';
    $sub = sub (@$) { };
}

BEGIN {
    no_warnings_ok;
}

{
    no warnings 'syntax';
    use warnings 'illegalproto';
    $sub = sub (@$) { };
}

BEGIN {
    one_warning_ok;
}
