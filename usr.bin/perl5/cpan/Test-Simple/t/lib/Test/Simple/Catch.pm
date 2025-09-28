# For testing Test::Simple;
package Test::Simple::Catch;

use strict;

use Symbol;
use TieOut;
my( $out_fh, $err_fh ) = ( gensym, gensym );
my $out = tie *$out_fh, 'TieOut';
my $err = tie *$err_fh, 'TieOut';

use Test::Builder;
require Test::Builder::Formatter;
my $t = Test::Builder->new;
$t->{Stack}->top->format(Test::Builder::Formatter->new);
$t->output($out_fh);
$t->failure_output($err_fh);
$t->todo_output($err_fh);

sub caught { return( $out, $err ) }

1;
