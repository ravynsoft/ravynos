#!./miniperl
use strict;
use warnings;
use Config;
use constant{IS_CROSS => defined $Config::Config{usecrosscompile} ? 1 : 0,
             IS_WIN32 => $^O eq 'MSWin32',
             IS_VMS   => $^O eq 'VMS',
             IS_UNIX  => $^O ne 'MSWin32' && $^O ne 'VMS',
};

my @ext_dirs = qw(cpan dist ext);
my $ext_dirs_re = '(?:' . join('|', @ext_dirs) . ')';

# This script acts as a simple interface for building extensions.

# It's actually a cut and shut of the Unix version ext/utils/makeext and the
# Windows version win32/build_ext.pl hence the two invocation styles.

# On Unix, it primarily used by the perl Makefile one extension at a time:
#
# d_dummy $(dynamic_ext): miniperl preplibrary FORCE
# 	@$(RUN) ./miniperl make_ext.pl --target=dynamic $@ MAKE=$(MAKE) LIBPERL_A=$(LIBPERL)
#
# On Windows or VMS,
# If '--static' is specified, static extensions will be built.
# If '--dynamic' is specified, dynamic extensions will be built.
# If '--nonxs' is specified, nonxs extensions will be built.
# If '--dynaloader' is specified, DynaLoader will be built.
# If '--all' is specified, all extensions will be built.
#
#    make_ext.pl "MAKE=make [-make_opts]" --dir=directory [--target=target] [--static|--dynamic|--all] +ext2 !ext1
#
# E.g.
# 
#     make_ext.pl "MAKE=nmake -nologo" --dir=..\ext
# 
#     make_ext.pl "MAKE=nmake -nologo" --dir=..\ext --target=clean
# 
# Will skip building extensions which are marked with an '!' char.
# Mostly because they still not ported to specified platform.
# 
# If any extensions are listed with a '+' char then only those
# extensions will be built, but only if they aren't countermanded
# by an '!ext' and are appropriate to the type of building being done.
# An extensions follows the format of Foo/Bar, which would be extension Foo::Bar

# To fix dependency ordering, on *nix systems, edit Makefile.SH to create a
# rule.  That isn't sufficient for other systems; you also have to do
# something in this file.  See the code at
#       '# XXX hack for dependency # ordering'
# below.
#
# The basic logic is:
#   1) if there's a Makefile.PL in git for the module, use it. and call make
#   2) If not, auto-generate one (normally)
#   3) unless the auto-generation code figures out that the extension is
#      *really* simple, in which case don't.  This will be for pure perl
#      modules, and all that is needed to be done is to copy from the source
#      to the dest directories.
#
# It may be deleted in a later release of perl so try to
# avoid using it for other purposes.

my (%excl, %incl, %opts, @extspec, @pass_through, $verbose);

foreach (@ARGV) {
    if (/^!(.*)$/) {
	$excl{$1} = 1;
    } elsif (/^\+(.*)$/) {
	$incl{$1} = 1;
    } elsif (/^--verbose$/ or /^-v$/) {
	$verbose = 1;
    } elsif (/^--([\w\-]+)$/) {
	$opts{$1} = 1;
    } elsif (/^--([\w\-]+)=(.*)$/) {
	push @{$opts{$1}}, $2;
    } elsif (/=/) {
	push @pass_through, $_;
    } elsif (length) {
	push @extspec, $_;
    }
}

my $static = $opts{static} || $opts{all};
my $dynamic = $opts{dynamic} || $opts{all};
my $nonxs = $opts{nonxs} || $opts{all};
my $dynaloader = $opts{dynaloader} || $opts{all};

# The Perl Makefile.SH will expand all extensions to
#	lib/auto/X/X.a  (or lib/auto/X/Y/Y.a if nested)
# A user wishing to run make_ext might use
#	X (or X/Y or X::Y if nested)

# canonise into X/Y form (pname)

foreach (@extspec) {
    if (s{^lib/auto/}{}) {
	# Remove lib/auto prefix and /*.* suffix
	s{/[^/]+\.[^/]+$}{};
    } elsif (s{^$ext_dirs_re/}{}) {
	# Remove ext/ prefix and /pm_to_blib suffix
	s{/pm_to_blib$}{};
	# Targets are given as files on disk, but the extension spec is still
	# written using /s for each ::
	tr!-!/!;
    } elsif (s{::}{\/}g) {
	# Convert :: to /
    } else {
	s/\..*o//;
    }
}

my $makecmd  = shift @pass_through; # Should be something like MAKE=make
unshift @pass_through, 'PERL_CORE=1';

my @dirs  = @{$opts{dir} || \@ext_dirs};
my $target   = $opts{target}[0];
$target = 'all' unless defined $target;

# Previously, $make was taken from config.sh.  However, the user might
# instead be running a possibly incompatible make.  This might happen if
# the user types "gmake" instead of a plain "make", for example.  The
# correct current value of MAKE will come through from the main perl
# makefile as MAKE=/whatever/make in $makecmd.  We'll be cautious in
# case third party users of this script (are there any?) don't have the
# MAKE=$(MAKE) argument, which was added after 5.004_03.
unless(defined $makecmd and $makecmd =~ /^MAKE=(.*)$/) {
    die "$0:  WARNING:  Please include MAKE=\$(MAKE) in \@ARGV\n";
}

# This isn't going to cope with anything fancy, such as spaces inside command
# names, but neither did what it replaced. Once there is a use case that needs
# it, please supply patches. Until then, I'm sticking to KISS
my @make = split ' ', $1 || $Config{make} || $ENV{MAKE};


if ($target eq '') {
    die "make_ext: no make target specified (eg all or clean)\n";
} elsif ($target !~ /^(?:all|clean|distclean|realclean|veryclean)$/) {
    # we are strict about what make_ext is used for because we emulate these
    # targets for simple modules:
    die "$0: unknown make target '$target'\n";
}

if (!@extspec and !$static and !$dynamic and !$nonxs and !$dynaloader)  {
    die "$0: no extension specified\n";
}

my $perl;
my %extra_passthrough;

if (IS_WIN32) {
    require Cwd;
    require FindExt;
    my $build = Cwd::getcwd();
    $perl = $^X;
    if ($perl =~ m#^\.\.#) {
	my $here = $build;
	$here =~ s{/}{\\}g;
	$perl = "$here\\$perl";
    }
    (my $topdir = $perl) =~ s/\\[^\\]+$//;
    # miniperl needs to find perlglob and pl2bat
    $ENV{PATH} = "$topdir;$topdir\\win32\\bin;$ENV{PATH}";
    my $pl2bat = "$topdir\\win32\\bin\\pl2bat";
    unless (-f "$pl2bat.bat") {
	my @args = ($perl, "-I$topdir\\lib", "-I$topdir\\cpan\\ExtUtils-PL2Bat\\lib", ("$pl2bat.pl") x 2);
	print "@args\n" if $verbose;
	system(@args) unless IS_CROSS;
    }

    print "In $build" if $verbose;
    foreach my $dir (@dirs) {
	chdir($dir) or die "Cannot cd to $dir: $!\n";
	(my $ext = Cwd::getcwd()) =~ s{/}{\\}g;
	FindExt::scan_ext($ext);
	FindExt::set_static_extensions(split ' ', $Config{static_ext});
	chdir $build
	    or die "Couldn't chdir to '$build': $!"; # restore our start directory
    }

    my @ext;
    push @ext, FindExt::static_ext() if $static;
    push @ext, FindExt::dynamic_ext() if $dynamic;
    push @ext, FindExt::nonxs_ext() if $nonxs;
    push @ext, 'DynaLoader' if $dynaloader;

    foreach (sort @ext) {
	if (%incl and !exists $incl{$_}) {
	    #warn "Skipping extension $_, not in inclusion list\n";
	    next;
	}
	if (exists $excl{$_}) {
	    warn "Skipping extension $_, not ported to current platform";
	    next;
	}
	push @extspec, $_;
	if($_ ne 'DynaLoader' && FindExt::is_static($_)) {
	    push @{$extra_passthrough{$_}}, 'LINKTYPE=static';
	}
    }

    chdir '..'
	or die "Couldn't chdir to build directory: $!"; # now in the Perl build
}
elsif (IS_VMS) {
    $perl = $^X;
    push @extspec, (split ' ', $Config{static_ext}) if $static;
    push @extspec, (split ' ', $Config{dynamic_ext}) if $dynamic;
    push @extspec, (split ' ', $Config{nonxs_ext}) if $nonxs;
    push @extspec, 'DynaLoader' if $dynaloader;
}

{ # XXX hack for dependency ordering
    # Cwd needs to be built before Encode recurses into subdirectories.
    # Pod::Simple needs to be built before Pod::Functions, but after 'if'
    # lib needs to be built before IO-Compress
    # This seems to be the simplest way to ensure this ordering:
    my (@first, @second, @other);
    foreach (@extspec) {
	if ($_ eq 'Cwd' || $_ eq 'if' || $_ eq 'lib') {
	    push @first, $_;
        }
	elsif ($_ eq 'Pod/Simple') {
	    push @second, $_;
	} else {
	    push @other, $_;
	}
    }
    @extspec = (@first, @second, @other);
}

if ($Config{osname} eq 'catamount' and @extspec) {
    # Snowball's chance of building extensions.
    die "This is $Config{osname}, not building $extspec[0], sorry.\n";
}
$ENV{PERL_CORE} = 1;

foreach my $spec (@extspec)  {
    my $mname = $spec;
    $mname =~ s!/!::!g;
    my $ext_pathname;

    # Try new style ext/Data-Dumper/ first
    my $copy = $spec;
    $copy =~ tr!/!-!;

    # List/Util.xs lives in Scalar-List-Utils, Cwd.xs lives in PathTools
    $copy = 'Scalar-List-Utils' if $copy eq 'List-Util';
    $copy = 'PathTools'         if $copy eq 'Cwd';

    foreach my $dir (@ext_dirs) {
	if (-d "$dir/$copy") {
	    $ext_pathname = "$dir/$copy";
	    last;
	}
    }

    if (!defined $ext_pathname) {
	if (-d "ext/$spec") {
	    # Old style ext/Data/Dumper/
	    $ext_pathname = "ext/$spec";
	} else {
	    warn "Can't find extension $spec in any of @ext_dirs";
	    next;
	}
    }

    print "\tMaking $mname ($target)\n" if $verbose;

    build_extension($ext_pathname, $perl, $mname, $target,
		    [@pass_through, @{$extra_passthrough{$spec} || []}]);
}

sub build_extension {
    my ($ext_dir, $perl, $mname, $target, $pass_through) = @_;

    unless (chdir "$ext_dir") {
	warn "Cannot cd to $ext_dir: $!";
	return;
    }

    my $up = $ext_dir;
    $up =~ s![^/]+!..!g;

    $perl ||= "$up/miniperl";
    my $return_dir = $up;
    my $lib_dir = "$up/lib";

    my ($makefile, $makefile_no_minus_f);
    if (IS_VMS) {
	$makefile = 'descrip.mms';
	if ($target =~ /clean$/
	    && !-f $makefile
	    && -f "${makefile}_old") {
	    $makefile = "${makefile}_old";
	}
    } else {
	$makefile = 'Makefile';
    }
    
    if (-f $makefile) {
	$makefile_no_minus_f = 0;
	open my $mfh, '<', $makefile or die "Cannot open $makefile: $!";
	while (<$mfh>) {
	    # Plagiarised from CPAN::Distribution
	    last if /MakeMaker post_initialize section/;
	    next unless /^#\s+VERSION_FROM\s+=>\s+(.+)/;
	    my $vmod = eval $1;
	    my $oldv;
	    while (<$mfh>) {
		next unless /^XS_VERSION = (\S+)/;
		$oldv = $1;
		last;
	    }
	    last unless defined $oldv;
	    require ExtUtils::MM_Unix;
	    defined (my $newv = parse_version MM $vmod) or last;
	    if (version->parse($newv) ne $oldv) {
		close $mfh or die "close $makefile: $!";
		_unlink($makefile);
		{
		    no warnings 'deprecated';
		    goto NO_MAKEFILE;
		}
	    }
	}

        if (IS_CROSS) {
            # If we're cross-compiling, it's possible that the host's
            # Makefiles are around.
            seek($mfh, 0, 0) or die "Cannot seek $makefile: $!";
            
            my $cross_makefile;
            while (<$mfh>) {
                # XXX This might not be throughout enough.
                # For example, it's possible to cause a false-positive
                # if cross compiling on and for the Raspberry Pi,
                # which is insane but plausible.
                # False positives are really not troublesome, though;
                # all they mean is that the module gets rebuilt.
                if (/^CC = \Q$Config{cc}\E/) {
                    $cross_makefile = 1;
                    last;
                }
            }
            
            if (!$cross_makefile) {
                print "Deleting non-Cross makefile\n";
                close $mfh or die "close $makefile: $!";
                _unlink($makefile);
            }
        }
    } else {
	$makefile_no_minus_f = 1;
    }

    if ($makefile_no_minus_f || !-f $makefile) {
	NO_MAKEFILE:
	if (!-f 'Makefile.PL') {
            unless (just_pm_to_blib($target, $ext_dir, $mname, $return_dir)) {
                # No problems returned, so it has faked everything for us. :-)
                chdir $return_dir || die "Cannot cd to $return_dir: $!";
                return;
            }

	    print "\nCreating Makefile.PL in $ext_dir for $mname\n" if $verbose;
	    my ($fromname, $key, $value);

	    $key = 'ABSTRACT_FROM';
	    # We need to cope well with various possible layouts
	    my @dirs = split /::/, $mname;
	    my $leaf = pop @dirs;
	    my $leafname = "$leaf.pm";
	    my $pathname = join '/', @dirs, $leafname;
	    my @locations = ($leafname, $pathname, "lib/$pathname");
	    foreach (@locations) {
		if (-f $_) {
		    $fromname = $_;
		    last;
		}
	}

	unless ($fromname) {
	    die "For $mname tried @locations in $ext_dir but can't find source";
	}
	($value = $fromname) =~ s/\.pm\z/.pod/;
	$value = $fromname unless -e $value;

            if ($mname eq 'Pod::Checker') {
                # the abstract in the .pm file is unparseable by MM,
                # so special-case it. We can't use the package's own
                # Makefile.PL, as it doesn't handle the executable scripts
                # right.
                $key = 'ABSTRACT';
                # this is copied from the CPAN Makefile.PL v 1.171
                $value = 'Pod::Checker verifies POD documentation contents for compliance with the POD format specifications';
            }

	    open my $fh, '>', 'Makefile.PL'
		or die "Can't open Makefile.PL for writing: $!";
	    printf $fh <<'EOM', $0, $mname, $fromname, $key, $value;
#-*- buffer-read-only: t -*-

# This Makefile.PL was written by %s.
# It will be deleted automatically by make realclean

use strict;
use ExtUtils::MakeMaker;

# This is what the .PL extracts to. Not the ultimate file that is installed.
# (ie Win32 runs pl2bat after this)

# Doing this here avoids all sort of quoting issues that would come from
# attempting to write out perl source with literals to generate the arrays and
# hash.
my @temps = 'Makefile.PL';
foreach (glob('scripts/pod*.PL')) {
    # The various pod*.PL extractors change directory. Doing that with relative
    # paths in @INC breaks. It seems the lesser of two evils to copy (to avoid)
    # the chdir doing anything, than to attempt to convert lib paths to
    # absolute, and potentially run into problems with quoting special
    # characters in the path to our build dir (such as spaces)
    require File::Copy;

    my $temp = $_;
    $temp =~ s!scripts/!!;
    File::Copy::copy($_, $temp) or die "Can't copy $temp to $_: $!";
    push @temps, $temp;
}

my $script_ext = $^O eq 'VMS' ? '.com' : '';
my %%pod_scripts;
foreach (glob('pod*.PL')) {
    my $script = $_;
    s/.PL$/$script_ext/i;
    $pod_scripts{$script} = $_;
}
my @exe_files = values %%pod_scripts;

WriteMakefile(
    NAME          => '%s',
    VERSION_FROM  => '%s',
    %-13s => '%s',
    realclean     => { FILES => "@temps" },
    (%%pod_scripts ? (
        PL_FILES  => \%%pod_scripts,
        EXE_FILES => \@exe_files,
        clean     => { FILES => "@exe_files" },
    ) : ()),
);

# ex: set ro:
EOM
	    close $fh or die "Can't close Makefile.PL: $!";
	    # As described in commit 23525070d6c0e51f:
	    # Push the atime and mtime of generated Makefile.PLs back 4
	    # seconds. In certain circumstances ( on virtual machines ) the
	    # generated Makefile.PL can produce a Makefile that is older than
	    # the Makefile.PL. Altering the atime and mtime backwards by 4
	    # seconds seems to resolve the issue.
	    eval {
        my $ftime = (stat('Makefile.PL'))[9] - 4;
        utime $ftime, $ftime, 'Makefile.PL';
	    };
        } elsif ($mname =~ /\A(?:Carp
                            |ExtUtils::CBuilder
                            |Safe
                            |Search::Dict)\z/x) {
            # An explicit list of dual-life extensions that have a Makefile.PL
            # for CPAN, but we have verified can also be built using the fakery.
            my ($problem) = just_pm_to_blib($target, $ext_dir, $mname, $return_dir);
            # We really need to sanity test that we can fake it.
            # Otherwise "skips" will go undetected, and the build slow down for
            # everyone, defeating the purpose.
            if (defined $problem) {
                if (-d "$return_dir/.git") {
                    # Get the list of files that git isn't ignoring:
                    my @files = `git ls-files --cached --others --exclude-standard 2>/dev/null`;
                    # on error (eg no git) we get nothing, but that's not a
                    # problem. The goal is to see if git thinks that the problem
                    # file is interesting, by getting a positive match with
                    # something git told us about, and if so bail out:
                    foreach (@files) {
                        chomp;
                        # We really need to sanity test that we can fake it.
                        # The intent is that this should only fail because
                        # you've just added a file to the dual-life dist that
                        # we can't handle. In which case you should either
                        # 1) remove the dist from the regex a few lines above.
                        # or
                        # 2) add the file to regex of "safe" filenames earlier
                        #    in this function, that starts with ChangeLog
                        die "FATAL - $0 has $mname in the list of simple extensions, but it now contains file '$problem' which we can't handle"
                            if $problem eq $_;
                    }
                    # There's an unexpected file, but it seems to be something
                    # that git will ignore. So fall through to the regular
                    # Makefile.PL handling code below, on the assumption that
                    # we won't get here for a clean build.
                }
                warn "WARNING - $0 is building $mname using EU::MM, as it found file '$problem'";
            } else {
                # It faked everything for us.
                chdir $return_dir || die "Cannot cd to $return_dir: $!";
                return;
            }
	}

        # We are going to have to use Makefile.PL:
	print "\nRunning Makefile.PL in $ext_dir\n" if $verbose;

	my @args = ("-I$lib_dir", 'Makefile.PL');
	if (IS_VMS) {
	    my $libd = VMS::Filespec::vmspath($lib_dir);
	    push @args, "INST_LIB=$libd", "INST_ARCHLIB=$libd";
	} else {
	    push @args, 'INSTALLDIRS=perl', 'INSTALLMAN1DIR=none',
		'INSTALLMAN3DIR=none';
	}
	push @args, @$pass_through;
	push @args, 'PERL=' . $perl if $perl; # use miniperl to run the Makefile later
	_quote_args(\@args) if IS_VMS;
	print join(' ', $perl, @args), "\n" if $verbose;
	my $code = do {
	   local $ENV{PERL_MM_USE_DEFAULT} = 1;
	    system $perl, @args;
	};
	if($code != 0){
	    #make sure next build attempt/run of make_ext.pl doesn't succeed
	    _unlink($makefile);
	    die "Unsuccessful Makefile.PL($ext_dir): code=$code";
	}

	# Right. The reason for this little hack is that we're sitting inside
	# a program run by ./miniperl, but there are tasks we need to perform
	# when the 'realclean', 'distclean' or 'veryclean' targets are run.
	# Unfortunately, they can be run *after* 'clean', which deletes
	# ./miniperl
	# So we do our best to leave a set of instructions identical to what
	# we would do if we are run directly as 'realclean' etc
	# Whilst we're perfect, unfortunately the targets we call are not, as
	# some of them rely on a $(PERL) for their own distclean targets.
	# But this always used to be a problem with the old /bin/sh version of
	# this.
	if (IS_UNIX) {
	    foreach my $clean_target ('realclean', 'veryclean') {
                fallback_cleanup($return_dir, $clean_target, <<"EOS");
cd $ext_dir
if test ! -f Makefile -a -f Makefile.old; then
    echo "Note: Using Makefile.old"
    make -f Makefile.old $clean_target MAKE='@make' @pass_through
else
    if test ! -f Makefile ; then
	echo "Warning: No Makefile!"
    fi
    @make $clean_target MAKE='@make' @pass_through
fi
cd $return_dir
EOS
	    }
	}
    }

    if (not -f $makefile) {
	print "Warning: No Makefile!\n";
    }

    if (IS_VMS) {
	_quote_args($pass_through);
	@$pass_through = (
			  "/DESCRIPTION=$makefile",
			  '/MACRO=(' . join(',',@$pass_through) . ')'
			 );
    }

    my @targ = ($target, @$pass_through);
    print "Making $target in $ext_dir\n@make @targ\n" if $verbose;
    local $ENV{PERL_INSTALL_QUIET} = 1;
    my $code = system(@make, @targ);
    if($code >> 8 != 0){ # probably cleaned itself, try again once more time
        $code = system(@make, @targ);
    }
    die "Unsuccessful make($ext_dir): code=$code" if $code != 0;

    chdir $return_dir || die "Cannot cd to $return_dir: $!";
}

sub _quote_args {
    my $args = shift; # must be array reference

    # Do not quote qualifiers that begin with '/'.
    map { if (!/^\//) {
          $_ =~ s/\"/""/g;     # escape C<"> by doubling
          $_ = q(").$_.q(");
        }
    } @{$args}
    ;
}

#guarentee that a file is deleted or die, void _unlink($filename)
#xxx replace with _unlink_or_rename from EU::Install?
sub _unlink {
    1 while unlink $_[0];
    my $err = $!;
    die "Can't unlink $_[0]: $err" if -f $_[0];
}

# Figure out if this extension is simple enough that it would only use
# ExtUtils::MakeMaker's pm_to_blib target. If we're confident that it would,
# then do all the work ourselves (returning an empty list), else return the
# name of a file that we identified as beyond our ability to handle.
#
# While this is clearly quite a bit more work than just letting
# ExtUtils::MakeMaker do it, and effectively is some code duplication, the time
# savings are impressive.

sub just_pm_to_blib {
    my ($target, $ext_dir, $mname, $return_dir) = @_;
    my ($has_lib, $has_top, $has_topdir);
    my ($last) = $mname =~ /([^:]+)$/;
    my ($first) = $mname =~ /^([^:]+)/;

    my $pm_to_blib = IS_VMS ? 'pm_to_blib.ts' : 'pm_to_blib';
    my $silent = defined $ENV{MAKEFLAGS} && $ENV{MAKEFLAGS} =~ /\b(s|silent|quiet)\b/;

    foreach my $leaf (<*>) {
        if (-d $leaf) {
            $leaf =~ s/\.DIR\z//i
                if IS_VMS;
            next if $leaf =~ /\A(?:\.|\.\.|t|demo)\z/;
            if ($leaf eq 'lib') {
                ++$has_lib;
                next;
            }
            if ($leaf eq $first) {
                ++$has_topdir;
                next;
            }
        }
        return $leaf
            unless -f _;
        $leaf =~ s/\.\z//
            if IS_VMS;
        # Makefile.PL is "safe" to ignore because we will only be called for
        # directories that hold a Makefile.PL if they are in the exception list.
        next
            if $leaf =~ /\A(ChangeLog
                            |Changes
                            |LICENSE
                            |Makefile\.PL
                            |MANIFEST
                            |META\.yml
                            |\Q$pm_to_blib\E
                            |README
                            |README\.patching
                            |README\.release
                            |\.gitignore
                            )\z/xi; # /i to deal with case munging systems.
        if ($leaf eq "$last.pm") {
            ++$has_top;
            next;
        }
        return $leaf;
    }
    return 'no lib/'
        unless $has_lib || $has_top;
    die "Inconsistent module $mname has both lib/ and $first/"
        if $has_lib && $has_topdir;

    print "Running pm_to_blib for $ext_dir directly\n"
      unless $silent;

    my %pm;
    if ($has_top) {
        my $to = $mname =~ s!::!/!gr;
        $pm{"$last.pm"} = "../../lib/$to.pm";
    }
    if ($has_lib || $has_topdir) {
        # strictly ExtUtils::MakeMaker uses the pm_to_blib target to install
        # .pm, pod and .pl files. We're just going to do it for .pm and .pod
        # files, to avoid problems on case munging file systems. Specifically,
        # _pm.PL which ExtUtils::MakeMaker should run munges to _PM.PL, and
        # looks a lot like a regular foo.pl (ie FOO.PL)
        my @found;
        require File::Find;
        unless (eval {
            File::Find::find({
                              no_chdir => 1,
                              wanted => sub {
                                  return if -d $_;
                                  # Bail out immediately with the problem file:
                                  die \$_
                                      unless -f _;
                                  die \$_
                                      unless /\A[^.]+\.(?:pm|pod)\z/i;
                                  push @found, $_;
                              }
                             }, $has_lib ? 'lib' : $first);
            1;
        }) {
            # Problem files aren't really errors:
            return ${$@}
                if ref $@ eq 'SCALAR';
            # But anything else is:
            die $@;
        }
        if ($has_lib) {
            $pm{$_} = "../../$_"
                foreach @found;
        } else {
            $pm{$_} = "../../lib/$_"
                foreach @found;
        }
    }
    # This is running under miniperl, so no autodie
    if ($target eq 'all') {
        my $need_update = 1;
        if (-f $pm_to_blib) {
            # avoid touching pm_to_blib unless there's something that
            # needs updating, see #126710
            $need_update = 0;
            my $test_at = -M _;
            while (my $from = each(%pm)) {
                if (-M $from < $test_at) {
                    ++$need_update;
                    last;
                }
            }
            keys %pm; # reset iterator
        }

        if ($need_update) {
            local $ENV{PERL_INSTALL_QUIET} = 1;
            require ExtUtils::Install;
            ExtUtils::Install::pm_to_blib(\%pm, '../../lib/auto');
            open my $fh, '>', $pm_to_blib
                or die "Can't open '$pm_to_blib': $!";
            print $fh "$0 has handled pm_to_blib directly\n";
            close $fh
                or die "Can't close '$pm_to_blib': $!";
            if (IS_UNIX) {
                # Fake the fallback cleanup
                my $fallback
                    = join '', map {s!^\.\./\.\./!!; "rm -f $_\n"} sort values %pm;
                foreach my $clean_target ('realclean', 'veryclean') {
                    fallback_cleanup($return_dir, $clean_target, $fallback);
                }
            }
        }
    } else {
        # A clean target.
        # For now, make the targets behave the same way as ExtUtils::MakeMaker
        # does
        _unlink($pm_to_blib);
        unless ($target eq 'clean') {
            # but cheat a bit, by relying on the top level Makefile clean target
            # to take out our directory lib/auto/...
            # (which it has to deal with, as cpan/foo/bar creates
            # lib/auto/foo/bar, but the EU::MM rule will only
            # rmdir lib/auto/foo/bar, leaving lib/auto/foo
            _unlink($_)
                foreach sort values %pm;
        }
    }
    return;
}

sub fallback_cleanup {
    my ($dir, $clean_target, $contents) = @_;
    my $file = "$dir/$clean_target.sh";
    open my $fh, '>>', $file or die "open $file: $!";
    # Quite possible that we're being run in parallel here.
    # Can't use Fcntl this early to get the LOCK_EX
    flock $fh, 2 or warn "flock $file: $!";
    print $fh $contents or die "print $file: $!";
    close $fh or die "close $file: $!";
}
