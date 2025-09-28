use Test::More tests => 5;
use XS::APItest;


sub fribbler { 2*shift }
{
    BEGIN { lexical_import fribbler => sub { 3*shift } }
    is fribbler(15), 45, 'lexical subs via pad_add_name';
}
is fribbler(15), 30, 'XS-allocated lexical subs falling out of scope';

{
    BEGIN { lexical_import fribbler => sub { 3*shift } }
    is fribbler(15), 45, 'lexical subs via pad_add_name';
    no warnings;
    use feature 'lexical_subs';
    our sub fribbler;
    is fribbler(15), 30, 'our sub overrides XS-registered lexical sub';
}

# With ‘use’ rather than explicit BEGIN:
package Lexical::Exporter {
    sub import { shift; ::lexical_import @_; return }
}
BEGIN { ++$INC{"Lexical/Exporter.pm"} }

{
    use Lexical::Exporter fribbler => sub { shift() . "foo" };
    is fribbler("bar"), "barfoo";
}
