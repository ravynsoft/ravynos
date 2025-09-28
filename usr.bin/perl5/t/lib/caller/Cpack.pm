# for use by caller.t for GH #15109 and other tests
package Cpack;


my $i = 0;

while (my ($package, $file, $line) = caller($i++)) {
    push @Cpack::callers, "$file:$line";
}

1;
