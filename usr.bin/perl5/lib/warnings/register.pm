package warnings::register 1.05;

require warnings;

# left here as cruft in case other users were using this undocumented routine
# -- rjbs, 2010-09-08
sub mkMask
{
    my ($bit) = @_;
    my $mask = "";

    vec($mask, $bit, 1) = 1;
    return $mask;
}

sub import
{
    shift;
    my @categories = @_;

    my $package = caller;
    warnings::register_categories($package);

    warnings::register_categories($package . "::$_") for @categories;
}
1;
__END__

=pod

=head1 NAME

warnings::register - warnings import function

=head1 SYNOPSIS

    use warnings::register;

=head1 DESCRIPTION

Creates a warnings category with the same name as the current package.

See L<warnings> for more information on this module's usage.

=cut
