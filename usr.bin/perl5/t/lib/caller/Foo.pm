# for use by caller.t for GH #15109

package Foo;

sub import {
    use warnings; # restore default warnings
    () = caller(1); # this used to cause valgrind errors
}
1;
