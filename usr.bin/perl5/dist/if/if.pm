package if;
use strict;
our $VERSION = '0.0610';

sub work {
  my $method = shift() ? 'import' : 'unimport';
  unless (@_ >= 2) {
    my $type = ($method eq 'import') ? 'use' : 'no';
    die "Too few arguments to '$type if' (some code returning an empty list in list context?)"
  }
  return unless shift;		# CONDITION

  my $p = $_[0];		# PACKAGE
  (my $file = "$p.pm") =~ s!::!/!g;
  require $file;		# Works even if $_[0] is a keyword (like open)
  my $m = $p->can($method);
  goto &$m if $m;
}

sub import   { shift; unshift @_, 1; goto &work }
sub unimport { shift; unshift @_, 0; goto &work }

1;
__END__

=head1 NAME

if - C<use> a Perl module if a condition holds

=head1 SYNOPSIS

    use if CONDITION, "MODULE", ARGUMENTS;
    no  if CONDITION, "MODULE", ARGUMENTS;

=head1 DESCRIPTION

=head2 C<use if>

The C<if> module is used to conditionally load another module.  The construct:

    use if CONDITION, "MODULE", ARGUMENTS;

... will load C<MODULE> only if C<CONDITION> evaluates to true; it has no
effect if C<CONDITION> evaluates to false.  (The module name, assuming it
contains at least one C<::>, must be quoted when C<'use strict "subs";'> is in
effect.)  If the CONDITION does evaluate to true, then the above line has the
same effect as:

    use MODULE ARGUMENTS;

For example, the F<Unicode::UCD> module's F<charinfo> function will use two functions from F<Unicode::Normalize> only if a certain condition is met:

    use if defined &DynaLoader::boot_DynaLoader,
        "Unicode::Normalize" => qw(getCombinClass NFD);

Suppose you wanted C<ARGUMENTS> to be an empty list, I<i.e.>, to have the
effect of:

    use MODULE ();

You can't do this with the C<if> pragma; however, you can achieve
exactly this effect, at compile time, with:

    BEGIN { require MODULE if CONDITION }

=head2 C<no if>

The C<no if> construct is mainly used to deactivate categories of warnings
when those categories would produce superfluous output under specified
versions of F<perl>.

For example, the C<redundant> category of warnings was introduced in
Perl-5.22.  This warning flags certain instances of superfluous arguments to
C<printf> and C<sprintf>.  But if your code was running warnings-free on
earlier versions of F<perl> and you don't care about C<redundant> warnings in
more recent versions, you can call:

    use warnings;
    no if $] >= 5.022, q|warnings|, qw(redundant);

    my $test    = { fmt  => "%s", args => [ qw( x y ) ] };
    my $result  = sprintf $test->{fmt}, @{$test->{args}};

The C<no if> construct assumes that a module or pragma has correctly
implemented an C<unimport()> method -- but most modules and pragmata have not.
That explains why the C<no if> construct is of limited applicability.

=head1 BUGS

The current implementation does not allow specification of the required
version of the module.

=head1 SEE ALSO

L<Module::Requires> can be used to conditionally load one or more modules,
with constraints based on the version of the module.
Unlike C<if> though, L<Module::Requires> is not a core module.

L<Module::Load::Conditional> provides a number of functions you can use to
query what modules are available, and then load one or more of them at runtime.

The L<provide> module from CPAN can be used to select one of several possible
modules to load based on the version of Perl that is running.

=head1 AUTHOR

Ilya Zakharevich L<mailto:ilyaz@cpan.org>.

=head1 COPYRIGHT AND LICENCE

This software is copyright (c) 2002 by Ilya Zakharevich.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut
