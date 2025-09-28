#!./perl -w

#
# Tests derived from Japhs.
#
# These test use obscure features of Perl, or surprising combinations
# of features. The tests were added because in the past, they have
# exposed several bugs in Perl.
#
# Some of these tests may actually (mis)use bugs or use undefined behaviour.
# These tests are still useful - behavioural changes or bugfixes will be
# noted, and a remark can be put in the documentation. (Don't forget to
# disable the test!)
#
# Getting everything to run well on the myriad of platforms Perl runs on
# is unfortunately not a trivial task.
#
# WARNING: these tests are obfuscated.  Do not get frustrated.
# Ask Abigail <abigail@abigail.be>, or use the Deparse or Concise
# modules (the former parses Perl to Perl, the latter shows the
# op syntax tree) like this:
# ./perl -Ilib -MO=Deparse foo.pl
# ./perl -Ilib -MO=Concise foo.pl
#

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require "./test.pl";
    skip_all('EBCDIC') if $::IS_EBCDIC;
    undef &skip;
}

#
# ./test.pl does real evilness by jumping to a label.
# This function copies the skip from ./test, omitting the goto.
#
sub skip {
    my $why  = shift;
    my $n    = @_ ? shift : 1;
    for (1..$n) {
        my $test = curr_test;
        print STDOUT "ok $test # skip: $why\n";
        next_test;
    }
}


#
# ./test.pl doesn't give use 'notok', so we make it here.
#
sub notok {
    my ($pass, $name, @mess) = @_;
    _ok(!$pass, _where(), $name, @mess);
}

my $JaPH   = "Just another Perl Hacker";
my $JaPh   = "Just another Perl hacker";
my $JaPH_n = "Just another Perl Hacker\n";
my $JaPh_n = "Just another Perl hacker\n";
my $JaPH_s = "Just another Perl Hacker ";
my $JaPh_s = "Just another Perl hacker ";
my $JaPH_c = "Just another Perl Hacker,";
my $JaPh_c = "Just another Perl hacker,";

plan tests => 130;
     
{   
    my $out  = sprintf "Just another Perl Hacker";
    is ($out, $JaPH);
}


{   
    my @primes     = (2,  3,  7, 13, 53, 101,  557, 1429);
    my @composites = (4, 10, 25, 32, 75, 143, 1333, 1728);

    my %primeness  = ((map {$_ => 1} @primes),
                      (map {$_ => 0} @composites));

    while (my ($num, $is_prime) = each %primeness) {
        my $comment = "$num is " . ($is_prime ? "prime." : "composite.");

        my $sub     = $is_prime ? "ok" : "notok";

        &$sub (( 1  x $num) !~ /^1?$|^(11+?)\1+$/,       $comment);
        &$sub (( 0  x $num) !~ m 0^\0?$|^(\0\0+?)\1+$0,  $comment);
        &$sub (("m" x $num) !~ m m^\m?$|^(\m\m+?)\1+$mm, $comment);
    }
}


{   # Some platforms use different quoting techniques.
    # I do not have access to those platforms to test
    # things out. So, we'll skip things....
    if ($^O eq 'MSWin32' ||
        $^O eq 'VMS') {
            skip "Your platform quotes differently.", 3;
            last;
    }

    my $expected  =  $JaPH;
       $expected  =~ s/ /\n/g;
       $expected .= "\n";
    is (runperl (switches => [qw /'-weprint<<EOT;' -eJust -eanother
                                   -ePerl -eHacker -eEOT/],
                 verbose  => 0),
        $expected, "Multiple -e switches");

    is (runperl (switches => [q  !'-wle$_=<<EOT;y/\n/ /;print;'!,
                              qw ! -eJust -eanother -ePerl -eHacker -eEOT!],
                 verbose  => 0),
        $JaPH . " \n", "Multiple -e switches");

    is (runperl (switches => [qw !-wl!],
                 progs    => [qw !print qq-@{[ qw+ Just
                                  another Perl Hacker +]}-!],
                 verbose  => 0),
        $JaPH_n, "Multiple -e switches");
}

{
    if ($^O eq 'MSWin32' ||
        $^O eq 'VMS') {
            skip "Your platform quotes differently.", 1;
            last;
    }
    is (runperl (switches => [qw /-sweprint --/,
                              "-_='Just another Perl Hacker'"],
                 nolib    => 1,
                 verbose  => 0),
        $JaPH, 'setting $_ via -s');
}

{
    my $datafile = "datatmp000";
    1 while -f ++ $datafile;
    END {unlink_all $datafile if $datafile}

    open  MY_DATA, "> $datafile" or die "Failed to open $datafile: $!";
    print MY_DATA  << "    --";
        One
        Two
        Three
        Four
        Five
        Six
    --
    close MY_DATA or die "Failed to close $datafile: $!\n";

    my @progs;
    my $key;
    while (<DATA>) {
        last if /^__END__$/;

        if (/^#{7}(?:\s+(.*))?/) {
            push @progs => {COMMENT  => $1 || '',
                            CODE     => '',
                            SKIP_OS  => [],
                            ARGS     => [],
                            SWITCHES => [],};
            $key = 'CODE';
            next;
        }
        elsif (/^(COMMENT|CODE|ARGS|SWITCHES|EXPECT|SKIP|SKIP_OS)
                 (?::\s+(.*))?$/sx) {
            $key = $1;
            $progs [-1] {$key} = '' unless exists $progs [-1] {$key};
            next unless defined $2;
            $_ = $2;
        }
        elsif (/^$/) {
            next;
        }

        if (ref ($progs [-1] {$key})) {
            push @{$progs [-1] {$key}} => $_;
        }
        else {
            $progs [-1] {$key} .=  $_;
        }
    }

    foreach my $program (@progs) {
        if (exists $program -> {SKIP}) {
            chomp  $program -> {SKIP};
            skip   $program -> {SKIP}, 1;
            next;
        }

	chomp @{$program -> {SKIP_OS}};
        if (@{$program -> {SKIP_OS}}) {
            if (grep {$^O eq $_} @{$program -> {SKIP_OS}}) {
                skip "Your OS uses different quoting.", 1;
                next;
            }
        }

        map {s/\$datafile/$datafile/} @{$program -> {ARGS}};
        $program -> {EXPECT} = $JaPH unless exists $program -> {EXPECT};
        $program -> {EXPECT} =~ s/\$JaPH_s\b/$JaPH_s/g;
        $program -> {EXPECT} =~ s/\$JaPh_c\b/$JaPh_c/g;
        $program -> {EXPECT} =~ s/\$JaPh\b/$JaPh/g;
        chomp ($program -> {EXPECT}, @{$program -> {SWITCHES}},
                                     @{$program -> {ARGS}});
        fresh_perl_is ($program -> {CODE},
                       $program -> {EXPECT},
                      {switches => $program -> {SWITCHES},
                       args     => $program -> {ARGS},
                       verbose  =>  0},
                       $program -> {COMMENT});
    }
}

{
    my $progfile = "progtmp000";
    1 while -f ++ $progfile;
    END {unlink_all $progfile if $progfile}

    my @programs = (<< '    --', << '    --');
#!./perl
BEGIN{$|=$SIG{__WARN__}=sub{$_=$_[0];y-_- -;print/(.)"$/;seek _,-open(_ 
,"+<$0"),2;truncate _,tell _;close _;exec$0}}//rekcaH_lreP_rehtona_tsuJ
    --
#!./perl
BEGIN{$SIG{__WARN__}=sub{$_=pop;y-_- -;print/".*(.)"/;  
truncate$0,-1+-s$0;exec$0;}}//rekcaH_lreP_rehtona_tsuJ
    --
    chomp @programs;

    if ($^O eq 'VMS' or $^O eq 'MSWin32') {
        # VMS needs extensions for files to be executable,
        # but the Japhs above rely on $0 being exactly the
        # filename of the program.
        skip $^O, 2 * @programs;
        last
    }

    use Config;
    unless (defined $Config {useperlio}) {
        skip "Uuseperlio", 2 * @programs;
        last
    }

    my $i = 1;
    foreach my $program (@programs) {
        open my $fh => "> $progfile" or die "Failed to open $progfile: $!\n";
        print   $fh $program;
        close   $fh or die "Failed to close $progfile: $!\n";

        chmod 0755   => $progfile or die "Failed to chmod $progfile: $!\n";
        my $command  = "./$progfile 2>&1";
        if ( $^O eq 'qnx' ) {
          skip "#!./perl not supported in QNX4";
          skip "#!./perl not supported in QNX4";
        } else {
          my $output   = `$command`;

          is ($output, $JaPH, "Self correcting code $i");

                 $output   = `$command`;
          is ($output, "",    "Self corrected code $i");
        }
        $i ++;
    }
}

__END__
#######  Funky loop 1.
$_ = q ;4a75737420616e6f74686572205065726c204861636b65720as;;
     for (s;s;s;s;s;s;s;s;s;s;s;s)
         {s;(..)s?;qq qprint chr 0x$1 and \161 ssq;excess;}

#######  Funky loop 2.
$_ = q *4a75737420616e6f74686572205065726c204861636b65720a*;
for ($*=******;$**=******;$**=******) {$**=*******s*..*qq}
print chr 0x$& and q
qq}*excess********}
SKIP: $* was removed.

#######  Funky loop 3.
$_ = q *4a75737420616e6f74686572205065726c204861636b65720a*;
for ($*=******;$**=******;$**=******) {$**=*******s*..*qq}
print chr 0x$& and q
qq}*excess********}
SKIP: $* was removed.

#######  Funky loop 4.
$_ = q ?4a75737420616e6f74686572205065726c204861636b65720as?;??;
for (??;(??)x??;??)
    {??;s;(..)s?;qq ?print chr 0x$1 and \161 ss?;excess;??}
SKIP: Abuses a fixed bug.

#######  Funky loop 5.
for (s??4a75737420616e6f74686572205065726c204861636b65720as?;??;??) 
    {s?(..)s\??qq \?print chr 0x$1 and q ss\??excess}
SKIP: Abuses a fixed bug.

#######  Funky loop 6.
$a = q 94a75737420616e6f74686572205065726c204861636b65720a9 and
${qq$\x5F$} = q 97265646f9 and s g..g;
qq e\x63\x68\x72\x20\x30\x78$&eggee;
{eval if $a =~ s e..eqq qprint chr 0x$& and \x71\x20\x71\x71qeexcess}

#######  Roman Dates.
@r=reverse(M=>(0)x99=>CM=>(0)x399=>D=>(0)x99=>CD=>(
0)x299=>C=>(0)x9=>XC=>(0)x39=>L=>(0)x9=>XL=>(0)x29=>X=>IX=>0=>0=>0=>V=>IV=>0=>0
=>I=>$==-2449231+gm_julian_day+time);do{until($=<$#r){$_.=$r[$#r];$=-=$#r}for(;
!$r[--$#r];){}}while$=;$,="\x20";print+$_=>September=>MCMXCIII=>=>=>=>=>=>=>=>
SWITCHES
-MTimes::JulianDay
-l
SKIP: Times::JulianDay not part of the main distribution.

#######  Autoload 1.
sub _'_{$_'_=~s/$a/$_/}map{$$_=$Z++}Y,a..z,A..X;*{($_::_=sprintf+q=%X==>"$A$Y".
"$b$r$T$u")=~s~0~O~g;map+_::_,U=>T=>L=>$Z;$_::_}=*_;sub _{print+/.*::(.*)/s};;;
*{chr($b*$e)}=*_'_;*__=*{chr(1<<$e)};                # Perl 5.6.0 broke this...
_::_(r(e(k(c(a(H(__(l(r(e(P(__(r(e(h(t(o(n(a(__(t(us(J())))))))))))))))))))))))
EXPECT: Just__another__Perl__Hacker

#######  Autoload 2.
$"=$,;*{;qq{@{[(A..Z)[qq[0020191411140003]=~m[..]g]]}}}=*_=sub{print/::(.*)/};
$\=$/;q<Just another Perl Hacker>->();

#######  Autoload 3.
$"=$,;*{;qq{@{[(A..Z)[qq[0020191411140003]=~m[..]g]]}}}=*_;
sub   _   {push         @_ => /::(.*)/s and goto &{ shift}}
sub shift {print shift; @_              and goto &{+shift}}
Hack ("Just", "Perl ", " ano", "er\n", "ther "); # YYYYMMDD

#######  Autoload 4.
$, = " "; sub AUTOLOAD {($AUTOLOAD =~ /::(.*)/) [0];}
print+Just (), another (), Perl (), Hacker ();

#######  Look ma! No letters!
$@="\145\143\150\157\040\042\112\165\163\164\040\141\156\157\164".
   "\150\145\162\040\120\145\162\154\040\110\141\143\153\145\162".
   "\042\040\076\040\057\144\145\166\057\164\164\171";`$@`
SKIP: Unix specific

#######  sprintf fun 1.
sub f{sprintf$_[0],$_[1],$_[2]}print f('%c%s',74,f('%c%s',117,f('%c%s',115,f(
'%c%s',116,f('%c%s',32,f('%c%s',97,f('%c%s',0x6e,f('%c%s',111,f('%c%s',116,f(
'%c%s',104,f('%c%s',0x65,f('%c%s',114,f('%c%s',32,f('%c%s',80,f('%c%s',101,f(
'%c%s',114,f('%c%s',0x6c,f('%c%s',32,f('%c%s',0x48,f('%c%s',97,f('%c%s',99,f(
'%c%s',107,f('%c%s',101,f('%c%s',114,f('%c%s',10,)))))))))))))))))))))))))

#######  sprintf fun 2.
sub f{sprintf'%c%s',$_[0],$_[1]}print f(74,f(117,f(115,f(116,f(32,f(97,
f(110,f(111,f(116,f(104,f(0x65,f(114,f(32,f(80,f(101,f(114,f(0x6c,f(32,
f(0x48,f(97,f(99,f(107,f(101,f(114,f(10,q ff)))))))))))))))))))))))))

#######  Hanoi.
%0=map{local$_=$_;reverse+chop,$_}ABC,ACB,BAC,BCA,CAB,CBA;$_=3 .AC;1while+
s/(\d+)((.)(.))/($0=$1-1)?"$0$3$0{$2}1$2$0$0{$2}$4":"$3 => $4\n"/xeg;print
EXPECT
A => C
A => B
C => B
A => C
B => A
B => C
A => C

#######  Funky -p 1
}{$_=$.
SWITCHES: -wlp
ARGS:     $datafile
EXPECT:   6

#######  Funky -p 2
}$_=$.;{
SWITCHES: -wlp
ARGS:     $datafile
EXPECT:   6

#######  Funky -p 3
}{$_=$.}{
SWITCHES: -wlp
ARGS:     $datafile
EXPECT:   6

#######  Funky -p 4
}{*_=*.}{
SWITCHES: -wlp
ARGS:     $datafile
EXPECT:   6

#######  Funky -p 5
}for($.){print
SWITCHES: -wln
ARGS:     $datafile
EXPECT:   6

#######  Funky -p 6
}{print$.
SWITCHES: -wln
ARGS:     $datafile
EXPECT:   6

#######  Funky -p 7
}print$.;{
SWITCHES: -wln
ARGS:     $datafile
EXPECT:   6

#######  Abusing -M
1
SWITCHES
-Mstrict='}); print "Just another Perl Hacker"; ({'
-l
SKIP: No longer works in 5.8.2 and beyond.
SKIP_OS: MSWin32

#######  rand
srand 123456;$-=rand$_--=>@[[$-,$_]=@[[$_,$-]for(reverse+1..(@[=split
//=>"IGrACVGQ\x02GJCWVhP\x02PL\x02jNMP"));print+(map{$_^q^"^}@[),"\n"
SKIP: Solaris specific.

#######  print and __PACKAGE__
package Just_another_Perl_Hacker; sub print {($_=$_[0])=~ s/_/ /g;
                                      print } sub __PACKAGE__ { &
                                      print (     __PACKAGE__)} &
                                                  __PACKAGE__
                                            (                )

#######  Decorations.
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
/ / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / / 
% % % % % % % % % % % % % % % % % % % % % % % % % % % % % % % %;
BEGIN {% % = ($ _ = " " => print "Just another Perl Hacker\n")}

#######  Tie 1
sub J::FETCH{Just   }$_.='print+"@{[map';sub J::TIESCALAR{bless\my$J,J}
sub A::FETCH{another}$_.='{tie my($x),$';sub A::TIESCALAR{bless\my$A,A}
sub P::FETCH{Perl   }$_.='_;$x}qw/J A P';sub P::TIESCALAR{bless\my$P,P}
sub H::FETCH{Hacker }$_.=' H/]}\n"';eval;sub H::TIESCALAR{bless\my$H,H}

#######  Tie 2
package Z;use overload'""'=>sub{$b++?Hacker:another};
sub TIESCALAR{bless\my$y=>Z}sub FETCH{$a++?Perl:Just}
$,=$";my$x=tie+my$y=>Z;print$y,$x,$y,$x,"\n";#Abigail
EXPECT: $JaPH_s

#######  Tie 3
sub A::TIESCALAR{bless\my$x=>A};package B;@q[0..3]=qw/Hacker Perl
another Just/;use overload'""'=>sub{pop @q};sub A::FETCH{bless\my
$y=>B}; tie my $shoe => qq 'A';print "$shoe $shoe $shoe $shoe\n";

#######  Tie 4
sub A::TIESCALAR{bless\my$x=>'A'};package B;@q=qw/Hacker Perl
another Just/;use overload'""',sub{pop @q};sub A::FETCH{bless
\my $y=>B};tie my$shoe=>'A';print"$shoe $shoe $shoe $shoe\n";

#######  Tie 5
tie $" => A; $, = " "; $\ = "\n"; @a = ("") x 2; print map {"@a"} 1 .. 4;
sub A::TIESCALAR {bless \my $A => A} #  Yet Another silly JAPH by Abigail
sub A::FETCH     {@q = qw /Just Another Perl Hacker/ unless @q; shift @q}
SKIP: Pending a bug fix.

#######  Prototype fun 1
sub camel (^#87=i@J&&&#]u'^^s]#'#={123{#}7890t[0.9]9@+*`"'***}A&&&}n2o}00}t324i;
h[{e **###{r{+P={**{e^^^#'#i@{r'^=^{l+{#}H***i[0.9]&@a5`"':&^;&^,*&^$43##@@####;
c}^^^&&&k}&&&}#=e*****[]}'r####'`=437*{#};::'1[0.9]2@43`"'*#==[[.{{],,,1278@#@);
print+((($llama=prototype'camel')=~y|+{#}$=^*&[0-9]i@:;`"',.| |d)&&$llama."\n");
SKIP: Abuses a fixed bug.

#######  Prototype fun 2
print prototype sub "Just another Perl Hacker" {};
SKIP: Abuses a fixed bug.

#######  Prototype fun 3
sub _ "Just another Perl Hacker"; print prototype \&_
SKIP: Abuses a fixed bug.

#######  Split 1
               split // => '"';
${"@_"} = "/"; split // => eval join "+" => 1 .. 7;
*{"@_"} = sub {foreach (sort keys %_)  {print "$_ $_{$_} "}};
%{"@_"} = %_ = (Just => another => Perl => Hacker); &{%{%_}};
SKIP: Hashes are now randomized.
EXPECT: $JaPH_s

#######  Split 2
$" = "/"; split // => eval join "+" => 1 .. 7;
*{"@_"} = sub {foreach (sort keys %_) {print "$_ $_{$_} "}};
%_ = (Just => another => Perl => Hacker); &{%_};
SKIP: Hashes are now randomized.
EXPECT: $JaPH_s

#######  Split 3
$" = "/"; split $, => eval join "+" => 1 .. 7;
*{"@_"} = sub  {foreach (sort keys %_) {print "$_ $_{$_} "}};
%{"@_"} = %_ = (Just => another => Perl => Hacker); &{%{%_}};
SKIP: Hashes are now randomized.
EXPECT: $JaPH_s

#######  Here documents 1
$_ = "\x3C\x3C\x45\x4F\x54"; s/<<EOT/<<EOT/e; print;
Just another Perl Hacker
EOT

#######  Here documents 2
$_ = "\x3C\x3C\x45\x4F\x54";
print if s/<<EOT/<<EOT/e;
Just another Perl Hacker
EOT

#######  Here documents 3
$_ = "\x3C\x3C\x45\x4F\x54" and s/<<EOT/<<EOT/e and print;
Just another Perl Hacker
EOT

#######  Here documents 4
$_ = "\x3C\x3C\x45\x4F\x54\n" and s/<<EOT/<<EOT/ee and print;
"Just another Perl Hacker"
EOT

#######  Self modifying code 1
$_ = "goto F.print chop;\n=rekcaH lreP rehtona tsuJ";F1:eval
SWITCHES: -w

#######  Overloaded constants 1
BEGIN {$^H {q} = sub {pop and pop and print pop}; $^H = 2**4.2**12}
"Just "; "another "; "Perl "; "Hacker";
SKIP_OS: qnx

#######  Overloaded constants 2
BEGIN {$^H {q} = sub {$_ [1] =~ y/S-ZA-IK-O/q-tc-fe-m/d; $_ [1]}; $^H = 0x28100}
print "Just another PYTHON hacker\n";
EXPECT: $JaPh

#######  Overloaded constants 3
BEGIN {$^H {join "" => ("a" .. "z") [8, 13, 19, 4, 6, 4, 17]} = sub
           {["", "Just ", "another ", "Perl ", "Hacker\n"] -> [shift]};
       $^H = hex join "" => reverse map {int ($_ / 2)} 0 .. 4}
print 1, 2, 3, 4;

#######  Overloaded constants 4
BEGIN {$^H {join "" => ("a" .. "z") [8, 13, 19, 4, 6, 4, 17]} = sub
           {["", "Just ", "another ", "Perl ", "Hacker"] -> [shift]};
       $^H = hex join "" => reverse map {int ($_ / 2)} 0 .. 4}
print 1, 2, 3, 4, "\n";

#######  Overloaded constants 5
BEGIN {my $x = "Knuth heals rare project\n";
       $^H {integer} = sub {my $y = shift; $_ = substr $x => $y & 0x1F, 1;
       $y > 32 ? uc : lc}; $^H = hex join "" => 2, 1, 1, 0, 0}
print 52,2,10,23,16,8,1,19,3,6,15,12,5,49,21,14,9,11,36,13,22,32,7,18,24;

#######  v-strings 1
print v74.117.115.116.32;
print v97.110.111.116.104.101.114.32;
print v80.101.114.108.32;
print v72.97.99.107.101.114.10;

#######  v-strings 2
print 74.117.115.116.32;
print 97.110.111.116.104.101.114.32;
print 80.101.114.108.32;
print 72.97.99.107.101.114.10;

#######  v-strings 3
print v74.117.115.116.32, v97.110.111.116.104.101.114.32,
      v80.101.114.108.32, v72.97.99.107.101.114.10;

#######  v-strings 4
print 74.117.115.116.32, 97.110.111.116.104.101.114.32,
      80.101.114.108.32, 72.97.99.107.101.114.10;

#######  v-strings 5
print v74.117.115.116.32.97.110.111.116.104.101.114.
      v32.80.101.114.108.32.72.97.99.107.101.114.10;

#######  v-strings 6
print 74.117.115.116.32.97.110.111.116.104.101.114.
      32.80.101.114.108.32.72.97.99.107.101.114.10;

#######  Symbolic references.
map{${+chr}=chr}map{$_=>$_^ord$"}$=+$]..3*$=/2;        
print "$J$u$s$t $a$n$o$t$h$e$r $P$e$r$l $H$a$c$k$e$r\n";

#######  $; fun
$;                                   # A lone dollar?
=$";                                 # Pod?
$;                                   # The return of the lone dollar?
{Just=>another=>Perl=>Hacker=>}      # Bare block?
=$/;                                 # More pod?
print%;                              # No right operand for %?

#######  @; fun
@;=split//=>"Joel, Preach sartre knuth\n";$;=chr 65;%;=map{$;++=>$_}
0,22,13,16,5,14,21,1,23,11,2,7,12,6,8,15,3,19,24,14,10,20,18,17,4,25
;print@;[@;{A..Z}];
EXPECT: $JaPh_c

#######  %; fun
$;=$";$;{Just=>another=>Perl=>Hacker=>}=$/;print%;

####### &func;
$_ = "\112\165\163\1648\141\156\157\164\150\145\1628\120\145"
   . "\162\1548\110\141\143\153\145\162\0128\177"  and &japh;
sub japh {print "@_" and return if pop; split /\d/ and &japh}
SKIP: As of 5.12.0, split() in void context no longer populates @_.

####### magic goto.
sub _ {$_ = shift and y/b-yB-Y/a-yB-Y/                xor      !@ _?
       exit print                                                  :
            print and push @_ => shift and goto &{(caller (0)) [3]}}
            split // => "KsvQtbuf fbsodpmu\ni flsI "  xor       & _
SKIP: As of 5.12.0, split() in void context no longer populates @_.

####### $: fun 1
:$:=~s:$":Just$&another$&:;$:=~s:
:Perl$"Hacker$&:;chop$:;print$:#:

####### $: fun 2
 :;$:=~s:
-:;another Perl Hacker
 :;chop
$:;$:=~y
 :;::d;print+Just.
$:;

####### $: fun 3
 :;$:=~s:
-:;another Perl Hacker
 :;chop
$:;$:=~y:;::d;print+Just.$:

####### $!
s[$,][join$,,(split$,,($!=85))[(q[0006143730380126152532042307].
q[41342211132019313505])=~m[..]g]]e and y[yIbp][HJkP] and print;
SKIP: Platform dependent.

####### die 1
eval {die ["Just another Perl Hacker"]}; print ${$@}[$#{@${@}}]

####### die 2
eval {die ["Just another Perl Hacker\n"]}; print ${$@}[$#{@${@}}]

####### die 3
eval {die ["Just another Perl Hacker"]}; print ${${@}}[$#{@{${@}}}]

####### die 4
eval {die ["Just another Perl Hacker\n"]}; print ${${@}}[$#{@{${@}}}]

####### die 5
eval {die [[qq [Just another Perl Hacker]]]};; print
${${${@}}[$#{@{${@}}}]}[$#{${@{${@}}}[$#{@{${@}}}]}]
SKIP: Abuses a fixed bug; what is in $#{...} must be an arrayref, not an array

####### Closure returning itself.
$_ = "\nrekcaH lreP rehtona tsuJ"; my $chop; $chop = sub {print chop; $chop};
$chop -> () -> () -> () -> () -> () -> () -> () -> () -> () -> () -> () -> ()
-> () -> () -> () -> () -> () -> () -> () -> () -> () -> () -> () -> () -> ()

####### Special blocks 1
BEGIN {print "Just "   }
CHECK {print "another "}
INIT  {print "Perl "   }
END   {print "Hacker\n"}

####### Special blocks 2
END   {print "Hacker\n"}
INIT  {print "Perl "   }
CHECK {print "another "}
BEGIN {print "Just "   }

####### Recursive regex.
   my $qr =  qr/^.+?(;).+?\1|;Just another Perl Hacker;|;.+$/;
      $qr =~  s/$qr//g;
print $qr, "\n";

####### use lib 'coderef'
use   lib sub {($\) = split /\./ => pop; print $"};
eval "use Just" || eval "use another" || eval "use Perl" || eval "use Hacker";
EXPECT
 Just another Perl Hacker
