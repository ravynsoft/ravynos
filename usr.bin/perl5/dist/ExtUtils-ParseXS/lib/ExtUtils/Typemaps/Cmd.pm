package ExtUtils::Typemaps::Cmd;
use 5.006001;
use strict;
use warnings;
our $VERSION = '3.51';

use ExtUtils::Typemaps;

require Exporter;

our @ISA = qw(Exporter);
our @EXPORT = qw(embeddable_typemap);
our %EXPORT_TAGS = (all => \@EXPORT);

sub embeddable_typemap {
  my @tms = @_;

  # Get typemap objects
  my @tm_objs = map [$_, _intuit_typemap_source($_)], @tms;

  # merge or short-circuit
  my $final_tm;
  if (@tm_objs == 1) {
    # just one, merge would be pointless
    $final_tm = shift(@tm_objs)->[1];
  }
  else {
    # multiple, need merge
    $final_tm = ExtUtils::Typemaps->new;
    foreach my $other_tm (@tm_objs) {
      my ($tm_ident, $tm_obj) = @$other_tm;
      eval {
        $final_tm->merge(typemap => $tm_obj);
        1
      } or do {
        my $err = $@ || 'Zombie error';
        die "Failed to merge typ";
      }
    }
  }

  # stringify for embedding
  return $final_tm->as_embedded_typemap();
}

sub _load_module {
  my $name = shift;
  return eval "require $name; 1";
}

SCOPE: {
  my %sources = (
    module => sub {
      my $ident = shift;
      my $tm;
      if (/::/) { # looks like FQ module name, try that first
        foreach my $module ($ident, "ExtUtils::Typemaps::$ident") {
          if (_load_module($module)) {
            eval { $tm = $module->new }
              and return $tm;
          }
        }
      }
      else {
        foreach my $module ("ExtUtils::Typemaps::$ident", "$ident") {
          if (_load_module($module)) {
            eval { $tm = $module->new }
              and return $tm;
          }
        }
      }
      return();
    },
    file => sub {
      my $ident = shift;
      return unless -e $ident and -r _;
      return ExtUtils::Typemaps->new(file => $ident);
    },
  );
  # Try to find typemap either from module or file
  sub _intuit_typemap_source {
    my $identifier = shift;

    my @locate_attempts;
    if ($identifier =~ /::/ || $identifier !~ /[^\w_]/) {
      @locate_attempts = qw(module file);
    }
    else {
      @locate_attempts = qw(file module);
    }

    foreach my $source (@locate_attempts) {
      my $tm = $sources{$source}->($identifier);
      return $tm if defined $tm;
    }

    die "Unable to find typemap for '$identifier': "
        . "Tried to load both as file or module and failed.\n";
  }
} # end SCOPE

=head1 NAME

ExtUtils::Typemaps::Cmd - Quick commands for handling typemaps

=head1 SYNOPSIS

From XS:

  INCLUDE_COMMAND: $^X -MExtUtils::Typemaps::Cmd \
                   -e "print embeddable_typemap(q{Excommunicated})"

Loads C<ExtUtils::Typemaps::Excommunicated>, instantiates an object,
and dumps it as an embeddable typemap for use directly in your XS file.

=head1 DESCRIPTION

This is a helper module for L<ExtUtils::Typemaps> for quick
one-liners, specifically for inclusion of shared typemaps
that live on CPAN into an XS file (see SYNOPSIS).

For this reason, the following functions are exported by default:

=head1 EXPORTED FUNCTIONS

=head2 embeddable_typemap

Given a list of identifiers, C<embeddable_typemap>
tries to load typemaps from a file of the given name(s),
or from a module that is an C<ExtUtils::Typemaps> subclass.

Returns a string representation of the merged typemaps that can
be included verbatim into XS. Example:

  print embeddable_typemap(
    "Excommunicated", "ExtUtils::Typemaps::Basic", "./typemap"
  );

This will try to load a module C<ExtUtils::Typemaps::Excommunicated>
and use it as an C<ExtUtils::Typemaps> subclass. If that fails, it'll
try loading C<Excommunicated> as a module, if that fails, it'll try to
read a file called F<Excommunicated>. It'll work similarly for the
second argument, but the third will be loaded as a file first.

After loading all typemap files or modules, it will merge them in the
specified order and dump the result as an embeddable typemap.

=head1 SEE ALSO

L<ExtUtils::Typemaps>

L<perlxs>

=head1 AUTHOR

Steffen Mueller C<<smueller@cpan.org>>

=head1 COPYRIGHT & LICENSE

Copyright 2012 Steffen Mueller

This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=cut

1;

