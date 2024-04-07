package Caller_helper;

our $line;

sub foo {
    use autodie;

    $line = __LINE__; open(my $fh, '<', "no_such_file_here");

    return;
}

1;
