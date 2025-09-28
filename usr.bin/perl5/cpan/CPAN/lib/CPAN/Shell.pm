package CPAN::Shell;
use strict;

# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:

use vars qw(
            $ADVANCED_QUERY
            $AUTOLOAD
            $COLOR_REGISTERED
            $Help
            $autoload_recursion
            $reload
            @ISA
            @relo
            $VERSION
           );
@relo =     (
             "CPAN.pm",
             "CPAN/Author.pm",
             "CPAN/CacheMgr.pm",
             "CPAN/Complete.pm",
             "CPAN/Debug.pm",
             "CPAN/DeferredCode.pm",
             "CPAN/Distribution.pm",
             "CPAN/Distroprefs.pm",
             "CPAN/Distrostatus.pm",
             "CPAN/Exception/RecursiveDependency.pm",
             "CPAN/Exception/yaml_not_installed.pm",
             "CPAN/FirstTime.pm",
             "CPAN/FTP.pm",
             "CPAN/FTP/netrc.pm",
             "CPAN/HandleConfig.pm",
             "CPAN/Index.pm",
             "CPAN/InfoObj.pm",
             "CPAN/Kwalify.pm",
             "CPAN/LWP/UserAgent.pm",
             "CPAN/Module.pm",
             "CPAN/Prompt.pm",
             "CPAN/Queue.pm",
             "CPAN/Reporter/Config.pm",
             "CPAN/Reporter/History.pm",
             "CPAN/Reporter/PrereqCheck.pm",
             "CPAN/Reporter.pm",
             "CPAN/Shell.pm",
             "CPAN/SQLite.pm",
             "CPAN/Tarzip.pm",
             "CPAN/Version.pm",
            );
$VERSION = "5.5009";
# record the initial timestamp for reload.
$reload = { map {$INC{$_} ? ($_,(stat $INC{$_})[9]) : ()} @relo };
@CPAN::Shell::ISA = qw(CPAN::Debug);
use Cwd qw(chdir);
use Carp ();
$COLOR_REGISTERED ||= 0;
$Help = {
         '?' => \"help",
         '!' => "eval the rest of the line as perl",
         a => "whois author",
         autobundle => "write inventory into a bundle file",
         b => "info about bundle",
         bye => \"quit",
         clean => "clean up a distribution's build directory",
         # cvs_import
         d => "info about a distribution",
         # dump
         exit => \"quit",
         failed => "list all failed actions within current session",
         fforce => "redo a command from scratch",
         force => "redo a command",
         get => "download a distribution",
         h => \"help",
         help => "overview over commands; 'help ...' explains specific commands",
         hosts => "statistics about recently used hosts",
         i => "info about authors/bundles/distributions/modules",
         install => "install a distribution",
         install_tested => "install all distributions tested OK",
         is_tested => "list all distributions tested OK",
         look => "open a subshell in a distribution's directory",
         ls => "list distributions matching a fileglob",
         m => "info about a module",
         make => "make/build a distribution",
         mkmyconfig => "write current config into a CPAN/MyConfig.pm file",
         notest => "run a (usually install) command but leave out the test phase",
         o => "'o conf ...' for config stuff; 'o debug ...' for debugging",
         perldoc => "try to get a manpage for a module",
         q => \"quit",
         quit => "leave the cpan shell",
         r => "review upgradable modules",
         readme => "display the README of a distro with a pager",
         recent => "show recent uploads to the CPAN",
         # recompile
         reload => "'reload cpan' or 'reload index'",
         report => "test a distribution and send a test report to cpantesters",
         reports => "info about reported tests from cpantesters",
         # scripts
         # smoke
         test => "test a distribution",
         u => "display uninstalled modules",
         upgrade => "combine 'r' command with immediate installation",
        };
{
    $autoload_recursion   ||= 0;

    #-> sub CPAN::Shell::AUTOLOAD ;
    sub AUTOLOAD { ## no critic
        $autoload_recursion++;
        my($l) = $AUTOLOAD;
        my $class = shift(@_);
        # warn "autoload[$l] class[$class]";
        $l =~ s/.*:://;
        if ($CPAN::Signal) {
            warn "Refusing to autoload '$l' while signal pending";
            $autoload_recursion--;
            return;
        }
        if ($autoload_recursion > 1) {
            my $fullcommand = join " ", map { "'$_'" } $l, @_;
            warn "Refusing to autoload $fullcommand in recursion\n";
            $autoload_recursion--;
            return;
        }
        if ($l =~ /^w/) {
            # XXX needs to be reconsidered
            if ($CPAN::META->has_inst('CPAN::WAIT')) {
                CPAN::WAIT->$l(@_);
            } else {
                $CPAN::Frontend->mywarn(qq{
Commands starting with "w" require CPAN::WAIT to be installed.
Please consider installing CPAN::WAIT to use the fulltext index.
For this you just need to type
    install CPAN::WAIT
});
            }
        } else {
            $CPAN::Frontend->mywarn(qq{Unknown shell command '$l'. }.
                                    qq{Type ? for help.
});
        }
        $autoload_recursion--;
    }
}


#-> sub CPAN::Shell::h ;
sub h {
    my($class,$about) = @_;
    if (defined $about) {
        my $help;
        if (exists $Help->{$about}) {
            if (ref $Help->{$about}) { # aliases
                $about = ${$Help->{$about}};
            }
            $help = $Help->{$about};
        } else {
            $help = "No help available";
        }
        $CPAN::Frontend->myprint("$about\: $help\n");
    } else {
        my $filler = " " x (80 - 28 - length($CPAN::VERSION));
        $CPAN::Frontend->myprint(qq{
Display Information $filler (ver $CPAN::VERSION)
 command  argument          description
 a,b,d,m  WORD or /REGEXP/  about authors, bundles, distributions, modules
 i        WORD or /REGEXP/  about any of the above
 ls       AUTHOR or GLOB    about files in the author's directory
    (with WORD being a module, bundle or author name or a distribution
    name of the form AUTHOR/DISTRIBUTION)

Download, Test, Make, Install...
 get      download                     clean    make clean
 make     make (implies get)           look     open subshell in dist directory
 test     make test (implies make)     readme   display these README files
 install  make install (implies test)  perldoc  display POD documentation

Upgrade installed modules
 r        WORDs or /REGEXP/ or NONE    report updates for some/matching/all
 upgrade  WORDs or /REGEXP/ or NONE    upgrade some/matching/all modules

Pragmas
 force  CMD    try hard to do command  fforce CMD    try harder
 notest CMD    skip testing

Other
 h,?           display this menu       ! perl-code   eval a perl command
 o conf [opt]  set and query options   q             quit the cpan shell
 reload cpan   load CPAN.pm again      reload index  load newer indices
 autobundle    Snapshot                recent        latest CPAN uploads});
}
}

*help = \&h;

#-> sub CPAN::Shell::a ;
sub a {
  my($self,@arg) = @_;
  # authors are always UPPERCASE
  for (@arg) {
    $_ = uc $_ unless /=/;
  }
  $CPAN::Frontend->myprint($self->format_result('Author',@arg));
}

#-> sub CPAN::Shell::globls ;
sub globls {
    my($self,$s,$pragmas) = @_;
    # ls is really very different, but we had it once as an ordinary
    # command in the Shell (up to rev. 321) and we could not handle
    # force well then
    my(@accept,@preexpand);
    if ($s =~ /[\*\?\/]/) {
        if ($CPAN::META->has_inst("Text::Glob")) {
            if (my($au,$pathglob) = $s =~ m|(.*?)/(.*)|) {
                my $rau = Text::Glob::glob_to_regex(uc $au);
                CPAN::Shell->debug("au[$au]pathglob[$pathglob]rau[$rau]")
                      if $CPAN::DEBUG;
                push @preexpand, map { $_->id . "/" . $pathglob }
                    CPAN::Shell->expand_by_method('CPAN::Author',['id'],"/$rau/");
            } else {
                my $rau = Text::Glob::glob_to_regex(uc $s);
                push @preexpand, map { $_->id }
                    CPAN::Shell->expand_by_method('CPAN::Author',
                                                  ['id'],
                                                  "/$rau/");
            }
        } else {
            $CPAN::Frontend->mydie("Text::Glob not installed, cannot proceed");
        }
    } else {
        push @preexpand, uc $s;
    }
    for (@preexpand) {
        unless (/^[A-Z0-9\-]+(\/|$)/i) {
            $CPAN::Frontend->mywarn("ls command rejects argument $_: not an author\n");
            next;
        }
        push @accept, $_;
    }
    my $silent = @accept>1;
    my $last_alpha = "";
    my @results;
    for my $a (@accept) {
        my($author,$pathglob);
        if ($a =~ m|(.*?)/(.*)|) {
            my $a2 = $1;
            $pathglob = $2;
            $author = CPAN::Shell->expand_by_method('CPAN::Author',
                                                    ['id'],
                                                    $a2)
                or $CPAN::Frontend->mydie("No author found for $a2\n");
        } else {
            $author = CPAN::Shell->expand_by_method('CPAN::Author',
                                                    ['id'],
                                                    $a)
                or $CPAN::Frontend->mydie("No author found for $a\n");
        }
        if ($silent) {
            my $alpha = substr $author->id, 0, 1;
            my $ad;
            if ($alpha eq $last_alpha) {
                $ad = "";
            } else {
                $ad = "[$alpha]";
                $last_alpha = $alpha;
            }
            $CPAN::Frontend->myprint($ad);
        }
        for my $pragma (@$pragmas) {
            if ($author->can($pragma)) {
                $author->$pragma();
            }
        }
        CPAN->debug("author[$author]pathglob[$pathglob]silent[$silent]") if $CPAN::DEBUG;
        push @results, $author->ls($pathglob,$silent); # silent if
                                                       # more than one
                                                       # author
        for my $pragma (@$pragmas) {
            my $unpragma = "un$pragma";
            if ($author->can($unpragma)) {
                $author->$unpragma();
            }
        }
    }
    @results;
}

#-> sub CPAN::Shell::local_bundles ;
sub local_bundles {
    my($self,@which) = @_;
    my($incdir,$bdir,$dh);
    foreach $incdir ($CPAN::Config->{'cpan_home'},@INC) {
        my @bbase = "Bundle";
        while (my $bbase = shift @bbase) {
            $bdir = File::Spec->catdir($incdir,split /::/, $bbase);
            CPAN->debug("bdir[$bdir]\@bbase[@bbase]") if $CPAN::DEBUG;
            if ($dh = DirHandle->new($bdir)) { # may fail
                my($entry);
                for $entry ($dh->read) {
                    next if $entry =~ /^\./;
                    next unless $entry =~ /^\w+(\.pm)?(?!\n)\Z/;
                    if (-d File::Spec->catdir($bdir,$entry)) {
                        push @bbase, "$bbase\::$entry";
                    } else {
                        next unless $entry =~ s/\.pm(?!\n)\Z//;
                        $CPAN::META->instance('CPAN::Bundle',"$bbase\::$entry");
                    }
                }
            }
        }
    }
}

#-> sub CPAN::Shell::b ;
sub b {
    my($self,@which) = @_;
    CPAN->debug("which[@which]") if $CPAN::DEBUG;
    $self->local_bundles;
    $CPAN::Frontend->myprint($self->format_result('Bundle',@which));
}

#-> sub CPAN::Shell::d ;
sub d { $CPAN::Frontend->myprint(shift->format_result('Distribution',@_));}

#-> sub CPAN::Shell::m ;
sub m { # emacs confused here }; sub mimimimimi { # emacs in sync here
    my $self = shift;
    my @m = @_;
    for (@m) {
        if (m|(?:\w+/)*\w+\.pm$|) { # same regexp in expandany
            s/.pm$//;
            s|/|::|g;
        }
    }
    $CPAN::Frontend->myprint($self->format_result('Module',@m));
}

#-> sub CPAN::Shell::i ;
sub i {
    my($self) = shift;
    my(@args) = @_;
    @args = '/./' unless @args;
    my(@result);
    for my $type (qw/Bundle Distribution Module/) {
        push @result, $self->expand($type,@args);
    }
    # Authors are always uppercase.
    push @result, $self->expand("Author", map { uc $_ } @args);

    my $result = @result == 1 ?
        $result[0]->as_string :
            @result == 0 ?
                "No objects found of any type for argument @args\n" :
                    join("",
                         (map {$_->as_glimpse} @result),
                         scalar @result, " items found\n",
                        );
    $CPAN::Frontend->myprint($result);
}

#-> sub CPAN::Shell::o ;

# CPAN::Shell::o and CPAN::HandleConfig::edit are closely related. 'o
# conf' calls through to CPAN::HandleConfig::edit. 'o conf' should
# probably have been called 'set' and 'o debug' maybe 'set debug' or
# 'debug'; 'o conf ARGS' calls ->edit in CPAN/HandleConfig.pm
sub o {
    my($self,$o_type,@o_what) = @_;
    $o_type ||= "";
    CPAN->debug("o_type[$o_type] o_what[".join(" | ",@o_what)."]\n");
    if ($o_type eq 'conf') {
        my($cfilter);
        ($cfilter) = $o_what[0] =~ m|^/(.*)/$| if @o_what;
        if (!@o_what or $cfilter) { # print all things, "o conf"
            $cfilter ||= "";
            my $qrfilter = eval 'qr/$cfilter/';
            if ($@) {
                $CPAN::Frontend->mydie("Cannot parse commandline: $@");
            }
            my($k,$v);
            my $configpm = CPAN::HandleConfig->require_myconfig_or_config;
            $CPAN::Frontend->myprint("\$CPAN::Config options from $configpm\:\n");
            for $k (sort keys %CPAN::HandleConfig::can) {
                next unless $k =~ /$qrfilter/;
                $v = $CPAN::HandleConfig::can{$k};
                $CPAN::Frontend->myprint(sprintf "    %-18s [%s]\n", $k, $v);
            }
            $CPAN::Frontend->myprint("\n");
            for $k (sort keys %CPAN::HandleConfig::keys) {
                next unless $k =~ /$qrfilter/;
                CPAN::HandleConfig->prettyprint($k);
            }
            $CPAN::Frontend->myprint("\n");
        } else {
            if (CPAN::HandleConfig->edit(@o_what)) {
            } else {
                $CPAN::Frontend->myprint(qq{Type 'o conf' to view all configuration }.
                                         qq{items\n\n});
            }
        }
    } elsif ($o_type eq 'debug') {
        my(%valid);
        @o_what = () if defined $o_what[0] && $o_what[0] =~ /help/i;
        if (@o_what) {
            while (@o_what) {
                my($what) = shift @o_what;
                if ($what =~ s/^-// && exists $CPAN::DEBUG{$what}) {
                    $CPAN::DEBUG &= $CPAN::DEBUG ^ $CPAN::DEBUG{$what};
                    next;
                }
                if ( exists $CPAN::DEBUG{$what} ) {
                    $CPAN::DEBUG |= $CPAN::DEBUG{$what};
                } elsif ($what =~ /^\d/) {
                    $CPAN::DEBUG = $what;
                } elsif (lc $what eq 'all') {
                    my($max) = 0;
                    for (values %CPAN::DEBUG) {
                        $max += $_;
                    }
                    $CPAN::DEBUG = $max;
                } else {
                    my($known) = 0;
                    for (keys %CPAN::DEBUG) {
                        next unless lc($_) eq lc($what);
                        $CPAN::DEBUG |= $CPAN::DEBUG{$_};
                        $known = 1;
                    }
                    $CPAN::Frontend->myprint("unknown argument [$what]\n")
                        unless $known;
                }
            }
        } else {
            my $raw = "Valid options for debug are ".
                join(", ",sort(keys %CPAN::DEBUG), 'all').
                     qq{ or a number. Completion works on the options. }.
                     qq{Case is ignored.};
            require Text::Wrap;
            $CPAN::Frontend->myprint(Text::Wrap::fill("","",$raw));
            $CPAN::Frontend->myprint("\n\n");
        }
        if ($CPAN::DEBUG) {
            $CPAN::Frontend->myprint("Options set for debugging ($CPAN::DEBUG):\n");
            my($k,$v);
            for $k (sort {$CPAN::DEBUG{$a} <=> $CPAN::DEBUG{$b}} keys %CPAN::DEBUG) {
                $v = $CPAN::DEBUG{$k};
                $CPAN::Frontend->myprint(sprintf "    %-14s(%s)\n", $k, $v)
                    if $v & $CPAN::DEBUG;
            }
        } else {
            $CPAN::Frontend->myprint("Debugging turned off completely.\n");
        }
    } else {
        $CPAN::Frontend->myprint(qq{
Known options:
  conf    set or get configuration variables
  debug   set or get debugging options
});
    }
}

# CPAN::Shell::paintdots_onreload
sub paintdots_onreload {
    my($ref) = shift;
    sub {
        if ( $_[0] =~ /[Ss]ubroutine ([\w:]+) redefined/ ) {
            my($subr) = $1;
            ++$$ref;
            local($|) = 1;
            # $CPAN::Frontend->myprint(".($subr)");
            $CPAN::Frontend->myprint(".");
            if ($subr =~ /\bshell\b/i) {
                # warn "debug[$_[0]]";

                # It would be nice if we could detect that a
                # subroutine has actually changed, but for now we
                # practically always set the GOTOSHELL global

                $CPAN::GOTOSHELL=1;
            }
            return;
        }
        warn @_;
    };
}

#-> sub CPAN::Shell::hosts ;
sub hosts {
    my($self) = @_;
    my $fullstats = CPAN::FTP->_ftp_statistics();
    my $history = $fullstats->{history} || [];
    my %S; # statistics
    while (my $last = pop @$history) {
        my $attempts = $last->{attempts} or next;
        my $start;
        if (@$attempts) {
            $start = $attempts->[-1]{start};
            if ($#$attempts > 0) {
                for my $i (0..$#$attempts-1) {
                    my $url = $attempts->[$i]{url} or next;
                    $S{no}{$url}++;
                }
            }
        } else {
            $start = $last->{start};
        }
        next unless $last->{thesiteurl}; # C-C? bad filenames?
        $S{start} = $start;
        $S{end} ||= $last->{end};
        my $dltime = $last->{end} - $start;
        my $dlsize = $last->{filesize} || 0;
        my $url = ref $last->{thesiteurl} ? $last->{thesiteurl}->text : $last->{thesiteurl};
        my $s = $S{ok}{$url} ||= {};
        $s->{n}++;
        $s->{dlsize} ||= 0;
        $s->{dlsize} += $dlsize/1024;
        $s->{dltime} ||= 0;
        $s->{dltime} += $dltime;
    }
    my $res;
    for my $url (sort keys %{$S{ok}}) {
        next if $S{ok}{$url}{dltime} == 0; # div by zero
        push @{$res->{ok}}, [@{$S{ok}{$url}}{qw(n dlsize dltime)},
                             $S{ok}{$url}{dlsize}/$S{ok}{$url}{dltime},
                             $url,
                            ];
    }
    for my $url (sort keys %{$S{no}}) {
        push @{$res->{no}}, [$S{no}{$url},
                             $url,
                            ];
    }
    my $R = ""; # report
    if ($S{start} && $S{end}) {
        $R .= sprintf "Log starts: %s\n", $S{start} ? scalar(localtime $S{start}) : "unknown";
        $R .= sprintf "Log ends  : %s\n", $S{end}   ? scalar(localtime $S{end})   : "unknown";
    }
    if ($res->{ok} && @{$res->{ok}}) {
        $R .= sprintf "\nSuccessful downloads:
   N       kB  secs      kB/s url\n";
        my $i = 20;
        for (sort { $b->[3] <=> $a->[3] } @{$res->{ok}}) {
            $R .= sprintf "%4d %8d %5d %9.1f %s\n", @$_;
            last if --$i<=0;
        }
    }
    if ($res->{no} && @{$res->{no}}) {
        $R .= sprintf "\nUnsuccessful downloads:\n";
        my $i = 20;
        for (sort { $b->[0] <=> $a->[0] } @{$res->{no}}) {
            $R .= sprintf "%4d %s\n", @$_;
            last if --$i<=0;
        }
    }
    $CPAN::Frontend->myprint($R);
}

# here is where 'reload cpan' is done
#-> sub CPAN::Shell::reload ;
sub reload {
    my($self,$command,@arg) = @_;
    $command ||= "";
    $self->debug("self[$self]command[$command]arg[@arg]") if $CPAN::DEBUG;
    if ($command =~ /^cpan$/i) {
        my $redef = 0;
        chdir "$CPAN::iCwd" if $CPAN::iCwd; # may fail
        my $failed;
      MFILE: for my $f (@relo) {
            next unless exists $INC{$f};
            my $p = $f;
            $p =~ s/\.pm$//;
            $p =~ s|/|::|g;
            $CPAN::Frontend->myprint("($p");
            local($SIG{__WARN__}) = paintdots_onreload(\$redef);
            $self->_reload_this($f) or $failed++;
            my $v = eval "$p\::->VERSION";
            $CPAN::Frontend->myprint("v$v)");
        }
        $CPAN::Frontend->myprint("\n$redef subroutines redefined\n");
        if ($failed) {
            my $errors = $failed == 1 ? "error" : "errors";
            $CPAN::Frontend->mywarn("\n$failed $errors during reload. You better quit ".
                                    "this session.\n");
        }
    } elsif ($command =~ /^index$/i) {
      CPAN::Index->force_reload;
    } else {
      $CPAN::Frontend->myprint(qq{cpan     re-evals the CPAN modules
index    re-reads the index files\n});
    }
}

# reload means only load again what we have loaded before
#-> sub CPAN::Shell::_reload_this ;
sub _reload_this {
    my($self,$f,$args) = @_;
    CPAN->debug("f[$f]") if $CPAN::DEBUG;
    return 1 unless $INC{$f}; # we never loaded this, so we do not
                              # reload but say OK
    my $pwd = CPAN::anycwd();
    CPAN->debug("pwd[$pwd]") if $CPAN::DEBUG;
    my($file);
    for my $inc (@INC) {
        $file = File::Spec->catfile($inc,split /\//, $f);
        last if -f $file;
        $file = "";
    }
    CPAN->debug("file[$file]") if $CPAN::DEBUG;
    my @inc = @INC;
    unless ($file && -f $file) {
        # this thingy is not in the INC path, maybe CPAN/MyConfig.pm?
        $file = $INC{$f};
        unless (CPAN->has_inst("File::Basename")) {
            @inc = File::Basename::dirname($file);
        } else {
            # do we ever need this?
            @inc = substr($file,0,-length($f)-1); # bring in back to me!
        }
    }
    CPAN->debug("file[$file]inc[@inc]") if $CPAN::DEBUG;
    unless (-f $file) {
        $CPAN::Frontend->mywarn("Found no file to reload for '$f'\n");
        return;
    }
    my $mtime = (stat $file)[9];
    $reload->{$f} ||= -1;
    my $must_reload = $mtime != $reload->{$f};
    $args ||= {};
    $must_reload ||= $args->{reloforce}; # o conf defaults needs this
    if ($must_reload) {
        my $fh = FileHandle->new($file) or
            $CPAN::Frontend->mydie("Could not open $file: $!");
        my $content;
        {
            local($/);
            local $^W = 1;
            $content = <$fh>;
        }
        CPAN->debug(sprintf("reload file[%s] content[%s...]",$file,substr($content,0,128)))
            if $CPAN::DEBUG;
        my $includefile;
        if ($includefile = $INC{$f} and -e $includefile) {
            $f = $includefile;
        }
        delete $INC{$f};
        local @INC = @inc;
        eval "require '$f'";
        if ($@) {
            warn $@;
            return;
        }
        $reload->{$f} = $mtime;
    } else {
        $CPAN::Frontend->myprint("__unchanged__");
    }
    return 1;
}

#-> sub CPAN::Shell::mkmyconfig ;
sub mkmyconfig {
    my($self) = @_;
    if ( my $configpm = $INC{'CPAN/MyConfig.pm'} ) {
        $CPAN::Frontend->myprint(
            "CPAN::MyConfig already exists as $configpm.\n" .
            "Running configuration again...\n"
        );
        require CPAN::FirstTime;
        CPAN::FirstTime::init($configpm);
    }
    else {
        # force some missing values to be filled in with defaults
        delete $CPAN::Config->{$_}
            for qw/build_dir cpan_home keep_source_where histfile/;
        CPAN::HandleConfig->load( make_myconfig => 1 );
    }
}

#-> sub CPAN::Shell::_binary_extensions ;
sub _binary_extensions {
    my($self) = shift @_;
    my(@result,$module,%seen,%need,$headerdone);
    for $module ($self->expand('Module','/./')) {
        my $file  = $module->cpan_file;
        next if $file eq "N/A";
        next if $file =~ /^Contact Author/;
        my $dist = $CPAN::META->instance('CPAN::Distribution',$file);
        next if $dist->isa_perl;
        next unless $module->xs_file;
        local($|) = 1;
        $CPAN::Frontend->myprint(".");
        push @result, $module;
    }
#    print join " | ", @result;
    $CPAN::Frontend->myprint("\n");
    return @result;
}

#-> sub CPAN::Shell::recompile ;
sub recompile {
    my($self) = shift @_;
    my($module,@module,$cpan_file,%dist);
    @module = $self->_binary_extensions();
    for $module (@module) { # we force now and compile later, so we
                            # don't do it twice
        $cpan_file = $module->cpan_file;
        my $pack = $CPAN::META->instance('CPAN::Distribution',$cpan_file);
        $pack->force;
        $dist{$cpan_file}++;
    }
    for $cpan_file (sort keys %dist) {
        $CPAN::Frontend->myprint("  CPAN: Recompiling $cpan_file\n\n");
        my $pack = $CPAN::META->instance('CPAN::Distribution',$cpan_file);
        $pack->install;
        $CPAN::Signal = 0; # it's tempting to reset Signal, so we can
                           # stop a package from recompiling,
                           # e.g. IO-1.12 when we have perl5.003_10
    }
}

#-> sub CPAN::Shell::scripts ;
sub scripts {
    my($self, $arg) = @_;
    $CPAN::Frontend->mywarn(">>>> experimental command, currently unsupported <<<<\n\n");

    for my $req (qw( HTML::LinkExtor Sort::Versions List::Util )) {
        unless ($CPAN::META->has_inst($req)) {
            $CPAN::Frontend->mywarn("  $req not available\n");
        }
    }
    my $p = HTML::LinkExtor->new();
    my $indexfile = "/home/ftp/pub/PAUSE/scripts/new/index.html";
    unless (-f $indexfile) {
        $CPAN::Frontend->mydie("found no indexfile[$indexfile]\n");
    }
    $p->parse_file($indexfile);
    my @hrefs;
    my $qrarg;
    if ($arg =~ s|^/(.+)/$|$1|) {
        $qrarg = eval 'qr/$arg/'; # hide construct from 5.004
    }
    for my $l ($p->links) {
        my $tag = shift @$l;
        next unless $tag eq "a";
        my %att = @$l;
        my $href = $att{href};
        next unless $href =~ s|^\.\./authors/id/./../||;
        if ($arg) {
            if ($qrarg) {
                if ($href =~ $qrarg) {
                    push @hrefs, $href;
                }
            } else {
                if ($href =~ /\Q$arg\E/) {
                    push @hrefs, $href;
                }
            }
        } else {
            push @hrefs, $href;
        }
    }
    # now filter for the latest version if there is more than one of a name
    my %stems;
    for (sort @hrefs) {
        my $href = $_;
        s/-v?\d.*//;
        my $stem = $_;
        $stems{$stem} ||= [];
        push @{$stems{$stem}}, $href;
    }
    for (sort keys %stems) {
        my $highest;
        if (@{$stems{$_}} > 1) {
            $highest = List::Util::reduce {
                Sort::Versions::versioncmp($a,$b) > 0 ? $a : $b
              } @{$stems{$_}};
        } else {
            $highest = $stems{$_}[0];
        }
        $CPAN::Frontend->myprint("$highest\n");
    }
}

sub _guess_manpage {
    my($self,$d,$contains,$dist) = @_;
    $dist =~ s/-/::/g;
    my $module;
    if (exists $contains->{$dist}) {
        $module = $dist;
    } elsif (1 == keys %$contains) {
        ($module) = keys %$contains;
    }
    my $manpage;
    if ($module) {
        my $m = $self->expand("Module",$module);
        $m->as_string; # called for side-effects, shame
        $manpage = $m->{MANPAGE};
    } else {
        $manpage = "unknown";
    }
    return $manpage;
}

#-> sub CPAN::Shell::_specfile ;
sub _specfile {
    die "CPAN::Shell::_specfile() has been moved to CPAN::Plugin::Specfile::post_test()";
}

#-> sub CPAN::Shell::report ;
sub report {
    my($self,@args) = @_;
    unless ($CPAN::META->has_inst("CPAN::Reporter")) {
        $CPAN::Frontend->mydie("CPAN::Reporter not installed; cannot continue");
    }
    local $CPAN::Config->{test_report} = 1;
    $self->force("test",@args); # force is there so that the test be
                                # re-run (as documented)
}

# compare with is_tested
#-> sub CPAN::Shell::install_tested
sub install_tested {
    my($self,@some) = @_;
    $CPAN::Frontend->mywarn("install_tested() must not be called with arguments.\n"),
        return if @some;
    CPAN::Index->reload;

    for my $b (reverse $CPAN::META->_list_sorted_descending_is_tested) {
        my $yaml = "$b.yml";
        unless (-f $yaml) {
            $CPAN::Frontend->mywarn("No YAML file for $b available, skipping\n");
            next;
        }
        my $yaml_content = CPAN->_yaml_loadfile($yaml);
        my $id = $yaml_content->[0]{distribution}{ID};
        unless ($id) {
            $CPAN::Frontend->mywarn("No ID found in '$yaml', skipping\n");
            next;
        }
        my $do = CPAN::Shell->expandany($id);
        unless ($do) {
            $CPAN::Frontend->mywarn("Could not expand ID '$id', skipping\n");
            next;
        }
        unless ($do->{build_dir}) {
            $CPAN::Frontend->mywarn("Distro '$id' has no build_dir, skipping\n");
            next;
        }
        unless ($do->{build_dir} eq $b) {
            $CPAN::Frontend->mywarn("Distro '$id' has build_dir '$do->{build_dir}' but expected '$b', skipping\n");
            next;
        }
        push @some, $do;
    }

    $CPAN::Frontend->mywarn("No tested distributions found.\n"),
        return unless @some;

    @some = grep { $_->{make_test} && ! $_->{make_test}->failed } @some;
    $CPAN::Frontend->mywarn("No distributions tested with this build of perl found.\n"),
        return unless @some;

    # @some = grep { not $_->uptodate } @some;
    # $CPAN::Frontend->mywarn("No non-uptodate distributions tested with this build of perl found.\n"),
    #     return unless @some;

    CPAN->debug("some[@some]");
    for my $d (@some) {
        my $id = $d->can("pretty_id") ? $d->pretty_id : $d->id;
        $CPAN::Frontend->myprint("install_tested: Running for $id\n");
        $CPAN::Frontend->mysleep(1);
        $self->install($d);
    }
}

#-> sub CPAN::Shell::upgrade ;
sub upgrade {
    my($self,@args) = @_;
    $self->install($self->r(@args));
}

#-> sub CPAN::Shell::_u_r_common ;
sub _u_r_common {
    my($self) = shift @_;
    my($what) = shift @_;
    CPAN->debug("self[$self] what[$what] args[@_]") if $CPAN::DEBUG;
    Carp::croak "Usage: \$obj->_u_r_common(a|r|u)" unless
          $what && $what =~ /^[aru]$/;
    my(@args) = @_;
    @args = '/./' unless @args;
    my(@result,$module,%seen,%need,$headerdone,
       $version_undefs,$version_zeroes,
       @version_undefs,@version_zeroes);
    $version_undefs = $version_zeroes = 0;
    my $sprintf = "%s%-25s%s %9s %9s  %s\n";
    my @expand = $self->expand('Module',@args);
    if ($CPAN::DEBUG) { # Looks like noise to me, was very useful for debugging
             # for metadata cache
        my $expand = scalar @expand;
        $CPAN::Frontend->myprint(sprintf "%d matches in the database, time[%d]\n", $expand, time);
    }
    my @sexpand;
    if ($] < 5.008) {
        # hard to believe that the more complex sorting can lead to
        # stack curruptions on older perl
        @sexpand = sort {$a->id cmp $b->id} @expand;
    } else {
        @sexpand = map {
            $_->[1]
        } sort {
            $b->[0] <=> $a->[0]
            ||
            $a->[1]{ID} cmp $b->[1]{ID},
        } map {
            [$_->_is_representative_module,
             $_
            ]
        } @expand;
    }
    if ($CPAN::DEBUG) {
        $CPAN::Frontend->myprint(sprintf "sorted at time[%d]\n", time);
        sleep 1;
    }
  MODULE: for $module (@sexpand) {
        my $file  = $module->cpan_file;
        next MODULE unless defined $file; # ??
        $file =~ s!^./../!!;
        my($latest) = $module->cpan_version;
        my($inst_file) = $module->inst_file;
        CPAN->debug("file[$file]latest[$latest]") if $CPAN::DEBUG;
        my($have);
        return if $CPAN::Signal;
        my($next_MODULE);
        eval { # version.pm involved!
            if ($inst_file) {
                if ($what eq "a") {
                    $have = $module->inst_version;
                } elsif ($what eq "r") {
                    $have = $module->inst_version;
                    local($^W) = 0;
                    if ($have eq "undef") {
                        $version_undefs++;
                        push @version_undefs, $module->as_glimpse;
                    } elsif (CPAN::Version->vcmp($have,0)==0) {
                        $version_zeroes++;
                        push @version_zeroes, $module->as_glimpse;
                    }
                    ++$next_MODULE unless CPAN::Version->vgt($latest, $have);
                    # to be pedantic we should probably say:
                    #    && !($have eq "undef" && $latest ne "undef" && $latest gt "");
                    # to catch the case where CPAN has a version 0 and we have a version undef
                } elsif ($what eq "u") {
                    ++$next_MODULE;
                }
            } else {
                if ($what eq "a") {
                    ++$next_MODULE;
                } elsif ($what eq "r") {
                    ++$next_MODULE;
                } elsif ($what eq "u") {
                    $have = "-";
                }
            }
        };
        next MODULE if $next_MODULE;
        if ($@) {
            $CPAN::Frontend->mywarn
                (sprintf("Error while comparing cpan/installed versions of '%s':
INST_FILE: %s
INST_VERSION: %s %s
CPAN_VERSION: %s %s
",
                         $module->id,
                         $inst_file || "",
                         (defined $have ? $have : "[UNDEFINED]"),
                         (ref $have ? ref $have : ""),
                         $latest,
                         (ref $latest ? ref $latest : ""),
                        ));
            next MODULE;
        }
        return if $CPAN::Signal; # this is sometimes lengthy
        $seen{$file} ||= 0;
        if ($what eq "a") {
            push @result, sprintf "%s %s\n", $module->id, $have;
        } elsif ($what eq "r") {
            push @result, $module->id;
            next MODULE if $seen{$file}++;
        } elsif ($what eq "u") {
            push @result, $module->id;
            next MODULE if $seen{$file}++;
            next MODULE if $file =~ /^Contact/;
        }
        unless ($headerdone++) {
            $CPAN::Frontend->myprint("\n");
            $CPAN::Frontend->myprint(sprintf(
                                             $sprintf,
                                             "",
                                             "Package namespace",
                                             "",
                                             "installed",
                                             "latest",
                                             "in CPAN file"
                                            ));
        }
        my $color_on = "";
        my $color_off = "";
        if (
            $COLOR_REGISTERED
            &&
            $CPAN::META->has_inst("Term::ANSIColor")
            &&
            $module->description
           ) {
            $color_on = Term::ANSIColor::color("green");
            $color_off = Term::ANSIColor::color("reset");
        }
        $CPAN::Frontend->myprint(sprintf $sprintf,
                                 $color_on,
                                 $module->id,
                                 $color_off,
                                 $have,
                                 $latest,
                                 $file);
        $need{$module->id}++;
    }
    unless (%need) {
        if (!@expand || $what eq "u") {
            $CPAN::Frontend->myprint("No modules found for @args\n");
        } elsif ($what eq "r") {
            $CPAN::Frontend->myprint("All modules are up to date for @args\n");
        }
    }
    if ($what eq "r") {
        if ($version_zeroes) {
            my $s_has = $version_zeroes > 1 ? "s have" : " has";
            $CPAN::Frontend->myprint(qq{$version_zeroes installed module$s_has }.
                                     qq{a version number of 0\n});
            if ($CPAN::Config->{show_zero_versions}) {
                local $" = "\t";
                $CPAN::Frontend->myprint(qq{  they are\n\t@version_zeroes\n});
                $CPAN::Frontend->myprint(qq{(use 'o conf show_zero_versions 0' }.
                                         qq{to hide them)\n});
            } else {
                $CPAN::Frontend->myprint(qq{(use 'o conf show_zero_versions 1' }.
                                         qq{to show them)\n});
            }
        }
        if ($version_undefs) {
            my $s_has = $version_undefs > 1 ? "s have" : " has";
            $CPAN::Frontend->myprint(qq{$version_undefs installed module$s_has no }.
                                     qq{parsable version number\n});
            if ($CPAN::Config->{show_unparsable_versions}) {
                local $" = "\t";
                $CPAN::Frontend->myprint(qq{  they are\n\t@version_undefs\n});
                $CPAN::Frontend->myprint(qq{(use 'o conf show_unparsable_versions 0' }.
                                         qq{to hide them)\n});
            } else {
                $CPAN::Frontend->myprint(qq{(use 'o conf show_unparsable_versions 1' }.
                                         qq{to show them)\n});
            }
        }
    }
    @result;
}

#-> sub CPAN::Shell::r ;
sub r {
    shift->_u_r_common("r",@_);
}

#-> sub CPAN::Shell::u ;
sub u {
    shift->_u_r_common("u",@_);
}

#-> sub CPAN::Shell::failed ;
sub failed {
    my($self,$only_id,$silent) = @_;
    my @failed = $self->find_failed($only_id);
    my $scope;
    if ($only_id) {
        $scope = "this command";
    } elsif ($CPAN::Index::HAVE_REANIMATED) {
        $scope = "this or a previous session";
        # it might be nice to have a section for previous session and
        # a second for this
    } else {
        $scope = "this session";
    }
    if (@failed) {
        my $print;
        my $debug = 0;
        if ($debug) {
            $print = join "",
                map { sprintf "%5d %-45s: %s %s\n", @$_ }
                    sort { $a->[0] <=> $b->[0] } @failed;
        } else {
            $print = join "",
                map { sprintf " %-45s: %s %s\n", @$_[1..3] }
                    sort {
                        $a->[0] <=> $b->[0]
                            ||
                                $a->[4] <=> $b->[4]
                       } @failed;
        }
        $CPAN::Frontend->myprint("Failed during $scope:\n$print");
    } elsif (!$only_id || !$silent) {
        $CPAN::Frontend->myprint("Nothing failed in $scope\n");
    }
}

sub find_failed {
    my($self,$only_id) = @_;
    my @failed;
  DIST: for my $d (sort { $a->id cmp $b->id } $CPAN::META->all_objects("CPAN::Distribution")) {
        my $failed = "";
      NAY: for my $nosayer ( # order matters!
                            "unwrapped",
                            "writemakefile",
                            "signature_verify",
                            "make",
                            "make_test",
                            "install",
                            "make_clean",
                           ) {
            next unless exists $d->{$nosayer};
            next unless defined $d->{$nosayer};
            next unless (
                         UNIVERSAL::can($d->{$nosayer},"failed") ?
                         $d->{$nosayer}->failed :
                         $d->{$nosayer} =~ /^NO/
                        );
            next NAY if $only_id && $only_id != (
                                                 UNIVERSAL::can($d->{$nosayer},"commandid")
                                                 ?
                                                 $d->{$nosayer}->commandid
                                                 :
                                                 $CPAN::CurrentCommandId
                                                );
            $failed = $nosayer;
            last;
        }
        next DIST unless $failed;
        my $id = $d->id;
        $id =~ s|^./../||;
        ### XXX need to flag optional modules as '(optional)' if they are
        # from recommends/suggests -- i.e. *show* failure, but make it clear
        # it was failure of optional module -- xdg, 2012-04-01
        $id = "(optional) $id" if ! $d->{mandatory};
        #$print .= sprintf(
        #                  "  %-45s: %s %s\n",
        push @failed,
            (
             UNIVERSAL::can($d->{$failed},"failed") ?
             [
              $d->{$failed}->commandid,
              $id,
              $failed,
              $d->{$failed}->text,
              $d->{$failed}{TIME}||0,
              !! $d->{mandatory},
             ] :
             [
              1,
              $id,
              $failed,
              $d->{$failed},
              0,
              !! $d->{mandatory},
             ]
            );
    }
    return @failed;
}

sub mandatory_dist_failed {
    my ($self) = @_;
    return grep { $_->[5] } $self->find_failed($CPAN::CurrentCommandID);
}

# XXX intentionally undocumented because completely bogus, unportable,
# useless, etc.

#-> sub CPAN::Shell::status ;
sub status {
    my($self) = @_;
    require Devel::Size;
    my $ps = FileHandle->new;
    open $ps, "/proc/$$/status";
    my $vm = 0;
    while (<$ps>) {
        next unless /VmSize:\s+(\d+)/;
        $vm = $1;
        last;
    }
    $CPAN::Frontend->mywarn(sprintf(
                                    "%-27s %6d\n%-27s %6d\n",
                                    "vm",
                                    $vm,
                                    "CPAN::META",
                                    Devel::Size::total_size($CPAN::META)/1024,
                                   ));
    for my $k (sort keys %$CPAN::META) {
        next unless substr($k,0,4) eq "read";
        warn sprintf " %-26s %6d\n", $k, Devel::Size::total_size($CPAN::META->{$k})/1024;
        for my $k2 (sort keys %{$CPAN::META->{$k}}) {
            warn sprintf "  %-25s %6d (keys: %6d)\n",
                $k2,
                    Devel::Size::total_size($CPAN::META->{$k}{$k2})/1024,
                          scalar keys %{$CPAN::META->{$k}{$k2}};
        }
    }
}

# compare with install_tested
#-> sub CPAN::Shell::is_tested
sub is_tested {
    my($self) = @_;
    CPAN::Index->reload;
    for my $b (reverse $CPAN::META->_list_sorted_descending_is_tested) {
        my $time;
        if ($CPAN::META->{is_tested}{$b}) {
            $time = scalar(localtime $CPAN::META->{is_tested}{$b});
        } else {
            $time = scalar localtime;
            $time =~ s/\S/?/g;
        }
        $CPAN::Frontend->myprint(sprintf "%s %s\n", $time, $b);
    }
}

#-> sub CPAN::Shell::autobundle ;
sub autobundle {
    my($self) = shift;
    CPAN::HandleConfig->load unless $CPAN::Config_loaded++;
    my(@bundle) = $self->_u_r_common("a",@_);
    my($todir) = File::Spec->catdir($CPAN::Config->{'cpan_home'},"Bundle");
    File::Path::mkpath($todir);
    unless (-d $todir) {
        $CPAN::Frontend->myprint("Couldn't mkdir $todir for some reason\n");
        return;
    }
    my($y,$m,$d) =  (localtime)[5,4,3];
    $y+=1900;
    $m++;
    my($c) = 0;
    my($me) = sprintf "Snapshot_%04d_%02d_%02d_%02d", $y, $m, $d, $c;
    my($to) = File::Spec->catfile($todir,"$me.pm");
    while (-f $to) {
        $me = sprintf "Snapshot_%04d_%02d_%02d_%02d", $y, $m, $d, ++$c;
        $to = File::Spec->catfile($todir,"$me.pm");
    }
    my($fh) = FileHandle->new(">$to") or Carp::croak "Can't open >$to: $!";
    $fh->print(
               "package Bundle::$me;\n\n",
               "\$","VERSION = '0.01';\n\n", # hide from perl-reversion
               "1;\n\n",
               "__END__\n\n",
               "=head1 NAME\n\n",
               "Bundle::$me - Snapshot of installation on ",
               $Config::Config{'myhostname'},
               " on ",
               scalar(localtime),
               "\n\n=head1 SYNOPSIS\n\n",
               "perl -MCPAN -e 'install Bundle::$me'\n\n",
               "=head1 CONTENTS\n\n",
               join("\n", @bundle),
               "\n\n=head1 CONFIGURATION\n\n",
               Config->myconfig,
               "\n\n=head1 AUTHOR\n\n",
               "This Bundle has been generated automatically ",
               "by the autobundle routine in CPAN.pm.\n",
              );
    $fh->close;
    $CPAN::Frontend->myprint("\nWrote bundle file
    $to\n\n");
    return $to;
}

#-> sub CPAN::Shell::expandany ;
sub expandany {
    my($self,$s) = @_;
    CPAN->debug("s[$s]") if $CPAN::DEBUG;
    my $module_as_path = "";
    if ($s =~ m|(?:\w+/)*\w+\.pm$|) { # same regexp in sub m
        $module_as_path = $s;
        $module_as_path =~ s/.pm$//;
        $module_as_path =~ s|/|::|g;
    }
    if ($module_as_path) {
        if ($module_as_path =~ m|^Bundle::|) {
            $self->local_bundles;
            return $self->expand('Bundle',$module_as_path);
        } else {
            return $self->expand('Module',$module_as_path)
                if $CPAN::META->exists('CPAN::Module',$module_as_path);
        }
    } elsif ($s =~ m|/| or substr($s,-1,1) eq ".") { # looks like a file or a directory
        $s = CPAN::Distribution->normalize($s);
        return $CPAN::META->instance('CPAN::Distribution',$s);
        # Distributions spring into existence, not expand
    } elsif ($s =~ m|^Bundle::|) {
        $self->local_bundles; # scanning so late for bundles seems
                              # both attractive and crumpy: always
                              # current state but easy to forget
                              # somewhere
        return $self->expand('Bundle',$s);
    } else {
        return $self->expand('Module',$s)
            if $CPAN::META->exists('CPAN::Module',$s);
    }
    return;
}

#-> sub CPAN::Shell::expand ;
sub expand {
    my $self = shift;
    my($type,@args) = @_;
    CPAN->debug("type[$type]args[@args]") if $CPAN::DEBUG;
    my $class = "CPAN::$type";
    my $methods = ['id'];
    for my $meth (qw(name)) {
        next unless $class->can($meth);
        push @$methods, $meth;
    }
    $self->expand_by_method($class,$methods,@args);
}

#-> sub CPAN::Shell::expand_by_method ;
sub expand_by_method {
    my $self = shift;
    my($class,$methods,@args) = @_;
    my($arg,@m);
    for $arg (@args) {
        my($regex,$command);
        if ($arg =~ m|^/(.*)/$|) {
            $regex = $1;
# FIXME:  there seem to be some ='s in the author data, which trigger
#         a failure here.  This needs to be contemplated.
#            } elsif ($arg =~ m/=/) {
#                $command = 1;
        }
        my $obj;
        CPAN->debug(sprintf "class[%s]regex[%s]command[%s]",
                    $class,
                    defined $regex ? $regex : "UNDEFINED",
                    defined $command ? $command : "UNDEFINED",
                   ) if $CPAN::DEBUG;
        if (defined $regex) {
            if (CPAN::_sqlite_running()) {
                CPAN::Index->reload;
                $CPAN::SQLite->search($class, $regex);
            }
            for $obj (
                      $CPAN::META->all_objects($class)
                     ) {
                unless ($obj && UNIVERSAL::can($obj,"id") && $obj->id) {
                    # BUG, we got an empty object somewhere
                    require Data::Dumper;
                    CPAN->debug(sprintf(
                                        "Bug in CPAN: Empty id on obj[%s][%s]",
                                        $obj,
                                        Data::Dumper::Dumper($obj)
                                       )) if $CPAN::DEBUG;
                    next;
                }
                for my $method (@$methods) {
                    my $match = eval {$obj->$method() =~ /$regex/i};
                    if ($@) {
                        my($err) = $@ =~ /^(.+) at .+? line \d+\.$/;
                        $err ||= $@; # if we were too restrictive above
                        $CPAN::Frontend->mydie("$err\n");
                    } elsif ($match) {
                        push @m, $obj;
                        last;
                    }
                }
            }
        } elsif ($command) {
            die "equal sign in command disabled (immature interface), ".
                "you can set
 ! \$CPAN::Shell::ADVANCED_QUERY=1
to enable it. But please note, this is HIGHLY EXPERIMENTAL code
that may go away anytime.\n"
                    unless $ADVANCED_QUERY;
            my($method,$criterion) = $arg =~ /(.+?)=(.+)/;
            my($matchcrit) = $criterion =~ m/^~(.+)/;
            for my $self (
                          sort
                          {$a->id cmp $b->id}
                          $CPAN::META->all_objects($class)
                         ) {
                my $lhs = $self->$method() or next; # () for 5.00503
                if ($matchcrit) {
                    push @m, $self if $lhs =~ m/$matchcrit/;
                } else {
                    push @m, $self if $lhs eq $criterion;
                }
            }
        } else {
            my($xarg) = $arg;
            if ( $class eq 'CPAN::Bundle' ) {
                $xarg =~ s/^(Bundle::)?(.*)/Bundle::$2/;
            } elsif ($class eq "CPAN::Distribution") {
                $xarg = CPAN::Distribution->normalize($arg);
            } else {
                $xarg =~ s/:+/::/g;
            }
            if ($CPAN::META->exists($class,$xarg)) {
                $obj = $CPAN::META->instance($class,$xarg);
            } elsif ($CPAN::META->exists($class,$arg)) {
                $obj = $CPAN::META->instance($class,$arg);
            } else {
                next;
            }
            push @m, $obj;
        }
    }
    @m = sort {$a->id cmp $b->id} @m;
    if ( $CPAN::DEBUG ) {
        my $wantarray = wantarray;
        my $join_m = join ",", map {$_->id} @m;
        # $self->debug("wantarray[$wantarray]join_m[$join_m]");
        my $count = scalar @m;
        $self->debug("class[$class]wantarray[$wantarray]count m[$count]");
    }
    return wantarray ? @m : $m[0];
}

#-> sub CPAN::Shell::format_result ;
sub format_result {
    my($self) = shift;
    my($type,@args) = @_;
    @args = '/./' unless @args;
    my(@result) = $self->expand($type,@args);
    my $result = @result == 1 ?
        $result[0]->as_string :
            @result == 0 ?
                "No objects of type $type found for argument @args\n" :
                    join("",
                         (map {$_->as_glimpse} @result),
                         scalar @result, " items found\n",
                        );
    $result;
}

#-> sub CPAN::Shell::report_fh ;
{
    my $installation_report_fh;
    my $previously_noticed = 0;

    sub report_fh {
        return $installation_report_fh if $installation_report_fh;
        if ($CPAN::META->has_usable("File::Temp")) {
            $installation_report_fh
                = File::Temp->new(
                                  dir      => File::Spec->tmpdir,
                                  template => 'cpan_install_XXXX',
                                  suffix   => '.txt',
                                  unlink   => 0,
                                 );
        }
        unless ( $installation_report_fh ) {
            warn("Couldn't open installation report file; " .
                 "no report file will be generated."
                ) unless $previously_noticed++;
        }
    }
}


# The only reason for this method is currently to have a reliable
# debugging utility that reveals which output is going through which
# channel. No, I don't like the colors ;-)

# to turn colordebugging on, write
# cpan> o conf colorize_output 1

#-> sub CPAN::Shell::colorize_output ;
{
    my $print_ornamented_have_warned = 0;
    sub colorize_output {
        my $colorize_output = $CPAN::Config->{colorize_output};
        if ($colorize_output && $^O eq 'MSWin32' && !$CPAN::META->has_inst("Win32::Console::ANSI")) {
            unless ($print_ornamented_have_warned++) {
                # no myprint/mywarn within myprint/mywarn!
                warn "Colorize_output is set to true but Win32::Console::ANSI is not
installed. To activate colorized output, please install Win32::Console::ANSI.\n\n";
            }
            $colorize_output = 0;
        }
        if ($colorize_output && !$CPAN::META->has_inst("Term::ANSIColor")) {
            unless ($print_ornamented_have_warned++) {
                # no myprint/mywarn within myprint/mywarn!
                warn "Colorize_output is set to true but Term::ANSIColor is not
installed. To activate colorized output, please install Term::ANSIColor.\n\n";
            }
            $colorize_output = 0;
        }
        return $colorize_output;
    }
}


#-> sub CPAN::Shell::print_ornamented ;
sub print_ornamented {
    my($self,$what,$ornament) = @_;
    return unless defined $what;

    local $| = 1; # Flush immediately
    if ( $CPAN::Be_Silent ) {
        # WARNING: variable Be_Silent is poisoned and must be eliminated.
        print {report_fh()} $what;
        return;
    }
    my $swhat = "$what"; # stringify if it is an object
    if ($CPAN::Config->{term_is_latin}) {
        # note: deprecated, need to switch to $LANG and $LC_*
        # courtesy jhi:
        $swhat
            =~ s{([\xC0-\xDF])([\x80-\xBF])}{chr(ord($1)<<6&0xC0|ord($2)&0x3F)}eg; #};
    }
    if ($self->colorize_output) {
        if ( $CPAN::DEBUG && $swhat =~ /^Debug\(/ ) {
            # if you want to have this configurable, please file a bug report
            $ornament = $CPAN::Config->{colorize_debug} || "black on_cyan";
        }
        my $color_on = eval { Term::ANSIColor::color($ornament) } || "";
        if ($@) {
            print "Term::ANSIColor rejects color[$ornament]: $@\n
Please choose a different color (Hint: try 'o conf init /color/')\n";
        }
        # GGOLDBACH/Test-GreaterVersion-0.008 broke without this
        # $trailer construct. We want the newline be the last thing if
        # there is a newline at the end ensuring that the next line is
        # empty for other players
        my $trailer = "";
        $trailer = $1 if $swhat =~ s/([\r\n]+)\z//;
        print $color_on,
            $swhat,
                Term::ANSIColor::color("reset"),
                      $trailer;
    } else {
        print $swhat;
    }
}

#-> sub CPAN::Shell::myprint ;

# where is myprint/mywarn/Frontend/etc. documented? Where to use what?
# I think, we send everything to STDOUT and use print for normal/good
# news and warn for news that need more attention. Yes, this is our
# working contract for now.
sub myprint {
    my($self,$what) = @_;
    $self->print_ornamented($what,
                            $CPAN::Config->{colorize_print}||'bold blue on_white',
                           );
}

my %already_printed;
#-> sub CPAN::Shell::mywarnonce ;
sub myprintonce {
    my($self,$what) = @_;
    $self->myprint($what) unless $already_printed{$what}++;
}

sub optprint {
    my($self,$category,$what) = @_;
    my $vname = $category . "_verbosity";
    CPAN::HandleConfig->load unless $CPAN::Config_loaded++;
    if (!$CPAN::Config->{$vname}
        || $CPAN::Config->{$vname} =~ /^v/
       ) {
        $CPAN::Frontend->myprint($what);
    }
}

#-> sub CPAN::Shell::myexit ;
sub myexit {
    my($self,$what) = @_;
    $self->myprint($what);
    exit;
}

#-> sub CPAN::Shell::mywarn ;
sub mywarn {
    my($self,$what) = @_;
    $self->print_ornamented($what, $CPAN::Config->{colorize_warn}||'bold red on_white');
}

my %already_warned;
#-> sub CPAN::Shell::mywarnonce ;
sub mywarnonce {
    my($self,$what) = @_;
    $self->mywarn($what) unless $already_warned{$what}++;
}

# only to be used for shell commands
#-> sub CPAN::Shell::mydie ;
sub mydie {
    my($self,$what) = @_;
    $self->mywarn($what);

    # If it is the shell, we want the following die to be silent,
    # but if it is not the shell, we would need a 'die $what'. We need
    # to take care that only shell commands use mydie. Is this
    # possible?

    die "\n";
}

# sub CPAN::Shell::colorable_makemaker_prompt ;
sub colorable_makemaker_prompt {
    my($foo,$bar,$ornament) = @_;
    $ornament ||= "colorize_print";
    if (CPAN::Shell->colorize_output) {
        my $ornament = $CPAN::Config->{$ornament}||'bold blue on_white';
        my $color_on = eval { Term::ANSIColor::color($ornament); } || "";
        print $color_on;
    }
    my $ans = ExtUtils::MakeMaker::prompt($foo,$bar);
    if (CPAN::Shell->colorize_output) {
        print Term::ANSIColor::color('reset');
    }
    return $ans;
}

# use this only for unrecoverable errors!
#-> sub CPAN::Shell::unrecoverable_error ;
sub unrecoverable_error {
    my($self,$what) = @_;
    my @lines = split /\n/, $what;
    my $longest = 0;
    for my $l (@lines) {
        $longest = length $l if length $l > $longest;
    }
    $longest = 62 if $longest > 62;
    for my $l (@lines) {
        if ($l =~ /^\s*$/) {
            $l = "\n";
            next;
        }
        $l = "==> $l";
        if (length $l < 66) {
            $l = pack "A66 A*", $l, "<==";
        }
        $l .= "\n";
    }
    unshift @lines, "\n";
    $self->mydie(join "", @lines);
}

#-> sub CPAN::Shell::mysleep ;
sub mysleep {
    return if $ENV{AUTOMATED_TESTING} || ! -t STDOUT;
    my($self, $sleep) = @_;
    if (CPAN->has_inst("Time::HiRes")) {
        Time::HiRes::sleep($sleep);
    } else {
        sleep($sleep < 1 ? 1 : int($sleep + 0.5));
    }
}

#-> sub CPAN::Shell::setup_output ;
sub setup_output {
    return if -t STDOUT;
    my $odef = select STDERR;
    $| = 1;
    select STDOUT;
    $| = 1;
    select $odef;
}

#-> sub CPAN::Shell::rematein ;
# RE-adme||MA-ke||TE-st||IN-stall : nearly everything runs through here
sub rematein {
    my $self = shift;
    # this variable was global and disturbed programmers, so localize:
    local $CPAN::Distrostatus::something_has_failed_at;
    my($meth,@some) = @_;
    my @pragma;
    while($meth =~ /^(ff?orce|notest)$/) {
        push @pragma, $meth;
        $meth = shift @some or
            $CPAN::Frontend->mydie("Pragma $pragma[-1] used without method: ".
                                   "cannot continue");
    }
    setup_output();
    CPAN->debug("pragma[@pragma]meth[$meth]some[@some]") if $CPAN::DEBUG;

    # Here is the place to set "test_count" on all involved parties to
    # 0. We then can pass this counter on to the involved
    # distributions and those can refuse to test if test_count > X. In
    # the first stab at it we could use a 1 for "X".

    # But when do I reset the distributions to start with 0 again?
    # Jost suggested to have a random or cycling interaction ID that
    # we pass through. But the ID is something that is just left lying
    # around in addition to the counter, so I'd prefer to set the
    # counter to 0 now, and repeat at the end of the loop. But what
    # about dependencies? They appear later and are not reset, they
    # enter the queue but not its copy. How do they get a sensible
    # test_count?

    # With configure_requires, "get" is vulnerable in recursion.

    my $needs_recursion_protection = "get|make|test|install";

    # construct the queue
    my($s,@s,@qcopy);
  STHING: foreach $s (@some) {
        my $obj;
        if (ref $s) {
            CPAN->debug("s is an object[$s]") if $CPAN::DEBUG;
            $obj = $s;
        } elsif ($s =~ m|[\$\@\%]|) { # looks like a perl variable
        } elsif ($s =~ m|^/|) { # looks like a regexp
            if (substr($s,-1,1) eq ".") {
                $obj = CPAN::Shell->expandany($s);
            } else {
                my @obj;
            CLASS: for my $class (qw(Distribution Bundle Module)) {
                    if (@obj = $self->expand($class,$s)) {
                        last CLASS;
                    }
                }
                if (@obj) {
                    if (1==@obj) {
                        $obj = $obj[0];
                    } else {
                        $CPAN::Frontend->mywarn("Sorry, $meth with a regular expression is ".
                                                "only supported when unambiguous.\nRejecting argument '$s'\n");
                        $CPAN::Frontend->mysleep(2);
                        next STHING;
                    }
                }
            }
        } elsif ($meth eq "ls") {
            $self->globls($s,\@pragma);
            next STHING;
        } else {
            CPAN->debug("calling expandany [$s]") if $CPAN::DEBUG;
            $obj = CPAN::Shell->expandany($s);
        }
        if (0) {
        } elsif (ref $obj) {
            if ($meth =~ /^($needs_recursion_protection)$/) {
                # it would be silly to check for recursion for look or dump
                # (we are in CPAN::Shell::rematein)
                CPAN->debug("Testing against recursion") if $CPAN::DEBUG;
                eval {  $obj->color_cmd_tmps(0,1); };
                if ($@) {
                    if (ref $@
                        and $@->isa("CPAN::Exception::RecursiveDependency")) {
                        $CPAN::Frontend->mywarn($@);
                    } else {
                        if (0) {
                            require Carp;
                            Carp::confess(sprintf "DEBUG: \$\@[%s]ref[%s]", $@, ref $@);
                        }
                        die;
                    }
                }
            }
            CPAN::Queue->queue_item(qmod => $obj->id, reqtype => "c", optional => '');
            push @qcopy, $obj;
        } elsif ($CPAN::META->exists('CPAN::Author',uc($s))) {
            $obj = $CPAN::META->instance('CPAN::Author',uc($s));
            if ($meth =~ /^(dump|ls|reports)$/) {
                $obj->$meth();
            } else {
                $CPAN::Frontend->mywarn(
                                        join "",
                                        "Don't be silly, you can't $meth ",
                                        $obj->fullname,
                                        " ;-)\n"
                                       );
                $CPAN::Frontend->mysleep(2);
            }
        } elsif ($s =~ m|[\$\@\%]| && $meth eq "dump") {
            CPAN::InfoObj->dump($s);
        } else {
            $CPAN::Frontend
                ->mywarn(qq{Warning: Cannot $meth $s, }.
                         qq{don't know what it is.
Try the command

    i /$s/

to find objects with matching identifiers.
});
            $CPAN::Frontend->mysleep(2);
        }
    }

    # queuerunner (please be warned: when I started to change the
    # queue to hold objects instead of names, I made one or two
    # mistakes and never found which. I reverted back instead)
  QITEM: while (my $q = CPAN::Queue->first) {
        my $obj;
        my $s = $q->as_string;
        my $reqtype = $q->reqtype || "";
        my $optional = $q->optional || "";
        $obj = CPAN::Shell->expandany($s);
        unless ($obj) {
            # don't know how this can happen, maybe we should panic,
            # but maybe we get a solution from the first user who hits
            # this unfortunate exception?
            $CPAN::Frontend->mywarn("Warning: Could not expand string '$s' ".
                                    "to an object. Skipping.\n");
            $CPAN::Frontend->mysleep(5);
            CPAN::Queue->delete_first($s);
            next QITEM;
        }
        $obj->{reqtype} ||= "";
        my $type = ref $obj;
        if ( $type eq 'CPAN::Distribution' || $type eq 'CPAN::Bundle' ) {
            $obj->{mandatory} ||= ! $optional; # once mandatory, always mandatory
        }
        elsif ( $type eq 'CPAN::Module' ) {
            $obj->{mandatory} ||= ! $optional; # once mandatory, always mandatory
            if (my $d = $obj->distribution) {
                $d->{mandatory} ||= ! $optional; # once mandatory, always mandatory
            } elsif ($optional) {
                # the queue object does not know who was recommending/suggesting us:(
                # So we only vaguely write "optional".
                $CPAN::Frontend->mywarn("Warning: optional module '$s' ".
                                        "not known. Skipping.\n");
                CPAN::Queue->delete_first($s);
                next QITEM;
            }
        }
        {
            # force debugging because CPAN::SQLite somehow delivers us
            # an empty object;

            # local $CPAN::DEBUG = 1024; # Shell; probably fixed now

            CPAN->debug("s[$s]obj-reqtype[$obj->{reqtype}]".
                        "q-reqtype[$reqtype]") if $CPAN::DEBUG;
        }
        if ($obj->{reqtype}) {
            if ($obj->{reqtype} eq "b" && $reqtype =~ /^[rc]$/) {
                $obj->{reqtype} = $reqtype;
                if (
                    exists $obj->{install}
                    &&
                    (
                     UNIVERSAL::can($obj->{install},"failed") ?
                     $obj->{install}->failed :
                     $obj->{install} =~ /^NO/
                    )
                   ) {
                    delete $obj->{install};
                    $CPAN::Frontend->mywarn
                        ("Promoting $obj->{ID} from 'build_requires' to 'requires'");
                }
            }
        } else {
            $obj->{reqtype} = $reqtype;
        }

        for my $pragma (@pragma) {
            if ($pragma
                &&
                $obj->can($pragma)) {
                $obj->$pragma($meth);
            }
        }
        if (UNIVERSAL::can($obj, 'called_for')) {
            $obj->called_for($s) unless $obj->called_for;
        }
        CPAN->debug(qq{pragma[@pragma]meth[$meth]}.
                    qq{ID[$obj->{ID}]}) if $CPAN::DEBUG;

        push @qcopy, $obj;
        if ($meth =~ /^(report)$/) { # they came here with a pragma?
            $self->$meth($obj);
        } elsif (! UNIVERSAL::can($obj,$meth)) {
            # Must never happen
            my $serialized = "";
            if (0) {
            } elsif ($CPAN::META->has_inst("YAML::Syck")) {
                $serialized = YAML::Syck::Dump($obj);
            } elsif ($CPAN::META->has_inst("YAML")) {
                $serialized = YAML::Dump($obj);
            } elsif ($CPAN::META->has_inst("Data::Dumper")) {
                $serialized = Data::Dumper::Dumper($obj);
            } else {
                require overload;
                $serialized = overload::StrVal($obj);
            }
            CPAN->debug("Going to panic. meth[$meth]s[$s]") if $CPAN::DEBUG;
            $CPAN::Frontend->mydie("Panic: obj[$serialized] cannot meth[$meth]");
        } else {
            my $upgraded_meth = $meth;
            if ( $meth eq "make" and $obj->{reqtype} eq "b" ) {
                # rt 86915
                $upgraded_meth = "test";
            }
            if ($obj->$upgraded_meth()) {
                CPAN::Queue->delete($s);
                CPAN->debug("Succeeded and deleted from queue. pragma[@pragma]meth[$meth][s][$s]") if $CPAN::DEBUG;
            } else {
                CPAN->debug("Failed. pragma[@pragma]meth[$meth]s[$s]") if $CPAN::DEBUG;
            }
        }

        $obj->undelay;
        for my $pragma (@pragma) {
            my $unpragma = "un$pragma";
            if ($obj->can($unpragma)) {
                $obj->$unpragma();
            }
        }
        # if any failures occurred and the current object is mandatory, we
        # still don't know if *it* failed or if it was another (optional)
        # module, so we have to check that explicitly (and expensively)
        if (    $CPAN::Config->{halt_on_failure}
            && $obj->{mandatory}
            && CPAN::Distrostatus::something_has_just_failed()
            && $self->mandatory_dist_failed()
        ) {
            $CPAN::Frontend->mywarn("Stopping: '$meth' failed for '$s'.\n");
            CPAN::Queue->nullify_queue;
            last QITEM;
        }
        CPAN::Queue->delete_first($s);
    }
    if ($meth =~ /^($needs_recursion_protection)$/) {
        for my $obj (@qcopy) {
            $obj->color_cmd_tmps(0,0);
        }
    }
}

#-> sub CPAN::Shell::recent ;
sub recent {
  my($self) = @_;
  if ($CPAN::META->has_inst("XML::LibXML")) {
      my $url = $CPAN::Defaultrecent;
      $CPAN::Frontend->myprint("Fetching '$url'\n");
      unless ($CPAN::META->has_usable("LWP")) {
          $CPAN::Frontend->mydie("LWP not installed; cannot continue");
      }
      CPAN::LWP::UserAgent->config;
      my $Ua;
      eval { $Ua = CPAN::LWP::UserAgent->new; };
      if ($@) {
          $CPAN::Frontend->mydie("CPAN::LWP::UserAgent->new dies with $@\n");
      }
      my $resp = $Ua->get($url);
      unless ($resp->is_success) {
          $CPAN::Frontend->mydie(sprintf "Could not download '%s': %s\n", $url, $resp->code);
      }
      $CPAN::Frontend->myprint("DONE\n\n");
      my $xml = XML::LibXML->new->parse_string($resp->content);
      if (0) {
          my $s = $xml->serialize(2);
          $s =~ s/\n\s*\n/\n/g;
          $CPAN::Frontend->myprint($s);
          return;
      }
      my @distros;
      if ($url =~ /winnipeg/) {
          my $pubdate = $xml->findvalue("/rss/channel/pubDate");
          $CPAN::Frontend->myprint("    pubDate: $pubdate\n\n");
          for my $eitem ($xml->findnodes("/rss/channel/item")) {
              my $distro = $eitem->findvalue("enclosure/\@url");
              $distro =~ s|.*?/authors/id/./../||;
              my $size   = $eitem->findvalue("enclosure/\@length");
              my $desc   = $eitem->findvalue("description");
              $desc =~ s/.+? - //;
              $CPAN::Frontend->myprint("$distro [$size b]\n    $desc\n");
              push @distros, $distro;
          }
      } elsif ($url =~ /search.*uploads.rdf/) {
          # xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
          # xmlns="http://purl.org/rss/1.0/"
          # xmlns:taxo="http://purl.org/rss/1.0/modules/taxonomy/"
          # xmlns:dc="http://purl.org/dc/elements/1.1/"
          # xmlns:syn="http://purl.org/rss/1.0/modules/syndication/"
          # xmlns:admin="http://webns.net/mvcb/"


          my $dc_date = $xml->findvalue("//*[local-name(.) = 'RDF']/*[local-name(.) = 'channel']/*[local-name(.) = 'date']");
          $CPAN::Frontend->myprint("    dc:date: $dc_date\n\n");
          my $finish_eitem = 0;
          local $SIG{INT} = sub { $finish_eitem = 1 };
        EITEM: for my $eitem ($xml->findnodes("//*[local-name(.) = 'RDF']/*[local-name(.) = 'item']")) {
              my $distro = $eitem->findvalue("\@rdf:about");
              $distro =~ s|.*~||; # remove up to the tilde before the name
              $distro =~ s|/$||; # remove trailing slash
              $distro =~ s|([^/]+)|\U$1\E|; # upcase the name
              my $author = uc $1 or die "distro[$distro] without author, cannot continue";
              my $desc   = $eitem->findvalue("*[local-name(.) = 'description']");
              my $i = 0;
            SUBDIRTEST: while () {
                  last SUBDIRTEST if ++$i >= 6; # half a dozen must do!
                  if (my @ret = $self->globls("$distro*")) {
                      @ret = grep {$_->[2] !~ /meta/} @ret;
                      @ret = grep {length $_->[2]} @ret;
                      if (@ret) {
                          $distro = "$author/$ret[0][2]";
                          last SUBDIRTEST;
                      }
                  }
                  $distro =~ s|/|/*/|; # allow it to reside in a subdirectory
              }

              next EITEM if $distro =~ m|\*|; # did not find the thing
              $CPAN::Frontend->myprint("____$desc\n");
              push @distros, $distro;
              last EITEM if $finish_eitem;
          }
      }
      return \@distros;
  } else {
      # deprecated old version
      $CPAN::Frontend->mydie("no XML::LibXML installed, cannot continue\n");
  }
}

#-> sub CPAN::Shell::smoke ;
sub smoke {
    my($self) = @_;
    my $distros = $self->recent;
  DISTRO: for my $distro (@$distros) {
        next if $distro =~ m|/Bundle-|; # XXX crude heuristic to skip bundles
        $CPAN::Frontend->myprint(sprintf "Downloading and testing '$distro'\n");
        {
            my $skip = 0;
            local $SIG{INT} = sub { $skip = 1 };
            for (0..9) {
                $CPAN::Frontend->myprint(sprintf "\r%2d (Hit ^C to skip)", 10-$_);
                sleep 1;
                if ($skip) {
                    $CPAN::Frontend->myprint(" skipped\n");
                    next DISTRO;
                }
            }
        }
        $CPAN::Frontend->myprint("\r  \n"); # leave the dirty line with a newline
        $self->test($distro);
    }
}

{
    # set up the dispatching methods
    no strict "refs";
    for my $command (qw(
                        clean
                        cvs_import
                        dump
                        force
                        fforce
                        get
                        install
                        look
                        ls
                        make
                        notest
                        perldoc
                        readme
                        reports
                        test
                       )) {
        *$command = sub { shift->rematein($command, @_); };
    }
}

1;
