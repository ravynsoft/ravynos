package stable;
$stable::VERSION = '0.031';
use strict;
use warnings;
use version ();

use experimental ();
use Carp qw/croak carp/;

my %allow_at = (
	bitwise       => 5.022000,
	isa           => 5.032000,
	lexical_subs  => 5.022000,
	postderef     => 5.020000,
);

sub import {
	my ($self, @pragmas) = @_;

	for my $pragma (@pragmas) {
		my $min_ver = $allow_at{$pragma};
		croak "unknown stablized experiment $pragma" unless defined $min_ver;
		croak "requested stablized experiment $pragma, which is stable at $min_ver but this is $]"
			unless $] >= $min_ver;
	}

	experimental->import(@pragmas);
	return;
}

sub unimport {
	my ($self, @pragmas) = @_;

	# Look, we could say "You can't unimport stable experiment 'bitwise' on
	# 5.20" but it just seems weird. -- rjbs, 2022-03-05
	experimental->unimport(@pragmas);
	return;
}

1;

#ABSTRACT: Experimental features made easy, once we know they're stable

__END__

=pod

=encoding UTF-8

=head1 NAME

stable - Experimental features made easy, once we know they're stable

=head1 VERSION

version 0.031

=head1 SYNOPSIS

	use stable 'lexical_subs', 'bitwise';
	my sub is_odd($value) { $value & 1 }

=head1 DESCRIPTION

The L<experimental> pragma makes it easy to turn on experimental while turning
off associated warnings.  You should read about it, if you don't already know
what it does.

Seeing C<use experimental> in code might be scary.  In fact, it probably should
be!  Code that uses experimental features might break in the future if the perl
development team decides that the experiment needs to be altered.  When
experiments become stable, because the developers decide they're a success, the
warnings associated with them go away.  When that happens, they can generally
be turned on with C<use feature>.

This is great, if you are using a version of perl where the feature you want is
already stable.  If you're using an older perl, though, it might be the case
that you want to use an experimental feature that still warns, even though
there's no risk in using it, because subsequent versions of perl have that
feature unchanged and now stable.

Here's an example:  The C<postderef> feature was added in perl 5.20.0.  In perl
5.24.0, it was marked stable.  Using it would no longer trigger a warning.  The
behavior of the feature didn't change between 5.20.0 and 5.24.0.  That means
that it's perfectly safe to use the feature on 5.20 or 5.22, even though
there's a warning.

In that case, you could very justifiably add C<use experimental 'postderef'>
but the casual reader may still be worried at seeing that.  The C<stable>
pragma exists to turn on experimental features only when it's known that
their behavior in the running perl is their stable behavior.

If you try to use an experimental feature that isn't stable or available on
the running version of perl, an exception will be thrown.  You should also take
care that you've required the version of C<stable> that you need!

If it's not immediately obvious why, here's a bit of explanation:

=over 4

=item *

C<stable> comes with perl, starting with perl v5.38.

=item *

Imagine that v5.38 adds a feature called "florps".  It will stop being
experimental in v5.42.

=item *

The version of C<stable> that comes with perl v5.38 can't know that the
I<florps> experiment will succeed, so you can't C<use stable 'florps'> on the
version of stable ships with v5.38, because it can't see the future!

=item *

You'll need to write C<use stable 1.234 'florps'> to say that you need version
1.234 of stable, which is when I<florps> became known to stable.

=back

Sure, it's a little weird, but it's worth it!  The documentation of this pragma
will tell you what version of C<stable> you need to require in order to use
various features.  See below.

At present there are only a few "stable" features:

=over 4

=item * C<bitwise> - stable as of perl 5.22, available via stable 0.031

=item * C<isa> - stable as of perl 5.32, available via stable 0.031

=item * C<lexical_subs> - stable as of perl 5.22, available via stable 0.031

Lexical subroutines were actually added in 5.18, and their design did not
change, but significant bugs makes them unsafe to use before 5.22.

=item * C<postderef> - stable as of perl 5.20, available via stable 0.031

=back

=head1 SEE ALSO

L<perlexperiment|perlexperiment> contains more information about experimental features.

=head1 AUTHOR

Leon Timmermans <leont@cpan.org>

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2013 by Leon Timmermans.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut
