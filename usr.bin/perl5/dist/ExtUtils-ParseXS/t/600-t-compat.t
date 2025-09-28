#!/usr/bin/perl
use strict;
use warnings;

use Test::More;

# This test is for making sure that the new EU::Typemaps
# based typemap merging produces the same result as the old
# EU::ParseXS code.

use ExtUtils::Typemaps;
use ExtUtils::ParseXS::Utilities qw(
  C_string
  trim_whitespace
  process_typemaps
);
use ExtUtils::ParseXS::Constants;
use File::Spec;

my $path_prefix = File::Spec->catdir(-d 't' ? qw(t data) : qw(data));

my @tests = (
  {
    name => 'Simple conflict',
    local_maps => [
      File::Spec->catfile($path_prefix, "conflicting.typemap"),
    ],
    std_maps => [
      File::Spec->catfile($path_prefix, "other.typemap"),
    ],
  },
  {
    name => 'B',
    local_maps => [
      File::Spec->catfile($path_prefix, "b.typemap"),
    ],
    std_maps => [],
  },
  {
    name => 'B and perl',
    local_maps => [
      File::Spec->catfile($path_prefix, "b.typemap"),
    ],
    std_maps => [
      File::Spec->catfile($path_prefix, "perl.typemap"),
    ],
  },
  {
    name => 'B and perl and B again',
    local_maps => [
      File::Spec->catfile($path_prefix, "b.typemap"),
    ],
    std_maps => [
      File::Spec->catfile($path_prefix, "perl.typemap"),
      File::Spec->catfile($path_prefix, "b.typemap"),
    ],
  },
);
plan tests => scalar(@tests);

my @local_tmaps;
my @standard_typemap_locations;
SCOPE: {
  no warnings 'redefine';
  sub ExtUtils::ParseXS::Utilities::standard_typemap_locations {
    @standard_typemap_locations;
  }
  sub standard_typemap_locations {
    @standard_typemap_locations;
  }
}

foreach my $test (@tests) {
  @local_tmaps = @{ $test->{local_maps} };
  @standard_typemap_locations = @{ $test->{std_maps} };

  my $res = [_process_typemaps([@local_tmaps], '.')];
  my $tm = process_typemaps([@local_tmaps], '.');
  my $res_new = [map $tm->$_(), qw(_get_typemap_hash _get_prototype_hash _get_inputmap_hash _get_outputmap_hash) ];

  # Normalize trailing whitespace. Let's be that lenient, mkay?
  for ($res, $res_new) {
    for ($_->[2], $_->[3]) {
      for (values %$_) {
        s/\s+\z//;
      }
    }
  }
  #use Data::Dumper; warn Dumper $res;
  #use Data::Dumper; warn Dumper $res_new;

  is_deeply($res_new, $res, "typemap equivalency for '$test->{name}'");
}


# The code below is a reproduction of what the pre-ExtUtils::Typemaps
# typemap-parsing/handling code in ExtUtils::ParseXS looked like. For
# bug-compatibility, we want to produce the same data structures as that
# code as much as possible.
sub _process_typemaps {
  my ($tmap, $pwd) = @_;

  my @tm = ref $tmap ? @{$tmap} : ($tmap);

  foreach my $typemap (@tm) {
    die "Can't find $typemap in $pwd\n" unless -r $typemap;
  }

  push @tm, standard_typemap_locations( \@INC );

  my ($type_kind_ref, $proto_letter_ref, $input_expr_ref, $output_expr_ref)
    = ( {}, {}, {}, {} );

  foreach my $typemap (@tm) {
    next unless -f $typemap;
    # skip directories, binary files etc.
    warn("Warning: ignoring non-text typemap file '$typemap'\n"), next
      unless -T $typemap;
    ($type_kind_ref, $proto_letter_ref, $input_expr_ref, $output_expr_ref) =
      _process_single_typemap( $typemap,
        $type_kind_ref, $proto_letter_ref, $input_expr_ref, $output_expr_ref);
  }
  return ($type_kind_ref, $proto_letter_ref, $input_expr_ref, $output_expr_ref);
}

sub _process_single_typemap {
  my ($typemap,
    $type_kind_ref, $proto_letter_ref, $input_expr_ref, $output_expr_ref) = @_;
  open my $TYPEMAP, '<', $typemap
    or warn ("Warning: could not open typemap file '$typemap': $!\n"), next;
  my $mode = 'Typemap';
  my $junk = "";
  my $current = \$junk;
  while (<$TYPEMAP>) {
    # skip comments
    next if /^\s*#/;
    if (/^INPUT\s*$/) {
      $mode = 'Input';   $current = \$junk;  next;
    }
    if (/^OUTPUT\s*$/) {
      $mode = 'Output';  $current = \$junk;  next;
    }
    if (/^TYPEMAP\s*$/) {
      $mode = 'Typemap'; $current = \$junk;  next;
    }
    if ($mode eq 'Typemap') {
      chomp;
      my $logged_line = $_;
      trim_whitespace($_);
      # skip blank lines
      next if /^$/;
      my($type,$kind, $proto) =
        m/^\s*(.*?\S)\s+(\S+)\s*($ExtUtils::ParseXS::Constants::PrototypeRegexp*)\s*$/
          or warn(
            "Warning: File '$typemap' Line $.  '$logged_line' " .
            "TYPEMAP entry needs 2 or 3 columns\n"
          ),
          next;
      $type = ExtUtils::Typemaps::tidy_type($type);
      $type_kind_ref->{$type} = $kind;
      # prototype defaults to '$'
      $proto = "\$" unless $proto;
      $proto_letter_ref->{$type} = C_string($proto);
    }
    elsif (/^\s/) {
      $$current .= $_;
    }
    elsif ($mode eq 'Input') {
      s/\s+$//;
      $input_expr_ref->{$_} = '';
      $current = \$input_expr_ref->{$_};
    }
    else {
      s/\s+$//;
      $output_expr_ref->{$_} = '';
      $current = \$output_expr_ref->{$_};
    }
  }
  close $TYPEMAP;
  return ($type_kind_ref, $proto_letter_ref, $input_expr_ref, $output_expr_ref);
}
