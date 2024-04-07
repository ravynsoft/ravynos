package Pod::Html;
use strict;
use Exporter 'import';

our $VERSION = 1.34;
$VERSION = eval $VERSION;
our @EXPORT = qw(pod2html);

use Config;
use Cwd;
use File::Basename;
use File::Spec;
use Pod::Simple::Search;
use Pod::Simple::SimpleTree ();
use Pod::Html::Util qw(
    html_escape
    process_command_line
    trim_leading_whitespace
    unixify
    usage
    htmlify
    anchorify
    relativize_url
);
use locale; # make \w work right in non-ASCII lands

=head1 NAME

Pod::Html - module to convert pod files to HTML

=head1 SYNOPSIS

    use Pod::Html;
    pod2html([options]);

=head1 DESCRIPTION

Converts files from pod format (see L<perlpod>) to HTML format.  It
can automatically generate indexes and cross-references, and it keeps
a cache of things it knows how to cross-reference.

=head1 FUNCTIONS

=head2 pod2html

    pod2html("pod2html",
             "--podpath=lib:ext:pod:vms",
             "--podroot=/usr/src/perl",
             "--htmlroot=/perl/nmanual",
             "--recurse",
             "--infile=foo.pod",
             "--outfile=/perl/nmanual/foo.html");

pod2html takes the following arguments:

=over 4

=item backlink

    --backlink

Turns every C<head1> heading into a link back to the top of the page.
By default, no backlinks are generated.

=item cachedir

    --cachedir=name

Creates the directory cache in the given directory.

=item css

    --css=stylesheet

Specify the URL of a cascading style sheet.  Also disables all HTML/CSS
C<style> attributes that are output by default (to avoid conflicts).

=item flush

    --flush

Flushes the directory cache.

=item header

    --header
    --noheader

Creates header and footer blocks containing the text of the C<NAME>
section.  By default, no headers are generated.

=item help

    --help

Displays the usage message.

=item htmldir

    --htmldir=name

Sets the directory to which all cross references in the resulting
html file will be relative. Not passing this causes all links to be
absolute since this is the value that tells Pod::Html the root of the 
documentation tree.

Do not use this and --htmlroot in the same call to pod2html; they are
mutually exclusive.

=item htmlroot

    --htmlroot=name

Sets the base URL for the HTML files.  When cross-references are made,
the HTML root is prepended to the URL.

Do not use this if relative links are desired: use --htmldir instead.

Do not pass both this and --htmldir to pod2html; they are mutually
exclusive.

=item index

    --index
    --noindex

Generate an index at the top of the HTML file.  This is the default
behaviour.

=item infile

    --infile=name

Specify the pod file to convert.  Input is taken from STDIN if no
infile is specified.

=item outfile

    --outfile=name

Specify the HTML file to create.  Output goes to STDOUT if no outfile
is specified.

=item poderrors

    --poderrors
    --nopoderrors

Include a "POD ERRORS" section in the outfile if there were any POD 
errors in the infile. This section is included by default.

=item podpath

    --podpath=name:...:name

Specify which subdirectories of the podroot contain pod files whose
HTML converted forms can be linked to in cross references.

=item podroot

    --podroot=name

Specify the base directory for finding library pods. Default is the
current working directory.

=item quiet

    --quiet
    --noquiet

Don't display I<mostly harmless> warning messages.  These messages
will be displayed by default.  But this is not the same as C<verbose>
mode.

=item recurse

    --recurse
    --norecurse

Recurse into subdirectories specified in podpath (default behaviour).

=item title

    --title=title

Specify the title of the resulting HTML file.

=item verbose

    --verbose
    --noverbose

Display progress messages.  By default, they won't be displayed.

=back

=head2 Formerly Exported Auxiliary Functions

Prior to perl-5.36, the following three functions were exported by
F<Pod::Html>, either by default or on request:

=over 4

=item * C<htmlify()> (by default)

=item * C<anchorify()> (upon request)

=item * C<relativize_url()> (upon request)

=back

The definition and documentation of these functions have been moved to
F<Pod::Html::Util>, viewable via C<perldoc Pod::Html::Util>.

Beginning with perl-5.38 these functions must be explicitly imported from
F<Pod::Html::Util>.  Please modify your code as needed.

=head1 ENVIRONMENT

Uses C<$Config{pod2html}> to setup default options.

=head1 AUTHOR

Marc Green, E<lt>marcgreen@cpan.orgE<gt>. 

Original version by Tom Christiansen, E<lt>tchrist@perl.comE<gt>.

=head1 SEE ALSO

L<perlpod>

=head1 COPYRIGHT

This program is distributed under the Artistic License.

=cut

sub new {
    my $class = shift;
    return bless {}, $class;
}

sub pod2html {
    local(@ARGV) = @_;
    local $_;

    my $self = Pod::Html->new();
    $self->init_globals();

    my $opts = process_command_line;
    $self->process_options($opts);

    $self->refine_globals();

    # load or generate/cache %Pages
    unless ($self->get_cache()) {
        # generate %Pages
        #%Pages = $self->generate_cache(\%Pages);
        $self->generate_cache($self->{Pages});
    }
    my $input   = $self->identify_input();
    my $podtree = $self->parse_input_for_podtree($input);
    $self->set_Title_from_podtree($podtree);

    # set options for the HTML generator
    my $parser = Pod::Simple::XHTML::LocalPodLinks->new();
    $parser->codes_in_verbatim(0);
    $parser->anchor_items(1); # the old Pod::Html always did
    $parser->backlink($self->{Backlink}); # linkify =head1 directives
    $parser->force_title($self->{Title});
    $parser->htmldir($self->{Htmldir});
    $parser->htmlfileurl($self->{Htmlfileurl});
    $parser->htmlroot($self->{Htmlroot});
    $parser->index($self->{Doindex});
    $parser->output_string(\$self->{output}); # written to file later
    #$parser->pages(\%Pages);
    $parser->pages($self->{Pages});
    $parser->quiet($self->{Quiet});
    $parser->verbose($self->{Verbose});

    $parser = $self->refine_parser($parser);
    $self->feed_tree_to_parser($parser, $podtree);
    $self->write_file();
}

sub init_globals {
    my $self = shift;
    $self->{Cachedir} = ".";            # The directory to which directory caches
                                        #   will be written.

    $self->{Dircache} = "pod2htmd.tmp";

    $self->{Htmlroot} = "/";            # http-server base directory from which all
                                        #   relative paths in $podpath stem.
    $self->{Htmldir} = "";              # The directory to which the html pages
                                        #   will (eventually) be written.
    $self->{Htmlfile} = "";             # write to stdout by default
    $self->{Htmlfileurl} = "";          # The url that other files would use to
                                        # refer to this file.  This is only used
                                        # to make relative urls that point to
                                        # other files.

    $self->{Poderrors} = 1;
    $self->{Podfile} = "";              # read from stdin by default
    $self->{Podpath} = [];              # list of directories containing library pods.
    $self->{Podroot} = $self->{Curdir} = File::Spec->curdir;
                                        # filesystem base directory from which all
                                        #   relative paths in $podpath stem.
    $self->{Css} = '';                  # Cascading style sheet
    $self->{Recurse} = 1;               # recurse on subdirectories in $podpath.
    $self->{Quiet} = 0;                 # not quiet by default
    $self->{Verbose} = 0;               # not verbose by default
    $self->{Doindex} = 1;               # non-zero if we should generate an index
    $self->{Backlink} = 0;              # no backlinks added by default
    $self->{Header} = 0;                # produce block header/footer
    $self->{Title} = undef;             # title to give the pod(s)
    $self->{Saved_Cache_Key} = '';
    $self->{Pages} = {};
    return $self;
}

sub process_options {
    my ($self, $opts) = @_;

    $self->{Podpath}   = (defined $opts->{podpath})
                            ? [ split(":", $opts->{podpath}) ]
                            : [];

    $self->{Backlink}  =          $opts->{backlink}   if defined $opts->{backlink};
    $self->{Cachedir}  =  unixify($opts->{cachedir})  if defined $opts->{cachedir};
    $self->{Css}       =          $opts->{css}        if defined $opts->{css};
    $self->{Header}    =          $opts->{header}     if defined $opts->{header};
    $self->{Htmldir}   =  unixify($opts->{htmldir})   if defined $opts->{htmldir};
    $self->{Htmlroot}  =  unixify($opts->{htmlroot})  if defined $opts->{htmlroot};
    $self->{Doindex}   =          $opts->{index}      if defined $opts->{index};
    $self->{Podfile}   =  unixify($opts->{infile})    if defined $opts->{infile};
    $self->{Htmlfile}  =  unixify($opts->{outfile})   if defined $opts->{outfile};
    $self->{Poderrors} =          $opts->{poderrors}  if defined $opts->{poderrors};
    $self->{Podroot}   =  unixify($opts->{podroot})   if defined $opts->{podroot};
    $self->{Quiet}     =          $opts->{quiet}      if defined $opts->{quiet};
    $self->{Recurse}   =          $opts->{recurse}    if defined $opts->{recurse};
    $self->{Title}     =          $opts->{title}      if defined $opts->{title};
    $self->{Verbose}   =          $opts->{verbose}    if defined $opts->{verbose};

    warn "Flushing directory caches\n"
        if $opts->{verbose} && defined $opts->{flush};
    $self->{Dircache} = "$self->{Cachedir}/pod2htmd.tmp";
    if (defined $opts->{flush}) {
        1 while unlink($self->{Dircache});
    }
    return $self;
}

sub refine_globals {
    my $self = shift;

    # prevent '//' in urls
    $self->{Htmlroot} = "" if $self->{Htmlroot} eq "/";
    $self->{Htmldir} =~ s#/\z##;

    if (  $self->{Htmlroot} eq ''
       && defined( $self->{Htmldir} )
       && $self->{Htmldir} ne ''
       && substr( $self->{Htmlfile}, 0, length( $self->{Htmldir} ) ) eq $self->{Htmldir}
       ) {
        # Set the 'base' url for this file, so that we can use it
        # as the location from which to calculate relative links
        # to other files. If this is '', then absolute links will
        # be used throughout.
        #$self->{Htmlfileurl} = "$self->{Htmldir}/" . substr( $self->{Htmlfile}, length( $self->{Htmldir} ) + 1);
        # Is the above not just "$self->{Htmlfileurl} = $self->{Htmlfile}"?
        $self->{Htmlfileurl} = unixify($self->{Htmlfile});
    }
    return $self;
}

sub generate_cache {
    my $self = shift;
    my $pwd = getcwd();
    chdir($self->{Podroot}) ||
        die "$0: error changing to directory $self->{Podroot}: $!\n";

    # find all pod modules/pages in podpath, store in %Pages
    # - inc(0): do not prepend directories in @INC to search list;
    #     limit search to those in @{$self->{Podpath}}
    # - verbose: report (via 'warn') what search is doing
    # - laborious: to allow '.' in dirnames (e.g., /usr/share/perl/5.14.1)
    # - recurse: go into subdirectories
    # - survey: search for POD files in PodPath
    my ($name2path, $path2name) = 
        Pod::Simple::Search->new->inc(0)->verbose($self->{Verbose})->laborious(1)
        ->recurse($self->{Recurse})->survey(@{$self->{Podpath}});
    # remove Podroot and extension from each file
    for my $k (keys %{$name2path}) {
        $self->{Pages}{$k} = _transform($self, $name2path->{$k});
    }

    chdir($pwd) || die "$0: error changing to directory $pwd: $!\n";

    # cache the directory list for later use
    warn "caching directories for later use\n" if $self->{Verbose};
    open my $cache, '>', $self->{Dircache}
        or die "$0: error open $self->{Dircache} for writing: $!\n";

    print $cache join(":", @{$self->{Podpath}}) . "\n$self->{Podroot}\n";
    my $_updirs_only = ($self->{Podroot} =~ /\.\./) && !($self->{Podroot} =~ /[^\.\\\/]/);
    foreach my $key (keys %{$self->{Pages}}) {
        if($_updirs_only) {
          my $_dirlevel = $self->{Podroot};
          while($_dirlevel =~ /\.\./) {
            $_dirlevel =~ s/\.\.//;
            # Assume $Pagesref->{$key} has '/' separators (html dir separators).
            $self->{Pages}->{$key} =~ s/^[\w\s\-\.]+\///;
          }
        }
        print $cache "$key $self->{Pages}->{$key}\n";
    }
    close $cache or die "error closing $self->{Dircache}: $!";
}

sub _transform {
    my ($self, $v) = @_;
    $v = $self->{Podroot} eq File::Spec->curdir
               ? File::Spec->abs2rel($v)
               : File::Spec->abs2rel($v,
                                     File::Spec->canonpath($self->{Podroot}));

    # Convert path to unix style path
    $v = unixify($v);

    my ($file, $dir) = fileparse($v, qr/\.[^.]*/); # strip .ext
    return $dir.$file;
}

sub get_cache {
    my $self = shift;

    # A first-level cache:
    # Don't bother reading the cache files if they still apply
    # and haven't changed since we last read them.

    my $this_cache_key = $self->cache_key();
    return 1 if $self->{Saved_Cache_Key} and $this_cache_key eq $self->{Saved_Cache_Key};
    $self->{Saved_Cache_Key} = $this_cache_key;

    # load the cache of %Pages if possible.  $tests will be
    # non-zero if successful.
    my $tests = 0;
    if (-f $self->{Dircache}) {
        warn "scanning for directory cache\n" if $self->{Verbose};
        $tests = $self->load_cache();
    }

    return $tests;
}

sub cache_key {
    my $self = shift;
    return join('!',
        $self->{Dircache},
        $self->{Recurse},
        @{$self->{Podpath}},
        $self->{Podroot},
        stat($self->{Dircache}),
    );
}

#
# load_cache - tries to find if the cache stored in $dircache is a valid
#  cache of %Pages.  if so, it loads them and returns a non-zero value.
#
sub load_cache {
    my $self = shift;
    my $tests = 0;
    local $_;

    warn "scanning for directory cache\n" if $self->{Verbose};
    open(my $cachefh, '<', $self->{Dircache}) ||
        die "$0: error opening $self->{Dircache} for reading: $!\n";
    $/ = "\n";

    # is it the same podpath?
    $_ = <$cachefh>;
    chomp($_);
    $tests++ if (join(":", @{$self->{Podpath}}) eq $_);

    # is it the same podroot?
    $_ = <$cachefh>;
    chomp($_);
    $tests++ if ($self->{Podroot} eq $_);

    # load the cache if its good
    if ($tests != 2) {
        close($cachefh);
        return 0;
    }

    warn "loading directory cache\n" if $self->{Verbose};
    while (<$cachefh>) {
        /(.*?) (.*)$/;
        $self->{Pages}->{$1} = $2;
    }

    close($cachefh);
    return 1;
}

sub identify_input {
    my $self = shift;
    my $input;
    unless (@ARGV && $ARGV[0]) {
        if ($self->{Podfile} and $self->{Podfile} ne '-') {
            $input = $self->{Podfile};
        } else {
            $input = '-'; # XXX: make a test case for this
        }
    } else {
        $self->{Podfile} = $ARGV[0];
        $input = *ARGV;
    }
    return $input;
}

sub parse_input_for_podtree {
    my ($self, $input) = @_;
    # set options for input parser
    my $input_parser = Pod::Simple::SimpleTree->new;
    # Normalize whitespace indenting
    $input_parser->strip_verbatim_indent(\&trim_leading_whitespace);

    $input_parser->codes_in_verbatim(0);
    $input_parser->accept_targets(qw(html HTML));
    $input_parser->no_errata_section(!$self->{Poderrors}); # note the inverse

    warn "Converting input file $self->{Podfile}\n" if $self->{Verbose};
    my $podtree = $input_parser->parse_file($input)->root;
    return $podtree;
}

sub set_Title_from_podtree {
    my ($self, $podtree) = @_;
    unless(defined $self->{Title}) {
        if($podtree->[0] eq "Document" && ref($podtree->[2]) eq "ARRAY" &&
            $podtree->[2]->[0] eq "head1" && @{$podtree->[2]} == 3 &&
            ref($podtree->[2]->[2]) eq "" && $podtree->[2]->[2] eq "NAME" &&
            ref($podtree->[3]) eq "ARRAY" && $podtree->[3]->[0] eq "Para" &&
            @{$podtree->[3]} >= 3 &&
            !(grep { ref($_) ne "" }
                @{$podtree->[3]}[2..$#{$podtree->[3]}]) &&
            (@$podtree == 4 ||
                (ref($podtree->[4]) eq "ARRAY" &&
                $podtree->[4]->[0] eq "head1"))) {
            $self->{Title} = join("", @{$podtree->[3]}[2..$#{$podtree->[3]}]);
        }
    }

    $self->{Title} //= "";
    $self->{Title} = html_escape($self->{Title});
    return $self;
}

sub refine_parser {
    my ($self, $parser) = @_;
    # We need to add this ourselves because we use our own header, not
    # ::XHTML's header. We need to set $parser->backlink to linkify
    # the =head1 directives
    my $bodyid = $self->{Backlink} ? ' id="_podtop_"' : '';

    my $csslink = '';
    my $tdstyle = ' style="background-color: #cccccc; color: #000"';

    if ($self->{Css}) {
        $csslink = qq(\n<link rel="stylesheet" href="$self->{Css}" type="text/css" />);
        $csslink =~ s,\\,/,g;
        $csslink =~ s,(/.):,$1|,;
        $tdstyle= '';
    }

    # header/footer block
    my $block = $self->{Header} ? <<END_OF_BLOCK : '';
<table border="0" width="100%" cellspacing="0" cellpadding="3">
<tr><td class="_podblock_"$tdstyle valign="middle">
<big><strong><span class="_podblock_">&nbsp;$self->{Title}</span></strong></big>
</td></tr>
</table>
END_OF_BLOCK

    # create own header/footer because of --header
    $parser->html_header(<<"HTMLHEAD");
<?xml version="1.0" ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<title>$self->{Title}</title>$csslink
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
<link rev="made" href="mailto:$Config{perladmin}" />
</head>

<body$bodyid>
$block
HTMLHEAD

    $parser->html_footer(<<"HTMLFOOT");
$block
</body>

</html>
HTMLFOOT
    return $parser;
}

# This sub duplicates the guts of Pod::Simple::FromTree.  We could have
# used that module, except that it would have been a non-core dependency.
sub feed_tree_to_parser {
    my($self, $parser, $tree) = @_;
    if(ref($tree) eq "") {
        $parser->_handle_text($tree);
    } elsif(!($tree->[0] eq "X" && $parser->nix_X_codes)) {
        $parser->_handle_element_start($tree->[0], $tree->[1]);
        $self->feed_tree_to_parser($parser, $_) foreach @{$tree}[2..$#$tree];
        $parser->_handle_element_end($tree->[0]);
    }
}

sub write_file {
    my $self = shift;
    $self->{Htmlfile} = "-" unless $self->{Htmlfile}; # stdout
    my $fhout;
    if($self->{Htmlfile} and $self->{Htmlfile} ne '-') {
        open $fhout, ">", $self->{Htmlfile}
            or die "$0: cannot open $self->{Htmlfile} file for output: $!\n";
    } else {
        open $fhout, ">-";
    }
    binmode $fhout, ":utf8";
    print $fhout $self->{output};
    close $fhout or die "Failed to close $self->{Htmlfile}: $!";
    chmod 0644, $self->{Htmlfile} unless $self->{Htmlfile} eq '-';
}

package Pod::Simple::XHTML::LocalPodLinks;
use strict;
use warnings;
use parent 'Pod::Simple::XHTML';

use File::Spec;
use File::Spec::Unix;

__PACKAGE__->_accessorize(
 'htmldir',
 'htmlfileurl',
 'htmlroot',
 'pages', # Page name => relative/path/to/page from root POD dir
 'quiet',
 'verbose',
);

sub resolve_pod_page_link {
    my ($self, $to, $section) = @_;

    return undef unless defined $to || defined $section;
    if (defined $section) {
        $section = '#' . $self->idify($section, 1);
        return $section unless defined $to;
    } else {
        $section = '';
    }

    my $path; # path to $to according to %Pages
    unless (exists $self->pages->{$to}) {
        # Try to find a POD that ends with $to and use that.
        # e.g., given L<XHTML>, if there is no $Podpath/XHTML in %Pages,
        # look for $Podpath/*/XHTML in %Pages, with * being any path,
        # as a substitute (e.g., $Podpath/Pod/Simple/XHTML)
        my @matches;
        foreach my $modname (keys %{$self->pages}) {
            push @matches, $modname if $modname =~ /::\Q$to\E\z/;
        }

        # make it look like a path instead of a namespace
        my $modloc = File::Spec->catfile(split(/::/, $to));

        if ($#matches == -1) {
            warn "Cannot find file \"$modloc.*\" directly under podpath, " . 
                 "cannot find suitable replacement: link remains unresolved.\n"
                 if $self->verbose;
            return '';
        } elsif ($#matches == 0) {
            $path = $self->pages->{$matches[0]};
            my $matchloc = File::Spec->catfile(split(/::/, $path));
            warn "Cannot find file \"$modloc.*\" directly under podpath, but ".
                 "I did find \"$matchloc.*\", so I'll assume that is what you ".
                 "meant to link to.\n"
                 if $self->verbose;
        } else {
            # Use [-1] so newer (higher numbered) perl PODs are used
            # XXX currently, @matches isn't sorted so this is not true
            $path = $self->pages->{$matches[-1]};
            my $matchloc = File::Spec->catfile(split(/::/, $path));
            warn "Cannot find file \"$modloc.*\" directly under podpath, but ".
                 "I did find \"$matchloc.*\" (among others), so I'll use that " .
                 "to resolve the link.\n" if $self->verbose;
        }
    } else {
        $path = $self->pages->{$to};
    }

    my $url = File::Spec::Unix->catfile(Pod::Html::Util::unixify($self->htmlroot),
                                        $path);

    if ($self->htmlfileurl ne '') {
        # then $self->htmlroot eq '' (by definition of htmlfileurl) so
        # $self->htmldir needs to be prepended to link to get the absolute path
        # that will be relativized
        $url = Pod::Html::Util::relativize_url(
            File::Spec::Unix->catdir(Pod::Html::Util::unixify($self->htmldir), $url),
            $self->htmlfileurl # already unixified
        );
    }

    return $url . ".html$section";
}

1;
