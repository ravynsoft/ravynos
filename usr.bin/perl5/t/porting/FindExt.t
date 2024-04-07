#!../miniperl -w

BEGIN {
    @INC = qw(../win32 ../lib);
    require './test.pl';
    skip_all('FindExt not portable')
	if $^O eq 'VMS';
}
use strict;
use Config;

# Test that Win32/FindExt.pm is consistent with Configure in determining the
# types of extensions.

if ($^O eq "MSWin32" && !defined $ENV{PERL_STATIC_EXT}) {
    skip_all "PERL_STATIC_EXT must be set to the list of static extensions";
}

if ( $Config{usecrosscompile} ) {
  skip_all( "Not all files are available during cross-compilation" );
}

require FindExt;

FindExt::apply_config(\%Config);
FindExt::scan_ext("../$_")
    foreach qw(cpan dist ext);
FindExt::set_static_extensions(split ' ', $^O eq 'MSWin32'
                               ? $ENV{PERL_STATIC_EXT} : $Config{static_ext});

sub compare {
    my ($desc, $want, @have) = @_;
    $want = [sort split ' ', $want]
        unless ref $want eq 'ARRAY';
    local $::Level = $::Level + 1;
    is(scalar @have, scalar @$want, "We find the same number of $desc");
    is("@have", "@$want", "We find the same list of $desc");
}

unless (join (' ', sort split ' ', $Config{extensions})
        eq join(' ', FindExt::extensions())) {
    # There are various things that break our assumptions.
    # If Encode is a static extension, then Configure has special case logic
    # to add Encode/* as static extensions
    # -Uusedl causes Encode to be a static extension, and drops building
    # XS::APItest and XS::Typemap
    # Any use of -Dnoextensions to choose not to build a extension

    plan(tests => 2);
    note("configured extension list doesn't match, so only minimal testing is possible");
    compare('known_extensions', $Config{known_extensions},
            FindExt::known_extensions());
} else {
    # dynamic linking, and all possible extensions for this system were built,
    # so can test everything.
    plan(tests => 12);

    compare('known_extensions', $Config{known_extensions},
            FindExt::known_extensions());

    # Config.pm and FindExt.pm make different choices about what should be built
    my @config_built;
    my @found_built;

    foreach my $type (qw(static dynamic nonxs)) {
        my @this_found = eval "FindExt::${type}_ext()";
        push @found_built, @this_found;
	push @config_built, split ' ', $Config{"${type}_ext"};
        compare("${type}_ext", $Config{"${type}_ext"}, @this_found);
    }

    compare('"config" dynamic + static + nonxs', $Config{extensions},
            sort @config_built);
    compare('"found" dynamic + static + nonxs', [FindExt::extensions()],
            sort @found_built);
}

# ex: set ts=8 sts=4 sw=4 et:
