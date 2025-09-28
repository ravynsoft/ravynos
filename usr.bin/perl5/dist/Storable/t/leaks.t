#!./perl

use Test::More;
use Storable ();
BEGIN {
eval "use Test::LeakTrace";
plan 'skip_all' => 'Test::LeakTrace required for this tests' if $@;
}
plan 'tests' => 1;

{
    my $c = My::Simple->new;
    my $d;
    my $freezed = Storable::freeze($c);
    no_leaks_ok
    {
        $d = Storable::thaw($freezed);
        undef $d;
    };

    package My::Simple;
    sub new {
        my ($class, $arg) = @_;
        bless {t=>$arg}, $class;
    }
    sub STORABLE_freeze {
        return "abcderfgh";
    }
    sub STORABLE_attach {
        my ($class, $c, $serialized) = @_;
        return $class->new($serialized);
    }
}

{ # [cpan #97316]
  package TestClass;

  sub new {
    my $class = shift;
    return bless({}, $class);
  }
  sub STORABLE_freeze {
    die;
  }

  package main;
  my $obj = TestClass->new;
  eval { freeze($obj); };
}
