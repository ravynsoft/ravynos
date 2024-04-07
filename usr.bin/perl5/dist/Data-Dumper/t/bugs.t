#!perl
#
# regression tests for old bugs that do not fit other categories

use strict;
use warnings;

use Test::More tests => 24;
use Data::Dumper;

{
    sub iterate_hash {
	my ($h) = @_;
	my $count = 0;
	$count++ while each %$h;
	return $count;
    }

    my $dumper = Data::Dumper->new( [\%ENV], ['ENV'] )->Sortkeys(1);
    my $orig_count = iterate_hash(\%ENV);
    $dumper->Dump;
    my $new_count = iterate_hash(\%ENV);
    is($new_count, $orig_count, 'correctly resets hash iterators');
}

# [perl #38612] Data::Dumper core dump in 5.8.6, fixed by 5.8.7
sub foo {
     my $s = shift;
     local $Data::Dumper::Terse = 1;
     my $c = eval Dumper($s);
     sub bar::quote { }
     bless $c, 'bar';
     my $d = Data::Dumper->new([$c]);
     $d->Freezer('quote');
     return $d->Dump;
}
foo({});
ok(1, "[perl #38612]"); # Still no core dump? We are fine.

{
    my %h = (1,2,3,4);
    each %h;

    my $d = Data::Dumper->new([\%h]);
    $d->Useqq(1);
    my $txt = $d->Dump();
    my $VAR1;
    eval $txt;
    is_deeply($VAR1, \%h, '[perl #40668] Reset hash iterator'); 
}

# [perl #64744] Data::Dumper each() bad interaction
{
    local $Data::Dumper::Useqq = 1;
    my $a = {foo => 1, bar => 1};
    each %$a;
    $a = {x => $a};

    my $d = Data::Dumper->new([$a]);
    $d->Useqq(1);
    my $txt = $d->Dump();
    my $VAR1;
    eval $txt;
    is_deeply($VAR1, $a, '[perl #64744] Reset hash iterator'); 
}

# [perl #56766] Segfaults on bad syntax - fixed with version 2.121_17
sub doh
{
    # 2nd arg is supposed to be an arrayref
    my $doh = Data::Dumper->Dump([\@_],'@_');
}
doh('fixed');
ok(1, "[perl #56766]"); # Still no core dump? We are fine.

SKIP: {
 skip "perl 5.10.1 crashes and DD cannot help it", 1 if $] < 5.0119999;
 # [perl #72332] Segfault on empty-string glob
 Data::Dumper->Dump([*{*STDERR{IO}}]);
 ok("ok", #ok
   "empty-string glob [perl #72332]");
}

# writing out of bounds with malformed utf8
SKIP: {
    eval { require Encode };
    skip("Encode not available", 1) if $@;
    local $^W=1;
    local $SIG{__WARN__} = sub {};
    my $a="\x{fc}'" x 50;
    Encode::_utf8_on($a);
    Dumper $a;
    ok("ok", "no crash dumping malformed utf8 with the utf8 flag on");
}

{
  # We have to test reference equivalence, rather than actual output, as
  # Perl itself is buggy prior to 5.15.6.  Output from DD should at least
  # evaluate to the same typeglob, regardless of perl bugs.
  my $tests = sub {
    my $VAR1;
    no strict 'refs';
    is eval(Dumper \*{"foo::b\0ar"}), \*{"foo::b\0ar"},
      'GVs with nulls';
    # There is a strange 5.6 bug that causes the eval to fail a supposed
    # strict vars test (involving $VAR1).  Mentioning the glob beforehand
    # somehow makes it go away.
    () = \*{chr 256};
    is eval Dumper(\*{chr 256})||die ($@), \*{chr 256},
      'GVs with UTF8 names (or not, depending on perl version)';
    () = \*{"\0".chr 256}; # same bug
    is eval Dumper(\*{"\0".chr 256}), \*{"\0".chr 256},
      'GVs with UTF8 and nulls';
  };
  SKIP: {
    skip "no XS", 3 if not defined &Data::Dumper::Dumpxs;
    local $Data::Dumper::Useperl = 0;
    &$tests;
  }
  local $Data::Dumper::Useperl = 1;
  &$tests;
}

{
  # Test reference equivalence of dumping *{""}.
  my $tests = sub {
    my $VAR1;
    no strict 'refs';
    is eval(Dumper \*{""}), \*{""}, 'dumping \*{""}';
  };
  SKIP: {
    skip "no XS", 1 if not defined &Data::Dumper::Dumpxs;
    local $Data::Dumper::Useperl = 0;
    &$tests;
  }
  local $Data::Dumper::Useperl = 1;
  &$tests;
}

{ # https://rt.perl.org/Ticket/Display.html?id=128524
    my $want;
    my $runtime = "runtime";
    my $requires = "requires";
    utf8::upgrade(my $uruntime = $runtime);
    utf8::upgrade(my $urequires = $requires);
    for my $run ($runtime, $uruntime) {
        for my $req ($requires, $urequires) {
            my $data = { $run => { $req => { foo => "bar" } } };
            local $Data::Dumper::Useperl = 1;
            # we want them all the same
            defined $want or $want = Dumper($data);
            is(Dumper( $data ), $want, "utf-8 indents");
          SKIP:
            {
                defined &Data::Dumper::Dumpxs
                  or skip "No XS available", 1;
                local $Data::Dumper::Useperl = 0;
                is(Dumper( $data ), $want, "utf8-indents");
            }
        }
    }
}

# RT#130487 - stack management bug in XS deparse
SKIP: {
    skip "No XS available", 1 if !defined &Data::Dumper::Dumpxs;
    sub rt130487_args { 0 + @_ }
    my $code = sub {};
    local $Data::Dumper::Useperl = 0;
    local $Data::Dumper::Deparse = 1;
    my $got = rt130487_args( Dumper($code) );
    is($got, 1, "stack management in XS deparse works, rt 130487");
}

# EOF
