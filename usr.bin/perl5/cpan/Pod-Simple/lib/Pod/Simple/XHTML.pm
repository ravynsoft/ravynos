=pod

=head1 NAME

Pod::Simple::XHTML -- format Pod as validating XHTML

=head1 SYNOPSIS

  use Pod::Simple::XHTML;

  my $parser = Pod::Simple::XHTML->new();

  ...

  $parser->parse_file('path/to/file.pod');

=head1 DESCRIPTION

This class is a formatter that takes Pod and renders it as XHTML
validating HTML.

This is a subclass of L<Pod::Simple::Methody> and inherits all its
methods. The implementation is entirely different than
L<Pod::Simple::HTML>, but it largely preserves the same interface.

=head2 Minimal code

  use Pod::Simple::XHTML;
  my $psx = Pod::Simple::XHTML->new;
  $psx->output_string(\my $html);
  $psx->parse_file('path/to/Module/Name.pm');
  open my $out, '>', 'out.html' or die "Cannot open 'out.html': $!\n";
  print $out $html;

You can also control the character encoding and entities. For example, if
you're sure that the POD is properly encoded (using the C<=encoding> command),
you can prevent high-bit characters from being encoded as HTML entities and
declare the output character set as UTF-8 before parsing, like so:

  $psx->html_charset('UTF-8');
  $psx->html_encode_chars(q{&<>'"});

=cut

package Pod::Simple::XHTML;
use strict;
use vars qw( $VERSION @ISA $HAS_HTML_ENTITIES );
$VERSION = '3.43';
use Pod::Simple::Methody ();
@ISA = ('Pod::Simple::Methody');

BEGIN {
  $HAS_HTML_ENTITIES = eval "require HTML::Entities; 1";
}

my %entities = (
  q{>} => 'gt',
  q{<} => 'lt',
  q{'} => '#39',
  q{"} => 'quot',
  q{&} => 'amp',
);

sub encode_entities {
  my $self = shift;
  my $ents = $self->html_encode_chars;
  return HTML::Entities::encode_entities( $_[0], $ents ) if $HAS_HTML_ENTITIES;
  if (defined $ents) {
      $ents =~ s,(?<!\\)([]/]),\\$1,g;
      $ents =~ s,(?<!\\)\\\z,\\\\,;
  } else {
      $ents = join '', keys %entities;
  }
  my $str = $_[0];
  $str =~ s/([$ents])/'&' . ($entities{$1} || sprintf '#x%X', ord $1) . ';'/ge;
  return $str;
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

=head1 METHODS

Pod::Simple::XHTML offers a number of methods that modify the format of
the HTML output. Call these after creating the parser object, but before
the call to C<parse_file>:

  my $parser = Pod::PseudoPod::HTML->new();
  $parser->set_optional_param("value");
  $parser->parse_file($file);

=head2 perldoc_url_prefix

In turning L<Foo::Bar> into http://whatever/Foo%3a%3aBar, what
to put before the "Foo%3a%3aBar". The default value is
"https://metacpan.org/pod/".

=head2 perldoc_url_postfix

What to put after "Foo%3a%3aBar" in the URL. This option is not set by
default.

=head2 man_url_prefix

In turning C<< L<crontab(5)> >> into http://whatever/man/1/crontab, what
to put before the "1/crontab". The default value is
"http://man.he.net/man".

=head2 man_url_postfix

What to put after "1/crontab" in the URL. This option is not set by default.

=head2 title_prefix, title_postfix

What to put before and after the title in the head. The values should
already be &-escaped.

=head2 html_css

  $parser->html_css('path/to/style.css');

The URL or relative path of a CSS file to include. This option is not
set by default.

=head2 html_javascript

The URL or relative path of a JavaScript file to pull in. This option is
not set by default.

=head2 html_doctype

A document type tag for the file. This option is not set by default.

=head2 html_charset

The character set to declare in the Content-Type meta tag created by default
for C<html_header_tags>. Note that this option will be ignored if the value of
C<html_header_tags> is changed. Defaults to "ISO-8859-1".

=head2 html_header_tags

Additional arbitrary HTML tags for the header of the document. The
default value is just a content type header tag:

  <meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">

Add additional meta tags here, or blocks of inline CSS or JavaScript
(wrapped in the appropriate tags).

=head3 html_encode_chars

A string containing all characters that should be encoded as HTML entities,
specified using the regular expression character class syntax (what you find
within brackets in regular expressions). This value will be passed as the
second argument to the C<encode_entities> function of L<HTML::Entities>. If
L<HTML::Entities> is not installed, then any characters other than C<&<>"'>
will be encoded numerically.

=head2 html_h_level

This is the level of HTML "Hn" element to which a Pod "head1" corresponds.  For
example, if C<html_h_level> is set to 2, a head1 will produce an H2, a head2
will produce an H3, and so on.

=head2 default_title

Set a default title for the page if no title can be determined from the
content. The value of this string should already be &-escaped.

=head2 force_title

Force a title for the page (don't try to determine it from the content).
The value of this string should already be &-escaped.

=head2 html_header, html_footer

Set the HTML output at the beginning and end of each file. The default
header includes a title, a doctype tag (if C<html_doctype> is set), a
content tag (customized by C<html_header_tags>), a tag for a CSS file
(if C<html_css> is set), and a tag for a Javascript file (if
C<html_javascript> is set). The default footer simply closes the C<html>
and C<body> tags.

The options listed above customize parts of the default header, but
setting C<html_header> or C<html_footer> completely overrides the
built-in header or footer. These may be useful if you want to use
template tags instead of literal HTML headers and footers or are
integrating converted POD pages in a larger website.

If you want no headers or footers output in the HTML, set these options
to the empty string.

=head2 index

Whether to add a table-of-contents at the top of each page (called an
index for the sake of tradition).

=head2 anchor_items

Whether to anchor every definition C<=item> directive. This needs to be
enabled if you want to be able to link to specific C<=item> directives, which
are output as C<< <dt> >> elements. Disabled by default.

=head2 backlink

Whether to turn every =head1 directive into a link pointing to the top 
of the page (specifically, the opening body tag).

=cut

__PACKAGE__->_accessorize(
 'perldoc_url_prefix',
 'perldoc_url_postfix',
 'man_url_prefix',
 'man_url_postfix',
 'title_prefix',  'title_postfix',
 'html_css',
 'html_javascript',
 'html_doctype',
 'html_charset',
 'html_encode_chars',
 'html_h_level',
 'title', # Used internally for the title extracted from the content
 'default_title',
 'force_title',
 'html_header',
 'html_footer',
 'index',
 'anchor_items',
 'backlink',
 'batch_mode', # whether we're in batch mode
 'batch_mode_current_level',
    # When in batch mode, how deep the current module is: 1 for "LWP",
    #  2 for "LWP::Procotol", 3 for "LWP::Protocol::GHTTP", etc
);

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

=head1 SUBCLASSING

If the standard options aren't enough, you may want to subclass
Pod::Simple::XHMTL. These are the most likely candidates for methods
you'll want to override when subclassing.

=cut

sub new {
  my $self = shift;
  my $new = $self->SUPER::new(@_);
  $new->{'output_fh'} ||= *STDOUT{IO};
  $new->perldoc_url_prefix('https://metacpan.org/pod/');
  $new->man_url_prefix('http://man.he.net/man');
  $new->html_charset('ISO-8859-1');
  $new->nix_X_codes(1);
  $new->{'scratch'} = '';
  $new->{'to_index'} = [];
  $new->{'output'} = [];
  $new->{'saved'} = [];
  $new->{'ids'} = { '_podtop_' => 1 }; # used in <body>
  $new->{'in_li'} = [];

  $new->{'__region_targets'}  = [];
  $new->{'__literal_targets'} = {};
  $new->accept_targets_as_html( 'html', 'HTML' );

  return $new;
}

sub html_header_tags {
    my $self = shift;
    return $self->{html_header_tags} = shift if @_;
    return $self->{html_header_tags}
        ||= '<meta http-equiv="Content-Type" content="text/html; charset='
            . $self->html_charset . '" />';
}

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

=head2 handle_text

This method handles the body of text within any element: it's the body
of a paragraph, or everything between a "=begin" tag and the
corresponding "=end" tag, or the text within an L entity, etc. You would
want to override this if you are adding a custom element type that does
more than just display formatted text. Perhaps adding a way to generate
HTML tables from an extended version of POD.

So, let's say you want to add a custom element called 'foo'. In your
subclass's C<new> method, after calling C<SUPER::new> you'd call:

  $new->accept_targets_as_text( 'foo' );

Then override the C<start_for> method in the subclass to check for when
"$flags->{'target'}" is equal to 'foo' and set a flag that marks that
you're in a foo block (maybe "$self->{'in_foo'} = 1"). Then override the
C<handle_text> method to check for the flag, and pass $text to your
custom subroutine to construct the HTML output for 'foo' elements,
something like:

  sub handle_text {
      my ($self, $text) = @_;
      if ($self->{'in_foo'}) {
          $self->{'scratch'} .= build_foo_html($text);
          return;
      }
      $self->SUPER::handle_text($text);
  }

=head2 handle_code

This method handles the body of text that is marked up to be code.
You might for instance override this to plug in a syntax highlighter.
The base implementation just escapes the text.

The callback methods C<start_code> and C<end_code> emits the C<code> tags
before and after C<handle_code> is invoked, so you might want to override these
together with C<handle_code> if this wrapping isn't suitable.

Note that the code might be broken into multiple segments if there are
nested formatting codes inside a C<< CE<lt>...> >> sequence.  In between the
calls to C<handle_code> other markup tags might have been emitted in that
case.  The same is true for verbatim sections if the C<codes_in_verbatim>
option is turned on.

=head2 accept_targets_as_html

This method behaves like C<accept_targets_as_text>, but also marks the region
as one whose content should be emitted literally, without HTML entity escaping
or wrapping in a C<div> element.

=cut

sub __in_literal_xhtml_region {
    return unless @{ $_[0]{__region_targets} };
    my $target = $_[0]{__region_targets}[-1];
    return $_[0]{__literal_targets}{ $target };
}

sub accept_targets_as_html {
    my ($self, @targets) = @_;
    $self->accept_targets(@targets);
    $self->{__literal_targets}{$_} = 1 for @targets;
}

sub handle_text {
    # escape special characters in HTML (<, >, &, etc)
    my $text = $_[0]->__in_literal_xhtml_region
        ? $_[1]
        : $_[0]->encode_entities( $_[1] );

    if ($_[0]{'in_code'} && @{$_[0]{'in_code'}}) {
        # Intentionally use the raw text in $_[1], even if we're not in a
        # literal xhtml region, since handle_code calls encode_entities.
        $_[0]->handle_code( $_[1], $_[0]{'in_code'}[-1] );
    } else {
        if ($_[0]->{in_for}) {
            my $newlines = $_[0]->__in_literal_xhtml_region ? "\n\n" : '';
            if ($_[0]->{started_for}) {
                if ($text =~ /\S/) {
                    delete $_[0]->{started_for};
                    $_[0]{'scratch'} .= $text . $newlines;
                }
                # Otherwise, append nothing until we have something to append.
            } else {
                # The parser sometimes preserves newlines and sometimes doesn't!
                $text =~ s/\n\z//;
                $_[0]{'scratch'} .= $text . $newlines;
            }
        } else {
            # Just plain text.
            $_[0]{'scratch'} .= $text;
        }
    }

    $_[0]{htext} .= $text if $_[0]{'in_head'};
}

sub start_code {
    $_[0]{'scratch'} .= '<code>';
}

sub end_code {
    $_[0]{'scratch'} .= '</code>';
}

sub handle_code {
    $_[0]{'scratch'} .= $_[0]->encode_entities( $_[1] );
}

sub start_Para {
    $_[0]{'scratch'} .= '<p>';
}

sub start_Verbatim {
    $_[0]{'scratch'} = '<pre>';
    push(@{$_[0]{'in_code'}}, 'Verbatim');
    $_[0]->start_code($_[0]{'in_code'}[-1]);
}

sub start_head1 {  $_[0]{'in_head'} = 1; $_[0]{htext} = ''; }
sub start_head2 {  $_[0]{'in_head'} = 2; $_[0]{htext} = ''; }
sub start_head3 {  $_[0]{'in_head'} = 3; $_[0]{htext} = ''; }
sub start_head4 {  $_[0]{'in_head'} = 4; $_[0]{htext} = ''; }
sub start_head5 {  $_[0]{'in_head'} = 5; $_[0]{htext} = ''; }
sub start_head6 {  $_[0]{'in_head'} = 6; $_[0]{htext} = ''; }

sub start_item_number {
    $_[0]{'scratch'} = "</li>\n" if ($_[0]{'in_li'}->[-1] && pop @{$_[0]{'in_li'}});
    $_[0]{'scratch'} .= '<li><p>';
    push @{$_[0]{'in_li'}}, 1;
}

sub start_item_bullet {
    $_[0]{'scratch'} = "</li>\n" if ($_[0]{'in_li'}->[-1] && pop @{$_[0]{'in_li'}});
    $_[0]{'scratch'} .= '<li><p>';
    push @{$_[0]{'in_li'}}, 1;
}

sub start_item_text   {
    # see end_item_text
}

sub start_over_bullet { $_[0]{'scratch'} = '<ul>'; push @{$_[0]{'in_li'}}, 0; $_[0]->emit }
sub start_over_block  { $_[0]{'scratch'} = '<ul>'; $_[0]->emit }
sub start_over_number { $_[0]{'scratch'} = '<ol>'; push @{$_[0]{'in_li'}}, 0; $_[0]->emit }
sub start_over_text   {
    $_[0]{'scratch'} = '<dl>';
    $_[0]{'dl_level'}++;
    $_[0]{'in_dd'} ||= [];
    $_[0]->emit
}

sub end_over_block  { $_[0]{'scratch'} .= '</ul>'; $_[0]->emit }

sub end_over_number   {
    $_[0]{'scratch'} = "</li>\n" if ( pop @{$_[0]{'in_li'}} );
    $_[0]{'scratch'} .= '</ol>';
    pop @{$_[0]{'in_li'}};
    $_[0]->emit;
}

sub end_over_bullet   {
    $_[0]{'scratch'} = "</li>\n" if ( pop @{$_[0]{'in_li'}} );
    $_[0]{'scratch'} .= '</ul>';
    pop @{$_[0]{'in_li'}};
    $_[0]->emit;
}

sub end_over_text   {
    if ($_[0]{'in_dd'}[ $_[0]{'dl_level'} ]) {
        $_[0]{'scratch'} = "</dd>\n";
        $_[0]{'in_dd'}[ $_[0]{'dl_level'} ] = 0;
    }
    $_[0]{'scratch'} .= '</dl>';
    $_[0]{'dl_level'}--;
    $_[0]->emit;
}

# . . . . . Now the actual formatters:

sub end_Para     { $_[0]{'scratch'} .= '</p>'; $_[0]->emit }
sub end_Verbatim {
    $_[0]->end_code(pop(@{$_[0]->{'in_code'}}));
    $_[0]{'scratch'} .= '</pre>';
    $_[0]->emit;
}

sub _end_head {
    my $h = delete $_[0]{in_head};

    my $add = $_[0]->html_h_level;
    $add = 1 unless defined $add;
    $h += $add - 1;

    my $id = $_[0]->idify($_[0]{htext});
    my $text = $_[0]{scratch};
    $_[0]{'scratch'} = $_[0]->backlink && ($h - $add == 0)
                         # backlinks enabled && =head1
                         ? qq{<a href="#_podtop_"><h$h id="$id">$text</h$h></a>}
                         : qq{<h$h id="$id">$text</h$h>};
    $_[0]->emit;
    push @{ $_[0]{'to_index'} }, [$h, $id, delete $_[0]{'htext'}];
}

sub end_head1       { shift->_end_head(@_); }
sub end_head2       { shift->_end_head(@_); }
sub end_head3       { shift->_end_head(@_); }
sub end_head4       { shift->_end_head(@_); }
sub end_head5       { shift->_end_head(@_); }
sub end_head6       { shift->_end_head(@_); }

sub end_item_bullet { $_[0]{'scratch'} .= '</p>'; $_[0]->emit }
sub end_item_number { $_[0]{'scratch'} .= '</p>'; $_[0]->emit }

sub end_item_text   {
    # idify and anchor =item content if wanted
    my $dt_id = $_[0]{'anchor_items'} 
                 ? ' id="'. $_[0]->idify($_[0]{'scratch'}) .'"'
                 : '';

    # reset scratch
    my $text = $_[0]{scratch};
    $_[0]{'scratch'} = '';

    if ($_[0]{'in_dd'}[ $_[0]{'dl_level'} ]) {
        $_[0]{'scratch'} = "</dd>\n";
        $_[0]{'in_dd'}[ $_[0]{'dl_level'} ] = 0;
    }

    $_[0]{'scratch'} .= qq{<dt$dt_id>$text</dt>\n<dd>};
    $_[0]{'in_dd'}[ $_[0]{'dl_level'} ] = 1;
    $_[0]->emit;
}

# This handles =begin and =for blocks of all kinds.
sub start_for {
  my ($self, $flags) = @_;

  push @{ $self->{__region_targets} }, $flags->{target_matching};
  $self->{started_for} = 1;
  $self->{in_for} = 1;

  unless ($self->__in_literal_xhtml_region) {
    $self->{scratch} .= '<div';
    $self->{scratch} .= qq( class="$flags->{target}") if $flags->{target};
    $self->{scratch} .= ">\n\n";
  }
}

sub end_for {
  my ($self) = @_;
  delete $self->{started_for};
  delete $self->{in_for};

  if ($self->__in_literal_xhtml_region) {
    # Remove trailine newlines.
    $self->{'scratch'} =~ s/\s+\z//s;
  } else {
    $self->{'scratch'} .= '</div>';
  }

  pop @{ $self->{__region_targets} };
  $self->emit;
}

sub start_Document {
  my ($self) = @_;
  if (defined $self->html_header) {
    $self->{'scratch'} .= $self->html_header;
    $self->emit unless $self->html_header eq "";
  } else {
    my ($doctype, $title, $metatags, $bodyid);
    $doctype = $self->html_doctype || '';
    $title = $self->force_title || $self->title || $self->default_title || '';
    $metatags = $self->html_header_tags || '';
    if (my $css = $self->html_css) {
        if ($css !~ /<link/) {
            # this is required to be compatible with Pod::Simple::BatchHTML
            $metatags .= '<link rel="stylesheet" href="'
                . $self->encode_entities($css) . '" type="text/css" />';
        } else {
            $metatags .= $css;
        }
    }
    if ($self->html_javascript) {
      $metatags .= qq{\n<script type="text/javascript" src="} .
                    $self->html_javascript . '"></script>';
    }
    $bodyid = $self->backlink ? ' id="_podtop_"' : '';
    $self->{'scratch'} .= <<"HTML";
$doctype
<html>
<head>
<title>$title</title>
$metatags
</head>
<body$bodyid>
HTML
    $self->emit;
  }
}

sub end_Document   {
  my ($self) = @_;
  my $to_index = $self->{'to_index'};
  if ($self->index && @{ $to_index } ) {
      my @out;
      my $level  = 0;
      my $indent = -1;
      my $space  = '';
      my $id     = ' id="index"';

      for my $h (@{ $to_index }, [0]) {
          my $target_level = $h->[0];
          # Get to target_level by opening or closing ULs
          if ($level == $target_level) {
              $out[-1] .= '</li>';
          } elsif ($level > $target_level) {
              $out[-1] .= '</li>' if $out[-1] =~ /^\s+<li>/;
              while ($level > $target_level) {
                  --$level;
                  push @out, ('  ' x --$indent) . '</li>' if @out && $out[-1] =~ m{^\s+<\/ul};
                  push @out, ('  ' x --$indent) . '</ul>';
              }
              push @out, ('  ' x --$indent) . '</li>' if $level;
          } else {
              while ($level < $target_level) {
                  ++$level;
                  push @out, ('  ' x ++$indent) . '<li>' if @out && $out[-1]=~ /^\s*<ul/;
                  push @out, ('  ' x ++$indent) . "<ul$id>";
                  $id = '';
              }
              ++$indent;
          }

          next unless $level;
          $space = '  '  x $indent;
          push @out, sprintf '%s<li><a href="#%s">%s</a>',
              $space, $h->[1], $h->[2];
      }
      # Splice the index in between the HTML headers and the first element.
      my $offset = defined $self->html_header ? $self->html_header eq '' ? 0 : 1 : 1;
      splice @{ $self->{'output'} }, $offset, 0, join "\n", @out;
  }

  if (defined $self->html_footer) {
    $self->{'scratch'} .= $self->html_footer;
    $self->emit unless $self->html_footer eq "";
  } else {
    $self->{'scratch'} .= "</body>\n</html>";
    $self->emit;
  }

  if ($self->index) {
      print {$self->{'output_fh'}} join ("\n\n", @{ $self->{'output'} }), "\n\n";
      @{$self->{'output'}} = ();
  }

}

# Handling code tags
sub start_B { $_[0]{'scratch'} .= '<b>' }
sub end_B   { $_[0]{'scratch'} .= '</b>' }

sub start_C { push(@{$_[0]{'in_code'}}, 'C'); $_[0]->start_code($_[0]{'in_code'}[-1]); }
sub end_C   { $_[0]->end_code(pop(@{$_[0]{'in_code'}})); }

sub start_F { $_[0]{'scratch'} .= '<i>' }
sub end_F   { $_[0]{'scratch'} .= '</i>' }

sub start_I { $_[0]{'scratch'} .= '<i>' }
sub end_I   { $_[0]{'scratch'} .= '</i>' }

sub start_L {
  my ($self, $flags) = @_;
    my ($type, $to, $section) = @{$flags}{'type', 'to', 'section'};
    my $url = $self->encode_entities(
        $type eq 'url' ? $to
            : $type eq 'pod' ? $self->resolve_pod_page_link($to, $section)
            : $type eq 'man' ? $self->resolve_man_page_link($to, $section)
            :                  undef
    );

    # If it's an unknown type, use an attribute-less <a> like HTML.pm.
    $self->{'scratch'} .= '<a' . ($url ? ' href="'. $url . '">' : '>');
}

sub end_L   { $_[0]{'scratch'} .= '</a>' }

sub start_S { $_[0]{'scratch'} .= '<span style="white-space: nowrap;">' }
sub end_S   { $_[0]{'scratch'} .= '</span>' }

sub emit {
  my($self) = @_;
  if ($self->index) {
      push @{ $self->{'output'} }, $self->{'scratch'};
  } else {
      print {$self->{'output_fh'}} $self->{'scratch'}, "\n\n";
  }
  $self->{'scratch'} = '';
  return;
}

=head2 resolve_pod_page_link

  my $url = $pod->resolve_pod_page_link('Net::Ping', 'INSTALL');
  my $url = $pod->resolve_pod_page_link('perlpodspec');
  my $url = $pod->resolve_pod_page_link(undef, 'SYNOPSIS');

Resolves a POD link target (typically a module or POD file name) and section
name to a URL. The resulting link will be returned for the above examples as:

  https://metacpan.org/pod/Net::Ping#INSTALL
  https://metacpan.org/pod/perlpodspec
  #SYNOPSIS

Note that when there is only a section argument the URL will simply be a link
to a section in the current document.

=cut

sub resolve_pod_page_link {
    my ($self, $to, $section) = @_;
    return undef unless defined $to || defined $section;
    if (defined $section) {
        $section = '#' . $self->idify($self->encode_entities($section), 1);
        return $section unless defined $to;
    } else {
        $section = ''
    }

    return ($self->perldoc_url_prefix || '')
        . $self->encode_entities($to) . $section
        . ($self->perldoc_url_postfix || '');
}

=head2 resolve_man_page_link

  my $url = $pod->resolve_man_page_link('crontab(5)', 'EXAMPLE CRON FILE');
  my $url = $pod->resolve_man_page_link('crontab');

Resolves a man page link target and numeric section to a URL. The resulting
link will be returned for the above examples as:

    http://man.he.net/man5/crontab
    http://man.he.net/man1/crontab

Note that the first argument is required. The section number will be parsed
from it, and if it's missing will default to 1. The second argument is
currently ignored, as L<man.he.net|http://man.he.net> does not currently
include linkable IDs or anchor names in its pages. Subclass to link to a
different man page HTTP server.

=cut

sub resolve_man_page_link {
    my ($self, $to, $section) = @_;
    return undef unless defined $to;
    my ($page, $part) = $to =~ /^([^(]+)(?:[(](\d+)[)])?$/;
    return undef unless $page;
    return ($self->man_url_prefix || '')
        . ($part || 1) . "/" . $self->encode_entities($page)
        . ($self->man_url_postfix || '');

}

=head2 idify

  my $id   = $pod->idify($text);
  my $hash = $pod->idify($text, 1);

This method turns an arbitrary string into a valid XHTML ID attribute value.
The rules enforced, following
L<http://webdesign.about.com/od/htmltags/a/aa031707.htm>, are:

=over

=item *

The id must start with a letter (a-z or A-Z)

=item *

All subsequent characters can be letters, numbers (0-9), hyphens (-),
underscores (_), colons (:), and periods (.).

=item *

The final character can't be a hyphen, colon, or period. URLs ending with these
characters, while allowed by XHTML, can be awkward to extract from plain text.

=item *

Each id must be unique within the document.

=back

In addition, the returned value will be unique within the context of the
Pod::Simple::XHTML object unless a second argument is passed a true value. ID
attributes should always be unique within a single XHTML document, but pass
the true value if you are creating not an ID but a URL hash to point to
an ID (i.e., if you need to put the "#foo" in C<< <a href="#foo">foo</a> >>.

=cut

sub idify {
    my ($self, $t, $not_unique) = @_;
    for ($t) {
        s/<[^>]+>//g;            # Strip HTML.
        s/&[^;]+;//g;            # Strip entities.
        s/^\s+//; s/\s+$//;      # Strip white space.
        s/^([^a-zA-Z]+)$/pod$1/; # Prepend "pod" if no valid chars.
        s/^[^a-zA-Z]+//;         # First char must be a letter.
        s/[^-a-zA-Z0-9_:.]+/-/g; # All other chars must be valid.
        s/[-:.]+$//;             # Strip trailing punctuation.
    }
    return $t if $not_unique;
    my $i = '';
    $i++ while $self->{ids}{"$t$i"}++;
    return "$t$i";
}

=head2 batch_mode_page_object_init

  $pod->batch_mode_page_object_init($batchconvobj, $module, $infile, $outfile, $depth);

Called by L<Pod::Simple::HTMLBatch> so that the class has a chance to
initialize the converter. Internally it sets the C<batch_mode> property to
true and sets C<batch_mode_current_level()>, but Pod::Simple::XHTML does not
currently use those features. Subclasses might, though.

=cut

sub batch_mode_page_object_init {
  my ($self, $batchconvobj, $module, $infile, $outfile, $depth) = @_;
  $self->batch_mode(1);
  $self->batch_mode_current_level($depth);
  return $self;
}

sub html_header_after_title {
}


1;

__END__

=head1 SEE ALSO

L<Pod::Simple>, L<Pod::Simple::Text>, L<Pod::Spell>

=head1 SUPPORT

Questions or discussion about POD and Pod::Simple should be sent to the
pod-people@perl.org mail list. Send an empty email to
pod-people-subscribe@perl.org to subscribe.

This module is managed in an open GitHub repository,
L<https://github.com/perl-pod/pod-simple/>. Feel free to fork and contribute, or
to clone L<git://github.com/perl-pod/pod-simple.git> and send patches!

Patches against Pod::Simple are welcome. Please send bug reports to
<bug-pod-simple@rt.cpan.org>.

=head1 COPYRIGHT AND DISCLAIMERS

Copyright (c) 2003-2005 Allison Randal.

This library is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

This program is distributed in the hope that it will be useful, but
without any warranty; without even the implied warranty of
merchantability or fitness for a particular purpose.

=head1 ACKNOWLEDGEMENTS

Thanks to L<Hurricane Electric|http://he.net/> for permission to use its
L<Linux man pages online|http://man.he.net/> site for man page links.

Thanks to L<search.cpan.org|http://search.cpan.org/> for permission to use the
site for Perl module links.

=head1 AUTHOR

Pod::Simpele::XHTML was created by Allison Randal <allison@perl.org>.

Pod::Simple was created by Sean M. Burke <sburke@cpan.org>.
But don't bother him, he's retired.

Pod::Simple is maintained by:

=over

=item * Allison Randal C<allison@perl.org>

=item * Hans Dieter Pearcey C<hdp@cpan.org>

=item * David E. Wheeler C<dwheeler@cpan.org>

=back

=cut
