#!/usr/bin/perl -w
#
#  Copyright (c) 2006-2007 Apple Inc. All Rights Reserved.
#

my $pid = $$;

END {
    return unless $$ == $pid;
    rm_test($_) for @TOCLEAN;
}

use strict;
use Test::More;
use IPC::Run3;

sub plan_security {
    
    unless (1) {
	plan skip_all => "security not installed";
	exit;
    };
    plan @_;
}

use Carp;
our @TOCLEAN;
END {
    return unless $$ == $pid;
    $SIG{__WARN__} = sub { 1 };
    cleanup_test($_) for @TOCLEAN;
}

our $output = '';

sub build_test {
    my $xd = "/tmp/test-$pid";
    my $security = 'security';
    $ENV{HOME} = $xd;
    push @TOCLEAN, [$xd, $security];
    return ($xd, $security);
}

sub rm_test {
    my ($xd, $security) = @{+shift};
    #rmtree [$xd];
}

sub cleanup_test {
    return unless $ENV{TEST_VERBOSE};
    my ($xd, $security) = @{+shift};
}

sub is_output {
    my ($security, $cmd, $arg, $expected, $test) = @_;
    $output = '';
    run3([$security, $cmd, @$arg], \undef, \$output, \$output);
#    open(STDOUT, ">&STDERR") || die "couldn't dup strerr: $!";
#    open(my $out, '-|', $security, $cmd, @$arg);
#    while (<$out>) { $output .= $_; }

    my $cmp = (grep {ref ($_) eq 'Regexp'} @$expected)
	? \&is_deeply_like : \&is_deeply;
    @_ = ([sort split (/\r?\n/, $output)], [sort @$expected], $test || join(' ', $cmd, @$arg));
    goto &$cmp;
}

1;
