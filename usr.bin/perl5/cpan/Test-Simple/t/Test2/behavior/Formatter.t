use strict;
use warnings;

use Test2::Tools::Tiny;

use Test2::API qw/intercept run_subtest test2_stack/;
use Test2::Event::Bail;

{

	package Formatter::Subclass;
	use base 'Test2::Formatter';
	use Test2::Util::HashBase qw{f t};

    sub init {
        my $self = shift;
        $self->{+F} = [];
        $self->{+T} = [];
    }

	sub write         { }
	sub hide_buffered { 1 }

	sub terminate {
		my $s = shift;
		push @{$s->{+T}}, [@_];
	}

	sub finalize {
		my $s = shift;
		push @{$s->{+F}}, [@_];
	}
}

{
	my $f = Formatter::Subclass->new;
	intercept {
		my $hub = test2_stack->top;
		$hub->format($f);
		is(1, 1, 'test event 1');
		is(2, 2, 'test event 2');
		is(3, 2, 'test event 3');
		done_testing;
	};

	is(scalar @{$f->f}, 1, 'finalize method was called on formatter');
	is_deeply(
		$f->f->[0],
		[3, 3, 1, 0, 0],
		'finalize method received expected arguments'
	);

	ok(!@{$f->t}, 'terminate method was not called on formatter');
}

{
	my $f = Formatter::Subclass->new;

	intercept {
		my $hub = test2_stack->top;
		$hub->format($f);
		$hub->send(Test2::Event::Bail->new(reason => 'everything is terrible'));
		done_testing;
	};

	is(scalar @{$f->t}, 1, 'terminate method was called because of bail event');
	ok(!@{$f->f}, 'finalize method was not called on formatter');
}

{
	my $f = Formatter::Subclass->new;

	intercept {
		my $hub = test2_stack->top;
		$hub->format($f);
		$hub->send(Test2::Event::Plan->new(directive => 'skip_all', reason => 'Skipping all the tests'));
		done_testing;
	};

	is(scalar @{$f->t}, 1, 'terminate method was called because of plan skip_all event');
	ok(!@{$f->f}, 'finalize method was not called on formatter');
}

done_testing;
