package autodie::test::badname;
use parent qw(autodie);

sub exception_class {
    return 'autodie::test::badname::$@#%';  # Doesn't exist!
}

1;
