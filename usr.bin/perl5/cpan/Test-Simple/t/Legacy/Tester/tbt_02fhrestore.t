#!/usr/bin/perl

use Test::Builder::Tester tests => 4;
use Test::More;
use Symbol;

# create temporary file handles that still point indirectly
# to the right place

my $orig_o = gensym; 
my $orig_t = gensym;
my $orig_f = gensym; 

tie *$orig_o, "My::Passthru", \*STDOUT;
tie *$orig_t, "My::Passthru", \*STDERR;
tie *$orig_f, "My::Passthru", \*STDERR;

# redirect the file handles to somewhere else for a mo

use Test::Builder;
my $t = Test::Builder->new();

$t->output($orig_o);
$t->failure_output($orig_f);
$t->todo_output($orig_t);

# run a test

test_out("ok 1 - tested");
ok(1,"tested");
test_test("standard test okay");

# now check that they were restored okay

ok($orig_o == $t->output(), "output file reconnected");
ok($orig_t == $t->todo_output(), "todo output file reconnected");
ok($orig_f == $t->failure_output(), "failure output file reconnected");

#####################################################################

package My::Passthru;

sub PRINT  {
    my $self = shift;
    my $handle = $self->[0];
    print $handle @_;
}

sub TIEHANDLE {
    my $class = shift;
    my $self = [shift()];
    return bless $self, $class;
}

sub READ {}
sub READLINE {}
sub GETC {}
sub FILENO {}
