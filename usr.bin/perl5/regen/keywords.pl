#!/usr/bin/perl -w
#
# Regenerate (overwriting only if changed):
#
#    keywords.h keywords.c
#
# from information stored in the DATA section of this file.
#
# Accepts the standard regen_lib -q and -v args.

use strict;
use Devel::Tokenizer::C 0.05;

require './regen/regen_lib.pl';

my $h = open_new('keywords.h', '>',
                 { by => 'regen/keywords.pl', from => 'its data',
                   file => 'keywords.h', style => '*',
                   copyright => [1994 .. 1997, 1999 .. 2002, 2005 .. 2007]});
my $c = open_new('keywords.c', '>',
                 { by => 'regen/keywords.pl', from => 'its data', style => '*'});

my %by_strength;

my $keynum = 0;
while (<DATA>) {
    chop;
    next unless $_;
    next if /^#/;
    my ($strength, $keyword) = /^([- +])([A-Z_a-z2]+)/;
    die "Bad line '$_'" unless defined $strength;
    print $h tab(5, "#define KEY_$keyword"), $keynum++, "\n";
    push @{$by_strength{$strength}}, $keyword;
}

# If this hash changes, make sure the equivalent hash in
# lib/B/Deparse.pm (%feature_keywords) is also updated.
my %feature_kw = (
    state     => 'state',
    say       => 'say',
    given     => 'switch',
    when      => 'switch',
    default   => 'switch',
    # continue is already a keyword
    break     => 'switch',
    evalbytes => 'evalbytes',
    __SUB__   => '__SUB__',
    fc        => 'fc',
    isa       => 'isa',
    try       => 'try',
    catch     => 'try',
    finally   => 'try',
    defer     => 'defer',
    class     => 'class',
    field     => 'class',
    method    => 'class',
    ADJUST    => 'class',
);

my %pos = map { ($_ => 1) } @{$by_strength{'+'}};

my $t = Devel::Tokenizer::C->new(TokenFunc     => \&perl_keyword,
                                 TokenString   => 'name',
                                 StringLength  => 'len',
                                 MergeSwitches => 1,
                                );

$t->add_tokens(@{$by_strength{'+'}}, @{$by_strength{'-'}}, 'elseif');

my $switch = $t->generate(Indent => '  ');

print $c <<"END";
#include "EXTERN.h"
#define PERL_IN_KEYWORDS_C
#include "perl.h"
#include "keywords.h"
#include "feature.h"

I32
Perl_keyword (pTHX_ const char *name, I32 len, bool all_keywords)
{
  PERL_ARGS_ASSERT_KEYWORD;

$switch
unknown:
  return 0;
}
END

sub perl_keyword
{
  my $k = shift;
  my $sign = $pos{$k} ? '' : '-';

  if ($k eq 'elseif') {
    return <<END;
Perl_ck_warner_d(aTHX_ packWARN(WARN_SYNTAX), "elseif should be elsif");
END
  }
  elsif (my $feature = $feature_kw{$k}) {
    $feature =~ s/([\\"])/\\$1/g;
    return <<END;
return (all_keywords || FEATURE_\U$feature\E_IS_ENABLED ? ${sign}KEY_$k : 0);
END
  }
  return <<END;
return ${sign}KEY_$k;
END
}

read_only_bottom_close_and_rename($_, [$0]) foreach $c, $h;


# coresub_op in op.c expects __FILE__, __LINE__ and __PACKAGE__ to be the
# first three.

__END__

 NULL
-__FILE__
-__LINE__
-__PACKAGE__
+__DATA__
+__END__
-__SUB__
+ADJUST
+AUTOLOAD
+BEGIN
+UNITCHECK
+DESTROY
+END
+INIT
+CHECK
-abs
-accept
-alarm
-and
-atan2
-bind
-binmode
-bless
-break
-caller
+catch
-chdir
-chmod
-chomp
-chop
-chown
-chr
-chroot
-class
-close
-closedir
-cmp
-connect
-continue
-cos
-crypt
-dbmclose
-dbmopen
+default
+defer
+defined
+delete
-die
+do
-dump
-each
+else
+elsif
-endgrent
-endhostent
-endnetent
-endprotoent
-endpwent
-endservent
-eof
-eq
+eval
-evalbytes
-exec
+exists
-exit
-exp
-fc
-fcntl
-field
-fileno
+finally
-flock
+for
+foreach
-fork
+format
-formline
-ge
-getc
-getgrent
-getgrgid
-getgrnam
-gethostbyaddr
-gethostbyname
-gethostent
-getlogin
-getnetbyaddr
-getnetbyname
-getnetent
-getpeername
-getpgrp
-getppid
-getpriority
-getprotobyname
-getprotobynumber
-getprotoent
-getpwent
-getpwnam
-getpwuid
-getservbyname
-getservbyport
-getservent
-getsockname
-getsockopt
+given
+glob
-gmtime
+goto
+grep
-gt
-hex
+if
-index
-int
-ioctl
-isa
-join
-keys
-kill
+last
-lc
-lcfirst
-le
-length
-link
-listen
+local
-localtime
-lock
-log
-lstat
-lt
+m
+map
-method
-mkdir
-msgctl
-msgget
-msgrcv
-msgsnd
+my
-ne
+next
+no
-not
-oct
-open
-opendir
-or
-ord
+our
-pack
+package
-pipe
-pop
+pos
+print
+printf
+prototype
-push
+q
+qq
+qr
-quotemeta
+qw
+qx
-rand
-read
-readdir
-readline
-readlink
-readpipe
-recv
+redo
-ref
-rename
+require
-reset
+return
-reverse
-rewinddir
-rindex
-rmdir
+s
+say
+scalar
-seek
-seekdir
-select
-semctl
-semget
-semop
-send
-setgrent
-sethostent
-setnetent
-setpgrp
-setpriority
-setprotoent
-setpwent
-setservent
-setsockopt
-shift
-shmctl
-shmget
-shmread
-shmwrite
-shutdown
-sin
-sleep
-socket
-socketpair
+sort
-splice
+split
-sprintf
-sqrt
-srand
-stat
+state
+study
+sub
-substr
-symlink
-syscall
-sysopen
-sysread
-sysseek
-system
-syswrite
-tell
-telldir
-tie
-tied
-time
-times
+tr
+try
-truncate
-uc
-ucfirst
-umask
+undef
+unless
-unlink
-unpack
-unshift
-untie
+until
+use
-utime
-values
-vec
-wait
-waitpid
-wantarray
-warn
+when
+while
-write
-x
-xor
+y
