package autodie::test::missing;
use parent qw(autodie);

sub exception_class {
    return "autodie::test::missing::exception";  # Doesn't exist!
}

1;
