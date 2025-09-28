#!./perl

# This is a test for bugs in (?{ }) and (??{ }) caused by corrupting the regex
# engine state within the eval-ed code
# --rafl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
}

fresh_perl_is(<<'CODE', 'ok', {});
'42' =~ /4(?{ 'foo' =~ m{(foo)} })2/
    and print 'ok';
CODE

fresh_perl_is(<<'CODE', 'ok', {}, 'RT#33936');
'aba' =~ /(??{join('',split(qr{(?=)},'aba'))})/
    and print 'ok';
CODE

fresh_perl_is(<<'CODE', 'ok', {}, 'match vars are localized');
my $x = 'aba';
$x =~ s/(a)(?{ 'moo' =~ m{(o)} })/uc($1)/e;
print 'ok' if $x eq 'Aba';
CODE

my $preamble = <<'CODE';
sub build_obj {
  # In the real world we would die on validation fails, but RT#27838
  # is still unresolved, so don't tempt fate.
  $hash->{name} =~ /^[A-Z][a-z]+ [A-Z][a-z]+$/ or return "name error";
  $hash->{age} =~ /^[1-9][0-9]*$/ or return "age error";

  # Add another layer of (?{...}) to try really hard to break things
  $hash->{square} =~
  /^(\d+)(?(?{my $sqrt = sprintf "%.0f", sqrt($^N); $sqrt**2==$^N })|(?!))$/
  or return "squareness error";

  return bless { %$hash }, "Foo";
}

sub match {
  my $str = shift;
  our ($hash, $obj);
  # Do something like Regexp::Grammars does building an object.
  my $matched = $str =~ /
    ()
    ([A-Za-z][A-Za-z ]*)(?{ local $hash->{name} = $^N }),[ ]
    (\d+)(?{ local $hash->{age} = $^N })[ ]years[ ]old,[ ]
    secret[ ]number[ ](\d+)(?{ local $hash->{square} = $^N }).
    (?{ $obj = build_obj(); })
  /x;

  if ($matched) {
    print "match ";
    if (ref($obj)) {
      print ref($obj), ":$obj->{name}:$obj->{age}:$obj->{square}";
    } else {
      print $obj, ":$hash->{name}:$hash->{age}:$hash->{square}";
    }
  } else {
    print "no match $hash->{name}:$hash->{age}:$hash->{square}";
  }

}
CODE

fresh_perl_is($preamble . <<'CODE', 'match Foo:John Smith:42:36', {}, 'regex distillation 1');
match("John Smith, 42 years old, secret number 36.");
CODE

fresh_perl_is($preamble . <<'CODE', 'match Foo:John Smith:42:36', {}, 'regex distillation 2');
match("Jim Jones, 35 years old, secret wombat 007."
  ." John Smith, 42 years old, secret number 36.");
CODE

fresh_perl_is($preamble . <<'CODE', 'match squareness error:::', {}, 'regex distillation 3');
match("John Smith, 54 years old, secret number 7.");
CODE

fresh_perl_is($preamble . <<'CODE', 'no match ::', {}, 'regex distillation 4');
match("Jim Jones, 35 years old, secret wombat 007.");
CODE

# RT #129199: this is mainly for ASAN etc's benefit
fresh_perl_is(<<'CODE', '', {}, "RT #129199:");
/(?{<<""})/
0
CODE

done_testing;
