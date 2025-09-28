#
# $Id: Makefile.PL,v 2.25 2022/06/25 01:58:57 dankogai Exp $
#
use 5.007003;
use strict;
use warnings;
use utf8;
use ExtUtils::MakeMaker;
use File::Spec;
use Config;

# Just for sure :)
my %ARGV = map { my @r = split /=/,$_; defined $r[1] or $r[1]=1; @r } @ARGV;
$ARGV{DEBUG} and warn "$_ => $ARGV{$_}\n" for sort keys  %ARGV;
$ENV{PERL_CORE} ||= $ARGV{PERL_CORE} if $ARGV{PERL_CORE};
# similar strictness as in core
my $ccflags = $Config{ccflags};
if (!$ENV{PERL_CORE}) {
  if (my $gccver = $Config{gccversion}) {
    # no more 'Argument "Apple" isn't numeric in addition (+)'
    no warnings 'numeric';
    $gccver =~ s/\.//g; $gccver =~ s/ .*//;
    $gccver .= "0" while length $gccver < 3;
    $gccver = 0+$gccver;
    $ccflags .= ' -Werror=declaration-after-statement' if $gccver > 412 and $] < 5.035005;
    $ccflags .= ' -Wpointer-sign' if !$Config{d_cplusplus} and $gccver > 400;
    $ccflags .= ' -fpermissive' if $Config{d_cplusplus};
  }
}

my %tables = 
    (
     def_t => [
           'ascii.ucm',
           '8859-1.ucm',
           'cp1252.ucm',
           'null.ucm',
           'ctrl.ucm',
           ]
     );

my @exe_files = qw(bin/enc2xs
           bin/piconv
           bin/encguess
           );
my @more_exe_files = qw(
            unidump
            );
my @pmlibdirs = qw(lib Encode);

$ARGV{MORE_SCRIPTS} and push @exe_files, @more_exe_files;
$ARGV{INSTALL_UCM}   and push @pmlibdirs, "ucm";

WriteMakefile(
    NAME         => "Encode",
    EXE_FILES    => \@exe_files,
    VERSION_FROM => 'Encode.pm',
    ABSTRACT_FROM=> 'Encode.pm',
    AUTHOR       => 'Dan Kogai <dankogai@dan.co.jp>',
    OBJECT       => '$(O_FILES)',
    'dist'       => {
        COMPRESS     => 'gzip -9f',
        SUFFIX       => 'gz',
        DIST_DEFAULT => 'all tardist',
    },
    CCFLAGS     => $ccflags,
    INC         => '-I' . File::Spec->catfile( '.', 'Encode' ),
    LICENSE     => 'perl',
    PREREQ_PM   => {
        Exporter   => '5.57',   # use Exporter 'import';
	parent     => '0.221',  # version bundled with 5.10.1
        Storable   => '0',      # bundled with Perl 5.7.3
    },
    TEST_REQUIRES => {
        'Test::More' => '0.92',
    },
    PMLIBDIRS   => \@pmlibdirs,
    INSTALLDIRS => ($] < 5.011 ? 'perl' : 'site'),
    META_MERGE        => {
        resources => {
            repository  =>  'https://github.com/dankogai/p5-encode',
        },
        x_contributors => [
            'Alex Davies <alex.davies@talktalk.net>',
            'Alex Kapranoff <alex@kapranoff.ru>',
            'Alex Vandiver <alex@chmrr.net>',
            'Andreas J. Koenig <andreas.koenig@anima.de>',
            'Andrew Pennebaker <andrew.pennebaker@networkedinsights.com>',
            'Andy Grundman <andyg@activestate.com>',
            'Anton Tagunov <tagunov@motor.ru>',
            'Autrijus Tang <autrijus@autrijus.org>',
            'Benjamin Goldberg <goldbb2@earthlink.net>',
            'Bjoern Hoehrmann <derhoermi@gmx.net>',
            'Bjoern Jacke <debianbugs@j3e.de>',
            'bulk88 <bulk88@hotmail.com>',
            'Craig A. Berry <craigberry@mac.com>',
            'Curtis Jewell <csjewell@cpan.org>',
            'Dan Kogai <dankogai@dan.co.jp>',
            'Dave Evans <dave@rudolf.org.uk>',
            'David Golden <dagolden@cpan.org>',
            'David Steinbrunner <dsteinbrunner@pobox.com>',
            'Deng Liu <dengliu@ntu.edu.tw>',
            'Dominic Dunlop <domo@computer.org>',
            'drry',
            'Elizabeth Mattijsen <liz@dijkmat.nl>',
            'Flavio Poletti <flavio@polettix.it>',
            'Gerrit P. Haase <gp@familiehaase.de>',
            'Gisle Aas <gisle@ActiveState.com>',
            'Graham Barr <gbarr@pobox.com>',
            'Graham Knop <haarg@haarg.org>',
            'Graham Ollis <perl@wdlabs.com>',
            'Gurusamy Sarathy <gsar@activestate.com>',
            'H.Merijn Brand <h.m.brand@xs4all.nl>',
            'Hugo van der Sanden <hv@crypt.org>',
            'chansen <chansen@cpan.org>',
            'Chris Nandor <pudge@pobox.com>',
            'Inaba Hiroto <inaba@st.rim.or.jp>',
            'Jarkko Hietaniemi <jhi@iki.fi>',
            'Jesse Vincent <jesse@fsck.com>',
            'Jungshik Shin <jshin@mailaps.org>',
            'Karen Etheridge <ether@cpan.org>',
            'Karl Williamson <khw@cpan.org>',
            'Kenichi Ishigaki <ishigaki@cpan.org>',
            'KONNO Hiroharu <hiroharu.konno@bowneglobal.co.jp>',
            'Laszlo Molnar <ml1050@freemail.hu>',
            'Makamaka <makamaka@donzoko.net>',
            'Mark-Jason Dominus <mjd@plover.com>',
            'Masahiro Iuchi <masahiro.iuchi@gmail.com>',
            'MATSUNO Tokuhiro <tokuhirom+cpan@gmail.com>',
            'Mattia Barbon <mbarbon@dsi.unive.it>',
            'Michael G Schwern <schwern@pobox.com>',
            'Michael LaGrasta <michael@lagrasta.com>',
            'Miron Cuperman <miron@hyper.to>',
            'Moritz Lenz <moritz@faui2k3.org>',
            'MORIYAMA Masayuki <msyk@mtg.biglobe.ne.jp>',
            'Nick Ing-Simmons <nick@ing-simmons.net>',
            'Nicholas Clark <nick@ccl4.org>',
            'Olivier Mengué <dolmen@cpan.org>',
            'otsune',
            'Pali <pali@cpan.org>',
            'Paul Marquess <paul_marquess@yahoo.co.uk>',
            'Peter Prymmer <pvhp@best.com>',
            'Peter Rabbitson <ribasushi@cpan.org>',
            'Philip Newton <pne@cpan.org>',
            'Piotr Fusik <pfusik@op.pl>',
            'Rafael Garcia-Suarez <rgarciasuarez@mandriva.com>',
            'Randy Stauner <randy@magnificent-tears.com>',
            'Reini Urban <rurban@cpan.org>',
            'Robin Barker <rmb1@cise.npl.co.uk>',
            'SADAHIRO Tomoyuki <SADAHIRO@cpan.org>',
            'Simon Cozens <simon@netthink.co.uk>',
            'Slaven Rezic <SREZIC@cpan.org>',
            'Spider Boardman <spider@web.zk3.dec.com>',
            'Steve Hay <steve.m.hay@googlemail.com>',
            'Steve Peters <steve@fisharerojo.org>',
            'SUGAWARA Hajime <sugawara@hdt.co.jp>',
            'SUZUKI Norio <ZAP00217@nifty.com>',
            'szr8 <blz.marcel@gmail.com>',
            'Tatsuhiko Miyagawa <miyagawa@bulknews.net>',
            'Tels <perl_dummy@bloodgate.com>',
            'Tony Cook <tony@develop-help.com>',
            'Vadim Konovalov <vkonovalov@peterstar.ru>',
            'Victor <victor@vsespb.ru>',
            'Ville Skyttä <ville.skytta@iki.fi>',
            'Vincent van Dam <vvandam@sandvine.com>',
            'Yitzchak Scott-Thoennes <sthoenna@efn.org>',
        ],
    },
);

package MY;


sub post_initialize
{
    my ($self) = @_;
    my %o;
    # Find existing O_FILES
    foreach my $f (@{$self->{'O_FILES'}})
    {
    $o{$f} = 1;
    }
    my $x = $self->{'OBJ_EXT'};
    # Add the table O_FILES
    foreach my $e (keys %tables)
    {
    $o{$e.$x} = 1;
    }
    # Trick case-blind filesystems.
    delete $o{'encode'.$x};
    $o{'Encode'.$x} = 1;
    # Reset the variable
    $self->{'O_FILES'} = [sort keys %o];
    my @files;
    foreach my $table (sort keys %tables)
    {
    foreach my $ext (qw($(OBJ_EXT) .c .h .exh .fnm))
    {
    push (@files,$table.$ext);
    }
    $self->{SOURCE} .= " $table.c"
    if $^O eq 'MacOS' && $self->{SOURCE} !~ /\b$table\.c\b/;
}
$self->{'clean'}{'FILES'} .= join(' ',@files);
return '';
}

sub postamble
{
    my $self = shift;
    my $dir  = $self->catdir($self->curdir,'ucm');
    my $str  = "# Encode\$(OBJ_EXT) does not depend on .c files directly\n";
    $str  .= "# (except Encode.c), but on .h and .exh files written by enc2xs\n";
    $str  .= $^O eq 'MacOS' ? 'Encode.c.{$(MACPERL_BUILD_EXT_STATIC)}.o :' : 'Encode$(OBJ_EXT) :';
    $str  .= ' Encode.c';
    foreach my $table (sort keys %tables)
    {
    $str .= " $table.c";
    }
    $str .= "\n\n";
    foreach my $table (sort keys %tables)
    {
    my $numlines = 1;
    my $lengthsofar = length($str);
    my $continuator = '';
    my $enc2xs = $self->catfile('bin', 'enc2xs');
    $str .= "$table.c : $enc2xs Makefile.PL";
    foreach my $file (@{$tables{$table}})
    {
        $str .= $continuator.' '.$self->catfile($dir,$file);
        if ( length($str)-$lengthsofar > 128*$numlines )
        {
        $continuator .= " \\\n\t";
        $numlines++;
        } else {
        $continuator = '';
        }
    }
    my $plib   = $self->{PERL_CORE} ? '"-I$(PERL_LIB)"' : '';
    $plib .= " -MCross=$::Cross::platform" if defined $::Cross::platform;
    my $ucopts = '-"Q" -"O"';
    $str .=
        qq{\n\t\$(PERL) $plib $enc2xs $ucopts -o \$\@ -f $table.fnm\n\n};
    open (FILELIST, ">$table.fnm")
        || die "Could not open $table.fnm: $!";
    foreach my $file (@{$tables{$table}})
    {
        print FILELIST $self->catfile($dir,$file) . "\n";
    }
    close(FILELIST);
    }
    return $str;
}
