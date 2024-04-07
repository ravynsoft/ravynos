package ExtUtils::ParseXS::Eval;
use strict;
use warnings;

our $VERSION = '3.51';

=head1 NAME

ExtUtils::ParseXS::Eval - Clean package to evaluate code in

=head1 SYNOPSIS

  use ExtUtils::ParseXS::Eval;
  my $rv = ExtUtils::ParseXS::Eval::eval_typemap_code(
    $parsexs_obj, "some Perl code"
  );

=head1 SUBROUTINES

=head2 $pxs->eval_output_typemap_code($typemapcode, $other_hashref)

Sets up various bits of previously global state
(formerly ExtUtils::ParseXS package variables)
for eval'ing output typemap code that may refer to these
variables.

Warns the contents of C<$@> if any.

Not all these variables are necessarily considered "public" wrt. use in
typemaps, so beware. Variables set up from the ExtUtils::ParseXS object:

  $Package $ALIAS $func_name $Full_func_name $pname

Variables set up from C<$other_hashref>:

  $var $type $ntype $subtype $arg

=cut

sub eval_output_typemap_code {
  my ($_pxs, $_code, $_other) = @_;

  my ($Package, $ALIAS, $func_name, $Full_func_name, $pname)
    = @{$_pxs}{qw(Package ALIAS func_name Full_func_name pname)};

  my ($var, $type, $ntype, $subtype, $arg)
    = @{$_other}{qw(var type ntype subtype arg)};

  my $rv = eval $_code;
  warn $@ if $@;
  return $rv;
}

=head2 $pxs->eval_input_typemap_code($typemapcode, $other_hashref)

Sets up various bits of previously global state
(formerly ExtUtils::ParseXS package variables)
for eval'ing output typemap code that may refer to these
variables.

Warns the contents of C<$@> if any.

Not all these variables are necessarily considered "public" wrt. use in
typemaps, so beware. Variables set up from the ExtUtils::ParseXS object:

  $Package $ALIAS $func_name $Full_func_name $pname

Variables set up from C<$other_hashref>:

  $var $type $ntype $subtype $num $init $printed_name $arg $argoff

=cut

sub eval_input_typemap_code {
  my ($_pxs, $_code, $_other) = @_;

  my ($Package, $ALIAS, $func_name, $Full_func_name, $pname)
    = @{$_pxs}{qw(Package ALIAS func_name Full_func_name pname)};

  my ($var, $type, $num, $init, $printed_name, $arg, $ntype, $argoff, $subtype)
    = @{$_other}{qw(var type num init printed_name arg ntype argoff subtype)};

  my $rv = eval $_code;
  warn $@ if $@;
  return $rv;
}

=head1 TODO

Eventually, with better documentation and possible some cleanup,
this could be part of C<ExtUtils::Typemaps>.

=cut

1;

# vim: ts=2 sw=2 et:
