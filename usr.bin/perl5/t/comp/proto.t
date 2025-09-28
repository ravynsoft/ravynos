#!./perl
#
# Contributed by Graham Barr <Graham.Barr@tiuk.ti.com>
#
# So far there are tests for the following prototypes.
# none, () ($) ($@) ($%) ($;$) (&) (&\@) (&@) (%) (\%) (\@)
#
# It is impossible to test every prototype that can be specified, but
# we should test as many as we can.
#

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

# We need this, as in places we're testing the interaction of prototypes with
# strict
use strict;

print "1..216\n";

my $i = 1;

sub testing (&$) {
    my $p = prototype(shift);
    my $c = shift;
    my $what = defined $c ? '(' . $p . ')' : 'no prototype';   
    print '#' x 25,"\n";
    print '# Testing ',$what,"\n";
    print '#' x 25,"\n";
    print "not "
	if((defined($p) && defined($c) && $p ne $c)
	   || (defined($p) != defined($c)));
    printf "ok %d\n",$i++;
}

@_ = qw(a b c d);
my @array;
my %hash;

##
##
##

testing \&no_proto, undef;

sub no_proto {
    print "# \@_ = (",join(",",@_),")\n";
    scalar(@_)
}

print "not " unless 0 == no_proto();
printf "ok %d\n",$i++;

print "not " unless 1 == no_proto(5);
printf "ok %d\n",$i++;

print "not " unless 4 == &no_proto;
printf "ok %d\n",$i++;

print "not " unless 1 == no_proto +6;
printf "ok %d\n",$i++;

print "not " unless 4 == no_proto(@_);
printf "ok %d\n",$i++;

##
##
##


testing \&no_args, '';

sub no_args () {
    print "# \@_ = (",join(",",@_),")\n";
    scalar(@_)
}

print "not " unless 0 == no_args();
printf "ok %d\n",$i++;

print "not " unless 0 == no_args;
printf "ok %d\n",$i++;

print "not " unless 5 == no_args +5;
printf "ok %d\n",$i++;

print "not " unless 4 == &no_args;
printf "ok %d\n",$i++;

print "not " unless 2 == &no_args(1,2);
printf "ok %d\n",$i++;

eval "no_args(1)";
print "not " unless $@;
printf "ok %d\n",$i++;

##
##
##

testing \&one_args, '$';

sub one_args ($) {
    print "# \@_ = (",join(",",@_),")\n";
    scalar(@_)
}

print "not " unless 1 == one_args(1);
printf "ok %d\n",$i++;

print "not " unless 1 == one_args +5;
printf "ok %d\n",$i++;

print "not " unless 4 == &one_args;
printf "ok %d\n",$i++;

print "not " unless 2 == &one_args(1,2);
printf "ok %d\n",$i++;

eval "one_args(1,2)";
print "not " unless $@;
printf "ok %d\n",$i++;

eval "one_args()";
print "not " unless $@;
printf "ok %d\n",$i++;

sub one_a_args ($) {
    print "# \@_ = (",join(",",@_),")\n";
    print "not " unless @_ == 1 && $_[0] == 4;
    printf "ok %d\n",$i++;
}

one_a_args(@_);

##
##
##

testing \&over_one_args, '$@';

sub over_one_args ($@) {
    print "# \@_ = (",join(",",@_),")\n";
    scalar(@_)
}

print "not " unless 1 == over_one_args(1);
printf "ok %d\n",$i++;

print "not " unless 2 == over_one_args(1,2);
printf "ok %d\n",$i++;

print "not " unless 1 == over_one_args +5;
printf "ok %d\n",$i++;

print "not " unless 4 == &over_one_args;
printf "ok %d\n",$i++;

print "not " unless 2 == &over_one_args(1,2);
printf "ok %d\n",$i++;

print "not " unless 5 == &over_one_args(1,@_);
printf "ok %d\n",$i++;

eval "over_one_args()";
print "not " unless $@;
printf "ok %d\n",$i++;

sub over_one_a_args ($@) {
    print "# \@_ = (",join(",",@_),")\n";
    print "not " unless @_ >= 1 && $_[0] == 4;
    printf "ok %d\n",$i++;
}

over_one_a_args(@_);
over_one_a_args(@_,1);
over_one_a_args(@_,1,2);
over_one_a_args(@_,@_);

##
##
##

testing \&scalar_and_hash, '$%';

sub scalar_and_hash ($%) {
    print "# \@_ = (",join(",",@_),")\n";
    scalar(@_)
}

print "not " unless 1 == scalar_and_hash(1);
printf "ok %d\n",$i++;

print "not " unless 3 == scalar_and_hash(1,2,3);
printf "ok %d\n",$i++;

print "not " unless 1 == scalar_and_hash +5;
printf "ok %d\n",$i++;

print "not " unless 4 == &scalar_and_hash;
printf "ok %d\n",$i++;

print "not " unless 2 == &scalar_and_hash(1,2);
printf "ok %d\n",$i++;

print "not " unless 5 == &scalar_and_hash(1,@_);
printf "ok %d\n",$i++;

eval "scalar_and_hash()";
print "not " unless $@;
printf "ok %d\n",$i++;

sub scalar_and_hash_a ($@) {
    print "# \@_ = (",join(",",@_),")\n";
    print "not " unless @_ >= 1 && $_[0] == 4;
    printf "ok %d\n",$i++;
}

scalar_and_hash_a(@_);
scalar_and_hash_a(@_,1);
scalar_and_hash_a(@_,1,2);
scalar_and_hash_a(@_,@_);

##
##
##

testing \&one_or_two, '$;$';

sub one_or_two ($;$) {
    print "# \@_ = (",join(",",@_),")\n";
    scalar(@_)
}

print "not " unless 1 == one_or_two(1);
printf "ok %d\n",$i++;

print "not " unless 2 == one_or_two(1,3);
printf "ok %d\n",$i++;

print "not " unless 1 == one_or_two +5;
printf "ok %d\n",$i++;

print "not " unless 4 == &one_or_two;
printf "ok %d\n",$i++;

print "not " unless 3 == &one_or_two(1,2,3);
printf "ok %d\n",$i++;

print "not " unless 5 == &one_or_two(1,@_);
printf "ok %d\n",$i++;

eval "one_or_two()";
print "not " unless $@;
printf "ok %d\n",$i++;

eval "one_or_two(1,2,3)";
print "not " unless $@;
printf "ok %d\n",$i++;

sub one_or_two_a ($;$) {
    print "# \@_ = (",join(",",@_),")\n";
    print "not " unless @_ >= 1 && $_[0] == 4;
    printf "ok %d\n",$i++;
}

one_or_two_a(@_);
one_or_two_a(@_,1);
one_or_two_a(@_,@_);

##
##
##

testing \&a_sub, '&';

sub a_sub (&) {
    print "# \@_ = (",join(",",@_),")\n";
    return unless defined $_[0];
    &{$_[0]};
}

sub tmp_sub_1 { printf "ok %d\n",$i++ }

a_sub { printf "ok %d\n",$i++ };
a_sub \&tmp_sub_1;
a_sub \(&tmp_sub_1);

@array = ( \&tmp_sub_1 );
eval 'a_sub @array';
print "not " unless $@;
printf "ok %d\n",$i++;
eval 'a_sub \@array';
print "not " unless $@ =~ /Type of arg/;
printf "ok %d\n",$i++;
eval 'a_sub \%hash';
print "not " unless $@ =~ /Type of arg/;
printf "ok %d\n",$i++;
eval 'a_sub \$scalar';
print "not " unless $@ =~ /Type of arg/;
printf "ok %d\n",$i++;
eval 'a_sub \($list, %of, @refs)';
print "not " unless $@ =~ /Type of arg/;
printf "ok %d\n",$i++;
eval 'a_sub undef';
print "not " if $@;
printf "ok %d\n",$i++;

##
##
##

testing \&a_subx, '\&';

sub a_subx (\&) {
    print "# \@_ = (",join(",",@_),")\n";
    &{$_[0]};
}

sub tmp_sub_2 { printf "ok %d\n",$i++ }
a_subx &tmp_sub_2;

@array = ( \&tmp_sub_2 );
eval 'a_subx @array';
print "not " unless $@;
printf "ok %d\n",$i++;
my $bad =
    qr/Type of arg 1 to .* must be subroutine \(not subroutine entry\)/;
eval 'a_subx &tmp_sub_2()';
print "not " unless $@ =~ $bad;
printf "ok %d - \\& prohibits &foo()\n",$i++;
eval 'a_subx tmp_sub_2()';
print "not " unless $@ =~ $bad;
printf "ok %d - \\& prohibits foo()\n",$i++;
eval 'a_subx tmp_sub_2';
print "not " unless $@ =~ $bad;
printf "ok %d - \\& prohibits foo where foo is an existing sub\n",$i++;

##
##
##

testing \&sub_aref, '&\@';

sub sub_aref (&\@) {
    print "# \@_ = (",join(",",@_),")\n";
    my($sub,$array) = @_;
    print "not " unless @_ == 2 && @{$array} == 4;
    print map { &{$sub}($_) } @{$array}
}

@array = (qw(O K)," ", $i++);
sub_aref { lc shift } @array;
print "\n";

##
##
##

testing \&sub_array, '&@';

sub sub_array (&@) {
    print "# \@_ = (",join(",",@_),")\n";
    print "not " unless @_ == 5;
    my $sub = shift;
    print map { &{$sub}($_) } @_
}

@array = (qw(O K)," ", $i++);
sub_array { lc shift } @array;
sub_array { lc shift } ('O', 'K', ' ', $i++);
print "\n";

##
##
##

testing \&a_hash, '%';

sub a_hash (%) {
    print "# \@_ = (",join(",",@_),")\n";
    scalar(@_);
}

print "not " unless 1 == a_hash 'a';
printf "ok %d\n",$i++;

print "not " unless 2 == a_hash 'a','b';
printf "ok %d\n",$i++;

##
##
##

testing \&a_hash_ref, '\%';

sub a_hash_ref (\%) {
    print "# \@_ = (",join(",",@_),")\n";
    print "not " unless ref($_[0]) && $_[0]->{'a'};
    printf "ok %d\n",$i++;
    $_[0]->{'b'} = 2;
}

%hash = ( a => 1);
a_hash_ref %hash;
print "not " unless $hash{'b'} == 2;
printf "ok %d\n",$i++;

%hash = ( a => 1);
a_hash_ref +(%hash);
print "not " unless $hash{'b'} == 2;
printf "ok %d\n",$i++;

##
##
##

testing \&array_ref_plus, '\@@';

sub array_ref_plus (\@@) {
    print "# \@_ = (",join(",",@_),")\n";
    print "not " unless @_ == 2 && ref($_[0]) && 1 == @{$_[0]} && $_[1] eq 'x';
    printf "ok %d\n",$i++;
    @{$_[0]} = (qw(ok)," ",$i++,"\n");
}

@array = ('a');
{ my @more = ('x');
  array_ref_plus @array, @more; }
print "not " unless @array == 4;
print @array;

@array = ('a');
{ my @more = ('x');
  array_ref_plus +(@array), @more; }
print "not " unless @array == 4;
print @array;

##
##
##

my $p;
print "not " if defined prototype('CORE::print');
print "ok ", $i++, "\n";

print "not " if defined prototype('CORE::system');
print "ok ", $i++, "\n";

print "# CORE::open => ($p)\nnot " if ($p = prototype('CORE::open')) ne '*;$@';
print "ok ", $i++, "\n";

print "# CORE::Foo => ($p), \$@ => '$@'\nnot " 
    if defined ($p = eval { prototype('CORE::Foo') or 1 }) or $@ !~ /^Can't find an opnumber/;
print "ok ", $i++, "\n";

eval { prototype("CORE::a\0b") };
print "# CORE::a\\0b: \$@ => '$@'\nnot " 
    if $@ !~ /^Can't find an opnumber for "a\0b"/;
print "ok ", $i++, "\n";

eval { prototype("CORE::\x{100}") };
print "# CORE::\\x{100}: => ($p), \$@ => '$@'\nnot " 
    if $@ !~ /^Can't find an opnumber for "\x{100}"/;
print "ok ", $i++, "\n";

"CORE::Foo" =~ /(.*)/;
print "# \$1 containing CORE::Foo => ($p), \$@ => '$@'\nnot " 
    if defined ($p = eval { prototype($1) or 1 })
    or $@ !~ /^Can't find an opnumber/;
print "ok ", $i++, " - \$1 containing CORE::Foo\n";

# correctly note too-short parameter lists that don't end with '$',
#  a possible regression.

sub foo1 ($\@);
eval q{ foo1 "s" };
print "not " unless $@ =~ /^Not enough/;
print "ok ", $i++, "\n";

sub foo2 ($\%);
eval q{ foo2 "s" };
print "not " unless $@ =~ /^Not enough/;
print "ok ", $i++, "\n";

sub X::foo3;
*X::foo3 = sub {'ok'};
print "# $@not " unless eval {X->foo3} eq 'ok';
print "ok ", $i++, "\n";

sub X::foo4 ($);
*X::foo4 = sub ($) {'ok'};
print "not " unless X->foo4 eq 'ok';
print "ok ", $i++, "\n";

# test if the (*) prototype allows barewords, constants, scalar expressions,
# globs and globrefs (just as CORE::open() does), all under stricture
sub star (*&) { &{$_[1]} }
sub star2 (**&) { &{$_[2]} }
sub BAR { "quux" }
sub Bar::BAZ { "quuz" }
my $star = 'FOO';
star FOO, sub {
    print "not " unless $_[0] eq 'FOO';
    print "ok $i - star FOO\n";
}; $i++;
star(FOO, sub {
	print "not " unless $_[0] eq 'FOO';
	print "ok $i - star(FOO)\n";
    }); $i++;
star "FOO", sub {
    print "not " unless $_[0] eq 'FOO';
    print qq/ok $i - star "FOO"\n/;
}; $i++;
star("FOO", sub {
	print "not " unless $_[0] eq 'FOO';
	print qq/ok $i - star("FOO")\n/;
    }); $i++;
star $star, sub {
    print "not " unless $_[0] eq 'FOO';
    print "ok $i - star \$star\n";
}; $i++;
star($star, sub {
	print "not " unless $_[0] eq 'FOO';
	print "ok $i - star(\$star)\n";
    }); $i++;
star *FOO, sub {
    print "not " unless $_[0] eq \*FOO;
    print "ok $i - star *FOO\n";
}; $i++;
star(*FOO, sub {
	print "not " unless $_[0] eq \*FOO;
	print "ok $i - star(*FOO)\n";
    }); $i++;
star \*FOO, sub {
    print "not " unless $_[0] eq \*FOO;
    print "ok $i - star \\*FOO\n";
}; $i++;
star(\*FOO, sub {
	print "not " unless $_[0] eq \*FOO;
	print "ok $i - star(\\*FOO)\n";
    }); $i++;
star2 FOO, BAR, sub {
    print "not " unless $_[0] eq 'FOO' and $_[1] eq 'quux';
    print "ok $i - star2 FOO, BAR\n";
}; $i++;
star2(Bar::BAZ, FOO, sub {
	print "not " unless $_[0] eq 'quuz' and $_[1] eq 'FOO';
	print "ok $i - star2(Bar::BAZ, FOO)\n"
    }); $i++;
star2 BAR(), FOO, sub {
    print "not " unless $_[0] eq 'quux' and $_[1] eq 'FOO';
    print "ok $i - star2 BAR(), FOO\n"
}; $i++;
star2(FOO, BAR(), sub {
	print "not " unless $_[0] eq 'FOO' and $_[1] eq 'quux';
	print "ok $i - star2(FOO, BAR())\n";
    }); $i++;
star2 "FOO", "BAR", sub {
    print "not " unless $_[0] eq 'FOO' and $_[1] eq 'BAR';
    print qq/ok $i - star2 "FOO", "BAR"\n/;
}; $i++;
star2("FOO", "BAR", sub {
	print "not " unless $_[0] eq 'FOO' and $_[1] eq 'BAR';
	print qq/ok $i - star2("FOO", "BAR")\n/;
    }); $i++;
star2 $star, $star, sub {
    print "not " unless $_[0] eq 'FOO' and $_[1] eq 'FOO';
    print "ok $i - star2 \$star, \$star\n";
}; $i++;
star2($star, $star, sub {
	print "not " unless $_[0] eq 'FOO' and $_[1] eq 'FOO';
	print "ok $i - star2(\$star, \$star)\n";
    }); $i++;
star2 *FOO, *BAR, sub {
    print "not " unless $_[0] eq \*FOO and $_[1] eq \*BAR;
    print "ok $i - star2 *FOO, *BAR\n";
}; $i++;
star2(*FOO, *BAR, sub {
	print "not " unless $_[0] eq \*FOO and $_[1] eq \*BAR;
	print "ok $i - star2(*FOO, *BAR)\n";
    }); $i++;
star2 \*FOO, \*BAR, sub {
    no strict 'refs';
    print "not " unless $_[0] eq \*{'FOO'} and $_[1] eq \*{'BAR'};
    print "ok $i - star2 \*FOO, \*BAR\n";
}; $i++;
star2(\*FOO, \*BAR, sub {
	no strict 'refs';
	print "not " unless $_[0] eq \*{'FOO'} and $_[1] eq \*{'BAR'};
	print "ok $i - star2(\*FOO, \*BAR)\n";
    }); $i++;

# [perl #118585]
# Test that multiple semicolons are treated as one with *
sub star3(;;;*){}
sub star4( ; ; ; ; *){}
print "not " unless eval 'star3 STDERR; 1';
print "ok ", $i++, " star3 STDERR\n";
print "not " unless eval 'star4 STDERR; 1';
print "ok ", $i++, " star4 STDERR\n";

# [perl #2726]
# Test that prototype binding is late
print "not " unless eval 'sub l564($){ l564(); } 1';
print "ok ", $i++, " prototype checking not done within initial definition\n";
print "not " if eval 'sub l566($); sub l566($){ l566(); } 1';
print "ok ", $i++, " prototype checking done if sub pre-declared\n";

# test scalarref prototype
sub sreftest (\$$) {
    print "not " unless ref $_[0];
    print "ok $_[1] - sreftest\n";
}
{
    no strict 'vars';
    sreftest my $sref, $i++;
    sreftest($helem{$i}, $i++);
    sreftest $aelem[0], $i++;
    sreftest sub { [0] }->()[0], $i++;
    sreftest my $a = 'quidgley', $i++;
    print "not " if eval 'return 1; sreftest(3+4)';
    print "ok ", $i++, ' - \$ with invalid argument', "\n";
}

# test single term
sub lazy (+$$) {
    print "not " unless @_ == 3 && ref $_[0] eq $_[1];
    print "ok $_[2] - non container test\n";
}
sub quietlazy (+) { return shift(@_) }
sub give_aref { [] }
sub list_or_scalar { wantarray ? (1..10) : [] }
{
    my @multiarray = ("a".."z");
    my %bighash = @multiarray;
    lazy(\@multiarray, 'ARRAY', $i++);
    lazy(\%bighash, 'HASH', $i++);
    lazy({}, 'HASH', $i++);
    lazy(give_aref, 'ARRAY', $i++);
    lazy(3, '', $i++); # allowed by prototype, even if runtime error
    lazy(list_or_scalar, 'ARRAY', $i++); # propagate scalar context
}

# test prototypes when they are evaled and there is a syntax error
# Byacc generates the string "syntax error".  Bison gives the
# string "parse error".
#
for my $p ( "", qw{ () ($) ($@) ($%) ($;$) (&) (&\@) (&@) (%) (\%) (\@) } ) {
  my $warn = "";
  local $SIG{__WARN__} = sub {
    my $thiswarn = join("",@_);
    return if $thiswarn =~ /^Prototype mismatch: sub main::evaled_subroutine/;
    $warn .= $thiswarn;
  };
  my $eval = "sub evaled_subroutine $p { &void *; }";
  eval $eval;
  print "# eval[$eval]\nnot " unless $@ && $@ =~ /(parse|syntax) error/i;
  print "ok ", $i++, "\n";
  if ($warn eq '') {
     print "ok ", $i++, "\n";
  } else {
    print "not ok ", $i++, "# $warn \n";
  }
}

{
    my $myvar;
    my @myarray;
    my %myhash;
    sub mysub { print "not calling mysub I hope\n" }
    local *myglob;

    sub myref (\[$@%&*]) { print "# $_[0]\n"; return "$_[0]" }

    print "not " unless myref($myvar)   =~ /^SCALAR\(/;
    print "ok ", $i++, "\n";
    print "not " unless myref($myvar=7) =~ /^SCALAR\(/;
    print "ok ", $i++, "\n";
    print "not " unless myref(@myarray) =~ /^ARRAY\(/;
    print "ok ", $i++, "\n";
    print "not " unless myref(%myhash)  =~ /^HASH\(/;
    print "ok ", $i++, "\n";
    print "not " unless myref(&mysub)   =~ /^CODE\(/;
    print "ok ", $i++, "\n";
    print "not " unless myref(*myglob)  =~ /^GLOB\(/;
    print "ok ", $i++, "\n";

    eval q/sub multi1 (\[%@]) { 1 } multi1 $myvar;/;
    print "not "
	unless $@ =~ /Type of arg 1 to main::multi1 must be one of \[%\@\] /;
    print "ok ", $i++, "\n";
    eval q/sub multi2 (\[$*&]) { 1 } multi2 @myarray;/;
    print "not "
	unless $@ =~ /Type of arg 1 to main::multi2 must be one of \[\$\*&\] /;
    print "ok ", $i++, "\n";
    eval q/sub multi3 (\[$@]) { 1 } multi3 %myhash;/;
    print "not "
	unless $@ =~ /Type of arg 1 to main::multi3 must be one of \[\$\@\] /;
    print "ok ", $i++, "\n";
    eval q/sub multi4 ($\[%]) { 1 } multi4 1, &mysub;/;
    print "not "
	unless $@ =~ /Type of arg 2 to main::multi4 must be one of \[%\] /;
    print "ok ", $i++, "\n";
    eval q/sub multi5 (\[$@]$) { 1 } multi5 *myglob;/;
    print "not "
	unless $@ =~ /Type of arg 1 to main::multi5 must be one of \[\$\@\] /
	    && $@ =~ /Not enough arguments/;
    print "ok ", $i++, "\n";
}

# check that obviously bad prototypes are getting warnings
{
  local $^W = 1;
  my $warn = "";
  local $SIG{__WARN__} = sub { $warn .= join("",@_) };
  
  eval 'sub badproto (@bar) { 1; }';
  print "not " unless $warn =~ /Illegal character in prototype for main::badproto : \@bar/;
  print "ok ", $i++, " checking badproto - (\@bar)\n";

  eval 'sub badproto2 (bar) { 1; }';
  print "not " unless $warn =~ /Illegal character in prototype for main::badproto2 : bar/;
  print "ok ", $i++, " checking badproto2 - (bar)\n";
  
  eval 'sub badproto3 (&$bar$@) { 1; }';
  print "not " unless $warn =~ /Illegal character in prototype for main::badproto3 : &\$bar\$\@/;
  print "ok ", $i++, " checking badproto3 - (&\$bar\$\@)\n";
  
  eval 'sub badproto4 (@ $b ar) { 1; }';
  # This one emits two warnings
  print "not " unless $warn =~ /Illegal character in prototype for main::badproto4 : \@ \$b ar/;
  print "ok ", $i++, " checking badproto4 - (\@ \$b ar) - illegal character\n";
  print "not " unless $warn =~ /Prototype after '\@' for main::badproto4 : \@ \$b ar/;
  print "ok ", $i++, " checking badproto4 - (\@ \$b ar) - prototype after '\@'\n";

  eval 'sub badproto5 ($_$) { 1; }';
  print "not " unless $warn =~ /Illegal character after '_' in prototype for main::badproto5 : \$_\$/;
  print "ok ", $i++, " checking badproto5 - (\$_\$) - illegal character after '_'\n";
  print "not " if $warn =~ /Illegal character in prototype for main::badproto5 : \$_\$/;
  print "ok ", $i++, " checking badproto5 - (\$_\$) - but not just illegal character\n";

  eval 'sub badproto6 (bar_) { 1; }';
  print "not " unless $warn =~ /Illegal character in prototype for main::badproto6 : bar_/;
  print "ok ", $i++, " checking badproto6 - (bar_) - illegal character\n";
  print "not " if $warn =~ /Illegal character after '_' in prototype for main::badproto6 : bar_/;
  print "ok ", $i++, " checking badproto6 - (bar_) - shouldn't add \"after '_'\"\n";

  eval 'sub badproto7 (_;bar) { 1; }';
  print "not " unless $warn =~ /Illegal character in prototype for main::badproto7 : _;bar/;
  print "ok ", $i++, " checking badproto7 - (_;bar) - illegal character\n";
  print "not " if $warn =~ /Illegal character after '_' in prototype for main::badproto7 : _;bar/;
  print "ok ", $i++, " checking badproto7 - (_;bar) - shouldn't add \"after '_'\"\n";

  eval 'sub badproto8 (_b) { 1; }';
  print "not " unless $warn =~ /Illegal character after '_' in prototype for main::badproto8 : _b/;
  print "ok ", $i++, " checking badproto8 - (_b) - illegal character after '_'\n";
  print "not " unless $warn =~ /Illegal character in prototype for main::badproto8 : _b/;
  print "ok ", $i++, " checking badproto8 - (_b) - just illegal character\n";

  eval 'sub badproto9 ([) { 1; }';
  print "not " unless $warn =~ /Missing '\]' in prototype for main::badproto9 : \[/;
  print "ok ", $i++, " checking for matching bracket\n";

  eval 'sub badproto10 ([_]) { 1; }';
  print "not " if $warn =~ /Missing '\]' in prototype for main::badproto10 : \[/;
  print "ok ", $i++, " checking badproto10 - ([_]) - shouldn't trigger matching bracket\n";
  print "not " unless $warn =~ /Illegal character after '_' in prototype for main::badproto10 : \[_\]/;
  print "ok ", $i++, " checking badproto10 - ([_]) - should trigger after '_' warnings\n";
}

# make sure whitespace in prototypes works
eval "sub good (\$\t\$\n\$) { 1; }";
print "not " if $@;
print "ok ", $i++, "\n";
# [perl #118629]
{
  my $warnings = 0;
  local $SIG{__WARN__} = sub { $warnings++;};
  $::{ckproto_test} = ' $ $ ';
	eval 'sub ckproto_test($$){1;}';
  print "not " if $warnings;
  print "ok ", $i++, " Check that ckproto ignores spaces in comparisons\n";
}

# Ought to fail, doesn't in 5.8.1.
eval 'sub bug (\[%@]) {  } my $array = [0 .. 1]; bug %$array;';
print "not " unless $@ =~ /Not a HASH reference/;
print "ok ", $i++, "\n";

# [perl #75904]
# Test that the following prototypes make subs parse as unary functions:
#  * \sigil \[...] ;$ ;* ;\sigil ;\[...]
# [perl #118585]
# As a special case, make sure that ;;* is treated the same as ;*
print "not "
 unless eval 'sub uniproto1 (*) {} uniproto1 $_, 1' or warn $@;
print "ok ", $i++, "\n";
print "not "
 unless eval 'sub uniproto2 (\$) {} uniproto2 $_, 1' or warn $@;
print "ok ", $i++, "\n";
print "not "
 unless eval 'sub uniproto3 (\[$%]) {} uniproto3 %_, 1' or warn $@;
print "ok ", $i++, "\n";
print "not "
 unless eval 'sub uniproto4 (;$) {} uniproto4 $_, 1' or warn $@;
print "ok ", $i++, "\n";
print "not "
 unless eval 'sub uniproto5 (;*) {} uniproto5 $_, 1' or warn $@;
print "ok ", $i++, "\n";
print "not "
 unless eval 'sub uniproto6 (;\@) {} uniproto6 @_, 1' or warn $@;
print "ok ", $i++, "\n";
print "not "
 unless eval 'sub uniproto7 (;\[$%@]) {} uniproto7 @_, 1' or warn $@;
print "ok ", $i++, "\n";
print "not "
 unless eval 'sub uniproto8 (+) {} uniproto8 $_, 1' or warn $@;
print "ok ", $i++, "\n";
print "not "
 unless eval 'sub uniproto9 (;+) {} uniproto9 $_, 1' or warn $@;
print "ok ", $i++, "\n";
print "not "
 unless eval 'sub uniproto10 (;;;*) {} uniproto10 $_, 1' or warn $@;
print "ok ", $i++, " - uniproto10 (;;;*)\n";
print "not "
 unless eval 'sub uniproto11 ( ; ; ; * ) {} uniproto10 $_, 1' or warn $@;
print "ok ", $i++, " - uniproto11 ( ; ; ;  *)\n";
print "not "
 unless eval 'sub uniproto12 (;;;+) {} uniproto12 $_, 1' or warn $@;
print "ok ", $i++, " - uniproto12 (;;;*)\n";
print "not "
 unless eval 'sub uniproto13 ( ; ; ; + ) {} uniproto13 $_, 1' or warn $@;
print "ok ", $i++, " - uniproto13 ( ; ; ; * )\n";


# Test that a trailing semicolon makes a sub have listop precedence
sub unilist ($;)  { $_[0]+1 }
sub unilist2(_;)  { $_[0]+1 }
sub unilist3(;$;) { $_[0]+1 }
print "not " unless (unilist 0 || 5) == 6;
print "ok ", $i++, "\n";
print "not " unless (unilist2 0 || 5) == 6;
print "ok ", $i++, "\n";
print "not " unless (unilist3 0 || 5) == 6;
print "ok ", $i++, "\n";

{
  # Lack of prototype on a subroutine definition should override any prototype
  # on the declaration.
  sub z_zwap (&);

  local $SIG{__WARN__} = sub {
    my $thiswarn = join "",@_;
    if ($thiswarn =~ /^Prototype mismatch: sub main::z_zwap/) {
      print 'ok ', $i++, "\n";
    } else {
      print 'not ok ', $i++, "\n";
      print STDERR $thiswarn;
    }
  };

  eval q{sub z_zwap {return @_}};

  if ($@) {
    print "not ok ", $i++, "# $@";
  } else {
    print "ok ", $i++, "\n";
  }


  my @a = (6,4,2);
  my @got  = eval q{z_zwap(@a)};

  if ($@) {
    print "not ok ", $i++, " # $@";
  } else {
    print "ok ", $i++, "\n";
  }

  if ("@got" eq "@a") {
    print "ok ", $i++, "\n";
  } else {
    print "not ok ", $i++, " # >@got<\n";
  }
}

# [perl #123514] prototype with no arguments
$_ = sub ($$$$$$$) {};
@_ = (1, 2, 3, prototype(), 4, 5, 6);
print "not " unless "@_" eq '1 2 3 $$$$$$$ 4 5 6';
print "ok ", $i++, " - [perl #123514] (got @_)\n";

{
    my $weird_failure = q<>;

    if (eval 'sub scalarref (\$) { }; scalarref( sub {} ); 1') {
        print "not ok $i: Unexpected scalarref success!\n";
    }
    else {
        if ($@ !~ m/anonymous subroutine/ || $@ !~ m/scalarref/) {
            $weird_failure = $@;

            $weird_failure =~ s/^/# /m;

            print "# Unexpected error (or none):$/$weird_failure";
        }

        print( ($weird_failure ? 'not ok' : 'ok') . " $i - anonsub passed to \\\$\n" );
    }

    $i++;
}
