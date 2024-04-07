package File::Find;
use 5.006;
use strict;
use warnings;
use warnings::register;
our $VERSION = '1.43';
use Exporter 'import';
require Cwd;

our @EXPORT = qw(find finddepth);


use strict;
my $Is_VMS = $^O eq 'VMS';
my $Is_Win32 = $^O eq 'MSWin32';

require File::Basename;
require File::Spec;

# Should ideally be my() not our() but local() currently
# refuses to operate on lexicals

our %SLnkSeen;
our ($wanted_callback, $avoid_nlink, $bydepth, $no_chdir, $follow,
    $follow_skip, $full_check, $untaint, $untaint_skip, $untaint_pat,
    $pre_process, $post_process, $dangling_symlinks);

sub contract_name {
    my ($cdir,$fn) = @_;

    return substr($cdir,0,rindex($cdir,'/')) if $fn eq $File::Find::current_dir;

    $cdir = substr($cdir,0,rindex($cdir,'/')+1);

    $fn =~ s|^\./||;

    my $abs_name= $cdir . $fn;

    if (substr($fn,0,3) eq '../') {
       1 while $abs_name =~ s!/[^/]*/\.\./+!/!;
    }

    return $abs_name;
}

sub _is_absolute {
    return $_[0] =~ m|^(?:[A-Za-z]:)?/| if $Is_Win32;
    return substr($_[0], 0, 1) eq '/';
}

sub _is_root {
    return $_[0] =~ m|^(?:[A-Za-z]:)?/\z| if $Is_Win32;
    return $_[0] eq '/';
}

sub PathCombine($$) {
    my ($Base,$Name) = @_;
    my $AbsName;

    if (_is_absolute($Name)) {
        $AbsName= $Name;
    }
    else {
        $AbsName= contract_name($Base,$Name);
    }

    # (simple) check for recursion
    my $newlen= length($AbsName);
    if ($newlen <= length($Base)) {
        if (($newlen == length($Base) || substr($Base,$newlen,1) eq '/')
            && $AbsName eq substr($Base,0,$newlen))
        {
            return undef;
        }
    }
    return $AbsName;
}

sub Follow_SymLink($) {
    my ($AbsName) = @_;

    my ($NewName,$DEV, $INO);
    ($DEV, $INO)= lstat $AbsName;

    while (-l _) {
        if ($SLnkSeen{$DEV, $INO}++) {
            if ($follow_skip < 2) {
                die "$AbsName is encountered a second time";
            }
            else {
                return undef;
            }
        }
        my $Link = readlink($AbsName);
        # canonicalize directory separators
        $Link =~ s|\\|/|g if $Is_Win32;
        $NewName= PathCombine($AbsName, $Link);
        unless(defined $NewName) {
            if ($follow_skip < 2) {
                die "$AbsName is a recursive symbolic link";
            }
            else {
                return undef;
            }
        }
        else {
            $AbsName= $NewName;
        }
        ($DEV, $INO) = lstat($AbsName);
        return undef unless defined $DEV;  #  dangling symbolic link
    }

    if ($full_check && defined $DEV && $SLnkSeen{$DEV, $INO}++) {
        if ( ($follow_skip < 1) || ((-d _) && ($follow_skip < 2)) ) {
            die "$AbsName encountered a second time";
        }
        else {
            return undef;
        }
    }

    return $AbsName;
}

our($dir, $name, $fullname, $prune);
sub _find_dir_symlnk($$$);
sub _find_dir($$$);

# check whether or not a scalar variable is tainted
# (code straight from the Camel, 3rd ed., page 561)
sub is_tainted_pp {
    my $arg = shift;
    my $nada = substr($arg, 0, 0); # zero-length
    local $@;
    eval { eval "# $nada" };
    return length($@) != 0;
}


sub _find_opt {
    my $wanted = shift;
    return unless @_;
    die "invalid top directory" unless defined $_[0];

    # This function must local()ize everything because callbacks may
    # call find() or finddepth()

    local %SLnkSeen;
    local ($wanted_callback, $avoid_nlink, $bydepth, $no_chdir, $follow,
        $follow_skip, $full_check, $untaint, $untaint_skip, $untaint_pat,
        $pre_process, $post_process, $dangling_symlinks);
    local($dir, $name, $fullname, $prune);
    local *_ = \my $a;

    my $cwd            = $wanted->{bydepth} ? Cwd::fastcwd() : Cwd::getcwd();
    if ($Is_VMS) {
        # VMS returns this by default in VMS format which just doesn't
        # work for the rest of this module.
        $cwd = VMS::Filespec::unixpath($cwd);

        # Apparently this is not expected to have a trailing space.
        # To attempt to make VMS/UNIX conversions mostly reversible,
        # a trailing slash is needed.  The run-time functions ignore the
        # resulting double slash, but it causes the perl tests to fail.
        $cwd =~ s#/\z##;

        # This comes up in upper case now, but should be lower.
        # In the future this could be exact case, no need to change.
    }
    my $cwd_untainted  = $cwd;
    my $check_t_cwd    = 1;
    $wanted_callback   = $wanted->{wanted};
    $bydepth           = $wanted->{bydepth};
    $pre_process       = $wanted->{preprocess};
    $post_process      = $wanted->{postprocess};
    $no_chdir          = $wanted->{no_chdir};
    $full_check        = $wanted->{follow};
    $follow            = $full_check || $wanted->{follow_fast};
    $follow_skip       = $wanted->{follow_skip};
    $untaint           = $wanted->{untaint};
    $untaint_pat       = $wanted->{untaint_pattern};
    $untaint_skip      = $wanted->{untaint_skip};
    $dangling_symlinks = $wanted->{dangling_symlinks};

    # for compatibility reasons (find.pl, find2perl)
    local our ($topdir, $topdev, $topino, $topmode, $topnlink);

    # a symbolic link to a directory doesn't increase the link count
    $avoid_nlink      = $follow || $File::Find::dont_use_nlink;

    my ($abs_dir, $Is_Dir);

    Proc_Top_Item:
    foreach my $TOP (@_) {
        my $top_item = $TOP;
        $top_item = VMS::Filespec::unixify($top_item) if $Is_VMS;

        ($topdev,$topino,$topmode,$topnlink) = $follow ? stat $top_item : lstat $top_item;

        # canonicalize directory separators
        $top_item =~ s|[/\\]|/|g if $Is_Win32;

        # no trailing / unless path is root
        $top_item =~ s|/\z|| unless _is_root($top_item);

        $Is_Dir= 0;

        if ($follow) {

            if (_is_absolute($top_item)) {
                $abs_dir = $top_item;
            }
            elsif ($top_item eq $File::Find::current_dir) {
                $abs_dir = $cwd;
            }
            else {  # care about any  ../
                $top_item =~ s/\.dir\z//i if $Is_VMS;
                $abs_dir = contract_name("$cwd/",$top_item);
            }
            $abs_dir= Follow_SymLink($abs_dir);
            unless (defined $abs_dir) {
                if ($dangling_symlinks) {
                    if (ref $dangling_symlinks eq 'CODE') {
                        $dangling_symlinks->($top_item, $cwd);
                    } else {
                        warnings::warnif "$top_item is a dangling symbolic link\n";
                    }
                }
                next Proc_Top_Item;
            }

            if (-d _) {
                $top_item =~ s/\.dir\z//i if $Is_VMS;
                _find_dir_symlnk($wanted, $abs_dir, $top_item);
                $Is_Dir= 1;
            }
        }
        else { # no follow
            $topdir = $top_item;
            unless (defined $topnlink) {
                warnings::warnif "Can't stat $top_item: $!\n";
                next Proc_Top_Item;
            }
            if (-d _) {
                $top_item =~ s/\.dir\z//i if $Is_VMS;
                _find_dir($wanted, $top_item, $topnlink);
                $Is_Dir= 1;
            }
            else {
                $abs_dir= $top_item;
            }
        }

        unless ($Is_Dir) {
            unless (($_,$dir) = File::Basename::fileparse($abs_dir)) {
                ($dir,$_) = ('./', $top_item);
            }

            $abs_dir = $dir;
            if (( $untaint ) && (is_tainted($dir) )) {
                ( $abs_dir ) = $dir =~ m|$untaint_pat|;
                unless (defined $abs_dir) {
                    if ($untaint_skip == 0) {
                        die "directory $dir is still tainted";
                    }
                    else {
                        next Proc_Top_Item;
                    }
                }
            }

            unless ($no_chdir || chdir $abs_dir) {
                warnings::warnif "Couldn't chdir $abs_dir: $!\n";
                next Proc_Top_Item;
            }

            $name = $abs_dir . $_; # $File::Find::name
            $_ = $name if $no_chdir;

            { $wanted_callback->() }; # protect against wild "next"

        }

        unless ( $no_chdir ) {
            if ( ($check_t_cwd) && (($untaint) && (is_tainted($cwd) )) ) {
                ( $cwd_untainted ) = $cwd =~ m|$untaint_pat|;
                unless (defined $cwd_untainted) {
                    die "insecure cwd in find(depth)";
                }
                $check_t_cwd = 0;
            }
            unless (chdir $cwd_untainted) {
                die "Can't cd to $cwd: $!\n";
            }
        }
    }
}

# API:
#  $wanted
#  $p_dir :  "parent directory"
#  $nlink :  what came back from the stat
# preconditions:
#  chdir (if not no_chdir) to dir

sub _find_dir($$$) {
    my ($wanted, $p_dir, $nlink) = @_;
    my ($CdLvl,$Level) = (0,0);
    my @Stack;
    my @filenames;
    my ($subcount,$sub_nlink);
    my $SE= [];
    my $dir_name= $p_dir;
    my $dir_pref;
    my $dir_rel = $File::Find::current_dir;
    my $tainted = 0;
    my $no_nlink;

    if ($Is_VMS) {
        # VMS is returning trailing .dir on directories
        # and trailing . on files and symbolic links
        # in UNIX syntax.
        #

        $p_dir =~ s/\.(dir)?$//i unless $p_dir eq '.';

        $dir_pref = ($p_dir =~ m/[\]>]+$/ ? $p_dir : "$p_dir/" );
    }
    else {
        $dir_pref = _is_root($p_dir) ? $p_dir : "$p_dir/";
    }

    local ($dir, $name, $prune);

    unless ( $no_chdir || ($p_dir eq $File::Find::current_dir)) {
        my $udir = $p_dir;
        if (( $untaint ) && (is_tainted($p_dir) )) {
            ( $udir ) = $p_dir =~ m|$untaint_pat|;
            unless (defined $udir) {
                if ($untaint_skip == 0) {
                    die "directory $p_dir is still tainted";
                }
                else {
                    return;
                }
            }
        }
        unless (chdir ($Is_VMS && $udir !~ /[\/\[<]+/ ? "./$udir" : $udir)) {
            warnings::warnif "Can't cd to $udir: $!\n";
            return;
        }
    }

    # push the starting directory
    push @Stack,[$CdLvl,$p_dir,$dir_rel,-1]  if  $bydepth;

    while (defined $SE) {
        unless ($bydepth) {
            $dir= $p_dir; # $File::Find::dir
            $name= $dir_name; # $File::Find::name
            $_= ($no_chdir ? $dir_name : $dir_rel ); # $_
            # prune may happen here
            $prune= 0;
            { $wanted_callback->() };   # protect against wild "next"
            next if $prune;
        }

        # change to that directory
        unless ($no_chdir || ($dir_rel eq $File::Find::current_dir)) {
            my $udir= $dir_rel;
            if ( ($untaint) && (($tainted) || ($tainted = is_tainted($dir_rel) )) ) {
                ( $udir ) = $dir_rel =~ m|$untaint_pat|;
                unless (defined $udir) {
                    if ($untaint_skip == 0) {
                        die "directory (" . ($p_dir ne '/' ? $p_dir : '') . "/) $dir_rel is still tainted";
                    } else { # $untaint_skip == 1
                        next;
                    }
                }
            }
            unless (chdir ($Is_VMS && $udir !~ /[\/\[<]+/ ? "./$udir" : $udir)) {
                warnings::warnif "Can't cd to (" .
                    ($p_dir ne '/' ? $p_dir : '') . "/) $udir: $!\n";
                next;
            }
            $CdLvl++;
        }

        $dir= $dir_name; # $File::Find::dir

        # Get the list of files in the current directory.
        my $dh;
        unless (opendir $dh, ($no_chdir ? $dir_name : $File::Find::current_dir)) {
            warnings::warnif "Can't opendir($dir_name): $!\n";
            next;
        }
        @filenames = readdir $dh;
        closedir($dh);
        @filenames = $pre_process->(@filenames) if $pre_process;
        push @Stack,[$CdLvl,$dir_name,"",-2]   if $post_process;

        # default: use whatever was specified
        # (if $nlink >= 2, and $avoid_nlink == 0, this will switch back)
        $no_nlink = $avoid_nlink;
        # if dir has wrong nlink count, force switch to slower stat method
        $no_nlink = 1 if ($nlink < 2);

        if ($nlink == 2 && !$no_nlink) {
            # This dir has no subdirectories.
            for my $FN (@filenames) {
                if ($Is_VMS) {
                    # Big hammer here - Compensate for VMS trailing . and .dir
                    # No win situation until this is changed, but this
                    # will handle the majority of the cases with breaking the fewest

                    $FN =~ s/\.dir\z//i;
                    $FN =~ s#\.$## if ($FN ne '.');
                }
                next if $FN =~ $File::Find::skip_pattern;

                $name = $dir_pref . $FN; # $File::Find::name
                $_ = ($no_chdir ? $name : $FN); # $_
                { $wanted_callback->() }; # protect against wild "next"
            }

        }
        else {
            # This dir has subdirectories.
            $subcount = $nlink - 2;

            # HACK: insert directories at this position, so as to preserve
            # the user pre-processed ordering of files (thus ensuring
            # directory traversal is in user sorted order, not at random).
            my $stack_top = @Stack;

            for my $FN (@filenames) {
                next if $FN =~ $File::Find::skip_pattern;
                if ($subcount > 0 || $no_nlink) {
                    # Seen all the subdirs?
                    # check for directoriness.
                    # stat is faster for a file in the current directory
                    $sub_nlink = (lstat ($no_chdir ? $dir_pref . $FN : $FN))[3];

                    if (-d _) {
                        --$subcount;
                        $FN =~ s/\.dir\z//i if $Is_VMS;
                        # HACK: replace push to preserve dir traversal order
                        #push @Stack,[$CdLvl,$dir_name,$FN,$sub_nlink];
                        splice @Stack, $stack_top, 0,
                                 [$CdLvl,$dir_name,$FN,$sub_nlink];
                    }
                    else {
                        $name = $dir_pref . $FN; # $File::Find::name
                        $_= ($no_chdir ? $name : $FN); # $_
                        { $wanted_callback->() }; # protect against wild "next"
                    }
                }
                else {
                    $name = $dir_pref . $FN; # $File::Find::name
                    $_= ($no_chdir ? $name : $FN); # $_
                    { $wanted_callback->() }; # protect against wild "next"
                }
            }
        }
    }
    continue {
        while ( defined ($SE = pop @Stack) ) {
            ($Level, $p_dir, $dir_rel, $nlink) = @$SE;
            if ($CdLvl > $Level && !$no_chdir) {
                my $tmp;
                if ($Is_VMS) {
                    $tmp = '[' . ('-' x ($CdLvl-$Level)) . ']';
                }
                else {
                    $tmp = join('/',('..') x ($CdLvl-$Level));
                }
                die "Can't cd to $tmp from $dir_name: $!"
                    unless chdir ($tmp);
                $CdLvl = $Level;
            }

            if ($^O eq 'VMS') {
                if ($p_dir =~ m/[\]>]+$/) {
                    $dir_name = $p_dir;
                    $dir_name =~ s/([\]>]+)$/.$dir_rel$1/;
                    $dir_pref = $dir_name;
                }
                else {
                    $dir_name = "$p_dir/$dir_rel";
                    $dir_pref = "$dir_name/";
                }
            }
            else {
                $dir_name = _is_root($p_dir) ? "$p_dir$dir_rel" : "$p_dir/$dir_rel";
                $dir_pref = "$dir_name/";
            }

            if ( $nlink == -2 ) {
                $name = $dir = $p_dir; # $File::Find::name / dir
                $_ = $File::Find::current_dir;
                $post_process->();              # End-of-directory processing
            }
            elsif ( $nlink < 0 ) {  # must be finddepth, report dirname now
                $name = $dir_name;
                if ( substr($name,-2) eq '/.' ) {
                    substr($name, length($name) == 2 ? -1 : -2) = '';
                }
                $dir = $p_dir;
                $_ = ($no_chdir ? $dir_name : $dir_rel );
                if ( substr($_,-2) eq '/.' ) {
                    substr($_, length($_) == 2 ? -1 : -2) = '';
                }
                { $wanted_callback->() }; # protect against wild "next"
             }
             else {
                push @Stack,[$CdLvl,$p_dir,$dir_rel,-1]  if  $bydepth;
                last;
            }
        }
    }
}


# API:
#  $wanted
#  $dir_loc : absolute location of a dir
#  $p_dir   : "parent directory"
# preconditions:
#  chdir (if not no_chdir) to dir

sub _find_dir_symlnk($$$) {
    my ($wanted, $dir_loc, $p_dir) = @_; # $dir_loc is the absolute directory
    my @Stack;
    my @filenames;
    my $new_loc;
    my $updir_loc = $dir_loc; # untainted parent directory
    my $SE = [];
    my $dir_name = $p_dir;
    my $dir_pref;
    my $loc_pref;
    my $dir_rel = $File::Find::current_dir;
    my $byd_flag; # flag for pending stack entry if $bydepth
    my $tainted = 0;
    my $ok = 1;

    $dir_pref = _is_root($p_dir) ? $p_dir : "$p_dir/";
    $loc_pref = _is_root($dir_loc) ? $dir_loc : "$dir_loc/";

    local ($dir, $name, $fullname, $prune);

    unless ($no_chdir) {
        # untaint the topdir
        if (( $untaint ) && (is_tainted($dir_loc) )) {
            ( $updir_loc ) = $dir_loc =~ m|$untaint_pat|; # parent dir, now untainted
            # once untainted, $updir_loc is pushed on the stack (as parent directory);
            # hence, we don't need to untaint the parent directory every time we chdir
            # to it later
            unless (defined $updir_loc) {
                if ($untaint_skip == 0) {
                    die "directory $dir_loc is still tainted";
                }
                else {
                    return;
                }
            }
        }
        $ok = chdir($updir_loc) unless ($p_dir eq $File::Find::current_dir);
        unless ($ok) {
            warnings::warnif "Can't cd to $updir_loc: $!\n";
            return;
        }
    }

    push @Stack,[$dir_loc,$updir_loc,$p_dir,$dir_rel,-1]  if  $bydepth;

    while (defined $SE) {

        unless ($bydepth) {
            # change (back) to parent directory (always untainted)
            unless ($no_chdir) {
                unless (chdir $updir_loc) {
                    warnings::warnif "Can't cd to $updir_loc: $!\n";
                    next;
                }
            }
            $dir= $p_dir; # $File::Find::dir
            $name= $dir_name; # $File::Find::name
            $_= ($no_chdir ? $dir_name : $dir_rel ); # $_
            $fullname= $dir_loc; # $File::Find::fullname
            # prune may happen here
            $prune= 0;
            lstat($_); # make sure  file tests with '_' work
            { $wanted_callback->() }; # protect against wild "next"
            next if $prune;
        }

        # change to that directory
        unless ($no_chdir || ($dir_rel eq $File::Find::current_dir)) {
            $updir_loc = $dir_loc;
            if ( ($untaint) && (($tainted) || ($tainted = is_tainted($dir_loc) )) ) {
                # untaint $dir_loc, what will be pushed on the stack as (untainted) parent dir
                ( $updir_loc ) = $dir_loc =~ m|$untaint_pat|;
                unless (defined $updir_loc) {
                    if ($untaint_skip == 0) {
                        die "directory $dir_loc is still tainted";
                    }
                    else {
                        next;
                    }
                }
            }
            unless (chdir $updir_loc) {
                warnings::warnif "Can't cd to $updir_loc: $!\n";
                next;
            }
        }

        $dir = $dir_name; # $File::Find::dir

        # Get the list of files in the current directory.
        my $dh;
        unless (opendir $dh, ($no_chdir ? $dir_loc : $File::Find::current_dir)) {
            warnings::warnif "Can't opendir($dir_loc): $!\n";
            next;
        }
        @filenames = readdir $dh;
        closedir($dh);

        for my $FN (@filenames) {
            if ($Is_VMS) {
                # Big hammer here - Compensate for VMS trailing . and .dir
                # No win situation until this is changed, but this
                # will handle the majority of the cases with breaking the fewest.

                $FN =~ s/\.dir\z//i;
                $FN =~ s#\.$## if ($FN ne '.');
            }
            next if $FN =~ $File::Find::skip_pattern;

            # follow symbolic links / do an lstat
            $new_loc = Follow_SymLink($loc_pref.$FN);

            # ignore if invalid symlink
            unless (defined $new_loc) {
                if (!defined -l _ && $dangling_symlinks) {
                $fullname = undef;
                    if (ref $dangling_symlinks eq 'CODE') {
                        $dangling_symlinks->($FN, $dir_pref);
                    } else {
                        warnings::warnif "$dir_pref$FN is a dangling symbolic link\n";
                    }
                }
            else {
                $fullname = $loc_pref . $FN;
            }
                $name = $dir_pref . $FN;
                $_ = ($no_chdir ? $name : $FN);
                { $wanted_callback->() };
                next;
            }

            if (-d _) {
                if ($Is_VMS) {
                    $FN =~ s/\.dir\z//i;
                    $FN =~ s#\.$## if ($FN ne '.');
                    $new_loc =~ s/\.dir\z//i;
                    $new_loc =~ s#\.$## if ($new_loc ne '.');
                }
                push @Stack,[$new_loc,$updir_loc,$dir_name,$FN,1];
            }
            else {
                $fullname = $new_loc; # $File::Find::fullname
                $name = $dir_pref . $FN; # $File::Find::name
                $_ = ($no_chdir ? $name : $FN); # $_
                { $wanted_callback->() }; # protect against wild "next"
            }
        }

    }
    continue {
        while (defined($SE = pop @Stack)) {
            ($dir_loc, $updir_loc, $p_dir, $dir_rel, $byd_flag) = @$SE;
            $dir_name = _is_root($p_dir) ? "$p_dir$dir_rel" : "$p_dir/$dir_rel";
            $dir_pref = "$dir_name/";
            $loc_pref = "$dir_loc/";
            if ( $byd_flag < 0 ) {  # must be finddepth, report dirname now
                unless ($no_chdir || ($dir_rel eq $File::Find::current_dir)) {
                    unless (chdir $updir_loc) { # $updir_loc (parent dir) is always untainted
                        warnings::warnif "Can't cd to $updir_loc: $!\n";
                        next;
                    }
                }
                $fullname = $dir_loc; # $File::Find::fullname
                $name = $dir_name; # $File::Find::name
                if ( substr($name,-2) eq '/.' ) {
                    substr($name, length($name) == 2 ? -1 : -2) = ''; # $File::Find::name
                }
                $dir = $p_dir; # $File::Find::dir
                $_ = ($no_chdir ? $dir_name : $dir_rel); # $_
                if ( substr($_,-2) eq '/.' ) {
                    substr($_, length($_) == 2 ? -1 : -2) = '';
                }

                lstat($_); # make sure file tests with '_' work
                { $wanted_callback->() }; # protect against wild "next"
            }
            else {
                push @Stack,[$dir_loc, $updir_loc, $p_dir, $dir_rel,-1]  if  $bydepth;
                last;
            }
        }
    }
}


sub wrap_wanted {
    my $wanted = shift;
    if ( ref($wanted) eq 'HASH' ) {
        # RT #122547
        my %valid_options = map {$_ => 1} qw(
            wanted
            bydepth
            preprocess
            postprocess
            follow
            follow_fast
            follow_skip
            dangling_symlinks
            no_chdir
            untaint
            untaint_pattern
            untaint_skip
        );
        my @invalid_options = ();
        for my $v (keys %{$wanted}) {
            push @invalid_options, $v unless exists $valid_options{$v};
        }
        warn "Invalid option(s): @invalid_options" if @invalid_options;

        unless( exists $wanted->{wanted} and ref( $wanted->{wanted} ) eq 'CODE' ) {
            die 'no &wanted subroutine given';
        }
        if ( $wanted->{follow} || $wanted->{follow_fast}) {
            $wanted->{follow_skip} = 1 unless defined $wanted->{follow_skip};
        }
        if ( $wanted->{untaint} ) {
            $wanted->{untaint_pattern} = $File::Find::untaint_pattern
            unless defined $wanted->{untaint_pattern};
            $wanted->{untaint_skip} = 0 unless defined $wanted->{untaint_skip};
        }
        return $wanted;
    }
    elsif( ref( $wanted ) eq 'CODE' ) {
        return { wanted => $wanted };
    }
    else {
       die 'no &wanted subroutine given';
    }
}

sub find {
    my $wanted = shift;
    _find_opt(wrap_wanted($wanted), @_);
}

sub finddepth {
    my $wanted = wrap_wanted(shift);
    $wanted->{bydepth} = 1;
    _find_opt($wanted, @_);
}

# default
$File::Find::skip_pattern    = qr/^\.{1,2}\z/;
$File::Find::untaint_pattern = qr|^([-+@\w./]+)$|;

# this _should_ work properly on all platforms
# where File::Find can be expected to work
$File::Find::current_dir = File::Spec->curdir || '.';

$File::Find::dont_use_nlink = 1;

# We need a function that checks if a scalar is tainted. Either use the
# Scalar::Util module's tainted() function or our (slower) pure Perl
# fallback is_tainted_pp()
{
    local $@;
    eval { require Scalar::Util };
    *is_tainted = $@ ? \&is_tainted_pp : \&Scalar::Util::tainted;
}

1;

__END__

=head1 NAME

File::Find - Traverse a directory tree.

=head1 SYNOPSIS

    use File::Find;
    find(\&wanted, @directories_to_search);
    sub wanted { ... }

    use File::Find;
    finddepth(\&wanted, @directories_to_search);
    sub wanted { ... }

    use File::Find;
    find({ wanted => \&process, follow => 1 }, '.');

=head1 DESCRIPTION

These are functions for searching through directory trees doing work
on each file found similar to the Unix I<find> command.  File::Find
exports two functions, C<find> and C<finddepth>.  They work similarly
but have subtle differences.

=over 4

=item B<find>

  find(\&wanted,  @directories);
  find(\%options, @directories);

C<find()> does a depth-first search over the given C<@directories> in
the order they are given.  For each file or directory found, it calls
the C<&wanted> subroutine.  (See below for details on how to use the
C<&wanted> function).  Additionally, for each directory found, it will
C<chdir()> into that directory and continue the search, invoking the
C<&wanted> function on each file or subdirectory in the directory.

=item B<finddepth>

  finddepth(\&wanted,  @directories);
  finddepth(\%options, @directories);

C<finddepth()> works just like C<find()> except that it invokes the
C<&wanted> function for a directory I<after> invoking it for the
directory's contents.  It does a postorder traversal instead of a
preorder traversal, working from the bottom of the directory tree up
where C<find()> works from the top of the tree down.

=back

Despite the name of the C<finddepth()> function, both C<find()> and
C<finddepth()> perform a depth-first search of the directory hierarchy.

=head2 %options

The first argument to C<find()> is either a code reference to your
C<&wanted> function, or a hash reference describing the operations
to be performed for each file.  The
code reference is described in L</The wanted function> below.

Here are the possible keys for the hash:

=over 4

=item C<wanted>

The value should be a code reference.  This code reference is
described in L</The wanted function> below. The C<&wanted> subroutine is
mandatory.

=item C<bydepth>

Reports the name of a directory only AFTER all its entries
have been reported.  Entry point C<finddepth()> is a shortcut for
specifying C<< { bydepth => 1 } >> in the first argument of C<find()>.

=item C<preprocess>

The value should be a code reference. This code reference is used to
preprocess the current directory. The name of the currently processed
directory is in C<$File::Find::dir>. Your preprocessing function is
called after C<readdir()>, but before the loop that calls the C<wanted()>
function. It is called with a list of strings (actually file/directory
names) and is expected to return a list of strings. The code can be
used to sort the file/directory names alphabetically, numerically,
or to filter out directory entries based on their name alone. When
I<follow> or I<follow_fast> are in effect, C<preprocess> is a no-op.

=item C<postprocess>

The value should be a code reference. It is invoked just before leaving
the currently processed directory. It is called in void context with no
arguments. The name of the current directory is in C<$File::Find::dir>. This
hook is handy for summarizing a directory, such as calculating its disk
usage. When I<follow> or I<follow_fast> are in effect, C<postprocess> is a
no-op.

=item C<follow>

Causes symbolic links to be followed. Since directory trees with symbolic
links (followed) may contain files more than once and may even have
cycles, a hash has to be built up with an entry for each file.
This might be expensive both in space and time for a large
directory tree. See L</follow_fast> and L</follow_skip> below.
If either I<follow> or I<follow_fast> is in effect:

=over 4

=item *

It is guaranteed that an I<lstat> has been called before the user's
C<wanted()> function is called. This enables fast file checks involving C<_>.
Note that this guarantee no longer holds if I<follow> or I<follow_fast>
are not set.

=item *

There is a variable C<$File::Find::fullname> which holds the absolute
pathname of the file with all symbolic links resolved.  If the link is
a dangling symbolic link, then fullname will be set to C<undef>.

=back

=item C<follow_fast>

This is similar to I<follow> except that it may report some files more
than once.  It does detect cycles, however.  Since only symbolic links
have to be hashed, this is much cheaper both in space and time.  If
processing a file more than once (by the user's C<wanted()> function)
is worse than just taking time, the option I<follow> should be used.

=item C<follow_skip>

C<follow_skip==1>, which is the default, causes all files which are
neither directories nor symbolic links to be ignored if they are about
to be processed a second time. If a directory or a symbolic link
are about to be processed a second time, File::Find dies.

C<follow_skip==0> causes File::Find to die if any file is about to be
processed a second time.

C<follow_skip==2> causes File::Find to ignore any duplicate files and
directories but to proceed normally otherwise.

=item C<dangling_symlinks>

Specifies what to do with symbolic links whose target doesn't exist.
If true and a code reference, will be called with the symbolic link
name and the directory it lives in as arguments.  Otherwise, if true
and warnings are on, a warning of the form C<"symbolic_link_name is a dangling
symbolic link\n"> will be issued.  If false, the dangling symbolic link
will be silently ignored.

=item C<no_chdir>

Does not C<chdir()> to each directory as it recurses. The C<wanted()>
function will need to be aware of this, of course. In this case,
C<$_> will be the same as C<$File::Find::name>.

=item C<untaint>

If find is used in L<taint-mode|perlsec/Taint mode> (-T command line switch or
if EUID != UID or if EGID != GID), then internally directory names have to be
untainted before they can be C<chdir>'d to. Therefore they are checked against
a regular expression I<untaint_pattern>.  Note that all names passed to the
user's C<wanted()> function are still tainted. If this option is used while not
in taint-mode, C<untaint> is a no-op.

=item C<untaint_pattern>

See above. This should be set using the C<qr> quoting operator.
The default is set to C<qr|^([-+@\w./]+)$|>.
Note that the parentheses are vital.

=item C<untaint_skip>

If set, a directory which fails the I<untaint_pattern> is skipped,
including all its sub-directories. The default is to C<die> in such a case.

=back

=head2 The wanted function

The C<wanted()> function does whatever verifications you want on
each file and directory.  Note that despite its name, the C<wanted()>
function is a generic callback function, and does B<not> tell
File::Find if a file is "wanted" or not.  In fact, its return value
is ignored.

The wanted function takes no arguments but rather does its work
through a collection of variables.

=over 4

=item C<$File::Find::dir> is the current directory name,

=item C<$_> is the current filename within that directory

=item C<$File::Find::name> is the complete pathname to the file.

=back

The above variables have all been localized and may be changed without
affecting data outside of the wanted function.

For example, when examining the file F</some/path/foo.ext> you will have:

    $File::Find::dir  = /some/path/
    $_                = foo.ext
    $File::Find::name = /some/path/foo.ext

You are chdir()'d to C<$File::Find::dir> when the function is called,
unless C<no_chdir> was specified. Note that when changing to
directories is in effect, the root directory (F</>) is a somewhat
special case inasmuch as the concatenation of C<$File::Find::dir>,
C<'/'> and C<$_> is not literally equal to C<$File::Find::name>. The
table below summarizes all variants:

              $File::Find::name  $File::Find::dir  $_
 default      /                  /                 .
 no_chdir=>0  /etc               /                 etc
              /etc/x             /etc              x

 no_chdir=>1  /                  /                 /
              /etc               /                 /etc
              /etc/x             /etc              /etc/x


When C<follow> or C<follow_fast> are in effect, there is
also a C<$File::Find::fullname>.  The function may set
C<$File::Find::prune> to prune the tree unless C<bydepth> was
specified.  Unless C<follow> or C<follow_fast> is specified, for
compatibility reasons (find.pl, find2perl) there are in addition the
following globals available: C<$File::Find::topdir>,
C<$File::Find::topdev>, C<$File::Find::topino>,
C<$File::Find::topmode> and C<$File::Find::topnlink>.

This library is useful for the C<find2perl> tool (distributed as part of the
App-find2perl CPAN distribution), which when fed,

  find2perl / -name .nfs\* -mtime +7 \
    -exec rm -f {} \; -o -fstype nfs -prune

produces something like:

 sub wanted {
    /^\.nfs.*\z/s &&
    (($dev, $ino, $mode, $nlink, $uid, $gid) = lstat($_)) &&
    int(-M _) > 7 &&
    unlink($_)
    ||
    ($nlink || (($dev, $ino, $mode, $nlink, $uid, $gid) = lstat($_))) &&
    $dev < 0 &&
    ($File::Find::prune = 1);
 }

Notice the C<_> in the above C<int(-M _)>: the C<_> is a magical
filehandle that caches the information from the preceding
C<stat()>, C<lstat()>, or filetest.

Here's another interesting wanted function.  It will find all symbolic
links that don't resolve:

    sub wanted {
         -l && !-e && print "bogus link: $File::Find::name\n";
    }

Note that you may mix directories and (non-directory) files in the list of 
directories to be searched by the C<wanted()> function.

    find(\&wanted, "./foo", "./bar", "./baz/epsilon");

In the example above, no file in F<./baz/> other than F<./baz/epsilon> will be
evaluated by C<wanted()>.

See also the script C<pfind> on CPAN for a nice application of this
module.

=head1 WARNINGS

If you run your program with the C<-w> switch, or if you use the
C<warnings> pragma, File::Find will report warnings for several weird
situations. You can disable these warnings by putting the statement

    no warnings 'File::Find';

in the appropriate scope. See L<warnings> for more info about lexical
warnings.

=head1 BUGS AND CAVEATS

=over 4

=item $dont_use_nlink

You can set the variable C<$File::Find::dont_use_nlink> to 0 if you
are sure the filesystem you are scanning reflects the number of
subdirectories in the parent directory's C<nlink> count.

If you do set C<$File::Find::dont_use_nlink> to 0, you may notice an
improvement in speed at the risk of not recursing into subdirectories
if a filesystem doesn't populate C<nlink> as expected.

C<$File::Find::dont_use_nlink> now defaults to 1 on all platforms.

=item symlinks

Be aware that the option to follow symbolic links can be dangerous.
Depending on the structure of the directory tree (including symbolic
links to directories) you might traverse a given (physical) directory
more than once (only if C<follow_fast> is in effect).
Furthermore, deleting or changing files in a symbolically linked directory
might cause very unpleasant surprises, since you delete or change files
in an unknown directory.

=back

=head1 HISTORY

File::Find used to produce incorrect results if called recursively.
During the development of perl 5.8 this bug was fixed.
The first fixed version of File::Find was 1.01.

=head1 SEE ALSO

L<find(1)>, find2perl.

=cut
