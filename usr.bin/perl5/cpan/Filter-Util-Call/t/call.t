use Config;

BEGIN {
    if ($ENV{PERL_CORE}) {
        if ($Config{'extensions'} !~ m{\bFilter/Util/Call\b}) {
            print "1..0 # Skip: Filter::Util::Call was not built\n";
            exit 0;
        }
        require Cwd;
        unshift @INC, Cwd::cwd();
    }
}

use strict;
use warnings;

use FindBin;
use lib "$FindBin::Bin"; # required to load filter-util.pl

require 'filter-util.pl';

use vars qw($Inc $Perl);

print "1..34\n";

$Perl = "$Perl -w";

use Cwd ;
my $here = getcwd ;


my $filename = "call$$.tst" ;
my $filename2 = "call2$$.tst" ;
my $filenamebin = "call$$.bin" ;
my $module   = "MyTest" ;
my $module2  = "MyTest2" ;
my $module3  = "MyTest3" ;
my $module4  = "MyTest4" ;
my $module5  = "MyTest5" ;
my $module6  = "MyTest6" ;
my $nested   = "nested" ;
my $block   = "block" ;
my $redir   = $^O eq 'MacOS' ? "" : "2>&1";

# Test error cases
##################

# no filter function in module
###############################

writeFile("${module}.pm", <<EOM) ;
package ${module} ;

use Filter::Util::Call ;

sub import { filter_add(bless []) }

1 ;
EOM

my $a = `$Perl "-I." $Inc -e "use ${module} ;"  $redir` ;
ok(1, (($? >>8) != 0 or (($^O eq 'MSWin32' || $^O eq 'MacOS' || $^O eq 'NetWare' || $^O eq 'mpeix') && $? != 0))) ;
ok(2, $a =~ /^Can't locate object method "filter" via package "MyTest"/m) ;

# no reference parameter in filter_add
######################################

writeFile("${module}.pm", <<EOM) ;
package ${module} ;

use Filter::Util::Call ;

sub import { filter_add() }

1 ;
EOM

$a = `$Perl "-I." $Inc -e "use ${module} ;"  $redir` ;
#warn "# $a\n";
ok(3, (($? >>8) != 0
       or (($^O eq 'MSWin32' || $^O eq 'MacOS' || $^O eq 'NetWare' || $^O eq 'mpeix')
           && $? != 0))) ;
#ok(4, $a =~ /^usage: filter_add\(ref\) at ${module}.pm/) ;
my $errmsg = $Config{usecperl}
  ? qr/^Not enough arguments for subroutine entry Filter::Util::Call::filter_add at ${module}\.pm line/m
  : qr/^Not enough arguments for Filter::Util::Call::filter_add at ${module}\.pm line/m;
$a =~ s/^(.*?\n).*$/$1/s; # only the first line
if ($] < 5.007) {
    if ($a =~ $errmsg) {
        ok(4, 1);
    } else {
        ok(4, 1, "TODO");
    }
} else {
    ok(4, $a =~ $errmsg, 'usage error')
       or diag("The error was: ", $a);
}

# non-error cases
#################


# a simple filter, using a closure
#################

writeFile("${module}.pm", <<EOM, <<'EOM') ;
package ${module} ;

EOM
use Filter::Util::Call ;
sub import {
    filter_add(
  	sub {

    	    my ($status) ;

    	    if (($status = filter_read()) > 0) {
        	s/ABC/DEF/g
    	    }
    	    $status ;
  	} ) ;
}

1 ;
EOM

writeFile($filename, <<EOM, <<'EOM') ;

use $module ;
EOM

use Cwd ;
my $here = getcwd ;
print "I am $here\n" ;
print "some letters ABC\n" ;
my $y = "ABCDEF" ;
print <<EOF ;
Alphabetti Spagetti ($y)
EOF

EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(5, ($? >>8) == 0) or warn $a;
ok(6, $a eq <<EOM) ;
I am $here
some letters DEF
Alphabetti Spagetti (DEFDEF)
EOM

# a simple filter, not using a closure
#################

writeFile("${module}.pm", <<EOM, <<'EOM') ;
package ${module} ;

EOM
use Filter::Util::Call ;
sub import { filter_add(bless []) }

sub filter
{
    my ($self) = @_ ;
    my ($status) ;

    if (($status = filter_read()) > 0) {
        s/ABC/DEF/g
    }
    $status ;
}


1 ;
EOM

writeFile($filename, <<EOM, <<'EOM') ;

use $module ;
EOM

use Cwd ;
my $here = getcwd ;
print "I am $here\n" ;
print "some letters ABC\n" ;
my $y = "ABCDEF" ;
print <<EOF ;
Alphabetti Spagetti ($y)
EOF

EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(7, ($? >>8) == 0) or warn $a;
ok(8, $a eq <<EOM) ;
I am $here
some letters DEF
Alphabetti Spagetti (DEFDEF)
EOM


# nested filters
################


writeFile("${module2}.pm", <<EOM, <<'EOM') ;
package ${module2} ;
use Filter::Util::Call ;

EOM
sub import { filter_add(bless []) }

sub filter
{
    my ($self) = @_ ;
    my ($status) ;

    if (($status = filter_read()) > 0) {
        s/XYZ/PQR/g
    }
    $status ;
}

1 ;
EOM

writeFile("${module3}.pm", <<EOM, <<'EOM') ;
package ${module3} ;
use Filter::Util::Call ;

EOM
sub import { filter_add(

    sub
    {
        my ($status) ;

        if (($status = filter_read()) > 0) {
            s/Fred/Joe/g
        }
        $status ;
    } ) ;
}

1 ;
EOM

writeFile("${module4}.pm", <<EOM) ;
package ${module4} ;

use $module5 ;

print "I'm feeling used!\n" ;
print "Fred Joe ABC DEF PQR XYZ\n" ;
print "See you Today\n" ;
1;
EOM

writeFile("${module5}.pm", <<EOM, <<'EOM') ;
package ${module5} ;
use Filter::Util::Call ;

EOM
sub import { filter_add(bless []) }

sub filter
{
    my ($self) = @_ ;
    my ($status) ;

    if (($status = filter_read()) > 0) {
        s/Today/Tomorrow/g
    }
    $status ;
}

1 ;
EOM

writeFile($filename, <<EOM, <<'EOM') ;

# two filters for this file
use $module ;
use $module2 ;
require "$nested" ;
use $module4 ;
EOM

print "some letters ABCXYZ\n" ;
my $y = "ABCDEFXYZ" ;
print <<EOF ;
Fred likes Alphabetti Spagetti ($y)
EOF

EOM

writeFile($nested, <<EOM, <<'EOM') ;
use $module3 ;
EOM

print "This is another file XYZ\n" ;
print <<EOF ;
Where is Fred?
EOF

EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(9, ($? >>8) == 0) or warn $a;
ok(10, $a eq <<EOM) ;
I'm feeling used!
Fred Joe ABC DEF PQR XYZ
See you Tomorrow
This is another file XYZ
Where is Joe?
some letters DEFPQR
Fred likes Alphabetti Spagetti (DEFDEFPQR)
EOM

# using the module context (with a closure)
###########################################


writeFile("${module2}.pm", <<EOM, <<'EOM') ;
package ${module2} ;
use Filter::Util::Call ;

EOM
sub import
{
    my ($type) = shift ;
    my (@strings) = @_ ;


    filter_add (

	sub
	{
    	    my ($status) ;
    	    my ($pattern) ;

    	    if (($status = filter_read()) > 0) {
                foreach $pattern (@strings)
          	    { s/$pattern/PQR/g }
    	    }

    	    $status ;
	}
	)

}
1 ;
EOM


writeFile($filename, <<EOM, <<'EOM') ;

use $module2 qw( XYZ KLM) ;
use $module2 qw( ABC NMO) ;
EOM

print "some letters ABCXYZ KLM NMO\n" ;
my $y = "ABCDEFXYZKLMNMO" ;
print <<EOF ;
Alphabetti Spagetti ($y)
EOF

EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(11, ($? >>8) == 0) or warn $a;
ok(12, $a eq <<EOM) ;
some letters PQRPQR PQR PQR
Alphabetti Spagetti (PQRDEFPQRPQRPQR)
EOM



# using the module context (without a closure)
##############################################


writeFile("${module2}.pm", <<EOM, <<'EOM') ;
package ${module2} ;
use Filter::Util::Call ;

EOM
sub import
{
    my ($type) = shift ;
    my (@strings) = @_ ;


    filter_add (bless [@strings])
}

sub filter
{
    my ($self) = @_ ;
    my ($status) ;
    my ($pattern) ;

    if (($status = filter_read()) > 0) {
	foreach $pattern (@$self)
          { s/$pattern/PQR/g }
    }

    $status ;
}

1 ;
EOM


writeFile($filename, <<EOM, <<'EOM') ;

use $module2 qw( XYZ KLM) ;
use $module2 qw( ABC NMO) ;
EOM

print "some letters ABCXYZ KLM NMO\n" ;
my $y = "ABCDEFXYZKLMNMO" ;
print <<EOF ;
Alphabetti Spagetti ($y)
EOF

EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(13, ($? >>8) == 0) or warn $a;
ok(14, $a eq <<EOM) ;
some letters PQRPQR PQR PQR
Alphabetti Spagetti (PQRDEFPQRPQRPQR)
EOM

# multi line test
#################


writeFile("${module2}.pm", <<EOM, <<'EOM') ;
package ${module2} ;
use Filter::Util::Call ;

EOM
sub import
{
    my ($type) = shift ;
    my (@strings) = @_ ;


    filter_add(bless [])
}

sub filter
{
    my ($self) = @_ ;
    my ($status) ;

    # read first line
    if (($status = filter_read()) > 0) {
	chop ;
	s/\r$//;
	# and now the second line (it will append)
        $status = filter_read() ;
    }

    $status ;
}

1 ;
EOM


writeFile($filename, <<EOM, <<'EOM') ;

use $module2  ;
EOM
print "don't cut me 
in half\n" ;
print 
<<EOF ;
appen
ded
EO
F

EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(15, ($? >>8) == 0) or warn $a;
ok(16, $a eq <<EOM) ;
don't cut me in half
appended
EOM
#print "# $a\n";

# Block test
#############

writeFile("${block}.pm", <<EOM, <<'EOM') ;
package ${block} ;
use Filter::Util::Call ;

EOM
sub import
{
    my ($type) = shift ;
    my (@strings) = @_ ;


    filter_add (bless [@strings] )
}

sub filter
{
    my ($self) = @_ ;
    my ($status) ;
    my ($pattern) ;

    filter_read(20)  ;
}

1 ;
EOM

my $string = <<'EOM' ;
print "hello mum\n" ;
my $x = 'me ' x 3 ;
print "Who wants it?\n$x\n" ;
EOM


writeFile($filename, <<EOM, $string ) ;
use $block ;
EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(17, ($? >>8) == 0) or warn $a;
ok(18, $a eq <<EOM) ;
hello mum
Who wants it?
me me me 
EOM

# use in the filter
####################

writeFile("${block}.pm", <<EOM, <<'EOM') ;
package ${block} ;
use Filter::Util::Call ;

EOM
use Cwd ;

sub import
{
    my ($type) = shift ;
    my (@strings) = @_ ;


    filter_add(bless [@strings] )
}

sub filter
{
    my ($self) = @_ ;
    my ($status) ;
    my ($here) = quotemeta getcwd ;

    if (($status = filter_read()) > 0) {
        s/DIR/$here/g
    }
    $status ;
}

1 ;
EOM

writeFile($filename, <<EOM, <<'EOM') ;
use $block ;
EOM
print "We are in DIR\n" ;
EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(19, ($? >>8) == 0) or warn $a;
ok(20, $a eq <<EOM) ;
We are in $here
EOM


# filter_del
#############

writeFile("${block}.pm", <<EOM, <<'EOM') ;
package ${block} ;
use Filter::Util::Call ;

EOM

sub import
{
    my ($type) = shift ;
    my ($count) = @_ ;


    filter_add(bless \$count )
}

sub filter
{
    my ($self) = @_ ;
    my ($status) ;

    s/HERE/THERE/g
        if ($status = filter_read()) > 0 ;

    -- $$self ;
    filter_del() if $$self <= 0 ;

    $status ;
}

1 ;
EOM

writeFile($filename, <<EOM, <<'EOM') ;
use $block (3) ;
EOM
print "
HERE I am
I am HERE
HERE today gone tomorrow\n" ;
EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(21, ($? >>8) == 0) or warn $a;
ok(22, $a eq <<EOM) ;

THERE I am
I am THERE
HERE today gone tomorrow
EOM


# filter_read_exact
####################

writeFile("${block}.pm", <<EOM, <<'EOM') ;
package ${block} ;
use Filter::Util::Call ;

EOM

sub import
{
    my ($type) = shift ;

    filter_add(bless [] )
}

sub filter
{
    my ($self) = @_ ;
    my ($status) ;

    if (($status = filter_read_exact(9)) > 0) {
        s/HERE/THERE/g
    }

    $status ;
}

1 ;
EOM

writeFile($filenamebin, <<EOM, <<'EOM') ;
use $block ;
EOM
print "
HERE I am
I'm HERE
HERE today gone tomorrow\n" ;
EOM

$a = `$Perl "-I." $Inc $filenamebin  $redir` ;
ok(23, ($? >>8) == 0) or warn $a;
ok(24, $a eq <<EOM) ;

HERE I am
I'm THERE
THERE today gone tomorrow
EOM

{

# Check __DATA__
####################

writeFile("${block}.pm", <<EOM, <<'EOM') ;
package ${block} ;
use Filter::Util::Call ;

EOM

sub import
{
    my ($type) = shift ;

    filter_add([])
}

sub filter
{
    my ($self) = @_ ;
    my ($status) ;

    if (($status = filter_read()) > 0) {
        s/HERE/THERE/g
    }

    $status ;
}

1 ;
EOM

writeFile($filename, <<EOM, <<'EOM') ;
use $block ;
EOM
print "HERE HERE\n";
my @a = <DATA>;
print @a;
__DATA__
HERE I am
I'm HERE
HERE today gone tomorrow
EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(25, ($? >>8) == 0) or warn $a;
ok(26, $a eq <<EOM) ;
THERE THERE
HERE I am
I'm HERE
HERE today gone tomorrow
EOM

}

{

# Check __END__
####################

writeFile("${block}.pm", <<EOM, <<'EOM') ;
package ${block} ;
use Filter::Util::Call ;

EOM

sub import
{
    my ($type) = shift ;

    filter_add(bless [] )
}

sub filter
{
    my ($self) = @_ ;
    my ($status) ;

    if (($status = filter_read()) > 0) {
        s/HERE/THERE/g
    }

    $status ;
}

1 ;
EOM

writeFile($filename, <<EOM, <<'EOM') ;
use $block ;
EOM
print "HERE HERE\n";
my @a = <DATA>;
print @a;
__END__
HERE I am
I'm HERE
HERE today gone tomorrow
EOM

$a = `$Perl "-I." $Inc $filename  $redir` ;
ok(27, ($? >>8) == 0) or warn $a;
ok(28, $a eq <<EOM) ;
THERE THERE
HERE I am
I'm HERE
HERE today gone tomorrow
EOM

}

{

# no without use
# see Message-ID: <2002110621.427.A15377@ttul.org>
####################

writeFile("${module6}.pm", <<EOM);
package ${module6} ;
#use Filter::Simple;
#FILTER {}
use Filter::Util::Call;
sub import { filter_add(sub{}) }
sub unimport { filter_del() }
1;
EOM

writeFile($filename2, <<EOM);
no ${module6} ;
print "ok";
EOM

my $str = $^O eq 'MacOS' ? "'ok'" : "q{ok}";
my $a = `$Perl "-I." $Inc -e "no ${module6}; print $str"`;
ok(29, ($? >>8) == 0) or warn $a;
chomp( $a ) if $^O eq 'VMS';
ok(30, $a eq 'ok');

$a = `$Perl "-I." $Inc $filename2`;
ok(31, ($? >>8) == 0) or warn $a;
chomp( $a ) if $^O eq 'VMS';
ok(32, $a eq 'ok');

}

# error: filter_read_exact: size parameter must be > 0
######################################

writeFile("${block}.pm", <<EOM, <<'EOM') ;
package ${block} ;
use Filter::Util::Call ;

EOM

sub import
{
    my ($type) = shift ;
    filter_add(bless [] )
}

sub filter
{
    my ($self) = @_ ;
    my ($status) ;
    if (($status = filter_read_exact(0)) > 0) {
        s/HERE/THERE/g
    }
    $status ;
}

1 ;
EOM

writeFile($filenamebin, <<EOM, <<'EOM') ;
use $block ;
EOM
print "
HERE I am
I'm HERE
HERE today gone tomorrow\n" ;
EOM

$a = `$Perl "-I." $Inc $filenamebin  $redir` ;
ok(33, ($? >>8) != 0) or warn $a;
ok(34, $a =~ /^filter_read_exact: size parameter must be > 0 at block.pm/) ;


END {
    1 while unlink $filename ;
    1 while unlink $filename2 ;
    1 while unlink $filenamebin ;
    1 while unlink "${module}.pm" ;
    1 while unlink "${module2}.pm" ;
    1 while unlink "${module3}.pm" ;
    1 while unlink "${module4}.pm" ;
    1 while unlink "${module5}.pm" ;
    1 while unlink "${module6}.pm" ;
    1 while unlink $nested ;
    1 while unlink "${block}.pm" ;
}


