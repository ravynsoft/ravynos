package Module::Load::Conditional;

use strict;

use Module::Load qw/load autoload_remote/;
use Params::Check                       qw[check];
use Locale::Maketext::Simple Style  => 'gettext';

use Carp        ();
use File::Spec  ();
use FileHandle  ();
use version;

use Module::Metadata ();

use constant ON_VMS   => $^O eq 'VMS';
use constant ON_WIN32 => $^O eq 'MSWin32' ? 1 : 0;
use constant QUOTE    => do { ON_WIN32 ? q["] : q['] };

BEGIN {
    use vars        qw[ $VERSION @ISA $VERBOSE $CACHE @EXPORT_OK $DEPRECATED
                        $FIND_VERSION $ERROR $CHECK_INC_HASH $FORCE_SAFE_INC ];
    use Exporter;
    @ISA            = qw[Exporter];
    $VERSION        = '0.74';
    $VERBOSE        = 0;
    $DEPRECATED     = 0;
    $FIND_VERSION   = 1;
    $CHECK_INC_HASH = 0;
    $FORCE_SAFE_INC = 0;
    @EXPORT_OK      = qw[check_install can_load requires];
}

=pod

=head1 NAME

Module::Load::Conditional - Looking up module information / loading at runtime

=head1 SYNOPSIS

    use Module::Load::Conditional qw[can_load check_install requires];


    my $use_list = {
            CPANPLUS        => 0.05,
            LWP             => 5.60,
            'Test::More'    => undef,
    };

    print can_load( modules => $use_list )
            ? 'all modules loaded successfully'
            : 'failed to load required modules';


    my $rv = check_install( module => 'LWP', version => 5.60 )
                or print 'LWP is not installed!';

    print 'LWP up to date' if $rv->{uptodate};
    print "LWP version is $rv->{version}\n";
    print "LWP is installed as file $rv->{file}\n";


    print "LWP requires the following modules to be installed:\n";
    print join "\n", requires('LWP');

    ### allow M::L::C to peek in your %INC rather than just
    ### scanning @INC
    $Module::Load::Conditional::CHECK_INC_HASH = 1;

    ### reset the 'can_load' cache
    undef $Module::Load::Conditional::CACHE;

    ### don't have Module::Load::Conditional issue warnings --
    ### default is '1'
    $Module::Load::Conditional::VERBOSE = 0;

    ### The last error that happened during a call to 'can_load'
    my $err = $Module::Load::Conditional::ERROR;


=head1 DESCRIPTION

Module::Load::Conditional provides simple ways to query and possibly load any of
the modules you have installed on your system during runtime.

It is able to load multiple modules at once or none at all if one of
them was not able to load. It also takes care of any error checking
and so forth.

=head1 Methods

=head2 $href = check_install( module => NAME [, version => VERSION, verbose => BOOL ] );

C<check_install> allows you to verify if a certain module is installed
or not. You may call it with the following arguments:

=over 4

=item module

The name of the module you wish to verify -- this is a required key

=item version

The version this module needs to be -- this is optional

=item verbose

Whether or not to be verbose about what it is doing -- it will default
to $Module::Load::Conditional::VERBOSE

=back

It will return undef if it was not able to find where the module was
installed, or a hash reference with the following keys if it was able
to find the file:

=over 4

=item file

Full path to the file that contains the module

=item dir

Directory, or more exact the C<@INC> entry, where the module was
loaded from.

=item version

The version number of the installed module - this will be C<undef> if
the module had no (or unparsable) version number, or if the variable
C<$Module::Load::Conditional::FIND_VERSION> was set to true.
(See the C<GLOBAL VARIABLES> section below for details)

=item uptodate

A boolean value indicating whether or not the module was found to be
at least the version you specified. If you did not specify a version,
uptodate will always be true if the module was found.
If no parsable version was found in the module, uptodate will also be
true, since C<check_install> had no way to verify clearly.

See also C<$Module::Load::Conditional::DEPRECATED>, which affects
the outcome of this value.

=back

=cut

### this checks if a certain module is installed already ###
### if it returns true, the module in question is already installed
### or we found the file, but couldn't open it, OR there was no version
### to be found in the module
### it will return 0 if the version in the module is LOWER then the one
### we are looking for, or if we couldn't find the desired module to begin with
### if the installed version is higher or equal to the one we want, it will return
### a hashref with he module name and version in it.. so 'true' as well.
sub check_install {
    my %hash = @_;

    my $tmpl = {
            version => { default    => '0.0'    },
            module  => { required   => 1        },
            verbose => { default    => $VERBOSE },
    };

    my $args;
    unless( $args = check( $tmpl, \%hash, $VERBOSE ) ) {
        warn loc( q[A problem occurred checking arguments] ) if $VERBOSE;
        return;
    }

    my $file     = File::Spec->catfile( split /::/, $args->{module} ) . '.pm';
    my $file_inc = File::Spec::Unix->catfile(
                        split /::/, $args->{module}
                    ) . '.pm';

    ### where we store the return value ###
    my $href = {
            file        => undef,
            version     => undef,
            uptodate    => undef,
    };

    my $filename;

    ### check the inc hash if we're allowed to
    if( $CHECK_INC_HASH ) {
        $filename = $href->{'file'} =
            $INC{ $file_inc } if defined $INC{ $file_inc };

        ### find the version by inspecting the package
        if( defined $filename && $FIND_VERSION ) {
            no strict 'refs';
            $href->{version} = ${ "$args->{module}"."::VERSION" };
        }
    }

    ### we didn't find the filename yet by looking in %INC,
    ### so scan the dirs
    unless( $filename ) {

        local @INC = @INC[0..$#INC-1] if $FORCE_SAFE_INC && $INC[-1] eq '.';

        DIR: for my $dir ( @INC ) {

            my $fh;

            if ( ref $dir ) {
                ### @INC hook -- we invoke it and get the filehandle back
                ### this is actually documented behaviour as of 5.8 ;)

                my $existed_in_inc = $INC{$file_inc};

                if (UNIVERSAL::isa($dir, 'CODE')) {
                    ($fh) = $dir->($dir, $file);

                } elsif (UNIVERSAL::isa($dir, 'ARRAY')) {
                    ($fh) = $dir->[0]->($dir, $file, @{$dir}{1..$#{$dir}})

                } elsif (UNIVERSAL::can($dir, 'INC')) {
                    ($fh) = $dir->INC($file);
                }

                if (!UNIVERSAL::isa($fh, 'GLOB')) {
                    warn loc(q[Cannot open file '%1': %2], $file, $!)
                            if $args->{verbose};
                    next;
                }

                $filename = $INC{$file_inc} || $file;

                delete $INC{$file_inc} if not $existed_in_inc;

            } else {
                $filename = File::Spec->catfile($dir, $file);
                next unless -e $filename;

                $fh = FileHandle->new();
                if (!$fh->open($filename)) {
                    warn loc(q[Cannot open file '%1': %2], $file, $!)
                            if $args->{verbose};
                    next;
                }
            }

            ### store the directory we found the file in
            $href->{dir} = $dir;

            ### files need to be in unix format under vms,
            ### or they might be loaded twice
            $href->{file} = ON_VMS
                ? VMS::Filespec::unixify( $filename )
                : $filename;

            ### if we don't need the version, we're done
            last DIR unless $FIND_VERSION;

            ### otherwise, the user wants us to find the version from files

            {
              local $SIG{__WARN__} = sub {};
              my $ver = eval {
                my $mod_info = Module::Metadata->new_from_handle( $fh, $filename );
                $mod_info->version( $args->{module} );
              };

              if( defined $ver ) {
                  $href->{version} = $ver;

                  last DIR;
              }
            }
        }
    }

    ### if we couldn't find the file, return undef ###
    return unless defined $href->{file};

    ### only complain if we're expected to find a version higher than 0.0 anyway
    if( $FIND_VERSION and not defined $href->{version} ) {
        {   ### don't warn about the 'not numeric' stuff ###
            local $^W;

            ### if we got here, we didn't find the version
            warn loc(q[Could not check version on '%1'], $args->{module} )
                    if $args->{verbose} and $args->{version} > 0;
        }
        $href->{uptodate} = 1;

    } else {
        ### don't warn about the 'not numeric' stuff ###
        local $^W;

        ### use qv(), as it will deal with developer release number
        ### ie ones containing _ as well. This addresses bug report
        ### #29348: Version compare logic doesn't handle alphas?
        ###
        ### Update from JPeacock: apparently qv() and version->new
        ### are different things, and we *must* use version->new
        ### here, or things like #30056 might start happening

        ### We have to wrap this in an eval as version-0.82 raises
        ### exceptions and not warnings now *sigh*

        eval {

          $href->{uptodate} =
            version->new( $args->{version} ) <= version->new( $href->{version} )
                ? 1
                : 0;

        };
    }

    if ( $DEPRECATED and "$]" >= 5.011 ) {
        local @INC = @INC[0..$#INC-1] if $FORCE_SAFE_INC && $INC[-1] eq '.';
        require Module::CoreList;
        require Config;

        no warnings 'once';
        $href->{uptodate} = 0 if
           exists $Module::CoreList::version{ 0+$] }{ $args->{module} } and
           Module::CoreList::is_deprecated( $args->{module} ) and
           $Config::Config{privlibexp} eq $href->{dir}
           and $Config::Config{privlibexp} ne $Config::Config{sitelibexp};
    }

    return $href;
}

=head2 $bool = can_load( modules => { NAME => VERSION [,NAME => VERSION] }, [verbose => BOOL, nocache => BOOL, autoload => BOOL] )

C<can_load> will take a list of modules, optionally with version
numbers and determine if it is able to load them. If it can load *ALL*
of them, it will. If one or more are unloadable, none will be loaded.

This is particularly useful if you have More Than One Way (tm) to
solve a problem in a program, and only wish to continue down a path
if all modules could be loaded, and not load them if they couldn't.

This function uses the C<load> function or the C<autoload_remote> function
from Module::Load under the hood.

C<can_load> takes the following arguments:

=over 4

=item modules

This is a hashref of module/version pairs. The version indicates the
minimum version to load. If no version is provided, any version is
assumed to be good enough.

=item verbose

This controls whether warnings should be printed if a module failed
to load.
The default is to use the value of $Module::Load::Conditional::VERBOSE.

=item nocache

C<can_load> keeps its results in a cache, so it will not load the
same module twice, nor will it attempt to load a module that has
already failed to load before. By default, C<can_load> will check its
cache, but you can override that by setting C<nocache> to true.

=item autoload

This controls whether imports the functions of a loaded modules to the caller package. The default is no importing any functions.

See the C<autoload> function and the C<autoload_remote> function from L<Module::Load> for details.

=cut

sub can_load {
    my %hash = @_;

    my $tmpl = {
        modules     => { default => {}, strict_type => 1 },
        verbose     => { default => $VERBOSE },
        nocache     => { default => 0 },
        autoload    => { default => 0 },
    };

    my $args;

    unless( $args = check( $tmpl, \%hash, $VERBOSE ) ) {
        $ERROR = loc(q[Problem validating arguments!]);
        warn $ERROR if $VERBOSE;
        return;
    }

    ### layout of $CACHE:
    ### $CACHE = {
    ###     $ module => {
    ###             usable  => BOOL,
    ###             version => \d,
    ###             file    => /path/to/file,
    ###     },
    ### };

    $CACHE ||= {}; # in case it was undef'd

    my $error;
    BLOCK: {
        my $href = $args->{modules};

        my @load;
        for my $mod ( keys %$href ) {

            next if $CACHE->{$mod}->{usable} && !$args->{nocache};

            ### else, check if the hash key is defined already,
            ### meaning $mod => 0,
            ### indicating UNSUCCESSFUL prior attempt of usage

            ### use qv(), as it will deal with developer release number
            ### ie ones containing _ as well. This addresses bug report
            ### #29348: Version compare logic doesn't handle alphas?
            ###
            ### Update from JPeacock: apparently qv() and version->new
            ### are different things, and we *must* use version->new
            ### here, or things like #30056 might start happening
            if (    !$args->{nocache}
                    && defined $CACHE->{$mod}->{usable}
                    && (version->new( $CACHE->{$mod}->{version}||0 )
                        >= version->new( $href->{$mod} ) )
            ) {
                $error = loc( q[Already tried to use '%1', which was unsuccessful], $mod);
                last BLOCK;
            }

            my $mod_data = check_install(
                                    module  => $mod,
                                    version => $href->{$mod}
                                );

            if( !$mod_data or !defined $mod_data->{file} ) {
                $error = loc(q[Could not find or check module '%1'], $mod);
                $CACHE->{$mod}->{usable} = 0;
                last BLOCK;
            }

            map {
                $CACHE->{$mod}->{$_} = $mod_data->{$_}
            } qw[version file uptodate];

            push @load, $mod;
        }

        for my $mod ( @load ) {

            if ( $CACHE->{$mod}->{uptodate} ) {

                local @INC = @INC[0..$#INC-1] if $FORCE_SAFE_INC && $INC[-1] eq '.';

                if ( $args->{autoload} ) {
                    my $who = (caller())[0];
                    eval { autoload_remote $who, $mod };
                } else {
                    eval { load $mod };
                }

                ### in case anything goes wrong, log the error, the fact
                ### we tried to use this module and return 0;
                if( $@ ) {
                    $error = $@;
                    $CACHE->{$mod}->{usable} = 0;
                    last BLOCK;
                } else {
                    $CACHE->{$mod}->{usable} = 1;
                }

            ### module not found in @INC, store the result in
            ### $CACHE and return 0
            } else {

                $error = loc(q[Module '%1' is not uptodate!], $mod);
                $CACHE->{$mod}->{usable} = 0;
                last BLOCK;
            }
        }

    } # BLOCK

    if( defined $error ) {
        $ERROR = $error;
        Carp::carp( loc(q|%1 [THIS MAY BE A PROBLEM!]|,$error) ) if $args->{verbose};
        return;
    } else {
        return 1;
    }
}

=back

=head2 @list = requires( MODULE );

C<requires> can tell you what other modules a particular module
requires. This is particularly useful when you're intending to write
a module for public release and are listing its prerequisites.

C<requires> takes but one argument: the name of a module.
It will then first check if it can actually load this module, and
return undef if it can't.
Otherwise, it will return a list of modules and pragmas that would
have been loaded on the module's behalf.

Note: The list C<require> returns has originated from your current
perl and your current install.

=cut

sub requires {
    my $who = shift;

    unless( check_install( module => $who ) ) {
        warn loc(q[You do not have module '%1' installed], $who) if $VERBOSE;
        return undef;
    }

    local @INC = @INC[0..$#INC-1] if $FORCE_SAFE_INC && $INC[-1] eq '.';

    my $lib = join " ", map { qq["-I$_"] } @INC;
    my $oneliner = 'print(join(qq[\n],map{qq[BONG=$_]}keys(%INC)),qq[\n])';
    my $cmd = join '', qq["$^X" $lib -M$who -e], QUOTE, $oneliner, QUOTE;

    return  sort
                grep { !/^$who$/  }
                map  { chomp; s|/|::|g; $_ }
                grep { s|\.pm$||i; }
                map  { s!^BONG\=!!; $_ }
                grep { m!^BONG\=! }
            `$cmd`;
}

1;

__END__

=head1 Global Variables

The behaviour of Module::Load::Conditional can be altered by changing the
following global variables:

=head2 $Module::Load::Conditional::VERBOSE

This controls whether Module::Load::Conditional will issue warnings and
explanations as to why certain things may have failed. If you set it
to 0, Module::Load::Conditional will not output any warnings.
The default is 0;

=head2 $Module::Load::Conditional::FIND_VERSION

This controls whether Module::Load::Conditional will try to parse
(and eval) the version from the module you're trying to load.

If you don't wish to do this, set this variable to C<false>. Understand
then that version comparisons are not possible, and Module::Load::Conditional
can not tell you what module version you have installed.
This may be desirable from a security or performance point of view.
Note that C<$FIND_VERSION> code runs safely under C<taint mode>.

The default is 1;

=head2 $Module::Load::Conditional::CHECK_INC_HASH

This controls whether C<Module::Load::Conditional> checks your
C<%INC> hash to see if a module is available. By default, only
C<@INC> is scanned to see if a module is physically on your
filesystem, or available via an C<@INC-hook>. Setting this variable
to C<true> will trust any entries in C<%INC> and return them for
you.

The default is 0;

=head2 $Module::Load::Conditional::FORCE_SAFE_INC

This controls whether C<Module::Load::Conditional> sanitises C<@INC>
by removing "C<.>". The current default setting is C<0>, but this
may change in a future release.

=head2 $Module::Load::Conditional::CACHE

This holds the cache of the C<can_load> function. If you explicitly
want to remove the current cache, you can set this variable to
C<undef>

=head2 $Module::Load::Conditional::ERROR

This holds a string of the last error that happened during a call to
C<can_load>. It is useful to inspect this when C<can_load> returns
C<undef>.

=head2 $Module::Load::Conditional::DEPRECATED

This controls whether C<Module::Load::Conditional> checks if
a dual-life core module has been deprecated. If this is set to
true C<check_install> will return false to C<uptodate>, if
a dual-life module is found to be loaded from C<$Config{privlibexp}>

The default is 0;

=head1 See Also

C<Module::Load>

=head1 BUG REPORTS

Please report bugs or other issues to E<lt>bug-module-load-conditional@rt.cpan.orgE<gt>.

=head1 AUTHOR

This module by Jos Boumans E<lt>kane@cpan.orgE<gt>.

=head1 COPYRIGHT

This library is free software; you may redistribute and/or modify it
under the same terms as Perl itself.

=cut
