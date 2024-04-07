#!./perl
BEGIN {
    if ( $^O eq 'VMS' ) {
        my $p = "/bin:/usr/bin:/usr/xpg4/bin:/usr/ucb";
        if ( $ENV{PATH} ) {
            $p .= ":$ENV{PATH}";
        }
        $ENV{PATH} = $p;
    }
    $ENV{LC_ALL} = "C"; # so that external utilities speak English
    $ENV{LANGUAGE} = 'C'; # GNU locale extension

    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( '../lib' );
    skip_all_if_miniperl("no dynamic loading on miniperl, no POSIX");
}

use 5.010;
use strict;
use Config ();
use POSIX ();

skip_all('getgrgid() not implemented')
    unless eval { my($foo) = getgrgid(0); 1 };

skip_all("No 'id' or 'groups'") if
    $^O eq 'MSWin32' || $^O eq 'VMS' || $^O =~ /lynxos/i;

Test();
exit;



sub Test {

    # Get our supplementary groups from the system by running commands
    # like `id -a'.
    my ( $groups_command, $groups_string ) = system_groups()
        or skip_all("No 'id' or 'groups'");
    my @extracted_groups = extract_system_groups( $groups_string )
        or skip_all("Can't parse '${groups_command}'");

    my $pwgid = $( + 0;
    my ($pwgnam) = getgrgid($pwgid);
    $pwgnam //= '';
    note "pwgid=$pwgid pwgnam=$pwgnam \$(=$(";

    # Get perl's supplementary groups by looking at $(
    my ( $gid_count, $all_perl_groups ) = perl_groups();
    my %basegroup = basegroups( $pwgid, $pwgnam );
    my @extracted_supplementary_groups = remove_basegroup( \ %basegroup, \ @extracted_groups );

    plan 4;

    {
        my @warnings = do {
            my @w;
            local $SIG{'__WARN__'} = sub { push @w, @_ };

            use warnings;
            my $v = $( + 1;
            $v = $) + 1;

            @w;
        };

        is ("@warnings", "", 'Neither $( nor $) trigger warnings when used as number.' );
    }

    # Test: The supplementary groups in $( should match the
    # getgroups(2) kernal API call.
    #
    SKIP: {
        my $ngroups_max = posix_ngroups_max();
        if ( defined $ngroups_max && $ngroups_max < @extracted_groups ) {
            # Some OSes (like darwin)but conceivably others might return
            # more groups from `id -a' than can be handled by the
            # kernel. On darwin, NGROUPS_MAX is 16 and 12 are taken up for
            # the system already.
            #
            # There is more fall-out from this than just Perl's unit
            # tests. You may be a member of a group according to Active
            # Directory (or whatever) but the OS won't respect it because
            # it's the 17th (or higher) group and there's no space to
            # store your membership.
            skip "Your platform's `$groups_command' is broken";
        }

        if ( darwin() ) {
            # darwin uses getgrouplist(3) or an Open Directory API within
            # /usr/bin/id and /usr/bin/groups which while "nice" isn't
            # accurate for this test. The hard, real, list of groups we're
            # running in derives from getgroups(2) and is not dynamic but
            # the Libc API getgrouplist(3) is.
            #
            # In practical terms, this meant that while `id -a' can be
            # relied on in other OSes to purely use getgroups(2) and show
            # us what's real, darwin will use getgrouplist(3) to show us
            # what might be real if only we'd open a new console.
            #
            skip "darwin's `${groups_command}' can't be trusted";
        }

        # Read $( but ignore any groups in $( that we failed to parse
        # successfully out of the `id -a` mess.
        #
        my @perl_groups = remove_unparsed_entries( \ @extracted_groups,
                                                   \ @$all_perl_groups );
        my @supplementary_groups = remove_basegroup( \ %basegroup,
                                                     \ @perl_groups );

        my $ok1 = 0;
        if ( match_groups( \ @supplementary_groups,
                           \ @extracted_supplementary_groups,
                           $pwgid ) ) {
            $ok1 = 1;
        }
        elsif ( cygwin_nt() ) {
            %basegroup = unixy_cygwin_basegroups();
            @extracted_supplementary_groups = remove_basegroup( \ %basegroup, \ @extracted_groups );

            if ( match_groups( \ @supplementary_groups,
                               \ @extracted_supplementary_groups,
                               $pwgid ) ) {
                note "This Cygwin behaves like Unix (Win2k?)";
                $ok1 = 1;
            }
        }

        ok $ok1, "perl's `\$(' agrees with `${groups_command}'";
    }

    # multiple 0's indicate GROUPSTYPE is currently long but should be short
    $gid_count->{0} //= 0;
    ok 0 == $pwgid || $gid_count->{0} < 2, "groupstype should be type short, not long";

    SKIP: {
        # try to add a group as supplementary group
        my $root_uid = 0;
        skip "uid!=0", 1 if $< != $root_uid and $> != $root_uid;
        my @groups = split ' ', $);
        my @sup_group;
        setgrent;
        while(my @ent = getgrent) {
            next if grep { $_ == $ent[2] } @groups;
            @sup_group = @ent;
            last;
        }
        endgrent;
        skip "No group found we could add as a supplementary group", 1
            if (!@sup_group);
        $) = "$) $sup_group[2]";
        my $ok = grep { $_ == $sup_group[2] } split ' ', $);
        ok $ok, "Group `$sup_group[0]' added as supplementary group";
    }

    return;
}

# Get the system groups and the command used to fetch them.
#
sub system_groups {
    my ( $cmd, $groups_string ) = _system_groups();

    if ( $groups_string ) {
        chomp $groups_string;
        diag_variable( groups => $groups_string );
    }

    return ( $cmd, $groups_string );
}

# We have to find a command that prints all (effective
# and real) group names (not ids).  The known commands are:
# groups
# id -Gn
# id -a
# Beware 1: some systems do just 'id -G' even when 'id -Gn' is used.
# Beware 2: id -Gn or id -a format might be id(name) or name(id).
# Beware 3: the groups= might be anywhere in the id output.
# Beware 4: groups can have spaces ('id -a' being the only defense against this)
# Beware 5: id -a might not contain the groups= part.
#
# That is, we might meet the following:
#
# foo bar zot				# accept
# foo 22 42 bar zot			# accept
# 1 22 42 2 3				# reject
# groups=(42),foo(1),bar(2),zot me(3)	# parsed by $GROUP_RX1
# groups=22,42,1(foo),2(bar),3(zot(me))	# parsed by $GROUP_RX2
#
# and the groups= might be after, before, or between uid=... and gid=...
use constant GROUP_RX1 => qr/
    ^
    (?<gr_name>.+)
    \(
        (?<gid>\d+)
    \)
    $
/x;
use constant GROUP_RX2 => qr/
    ^
    (?<gid>\d+)
    \(
        (?<gr_name>.+)
    \)
    $
/x;
sub _system_groups {
    my $cmd;
    my $str;

    # prefer 'id' over 'groups' (is this ever wrong anywhere?)
    # and 'id -a' over 'id -Gn' (the former is good about spaces in group names)

    $cmd = 'id -a 2>/dev/null || id 2>/dev/null';
    $str = `$cmd`;
    if ( $str && $str =~ /groups=/ ) {
        # $str is of the form:
        # uid=39957(gsar) gid=22(users) groups=33536,39181,22(users),0(root),1067(dev)
        # FreeBSD since 6.2 has a fake id -a:
        # uid=1001(tobez) gid=20(staff) groups=20(staff), 0(wheel), 68(dialer)
        # On AIX it's id
        #
        # Linux may also have a context= field

        return ( $cmd, $str );
    }

    $cmd = 'id -Gn 2>/dev/null';
    $str = `$cmd`;
    if ( $str && $str !~ /^[\d\s]$/ ) {
        # $str could be of the form:
        # users 33536 39181 root dev
        return ( $cmd, $str );
    }

    $cmd = 'groups 2>/dev/null';
    $str = `$cmd`;
    if ( $str ) {
        # may not reflect all groups in some places, so do a sanity check
        if (-d '/afs') {
            print <<EOM;
# These test results *may* be bogus, as you appear to have AFS,
# and I can't find a working 'id' in your PATH (which I have set
# to '$ENV{PATH}').
#
# If these tests fail, report the particular incantation you use
# on this platform to find *all* the groups that an arbitrary
# user may belong to, using the issue tracker.
EOM
        }
        return ( $cmd, $str );
    }

    return ();
}

# Convert the strings produced by parsing `id -a' into a list of group
# names
sub extract_system_groups {
    my ( $groups_string ) = @_;

    # Remember that group names can contain whitespace, '-', '(parens)',
    # et cetera. That is: do not \w, do not \S.
    my @extracted;

    my @fields = split /\b(\w+=)/, $groups_string;
    my $gr;
    for my $i (0..@fields-2) {
        if ($fields[$i] eq 'groups=') {
            $gr = $fields[$i+1];
            $gr =~ s/ $//;
            last;
        }
    }
    if (defined $gr) {
        my @g = split m{, ?}, $gr;
        # prefer names over numbers
        for (@g) {
            if ( $_ =~ GROUP_RX1() || $_ =~ GROUP_RX2() ) {
                push @extracted, $+{gr_name} || $+{gid};
            }
            else {
                note "ignoring group entry [$_]";
            }
        }

        diag_variable( gr => $gr );
        diag_variable( g => join ',', @g );
        diag_variable( ex_gr => join ',', @extracted );
    }

    return @extracted;
}

# Get the POSIX value NGROUPS_MAX.
sub posix_ngroups_max {
    return eval {
        POSIX::NGROUPS_MAX();
    };
}

# Test if this is Apple's darwin
sub darwin {
    # Observed 'darwin-2level'
    return $Config::Config{myuname} =~ /^darwin/;
}

# Test if this is Cygwin
sub cygwin_nt {
    return $Config::Config{myuname} =~ /^cygwin_nt/i;
}

# Get perl's supplementary groups and the number of times each gid
# appeared.
sub perl_groups {
    # Lookup perl's own groups from $(
    my @gids = split ' ', $(;
    my %gid_count;
    my @gr_name;
    for my $gid ( @gids ) {
        ++ $gid_count{$gid};

        my ($group) = getgrgid $gid;

        # Why does this test prefer to not test groups which we don't have
        # a name for? One possible answer is that my primary group comes
        # from my entry in the user database but isn't mentioned in
        # the group database.  Are there more reasons?
        next if ! defined $group;


        push @gr_name, $group;
    }

    diag_variable( gr_name => join ',', @gr_name );

    return ( \ %gid_count, \ @gr_name );
}

# Remove entries from our parsing of $( that don't appear in our
# parsing of `id -a`.
sub remove_unparsed_entries {
    my ( $extracted_groups, $perl_groups ) = @_;

    my %was_extracted =
        map { $_ => 1 }
        @$extracted_groups;

    return
        grep { $was_extracted{$_} }
        @$perl_groups;
}

# Get a list of base groups. I'm not sure why cygwin by default is
# skipped here.
sub basegroups {
    my ( $pwgid, $pwgnam ) = @_;

    if ( cygwin_nt() ) {
        return;
    }
    else {
        return (
            $pwgid  => 1,
            $pwgnam => 1,
        );
    }
}

# Cygwin might have another form of basegroup which we should actually use
sub unixy_cygwin_basegroups {
    my ( $pwgid, $pwgnam ) = @_;
    return (
        $pwgid  => 1,
        $pwgnam => 1,
    );
}

# Filter a full list of groups and return only the supplementary
# gorups.
sub remove_basegroup {
    my ( $basegroups, $groups ) = @_;

    return
        grep { ! $basegroups->{$_} }
        @$groups;
}

# Test supplementary groups to see if they're a close enough match or
# if there aren't any supplementary groups then validate the current
# group against $(.
sub match_groups {
    my ( $supplementary_groups, $extracted_supplementary_groups, $pwgid ) = @_;

    # Compare perl vs system groups
    my %g;
    $g{$_}[0] = 1 for @$supplementary_groups;
    $g{$_}[1] = 1 for @$extracted_supplementary_groups;

    # Find any mismatches
    my @misses =
        grep { ! ( $g{$_}[0] && $g{$_}[1] ) }
        sort keys %g;

    return
        ! @misses
        || ( ! @$supplementary_groups
             && 1 == @$extracted_supplementary_groups
             && $pwgid == $extracted_supplementary_groups->[0] );
}

# Print a nice little diagnostic.
sub diag_variable {
    my ( $label, $content ) = @_;

    printf "# %-11s=%s\n", $label, $content;
    return;
}

# Removes duplicates from a list
sub uniq {
    my %seen;
    return
        grep { ! $seen{$_}++ }
        @_;
}

# ex: set ts=8 sts=4 sw=4 et:
