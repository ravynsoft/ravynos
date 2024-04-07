#!perl
use strict;
use warnings;
use autodie;
use feature qw(say);
require File::Find::Rule;
require File::Slurp;
require File::Spec;
require IO::Socket::SSL;
use List::Util qw(sum);
require LWP::UserAgent;
require Net::FTP;
require Parallel::Fork::BossWorkerAsync;
require Term::ProgressBar::Simple;
require URI::Find::Simple;
$| = 1;

my %ignore;
while ( my $line = <main::DATA> ) {
    chomp $line;
    next if $line =~ /^#/;
    next unless $line;
    $ignore{$line} = 1;
}

my $ua = LWP::UserAgent->new(ssl_opts => { verify_hostname => 0 });
$ua->timeout(58);
$ua->env_proxy;

my @filenames = @ARGV;
@filenames = sort grep { $_ !~ /^\.git/ } File::Find::Rule->new->file->in('.')
    unless @filenames;

my $total_bytes = sum map {-s} @filenames;

my $extract_progress = Term::ProgressBar::Simple->new(
    {   count => $total_bytes,
        name  => 'Extracting URIs',
    }
);

my %uris;
foreach my $filename (@filenames) {
    next if $filename =~ /uris\.txt/;
    next if $filename =~ /check_uris/;
    next if $filename =~ /\.patch$/;
    next if $filename =~ 'cpan/Pod-Simple/t/perlfaqo?\.pod';
    next if $filename =~ /checkURL\.pl$/;
    my $contents = File::Slurp::read_file($filename);
    my @uris     = URI::Find::Simple::list_uris($contents);
    foreach my $uri (@uris) {
        next unless $uri =~ /^(http|ftp)/;
        next if $ignore{$uri};

        # no need to hit rt.perl.org
        next
            if $uri =~ m{^https?://rt.perl.org/(?:rt3/)?Ticket/Display.html?id=\d+$};

        # no need to hit github
        next
            if $uri =~ m{^https?://(?:www\.)?github\.com/[pP]erl/perl5/issues/\d+$};

        # no need to hit rt.cpan.org
        next
            if $uri =~ m{^https?://rt.cpan.org/Public/Bug/Display.html?id=\d+$};

        # no need to hit google groups (weird redirect LWP does not like)
        next
            if $uri =~ m{^http://groups\.google\.com/};

        push @{ $uris{$uri} }, $filename;
    }
    $extract_progress += -s $filename;
}

my $bw = Parallel::Fork::BossWorkerAsync->new(
    work_handler   => \&work_alarmed,
    global_timeout => 120,
    worker_count   => 20,
);

foreach my $uri ( keys %uris ) {
    my @filenames = @{ $uris{$uri} };
    $bw->add_work( { uri => $uri, filenames => \@filenames } );
}

undef $extract_progress;

my $fetch_progress = Term::ProgressBar::Simple->new(
    {   count => scalar( keys %uris ),
        name  => 'Fetching URIs',
    }
);

my %filenames;
while ( $bw->pending() ) {
    my $response   = $bw->get_result();
    my $uri        = $response->{uri};
    my @filenames  = @{ $response->{filenames} };
    my $is_success = $response->{is_success};
    my $message    = $response->{message};

    unless ($is_success) {
        foreach my $filename (@filenames) {
            push @{ $filenames{$filename} },
                { uri => $uri, message => $message };
        }
    }
    $fetch_progress++;
}
$bw->shut_down();

my $fh = IO::File->new('> uris.txt');
foreach my $filename ( sort keys %filenames ) {
    $fh->say("* $filename");
    my @bits = @{ $filenames{$filename} };
    foreach my $bit (@bits) {
        my $uri     = $bit->{uri};
        my $message = $bit->{message};
        $fh->say("  $uri");
        $fh->say("    $message");
    }
}
$fh->close;

say 'Finished, see uris.txt';

sub work_alarmed {
    my $conf = shift;
    eval {
        local $SIG{ALRM} = sub { die "alarm\n" };    # NB: \n required
        alarm 60;
        $conf = work($conf);
        alarm 0;
    };
    if ($@) {
        $conf->{is_success} = 0;
        $conf->{message}    = 'Timed out';

    }
    return $conf;
}

sub work {
    my $conf      = shift;
    my $uri       = $conf->{uri};
    my @filenames = @{ $conf->{filenames} };

    if ( $uri =~ /^http/ ) {
        my $uri_without_fragment = URI->new($uri);
        my $fragment             = $uri_without_fragment->fragment(undef);
        my $response             = $ua->head($uri_without_fragment);

        $conf->{is_success} = $response->is_success;
        $conf->{message}    = $response->status_line;
        return $conf;
    } else {

        my $uri_object = URI->new($uri);
        my $host       = $uri_object->host;
        my $path       = $uri_object->path;
        my ( $volume, $directories, $filename )
            = File::Spec->splitpath($path);

        my $ftp = Net::FTP->new( $host, Passive => 1, Timeout => 60 );
        unless ($ftp) {
            $conf->{is_succcess} = 0;
            $conf->{message}     = "Can not connect to $host: $@";
            return $conf;
        }

        my $can_login = $ftp->login( "anonymous", '-anonymous@' );
        unless ($can_login) {
            $conf->{is_success} = 0;
            $conf->{message} = "Can not login ", $ftp->message;
            return $conf;
        }

        my $can_binary = $ftp->binary();
        unless ($can_binary) {
            $conf->{is_success} = 0;
            $conf->{message} = "Can not binary ", $ftp->message;
            return $conf;
        }

        my $can_cwd = $ftp->cwd($directories);
        unless ($can_cwd) {
            $conf->{is_success} = 0;
            $conf->{message} = "Can not cwd to $directories ", $ftp->message;
            return $conf;
        }

        if ($filename) {
            my $can_size = $ftp->size($filename);
            unless ($can_size) {
                $conf->{is_success} = 0;
                $conf->{message}
                    = "Can not size $filename in $directories",
                    $ftp->message;
                return $conf;
            }
        } else {
            my ($can_dir) = $ftp->dir;
            unless ($can_dir) {
                my ($can_ls) = $ftp->ls;
                unless ($can_ls) {
                    $conf->{is_success} = 0;
                    $conf->{message}
                        = "Can not dir or ls in $directories ",
                        $ftp->message;
                    return $conf;
                }
            }
        }

        $conf->{is_success} = 1;
        return $conf;
    }
}

__DATA__
# these are fine but give errors
ftp://ftp.stratus.com/pub/vos/posix/ga/ga.html
ftp://ftp.stratus.com/pub/vos/utility/utility.html

# these are missing, sigh
ftp://ftp.sco.com/SLS/ptf7051e.Z
http://perlmonks.thepen.com/42898.html
http://svn.mutatus.co.uk/wsvn/Scalar-List-Utils/trunk/
http://public.activestate.com/cgi-bin/perlbrowse
http://svn.mutatus.co.uk/browse/libnet/tags/libnet-1.17/ChangeLog
http://aspn.activestate.com/ASPN/Mail/Message/perl6-internals/2746631
http://my.smithmicro.com/mac/stuffit/
http://www.wg.omron.co.jp/cgi-bin/j-e/jfriedl.html
http://persephone.cps.unizar.es/general/gente/spd/gzip/gzip.html
http://www.openzaurus.org/
http://Casbah.org/
http://download.microsoft.com/download/vc15/Patch/1.52/W95/EN-US/nmake15.exe
http://www.pvhp.com/~pvhp/
http://www.pvhp.com/%7Epvhp/
http://www.pvhp.com/%7epvhp/
http://www.leo.org
http://www.madgoat.com
http://www.mks.com/s390/gnu/
http://www.tpj.com/
http://safaribooksonline.com/
http://use.perl.org/~autrijus/journal/25768
http://www.s390.ibm.com/products/oe/bpxqp1.html
http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg01396.html
http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg01489.html
http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg01491.html
http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg01608.html
http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg02144.html
http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1998-02/msg02998.html
http://www.xray.mpe.mpg.de/mailing-lists/perl5-porters/1999-03/msg00520.html
http://www.w3.org/Security/Faq/

# these are URI extraction bugs
http://www.perl.org/E
http://en.wikipedia.org/wiki/SREC_(file_format
http://somewhere.else',-type=/
ftp:passive-mode
ftp:
http:[-
http://search.cpan.org/src/KWILLIAMS/Module-Build-0.30/lib/Module/Build/Platform/Windows.pm:split_like_shell
http://www.xray.mpe.mpg.de/mailing-lists/perl5-
http://support.microsoft.com/support/kb/articles/Q280/3/41.ASP:
http://perl.come/
http://www.perl.come/

# these are used as an example
http://example.com/
http://something.here/
http://users.perl5.git.perl.org/~yourlogin/
http://github.com/USERNAME/perl/tree/orange
http://the.good.ship.lollypop.com:8080/cgi-bin/foo.cgi
http://the.good.ship.lollypop.com:8080/cgi-bin/foo.cgi/somewhere/else?game=chess;game=checkers;weather=dull;foo=bar
http://somewhere.else$/
http://somewhere.else$/
http://somewhere.else/bin/foo&bar',-Type=
http://the.good.ship.lollypop.com:8080/cgi-bin/foo.cgi
http://the.good.ship.lollypop.com:8080/cgi-bin/foo.cgi/somewhere/else?game=chess;game=checkers;weather=dull;foo=bar
http://www.perl.org/test.cgi
http://cpan2.local/
http://search.cpan.org/perldoc?
http://cpan1.local/
http://cpan.dev.local/CPAN
http:///
ftp://
ftp://myurl/
ftp://ftp.software.ibm.com/aix/fixes/v4/other/vac.C.5.0.2.6.bff
http://www14.software.ibm.com/webapp/download/downloadaz.jsp
http://www.cpan.org/modules/by-module/Archive/Archive-Zip-*.tar.gz
http://www.unicode.org/Public/MAPPINGS/ISO8859/8859-*.TXT
http://localhost/tmp/index.txt
http://example.com/foo/bar.html
http://example.com/Text-Bastardize-1.06.tar.gz
ftp://example.com/sources/packages.txt
http://example.com/sources/packages.txt
http://example.com/sources
ftp://example.com/sources
http://some.where.com/dir/file.txt
http://some.where.com/dir/a.txt
http://foo.com/X.tgz
ftp://foo.com/X.tgz
http://foo/
http://www.foo.com:8000/
http://mysite.com/cgi-bin/log.cgi/http://www.some.other.site/args
http://decoded/mirror/path
http://a/b/c/d/e/f/g/h/i/j
http://foo/bar.gz
ftp://ftp.perl.org
http://purl.org/rss/1.0/modules/taxonomy/
ftp://ftp.sun.ac.za/CPAN/CPAN/
ftp://ftp.cpan.org/pub/mirror/index.txt
ftp://cpan.org/pub/mirror/index.txt
http://example.com/~eh/
http://plagger.org/.../rss
http://umn.dl.sourceforge.net/mingw/gcc-g++-3.4.5-20060117-1.tar.gz
http://umn.dl.sourceforge.net/mingw/binutils-2.16.91-20060119-1.tar.gz
http://umn.dl.sourceforge.net/mingw/mingw-runtime-3.10.tar.gz
http://umn.dl.sourceforge.net/mingw/gcc-core-3.4.5-20060117-1.tar.gz
http://umn.dl.sourceforge.net/mingw/w32api-3.6.tar.gz
http://search.cpan.org/CPAN/authors/id/S/SH/SHAY/dmake-4.5-20060619-SHAY.zip  
http://module-build.sourceforge.net/META-spec-new.html
http://module-build.sourceforge.net/META-spec-v1.4.html
http://www.cs.vu.nl/~tmgil/vi.html
http://perlcomposer.sourceforge.net/vperl.html
http://www.perl.com/CPAN/authors/Tom_Christiansen/scripts/rep
http://w4.lns.cornell.edu/~pvhp/ptk/ptkTOC.html
http://world.std.com/~aep/ptkdb/
http://www.castlelink.co.uk/object_system/
http://www.fh-wedel.de/elvis/
ftp://ftp.blarg.net/users/amol/zsh/
ftp://ftp.funet.fi/pub/languages/perl/CPAN
http://search.cpan.org/CPAN/authors/id/S/SH/SHAY/dmake-4.5-20060619-SHAY.zip
http://users.perl5.git.perl.org/~USERNAME
http://foo/x//y/script.cgi/a//b
http://xxx/script.cgi/http://foo
http://foo/./x//z/script.cgi/a/../b//c
http://somewhere.else/in/movie/land
http://somewhere.else/finished.html
http://somewhere.else/bin/foo&bar$
http://somewhere.else/
http://proxy:8484/
http://proxy/
http://myrepo.example.com/
http://remote/source
https://example.com/
http://example.com:1024/
http:///path?foo=bar
http://[::]:1024/
http://([/
http://example.com:9000/index.html
http://proxy.example.com:8080/
http:///index.html
http://[www.json::pp.org]/
http://localhost/
http://foo.example.com/
http://abc.com/a.js
http://whatever/man/1/crontab
http://abc.com/c.js
http://whatever/Foo%3A%3ABar
http://abc.com/b.js
http://remote.server.com/jquery.css
http://some.other.com/page.html
https://text.com/1/2
https://text.com/1/2
http://link.included.here?o=1&p=2
http://link.included.here?o=1&amp;p=2
http://link.included.here?o=1&amp;p=2
http://link.included.here/
http://foo/x//y/script.cgi/a//b
http://xxx/script.cgi/http://foo
http://foo/./x//z/script.cgi/a/../b//c
http://somewhere.else/in/movie/land
http://somewhere.else/finished.html
http://webproxy:3128/
http://www/

# these are used to generate or match URLs
http://www.cpan.org/modules/by-authors/id/$1/$1$2/$dist
http://www.cpantesters.org/show/%s.yaml
ftp://(.*?)/(.*)/(.*
ftp://(.*?)/(.*)/(.*
ftp://(.*?)/(.*)/(.*
ftp://ftp.foo.bar/
http://$host/
http://wwwe%3C46/
ftp:/
http://$addr/mark?commit=$
http://search.cpan.org/~
http:/
ftp:%5Cn$url
http://www.ietf.org/rfc/rfc$2.txt
http://search.cpan.org/~
ftp:%5Cn$url

# weird redirects that LWP doesn't like
http://www.theperlreview.com/community_calendar
http://www.software.hp.com/portal/swdepot/displayProductInfo.do?productNumber=PERL
http://sunsolve.sun.com

# broken webserver that doesn't like HEAD requests
http://docs.sun.com/app/docs/doc/816-5168/syslog-3c?a=view
http://www.w3.org/TR/html4/loose.dtd

# these have been reported upstream to CPAN authors
http://www.gnu.org/manual/tar/html_node/tar_139.html
http://www.w3.org/pub/WWW/TR/Wd-css-1.html
http://support.microsoft.com/support/kb/articles/Q280/3/41.ASP
http://msdn.microsoft.com/workshop/author/dhtml/httponly_cookies.asp
http://search.cpan.org/search?query=Module::Build::Convert
http://www.refcnt.org/papers/module-build-convert
http://csrc.nist.gov/cryptval/shs.html
http://msdn.microsoft.com/workshop/author/dhtml/reference/charsets/charset4.asp
http://www.debian.or.jp/~kubota/unicode-symbols.html.en
http://www.mail-archive.com/perl5-porters@perl.org/msg69766.html
http://www.debian.or.jp/~kubota/unicode-symbols.html.en
http://rfc.net/rfc2781.html
http://www.icu-project.org/charset/
http://ppewww.ph.gla.ac.uk/~flavell/charset/form-i18n.html
http://www.rfc-editor.org/
http://www.rfc.net/
http://www.oreilly.com/people/authors/lunde/cjk_inf.html
http://www.oreilly.com/catalog/cjkvinfo/
http://www.cpan.org/modules/by-author/Damian_Conway/Filter-Simple.tar.gz
http://www.csse.monash.edu.au/~damian/CPAN/Filter-Simple.tar.gz
http://www.egt.ie/standards/iso3166/iso3166-1-en.html
http://www.bsi-global.com/iso4217currency
http://www.plover.com/~mjd/perl/Memoize/
http://www.plover.com/~mjd/perl/MiniMemoize/
http://www.sysadminmag.com/tpj/issues/vol5_5/
ftp://ftp.tpc.int/tpc/server/UNIX/
http://www.nara.gov/genealogy/
http://home.utah-inter.net/kinsearch/Soundex.html
http://www.nara.gov/genealogy/soundex/soundex.html
http://rfc.net/rfc3461.html
ftp://ftp.cs.pdx.edu/pub/elvis/
http://www.fh-wedel.de/elvis/
http://lists.perl.org/list/perl-mvs.html
http://www.cpan.org/ports/os2/
http://github.com/dagolden/cpan-meta-spec
http://github.com/dagolden/cpan-meta-spec/issues
http://www.opensource.org/licenses/lgpl-license.phpt
http://reality.sgi.com/ariel
http://www.chiark.greenend.org.uk/pipermail/ukcrypto/1999-February/003538.html
http://www.chiark.greenend.org.uk/pipermail/ukcrypto/1999-February/003538.html
http://www.nsrl.nist.gov/testdata/
http://public.activestate.com/cgi-bin/perlbrowse/p/31194
http://public.activestate.com/cgi-bin/perlbrowse?patch=16173
http://public.activestate.com/cgi-bin/perlbrowse?patch=16049
http://www.li18nux.org/docs/html/CodesetAliasTable-V10.html
http://aspn.activestate.com/ASPN/Mail/Message/perl5-porters/3486118
http://lxr.mozilla.org/seamonkey/source/intl/uconv/ucvlatin/vps.ut
http://lxr.mozilla.org/seamonkey/source/intl/uconv/ucvlatin/vps.uf
http://github.com/schwern/extutils-makemaker
https://github.com/dagolden/cpanpm/compare/master...private%2Fuse-http-lite
http://www.json.org/JSON::PP_checker/
ftp://ftp.kiae.su/pub/unix/fido/
http://www.gallistel.net/nparker/weather/code/
http://www.javaworld.com/javaworld/jw-09-2001/jw-0928-ntmessages.html
ftp://ftp-usa.tpc.int/pub/tpc/server/UNIX/
http://www.cis.ohio-state.edu/htbin/rfc/rfc959.html
http://public.activestate.com/cgi-bin/perlbrowse/p/33567
http://public.activestate.com/cgi-bin/perlbrowse/p/33566
http://www.dsmit.com/cons/
http://www.makemaker.org/wiki/index.cgi?ModuleBuildConversionGuide

__END__

=head1 NAME

checkURL.pl - Check that all the URLs in the Perl source are valid

=head1 DESCRIPTION

This program checks that all the URLs in the Perl source are valid. It
checks HTTP and FTP links in parallel and contains a list of known
bad example links in its source. It takes 4 minutes to run on my
machine. The results are written to 'uris.txt' and list the filename,
the URL and the error:

  * ext/Locale-Maketext/lib/Locale/Maketext.pod
    http://sunsite.dk/RFC/rfc/rfc2277.html
      404 Not Found
  ...

It should be run every so often and links fixed and upstream authors
notified.

Note that the web is unstable and some websites are temporarily down.
