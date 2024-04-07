package Module::Load;

use strict;
use warnings;
use File::Spec ();

our $VERSION = '0.36';


sub import {
    my $who = _who();
    my $h; shift;

    {   no strict 'refs';

        @_ or (
            *{"${who}::load"} = \&load, # compat to prev version
            *{"${who}::autoload"} = \&autoload,
            return
        );

        map { $h->{$_} = () if defined $_ } @_;

        (exists $h->{none} or exists $h->{''})
            and shift, last;

        ((exists $h->{autoload} and shift,1) or (exists $h->{all} and shift))
            and *{"${who}::autoload"} = \&autoload;

        ((exists $h->{load} and shift,1) or exists $h->{all})
            and *{"${who}::load"} = \&load;

        ((exists $h->{load_remote} and shift,1) or exists $h->{all})
            and *{"${who}::load_remote"} = \&load_remote;

        ((exists $h->{autoload_remote} and shift,1) or exists $h->{all})
            and *{"${who}::autoload_remote"} = \&autoload_remote;

    }

}

sub load(*;@){
    goto &_load;
}

sub autoload(*;@){
    unshift @_, 'autoimport';
    goto &_load;
}

sub load_remote($$;@){
    my ($dst, $src, @exp) = @_;

    eval "package $dst;Module::Load::load('$src', qw/@exp/);";
    $@ && die "$@";
}

sub autoload_remote($$;@){
    my ($dst, $src, @exp) = @_;

    eval "package $dst;Module::Load::autoload('$src', qw/@exp/);";
    $@ && die "$@";
}

sub _load{
    my $autoimport = $_[0] eq 'autoimport' and shift;
    my $mod = shift or return;
    my $who = _who();

    if( _is_file( $mod ) ) {
        require $mod;
    } else {
        LOAD: {
            my $err;
            for my $flag ( qw[1 0] ) {
                my $file = _to_file( $mod, $flag);
                eval { require $file };
                $@ ? $err .= $@ : last LOAD;
            }
            die $err if $err;
        }
    }

    ### This addresses #41883: Module::Load cannot import
    ### non-Exporter module. ->import() routines weren't
    ### properly called when load() was used.

    {   no strict 'refs';
        my $import;

    ((@_ or $autoimport) and (
        $import = $mod->can('import')
        ) and (
        unshift(@_, $mod),
        goto &$import
        )
    );
    }

}

sub _to_file{
    local $_    = shift;
    my $pm      = shift || '';

    ## trailing blanks ignored by default. [rt #69886]
    my @parts = split /::|'/, $_, -1;
    ## make sure that we can't hop out of @INC
    shift @parts if @parts && !$parts[0];

    ### because of [perl #19213], see caveats ###
    my $file = $^O eq 'MSWin32'
                    ? join "/", @parts
                    : File::Spec->catfile( @parts );

    $file   .= '.pm' if $pm;

    ### on perl's before 5.10 (5.9.5@31746) if you require
    ### a file in VMS format, it's stored in %INC in VMS
    ### format. Therefor, better unixify it first
    ### Patch in reply to John Malmbergs patch (as mentioned
    ### above) on p5p Tue 21 Aug 2007 04:55:07
    $file = VMS::Filespec::unixify($file) if $^O eq 'VMS';

    return $file;
}

sub _who { (caller(1))[0] }

sub _is_file {
    local $_ = shift;
    return  /^\./               ? 1 :
            /[^\w:']/           ? 1 :
            undef
    #' silly bbedit..
}


1;

__END__

=pod

=head1 NAME

Module::Load - runtime require of both modules and files

=head1 SYNOPSIS

  use Module::Load;

  my $module = 'Data::Dumper';

  load Data::Dumper;     # loads that module, but not import any functions
                         # -> cannot use 'Dumper' function

  load 'Data::Dumper';   # ditto
  load $module           # tritto

  autoload Data::Dumper; # loads that module and imports the default functions
                         # -> can use 'Dumper' function

  my $script = 'some/script.pl'
  load $script;
  load 'some/script.pl';  # use quotes because of punctuations

  load thing;             # try 'thing' first, then 'thing.pm'

  load CGI, ':all';       # like 'use CGI qw[:standard]'

=head1 DESCRIPTION

C<Module::Load> eliminates the need to know whether you are trying
to require either a file or a module.

If you consult C<perldoc -f require> you will see that C<require> will
behave differently when given a bareword or a string.

In the case of a string, C<require> assumes you are wanting to load a
file. But in the case of a bareword, it assumes you mean a module.

This gives nasty overhead when you are trying to dynamically require
modules at runtime, since you will need to change the module notation
(C<Acme::Comment>) to a file notation fitting the particular platform
you are on.

C<Module::Load> eliminates the need for this overhead and will
just DWYM.

=head2 Difference between C<load> and C<autoload>

C<Module::Load> imports the two functions - C<load> and C<autoload>

C<autoload> imports the default functions automatically,
but C<load> do not import any functions.

C<autoload> is usable under C<BEGIN{};>.

Both the functions can import the functions that are specified.

Following codes are same.

  load File::Spec::Functions, qw/splitpath/;

  autoload File::Spec::Functions, qw/splitpath/;

=head1 FUNCTIONS

=over 4

=item load

Loads a specified module.

See L</Rules> for detailed loading rule.

=item autoload

Loads a specified module and imports the default functions.

Except importing the functions, 'autoload' is same as 'load'.

=item load_remote

Loads a specified module to the specified package.

  use Module::Load 'load_remote';

  my $pkg = 'Other::Package';

  load_remote $pkg, 'Data::Dumper'; # load a module to 'Other::Package'
                                    # but do not import 'Dumper' function

A module for loading must be quoted.

Except specifing the package and quoting module name,
'load_remote' is same as 'load'.

=item autoload_remote

Loads a specified module and imports the default functions to the specified package.

  use Module::Load 'autoload_remote';

  my $pkg = 'Other::Package';

  autoload_remote $pkg, 'Data::Dumper'; # load a module to 'Other::Package'
                                        # and imports 'Dumper' function

A module for loading must be quoted.

Except specifing the package and quoting module name,
'autoload_remote' is same as 'load_remote'.

=back

=head1 Rules

All functions have the following rules to decide what it thinks
you want:

=over 4

=item *

If the argument has any characters in it other than those matching
C<\w>, C<:> or C<'>, it must be a file

=item *

If the argument matches only C<[\w:']>, it must be a module

=item *

If the argument matches only C<\w>, it could either be a module or a
file. We will try to find C<file.pm> first in C<@INC> and if that
fails, we will try to find C<file> in @INC.  If both fail, we die with
the respective error messages.

=back

=head1 IMPORTS THE FUNCTIONS

'load' and 'autoload' are imported by default, but 'load_remote' and
'autoload_remote' are not imported.

To use 'load_remote' or 'autoload_remote', specify at 'use'.

=over 4

=item "load","autoload","load_remote","autoload_remote"

Imports the selected functions.

  # imports 'load' and 'autoload' (default)
  use Module::Load;

  # imports 'autoload' only
  use Module::Load 'autoload';

  # imports 'autoload' and 'autoload_remote', but don't import 'load';
  use Module::Load qw/autoload autoload_remote/;

=item 'all'

Imports all the functions.

  use Module::Load 'all'; # imports load, autoload, load_remote, autoload_remote

=item '','none',undef

Not import any functions (C<load> and C<autoload> are not imported).

  use Module::Load '';

  use Module::Load 'none';

  use Module::Load undef;

=back

=head1 Caveats

Because of a bug in perl (#19213), at least in version 5.6.1, we have
to hardcode the path separator for a require on Win32 to be C</>, like
on Unix rather than the Win32 C<\>. Otherwise perl will not read its
own %INC accurately double load files if they are required again, or
in the worst case, core dump.

C<Module::Load> cannot do implicit imports, only explicit imports.
(in other words, you always have to specify explicitly what you wish
to import from a module, even if the functions are in that modules'
C<@EXPORT>)

=head1 SEE ALSO

L<Module::Runtime> provides functions for loading modules,
checking the validity of a module name,
converting a module name to partial C<.pm> path,
and related utility functions.

L<"require" in perlfunc|https://metacpan.org/pod/perlfunc#require>
and
L<"use" in perlfunc|https://metacpan.org/pod/perlfunc#use>.

L<Mojo::Loader> is a "class loader and plugin framework",
and is included in the
L<Mojolicious|https://metacpan.org/release/Mojolicious> distribution.

L<Module::Loader> is a module for finding and loading modules
in a given namespace, inspired by C<Mojo::Loader>.


=head1 ACKNOWLEDGEMENTS

Thanks to Jonas B. Nielsen for making explicit imports work.

=head1 BUG REPORTS

Please report bugs or other issues to E<lt>bug-module-load@rt.cpan.orgE<gt>.

=head1 AUTHOR

This module by Jos Boumans E<lt>kane@cpan.orgE<gt>.

=head1 COPYRIGHT

This library is free software; you may redistribute and/or modify it
under the same terms as Perl itself.

=cut
