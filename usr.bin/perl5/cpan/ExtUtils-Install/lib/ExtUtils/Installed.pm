use strict;
package ExtUtils::Installed;

#use warnings; # XXX requires 5.6
use Carp qw();
use ExtUtils::Packlist;
use ExtUtils::MakeMaker;
use Config;
use File::Find;
use File::Basename;
use File::Spec;

my $Is_VMS = $^O eq 'VMS';
my $DOSISH = ($^O =~ /^(MSWin\d\d|os2|dos|mint)$/);

require VMS::Filespec if $Is_VMS;

our $VERSION = '2.22';
$VERSION = eval $VERSION;

sub _is_prefix {
    my ($self, $path, $prefix) = @_;
    return unless defined $prefix && defined $path;

    if( $Is_VMS ) {
        $prefix = VMS::Filespec::unixify($prefix);
        $path   = VMS::Filespec::unixify($path);
    }

    # Unix path normalization.
    $prefix = File::Spec->canonpath($prefix);

    return 1 if substr($path, 0, length($prefix)) eq $prefix;

    if ($DOSISH) {
        $path =~ s|\\|/|g;
        $prefix =~ s|\\|/|g;
        return 1 if $path =~ m{^\Q$prefix\E}i;
    }
    return(0);
}

sub _is_doc {
    my ($self, $path) = @_;

    my $man1dir = $self->{':private:'}{Config}{man1direxp};
    my $man3dir = $self->{':private:'}{Config}{man3direxp};
    return(($man1dir && $self->_is_prefix($path, $man1dir))
           ||
           ($man3dir && $self->_is_prefix($path, $man3dir))
           ? 1 : 0)
}

sub _is_type {
    my ($self, $path, $type) = @_;
    return 1 if $type eq "all";

    return($self->_is_doc($path)) if $type eq "doc";
    my $conf= $self->{':private:'}{Config};
    if ($type eq "prog") {
        return($self->_is_prefix($path, $conf->{prefix} || $conf->{prefixexp})
               && !($self->_is_doc($path)) ? 1 : 0);
    }
    return(0);
}

sub _is_under {
    my ($self, $path, @under) = @_;
    $under[0] = "" if (! @under);
    foreach my $dir (@under) {
        return(1) if ($self->_is_prefix($path, $dir));
    }

    return(0);
}

sub _fix_dirs {
    my ($self, @dirs)= @_;
    # File::Find does not know how to deal with VMS filepaths.
    if( $Is_VMS ) {
        $_ = VMS::Filespec::unixify($_)
            for @dirs;
    }

    if ($DOSISH) {
        s|\\|/|g for @dirs;
    }
    return wantarray ? @dirs : $dirs[0];
}

sub _make_entry {
    my ($self, $module, $packlist_file, $modfile)= @_;

    my $data= {
        module => $module,
        packlist => scalar(ExtUtils::Packlist->new($packlist_file)),
        packlist_file => $packlist_file,
    };

    if (!$modfile) {
        $data->{version} = $self->{':private:'}{Config}{version};
    } else {
        $data->{modfile} = $modfile;
        # Find the top-level module file in @INC
        $data->{version} = '';
        foreach my $dir (@{$self->{':private:'}{INC}}) {
            my $p = File::Spec->catfile($dir, $modfile);
            if (-r $p) {
                $module = _module_name($p, $module) if $Is_VMS;

                $data->{version} = MM->parse_version($p);
                $data->{version_from} = $p;
                $data->{packlist_valid} = exists $data->{packlist}{$p};
                last;
            }
        }
    }
    $self->{$module}= $data;
}

our $INSTALLED;
sub new {
    my ($class) = shift(@_);
    $class = ref($class) || $class;

    my %args = @_;

    return $INSTALLED if $INSTALLED and ($args{default_get} || $args{default});

    my $self = bless {}, $class;

    $INSTALLED= $self if $args{default_set} || $args{default};


    if ($args{config_override}) {
        eval {
            $self->{':private:'}{Config} = { %{$args{config_override}} };
        } or Carp::croak(
            "The 'config_override' parameter must be a hash reference."
        );
    }
    else {
        $self->{':private:'}{Config} = \%Config;
    }

    for my $tuple ([inc_override => INC => [ @INC ] ],
                   [ extra_libs => EXTRA => [] ])
    {
        my ($arg,$key,$val)=@$tuple;
        if ( $args{$arg} ) {
            eval {
                $self->{':private:'}{$key} = [ @{$args{$arg}} ];
            } or Carp::croak(
                "The '$arg' parameter must be an array reference."
            );
        }
        elsif ($val) {
            $self->{':private:'}{$key} = $val;
        }
    }
    {
        my %dupe;
        @{$self->{':private:'}{LIBDIRS}} =
            grep { $_ ne '.' || ! $args{skip_cwd} }
            grep { -e $_ && !$dupe{$_}++ }
            @{$self->{':private:'}{EXTRA}}, @{$self->{':private:'}{INC}};
    }

    my @dirs= $self->_fix_dirs(@{$self->{':private:'}{LIBDIRS}});

    # Read the core packlist
    my $archlib = $self->_fix_dirs($self->{':private:'}{Config}{archlibexp});
    $self->_make_entry("Perl",File::Spec->catfile($archlib, '.packlist'));

    my $root;
    # Read the module packlists
    my $sub = sub {
        # Only process module .packlists
        return if $_ ne ".packlist" || $File::Find::dir eq $archlib;

        # Hack of the leading bits of the paths & convert to a module name
        my $module = $File::Find::name;
        my $found = $module =~ s!^.*?/auto/(.*)/.packlist!$1!s
            or do {
            # warn "Woah! \$_=$_\n\$module=$module\n\$File::Find::dir=$File::Find::dir\n",
            #    join ("\n",@dirs);
            return;
        };

        my $modfile = "$module.pm";
        $module =~ s!/!::!g;

        return if $self->{$module}; #shadowing?
        $self->_make_entry($module,$File::Find::name,$modfile);
    };
    while (@dirs) {
        $root= shift @dirs;
        next if !-d $root;
        find($sub,$root);
    }

    return $self;
}

# VMS's non-case preserving file-system means the package name can't
# be reconstructed from the filename.
sub _module_name {
    my($file, $orig_module) = @_;

    my $module = '';
    if (open PACKFH, $file) {
        while (<PACKFH>) {
            if (/package\s+(\S+)\s*;/) {
                my $pack = $1;
                # Make a sanity check, that lower case $module
                # is identical to lowercase $pack before
                # accepting it
                if (lc($pack) eq lc($orig_module)) {
                    $module = $pack;
                    last;
                }
            }
        }
        close PACKFH;
    }

    print STDERR "Couldn't figure out the package name for $file\n"
      unless $module;

    return $module;
}

sub modules {
    my ($self) = @_;
    $self= $self->new(default=>1) if !ref $self;

    # Bug/feature of sort in scalar context requires this.
    return wantarray
        ? sort grep { not /^:private:$/ } keys %$self
        : grep { not /^:private:$/ } keys %$self;
}

sub files {
    my ($self, $module, $type, @under) = @_;
    $self= $self->new(default=>1) if !ref $self;

    # Validate arguments
    Carp::croak("$module is not installed") if (! exists($self->{$module}));
    $type = "all" if (! defined($type));
    Carp::croak('type must be "all", "prog" or "doc"')
        if ($type ne "all" && $type ne "prog" && $type ne "doc");

    my (@files);
    foreach my $file (keys(%{$self->{$module}{packlist}})) {
        push(@files, $file)
          if ($self->_is_type($file, $type) &&
              $self->_is_under($file, @under));
    }
    return(@files);
}

sub directories {
    my ($self, $module, $type, @under) = @_;
    $self= $self->new(default=>1) if !ref $self;
    my (%dirs);
    foreach my $file ($self->files($module, $type, @under)) {
        $dirs{dirname($file)}++;
    }
    return sort keys %dirs;
}

sub directory_tree {
    my ($self, $module, $type, @under) = @_;
    $self= $self->new(default=>1) if !ref $self;
    my (%dirs);
    foreach my $dir ($self->directories($module, $type, @under)) {
        $dirs{$dir}++;
        my ($last) = ("");
        while ($last ne $dir) {
            $last = $dir;
            $dir = dirname($dir);
            last if !$self->_is_under($dir, @under);
            $dirs{$dir}++;
        }
    }
    return(sort(keys(%dirs)));
}

sub validate {
    my ($self, $module, $remove) = @_;
    $self= $self->new(default=>1) if !ref $self;
    Carp::croak("$module is not installed") if (! exists($self->{$module}));
    return($self->{$module}{packlist}->validate($remove));
}

sub packlist {
    my ($self, $module) = @_;
    $self= $self->new(default=>1) if !ref $self;
    Carp::croak("$module is not installed") if (! exists($self->{$module}));
    return($self->{$module}{packlist});
}

sub version {
    my ($self, $module) = @_;
    $self= $self->new(default=>1) if !ref $self;
    Carp::croak("$module is not installed") if (! exists($self->{$module}));
    return($self->{$module}{version});
}

sub _debug_dump {
    my ($self, $module) = @_;
    $self= $self->new(default=>1) if !ref $self;
    local $self->{":private:"}{Config};
    require Data::Dumper;
    print Data::Dumper->new([$self])->Sortkeys(1)->Indent(1)->Dump();
}


1;

__END__

=head1 NAME

ExtUtils::Installed - Inventory management of installed modules

=head1 SYNOPSIS

   use ExtUtils::Installed;
   my ($inst) = ExtUtils::Installed->new( skip_cwd => 1 );
   my (@modules) = $inst->modules();
   my (@missing) = $inst->validate("DBI");
   my $all_files = $inst->files("DBI");
   my $files_below_usr_local = $inst->files("DBI", "all", "/usr/local");
   my $all_dirs = $inst->directories("DBI");
   my $dirs_below_usr_local = $inst->directory_tree("DBI", "prog");
   my $packlist = $inst->packlist("DBI");

=head1 DESCRIPTION

ExtUtils::Installed  provides a standard way to find out what core and module
files have been installed.  It uses the information stored in .packlist files
created during installation to provide this information.  In addition it
provides facilities to classify the installed files and to extract directory
information from the .packlist files.

=head1 USAGE

The new() function searches for all the installed .packlists on the system, and
stores their contents. The .packlists can be queried with the functions
described below. Where it searches by default is determined by the settings found
in C<%Config::Config>, and what the value is of the PERL5LIB environment variable.

=head1 METHODS

Unless specified otherwise all method can be called as class methods, or as object
methods. If called as class methods then the "default" object will be used, and if
necessary created using the current processes %Config and @INC.  See the
'default' option to new() for details.


=over 4

=item new()

This takes optional named parameters. Without parameters, this
searches for all the installed .packlists on the system using
information from C<%Config::Config> and the default module search
paths C<@INC>. The packlists are read using the
L<ExtUtils::Packlist> module.

If the named parameter C<skip_cwd> is true, the current directory C<.> will
be stripped from C<@INC> before searching for .packlists.  This keeps
ExtUtils::Installed from finding modules installed in other perls that
happen to be located below the current directory.

If the named parameter C<config_override> is specified,
it should be a reference to a hash which contains all information
usually found in C<%Config::Config>. For example, you can obtain
the configuration information for a separate perl installation and
pass that in.

    my $yoda_cfg  = get_fake_config('yoda');
    my $yoda_inst =
               ExtUtils::Installed->new(config_override=>$yoda_cfg);

Similarly, the parameter C<inc_override> may be a reference to an
array which is used in place of the default module search paths
from C<@INC>.

    use Config;
    my @dirs = split(/\Q$Config{path_sep}\E/, $ENV{PERL5LIB});
    my $p5libs = ExtUtils::Installed->new(inc_override=>\@dirs);

B<Note>: You probably do not want to use these options alone, almost always
you will want to set both together.

The parameter C<extra_libs> can be used to specify B<additional> paths to
search for installed modules. For instance

    my $installed =
             ExtUtils::Installed->new(extra_libs=>["/my/lib/path"]);

This should only be necessary if F</my/lib/path> is not in PERL5LIB.

Finally there is the 'default', and the related 'default_get' and 'default_set'
options. These options control the "default" object which is provided by the
class interface to the methods. Setting C<default_get> to true tells the constructor
to return the default object if it is defined. Setting C<default_set> to true tells
the constructor to make the default object the constructed object. Setting the
C<default> option is like setting both to true. This is used primarily internally
and probably isn't interesting to any real user.

=item modules()

This returns a list of the names of all the installed modules.  The perl 'core'
is given the special name 'Perl'.

=item files()

This takes one mandatory parameter, the name of a module.  It returns a list of
all the filenames from the package.  To obtain a list of core perl files, use
the module name 'Perl'.  Additional parameters are allowed.  The first is one
of the strings "prog", "doc" or "all", to select either just program files,
just manual files or all files.  The remaining parameters are a list of
directories. The filenames returned will be restricted to those under the
specified directories.

=item directories()

This takes one mandatory parameter, the name of a module.  It returns a list of
all the directories from the package.  Additional parameters are allowed.  The
first is one of the strings "prog", "doc" or "all", to select either just
program directories, just manual directories or all directories.  The remaining
parameters are a list of directories. The directories returned will be
restricted to those under the specified directories.  This method returns only
the leaf directories that contain files from the specified module.

=item directory_tree()

This is identical in operation to directories(), except that it includes all the
intermediate directories back up to the specified directories.

=item validate()

This takes one mandatory parameter, the name of a module.  It checks that all
the files listed in the modules .packlist actually exist, and returns a list of
any missing files.  If an optional second argument which evaluates to true is
given any missing files will be removed from the .packlist

=item packlist()

This returns the ExtUtils::Packlist object for the specified module.

=item version()

This returns the version number for the specified module.

=back

=head1 EXAMPLE

See the example in L<ExtUtils::Packlist>.

=head1 AUTHOR

Alan Burlison <Alan.Burlison@uk.sun.com>

=cut
