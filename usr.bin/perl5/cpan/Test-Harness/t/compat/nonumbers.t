if ( $ENV{PERL_CORE} && !$ENV{HARNESS_ACTIVE} ) {
    print "1..0 # Skip: t/TEST needs numbers\n";
    exit;
}

print <<END;
1..6
ok
ok
ok
ok
ok
ok
END
