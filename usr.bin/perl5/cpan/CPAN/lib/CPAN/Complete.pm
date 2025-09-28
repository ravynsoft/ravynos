# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::Complete;
use strict;
@CPAN::Complete::ISA = qw(CPAN::Debug);
# Q: where is the "How do I add a new command" HOWTO?
# A: git log -p -1 355c44e9caaec857e4b12f51afb96498833c3e36 where andk added the report command
@CPAN::Complete::COMMANDS = sort qw(
                                    ? ! a b d h i m o q r u
                                    autobundle
                                    bye
                                    clean
                                    cvs_import
                                    dump
                                    exit
                                    failed
                                    force
                                    fforce
                                    hosts
                                    install
                                    install_tested
                                    is_tested
                                    look
                                    ls
                                    make
                                    mkmyconfig
                                    notest
                                    perldoc
                                    quit
                                    readme
                                    recent
                                    recompile
                                    reload
                                    report
                                    reports
                                    scripts
                                    smoke
                                    test
                                    upgrade
);

use vars qw(
            $VERSION
);
$VERSION = "5.5001";

package CPAN::Complete;
use strict;

sub gnu_cpl {
    my($text, $line, $start, $end) = @_;
    my(@perlret) = cpl($text, $line, $start);
    # find longest common match. Can anybody show me how to peruse
    # T::R::Gnu to have this done automatically? Seems expensive.
    return () unless @perlret;
    my($newtext) = $text;
    for (my $i = length($text)+1;;$i++) {
        last unless length($perlret[0]) && length($perlret[0]) >= $i;
        my $try = substr($perlret[0],0,$i);
        my @tries = grep {substr($_,0,$i) eq $try} @perlret;
        # warn "try[$try]tries[@tries]";
        if (@tries == @perlret) {
            $newtext = $try;
        } else {
            last;
        }
    }
    ($newtext,@perlret);
}

#-> sub CPAN::Complete::cpl ;
sub cpl {
    my($word,$line,$pos) = @_;
    $word ||= "";
    $line ||= "";
    $pos ||= 0;
    CPAN->debug("word [$word] line[$line] pos[$pos]") if $CPAN::DEBUG;
    $line =~ s/^\s*//;
    if ($line =~ s/^((?:notest|f?force)\s*)//) {
        $pos -= length($1);
    }
    my @return;
    if ($pos == 0 || $line =~ /^(?:h(?:elp)?|\?)\s/) {
        @return = grep /^\Q$word\E/, @CPAN::Complete::COMMANDS;
    } elsif ( $line !~ /^[\!abcdghimorutl]/ ) {
        @return = ();
    } elsif ($line =~ /^a\s/) {
        @return = cplx('CPAN::Author',uc($word));
    } elsif ($line =~ /^ls\s/) {
        my($author,$rest) = $word =~ m|([^/]+)/?(.*)|;
        @return = $rest ? () : map {"$_/"} cplx('CPAN::Author',uc($author||""));
        if (0 && 1==@return) { # XXX too slow and even wrong when there is a * already
            @return = grep /^\Q$word\E/, map {"$author/$_->[2]"} CPAN::Shell->expand("Author",$author)->ls("$rest*","2");
        }
    } elsif ($line =~ /^b\s/) {
        CPAN::Shell->local_bundles;
        @return = cplx('CPAN::Bundle',$word);
    } elsif ($line =~ /^d\s/) {
        @return = cplx('CPAN::Distribution',$word);
    } elsif ($line =~ m/^(
                          [mru]|make|clean|dump|get|test|install|readme|look|cvs_import|perldoc|recent
                         )\s/x ) {
        if ($word =~ /^Bundle::/) {
            CPAN::Shell->local_bundles;
        }
        @return = (cplx('CPAN::Module',$word),cplx('CPAN::Bundle',$word));
    } elsif ($line =~ /^i\s/) {
        @return = cpl_any($word);
    } elsif ($line =~ /^reload\s/) {
        @return = cpl_reload($word,$line,$pos);
    } elsif ($line =~ /^o\s/) {
        @return = cpl_option($word,$line,$pos);
    } elsif ($line =~ m/^\S+\s/ ) {
        # fallback for future commands and what we have forgotten above
        @return = (cplx('CPAN::Module',$word),cplx('CPAN::Bundle',$word));
    } else {
        @return = ();
    }
    return @return;
}

#-> sub CPAN::Complete::cplx ;
sub cplx {
    my($class, $word) = @_;
    if (CPAN::_sqlite_running()) {
        $CPAN::SQLite->search($class, "^\Q$word\E");
    }
    my $method = "id";
    $method = "pretty_id" if $class eq "CPAN::Distribution";
    sort grep /^\Q$word\E/, map { $_->$method() } $CPAN::META->all_objects($class);
}

#-> sub CPAN::Complete::cpl_any ;
sub cpl_any {
    my($word) = shift;
    return (
            cplx('CPAN::Author',$word),
            cplx('CPAN::Bundle',$word),
            cplx('CPAN::Distribution',$word),
            cplx('CPAN::Module',$word),
           );
}

#-> sub CPAN::Complete::cpl_reload ;
sub cpl_reload {
    my($word,$line,$pos) = @_;
    $word ||= "";
    my(@words) = split " ", $line;
    CPAN->debug("word[$word] line[$line] pos[$pos]") if $CPAN::DEBUG;
    my(@ok) = qw(cpan index);
    return @ok if @words == 1;
    return grep /^\Q$word\E/, @ok if @words == 2 && $word;
}

#-> sub CPAN::Complete::cpl_option ;
sub cpl_option {
    my($word,$line,$pos) = @_;
    $word ||= "";
    my(@words) = split " ", $line;
    CPAN->debug("word[$word] line[$line] pos[$pos]") if $CPAN::DEBUG;
    my(@ok) = qw(conf debug);
    return @ok if @words == 1;
    return grep /^\Q$word\E/, @ok if @words == 2 && length($word);
    if (0) {
    } elsif ($words[1] eq 'index') {
        return ();
    } elsif ($words[1] eq 'conf') {
        return CPAN::HandleConfig::cpl(@_);
    } elsif ($words[1] eq 'debug') {
        return sort grep /^\Q$word\E/i,
            sort keys %CPAN::DEBUG, 'all';
    }
}

1;
