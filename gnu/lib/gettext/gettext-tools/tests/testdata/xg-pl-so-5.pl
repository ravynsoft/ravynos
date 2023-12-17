# HTML.pm: output tree as HTML.
#
# Copyright 2011-2022 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License,
# or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#
# The documentation of the HTML customization API which is both
# used and implemented in the current file is in the customization_api
# Texinfo manual.
#
# Formatting and conversion functions that can be replaced by user-defined
# functions should only use documented functions to pass information
# and formatted content, such that users can overrides them independently
# without risking unwanted results.  Also in formatting functions, the state of
# the converter should only be accessed through functions, such as in_math,
# in_preformatted, preformatted_classes_stack and similar functions.
#
# Original author: Patrice Dumas <pertusus@free.fr>

package Texinfo::Convert::HTML;

use 5.00405;

# See 'The "Unicode Bug"' under 'perlunicode' man page.  This means
# that regular expressions will treat characters 128-255 in a Perl string
# the same regardless of whether the string is using a UTF-8 encoding.
#  For older Perls, you can use utf8::upgrade on the strings, where the
# difference matters.
use if $] >= 5.012, feature => 'unicode_strings';

use if $] >= 5.014, re => '/a';  # ASCII-only character classes in regexes

use strict;

# To check if there is no erroneous autovivification
#no autovivification qw(fetch delete exists store strict);

use Carp qw(cluck confess);

use File::Copy qw(copy);

use Storable;

use Encode qw(find_encoding decode encode);

use Texinfo::Commands;
use Texinfo::Common;
use Texinfo::Config;
use Texinfo::Convert::Unicode;
use Texinfo::Convert::Texinfo;
use Texinfo::Convert::Utils;
use Texinfo::Convert::Text;
use Texinfo::Convert::NodeNameNormalization;
use Texinfo::Structuring;
use Texinfo::Convert::Converter;

# used to convert Texinfo to LaTeX math in @math and @displaymath
# for further conversion by softwares that only convert LaTeX.
# NOTE mathjax does not implement some constructs output by the
# Texinfo::Convert::LaTeX converter.  Examples in 2022:
# \mathord{\text{}} \textsl{} \copyright{} \mathsterling{}
use Texinfo::Convert::LaTeX;


require Exporter;
use vars qw($VERSION @ISA);
@ISA = qw(Texinfo::Convert::Converter);

$VERSION = '7.0dev';

our $module_loaded = 0;
sub import {
  if (!$module_loaded) {
    Texinfo::XSLoader::override(
      "Texinfo::Convert::HTML::_default_format_protect_text",
      "Texinfo::MiscXS::default_format_protect_text");
    Texinfo::XSLoader::override(
      "Texinfo::Convert::HTML::_entity_text",
      "Texinfo::MiscXS::entity_text");
    $module_loaded = 1;
  }
  # The usual import method
  goto &Exporter::import;
}



my %nobrace_commands = %Texinfo::Commands::nobrace_commands;
my %line_commands = %Texinfo::Commands::line_commands;
my %nobrace_symbol_text = %Texinfo::Common::nobrace_symbol_text;
my %accent_commands = %Texinfo::Commands::accent_commands;
my %sectioning_heading_commands = %Texinfo::Commands::sectioning_heading_commands;
my %def_commands = %Texinfo::Commands::def_commands;
my %ref_commands = %Texinfo::Commands::ref_commands;
my %brace_commands = %Texinfo::Commands::brace_commands;
my %block_commands = %Texinfo::Commands::block_commands;
my %root_commands = %Texinfo::Commands::root_commands;
my %preformatted_commands = %Texinfo::Commands::preformatted_commands;
my %math_commands = %Texinfo::Commands::math_commands;
my %preformatted_code_commands = %Texinfo::Commands::preformatted_code_commands;
my %letter_no_arg_commands = %Texinfo::Commands::letter_no_arg_commands;

my %formatted_line_commands = %Texinfo::Commands::formatted_line_commands;
my %formatted_nobrace_commands = %Texinfo::Commands::formatted_nobrace_commands;
my %formattable_line_commands = %Texinfo::Commands::formattable_line_commands;
my %explained_commands = %Texinfo::Commands::explained_commands;
my %inline_format_commands = %Texinfo::Commands::inline_format_commands;
my %brace_code_commands       = %Texinfo::Commands::brace_code_commands;
my %default_index_commands = %Texinfo::Commands::default_index_commands;
my %small_block_associated_command = %Texinfo::Common::small_block_associated_command;

foreach my $def_command (keys(%def_commands)) {
  $formatted_line_commands{$def_command} = 1 if ($line_commands{$def_command});
}

# FIXME remove raw commands?
my %format_context_commands = (%block_commands, %root_commands);

foreach my $misc_context_command('tab', 'item', 'itemx', 'headitem') {
  $format_context_commands{$misc_context_command} = 1;
}

my %HTML_align_commands;
foreach my $align_command('raggedright', 'flushleft', 'flushright', 'center') {
  $HTML_align_commands{$align_command} = 1;
}

my %composition_context_commands = (%preformatted_commands, %root_commands,
  %HTML_align_commands);
$composition_context_commands{'float'} = 1;
my %format_raw_commands;
foreach my $block_command (keys(%block_commands)) {
  $composition_context_commands{$block_command} = 1
    if ($block_commands{$block_command} eq 'menu');
  $format_raw_commands{$block_command} = 1
    if ($block_commands{$block_command} eq 'format_raw');
}

# FIXME allow customization? (also in DocBook)
my %upper_case_commands = ( 'sc' => 1 );


# API for html formatting

sub _collect_css_element_class($$)
{
  my $self = shift;
  my $element_class = shift;

  #if (not defined($self->{'current_filename'})) {
  #  cluck "CFND";
  #}
  if (defined($self->{'css_element_class_styles'}->{$element_class})) {
    if ($self->{'document_global_context'}) {
      $self->{'document_global_context_css'}->{$element_class} = 1;
    } elsif (defined($self->{'current_filename'})) {
      $self->{'file_css'}->{$self->{'current_filename'}} = {}
        if (!$self->{'file_css'}->{$self->{'current_filename'}});
      $self->{'file_css'}->{$self->{'current_filename'}}->{$element_class} = 1;
    }
  }
}

# $classes should be an array reference or undef
sub html_attribute_class($$;$)
{
  my $self = shift;
  my $element = shift;
  my $classes = shift;

  if (defined($classes) and ref($classes) ne 'ARRAY') {
    confess("html_attribute_class: $classes not an array ref (for $element)");
  }
  if (!defined($classes) or scalar(@$classes) == 0
      # API info: get_conf() API code conforming would be:
      #  or $self->get_conf('NO_CSS')) {
        or $self->{'conf'}->{'NO_CSS'}) {
    if ($element eq 'span') {
      return '';
    } else {
      return "<$element";
    }
  }

  my $style = '';

  # API info: get_conf() API code conforming would be:
  #  if ($self->get_conf('INLINE_CSS_STYLE')) {
  if ($self->{'conf'}->{'INLINE_CSS_STYLE'}) {
    my @styles = ();
    foreach my $style_class (@$classes) {
      if (not defined($style_class)) {
        confess ("class not defined (for $element)");
      }
      if (defined($self->{'css_element_class_styles'}
                                   ->{"$element.$style_class"})) {
        push @styles,
          $self->{'css_element_class_styles'}->{"$element.$style_class"};
      }
    }
    if (scalar(@styles) >  0) {
      $style = ' style="'.join(';', @styles).'"';
    }
  } else {
    foreach my $style_class (@$classes) {
      if (not defined($style_class)) {
        confess ("class not defined (for $element)");
      }
      $self->_collect_css_element_class("$element.$style_class");
    }
  }
  my $class_str = join(' ', map {_protect_class_name($self, $_)} @$classes);
  return "<$element class=\"$class_str\"$style";
}

# for rules that cannot be collected during document output since they
# are not associated with a class attribute element setting
my %css_rules_not_collected = (
);

# returns an array of CSS element.class seen in the $FILENAME
sub html_get_css_elements_classes($;$)
{
  my $self = shift;
  my $filename = shift;

  my %css_elements_classes = %css_rules_not_collected;
  if ($self->{'document_global_context_css'}) {
    %css_elements_classes = ( %css_elements_classes,
                              %{$self->{'document_global_context_css'}} );
  }

  if (defined($filename) and $self->{'file_css'}
      and $self->{'file_css'}->{$filename}) {
    %css_elements_classes = ( %css_elements_classes,
                              %{$self->{'file_css'}->{$filename}} );
  }

  if ($css_elements_classes{'a.copiable-link'}) {
    $css_elements_classes{'span:hover a.copiable-link'} = 1;
  }

  return sort(keys(%css_elements_classes));
}

sub close_html_lone_element($$) {
  my $self = shift;
  my $html_element = shift;
  if ($self->get_conf('USE_XML_SYNTAX')) {
    return $html_element . '/>';
  }
  return $html_element .'>';
}

my $xml_numeric_entity_nbsp = '&#'.hex('00A0').';';
my $xml_named_entity_nbsp = '&nbsp;';

my $html_default_entity_nbsp = $xml_named_entity_nbsp;

sub substitute_html_non_breaking_space($$)
{
  my $self = shift;
  my $text = shift;

  # do not use get_info() as it may not be set yet
  my $non_breaking_space = $self->{'non_breaking_space'};
  # using \Q \E on the substitution leads to spurious \
  $text =~ s/\Q$html_default_entity_nbsp\E/$non_breaking_space/g;
  return $text;
}

my @image_files_extensions = ('.png', '.jpg', '.jpeg', '.gif');

# this allows init files to get the location of the image files
# which cannot be determined from the result, as the file
# location is not used in the element output.
# FIXME use filenametext or url?  url is always UTF-8 encoded
# to fit with percent encoding, filenametext uses the output
# encoding.  As a file name, filenametext could make sense,
# although the underlying character obtained with utf-8 may also
# make sense.  It is also used as the path part of a url.
# In practice, the user should check that the output encoding
# and the commands used in file names match, so url or
# filenametext should be the same.
sub html_image_file_location_name($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my @extensions = @image_files_extensions;

  my $image_file;
  my $image_basefile;
  my $image_extension;
  # this variable is bytes encoded in the filesystem encoding
  my ($image_path, $image_path_encoding);
  if (defined($args->[0]->{'filenametext'})
      and $args->[0]->{'filenametext'} ne '') {
    $image_basefile = $args->[0]->{'filenametext'};
    my $extension;
    if (defined($args->[4]) and defined($args->[4]->{'filenametext'})) {
      $extension = $args->[4]->{'filenametext'};
      unshift @extensions, ("$extension", ".$extension");
    }
    foreach my $extension (@extensions) {
      my ($file_name, $file_name_encoding)
        = $self->encoded_input_file_name($image_basefile.$extension);
      my $located_image_path
           = $self->Texinfo::Common::locate_include_file($file_name);
      if (defined($located_image_path) and $located_image_path ne '') {
        $image_path = $located_image_path;
        $image_path_encoding = $file_name_encoding;
        # use the @-command argument and not the file found using the
        # include paths.  It is considered that the files in include paths
        # will be moved by the caller anyway.
        # If the file path found was to be used it should be decoded to perl
        # codepoints too.
        $image_file = $image_basefile.$extension;
        $image_extension = $extension;
        last;
      }
    }
    if (!defined($image_file) or $image_file eq '') {
      if (defined($extension) and $extension ne '') {
        $image_file = $image_basefile.$extension;
        $image_extension = $extension;
      } else {
        $image_file = "$image_basefile.jpg";
        $image_extension = 'jpg';
      }
    }
  }
  return ($image_file, $image_basefile, $image_extension, $image_path,
          $image_path_encoding);
}

sub css_add_info($$$;$)
{
  my $self = shift;
  my $spec = shift;
  my $css_info = shift;
  my $css_style = shift;

  if ($spec eq 'rules') {
    push @{$self->{'css_rule_lines'}}, $css_info;
  } elsif ($spec eq 'imports') {
    push @{$self->{'css_import_lines'}}, $css_info;
  } else {
    $self->{'css_element_class_styles'}->{$css_info} = $css_style;
  }
}

sub css_get_info($$;$) {
  my $self = shift;
  my $spec = shift;
  my $css_info = shift;

  if ($spec eq 'rules') {
    if (defined($self->{'css_rule_lines'})) {
      return @{$self->{'css_rule_lines'}};
    } else {
      return ();
    }
  } elsif ($spec eq 'imports') {
    if (defined($self->{'css_import_lines'})) {
      return @{$self->{'css_import_lines'}};
    } else {
      return ();
    }
  } else {
    if (defined($css_info)) {
      if ($self->{'css_element_class_styles'}->{$css_info}) {
        return $self->{'css_element_class_styles'}->{$css_info};
      } else {
        return undef;
      }
    } else {
      return { %{$self->{'css_element_class_styles'}} };
    }
  }
}

my %default_css_string_commands_conversion;
my %default_css_string_types_conversion;
my %default_css_string_formatting_references;

sub html_convert_css_string($$;$)
{
  my $self = shift;
  my $element = shift;
  my $explanation = shift;

  my $saved_commands = {};
  my $saved_types = {};
  my $saved_formatting_references = {};
  foreach my $cmdname (keys(%default_css_string_commands_conversion)) {
    $saved_commands->{$cmdname} = $self->{'commands_conversion'}->{$cmdname};
    $self->{'commands_conversion'}->{$cmdname}
      = $default_css_string_commands_conversion{$cmdname};
  }
  foreach my $type (keys(%default_css_string_types_conversion)) {
    $saved_types->{$type} = $self->{'types_conversion'}->{$type};
    $self->{'types_conversion'}->{$type}
      = $default_css_string_types_conversion{$type};
  }
  foreach my $formatting_reference
                          (keys(%default_css_string_formatting_references)) {
    $saved_formatting_references->{$formatting_reference}
      = $self->{'formatting_function'}->{$formatting_reference};
    $self->{'formatting_function'}->{$formatting_reference}
      = $default_css_string_formatting_references{$formatting_reference};
  }

  my $result = $self->convert_tree_new_formatting_context(
                                           {'type' => '_string',
                                            'contents' => [$element]},
                                            'css_string', $explanation);
  foreach my $cmdname (keys (%default_css_string_commands_conversion)) {
    $self->{'commands_conversion'}->{$cmdname} = $saved_commands->{$cmdname};
  }
  foreach my $type (keys(%default_css_string_types_conversion)) {
    $self->{'types_conversion'}->{$type} = $saved_types->{$type};
  }
  foreach my $formatting_reference (keys(%default_css_string_formatting_references)) {
    $self->{'formatting_function'}->{$formatting_reference}
     = $saved_formatting_references->{$formatting_reference};
  }
  return $result;
}

my %special_list_mark_css_string_no_arg_command = (
# tried to use HYPHEN BULLET \2043 for use as in a bullet list, but, at least
# with my test of firefox the result is very different from a bullet.
# hyphen minus or hyphen \2010 are even smaller than hyphen bullet.
# Use the Unicode codepoint used normally for a mathematical minus \2212
# even though it is too large, since the others are too short...
# (which is actually the default, but this could change).
  #'minus' => '-',
  #'minus' => '\2010 ',
  'minus' => '\2212 ',
);

sub html_convert_css_string_for_list_mark($$;$)
{
  my $self = shift;
  my $element = shift;
  my $explanation = shift;

  my $saved_css_string_no_arg_command = {};
  foreach my $command (keys(%special_list_mark_css_string_no_arg_command)) {
    $saved_css_string_no_arg_command->{$command}
      = $self->{'no_arg_commands_formatting'}->{'css_string'}->{$command};
    $self->{'no_arg_commands_formatting'}->{'css_string'}->{$command}
      = $special_list_mark_css_string_no_arg_command{$command};
  }
  my $result = $self->html_convert_css_string($element, $explanation);
  foreach my $command (keys(%special_list_mark_css_string_no_arg_command)) {
    $self->{'no_arg_commands_formatting'}->{'css_string'}->{$command}
      = $saved_css_string_no_arg_command->{$command};
  }
  return $result;
}

# API to access converter state for conversion

sub in_math($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'math'};
}

# set if in menu or preformatted command
sub in_preformatted($)
{
  my $self = shift;
  my $context = $self->{'document_context'}->[-1]->{'composition_context'}->[-1];
  if ($preformatted_commands{$context}
      or $self->{'pre_class_types'}->{$context}
      or ($block_commands{$context}
          and $block_commands{$context} eq 'menu'
          and $self->_in_preformatted_in_menu())) {
    return $context;
  } else {
    return undef;
  }
}

sub in_upper_case($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                              ->{'upper_case'};
}

sub in_non_breakable_space($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                         ->{'no_break'};
}

sub in_space_protected($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                         ->{'space_protected'};
}

sub in_code($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'monospace'}->[-1];
}

sub in_string($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'string'};
}

sub in_verbatim($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'verbatim'};
}

sub in_raw($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'raw'};
}

sub in_multi_expanded($)
{
  my $self = shift;
  if (scalar(@{$self->{'multiple_pass'}})) {
    return $self->{'multiple_pass'}->[-1];
  }
  return undef;
}

sub paragraph_number($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                     ->{'paragraph_number'};
}

sub preformatted_number($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                  ->{'preformatted_number'};
}

sub count_elements_in_filename($$$)
{
  my $self = shift;
  my $spec = shift;
  my $filename = shift;

  if ($spec eq 'total') {
    if (defined($self->{'elements_in_file_count'}->{$filename})) {
      return $self->{'elements_in_file_count'}->{$filename};
    }
  } elsif ($spec eq 'remaining') {
    if (defined($self->{'file_counters'}->{$filename})) {
      return $self->{'file_counters'}->{$filename};
    }
  } elsif ($spec eq 'current') {
    if (defined($self->{'file_counters'}->{$filename})) {
      return $self->{'elements_in_file_count'}->{$filename}
                - $self->{'file_counters'}->{$filename} +1;
    }
  }
  return undef;
}

sub top_block_command($)
{
  my $self = shift;
  return $self->{'document_context'}->[-1]->{'block_commands'}->[-1];
}

sub preformatted_classes_stack($)
{
  my $self = shift;
  return @{$self->{'document_context'}->[-1]->{'preformatted_classes'}};
}

sub in_align($)
{
  my $self = shift;
  my $context
       = $self->{'document_context'}->[-1]->{'composition_context'}->[-1];
  if ($HTML_align_commands{$context}) {
    return $context;
  } else {
    return undef;
  }
}

sub is_format_expanded($$)
{
  my $self = shift;
  my $format = shift;

  return $self->{'expanded_formats_hash'}->{$format};
}

# the main data structure of the element target API is a hash reference, called
# the target information.
# The 'target' and 'filename' keys should be set for every type of element,
# but the other keys will only be set on some elements.
#
# The following keys can be set:
#
# Strings
#
#   'target': A unique string representing the target.  Used as argument to
#             'id' attribute.
#   'contents_target': A unique string representing the target to the location
#                      of the element in the table of content.
#   'shortcontents_target': A unique string representing the target to the
#                      location of the element in the short table of contents
#   'node_filename': the file name deriving from the element node name
#   'section_filename': the file name deriving from the element section name
#   'special_element_filename': the file name of special elements
#                               (separate contents, about...)
#   'filename': the file name the element content is output to
#   'text', 'text_nonumber': a textual representation of the element where
#              there is no restriction on the text formatting (ie HTML elements
#              can be used).
#              With _nonumber, no section number.
#   'string', 'string_nonumber': a textual representation of the element with
#                   restrictions on the available formatting, in practice no
#                   HTML elements, only entities to be able to use in attributes.
#                   With _nonumber, no section number.
#
# Other types
#
#   'tree', 'tree_nonumber: a Texinfo tree element which conversion should
#                   correspond to the element name.
#                   With _nonumber, no section number.
#   'node_command': the node element associated with the target element.
#   'root_element_command': the command associated to the top level element
#                           associated with the target element.
#
# Some functions cache their results in these hashes.

# $COMMAND should be a tree element which is a possible target of a link.
# return the target information.
sub _get_target($$)
{
  my $self = shift;
  my $command = shift;
  my $target;
  if (!defined($command)) {
    cluck("_get_target command not defined");
  }
  if ($self->{'targets'}->{$command}) {
    $target = $self->{'targets'}->{$command};
  } elsif ($command->{'cmdname'}
    # This should only happen for @*heading*, root_commands targets should
    # already be set.
            and $sectioning_heading_commands{$command->{'cmdname'}}
            and !$root_commands{$command->{'cmdname'}}) {
    $target = $self->_new_sectioning_command_target($command);
  }
  return $target;
}

# API for links and elements directions formatting

# This returns the id specific of the $COMMAND tree element
sub command_id($$)
{
  my $self = shift;
  my $command = shift;
  my $target = $self->_get_target($command);
  if ($target) {
    return $target->{'target'};
  } else {
    return undef;
  }
}

sub command_contents_target($$$)
{
  my $self = shift;
  my $command = shift;
  my $contents_or_shortcontents = shift;
  $contents_or_shortcontents = 'shortcontents'
    if ($contents_or_shortcontents eq 'summarycontents');

  my $target = $self->_get_target($command);
  if ($target) {
    return $target->{$contents_or_shortcontents .'_target'};
  } else {
    return undef;
  }
}

sub _get_footnote_location_target($$)
{
  my $self = shift;
  my $command = shift;

  if (defined($self->{'special_targets'})
      and defined($self->{'special_targets'}->{'footnote_location'})
      and defined($self->{'special_targets'}->{'footnote_location'}->{$command})) {
    return $self->{'special_targets'}->{'footnote_location'}->{$command};
  }
  return undef;
}

sub footnote_location_target($$)
{
  my $self = shift;
  my $command = shift;

  my $footnote_location_special_target = _get_footnote_location_target($self,
                                                                   $command);
  if (defined($footnote_location_special_target)) {
    return $footnote_location_special_target->{'target'};
  }
}

sub command_filename($$)
{
  my $self = shift;
  my $command = shift;

  my $target = $self->_get_target($command);
  if ($target) {
    if (exists($target->{'filename'})) {
      return $target->{'filename'};
    }
    # this finds a special element for footnote command if such an element
    # exists.  This is best, the special element filename is the footnote
    # filename.
    my ($root_element, $root_command)
           = $self->_html_get_tree_root_element($command, 1);

    if (defined($root_element)
        and $root_element->{'structure'}
        and exists($root_element->{'structure'}->{'unit_filename'})) {
      $target->{'filename'}
        = $root_element->{'structure'}->{'unit_filename'};
      return $root_element->{'structure'}->{'unit_filename'};
    } else {
      $target->{'filename'} = undef;
    }
  }
  return undef;
}

sub command_root_element_command($$)
{
  my $self = shift;
  my $command = shift;

  my $target = $self->_get_target($command);
  if ($target) {
    if (not exists($target->{'root_element_command'})) {
      # in contrast with command_filename() we find the root element through
      # the location holding the @footnote command.  It is better, as the
      # footnote special element is not associated with a root command,
      # it is better to stay in the document to find a root element.
      my ($root_element, $root_command)
        = $self->_html_get_tree_root_element($command);
      if ($root_element and $root_element->{'extra'}) {
        $target->{'root_element_command'}
          = $root_element->{'extra'}->{'unit_command'};
      } else {
        $target->{'root_element_command'} = undef;
      }
    }
    return $target->{'root_element_command'};
  }
  return undef;
}

sub tree_unit_element_command($$)
{
  my $self = shift;
  my $element = shift;

  if ($element and $element->{'extra'}) {
    if ($element->{'extra'}->{'unit_command'}) {
      return $element->{'extra'}->{'unit_command'};
    } elsif (defined($element->{'type'})
             and $element->{'type'} eq 'special_element') {
      return $element;
    }
  }
  return undef;
}

sub command_node($$)
{
  my $self = shift;
  my $command = shift;

  my $target = $self->_get_target($command);
  if ($target) {
    if (not exists($target->{'node_command'})) {
      # this finds a special element for footnote command if
      # such an element exists
      my ($root_element, $root_command)
           = $self->_html_get_tree_root_element($command, 1);
      if (defined($root_command)) {
        if ($root_command->{'cmdname'} and $root_command->{'cmdname'} eq 'node') {
          $target->{'node_command'} = $root_command;
        }
        if ($root_command->{'extra'}
            and $root_command->{'extra'}->{'associated_node'}) {
          $target->{'node_command'}
                = $root_command->{'extra'}->{'associated_node'};
        }
      } else {
        $target->{'node_command'} = undef;
      }
    }
    return $target->{'node_command'};
  }
  return undef;
}

# Return string for linking to $COMMAND with <a href>
sub command_href($$;$$$)
{
  my $self = shift;
  my $command = shift;
  my $source_filename = shift;
  # for messages only
  my $source_command = shift;
  # to specify explicitly the target
  my $specified_target = shift;

  $source_filename = $self->{'current_filename'} if (!defined($source_filename));

  if ($command->{'manual_content'}) {
    return $self->_external_node_href($command, $source_filename,
                                      $source_command);
  }

  my $target;
  if (defined($specified_target)) {
    $target = $specified_target;
  } else {
    my $target_command = $command;
    # for sectioning command prefer the associated node
    if ($command->{'extra'} and $command->{'extra'}->{'associated_node'}) {
      $target_command = $command->{'extra'}->{'associated_node'};
    }
    my $target_information = $self->_get_target($target_command);
    $target = $target_information->{'target'} if ($target_information);
  }
  return '' if (!defined($target));
  my $href = '';

  my $target_filename = $self->command_filename($command);
  if (!defined($target_filename)) {
    # Happens if there are no pages, for example if OUTPUT is set to ''
    # as in the test cases.  Also for things in @titlepage when
    # titlepage is not output.
    if ($self->{'tree_units'} and $self->{'tree_units'}->[0]
        and $self->{'tree_units'}->[0]->{'structure'}
        and defined($self->{'tree_units'}->[0]
                                   ->{'structure'}->{'unit_filename'})) {
      # In that case use the first page.
      $target_filename
        = $self->{'tree_units'}->[0]->{'structure'}->{'unit_filename'};
    }
  }
  if (defined($target_filename)) {
    if (!defined($source_filename)
         or $source_filename ne $target_filename) {
      $href .= $target_filename;
      # omit target if the command is an element command, there is only
      # one element in file and there is a file in the href
      my $command_root_element_command
               = $self->command_root_element_command($command);
      if (defined($source_filename)
          and defined($command_root_element_command)
          and ($command_root_element_command eq $command
            or (defined($command_root_element_command->{'extra'})
              and defined($command_root_element_command->{'extra'}
                                                       ->{'associated_section'})
              and $command_root_element_command->{'extra'}->{'associated_section'}
                    eq $command))) {
        my $count_elements_in_file
           = $self->count_elements_in_filename('total', $target_filename);
        if (defined($count_elements_in_file) and $count_elements_in_file == 1) {
          $target = '';
        }
      }
    }
  }
  $href .= '#' . $target if ($target ne '');
  return $href;
}

my %contents_command_special_element_variety = (
  'contents' => 'contents',
  'shortcontents' => 'shortcontents',
  'summarycontents' => 'shortcontents',
);

# Return string for linking to $CONTENTS_OR_SHORTCONTENTS associated
# element from $COMMAND with <a href>
sub command_contents_href($$$;$)
{
  my $self = shift;
  my $command = shift;
  my $contents_or_shortcontents = shift;
  my $source_filename = shift;

  $source_filename = $self->{'current_filename'}
    if (not defined($source_filename));

  my ($special_element_variety, $target_element, $class_base,
    $special_element_direction)
     = $self->command_name_special_element_information($contents_or_shortcontents);
  my $target
    = $self->command_contents_target($command, $contents_or_shortcontents);
  my $target_filename;
  # !defined happens when called as convert() and not output()
  if (defined($target_element)) {
    $target_filename = $self->command_filename($target_element);
  }
  my $href = '';
  if (defined($target_filename) and
      (!defined($source_filename)
       or $source_filename ne $target_filename)) {
    $href .= $target_filename;
  }
  $href .= '#' . $target if ($target ne '');
  return $href;
}

sub footnote_location_href($$;$$$)
{
  my $self = shift;
  my $command = shift;
  my $source_filename = shift;
  my $specified_target = shift;
  my $target_filename = shift;

  $source_filename = $self->{'current_filename'}
    if (not defined($source_filename));

  my $special_target = _get_footnote_location_target($self, $command);
  my $target = '';
  if (defined($specified_target)) {
    $target = $specified_target;
  } elsif (defined($special_target)) {
    $target = $special_target->{'target'};
  }
  # In the default footnote formatting functions, which calls
  # footnote_location_href, the target file is always known as the
  # footnote in the document appears before the footnote text formatting.
  # $target_filename is therefore always defined.  It is a good thing
  # for the case of @footnote being formatted more than once (in multiple
  # @insertcopying for instance) as the file found just below may not be the
  # correct one in such a case.
  if (not defined($target_filename)) {
    if (defined($special_target) and defined($special_target->{'filename'})) {
      $target_filename = $special_target->{'filename'};
    } else {
      # in contrast with command_filename() we find the location holding
      # the @footnote command, not the footnote element with footnotes
      my ($root_element, $root_command)
        = $self->_html_get_tree_root_element($command);
      if (defined($root_element)) {
        if (not defined($special_target)) {
          $self->{'special_targets'}->{'footnote_location'}->{$command} = {};
          $special_target
            = $self->{'special_targets'}->{'footnote_location'}->{$command};
        }
        $special_target->{'filename'}
          = $root_element->{'structure'}->{'unit_filename'};
        $target_filename = $special_target->{'filename'};
      }
    }
  }
  my $href = '';
  if (defined($target_filename) and
      (!defined($source_filename)
       or $source_filename ne $target_filename)) {
    $href .= $target_filename;
  }
  $href .= '#' . $target if ($target ne '');
  return $href;
}

# Return text to be used for a hyperlink to $COMMAND.
# $TYPE refers to the type of value returned from this function:
#  'text' - return text
#  'tree' - return a tree
#  'tree_nonumber' - return tree representing text without a chapter number
#                    being included.
#  'string' - return simpler text that can be used in element attributes
sub command_text($$;$)
{
  my $self = shift;
  my $command = shift;
  my $type = shift;

  if (!defined($type)) {
    $type = 'text';
  }
  if (!defined($command)) {
    cluck "in command_text($type) command not defined";
  }

  if ($command->{'manual_content'}) {
    my $node_content = [];
    $node_content = $command->{'node_content'}
      if (defined($command->{'node_content'}));
    my $tree;
    if ($command->{'manual_content'}) {
      $tree = {'type' => '_code',
          'contents' => [{'text' => '('}, @{$command->{'manual_content'}},
                         {'text' => ')'}, @$node_content]};
    } else {
      $tree = {'type' => '_code',
          'contents' => $node_content};
    }
    if ($type eq 'tree') {
      return $tree;
    } else {
      if ($type eq 'string') {
        $tree = {'type' => '_string',
                 'contents' => [$tree]};
      }
      my $result = $self->convert_tree_new_formatting_context(
        # FIXME check if $document_global_context argument is really needed
            $tree, $command->{'cmdname'}, 'command_text manual_content');
      return $result;
    }
  }

  my $target = $self->_get_target($command);
  if ($target) {
    my $explanation;
    $explanation = "command_text:$type \@$command->{'cmdname'}"
       if ($command->{'cmdname'});
    if (defined($target->{$type})) {
      return $target->{$type};
    }
    my $tree;
    if (!$target->{'tree'}) {
      if (defined($command->{'type'})
          and $command->{'type'} eq 'special_element') {
        my $special_element_variety
           = $command->{'extra'}->{'special_element_variety'};
        $tree
          = $self->special_element_info('heading_tree',
                                        $special_element_variety);
        $tree = {} if (!defined($tree));
        $explanation = "command_text $special_element_variety";
      } elsif ($command->{'cmdname'} and ($command->{'cmdname'} eq 'node'
                                          or $command->{'cmdname'} eq 'anchor')) {
        # FIXME is it possible not to have contents (nor args)?
        $tree = {'type' => '_code',
                 'contents' => $command->{'args'}->[0]->{'contents'}};
      } elsif ($command->{'cmdname'} and ($command->{'cmdname'} eq 'float')) {
        $tree = $self->float_type_number($command);
      } elsif ($command->{'extra'}
               and $command->{'extra'}->{'missing_argument'}) {
        if ($type eq 'tree' or $type eq 'tree_nonumber') {
          return {};
        } else {
          return '';
        }
      } else {
        my $section_arg_contents = [];
        $section_arg_contents = $command->{'args'}->[0]->{'contents'}
          if $command->{'args'}->[0]->{'contents'};
        if ($command->{'structure'}
            and defined($command->{'structure'}->{'section_number'})
            and ($self->get_conf('NUMBER_SECTIONS')
                 or !defined($self->get_conf('NUMBER_SECTIONS')))) {
          if ($command->{'cmdname'} eq 'appendix'
              and $command->{'structure'}->{'section_level'} == 1) {
            $tree = $self->gdt('Appendix {number} {section_title}',
                    {'number' => {'text' => $command->{'structure'}
                                                    ->{'section_number'}},
                     'section_title'
                                => {'contents' => $section_arg_contents}});
          } else {
            # TRANSLATORS: numbered section title
            $tree = $self->gdt('{number} {section_title}',
                     {'number' => {'text' => $command->{'structure'}
                                                       ->{'section_number'}},
                     'section_title'
                         => {'contents' => $section_arg_contents}});
          }
        } else {
          $tree = {'contents' => $section_arg_contents};
        }

        $target->{'tree_nonumber'}
          = {'contents' => $section_arg_contents};
      }
      $target->{'tree'} = $tree;
    } else {
      $tree = $target->{'tree'};
    }
    return $target->{'tree_nonumber'} if ($type eq 'tree_nonumber'
                                          and $target->{'tree_nonumber'});
    return $tree if ($type eq 'tree' or $type eq 'tree_nonumber');

    $self->_new_document_context($command->{'cmdname'}, $explanation);

    if ($type eq 'string') {
      $tree = {'type' => '_string',
               'contents' => [$tree]};
    }

    if ($type =~ /^(.*)_nonumber$/) {
      $tree = $target->{'tree_nonumber'}
        if (defined($target->{'tree_nonumber'}));
    }
    $self->{'ignore_notice'}++;
    push @{$self->{'referred_command_stack'}}, $command;
    $target->{$type} = $self->_convert($tree, $explanation);
    pop @{$self->{'referred_command_stack'}};
    $self->{'ignore_notice'}--;

    $self->_pop_document_context();
    return $target->{$type};
  }
  return undef;
}

# Return the element in the tree that $LABEL refers to.
sub label_command($$)
{
  my $self = shift;
  my $label = shift;
  if (!defined($label)) {
    cluck;
  }
  if ($self->{'labels'}) {
    return $self->{'labels'}->{$label};
  }
  return undef;
}

sub special_direction_element($$)
{
  my $self = shift;
  my $direction = shift;
  return $self->{'special_elements_directions'}->{$direction};
}

sub command_name_special_element_information($$)
{
  my $self = shift;
  my $cmdname = shift;

  my $special_element_variety;
  if (exists($contents_command_special_element_variety{$cmdname})) {
    $special_element_variety
       = $contents_command_special_element_variety{$cmdname};
  } elsif ($cmdname eq 'footnote') {
    $special_element_variety = 'footnotes';
  } else {
    return (undef, undef, undef, undef);
  }
  my $special_element_direction
    = $self->special_element_info('direction', $special_element_variety);
  my $special_element
    = $self->special_direction_element($special_element_direction);
  my $class_base
    = $self->special_element_info('class', $special_element_variety);
  return ($special_element_variety, $special_element, $class_base,
          $special_element_direction);
}

sub global_direction_element($$)
{
  my $self = shift;
  my $direction = shift;
  return $self->{'global_target_elements_directions'}->{$direction};
}

sub get_element_root_command_element($$)
{
  my $self = shift;
  my $element = shift;

  my ($root_element, $root_command) = _html_get_tree_root_element($self, $element);
  if (defined($root_command)) {
    if ($self->get_conf('USE_NODES')) {
      if ($root_command->{'cmdname'} and $root_command->{'cmdname'} eq 'node') {
        return ($root_element, $root_command);
      } elsif ($root_command->{'extra'}
               and $root_command->{'extra'}->{'associated_node'}) {
        return ($root_element, $root_command->{'extra'}->{'associated_node'});
      }
    } elsif ($root_command->{'cmdname'}
             and $root_command->{'cmdname'} eq 'node'
             and $root_command->{'extra'}
             and $root_command->{'extra'}->{'associated_section'}) {
      return ($root_element, $root_command->{'extra'}->{'associated_section'});
    }
  }
  return ($root_element, $root_command);
}

my %valid_direction_return_type = (
  # a string that can be used in a href linking to the direction
  'href' => 1,
  # a string representing the direction that can be used in
  # context where only entities are available (attributes)
  'string' => 1,
  # a string representing the direction to be used in contexts
  # not restricted in term of available formatting (ie with HTML elements)
  'text' => 1,
  # Texinfo tree element representing the direction
  'tree' => 1,
  # string representing the target, typically used as id and in href
  'target' => 1,
  # same as 'text', but select node in priority
  'node' => 1,
  # same as 'text_nonumber' but select section in priority
  'section' => 1
);

foreach my $no_number_type ('text', 'tree', 'string') {
  # without section number
  $valid_direction_return_type{$no_number_type .'_nonumber'} = 1;
}

# sub from_element_direction($SELF, $DIRECTION, $TYPE, $SOURCE_ELEMENT,
#                            $SOURCE_FILENAME, $SOURCE_FOR_MESSAGES)
#
# Return text used for linking from $SOURCE_ELEMENT in direction $DIRECTION.
# The text returned depends on $TYPE.
#
# This is used both for tree unit elements and external nodes
#
# If $SOURCE_ELEMENT is undef, $self->{'current_root_element'} is used.
#
# $SOURCE_FOR_MESSAGES is an element used for messages formatting, to get a
# location in input file.  It is better to choose the node and not the
# sectioning command associated with the element, as the error messages
# are about external nodes not found.
#
# $self->{'current_root_element'} undef happens at least when there is no
# output file, or for the table of content when frames are used.  That call
# would result for instance from from_element_direction being called from
# _get_links, itself called from 'format_begin_file' which, in the default case
# points to _default_format_begin_file.
# TODO are there other cases?
sub from_element_direction($$$;$$$)
{
  my $self = shift;
  my $direction = shift;
  my $type = shift;
  my $source_element = shift;
  my $source_filename = shift;
  # for messages only
  my $source_command = shift;

  my $target_element;
  my $command;
  my $target;

  $source_element = $self->{'current_root_element'} if (!defined($source_element));
  $source_filename = $self->{'current_filename'} if (!defined($source_filename));

  if (!$valid_direction_return_type{$type}) {
    print STDERR "Incorrect type $type in from_element_direction call\n";
    return undef;
  }
  my $global_target_element = $self->global_direction_element($direction);
  if ($global_target_element) {
    $target_element = $global_target_element;
  # output TOP_NODE_UP related infos even if element is not
  # defined which should mostly correspond to cases when there is no
  # output file, for example in the tests.
  } elsif ((not defined($source_element)
            or ($source_element
                and $self->element_is_tree_unit_top($source_element)))
           and defined($self->get_conf('TOP_NODE_UP_URL'))
           and ($direction eq 'Up' or $direction eq 'NodeUp')) {
    if ($type eq 'href') {
      return $self->get_conf('TOP_NODE_UP_URL');
    } elsif ($type eq 'text' or $type eq 'node' or $type eq 'string'
                                                or $type eq 'section') {
      return $self->get_conf('TOP_NODE_UP');
    } else {
      cluck("type $type not available for TOP_NODE_UP\n");
      return '';
    }
  } elsif (not $target_element and $source_element
           and $source_element->{'structure'}
           and $source_element->{'structure'}->{'directions'}
           and $source_element->{'structure'}->{'directions'}->{$direction}) {
    $target_element
      = $source_element->{'structure'}->{'directions'}->{$direction};
  }

  if ($target_element) {
    ######## debug
    if (!$target_element->{'type'}) {
      die "No type for element_target $direction $target_element: "
       . Texinfo::Common::debug_print_element_details($target_element, 1)
       . "directions :"
           . Texinfo::Structuring::print_element_directions($source_element);
    }
    ########
    if ($target_element->{'type'} eq 'external_node') {
      my $external_node = $target_element->{'extra'};
      #print STDERR "FROM_ELEMENT_DIRECTION ext node $type $direction\n"
      #  if ($self->get_conf('DEBUG'));
      if ($type eq 'href') {
        return $self->command_href($external_node, $source_filename,
                                   $source_command);
      } elsif ($type eq 'text' or $type eq 'node') {
        return $self->command_text($external_node);
      } elsif ($type eq 'string') {
        return $self->command_text($external_node, $type);
      }
    } elsif ($type eq 'node') {
      if ($target_element->{'extra'}
          and $target_element->{'extra'}->{'unit_command'}) {
        if ($target_element->{'extra'}->{'unit_command'}->{'cmdname'} eq 'node') {
          $command = $target_element->{'extra'}->{'unit_command'};
        } elsif ($target_element->{'extra'}->{'unit_command'}->{'extra'}
                 and $target_element->{'extra'}->{'unit_command'}
                                          ->{'extra'}->{'associated_node'}) {
          $command = $target_element->{'extra'}->{'unit_command'}
                                          ->{'extra'}->{'associated_node'};
        }
      }
      $target = $self->{'targets'}->{$command} if ($command);
      $type = 'text';
    } elsif ($type eq 'section') {
      if ($target_element->{'extra'}
          and $target_element->{'extra'}->{'unit_command'}) {
        if ($target_element->{'extra'}->{'unit_command'}->{'cmdname'} ne 'node') {
          $command = $target_element->{'extra'}->{'unit_command'};
        } elsif ($target_element->{'extra'}->{'unit_command'}->{'extra'}
                 and $target_element->{'extra'}->{'unit_command'}
                                         ->{'extra'}->{'associated_section'}) {
          $command = $target_element->{'extra'}->{'unit_command'}
                                        ->{'extra'}->{'associated_section'};
        }
      }
      $target = $self->{'targets'}->{$command} if ($command);
      $type = 'text_nonumber';
    } else {
      if (defined($target_element->{'type'})
          and $target_element->{'type'} eq 'special_element') {
        $command = $target_element;
      } elsif ($target_element->{'extra'}) {
        $command = $target_element->{'extra'}->{'unit_command'};
      }
      if ($type eq 'href') {
        if (defined($command)) {
          return $self->command_href($command, $source_filename);
        } else {
          return '';
        }
      }
      $target = $self->{'targets'}->{$command} if ($command);
    }
  } elsif ($self->special_direction_element($direction)) {
    $target_element = $self->special_direction_element($direction);
    $command = $target_element;
    if ($type eq 'href') {
      return $self->command_href($target_element, $source_filename);
    }
    $target = $self->{'targets'}->{$target_element};
  } else {
    return undef;
  }

  if ($target and exists($target->{$type})) {
    return $target->{$type};
  } elsif ($type eq 'target') {
    return undef;
  } elsif ($command) {
    #print STDERR "FROM_ELEMENT_DIRECTION $type $direction\n"
    #  if ($self->get_conf('DEBUG'));
    return $self->command_text($command, $type);
  }
}


my %valid_direction_string_type = (
  # accesskey associated to the direction
  'accesskey' => 1,
  # direction button name
  'button' => 1,
  # description of the direction
  'description' => 1,
  # section number corresponding to the example in About text
  'example' => 1,
  # rel/ref string associated to the direction
  'rel' => 1,
  # few words text associated to the direction
  'text' => 1,
);

my %valid_direction_string_context = (
  'normal' => 1,
  'string' => 1,
);

my %direction_type_translation_context = (
  'button' => 'button label',
  'description' => 'description',
  'text' => 'string',
);

sub direction_string($$$;$)
{
  my $self = shift;
  my $direction = shift;
  my $string_type = shift;
  my $context = shift;

  if (!$valid_direction_string_type{$string_type}) {
    print STDERR "Incorrect type $string_type in direction_string call\n";
    return undef;
  }

  $context = 'normal' if (!defined($context));

  if (!$valid_direction_string_context{$context}) {
    print STDERR "Incorrect context $context in direction_string call\n";
    return undef;
  }

  $direction =~ s/^FirstInFile//;

  if (not exists($self->{'directions_strings'}->{$string_type}->{$direction})
      or not exists($self->{'directions_strings'}->{$string_type}
                                                 ->{$direction}->{$context})) {
    $self->{'directions_strings'}->{$string_type}->{$direction} = {}
      if not exists($self->{'directions_strings'}->{$string_type}->{$direction});
    my $translated_directions_strings = $self->{'translated_direction_strings'};
    if (defined($translated_directions_strings->{$string_type}
                                                ->{$direction}->{'converted'})) {
      # translate already converted direction strings
      my $converted_directions
       = $translated_directions_strings->{$string_type}->{$direction}->{'converted'};
      my $context_converted_string;
      if ($converted_directions->{$context}) {
        $context_converted_string = $converted_directions->{$context};
      } elsif ($context eq 'string'
               and defined($converted_directions->{'normal'})) {
        $context_converted_string = $converted_directions->{'normal'};
      }
      if (defined($context_converted_string)) {
        my $result_string
          = $self->gdt($context_converted_string, undef, undef, 'translated_text');
        $self->{'directions_strings'}->{$string_type}->{$direction}->{$context}
          = $self->substitute_html_non_breaking_space($result_string);
      } else {
        $self->{'directions_strings'}->{$string_type}->{$direction}->{$context}
          = undef;
      }
    } elsif (defined($translated_directions_strings->{$string_type}
                                            ->{$direction}->{'to_convert'})) {
      # translate direction strings that need to be translated and converted
      my $context_string = $direction;
      $context_string .= ' (current section)' if ($direction eq 'This');
      $context_string = $context_string.' direction '
                       .$direction_type_translation_context{$string_type};
      my $translated_tree
        = $self->pgdt($context_string,
                      $translated_directions_strings->{$string_type}
                                            ->{$direction}->{'to_convert'});
      my $converted_tree;
      if ($context eq 'string') {
        $converted_tree = {
              'type' => '_string',
              'contents' => [$translated_tree]};
      } else {
        $converted_tree = $translated_tree;
      }
      my $result_string = $self->convert_tree_new_formatting_context($converted_tree,
                             "direction $direction", undef, "direction $direction");
      $self->{'directions_strings'}->{$string_type}->{$direction}->{$context}
        = $result_string;
    } else {
      # FIXME or ''
      $self->{'directions_strings'}->{$string_type}->{$direction}->{$context}
         = undef;
    }
  }
  return $self->{'directions_strings'}->{$string_type}->{$direction}->{$context};
}

my %default_translated_special_element_info;

# if SPECIAL_ELEMENT_VARIETY is not set, return all the varieties
sub special_element_info($$;$) {
  my $self = shift;
  my $type = shift;
  my $special_element_variety = shift;

  if ($self->{'translated_special_element_info'}->{$type}) {
    my $translated_special_element_info
      = $self->{'translated_special_element_info'}->{$type}->[1];
    if (not defined($special_element_variety)) {
      return sort(keys(%{$translated_special_element_info}));
    }

    if (not exists($self->{'special_element_info'}->{$type}
                                    ->{$special_element_variety})) {
      my $special_element_info_string = $translated_special_element_info
                                            ->{$special_element_variety};
      my $translated_tree;
      if (defined($special_element_info_string)) {
        my $translation_context = "$special_element_variety section heading";
        $translated_tree = $self->pgdt($translation_context,
                                       $special_element_info_string);
      }
      $self->{'special_element_info'}->{$type}->{$special_element_variety}
        = $translated_tree;
    }
  }
  if (not defined($special_element_variety)) {
    return sort(keys(%{$self->{'special_element_info'}->{$type}}));
  }
  return $self->{'special_element_info'}->{$type}->{$special_element_variety};
}

# API for misc conversion and formatting functions

# it is considered 'top' only if element corresponds to @top or
# element is a node
sub element_is_tree_unit_top($$)
{
  my $self = shift;
  my $element = shift;
  my $top_element = $self->global_direction_element('Top');
  return (defined($top_element) and $top_element eq $element
          and $element->{'extra'}
          and $element->{'extra'}->{'unit_command'}
          and ($element->{'extra'}->{'unit_command'}->{'cmdname'} eq 'node'
               or $element->{'extra'}->{'unit_command'}->{'cmdname'} eq 'top'));
}

my %default_formatting_references;
sub default_formatting_function($$)
{
  my $self = shift;
  my $format = shift;
  return $default_formatting_references{$format};
}

sub formatting_function($$)
{
  my $self = shift;
  my $format = shift;
  return $self->{'formatting_function'}->{$format};
}

my %defaults_format_special_body_contents;

sub defaults_special_element_body_formatting($$)
{
  my $self = shift;
  my $special_element_variety = shift;

  return $defaults_format_special_body_contents{$special_element_variety};
}

sub special_element_body_formatting($$)
{
  my $self = shift;
  my $special_element_variety = shift;

  return $self->{'special_element_body'}->{$special_element_variety};
}

# Return the default for the function references used for
# the formatting of commands, in case a user still wants to call
# default @-commands formatting functions when replacing functions,
# using code along
# &{$self->default_command_conversion($cmdname)}($self, $cmdname, $command, args, $content)
my %default_commands_conversion;

sub default_command_conversion($$)
{
  my $self = shift;
  my $command = shift;
  return $default_commands_conversion{$command};
}

sub command_conversion($$)
{
  my $self = shift;
  my $command = shift;
  return $self->{'commands_conversion'}->{$command};
}

my %default_commands_open;

sub default_command_open($$)
{
  my $self = shift;
  my $command = shift;
  return $default_commands_open{$command};
}

# used for customization only (in t2h_singular.init)
sub get_value($$)
{
  my $self = shift;
  my $value = shift;
  if (defined($self->{'values'})
      and exists ($self->{'values'}->{$value})) {
    return $self->{'values'}->{$value};
  } else {
    return undef;
  }
}

# $INITIALIZATION_VALUE is only used for the initialization.
# If it is not a reference, it is turned into a scalar reference.
sub shared_conversion_state($$;$)
{
  my $self = shift;
  my $state_name = shift;
  my $initialization_value = shift;

  if (not defined($self->{'shared_conversion_state'}->{$state_name})) {
    if (not ref($initialization_value)) {
      $self->{'shared_conversion_state'}->{$state_name} = \$initialization_value;
    } else {
      $self->{'shared_conversion_state'}->{$state_name} = $initialization_value;
    }
  }
  return $self->{'shared_conversion_state'}->{$state_name};
}

sub register_footnote($$$$$$$)
{
  my ($self, $command, $footid, $docid, $number_in_doc,
      $footnote_location_filename, $multi_expanded_region) = @_;
  my $in_skipped_node_top
    = $self->shared_conversion_state('in_skipped_node_top', 0);
  if ($$in_skipped_node_top != 1) {
    push @{$self->{'pending_footnotes'}}, [$command, $footid, $docid,
      $number_in_doc, $footnote_location_filename, $multi_expanded_region];
  }
}

sub get_pending_footnotes($)
{
  my $self = shift;

  my @result = @{$self->{'pending_footnotes'}};
  @{$self->{'pending_footnotes'}} = ();
  return @result;
}


# API to register, cancel and get inline content that should be output
# when in an inline situation, mostly in a paragraph or preformatted
sub register_pending_formatted_inline_content($$$)
{
  my $self = shift;
  my $category = shift;
  my $inline_content = shift;

  if (not defined($self->{'pending_inline_content'})) {
    $self->{'pending_inline_content'} = [];
  }
  push @{$self->{'pending_inline_content'}}, [$category, $inline_content];
}

# cancel only the first pending content for the category
sub cancel_pending_formatted_inline_content($$$)
{
  my $self = shift;
  my $category = shift;

  if (defined($self->{'pending_inline_content'})) {
    my @other_category_contents = ();
    while (@{$self->{'pending_inline_content'}}) {
      my $category_inline_content = pop @{$self->{'pending_inline_content'}};
      if ($category_inline_content->[0] eq $category) {
        push @{$self->{'pending_inline_content'}}, @other_category_contents;
        return $category_inline_content->[1];
      }
      unshift @other_category_contents, $category_inline_content;
    }
    push @{$self->{'pending_inline_content'}}, @other_category_contents;
  }
  return undef;
}

sub get_pending_formatted_inline_content($) {
  my $self = shift;

  if (not defined($self->{'pending_inline_content'})) {
    return '';
  } else {
    my $result = '';
    foreach my $category_inline_content (@{$self->{'pending_inline_content'}}) {
      if (defined($category_inline_content->[1])) {
        $result .= $category_inline_content->[1];
      }
    }
    $self->{'pending_inline_content'} = undef;
    return $result;
  }
}

# API to associate inline content to an element, typically
# paragraph or preformatted.  Allows to associate the pending
# content to the first inline element.
sub associate_pending_formatted_inline_content($$$) {
  my $self = shift;
  my $element = shift;
  my $inline_content = shift;

  if (not $self->{'associated_inline_content'}->{$element}) {
    $self->{'associated_inline_content'}->{$element} = '';
  }
  $self->{'associated_inline_content'}->{$element} .= $inline_content;
}

sub get_associated_formatted_inline_content($$) {
  my $self = shift;
  my $element = shift;

  if ($self->{'associated_inline_content'}->{$element}) {
    my $result = $self->{'associated_inline_content'}->{$element};
    delete $self->{'associated_inline_content'}->{$element};
    return $result;
  }
  return '';
}

# API to register an information to a file and get it.  To be able to
# set an information during conversion and get it back during headers
# and footers conversion
sub register_file_information($$;$)
{
  my $self = shift;
  my $key = shift;
  my $value = shift;

  $self->{'files_information'}->{$self->{'current_filename'}} = {}
    if (!$self->{'files_information'}->{$self->{'current_filename'}});
  $self->{'files_information'}->{$self->{'current_filename'}}->{$key} = $value;
}

sub get_file_information($$;$)
{
  my $self = shift;
  my $key = shift;
  my $filename = shift;

  if (not defined($filename)) {
    $filename = $self->{'current_filename'};
  }
  if (not defined($filename)
      or not $self->{'files_information'}
      or not $self->{'files_information'}->{$filename}
      or not exists($self->{'files_information'}->{$filename}->{$key})) {
    return (0, undef);
  }
  return (1, $self->{'files_information'}->{$filename}->{$key})
}

# information from converter available 'read-only', in general set up before
# really starting the formatting (except for current_filename).
# 'floats', 'global_commands' and 'structuring' are set up in the generic
# converter
my %available_converter_info;
foreach my $converter_info ('copying_comment', 'current_filename',
   'destination_directory', 'document_name', 'documentdescription_string',
   'floats', 'global_commands',
   'index_entries', 'index_entries_by_letter', 'indices_information',
   'jslicenses', 'line_break_element', 'non_breaking_space', 'paragraph_symbol',
   'simpletitle_command_name', 'simpletitle_tree', 'structuring',
   'title_string', 'title_tree', 'title_titlepage') {
  $available_converter_info{$converter_info} = 1;
}

sub get_info($$)
{
  my $self = shift;
  my $converter_info = shift;

  if (not $available_converter_info{$converter_info}) {
    confess("BUG: $converter_info not an available converter info");
  }
  if (defined($self->{'converter_info'}->{$converter_info})) {
    if (ref($self->{'converter_info'}->{$converter_info}) eq 'SCALAR') {
      return ${$self->{'converter_info'}->{$converter_info}};
    } else {
      return $self->{'converter_info'}->{$converter_info};
    }
  #} else {
  #  cluck();
  }
}

# This function should be used in formatting functions when some
# Texinfo tree need to be converted.
sub convert_tree_new_formatting_context($$;$$$)
{
  my $self = shift;
  my $tree = shift;
  my $context_string = shift;
  my $multiple_pass = shift;
  my $document_global_context = shift;

  my $context_string_str = '';
  if (defined($context_string)) {
    $self->_new_document_context($context_string, $document_global_context);
    $context_string_str = "C($context_string)";
  }
  my $multiple_pass_str = '';
  if ($multiple_pass) {
    $self->{'ignore_notice'}++;
    push @{$self->{'multiple_pass'}}, $multiple_pass;
    $multiple_pass_str = '|M'
  }
  print STDERR "new_fmt_ctx ${context_string_str}${multiple_pass_str}\n"
        if ($self->get_conf('DEBUG'));
  my $result = $self->convert_tree($tree, "new_fmt_ctx ${context_string_str}");
  if (defined($context_string)) {
    $self->_pop_document_context();
  }
  if ($multiple_pass) {
    $self->{'ignore_notice'}--;
    pop @{$self->{'multiple_pass'}};
  }
  return $result;
}

my %defaults = (
  'AVOID_MENU_REDUNDANCY' => 0,
  'BIG_RULE'              => '<hr>',
  'BODYTEXT'              => undef,
  'CHAPTER_HEADER_LEVEL'  => 2,
  'CLOSE_QUOTE_SYMBOL'    => undef,
  'CONTENTS_OUTPUT_LOCATION' => 'after_top',
  'CONVERT_TO_LATEX_IN_MATH' => undef,
  'COMPLEX_FORMAT_IN_TABLE' => 0,
  'COPIABLE_LINKS'        => 1,
  'DATE_IN_HEADER'        => 0,
  'DEFAULT_RULE'          => '<hr>',
  'documentlanguage'      => 'en',
  'DOCTYPE'               => '<!DOCTYPE html>',
  'DO_ABOUT'              => 0,
  'OUTPUT_CHARACTERS'     => 0,
  'EXTENSION'             => 'html',
  'EXTERNAL_CROSSREF_EXTENSION' => undef, # based on EXTENSION
  'FOOTNOTE_END_HEADER_LEVEL' => 4,
  'FOOTNOTE_SEPARATE_HEADER_LEVEL' => 4,
  'FORMAT_MENU'           => 'sectiontoc',
  'HEADERS'               => 1,
  'INDEX_ENTRY_COLON'     => '',
# if set style is added in attribute.
  'INLINE_CSS_STYLE'      => 0,
  'JS_WEBLABELS'          => 'generate',
  'JS_WEBLABELS_FILE'     => 'js_licenses.html', # no clash with node name
  'MAX_HEADER_LEVEL'      => 4,
  'MENU_ENTRY_COLON'      => ':',
  'MENU_SYMBOL'           => undef,
  'MONOLITHIC'            => 1,
  'NO_CUSTOM_HTML_ATTRIBUTE' => 0,
# if set, no css is used.
  'NO_CSS'                => 0,
  'NO_NUMBER_FOOTNOTE_SYMBOL' => '*',
  'NODE_NAME_IN_MENU'     => 1,
  'OPEN_QUOTE_SYMBOL'     => undef,
  'OUTPUT_ENCODING_NAME'  => 'utf-8',
  'SECTION_NAME_IN_TITLE' => 0,
  'SHORT_TOC_LINK_TO_TOC' => 1,
  'SHOW_TITLE'            => undef,
  'SPLIT'                 => 'node',
  'TOP_FILE'              => 'index.html', # ignores EXTENSION
  'TOP_NODE_FILE_TARGET'  => 'index.html', # ignores EXTENSION
  'USE_ACCESSKEY'         => 1,
  'USE_ISO'               => 1,
  'USE_LINKS'             => 1,
  'USE_NODES'             => 1,
  'USE_NODE_DIRECTIONS'   => undef,
  'USE_REL_REV'           => 1,
  'USE_TITLEPAGE_FOR_TITLE' => 1,
  'WORDS_IN_PAGE'         => 300,
  'XREF_USE_NODE_NAME_ARG' => undef,
  'XREF_USE_FLOAT_LABEL'   => 0,
  'xrefautomaticsectiontitle' => 'on',

  # obsolete
  'FRAMESET_DOCTYPE'      => '<!DOCTYPE html>',

  # Non-string customization variables
  # _default_panel_button_dynamic_direction use nodes direction based on USE_NODE_DIRECTIONS
  # or USE_NODES if USE_NODE_DIRECTIONS is undefined
  'SECTION_BUTTONS'      => [[ 'Next', \&_default_panel_button_dynamic_direction ],
                             [ 'Prev', \&_default_panel_button_dynamic_direction ],
                             [ 'Up', \&_default_panel_button_dynamic_direction ], ' ',
                             'Contents', 'Index', 'About'],
  'SECTION_FOOTER_BUTTONS' => [[ 'Next', \&_default_panel_button_dynamic_direction_section_footer ],
                              [ 'Prev', \&_default_panel_button_dynamic_direction_section_footer ],
                              [ 'Up', \&_default_panel_button_dynamic_direction_section_footer ], ' ',
                              'Contents', 'Index'],
  'LINKS_BUTTONS'        => ['Top', 'Index', 'Contents', 'About',
                              'NodeUp', 'NodeNext', 'NodePrev'],
  'NODE_FOOTER_BUTTONS'  => [[ 'Next', \&_default_panel_button_dynamic_direction_node_footer ],
                             [ 'Prev', \&_default_panel_button_dynamic_direction_node_footer ],
                             [ 'Up', \&_default_panel_button_dynamic_direction_node_footer ],
                             ' ', 'Contents', 'Index'],
  'ACTIVE_ICONS'         => undef,
  'PASSIVE_ICONS'        => undef,
  # obsolete
  'frame_pages_file_string' => {
                              'Frame' => '_frame',
                              'Toc_Frame' => '_toc_frame',
                              },

  # non-customization variable converter defaults
  'converted_format'   => 'html',
);

foreach my $buttons ('CHAPTER_BUTTONS', 'MISC_BUTTONS', 'TOP_BUTTONS') {
  $defaults{$buttons} = [@{$defaults{'SECTION_BUTTONS'}}];
}

foreach my $buttons ('CHAPTER_FOOTER_BUTTONS') {
  $defaults{$buttons} = [@{$defaults{'SECTION_FOOTER_BUTTONS'}}];
}


my %default_special_element_info = (

  'class' => {
    'about'       => 'about',
    'contents'    => 'contents',
    'shortcontents'    => 'shortcontents',
    'footnotes'   => 'footnotes',
  },

  'direction' => {
     'about'       => 'About',
     'contents'    => 'Contents',
     'shortcontents'    => 'Overview',
     'footnotes'   => 'Footnotes',
   },

   'order' => {
     'contents' => 30,
     'shortcontents' => 20,
     'footnotes' => 10,
     'about' => 40,
   },

   'file_string' => {
     'contents' => '_toc',
     'shortcontents' => '_ovr',
     'footnotes' => '_fot',
     'about' => '_abt',
   },

   'target' => {
     'shortcontents' => 'SEC_Shortcontents',
     'contents' => 'SEC_Contents',
     'footnotes' => 'SEC_Footnotes',
     'about' => 'SEC_About',
   },
);

# translation context should be consistent with special_element_info()
%default_translated_special_element_info = (

   'heading' => {
     'about'       => Texinfo::Common::pgdt('about section heading',
                                            'About This Document'),
     'contents'    => Texinfo::Common::pgdt('contents section heading',
                                            'Table of Contents'),
     'shortcontents' => Texinfo::Common::pgdt(
                                           'shortcontents section heading',
                                           'Short Table of Contents'),
     'footnotes'   => Texinfo::Common::pgdt('footnotes section heading',
                                            'Footnotes'),
   },
);

my @global_directions = ('First', 'Last', 'Index', 'Top');
my %global_and_special_directions;
foreach my $global_direction (@global_directions) {
  $global_and_special_directions{$global_direction} = 1;
}
foreach my $special_direction (values(
                   %{$default_special_element_info{'direction'}})) {
  $global_and_special_directions{$special_direction} = 1;
}

my %default_converted_directions_strings = (

  # see http://www.w3.org/TR/REC-html40/types.html#type-links
  'rel' =>
   {
     'Top',         'start',
     'Contents',    'contents',
     'Overview',    '',
     'Index',       'index',
     'This',        '',
     'Back',        'prev',
     'FastBack',    '',
     'Prev',        'prev',
     'Up',          'up',
     'Next',        'next',
     'NodeUp',      'up',
     'NodeNext',    'next',
     'NodePrev',    'prev',
     'NodeForward', '',
     'NodeBack',    '',
     'Forward',     'next',
     'FastForward', '',
     'About' ,      'help',
     'First',       '',
     'Last',        '',
     'NextFile',    'next',
     'PrevFile',    'prev',
   },

  'accesskey' =>
   {
     'Top',         '',
     'Contents',    '',
     'Overview',    '',
     'Index',       '',
     'This',        '',
     'Back',        'p',
     'FastBack',    '',
     'Prev',        'p',
     'Up',          'u',
     'Next',        'n',
     'NodeUp',      'u',
     'NodeNext',    'n',
     'NodePrev',    'p',
     'NodeForward', '',
     'NodeBack',    '',
     'Forward',     'n',
     'FastForward', '',
     'About' ,      '',
     'First',       '',
     'Last',        '',
     'NextFile',    '',
     'PrevFile',    '',
   },

  'example' =>
   {
     'Top',         ' '.$html_default_entity_nbsp.' ',
     'Contents',    ' '.$html_default_entity_nbsp.' ',
     'Overview',    ' '.$html_default_entity_nbsp.' ',
     'Index',       ' '.$html_default_entity_nbsp.' ',
     'This',        '1.2.3',
     'Back',        '1.2.2',
     'FastBack',    '1',
     'Prev',        '1.2.2',
     'Up',          '1.2',
     'Next',        '1.2.4',
     'NodeUp',      '1.2',
     'NodeNext',    '1.2.4',
     'NodePrev',    '1.2.2',
     'NodeForward', '1.2.4',
     'NodeBack',    '1.2.2',
     'Forward',     '1.2.4',
     'FastForward', '2',
     'About',       ' '.$html_default_entity_nbsp.' ',
     'First',       '1.',
     'Last',        '1.2.4',
     'NextFile',    ' '.$html_default_entity_nbsp.' ',
     'PrevFile',    ' '.$html_default_entity_nbsp.' ',
   },
);

# translation contexts should be consistent with
# %direction_type_translation_context.  If the direction is not used
# as is, it should also be taken into account in direction_string().
# For now 'This' becomes 'This (current section)'.
my %default_translated_directions_strings = (
  'text' => {
     ' ' =>           {'converted' => ' '.$html_default_entity_nbsp.' '},
     'Top' =>         {'to_convert'
        => Texinfo::Common::pgdt('Top direction string', 'Top')},
     'Contents' =>    {'to_convert'
        => Texinfo::Common::pgdt('Contents direction string', 'Contents')},
     'Overview' =>    {'to_convert'
        => Texinfo::Common::pgdt(
                          'Overview direction string', 'Overview')},
     'Index' =>       {'to_convert'
        => Texinfo::Common::pgdt('Index direction string', 'Index')},
     'This' =>        {'to_convert'
        => Texinfo::Common::pgdt('This (current section) direction string',
                                 'current')},
     'Back' =>        {'converted' => ' &lt; '},
     'FastBack' =>    {'converted' => ' &lt;&lt; '},
     'Prev' =>        {'to_convert'
        => Texinfo::Common::pgdt('Prev direction string', 'Prev')},
     'Up' =>          {'to_convert'
        => Texinfo::Common::pgdt('Up direction string', ' Up ')},
     'Next' =>        {'to_convert'
        => Texinfo::Common::pgdt('Next direction string', 'Next')},
     'NodeUp' =>      {'to_convert'
        => Texinfo::Common::pgdt('NodeUp direction string', 'Up')},
     'NodeNext' =>    {'to_convert'
        => Texinfo::Common::pgdt('NodeNext direction string', 'Next')},
     'NodePrev' =>    {'to_convert'
        => Texinfo::Common::pgdt('NodePrev direction string', 'Previous')},
     'NodeForward' => {'to_convert'
        => Texinfo::Common::pgdt('NodeForward direction string', 'Forward node')},
     'NodeBack' =>    {'to_convert'
        => Texinfo::Common::pgdt('NodeBack direction string', 'Back node')},
     'Forward' =>     {'converted' => ' &gt; '},
     'FastForward' => {'converted' => ' &gt;&gt; '},
     'About' =>       {'converted' => ' ? '},
     'First' =>       {'converted' => ' |&lt; '},
     'Last' =>        {'converted' => ' &gt;| '},
     'NextFile' =>    {'to_convert'
        => Texinfo::Common::pgdt('NextFile direction string', 'Next file')},
     'PrevFile' =>    {'to_convert'
        => Texinfo::Common::pgdt('PrevFile direction string', 'Previous file')},
  },

  'description' => {
     'Top' =>         {'to_convert' => Texinfo::Common::pgdt(
                      'Top direction description', 'Cover (top) of document')},
     'Contents' =>    {'to_convert' => Texinfo::Common::pgdt(
                                            'Contents direction description',
                                            'Table of contents')},
     'Overview' =>    {'to_convert' => Texinfo::Common::pgdt(
                                              'Overview direction description',
                                              'Short table of contents')},
     'Index' =>       {'to_convert' => Texinfo::Common::pgdt(
                                       'Index direction description', 'Index')},
     'This' =>        {'to_convert' => Texinfo::Common::pgdt(
                                 'This (current section) direction description',
                                 'Current section')},
     'Back' =>        {'to_convert' => Texinfo::Common::pgdt(
                                        'Back direction description',
                                        'Previous section in reading order')},
     'FastBack' =>    {'to_convert' => Texinfo::Common::pgdt(
                              'FastBack direction description',
                              'Beginning of this chapter or previous chapter')},
     'Prev' =>        {'to_convert' => Texinfo::Common::pgdt(
                                          'Prev direction description',
                                          'Previous section on same level')},
     'Up' =>          {'to_convert' => Texinfo::Common::pgdt(
                            'Up direction description', 'Up section')},
     'Next' =>        {'to_convert' => Texinfo::Common::pgdt(
                    'Next direction description', 'Next section on same level')},
     'NodeUp' =>      {'to_convert' => Texinfo::Common::pgdt(
                                   'NodeUp direction description', 'Up node')},
     'NodeNext' =>    {'to_convert' => Texinfo::Common::pgdt(
                               'NodeNext direction description', 'Next node')},
     'NodePrev' =>    {'to_convert' => Texinfo::Common::pgdt(
                            'NodePrev direction description', 'Previous node')},
     'NodeForward' => {'to_convert' => Texinfo::Common::pgdt(
                                           'NodeForward direction description',
                                           'Next node in node reading order')},
     'NodeBack' =>    {'to_convert' => Texinfo::Common::pgdt(
                                        'NodeBack direction description',
                                        'Previous node in node reading order')},
     'Forward' =>     {'to_convert' => Texinfo::Common::pgdt(
                                               'Forward direction description',
                                               'Next section in reading order')},
     'FastForward' => {'to_convert' => Texinfo::Common::pgdt(
                            'FastForward direction description', 'Next chapter')},
     'About'  =>      {'to_convert' => Texinfo::Common::pgdt(
                            'About direction description', 'About (help)')},
     'First' =>       {'to_convert' => Texinfo::Common::pgdt(
                                          'First direction description',
                                          'First section in reading order')},
     'Last' =>        {'to_convert' => Texinfo::Common::pgdt(
                                            'Last direction description',
                                            'Last section in reading order')},
     'NextFile' =>    {'to_convert' => Texinfo::Common::pgdt(
                                             'NextFile direction description',
                                             'Forward section in next file')},
     'PrevFile' =>    {'to_convert' => Texinfo::Common::pgdt(
                                             'PrevFile direction description',
                                             'Back section in previous file')},
  },

  'button' => {
     ' ' =>           {'converted' => ' '},
     'Top' =>         {'to_convert'
        => Texinfo::Common::pgdt('Top direction button label', 'Top')},
     'Contents' =>    {'to_convert'
        => Texinfo::Common::pgdt('Contents direction button label', 'Contents')},
     'Overview' =>    {'to_convert'
        => Texinfo::Common::pgdt('Overview direction button label', 'Overview')},
     'Index' =>       {'to_convert'
        => Texinfo::Common::pgdt('Index direction button label', 'Index')},
     'This' =>        {'to_convert'
        => Texinfo::Common::pgdt('This direction button label', 'This')},
     'Back' =>        {'to_convert'
        => Texinfo::Common::pgdt('Back direction button label', 'Back')},
     'FastBack' =>    {'to_convert'
        => Texinfo::Common::pgdt('FastBack direction button label', 'FastBack')},
     'Prev' =>        {'to_convert'
        => Texinfo::Common::pgdt('Prev direction button label', 'Prev')},
     'Up' =>          {'to_convert'
        => Texinfo::Common::pgdt('Up direction button label', 'Up')},
     'Next' =>        {'to_convert'
        => Texinfo::Common::pgdt('Next direction button label', 'Next')},
     'NodeUp' =>      {'to_convert'
        => Texinfo::Common::pgdt('NodeUp direction button label', 'NodeUp')},
     'NodeNext' =>    {'to_convert'
        => Texinfo::Common::pgdt('NodeNext direction button label', 'NodeNext')},
     'NodePrev' =>    {'to_convert'
        => Texinfo::Common::pgdt('NodePrev direction button label', 'NodePrev')},
     'NodeForward' => {'to_convert'
        => Texinfo::Common::pgdt('NodeForward direction button label', 'NodeForward')},
     'NodeBack' =>    {'to_convert'
        => Texinfo::Common::pgdt('NodeBack direction button label', 'NodeBack')},
     'Forward' =>     {'to_convert'
        => Texinfo::Common::pgdt('Forward direction button label', 'Forward')},
     'FastForward' => {'to_convert'
        => Texinfo::Common::pgdt('FastForward direction button label', 'FastForward')},
     'About' =>       {'to_convert'
        => Texinfo::Common::pgdt('About direction button label', 'About')},
     'First' =>       {'to_convert'
        => Texinfo::Common::pgdt('First direction button label', 'First')},
     'Last' =>        {'to_convert'
        => Texinfo::Common::pgdt('Last direction button label', 'Last')},
     'NextFile' =>    {'to_convert'
        => Texinfo::Common::pgdt('NextFile direction button label', 'NextFile')},
     'PrevFile' =>    {'to_convert'
        => Texinfo::Common::pgdt('PrevFile direction button label', 'PrevFile')},
  }
);

sub _translate_names($)
{
  my $self = shift;
  print STDERR "\nTRANSLATE_NAMES encoding_name: "
    .$self->get_conf('OUTPUT_ENCODING_NAME')
    ." documentlanguage: ".$self->get_conf('documentlanguage')."\n"
      if ($self->get_conf('DEBUG'));

  # reset strings such that they are translated when needed.
  foreach my $string_type (keys(%default_translated_directions_strings)) {
    $self->{'directions_strings'}->{$string_type} = {};
  }

  # could also use keys of $self->{'translated_special_element_info'}
  foreach my $type (keys(%default_translated_special_element_info)) {
    $self->{'special_element_info'}->{$type.'_tree'} = {};
  }

  # delete the tree and formatted results for special elements
  # such that they are redone with the new tree when needed.
  foreach my $special_element_variety ($self->special_element_info('direction')) {
    my $special_element_direction
     = $self->special_element_info('direction', $special_element_variety);
    my $special_element
     = $self->special_direction_element($special_element_direction);
    if ($special_element and
        $self->{'targets'}->{$special_element}) {
      my $target = $self->{'targets'}->{$special_element};
      foreach my $key ('text', 'string', 'tree') {
        delete $target->{$key};
      }
    }
  }
  my %translated_commands;
  foreach my $context ('normal', 'preformatted', 'string', 'css_string') {
    foreach my $command (keys(%{$self->{'no_arg_commands_formatting'}
                                                              ->{$context}})) {
      if (defined($self->{'no_arg_commands_formatting'}
                         ->{$context}->{$command}->{'translated_converted'})
          and not $self->{'no_arg_commands_formatting'}
                                        ->{$context}->{$command}->{'unset'}) {
        $translated_commands{$command} = 1;
        $self->{'no_arg_commands_formatting'}->{$context}->{$command}->{'text'}
         = $self->gdt($self->{'no_arg_commands_formatting'}
                             ->{$context}->{$command}->{'translated_converted'},
                      undef, undef, 'translated_text');
      } elsif ($context eq 'normal') {
        my $translated_tree;
        if (defined($self->{'no_arg_commands_formatting'}
                      ->{$context}->{$command}->{'translated_to_convert'})) {
          $translated_tree = $self->gdt($self->{'no_arg_commands_formatting'}
                          ->{$context}->{$command}->{'translated_to_convert'});
        } else {
          # default translated commands
          $translated_tree
            = Texinfo::Convert::Utils::translated_command_tree($self, $command);
        }
        if (defined($translated_tree) and $translated_tree ne '') {
          $self->{'no_arg_commands_formatting'}->{$context}->{$command}->{'tree'}
            = $translated_tree;
          $translated_commands{$command} = 1;
        }
      }
    }
  }
  foreach my $command (keys(%translated_commands)) {
    $self->_complete_no_arg_commands_formatting($command, 1);
  }

  print STDERR "END TRANSLATE_NAMES\n\n" if ($self->get_conf('DEBUG'));
}

# redefined functions
#
# Texinfo::Translations::gdt redefined to call user defined function.
sub gdt($$;$$$$)
{
  my ($self, $message, $replaced_substrings, $message_context, $type, $lang) = @_;
  if (defined($self->{'formatting_function'}->{'format_translate_string'})) {
    my $format_lang = $lang;
    $format_lang = $self->get_conf('documentlanguage')
                           if ($self and !defined($format_lang));
    my $translated_string
      = &{$self->{'formatting_function'}->{'format_translate_string'}}($self,
                                 $message, $format_lang, $replaced_substrings,
                                 $message_context, $type);
    if (defined($translated_string)) {
      return $translated_string;
    }
  }
  return $self->SUPER::gdt($message, $replaced_substrings, $message_context,
                           $type, $lang);
}


sub converter_defaults($$)
{
  my $self = shift;
  my $conf = shift;
  if ($conf and defined($conf->{'TEXI2HTML'})) {
    my $default_ref = { %defaults };
    my %texi2html_defaults = %$default_ref;
    _set_variables_texi2html(\%texi2html_defaults);
    return %texi2html_defaults;
  }
  return %defaults;
}

my %css_element_class_styles = (
     %css_rules_not_collected,

     'ul.toc-numbered-mark'   => 'list-style: none',
     'pre.menu-comment-preformatted' => 'font-family: serif',
     # using display: inline is an attempt to avoid a line break when in
     # preformatted in menu.  In 2022 it does not seems to work in firefox,
     # there is still a line break.
     'pre.menu-entry-description-preformatted' => 'font-family: serif; display: inline',
     'pre.menu-preformatted'  => 'font-family: serif',
     'a.summary-letter-printindex'  => 'text-decoration: none',
     'pre.display-preformatted'     => 'font-family: inherit',
     'span.program-in-footer' => 'font-size: smaller', # used with PROGRAM_NAME_IN_FOOTER
     'span.sansserif'     => 'font-family: sans-serif; font-weight: normal',
     'span.r'             => 'font-family: initial; font-weight: normal; font-style: normal',
     'td.index-entry-level-1'  => 'padding-left: 1.5em',
     'td.index-entry-level-2'  => 'padding-left: 3.0em',
     'kbd.key'            => 'font-style: normal',
     'kbd.kbd'            => 'font-style: oblique',
     'strong.def-name'    => 'font-family: monospace; font-weight: bold; '
                            .'font-size: larger',
     'p.flushleft-paragraph'   => 'text-align:left',
     'p.flushright-paragraph'  => 'text-align:right',
     'h1.centerchap'      => 'text-align:center',
     'h2.centerchap'      => 'text-align:center',
     'h3.centerchap'      => 'text-align:center',
     'h1.settitle'        => 'text-align:center',
     'h1.shorttitlepage'  => 'text-align:center',
     'h3.subtitle'        => 'text-align:right',
     'h4.centerchap'      => 'text-align:center',
     'div.center'         => 'text-align:center',
     'blockquote.indentedblock' => 'margin-right: 0em',
     'td.printindex-index-entry'     => 'vertical-align: top',
     'td.printindex-index-section'   => 'vertical-align: top; padding-left: 1em',
     'td.printindex-index-see-also'  => 'vertical-align: top; padding-left: 1em',
     'td.menu-entry-destination'     => 'vertical-align: top',
     'td.menu-entry-description'     => 'vertical-align: top',
     'th.entries-header-printindex'  => 'text-align:left',
     'th.sections-header-printindex' => 'text-align:left; padding-left: 1em',
     'th.menu-comment'               => 'text-align:left',
     'td.category-def'               => 'text-align:right',
     'td.call-def'                   => 'text-align:left',
     'td.button-direction-about'     => 'text-align:center',
     'td.name-direction-about'       => 'text-align:center',

     # The anchor element is wrapped in a <span> rather than a block level
     # element to avoid it appearing unless the mouse pointer is directly
     # over the text, as it is annoying for anchors to flicker when
     # you are moving your pointer elsewhere. "line-height: 0em" stops the
     # invisible text from changing vertical spacing.
     'a.copiable-link' => 'visibility: hidden; '
                           .'text-decoration: none; line-height: 0em',
     'span:hover a.copiable-link'         => 'visibility: visible',
);

$css_element_class_styles{'pre.format-preformatted'}
  = $css_element_class_styles{'pre.display-preformatted'};

my %preformatted_commands_context = %preformatted_commands;
$preformatted_commands_context{'verbatim'} = 1;

my %pre_class_commands;
foreach my $preformatted_command (keys(%preformatted_commands_context)) {
  # no class for the @small* variants
  if ($small_block_associated_command{$preformatted_command}) {
    $pre_class_commands{$preformatted_command}
      = $small_block_associated_command{$preformatted_command};
  } else {
    $pre_class_commands{$preformatted_command} = $preformatted_command;
  }
}
$pre_class_commands{'menu'} = 'menu';

my %default_pre_class_types;
$default_pre_class_types{'menu_comment'} = 'menu-comment';

my %indented_preformatted_commands;
foreach my $indented_format ('example', 'display', 'lisp') {
  $indented_preformatted_commands{$indented_format} = 1;
  $indented_preformatted_commands{"small$indented_format"} = 1;

  $css_element_class_styles{"div.$indented_format"} = 'margin-left: 3.2em';
}
delete $css_element_class_styles{"div.lisp"}; # output as div.example instead

# types that are in code style in the default case.  '_code' is not
# a type that can appear in the tree built from Texinfo code, it is used
# to format a tree fragment as if it was in a @code @-command.
my %default_code_types = (
 '_code' => 1,
);

# specification of arguments formatting
my %default_commands_args = (
  'anchor' => [['monospacestring']],
  'email' => [['url', 'monospacestring'], ['normal']],
  'footnote' => [[]],
  'printindex' => [[]],
  'uref' => [['url', 'monospacestring'], ['normal'], ['normal']],
  'url' => [['url', 'monospacestring'], ['normal'], ['normal']],
  'sp' => [[]],
  'inforef' => [['monospace'],['normal'],['filenametext']],
  'xref' => [['monospace'],['normal'],['normal'],['filenametext'],['normal']],
  'pxref' => [['monospace'],['normal'],['normal'],['filenametext'],['normal']],
  'ref' => [['monospace'],['normal'],['normal'],['filenametext'],['normal']],
  'link' => [['monospace'],['normal'],['filenametext']],
  'image' => [['url', 'filenametext', 'monospacestring'],['filenametext'],['filenametext'],['string', 'normal'],['filenametext']],
  # FIXME shouldn't it better not to convert if later ignored?
  # note that right now ignored argument are in elided empty types
  # but this could change.
  'inlinefmt' => [['monospacetext'],['normal']],
  'inlinefmtifelse' => [['monospacetext'],['normal'],['normal']],
  'inlineraw' => [['monospacetext'],['raw']],
  'inlineifclear' => [['monospacetext'],['normal']],
  'inlineifset' => [['monospacetext'],['normal']],
  'item' => [[]],
  'itemx' => [[]],
  'value' => [['monospacestring']],
);

foreach my $explained_command (keys(%explained_commands)) {
  $default_commands_args{$explained_command}
     = [['normal'], ['string']];
}

# intercept warning and error messages to take 'ignore_notice' into
# account
sub _noticed_line_warn($$$)
{
  my $self = shift;
  my $text = shift;
  my $line_nr = shift;
  return if ($self->{'ignore_notice'});
  return $self->line_warn($self, $text, $line_nr);
}

my %kept_line_commands;

# TODO add the possibility to customize to add more commands to
# @informative_global_commands?
my @informative_global_commands = ('documentlanguage', 'footnotestyle',
  'xrefautomaticsectiontitle', 'deftypefnnewline');

my @contents_commands = ('contents', 'shortcontents', 'summarycontents');

foreach my $line_command (@informative_global_commands,
        @contents_commands, keys(%formattable_line_commands),
        keys(%formatted_line_commands),
        keys(%default_index_commands)) {
  $kept_line_commands{$line_command} = 1;
}

foreach my $line_command (keys(%line_commands)) {
  $default_commands_conversion{$line_command} = undef
    unless ($kept_line_commands{$line_command});
}

foreach my $nobrace_command (keys(%nobrace_commands)) {
  $default_commands_conversion{$nobrace_command} = undef
    unless ($formatted_nobrace_commands{$nobrace_command});
}

# formatted/formattable @-commands that are not converted in
# HTML in the default case.
$default_commands_conversion{'page'} = undef;
$default_commands_conversion{'need'} = undef;
$default_commands_conversion{'vskip'} = undef;

foreach my $ignored_brace_commands ('caption', 'shortcaption',
  'hyphenation', 'sortas') {
  $default_commands_conversion{$ignored_brace_commands} = undef;
}

foreach my $ignored_block_commands ('ignore', 'macro', 'rmacro', 'linemacro',
  'copying', 'documentdescription', 'titlepage', 'direntry') {
  $default_commands_conversion{$ignored_block_commands} = undef;
};

# Formatting of commands without args

# The hash holding the defaults for the formatting of
# most commands without args.  It has three contexts as keys,
# 'normal' in normal text, 'preformatted' in @example and similar
# commands, and 'string' for contexts where HTML elements should not
# be used.
my %default_no_arg_commands_formatting = (
  'normal' => {},
  'preformatted' => {},
  'string' => {},
  # more internal
  'css_string' => {},
);

foreach my $command (keys(%Texinfo::Convert::Converter::xml_text_entity_no_arg_commands_formatting)) {
  $default_no_arg_commands_formatting{'normal'}->{$command} =
 {'text' =>
  $Texinfo::Convert::Converter::xml_text_entity_no_arg_commands_formatting{
                                                                     $command}};
}

$default_no_arg_commands_formatting{'normal'}->{' '} = {'text' => '&nbsp;'};
$default_no_arg_commands_formatting{'normal'}->{"\t"} = {'text' => '&nbsp;'};
$default_no_arg_commands_formatting{'normal'}->{"\n"} = {'text' => '&nbsp;'};

# possible example of use, right now not used, as
# the generic Converter customization is directly used through
# the call to Texinfo::Convert::Utils::translated_command_tree().
#$default_no_arg_commands_formatting{'normal'}->{'error'}->{'translated_converted'} = 'error--&gt;';
## This is used to have gettext pick up the chain to be translated
#if (0) {
#  my $not_existing;
#  $not_existing->gdt('error--&gt;');
#}

$default_no_arg_commands_formatting{'normal'}->{'enddots'}
    = {'element' => 'small', 'text' => '...'};
$default_no_arg_commands_formatting{'preformatted'}->{'dots'}
    = {'text' => '...'};
$default_no_arg_commands_formatting{'preformatted'}->{'enddots'}
    = {'text' => '...'};
$default_no_arg_commands_formatting{'normal'}->{'*'} = {'text' => '<br>'};
# this is used in math too, not sure that it is the best
# in that context, '<br>' could be better.
$default_no_arg_commands_formatting{'preformatted'}->{'*'} = {'text' => "\n"};

# escaped code points in CSS
# https://www.w3.org/TR/css-syntax/#consume-escaped-code-point
# Consume as many hex digits as possible, but no more than 5. Note that this means 1-6 hex digits have been consumed in total. If the next input code point is whitespace, consume it as well. Interpret the hex digits as a hexadecimal number.
# Note that in style= HTML attributes entities are used to
# protect CSS strings.  For example, the CSS string a'b"
# is protected as CSS as a\'b", and " is escaped in an HTML style
# attribute: style="list-style-type: 'a\'b&quot;'"

foreach my $no_brace_command (keys(%nobrace_symbol_text)) {
  $default_no_arg_commands_formatting{'css_string'}->{$no_brace_command}
   = {'text' => $nobrace_symbol_text{$no_brace_command}};
}

foreach my $command (keys(%{$default_no_arg_commands_formatting{'normal'}})) {
  if (defined($Texinfo::Convert::Unicode::unicode_map{$command})
      and $Texinfo::Convert::Unicode::unicode_map{$command} ne '') {
    my $char_nr = hex($Texinfo::Convert::Unicode::unicode_map{$command});
    my $css_string;
    if ($char_nr < 128) { # 7bit ascii
      $css_string = chr($char_nr);
    } else {
      $css_string = "\\$Texinfo::Convert::Unicode::unicode_map{$command} ";
    }
    $default_no_arg_commands_formatting{'css_string'}->{$command}
       = {'text' => $css_string};
  } elsif ($default_no_arg_commands_formatting{'preformatted'}->{$command}) {
    $default_no_arg_commands_formatting{'css_string'}->{$command}
     = {'text'
        => $default_no_arg_commands_formatting{'preformatted'}->{$command}->{'text'}};
  } elsif ($default_no_arg_commands_formatting{'normal'}->{$command}->{'text'}) {
    $default_no_arg_commands_formatting{'css_string'}->{$command}
     = {'text' => $default_no_arg_commands_formatting{'normal'}->{$command}->{'text'}};
  } elsif (exists($nobrace_symbol_text{$command})
           and $nobrace_symbol_text{$command} eq '') {
    # @- @/ @/ @|
    $default_no_arg_commands_formatting{'css_string'}->{$command}
                                                    = {'text' => ''};
  } else {
    warn "BUG: $command: no css_string\n";
  }
}

# remove to force using only translations (as the command
# is in the default converter translated commands)
delete $default_no_arg_commands_formatting{'css_string'}->{'error'};

$default_no_arg_commands_formatting{'css_string'}->{'*'}->{'text'} = '\A ';

$default_no_arg_commands_formatting{'css_string'}->{' '}->{'text'} = ' ';
$default_no_arg_commands_formatting{'css_string'}->{"\t"}->{'text'} = ' ';
$default_no_arg_commands_formatting{'css_string'}->{"\n"}->{'text'} = ' ';
$default_no_arg_commands_formatting{'css_string'}->{'tie'}->{'text'} = ' ';

# w not in css_string, set the corresponding css_element_class_styles
# especially, which also has none and not w in the class
$css_element_class_styles{'ul.mark-none'} = 'list-style-type: none';

# setup css_element_class_styles for mark commands based on css strings
foreach my $mark_command (keys(%{$default_no_arg_commands_formatting{'css_string'}})) {
  if (defined($brace_commands{$mark_command})) {
    my $css_string;
    if ($mark_command eq 'bullet') {
      $css_string = 'disc';
    } elsif ($default_no_arg_commands_formatting{'css_string'}->{$mark_command}
             and $default_no_arg_commands_formatting{'css_string'}
                                                 ->{$mark_command}->{'text'}) {
      if ($special_list_mark_css_string_no_arg_command{$mark_command}) {
        $css_string = $special_list_mark_css_string_no_arg_command{$mark_command};
      } else {
        $css_string
           = $default_no_arg_commands_formatting{'css_string'}
                                             ->{$mark_command}->{'text'};
      }
      $css_string =~ s/^(\\[A-Z0-9]+) $/$1/;
      $css_string = '"'.$css_string.'"';
    }
    if (defined($css_string)) {
      $css_element_class_styles{"ul.mark-$mark_command"}
                               = "list-style-type: $css_string";
    }
  }
}

# used to show the built-in CSS rules
sub builtin_default_css_text()
{
  my $css_text = '';
  foreach my $css_rule (sort(keys(%css_element_class_styles))) {
    if ($css_element_class_styles{$css_rule} ne '') {
      $css_text .= "$css_rule {$css_element_class_styles{$css_rule}}\n";
    }
  }
  return $css_text;
}

sub _text_element_conversion($$$)
{
  my $self = shift;
  my $specification = shift;
  my $command = shift;

  my $text = '';
  # note that there could be elements in text
  if (exists($specification->{'text'})) {
    $text = $specification->{'text'};
  }

  if (exists($specification->{'element'})) {
    return $self->html_attribute_class($specification->{'element'}, [$command])
               .'>'. $text . '</'.$specification->{'element'}.'>';
  } else {
    return $text;
  }
}

sub _convert_no_arg_command($$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;

  if ($cmdname eq 'click' and $command->{'extra'}
      and exists($command->{'extra'}->{'clickstyle'})) {
    my $click_cmdname = $command->{'extra'}->{'clickstyle'};
    if (($self->in_preformatted() or $self->in_math()
         and $self->{'no_arg_commands_formatting'}->{'preformatted'}
                                                             ->{$click_cmdname})
        or ($self->in_string() and
            $self->{'no_arg_commands_formatting'}->{'string'}->{$click_cmdname})
        or ($self->{'no_arg_commands_formatting'}->{'normal'}->{$click_cmdname})) {
      $cmdname = $click_cmdname;
    }
  }
  if ($self->in_upper_case() and $letter_no_arg_commands{$cmdname}
      and $self->{'no_arg_commands_formatting'}->{'normal'}->{uc($cmdname)}) {
    $cmdname = uc($cmdname);
  }

  my $result;

  if ($self->in_preformatted() or $self->in_math()) {
    $result = $self->_text_element_conversion(
      $self->{'no_arg_commands_formatting'}->{'preformatted'}->{$cmdname},
      $cmdname);
  } elsif ($self->in_string()) {
    $result = $self->_text_element_conversion(
      $result = $self->{'no_arg_commands_formatting'}->{'string'}->{$cmdname},
      $cmdname);
  } else {
    $result = $self->_text_element_conversion(
      $result = $self->{'no_arg_commands_formatting'}->{'normal'}->{$cmdname},
      $cmdname);
  }

  return $result;
}

foreach my $command(keys(%{$default_no_arg_commands_formatting{'normal'}})) {
  $default_commands_conversion{$command} = \&_convert_no_arg_command;
}

sub _css_string_convert_no_arg_command($$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;

  if ($cmdname eq 'click' and $command->{'extra'}
      and exists($command->{'extra'}->{'clickstyle'})) {
    my $click_cmdname = $command->{'extra'}->{'clickstyle'};
    if ($self->{'no_arg_commands_formatting'}->{'css_string'}->{$click_cmdname}) {
      $cmdname = $click_cmdname;
    }
  }
  if ($self->in_upper_case() and $letter_no_arg_commands{$cmdname}
      and $self->{'no_arg_commands_formatting'}->{'css_string'}->{uc($cmdname)}) {
    $cmdname = uc($cmdname);
  }
  #if (not defined($self->{'no_arg_commands_formatting'}->{'css_string'}->{$cmdname}->{'text'})) {
  #  cluck ("BUG: CSS $cmdname no text");
  #}
  return $self->{'no_arg_commands_formatting'}->{'css_string'}
                                                 ->{$cmdname}->{'text'};
}

foreach my $command(keys(%{$default_no_arg_commands_formatting{'normal'}})) {
  $default_css_string_commands_conversion{$command} = \&_css_string_convert_no_arg_command;
}

sub _convert_today_command($$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;

  my $tree = $self->Texinfo::Convert::Utils::expand_today();
  return $self->convert_tree($tree, 'convert today');
}

$default_commands_conversion{'today'} = \&_convert_today_command;

# style commands

my %quoted_style_commands;
foreach my $quoted_command ('samp') {
  $quoted_style_commands{$quoted_command} = 1;
}

my %style_commands_element = ('preformatted' => {});
$style_commands_element{'normal'} = {
      'b'           => 'b',
      'cite'        => 'cite',
      'code'        => 'code',
      'command'     => 'code',
      'dfn'         => 'em',
      'dmn'         => 'span',
      'emph'        => 'em',
      'env'         => 'code',
      'file'        => 'samp',
      'headitemfont' => 'b', # no effect: the @multitable prototypes are ignored
                             # and headitem are in <th> rather than <td>.
                             # The mapping is based on style used in other
                             # formats.
      'i'           => 'i',
      'slanted'     => 'i',
      'sansserif'   => 'span',
      'kbd'         => 'kbd',
      'key'         => 'kbd',
      'option'      => 'samp',
      'r'           => 'span',
      'samp'        => 'samp',
      'sc'          => 'small',
      'strong'      => 'strong',
      'sub'         => 'sub',
      'sup'         => 'sup',
      't'           => 'code',
      'var'         => 'var',
      'verb'        => 'code', # other brace command
};

my %style_commands_formatting;
foreach my $formatting_context (keys(%style_commands_element)) {
  $style_commands_formatting{$formatting_context} = {};
}
$style_commands_formatting{'string'} = {};

my %style_brace_types = map {$_ => 1} ('style_other', 'style_code',
                                       'style_no_code');
# @all_style_commands is the union of style brace commands and commands
# in $style_commands_element{'normal'}, a few not being style brace commands.
# Using keys of a map generated hash does like uniq, it avoids duplicates.
# The first grep selects style brace commands, ie commands with %brace_commands
# type in %style_brace_types.
my @all_style_commands = keys %{{ map { $_ => 1 }
    ((grep {$style_brace_types{$brace_commands{$_}}} keys(%brace_commands)),
      keys(%{$style_commands_element{'normal'}})) }};

foreach my $command (@all_style_commands) {
  # default is no attribute.
  if ($style_commands_element{'normal'}->{$command}) {
    $style_commands_formatting{'normal'}->{$command}
     = {'element' => $style_commands_element{'normal'}->{$command}};
    $style_commands_formatting{'preformatted'}->{$command}
     = {'element' => $style_commands_element{'normal'}->{$command}};
  }
  if ($style_commands_element{'preformatted'}->{$command}) {
    $style_commands_formatting{'preformatted'}->{$command}
     = {'element' => $style_commands_element{'preformatted'}->{$command}};
  }
  if ($quoted_style_commands{$command}) {
    foreach my $context ('normal', 'string', 'preformatted') {
      $style_commands_formatting{$context}->{$command} = {}
        if (!$style_commands_formatting{$context}->{$command});
      $style_commands_formatting{$context}->{$command}->{'quote'} = 1;
    }
  }
  $default_commands_conversion{$command} = \&_convert_style_command;
}

$style_commands_formatting{'preformatted'}->{'sc'}->{'element'} = 'span';

# currently unused, could be re-used if there is a need to have attributes
# specified in %style_commands_element
sub _parse_attribute($)
{
  my $element = shift;
  return ('', '', '') if (!defined($element));
  my ($class, $attributes) = ('', '');
  if ($element =~ /^(\w+)(\s+.*)/)
  {
    $element = $1;
    $attributes = $2;
    if ($attributes =~ s/^\s+class=\"([^\"]+)\"//) {
      $class = $1;
    }
  }
  return ($element, $class, $attributes);
}

sub _convert_style_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $text;
  $text = $args->[0]->{'normal'} if ($args->[0]);
  if (!defined($text)) {
    # happens with bogus @-commands without argument, like @strong something
    #cluck "text not defined in _convert_style_command";
    return '';
  }
  my @classes;
  # handle the effect of kbdinputstyle
  if ($cmdname eq 'kbd' and $command->{'extra'}
      and $command->{'extra'}->{'code'}) {
    $cmdname = 'code';
    push @classes, 'as-code-kbd';
  }
  unshift @classes, $cmdname;

  my $attribute_hash = {};
  if ($self->in_preformatted()) {
    $attribute_hash = $self->{'style_commands_formatting'}->{'preformatted'};
  } elsif (!$self->in_string()) {
    $attribute_hash = $self->{'style_commands_formatting'}->{'normal'};
  }
  if (defined($attribute_hash->{$cmdname})) {
    my $attribute_text = '';
    my $style;
    if (defined($attribute_hash->{$cmdname}->{'element'})) {
      # the commented out code is useful only if there are attributes in
      # style_commands_element
      #my $class;
      #($style, $class, $attribute_text)
      #  = _parse_attribute($attribute_hash->{$cmdname}->{'element'});
      #if (defined($class) and $class ne '') {
      #  push @classes, $class;
      #}
      my $style = $attribute_hash->{$cmdname}->{'element'};
      my $open = $self->html_attribute_class($style, \@classes);
      if ($open ne '') {
        $text = $open . '>' . $text . "</$style>";
      #  $text = $open . "$attribute_text>" . $text . "</$style>";
      #} elsif ($attribute_text ne '') {
      #  $text = "<$style $attribute_text>". $text . "</$style>";
      }
    }
    if (defined($attribute_hash->{$cmdname}->{'quote'})) {
      $text = $self->get_conf('OPEN_QUOTE_SYMBOL') . $text
                . $self->get_conf('CLOSE_QUOTE_SYMBOL');
    }
  }
  return $text;
}

sub _convert_w_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $text;
  $text = $args->[0]->{'normal'} if ($args->[0]);

  if (!defined($text)) {
    $text = '';
  }
  if ($self->in_string()) {
    return $text;
  } else {
    return $text . '<!-- /@w -->';
  }
}
$default_commands_conversion{'w'} = \&_convert_w_command;

sub _convert_value_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  return $self->convert_tree($self->gdt('@{No value for `{value}\'@}',
                                  {'value' => $args->[0]->{'monospacestring'}}));
}

$default_commands_conversion{'value'} = \&_convert_value_command;

sub _convert_email_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $mail_arg = shift @$args;
  my $text_arg = shift @$args;
  my $mail = '';
  my $mail_string;
  if (defined($mail_arg)) {
    $mail = $mail_arg->{'url'};
    $mail_string = $mail_arg->{'monospacestring'};
  }
  my $text = '';
  if (defined($text_arg)) {
    $text = $text_arg->{'normal'};
  }
  $text = $mail_string unless ($text ne '');
  # match a non-space character.  Both ascii and non-ascii spaces are
  # considered as spaces.
  return $text unless ($mail =~ /[^\v\h\s]/);
  if ($self->in_string()) {
    return "$mail_string ($text)";
  } else {
    return $self->html_attribute_class('a', [$cmdname])
    .' href="'.$self->url_protect_url_text("mailto:$mail")."\">$text</a>";
  }
}

$default_commands_conversion{'email'} = \&_convert_email_command;

sub _convert_explained_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $with_explanation;
  my $explanation_result;
  my $explanation_string;
  my $normalized_type = '';
  if ($command->{'args'}->[0]
      and $command->{'args'}->[0]->{'contents'}) {
    $normalized_type = Texinfo::Convert::NodeNameNormalization::normalize_node(
          {'contents' => $command->{'args'}->[0]->{'contents'}});
  }

  my $explained_commands
    = $self->shared_conversion_state('explained_commands', {});
  $explained_commands->{$cmdname} = {} if (!$explained_commands->{$cmdname});
  my $element_explanation_contents
    = $self->shared_conversion_state('element_explanation_contents', {});
  if ($args->[1] and defined($args->[1]->{'string'})
                 and $args->[1]->{'string'} =~ /\S/) {
    $with_explanation = 1;
    $explanation_string = $args->[1]->{'string'};

    # Convert the explanation of the acronym.  Must do this before we save
    # the explanation for the future, otherwise we get infinite recursion
    # for recursively-defined acronyms.
    $explanation_result = $self->convert_tree($args->[1]->{'tree'},
                                              "convert $cmdname explanation");
    $explained_commands->{$cmdname}->{$normalized_type} =
       $command->{'args'}->[1]->{'contents'};
  } elsif ($element_explanation_contents->{$command}) {
    # if an acronym element is formatted more than once, this ensures that
    # only the first explanation (including a lack of explanation) is reused.
    # Note that this means that acronyms converted first on a sectioning
    # command line for a direction text may not get the explanation
    # from acronyms appearing later on in the document but before
    # the sectioning command.
    if (@{$element_explanation_contents->{$command}}) {
      $explanation_string = $self->convert_tree_new_formatting_context(
        {'type' => '_string',
         'contents' => $element_explanation_contents->{$command}},
        $cmdname, $cmdname);
    }
  } elsif ($explained_commands->{$cmdname}->{$normalized_type}) {
    $explanation_string = $self->convert_tree_new_formatting_context(
                      {'type' => '_string',
                       'contents' => $explained_commands
                                     ->{$cmdname}->{$normalized_type}},
                                                   $cmdname, $cmdname);

    $element_explanation_contents->{$command}
       = $explained_commands->{$cmdname}->{$normalized_type};
  } else {
    # Avoid ever giving an explanation for this element, even if an
    # explanation could appear later on, for instance if acronym is
    # formatted early on a sectioning command line and the acronym is
    # defined before the sectioning command in the document.  This prevents
    # infinite recursion for a recursively-defined acronym, when an
    # @acronym within the explanation could end up referring to the
    # containing @acronym.

    $element_explanation_contents->{$command} = [];
  }
  my $result = $args->[0]->{'normal'};
  if (!$self->in_string()) {
    my $explanation = '';
    $explanation = " title=\"$explanation_string\""
      if (defined($explanation_string));
    my $html_element = 'abbr';
    $result = $self->html_attribute_class($html_element, [$cmdname])
         ."${explanation}>".$result."</$html_element>";
  }
  if ($with_explanation) {
    # TRANSLATORS: abbreviation or acronym explanation
    $result = $self->convert_tree($self->gdt('{explained_string} ({explanation})',
          {'explained_string' => {'type' => '_converted',
                   'text' => $result},
           'explanation' => {'type' => '_converted',
                   'text' => $explanation_result}}), "convert explained $cmdname");
  }

  return $result;
}

foreach my $explained_command (keys(%explained_commands)) {
  $default_commands_conversion{$explained_command}
    = \&_convert_explained_command;
}

sub _convert_anchor_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $id = $self->command_id($command);
  if (defined($id) and $id ne '' and !$self->in_multi_expanded()
      and !$self->in_string()) {
    return &{$self->formatting_function('format_separate_anchor')}($self,
                                                           $id, 'anchor');
  }
  return '';
}

$default_commands_conversion{'anchor'} = \&_convert_anchor_command;

sub _convert_footnote_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $number_in_doc;
  my $foot_num = $self->shared_conversion_state('footnote_number', 0);
  ${$foot_num}++;
  if ($self->get_conf('NUMBER_FOOTNOTES')) {
    $number_in_doc = $$foot_num;
  } else {
    $number_in_doc = $self->get_conf('NO_NUMBER_FOOTNOTE_SYMBOL');
  }

  return "($number_in_doc)" if ($self->in_string());

  #print STDERR "FOOTNOTE $command\n";
  my $footid = $self->command_id($command);

  # happens for bogus footnotes
  if (!defined($footid)) {
    return '';
  }
  # ID for linking back to the main text from the footnote.
  my $docid = $self->footnote_location_target($command);

  my $multiple_expanded_footnote = 0;
  my $multi_expanded_region = $self->in_multi_expanded();
  if (defined($multi_expanded_region)) {
    # to avoid duplicate names, use a prefix that cannot happen in anchors
    my $target_prefix = "t_f";
    $footid = $target_prefix.$multi_expanded_region.'_'.$footid.'_'.$$foot_num;
    $docid = $target_prefix.$multi_expanded_region.'_'.$docid.'_'.$$foot_num;
  } else {
    my $footnote_id_numbers
      = $self->shared_conversion_state('footnote_id_numbers', {});
    if (!defined($footnote_id_numbers->{$footid})) {
      $footnote_id_numbers->{$footid} = $$foot_num;
    } else {
      # This should rarely happen, except for @footnote in @copying and
      # multiple @insertcopying...
      # Here it is not checked that there is no clash with another anchor.
      # However, unless there are more than 1000 footnotes this should not
      # happen.
      $footid .= '_'.$$foot_num;
      $docid .= '_'.$$foot_num;
      $multiple_expanded_footnote = 1;
    }
  }
  my $footnote_href;
  if ($self->get_conf('footnotestyle') eq 'end'
      and (defined($multi_expanded_region)
           or $multiple_expanded_footnote)) {
    # if the footnote appears multiple times, command_href() will select
    # one, but it may not be the one expanded at the location currently
    # formatted (in general the first one, but it depends if it is in a
    # tree element or not, for instance in @titlepage).
    # With footnotestyle end, considering that the footnote is in the same file
    # has a better change of being correct.
    $footnote_href = "#$footid";
  } else {
    $footnote_href = $self->command_href($command, undef, undef, $footid);
  }

  $self->register_footnote($command, $footid, $docid, $number_in_doc,
                    $self->get_info('current_filename'), $multi_expanded_region);

  my $footnote_number_text;
  if ($self->in_preformatted()) {
    $footnote_number_text = "($number_in_doc)";
  } else {
    $footnote_number_text = "<sup>$number_in_doc</sup>";
  }
  return $self->html_attribute_class('a', [$cmdname])
    ." id=\"$docid\" href=\"$footnote_href\">$footnote_number_text</a>";
}
$default_commands_conversion{'footnote'} = \&_convert_footnote_command;

sub _convert_uref_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my @args = @$args;
  my $url_arg = shift @args;
  my $text_arg = shift @args;
  my $replacement_arg = shift @args;

  my ($url, $url_string, $text, $replacement);
  if (defined($url_arg)) {
    $url = $url_arg->{'url'};
    $url_string = $url_arg->{'monospacestring'};
  }
  $text = $text_arg->{'normal'} if defined($text_arg);
  $replacement = $replacement_arg->{'normal'} if defined($replacement_arg);

  $text = $replacement if (defined($replacement) and $replacement ne '');
  $text = $url_string if (!defined($text) or $text eq '');
  return $text if (!defined($url) or $url eq '');
  return "$text ($url_string)" if ($self->in_string());

  return $self->html_attribute_class('a', [$cmdname])
           .' href="'.$self->url_protect_url_text($url)."\">$text</a>";
}

$default_commands_conversion{'uref'} = \&_convert_uref_command;
$default_commands_conversion{'url'} = \&_convert_uref_command;

sub _convert_image_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  if (defined($args->[0]->{'filenametext'})
      and $args->[0]->{'filenametext'} ne '') {
    my $basefile_string = '';
    $basefile_string = $args->[0]->{'monospacestring'}
        if (defined($args->[0]->{'monospacestring'}));
    return $basefile_string if ($self->in_string());
    my ($image_file, $image_basefile, $image_extension, $image_path)
      = $self->html_image_file_location_name($cmdname, $command, $args);
    if (not defined($image_path)) {
      $self->_noticed_line_warn(sprintf(
              __("\@image file `%s' (for HTML) not found, using `%s'"),
                 $image_basefile, $image_file), $command->{'source_info'});
    }
    if (defined($self->get_conf('IMAGE_LINK_PREFIX'))) {
      $image_file = $self->get_conf('IMAGE_LINK_PREFIX') . $image_file;
    }
    my $alt_string;
    if (defined($args->[3]) and defined($args->[3]->{'string'})) {
      $alt_string = $args->[3]->{'string'};
    }
    if (!defined($alt_string) or ($alt_string eq '')) {
      $alt_string = $basefile_string;
    }
    return $self->close_html_lone_element(
      $self->html_attribute_class('img', [$cmdname])
        . ' src="'.$self->url_protect_file_text($image_file)
        ."\" alt=\"$alt_string\"");
  }
  return '';
}

$default_commands_conversion{'image'} = \&_convert_image_command;

sub _convert_math_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $arg = $args->[0]->{'normal'};

  my $math_type = $self->get_conf('HTML_MATH');
  if ($math_type and $math_type eq 'mathjax') {
    $self->register_file_information('mathjax', 1);
    return $self->html_attribute_class('em', [$cmdname, 'tex2jax_process'])
                                          .">\\($arg\\)</em>";
  }
  return $self->html_attribute_class('em', [$cmdname]).">$arg</em>";
}

$default_commands_conversion{'math'} = \&_convert_math_command;

sub _accent_entities_html_accent($$$;$$$)
{
  my $self = shift;
  my $text = shift;
  my $command = shift;
  my $in_upper_case = shift;
  my $use_numeric_entities = shift;
  my $accent = $command->{'cmdname'};

  if ($in_upper_case and $text =~ /^\w$/) {
    $text = uc ($text);
  }

  # do not return a dotless i or j as such if it is further composed
  # with an accented letter, return the letter as is
  if ($accent eq 'dotless') {
    if ($Texinfo::Convert::Unicode::unicode_accented_letters{$accent}
        and exists($Texinfo::Convert::Unicode::unicode_accented_letters{
                                                             $accent}->{$text})
        and ($command->{'parent'}
             and $command->{'parent'}->{'parent'}
             and $command->{'parent'}->{'parent'}->{'cmdname'}
             and $Texinfo::Convert::Unicode::unicode_accented_letters{
                   $command->{'parent'}->{'parent'}->{'cmdname'} })) {
      return $text;
    }
  }

  if ($use_numeric_entities) {
    my $formatted_accent
      = Texinfo::Convert::Converter::xml_numeric_entity_accent($accent, $text);
    if (defined($formatted_accent)) {
      return $formatted_accent;
    }
  } else {
    my ($accent_command_entity, $accent_command_text_with_entities);
    if ($self->{'accent_entities'}->{$accent}) {
      ($accent_command_entity, $accent_command_text_with_entities)
        = @{$self->{'accent_entities'}->{$accent}};
    }
    return "&${text}$accent_command_entity;"
      if ($accent_command_entity
          and defined($accent_command_text_with_entities)
          and ($text =~ /^[$accent_command_text_with_entities]$/));
    my $formatted_accent
      = Texinfo::Convert::Converter::xml_numeric_entity_accent($accent, $text);
    if (defined($formatted_accent)) {
      return $formatted_accent;
    }
  }
  return $self->xml_accent($text, $command, $in_upper_case,
                           $use_numeric_entities);
}

sub _accent_entities_numeric_entities_accent($$$;$)
{
  my $self = shift;
  my $text = shift;
  my $command = shift;
  my $in_upper_case = shift;

  return _accent_entities_html_accent($self, $text, $command, $in_upper_case, 1);
}

sub _convert_accent_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $format_accents;
  if ($self->get_conf('USE_NUMERIC_ENTITY')) {
    $format_accents = \&_accent_entities_numeric_entities_accent;
  } else {
    $format_accents = \&_accent_entities_html_accent;
  }
  return $self->convert_accents($command, $format_accents,
                                $self->get_conf('OUTPUT_CHARACTERS'),
                                $self->in_upper_case());
}

foreach my $command (keys(%accent_commands)) {
  $default_commands_conversion{$command} = \&_convert_accent_command;
}

sub _css_string_accent($$$;$)
{
  my $self = shift;
  my $text = shift;
  my $command = shift;
  my $in_upper_case = shift;

  my $accent = $command->{'cmdname'};

  if ($in_upper_case and $text =~ /^\p{Word}$/) {
    $text = uc ($text);
  }
  if (exists($Texinfo::Convert::Unicode::unicode_accented_letters{$accent})
      and exists($Texinfo::Convert::Unicode::unicode_accented_letters{
                                                          $accent}->{$text})) {
    return '\\' .
      $Texinfo::Convert::Unicode::unicode_accented_letters{$accent}->{$text}. ' ';
  }
  if (exists($Texinfo::Convert::Unicode::unicode_diacritics{$accent})) {
    my $diacritic = '\\'
       .$Texinfo::Convert::Unicode::unicode_diacritics{$accent}. ' ';
    if ($accent ne 'tieaccent') {
      return $text . $diacritic;
    } else {
      # tieaccent diacritic is naturally and correctly composed
      # between two characters
      my $remaining_text = $text;
      # we consider that letters are either characters or escaped characters
      if ($remaining_text =~ s/^([\p{L}\d]|\\[a-zA-Z0-9]+ )([\p{L}\d]|\\[a-zA-Z0-9]+ )(.*)$/$3/) {
        return $1.$diacritic.$2 . $remaining_text;
      } else {
        return $text . $diacritic;
      }
    }
  }
  # should never happen, there are diacritics for every accent command
  return Texinfo::Convert::Text::ascii_accent($text, $command);
}

sub _css_string_convert_accent_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $format_accents = \&_css_string_accent;
  return $self->convert_accents($command, $format_accents,
                                $self->get_conf('OUTPUT_CHARACTERS'),
                                $self->in_upper_case());
}

foreach my $command (keys(%accent_commands)) {
  $default_css_string_commands_conversion{$command}
    = \&_css_string_convert_accent_command;
}

# argument is formatted as code since indicateurl is in brace_code_commands
sub _convert_indicateurl_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $text = $args->[0]->{'normal'};
  if (!defined($text)) {
    # happens with bogus @-commands without argument, like @strong something
    return '';
  }
  if (!$self->in_string()) {
    return $self->get_conf('OPEN_QUOTE_SYMBOL').
        $self->html_attribute_class('code', [$cmdname]).'>'.$text
                .'</code>'.$self->get_conf('CLOSE_QUOTE_SYMBOL');
  } else {
    return $self->get_conf('OPEN_QUOTE_SYMBOL').$text.
              $self->get_conf('CLOSE_QUOTE_SYMBOL');
  }
}

$default_commands_conversion{'indicateurl'} = \&_convert_indicateurl_command;


sub _convert_titlefont_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $text = $args->[0]->{'normal'};
  if (!defined($text)) {
    # happens with bogus @-commands without argument, like @strong something
    return '';
  }
  return &{$self->formatting_function('format_heading_text')}($self, $cmdname,
                                                         [$cmdname], $text, 0);
}
$default_commands_conversion{'titlefont'} = \&_convert_titlefont_command;

sub _convert_U_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $arg;
  $arg = $args->[0]->{'normal'} if ($args->[0]);
  my $res;
  if (defined($arg) and $arg ne '') {
    # checks on the value already done in Parser, just output it here.
    $res = "&#x$arg;";
  } else {
    $res = '';
  }
  return $res;
}
$default_commands_conversion{'U'} = \&_convert_U_command;

sub _default_format_comment($$) {
  my $self = shift;
  my $text = shift;
  return $self->xml_comment(' '.$text);
}

# Note: has an XS override
sub _default_format_protect_text {
  my $self = shift;
  my $text = shift;
  my $result = $self->xml_protect_text($text);
  $result =~ s/\f/&#12;/g;
  return $result;
}

sub _default_css_string_format_protect_text($$) {
  my $self = shift;
  my $text = shift;
  $text =~ s/\\/\\\\/g;
  $text =~ s/\'/\\'/g;
  return $text;
}

# can be called on root commands, tree units, special elements
# and title elements.  $cmdname can be undef for special elements.
sub _default_format_heading_text($$$$$;$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $classes = shift;
  my $text = shift;
  my $level = shift;
  my $id = shift;
  my $element = shift;
  my $target = shift;

  return '' if ($text !~ /\S/ and not defined($id));

  # This should seldom happen.
  if ($self->in_string()) {
    $text .= "\n" unless (defined($cmdname) and $cmdname eq 'titlefont');
    return $text;
  }

  if ($level < 1) {
    $level = 1;
  } elsif ($level > $self->get_conf('MAX_HEADER_LEVEL')) {
    $level = $self->get_conf('MAX_HEADER_LEVEL');
  }
  my $id_str = '';
  if (defined($id)) {
    $id_str = " id=\"$id\"";

    # The ID of this heading is likely the point the user would prefer being
    # linked to over the $target, since that's where they would be seeing a
    # copiable anchor.
    $target = $id;
  }
  my $inside = $text;
  if (defined $target && $self->get_conf('COPIABLE_LINKS')) {
    # Span-wrap this anchor, so that the existing span:hover a.copiable-link
    # rule applies.
    $inside = "<span>$text";
    $inside .= $self->_get_copiable_anchor($target);
    $inside .= '</span>';
  }
  my $result = $self->html_attribute_class("h$level", $classes)
                    ."${id_str}>$inside</h$level>";
  # titlefont appears inline in text, so no end of line is
  # added. The end of line should be added by the user if needed.
  $result .= "\n" unless (defined($cmdname) and $cmdname eq 'titlefont');
  $result .= $self->get_conf('DEFAULT_RULE') . "\n"
     if (defined($cmdname) and $cmdname eq 'part'
         and defined($self->get_conf('DEFAULT_RULE'))
         and $self->get_conf('DEFAULT_RULE') ne '');
  return $result;
}

sub _default_format_separate_anchor($$;$)
{
  my $self = shift;
  my $id = shift;
  my $class = shift;

  # html_attribute_class would not work with span, so if span is
  # used, html_attribute_class should not be used
  return $self->html_attribute_class('a', [$class])." id=\"$id\"></a>";
}

# Associated to a button.  Return text to use for a link in button bar.
# Depending on USE_NODE_DIRECTIONS and xrefautomaticsectiontitle
# use section or node for link direction and string.
sub _default_panel_button_dynamic_direction($$;$$$)
{
  my $self = shift;
  my $direction = shift;
  my $source_command = shift;
  my $omit_rel = shift;
  my $use_first_element_in_file_directions = shift;

  my $result = undef;

  if ((defined($self->get_conf('USE_NODE_DIRECTIONS'))
       and $self->get_conf('USE_NODE_DIRECTIONS'))
      or (not defined($self->get_conf('USE_NODE_DIRECTIONS'))
          and $self->get_conf('USE_NODES'))) {
    $direction = 'Node'.$direction;
  }

  if ($use_first_element_in_file_directions) {
    $direction = 'FirstInFile'.$direction;
  }

  my $href = $self->from_element_direction($direction, 'href',
                                           undef, undef, $source_command);
  my $node;

  if ($self->get_conf('xrefautomaticsectiontitle') eq 'on') {
    $node = $self->from_element_direction($direction, 'section');
  }

  if (!defined($node)) {
    $node = $self->from_element_direction($direction, 'node');
  }

  my $hyperlink;
  if (defined($href) and $href ne '' and defined($node) and $node =~ /\S/) {
    my $hyperlink_attributes = $omit_rel ? ''
      : $self->_direction_href_attributes($direction);
    $hyperlink = "<a href=\"$href\"${hyperlink_attributes}>$node</a>";
  } elsif (defined($node) and $node =~ /\S/) {
    $hyperlink = $node;
  }
  if (defined($hyperlink)) {
    # i18n
    $result = $self->direction_string($direction, 'text').": $hyperlink";
  }
  # 1 to communicate that a delimiter is needed for that button
  return ($result, 1);
}

# Used for button bar at the foot of a node, with "rel" and "accesskey"
# attributes omitted.
sub _default_panel_button_dynamic_direction_node_footer($$$)
{
  my $self = shift;
  my $direction = shift;
  my $source_command = shift;

  return _default_panel_button_dynamic_direction($self, $direction,
                                                 $source_command, 1);
}

# used for button bar at the foot of a section or chapter with
# directions of first element in file used instead of the last
# element directions.
sub _default_panel_button_dynamic_direction_section_footer($$$) {
  my $self = shift;
  my $direction = shift;
  my $source_command = shift;

  return _default_panel_button_dynamic_direction($self, $direction,
                                                 $source_command, undef, 1);
}

# Only used if ICONS is set and the button is active.
sub _default_format_button_icon_img($$$;$)
{
  my $self = shift;
  my $button = shift;
  my $icon = shift;
  my $name = shift;

  return '' if (!defined($icon));
  $button = '' if (!defined ($button));
  $name = '' if (!defined($name));
  my $alt = '';
  if ($name ne '') {
    if ($button ne '') {
      $alt = "$button: $name";
    } else {
      $alt = $name;
    }
  } else {
    $alt = $button;
  }
  return $self->close_html_lone_element(
    '<img src="'.$self->url_protect_url_text($icon)
       ."\" border=\"0\" alt=\"$alt\" align=\"middle\"");
}

sub _direction_href_attributes($$)
{
  my $self = shift;
  my $direction = shift;

  my $href_attributes = '';
  if ($self->get_conf('USE_ACCESSKEY')) {
    my $accesskey = $self->direction_string($direction, 'accesskey', 'string');
    if (defined($accesskey) and ($accesskey ne '')) {
      $href_attributes = " accesskey=\"$accesskey\"";
    }
  }
  if ($self->get_conf('USE_REL_REV')) {
    my $button_rel = $self->direction_string($direction, 'rel', 'string');
    if (defined($button_rel) and ($button_rel ne '')) {
      $href_attributes .= " rel=\"$button_rel\"";
    }
  }
  return $href_attributes;
}

my %html_default_node_directions;
foreach my $node_directions ('NodeNext', 'NodePrev', 'NodeUp') {
  $html_default_node_directions{$node_directions} = 1;
}

sub _default_format_button($$;$)
{
  my $self = shift;
  my $button = shift;
  my $source_command = shift;

  my ($active, $passive, $need_delimiter);
  if (ref($button) eq 'CODE') {
    ($active, $need_delimiter) = &$button($self);
  } elsif (ref($button) eq 'SCALAR') {
    $active = "$$button" if defined($$button);
    $need_delimiter = 1;
  } elsif (ref($button) eq 'ARRAY' and scalar(@$button == 2)) {
    my $text = $button->[1];
    my $direction = $button->[0];
    # $direction is simple text and $text is a reference
    if (defined($direction) and ref($direction) eq ''
        and defined($text) and (ref($text) eq 'SCALAR') and defined($$text)) {
      # use given text
      my $href = $self->from_element_direction($direction, 'href',
                                               undef, undef, $source_command);
      if ($href) {
        my $anchor_attributes = $self->_direction_href_attributes($direction);
        $active = "<a href=\"$href\"${anchor_attributes}>$$text</a>";
      } else {
        $passive = $$text;
      }
      $need_delimiter = 1;
    # $direction is simple text and $text is a reference on code
    } elsif (defined($direction) and ref($direction) eq ''
             and defined($text) and (ref($text) eq 'CODE')) {
      ($active, $need_delimiter) = &$text($self, $direction, $source_command);
    # $direction is simple text and $text is also a simple text
    } elsif (defined($direction) and ref($direction) eq ''
             and defined($text) and ref($text) eq '') {
      if ($text =~ s/^->\s*//) {
        # this case is mostly for tests, to test the direction type $text
        # with the direction $direction
        $active = $self->from_element_direction($direction, $text,
                                                undef, undef, $source_command);
      } else {
        my $href = $self->from_element_direction($direction, 'href',
                                                 undef, undef, $source_command);
        my $text_formatted = $self->from_element_direction($direction, $text);
        if ($href) {
          my $anchor_attributes = $self->_direction_href_attributes($direction);
          $active = "<a href=\"$href\"${anchor_attributes}>$text_formatted</a>";
        } else {
          $passive = $text_formatted;
        }
      }
      $need_delimiter = 1;
    }
  } elsif ($button eq ' ') {
    # handle space button
    if ($self->get_conf('ICONS') and $self->get_conf('ACTIVE_ICONS')
        and defined($self->get_conf('ACTIVE_ICONS')->{$button})
        and $self->get_conf('ACTIVE_ICONS')->{$button} ne '') {
      my $button_name_string = $self->direction_string($button,
                                                       'button', 'string');
      $active = &{$self->formatting_function('format_button_icon_img')}($self,
                   $button_name_string, $self->get_conf('ACTIVE_ICONS')->{' '});
    } else {
      $active = $self->direction_string($button, 'text');
    }
    $need_delimiter = 0;
  } else {
    my $href = $self->from_element_direction($button, 'href',
                                             undef, undef, $source_command);
    if ($href) {
      # button is active
      my $btitle = '';
      my $description = $self->direction_string($button, 'description', 'string');
      if (defined($description)) {
        $btitle = ' title="' . $description . '"';
      }
      if ($self->get_conf('USE_ACCESSKEY')) {
        my $accesskey = $self->direction_string($button, 'accesskey', 'string');
        if (defined($accesskey) and $accesskey ne '') {
          $btitle .= " accesskey=\"$accesskey\"";
        }
      }
      if ($self->get_conf('USE_REL_REV')) {
        my $button_rel = $self->direction_string($button, 'rel', 'string');
        if (defined($button_rel) and $button_rel ne '') {
          $btitle .= " rel=\"$button_rel\"";
        }
      }
      my $use_icon;
      if ($self->get_conf('ICONS') and $self->get_conf('ACTIVE_ICONS')) {
        # FIXME strip FirstInFile from $button to get $active_icon?
        my $active_icon = $self->get_conf('ACTIVE_ICONS')->{$button};
        my $button_name_string = $self->direction_string($button,
                                                         'button', 'string');
        if (defined($active_icon) and $active_icon ne '') {
          # use icon
          $active = "<a href=\"$href\"${btitle}>".
             &{$self->formatting_function('format_button_icon_img')}($self,
                      $button_name_string, $active_icon,
                      $self->from_element_direction($button, 'string')) ."</a>";
          $use_icon = 1;
        }
      }
      if (!$use_icon) {
        # use text
        $active = '[' . "<a href=\"$href\"${btitle}>".
          $self->direction_string($button, 'text')."</a>" . ']';
      }
    } else {
      # button is passive
      my $use_icon;
      if ($self->get_conf('ICONS') and $self->get_conf('PASSIVE_ICONS')) {
        # FIXME strip FirstInFile from $button to get $passive_icon?
        my $passive_icon = $self->get_conf('PASSIVE_ICONS')->{$button};
        my $button_name_string = $self->direction_string($button,
                                                         'button', 'string');
        if ($passive_icon and $passive_icon ne '') {
          $passive = &{$self->formatting_function('format_button_icon_img')}(
                      $self, $button_name_string, $passive_icon,
                      $self->from_element_direction($button, 'string'));
          $use_icon = 1;
        }
      }
      if (!$use_icon) {
        $passive =  '[' . $self->direction_string($button, 'text') . ']';
      }
    }
    $need_delimiter = 0;
  }
  # FIXME chose another option among those proposed in comments below?
  if (not defined($need_delimiter)) {
    # option 1: be forgiving if $need_delimiter is not set
    # if ($html_default_node_directions{$button}) {
    #   $need_delimiter = 1;
    # } else {
    #   $need_delimiter = 0;
    # }
    # option 2: be somewhat forgiving but show a backtrace
    #cluck ("need_delimiter not defined");
    # $need_delimiter = 0;
    # option3: no pity
    confess ("need_delimiter not defined");
  }
  return ($active, $passive, $need_delimiter);
}

# called for special elements and tree units
sub _default_format_navigation_panel($$$$;$)
{
  my $self = shift;
  my $buttons = shift;
  my $cmdname = shift;
  my $source_command = shift;
  my $vertical = shift;

  # if VERTICAL_HEAD_NAVIGATION, the buttons are in a vertical table which
  # is itself in the first column of a table opened in header_navigation
  #my $vertical = $self->get_conf('VERTICAL_HEAD_NAVIGATION');

  my $result = '';
  if ($self->get_conf('HEADER_IN_TABLE')) {
    $result .= $self->html_attribute_class('table', ['nav-panel'])
        .' cellpadding="1" cellspacing="1" border="0">'."\n";
    $result .= "<tr>" unless $vertical;
  } else {
    $result .= $self->html_attribute_class('div', ['nav-panel']).">\n<p>\n";
  }

  my $first_button = 1;
  foreach my $button (@$buttons) {
    if ($self->get_conf('HEADER_IN_TABLE')) {
      $result .= '<tr>'."\n" if $vertical;
      $result .=  '<td>';
    }
    my $direction;
    if (ref($button) eq 'ARRAY'
        and defined($button->[0]) and ref($button->[0]) eq '') {
      $direction = $button->[0];
    } elsif (defined($button) and ref($button) eq '') {
      $direction = $button;
    }

    my ($active, $passive, $need_delimiter)
      # API info: using the API to allow for customization would be:
      #  = &{$self->formatting_function('format_button')}($self, $button,
      #                                                   $source_command);
       = &{$self->{'formatting_function'}->{'format_button'}}($self, $button,
                                                              $source_command);
    if ($self->get_conf('HEADER_IN_TABLE')) {
      if (defined($active)) {
        $result .= $active;
      } elsif (defined($passive)) {
        $result .= $passive;
      }
      $result .= "</td>\n";
      $result .= "</tr>\n" if $vertical;
      $first_button = 0 if ($first_button);
    } elsif (defined($active)) {
      # only active buttons are print out when not in table
      if ($need_delimiter and !$first_button) {
        $active = ', ' .$active;
      }
      $result .= $active;
      $first_button = 0 if ($first_button);
    }
  }
  if ($self->get_conf('HEADER_IN_TABLE')) {
    $result .= "</tr>" unless $vertical;
    $result .= "</table>\n";
  } else {
     $result .= "</p>\n</div>\n";
  }
  return $result;
}

sub _default_format_navigation_header($$$$)
{
  my $self = shift;
  my $buttons = shift;
  my $cmdname = shift;
  my $element = shift;

  my $result = '';
  if ($self->get_conf('VERTICAL_HEAD_NAVIGATION')) {
    $result .= '<table border="0" cellpadding="0" cellspacing="0">
<tr>
<td>
';
  }
  $result .= &{$self->formatting_function('format_navigation_panel')}($self,
                                   $buttons, $cmdname, $element,
                                   $self->get_conf('VERTICAL_HEAD_NAVIGATION'));
  if ($self->get_conf('VERTICAL_HEAD_NAVIGATION')) {
    $result .= '</td>
<td>
';
  } elsif ($self->get_conf('SPLIT') eq 'node') {
    $result .= $self->get_conf('DEFAULT_RULE')."\n";
  }
  return $result;
}

# this can only be called on root commands and associated tree units
sub _default_format_element_header($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $tree_unit = shift;

  my $result = '';

  print STDERR "FORMAT elt header "
     # uncomment to get perl object names
     #."$tree_unit (@{$tree_unit->{'contents'}}) ".
     . "(".join('|', map{Texinfo::Common::debug_print_element($_)}
             @{$tree_unit->{'contents'}}) . ") ".
     Texinfo::Structuring::root_or_external_element_cmd_texi($tree_unit) ."\n"
        if ($self->get_conf('DEBUG'));

  # Do the heading if the command is the first command in the element
  if (($tree_unit->{'contents'}->[0] eq $command
       or (!$tree_unit->{'contents'}->[0]->{'cmdname'}
            and $tree_unit->{'contents'}->[1] eq $command))
      # and there is more than one element
      and ($tree_unit->{'structure'}
           and ($tree_unit->{'structure'}->{'unit_next'}
                or $tree_unit->{'structure'}->{'unit_prev'}))) {
    my $is_top = $self->element_is_tree_unit_top($tree_unit);
    my $first_in_page = (defined($tree_unit->{'structure'}->{'unit_filename'})
           and $self->count_elements_in_filename('current',
                           $tree_unit->{'structure'}->{'unit_filename'}) == 1);
    my $previous_is_top = 0;
    $previous_is_top = 1
      if ($tree_unit->{'structure'}->{'unit_prev'}
          and $self->element_is_tree_unit_top($tree_unit->{'structure'}
                                                             ->{'unit_prev'}));

    print STDERR "Header ($previous_is_top, $is_top, $first_in_page): "
     .Texinfo::Convert::Texinfo::root_heading_command_to_texinfo($command)."\n"
       if ($self->get_conf('DEBUG'));

    if ($is_top) {
      # use TOP_BUTTONS for top.
      $result .=
         &{$self->formatting_function('format_navigation_header')}($self,
                         $self->get_conf('TOP_BUTTONS'), $cmdname, $command)
           if ($self->get_conf('SPLIT') or $self->get_conf('HEADERS'));
    } else {
      if ($first_in_page and !$self->get_conf('HEADERS')) {
        if ($self->get_conf('SPLIT') eq 'chapter') {
          $result
           .= &{$self->formatting_function('format_navigation_header')}($self,
                        $self->get_conf('CHAPTER_BUTTONS'), $cmdname, $command);

          $result .= $self->get_conf('DEFAULT_RULE') ."\n"
            if (defined($self->get_conf('DEFAULT_RULE'))
                and !$self->get_conf('VERTICAL_HEAD_NAVIGATION'));
        } elsif ($self->get_conf('SPLIT') eq 'section') {
          $result
            .= &{$self->formatting_function('format_navigation_header')}($self,
                        $self->get_conf('SECTION_BUTTONS'), $cmdname, $command);
        }
      }
      if (($first_in_page or $previous_is_top)
           and $self->get_conf('HEADERS')) {
        $result
          .= &{$self->formatting_function('format_navigation_header')}($self,
                        $self->get_conf('SECTION_BUTTONS'), $cmdname, $command);
      } elsif($self->get_conf('HEADERS') or $self->get_conf('SPLIT') eq 'node') {
        # got to do this here, as it isn't done otherwise since
        # navigation_header is not called
        $result
          .= &{$self->formatting_function('format_navigation_panel')}($self,
                       $self->get_conf('SECTION_BUTTONS'), $cmdname, $command);
      }
    }
  }
  return $result;
}

sub register_opened_section_level($$$)
{
  my $self = shift;
  my $level = shift;
  my $close = shift;
  while (@{$self->{'pending_closes'}} < $level) {
    push(@{$self->{'pending_closes'}}, "");
  }
  push(@{$self->{'pending_closes'}}, $close);
}

sub close_registered_sections_level($$)
{
  my $self = shift;
  my $level = shift;
  if (not defined($level)) {
    cluck 'close_registered_sections_level $level not defined';
  }
  my @closed_elements;
  my $result = '';
  while (@{$self->{'pending_closes'}} > $level) {
      my $close = pop @{$self->{'pending_closes'}};
      push(@closed_elements, $close)
        if ($close);
  }
  return @closed_elements;
}

sub _convert_heading_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $element = shift;
  my $args = shift;
  my $content = shift;

  my $result = '';

  # No situation where this could happen
  if ($self->in_string()) {
    $result .= $self->command_text($element, 'string') ."\n"
      if ($cmdname ne 'node');
    $result .= $content if (defined($content));
    return $result;
  }

  my $element_id = $self->command_id($element);

  print STDERR "CONVERT elt heading "
        # uncomment next line for the perl object name
        #."$element "
        .Texinfo::Convert::Texinfo::root_heading_command_to_texinfo($element)."\n"
          if ($self->get_conf('DEBUG'));
  my $tree_unit;
  if ($Texinfo::Commands::root_commands{$element->{'cmdname'}}
      and $element->{'structure'}->{'associated_unit'}) {
    $tree_unit = $element->{'structure'}->{'associated_unit'};
  }
  my $element_header = '';
  if ($tree_unit) {
    $element_header = &{$self->formatting_function('format_element_header')}(
                                        $self, $cmdname, $element, $tree_unit);
  }

  my $tables_of_contents = '';
  my $structuring = $self->get_info('structuring');
  if ($self->get_conf('CONTENTS_OUTPUT_LOCATION') eq 'after_top'
      and $cmdname eq 'top'
      and $structuring and $structuring->{'sectioning_root'}
      and scalar(@{$structuring->{'sections_list'}}) > 1) {
    foreach my $content_command_name ('shortcontents', 'contents') {
      if ($self->get_conf($content_command_name)) {
        my $contents_text
          = $self->_contents_inline_element($content_command_name, undef);
        if ($contents_text ne '') {
          $tables_of_contents .= $contents_text;
        }
      }
    }
  }

  my $mini_toc = '';
  if ($tables_of_contents eq ''
      and $self->get_conf('FORMAT_MENU') eq 'sectiontoc'
      and $sectioning_heading_commands{$cmdname}) {
    $mini_toc = _mini_toc($self, $element);
  }

  if ($self->get_conf('NO_TOP_NODE_OUTPUT')
      and $Texinfo::Commands::root_commands{$cmdname}) {
    my $in_skipped_node_top
      = $self->shared_conversion_state('in_skipped_node_top', 0);
    my $node_element;
    if ($cmdname eq 'node') {
      $node_element = $element;
    } elsif ($cmdname eq 'part' and $element->{'extra'}
             and $element->{'extra'}->{'part_following_node'}) {
      $node_element = $element->{'extra'}->{'part_following_node'};
    }
    if ($node_element or $cmdname eq 'part') {
      if ($node_element and $node_element->{'extra'}
          and $node_element->{'extra'}->{'normalized'}
          and $node_element->{'extra'}->{'normalized'} eq 'Top') {
        $$in_skipped_node_top = 1;
      } elsif ($$in_skipped_node_top == 1) {
        $$in_skipped_node_top = -1;
      }
    }
    if ($$in_skipped_node_top == 1) {
      my $id_class = $cmdname;
      $result .= &{$self->formatting_function('format_separate_anchor')}($self,
                                                        $element_id, $id_class);
      $result .= $element_header;
      $result .= $tables_of_contents;
      $result .= $mini_toc;
      return $result;
    }
  }

  my @heading_classes;
  my $level_corrected_cmdname = $cmdname;
  if ($element->{'structure'}
      and defined $element->{'structure'}->{'section_level'}) {
    # if the level was changed, use a consistent command name
    $level_corrected_cmdname
      = Texinfo::Structuring::section_level_adjusted_command_name($element);
    if ($level_corrected_cmdname ne $cmdname) {
      push @heading_classes,
            "${cmdname}-level-set-${level_corrected_cmdname}";
    }
  }

  # find the section starting here, can be through the associated node
  # preceding the section, or the section itself
  my $opening_section;
  my $level_corrected_opening_section_cmdname;
  if ($cmdname eq 'node'
      and $element->{'extra'}
      and $element->{'extra'}->{'associated_section'}) {
    $opening_section = $element->{'extra'}->{'associated_section'};
    $level_corrected_opening_section_cmdname
     = Texinfo::Structuring::section_level_adjusted_command_name(
                                                             $opening_section);
  } elsif ($cmdname ne 'node'
           # if there is an associated node, it is not a section opening
           # the section was opened before when the node was encountered
           and (not $element->{'extra'}
                or not $element->{'extra'}->{'associated_node'})
           # to avoid *heading* @-commands
           and $Texinfo::Commands::root_commands{$cmdname}) {
    $opening_section = $element;
    $level_corrected_opening_section_cmdname = $level_corrected_cmdname;
  }

  # $heading not defined may happen if the command is a @node, for example
  # if there is an error in the node.
  my $heading = $self->command_text($element);
  my $heading_level;
  # node is used as heading if there is nothing else.
  if ($cmdname eq 'node') {
    # FIXME what to do if the $tree_unit extra does not contain any
    # unit_command, but tree_unit is defined (it can contain only
    # 'first_in_page')
    if ((!$tree_unit # or !$tree_unit->{'extra'}
         # or !$tree_unit->{'extra'}->{'unit_command'}
         or ($tree_unit->{'extra'}->{'unit_command'}
             and $tree_unit->{'extra'}->{'unit_command'} eq $element
             and (not $element->{'extra'}
                  or not $element->{'extra'}->{'associated_section'})))
        and defined($element->{'extra'})
        and defined($element->{'extra'}->{'normalized'})) {
      if ($element->{'extra'}->{'normalized'} eq 'Top') {
        $heading_level = 0;
      } else {
        $heading_level = 3;
      }
    }
  } elsif ($element->{'structure'}
           and defined($element->{'structure'}->{'section_level'})) {
    $heading_level = $element->{'structure'}->{'section_level'};
  } else {
    # for *heading* @-commands which do not have a level
    # in the document as they are not associated with the
    # sectioning tree, but still have a $heading_level
    $heading_level = Texinfo::Common::section_level($element);
  }

  my $do_heading = (defined($heading) and $heading ne ''
                    and defined($heading_level));

  # if set, the id is associated to the heading text
  my $heading_id;
  if ($opening_section) {
    my $level = $opening_section->{'structure'}->{'section_level'};
    $result .= join('', $self->close_registered_sections_level($level));
    $self->register_opened_section_level($level, "</div>\n");

    # use a specific class name to mark that this is the start of
    # the section extent. It is not necessary where the section is.
    $result .= $self->html_attribute_class('div',
                 ["${level_corrected_opening_section_cmdname}-level-extent"]);
    $result .= " id=\"$element_id\""
        if (defined($element_id) and $element_id ne '');
    $result .= ">\n";
  } elsif (defined($element_id) and $element_id ne '') {
    if ($element_header ne '') {
      # case of a @node without sectioning command and with a header.
      # put the node element anchor before the header.
      # Set the class name to the command name if there is no heading,
      # else the class will be with the heading element.
      my $id_class = $cmdname;
      if ($do_heading) {
        $id_class = "${cmdname}-id";
      }
      $result .= &{$self->formatting_function('format_separate_anchor')}($self,
                                                        $element_id, $id_class);
    } else {
      $heading_id = $element_id;
    }
  }

  $result .= $element_header;

  if ($do_heading) {
    if ($self->get_conf('TOC_LINKS')
        and $Texinfo::Commands::root_commands{$cmdname}
        and $sectioning_heading_commands{$cmdname}) {
      my $content_href = $self->command_contents_href($element, 'contents');
      if ($content_href ne '') {
        $heading = "<a href=\"$content_href\">$heading</a>";
      }
    }

    my $heading_class = $level_corrected_cmdname;
    unshift @heading_classes, $heading_class;
    if ($self->in_preformatted()) {
      my $id_str = '';
      if (defined($heading_id)) {
        $id_str = " id=\"$heading_id\"";
      }
      $result .= $self->html_attribute_class('strong', \@heading_classes)
                                   ."${id_str}>".$heading.'</strong>'."\n";
    } else {
      $result .= &{$self->formatting_function('format_heading_text')}($self,
                     $level_corrected_cmdname, \@heading_classes, $heading,
                     $heading_level +$self->get_conf('CHAPTER_HEADER_LEVEL') -1,
                     $heading_id, $element, $element_id);
    }
  } elsif (defined($heading_id)) {
    # case of a lone node and no header, and case of an empty @top
    $result .= &{$self->formatting_function('format_separate_anchor')}($self,
                                                       $heading_id, $cmdname);
  }
  $result .= $content if (defined($content));

  $result .= $tables_of_contents;
  $result .= $mini_toc;
  return $result;
}

foreach my $command (keys(%sectioning_heading_commands), 'node') {
  $default_commands_conversion{$command} = \&_convert_heading_command;
}

sub _convert_raw_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  if ($cmdname eq 'html') {
    return $content;
  }
  $self->_noticed_line_warn(sprintf(__("raw format %s is not converted"),
                                   $cmdname), $command->{'source_info'});
  return &{$self->formatting_function('format_protect_text')}($self, $content);
}

foreach my $command (keys(%format_raw_commands)) {
  $default_commands_conversion{$command} = \&_convert_raw_command;
}

sub _convert_inline_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $format_arg = shift @$args;

  my $format;
  if (defined($format_arg)) {
    $format = $format_arg->{'monospacetext'};
  }
  return '' if (!defined($format) or $format eq '');

  my $arg_index = undef;
  if ($inline_format_commands{$cmdname}) {
    if ($cmdname eq 'inlinefmtifelse' and !$self->is_format_expanded($format)) {
      $arg_index = 1;
    } elsif ($self->is_format_expanded($format)) {
      $arg_index = 0;
    }
  } elsif (defined($command->{'extra'})
           and defined($command->{'extra'}->{'expand_index'})) {
    $arg_index = 0;
  }
  if (defined($arg_index) and $arg_index < scalar(@$args)) {
    my $text_arg = $args->[$arg_index];
    if ($text_arg) {
      if ($text_arg->{'normal'}) {
        return $text_arg->{'normal'};
      } elsif ($text_arg->{'raw'}) {
        return $text_arg->{'raw'};
      }
    }
  }
  return '';
}

foreach my $command (grep {$brace_commands{$_} eq 'inline'}
                           keys(%brace_commands)) {
  $default_commands_conversion{$command} = \&_convert_inline_command;
}

sub _indent_with_table($$$;$)
{
  my $self = shift;
  my $cmdname = shift;
  my $content = shift;
  my $extra_classes = shift;

  my @classes;
  @classes = @$extra_classes if (defined($extra_classes));
  unshift @classes, $cmdname;
  return $self->html_attribute_class('table', \@classes)
         .'><tr><td>'.$self->get_info('non_breaking_space').'</td><td>'.$content
                ."</td></tr></table>\n";
}

sub _convert_preformatted_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  my @classes;

  # this is mainly for classes as there are purprosely no classes
  # for small*
  my $main_cmdname;
  if ($small_block_associated_command{$cmdname}) {
    $main_cmdname = $small_block_associated_command{$cmdname};
    push @classes, $cmdname;
  } else {
    $main_cmdname = $cmdname;
  }

  if ($cmdname eq 'example') {
    if ($command->{'args'}) {
      for my $example_arg (@{$command->{'args'}}) {
        # convert or remove all @-commands, using simple ascii and unicode
        # characters
        my $converted_arg
          = Texinfo::Convert::NodeNameNormalization::convert_to_normalized(
                                                                 $example_arg);
        if ($converted_arg ne '') {
          push @classes, 'user-' . $converted_arg;
        }
      }
    }
  } elsif ($main_cmdname eq 'lisp') {
    push @classes, $main_cmdname;
    $main_cmdname = 'example';
  }

  if ($content ne '' and !$self->in_string()) {
    if ($self->get_conf('COMPLEX_FORMAT_IN_TABLE')
        and $indented_preformatted_commands{$cmdname}) {
      return _indent_with_table($self, $cmdname, $content, \@classes);
    } else {
      unshift @classes, $main_cmdname;
      return $self->html_attribute_class('div', \@classes)
                                     .">\n".$content.'</div>'."\n";
    }
  } else {
    return $content;
  }
}

foreach my $preformatted_command (keys(%preformatted_commands)) {
  $default_commands_conversion{$preformatted_command}
    = \&_convert_preformatted_command;
}

sub _convert_indented_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  my @classes;

  my $main_cmdname;
  if ($small_block_associated_command{$cmdname}) {
    push @classes, $cmdname;
    $main_cmdname = $small_block_associated_command{$cmdname};
  } else {
    $main_cmdname = $cmdname;
  }
  if ($content ne '' and !$self->in_string()) {
    if ($self->get_conf('COMPLEX_FORMAT_IN_TABLE')) {
      return _indent_with_table($self, $main_cmdname, $content, \@classes);
    } else {
      unshift @classes, $main_cmdname;
      return $self->html_attribute_class('blockquote', \@classes).">\n"
                          . $content . '</blockquote>'."\n";
    }
  } else {
    return $content;
  }
}

$default_commands_conversion{'indentedblock'} = \&_convert_indented_command;

sub _convert_verbatim_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  if (!$self->in_string()) {
    return $self->html_attribute_class('pre', [$cmdname]).'>'
          .$content . '</pre>';
  } else {
    return $content;
  }
}

$default_commands_conversion{'verbatim'} = \&_convert_verbatim_command;

sub _convert_displaymath_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  if ($self->in_string()) {
    return $content;
  }

  my $result = '';
  $result .= $self->html_attribute_class('div', [$cmdname]).'>';
  if ($self->get_conf('HTML_MATH')
        and $self->get_conf('HTML_MATH') eq 'mathjax') {
    $self->register_file_information('mathjax', 1);
    $result .= $self->html_attribute_class('em', ['tex2jax_process']).'>'
          ."\\[$content\\]".'</em>';
  } else {
    $result .= $self->html_attribute_class('em').'>'."$content".'</em>';
  }
  $result .= '</div>';
  return $result;
}

$default_commands_conversion{'displaymath'} = \&_convert_displaymath_command;

sub _convert_verbatiminclude_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $verbatim_include_verbatim
    = Texinfo::Convert::Utils::expand_verbatiminclude($self, $self, $command);
  if (defined($verbatim_include_verbatim)) {
    return $self->convert_tree($verbatim_include_verbatim,
                               'convert verbatiminclude');
  } else {
    return '';
  }
}

$default_commands_conversion{'verbatiminclude'}
  = \&_convert_verbatiminclude_command;

sub _convert_command_simple_block($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  return $self->html_attribute_class('div', [$cmdname]).'>'
        .$content.'</div>';
}

$default_commands_conversion{'raggedright'} = \&_convert_command_simple_block;
$default_commands_conversion{'flushleft'} = \&_convert_command_simple_block;
$default_commands_conversion{'flushright'} = \&_convert_command_simple_block;
$default_commands_conversion{'group'} = \&_convert_command_simple_block;

sub _convert_sp_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  if (defined($command->{'extra'})
      and defined($command->{'extra'}->{'misc_args'}->[0])) {
    my $sp_nr = $command->{'extra'}->{'misc_args'}->[0];
    if ($self->in_preformatted() or $self->in_string()) {
      return "\n" x $sp_nr;
    } else {
      return ($self->get_info('line_break_element')."\n") x $sp_nr;
    }
  }
}

$default_commands_conversion{'sp'} = \&_convert_sp_command;

sub _convert_exdent_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;


  my $arg = $self->get_pending_formatted_inline_content().$args->[0]->{'normal'};

  if ($self->in_string()) {
    return $arg ."\n";
  }

  # FIXME do something with CSS?  Currently nothing is defined for exdent

  if ($self->in_preformatted()) {
    return $self->html_attribute_class('pre', [$cmdname]).'>'.$arg ."\n</pre>";
  } else {
    return $self->html_attribute_class('p', [$cmdname]).'>'.$arg ."\n</p>";
  }
}

$default_commands_conversion{'exdent'} = \&_convert_exdent_command;

sub _convert_center_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  if ($self->in_string()) {
    return $args->[0]->{'normal'}."\n";
  } else {
    return $self->html_attribute_class('div', [$cmdname]).">"
                                 .$args->[0]->{'normal'}."\n</div>";
  }
}

$default_commands_conversion{'center'} = \&_convert_center_command;

sub _convert_author_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  return '' if (!$args->[0] or !$command->{'extra'}
                or !$command->{'extra'}->{'titlepage'});
  if (!$self->in_string()) {
    return $self->html_attribute_class('strong', [$cmdname])
                .">$args->[0]->{'normal'}</strong>"
                .$self->get_info('line_break_element')."\n";
  } else {
    return $args->[0]->{'normal'} . "\n";
  }
}

$default_commands_conversion{'author'} = \&_convert_author_command;

sub _convert_title_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  return '' if (!$args->[0]);
  if (!$self->in_string()) {
    return $self->html_attribute_class('h1', [$cmdname])
                            .">$args->[0]->{'normal'}</h1>\n";
  } else {
    return $args->[0]->{'normal'};
  }
}
$default_commands_conversion{'title'} = \&_convert_title_command;

sub _convert_subtitle_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  return '' if (!$args->[0]);
  if (!$self->in_string()) {
    return $self->html_attribute_class('h3', [$cmdname])
                            .">$args->[0]->{'normal'}</h3>\n";
  } else {
    return $args->[0]->{'normal'};
  }
}
$default_commands_conversion{'subtitle'} = \&_convert_subtitle_command;

sub _convert_insertcopying_command($$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;

  my $global_commands = $self->get_info('global_commands');
  if ($global_commands and $global_commands->{'copying'}) {
    return $self->convert_tree({'contents'
               => $global_commands->{'copying'}->{'contents'}},
                               'convert insertcopying');
  }
  return '';
}
$default_commands_conversion{'insertcopying'}
   = \&_convert_insertcopying_command;

sub _convert_listoffloats_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  # should probably never happen
  return '' if ($self->in_string());

  my $floats = $self->get_info('floats');
  my $listoffloats_name = $command->{'extra'}->{'float_type'};
  if ($floats and $floats->{$listoffloats_name}
      and scalar(@{$floats->{$listoffloats_name}})) {
    my $result = $self->html_attribute_class('dl', [$cmdname]).">\n" ;
    foreach my $float (@{$floats->{$listoffloats_name}}) {
      my $float_href = $self->command_href($float);
      next if (!$float_href);
      $result .= '<dt>';
      my $float_text = $self->command_text($float);
      if (defined($float_text) and $float_text ne '') {
        if ($float_href) {
          $result .= "<a href=\"$float_href\">$float_text</a>";
        } else {
          $result .= $float_text;
        }
      }
      $result .= '</dt>';
      my $caption;
      my $caption_cmdname;
      if ($float->{'extra'} and $float->{'extra'}->{'shortcaption'}) {
        $caption = $float->{'extra'}->{'shortcaption'};
        $caption_cmdname = 'shortcaption';
      } elsif ($float->{'extra'} and $float->{'extra'}->{'caption'}) {
        $caption = $float->{'extra'}->{'caption'};
        $caption_cmdname = 'caption';
      }

      my $caption_text;
      my @caption_classes;
      if ($caption) {
        $caption_text = $self->convert_tree_new_formatting_context(
          $caption->{'args'}->[0], $cmdname, 'listoffloats');
        push @caption_classes, "${caption_cmdname}-in-${cmdname}";
      } else {
        $caption_text = '';
      }
      $result .= $self->html_attribute_class('dd', \@caption_classes).'>'
                                           .$caption_text.'</dd>'."\n";
    }
    return $result . "</dl>\n";
  } else {
    return '';
  }
}
$default_commands_conversion{'listoffloats'} = \&_convert_listoffloats_command;

sub _in_preformatted_in_menu($)
{
  my $self = shift;
  return 1 if ($self->get_conf('SIMPLE_MENU'));
  my @pre_classes = $self->preformatted_classes_stack();
  foreach my $pre_class (@pre_classes) {
    return 1 if ($preformatted_commands{$pre_class});
  }
  return 0;
}

sub _convert_menu_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  return $content if ($cmdname eq 'detailmenu');

  my $html_menu_entry_index
    = $self->shared_conversion_state('html_menu_entry_index', 0);
  $$html_menu_entry_index = 0;

  if ($content !~ /\S/) {
    return '';
  }
  # This can probably only happen with incorrect input,
  # for instance menu in copying
  # FIXME check?
  if ($self->in_string()) {
    return $content;
  }

  if ($self->get_conf('SIMPLE_MENU')) {
    return $self->html_attribute_class('div', [$cmdname]).'>'
       .$content ."</div>\n";
  }
  my $begin_row = '';
  my $end_row = '';
  if ($self->_in_preformatted_in_menu()) {
    $begin_row = '<tr><td>';
    $end_row = '</td></tr>';
  }
  return $self->html_attribute_class('table', [$cmdname])
    ." border=\"0\" cellspacing=\"0\">${begin_row}\n"
      . $content . "${end_row}</table>\n";
}
$default_commands_conversion{'menu'} = \&_convert_menu_command;
$default_commands_conversion{'detailmenu'} = \&_convert_menu_command;

sub _convert_float_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  my ($caption, $prepended)
     = Texinfo::Convert::Converter::float_name_caption($self, $command);
  my $caption_command_name;
  if (defined($caption)) {
    $caption_command_name = $caption->{'cmdname'};
  }
  if ($self->in_string()) {
    my $prepended_text;
    if ($prepended) {
      $prepended_text = $self->convert_tree_new_formatting_context(
        $prepended, 'float prepended');
    } else {
      $prepended_text = '';
    }
    my $caption_text = '';
    if ($caption and $caption->{'args'}->[0]
        and $caption->{'args'}->[0]->{'contents'}) {
      $caption_text = $self->convert_tree_new_formatting_context(
        {'contents' => $caption->{'args'}->[0]->{'contents'}},
        'float caption');
    }
    return $prepended.$content.$caption_text;
  }

  my $id = $self->command_id($command);
  my $id_str = '';;
  if (defined($id) and $id ne '') {
    $id_str = " id=\"$id\"";
  }

  my $prepended_text;
  my $caption_text = '';
  if ($prepended) {
    # FIXME add a span with a class name for the prependend information
    # if not empty?
    $prepended_text = $self->convert_tree_new_formatting_context(
                               {'cmdname' => 'strong',
                                'args' => [{'type' => 'brace_command_arg',
                                            'contents' => [$prepended]}]},
                               'float number type');
    if ($caption) {
      # register the converted prepended tree to be prepended to
      # the first paragraph in caption formatting
      $self->register_pending_formatted_inline_content($caption_command_name,
                                                       $prepended_text);
      $caption_text = $self->convert_tree_new_formatting_context(
               $caption->{'args'}->[0], 'float caption');
      my $cancelled_prepended
        = $self->cancel_pending_formatted_inline_content($caption_command_name);
      $prepended_text = '' if (not defined($cancelled_prepended));
    }
    if ($prepended_text ne '') {
      $prepended_text = '<p>'.$prepended_text.'</p>';
    }
  } else {
    $caption_text = $self->convert_tree_new_formatting_context(
      $caption->{'args'}->[0], 'float caption')
       if (defined($caption));
  }

  my $float_type_number_caption = '';
  if ($caption_text ne '') {
    $float_type_number_caption
      = $self->html_attribute_class('div', [$caption_command_name]). '>'
                       .$caption_text.'</div>';
  } elsif (defined($prepended) and $prepended_text ne '') {
    $float_type_number_caption
      = $self->html_attribute_class('div', ['type-number-float']). '>'
                       . $prepended_text .'</div>';
  }
  return $self->html_attribute_class('div', [$cmdname]). "${id_str}>\n"
     . $content . $float_type_number_caption . '</div>';
}
$default_commands_conversion{'float'} = \&_convert_float_command;

sub _convert_quotation_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  $self->cancel_pending_formatted_inline_content($cmdname);

  my @classes;

  my $main_cmdname;
  if ($small_block_associated_command{$cmdname}) {
    push @classes, $cmdname;
    $main_cmdname = $small_block_associated_command{$cmdname};
  } else {
    $main_cmdname = $cmdname;
  }
  unshift @classes, $main_cmdname;

  my $attribution = '';
  if ($command->{'extra'} and $command->{'extra'}->{'authors'}) {
    # FIXME there is no easy way to mark with a class the @author
    # @-command.  Add a span or a div (@center is in a div)?
    foreach my $author (@{$command->{'extra'}->{'authors'}}) {
      if ($author->{'args'}->[0]
          and $author->{'args'}->[0]->{'contents'}) {
        # TRANSLATORS: quotation author
        my $centered_author = $self->gdt("\@center --- \@emph{{author}}",
           {'author' => $author->{'args'}->[0]->{'contents'}});
        $centered_author->{'parent'} = $command;
        $attribution .= $self->convert_tree($centered_author,
                                            'convert quotation author');
      }
    }
  }

  if (!$self->in_string()) {
    return $self->html_attribute_class('blockquote', \@classes).">\n"
                           . $content . "</blockquote>\n" . $attribution;
  } else {
    return $content.$attribution;
  }
}
$default_commands_conversion{'quotation'} = \&_convert_quotation_command;

sub _convert_cartouche_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  return $content if ($self->in_string());

  my $title_content = '';
  if ($args->[0] and $args->[0]->{'normal'} ne '') {
    $title_content = "<tr><th>\n". $args->[0]->{'normal'} ."</th></tr>";
  }
  my $cartouche_content = '';
  if ($content =~ /\S/) {
    $cartouche_content = "<tr><td>\n". $content ."</td></tr>";
  }
  if ($cartouche_content ne '' or $title_content ne '') {
    return $self->html_attribute_class('table', [$cmdname])
       . " border=\"1\">${title_content}${cartouche_content}"
       . "</table>\n";
  }
  return $content;
}

$default_commands_conversion{'cartouche'} = \&_convert_cartouche_command;

sub _convert_itemize_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  if ($self->in_string()) {
    return $content;
  }
  my $command_as_argument_name;
  my $mark_class_name;
  if (defined($command->{'extra'})
      and defined($command->{'extra'}->{'command_as_argument'})) {
    my $command_as_argument = $command->{'extra'}->{'command_as_argument'};
    if ($command_as_argument->{'cmdname'} eq 'click'
        and $command_as_argument->{'extra'}->{'clickstyle'}) {
      $command_as_argument_name = $command_as_argument->{'extra'}->{'clickstyle'};
    } else {
      $command_as_argument_name = $command_as_argument->{'cmdname'};
    }

    if ($command_as_argument_name eq 'w') {
      $mark_class_name = 'none';
    } else {
      $mark_class_name = $command_as_argument_name;
    }
  }

  if (defined($mark_class_name)
      and defined($self->css_get_info('style', 'ul.mark-'.$mark_class_name))) {
    return $self->html_attribute_class('ul', [$cmdname,
                                              'mark-'.$mark_class_name])
        .">\n" . $content. "</ul>\n";
  } elsif ($self->get_conf('NO_CSS')) {
    return $self->html_attribute_class('ul', [$cmdname])
         .">\n" . $content. "</ul>\n";
  } else {
    my $css_string
      = $self->html_convert_css_string_for_list_mark($command->{'args'}->[0],
                                                      'itemize arg');
    if ($css_string ne '') {
      return $self->html_attribute_class('ul', [$cmdname])
        ." style=\"list-style-type: '".
          &{$self->formatting_function('format_protect_text')}($self,
                                                               $css_string)
             . "'\">\n" . $content. "</ul>\n";
    } else {
      return $self->html_attribute_class('ul', [$cmdname])
        .">\n" . $content. "</ul>\n";
    }
  }
}

$default_commands_conversion{'itemize'} = \&_convert_itemize_command;

sub _convert_enumerate_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  if ($self->in_string()) {
    return $content;
  }
  if ($content eq '') {
    return '';
  }
  my $type_attribute = '';
  my $start_attribute = '';
  my $specification = $command->{'extra'}->{'enumerate_specification'};
  if (defined $specification) {
    my ($start, $type);
    if ($specification =~ /^\d*$/ and $specification ne '1') {
      $start = $specification;
    } elsif ($specification =~ /^[A-Z]$/) {
      $start = 1 + ord($specification) - ord('A');
      $type = 'A';
    } elsif ($specification =~ /^[a-z]$/) {
      $start = 1 + ord($specification) - ord('a');
      $type = 'a';
    }
    $type_attribute = " type=\"$type\"" if (defined($type));
    $start_attribute = " start=\"$start\"" if (defined($start));
  }
  return $self->html_attribute_class('ol', [$cmdname]).$type_attribute
       .$start_attribute.">\n" . $content . "</ol>\n";
}

$default_commands_conversion{'enumerate'} = \&_convert_enumerate_command;

sub _convert_multitable_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  if ($self->in_string()) {
    return $content;
  }
  if ($content =~ /\S/) {
    return $self->html_attribute_class('table', [$cmdname]).">\n"
                                     . $content . "</table>\n";
  } else {
    return '';
  }
}

$default_commands_conversion{'multitable'} = \&_convert_multitable_command;

sub _convert_xtable_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  if ($self->in_string()) {
    return $content;
  }
  if ($content ne '') {
    return $self->html_attribute_class('dl', [$cmdname]).">\n"
      . $content . "</dl>\n";
  } else {
    return '';
  }
}
$default_commands_conversion{'table'} = \&_convert_xtable_command;
$default_commands_conversion{'ftable'} = \&_convert_xtable_command;
$default_commands_conversion{'vtable'} = \&_convert_xtable_command;

sub _convert_item_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  if ($self->in_string()) {
    return $content;
  }
  if ($command->{'parent'}->{'cmdname'}
      and $command->{'parent'}->{'cmdname'} eq 'itemize') {
    if ($content =~ /\S/) {
      return '<li>' . $content . '</li>';
    } else {
      return '';
    }
  } elsif ($command->{'parent'}->{'cmdname'}
      and $command->{'parent'}->{'cmdname'} eq 'enumerate') {
    if ($content =~ /\S/) {
      return '<li>' . ' ' . $content . '</li>';
    } else {
      return '';
    }
  } elsif ($command->{'parent'}->{'type'}
           and $command->{'parent'}->{'type'} eq 'table_term') {
    if ($args->[0]) {
      my $table_item_tree = $self->table_item_content_tree($command,
                                                [$args->[0]->{'tree'}]);
      my $result = $self->convert_tree($table_item_tree,
                                       'convert table_item_tree');
      if ($self->in_preformatted()) {
        my @pre_classes = $self->preformatted_classes_stack();
        foreach my $pre_class (@pre_classes) {
          if ($preformatted_code_commands{$pre_class}) {
            $result = $self->html_attribute_class('code',
                                    ['table-term-preformatted-code']).'>'
                        . $result . '</code>';
            last;
          }
        }
      }
      my $open_tag = ($cmdname eq 'item') ? '' : '<dt>';
      my $index_id = $self->command_id($command);
      my $anchor;
      my $anchor_span_open = '';
      my $anchor_span_close = '';
      if (defined($index_id)) {
        $anchor = $self->_get_copiable_anchor($index_id);
        $index_id = "<a id=\"$index_id\"></a>";
        if ($anchor ne '') {
          $anchor_span_open = '<span>';
          $anchor_span_close = '</span>';
        }
      } else {
        $anchor = '';
        $index_id = '';
      }
      return "$open_tag$index_id$anchor_span_open$result$anchor$anchor_span_close</dt>\n";
    } else {
      return '';
    }
  } elsif ($command->{'parent'}->{'type'}
           and $command->{'parent'}->{'type'} eq 'row') {
    return &{$self->command_conversion('tab')}($self, $cmdname, $command,
                                                           $args, $content);
  }
  return '';
}
$default_commands_conversion{'item'} = \&_convert_item_command;
$default_commands_conversion{'headitem'} = \&_convert_item_command;
$default_commands_conversion{'itemx'} = \&_convert_item_command;

sub _convert_tab_command($$$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  my $cell_nr = $command->{'extra'}->{'cell_number'};
  my $row = $command->{'parent'};
  my $row_cmdname = $row->{'contents'}->[0]->{'cmdname'};
  my $multitable = $row->{'parent'}->{'parent'};

  my $fractions = '';
  my $cf = $multitable->{'extra'}->{'columnfractions'};
  if ($cf) {
    if (exists($cf->{'extra'}->{'misc_args'}->[$cell_nr-1])) {
      my $fraction = sprintf('%d',
                             100*$cf->{'extra'}->{'misc_args'}->[$cell_nr-1]);
      $fractions = " width=\"$fraction%\"";
    }
  }

  $content =~ s/^\s*//;
  $content =~ s/\s*$//;

  if ($self->in_string()) {
    return $content;
  }
  if ($row_cmdname eq 'headitem') {
    return "<th${fractions}>" . $content . '</th>';
  } else {
    return "<td${fractions}>" . $content . '</td>';
  }
}
$default_commands_conversion{'tab'} = \&_convert_tab_command;

sub _convert_xref_commands($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $root = shift;
  my $args = shift;

  my $tree;
  my $name;
  if ($cmdname ne 'link' and $cmdname ne 'inforef'
      and $args->[2]
      and defined($args->[2]->{'normal'}) and $args->[2]->{'normal'} ne '') {
    $name = $args->[2]->{'normal'};
  } elsif ($args->[1]
           and defined($args->[1]->{'normal'}) and $args->[1]->{'normal'} ne '') {
    $name = $args->[1]->{'normal'}
  }

  if ($cmdname eq 'link' or $cmdname eq 'inforef') {
    $args->[3] = $args->[2];
    $args->[2] = undef;
  }

  my $file_arg_tree;
  my $file = '';
  if ($args->[3]
      and defined($args->[3]->{'filenametext'})
      and $args->[3]->{'filenametext'} ne '') {
    $file_arg_tree = $args->[3]->{'tree'};
    $file = $args->[3]->{'filenametext'};
  }

  my $book = '';
  $book = $args->[4]->{'normal'}
    if ($args->[4] and defined($args->[4]->{'normal'}));

  my $node_arg = $root->{'args'}->[0];

  # internal reference
  if ($cmdname ne 'inforef' and $book eq '' and $file eq ''
      and $node_arg and $node_arg->{'extra'}
      and defined($node_arg->{'extra'}->{'normalized'})
      and !$node_arg->{'extra'}->{'manual_content'}
      and $self->label_command($node_arg->{'extra'}->{'normalized'})) {
    my $node
     = $self->label_command($node_arg->{'extra'}->{'normalized'});
    # This is the node if USE_NODES, otherwise this may be the sectioning
    # command (if the sectioning command is really associated to the node)
    my $command = $self->command_root_element_command($node);
    $command = $node if (!$node->{'extra'}->{'associated_section'}
                         or $node->{'extra'}->{'associated_section'} ne $command);

    my $href = $self->command_href($command, undef, $root);

    if (!defined($name)) {
      if ($self->get_conf('xrefautomaticsectiontitle') eq 'on'
         and $node->{'extra'}
         and $node->{'extra'}->{'associated_section'}
         # this condition avoids infinite recursions, indeed in that case
         # the node will be used and not the section.  There should not be
         # @*ref in nodes, and even if there are, it does not seems to be
         # possible to construct an infinite recursion with nodes only
         # as the node must both be a reference target and refer to a specific
         # target at the same time, which is not possible.
         and not grep {$_ eq $node->{'extra'}->{'associated_section'}}
                     @{$self->{'referred_command_stack'}}) {
        $command = $node->{'extra'}->{'associated_section'};
        $name = $self->command_text($command, 'text_nonumber');
      } elsif ($node->{'cmdname'} eq 'float') {
        if (!$self->get_conf('XREF_USE_FLOAT_LABEL')) {
          $name = $self->command_text($command);
        }
        if (!defined($name) or $name eq '') {
          if (defined($args->[0]->{'monospace'})) {
            $name = $args->[0]->{'monospace'};
          } else {
            $name = '';
          }
        }
      } elsif (!$self->get_conf('XREF_USE_NODE_NAME_ARG')
               and (defined($self->get_conf('XREF_USE_NODE_NAME_ARG'))
                    or !$self->in_preformatted())) {
        $name = $self->command_text($command, 'text_nonumber');
        #die "$command $command->{'normalized'}" if (!defined($name));
      } elsif (defined($args->[0]->{'monospace'})) {
        $name = $args->[0]->{'monospace'};
      } else {
        $name = '';
      }
    }
    my $reference = $name;
    $reference = $self->html_attribute_class('a', [$cmdname])
                      ." href=\"$href\">$name</a>" if ($href ne ''
                                                       and !$self->in_string());

    my $is_section = ($command->{'cmdname'} ne 'node'
                      and $command->{'cmdname'} ne 'anchor'
                      and $command->{'cmdname'} ne 'float');
    if ($cmdname eq 'pxref') {
      $tree = $self->gdt('see {reference_name}',
        { 'reference_name' => {'type' => '_converted', 'text' => $reference} });
    } elsif ($cmdname eq 'xref') {
      $tree = $self->gdt('See {reference_name}',
        { 'reference_name' => {'type' => '_converted', 'text' => $reference} });
    } elsif ($cmdname eq 'ref' or $cmdname eq 'link') {
      $tree = $self->gdt('{reference_name}',
         { 'reference_name' => {'type' => '_converted', 'text' => $reference} });
    }
  } else {
    # external reference

    # We setup a label_info based on the node argument and not directly the
    # node argument to be able to use the $file argument
    my $label_info = {};
    if ($node_arg->{'extra'}) {
      $label_info->{'node_content'} = $node_arg->{'extra'}->{'node_content'}
        if ($node_arg->{'extra'}->{'node_content'});
      $label_info->{'normalized'} = $node_arg->{'extra'}->{'normalized'}
        if (exists($node_arg->{'extra'}->{'normalized'}));
    }
    # file argument takes precedence over the file in the node (file)node entry
    if (defined($file_arg_tree) and $file ne '') {
      $label_info->{'manual_content'} = $file_arg_tree->{'contents'};
    } elsif ($node_arg and $node_arg->{'extra'}
             and $node_arg->{'extra'}->{'manual_content'}) {
      $label_info->{'manual_content'}
        = $node_arg->{'extra'}->{'manual_content'};
      my $file_with_node_tree = {'type' => '_code',
                                  'contents' => [@{$label_info->{'manual_content'}}]};
      $file = $self->convert_tree($file_with_node_tree, 'node file in ref');
    }
    my $href = $self->command_href($label_info, undef, $root);

    if ($book eq '') {
      if (!defined($name)) {
        my $node_name = $self->command_text($label_info);
        $name = $node_name;
      }
    } elsif (!defined($name) and $label_info->{'node_content'}) {
      my $node_no_file_tree = {'type' => '_code',
                               'contents' => [@{$label_info->{'node_content'}}]};
      my $node_name = $self->convert_tree($node_no_file_tree, 'node in ref');
      if (defined($node_name) and $node_name ne 'Top') {
        $name = $node_name;
      }
    }

    # not exactly sure when it happens.  Something like @ref{(file),,,Manual}?
    $name = $args->[0]->{'monospace'}
       if (!defined($name)
           # FIXME could it really be Top?
           and $args->[0]->{'monospace'} ne 'Top');
    $name = '' if (!defined($name));

    my $reference = $name;
    my $book_reference = '';
    if (!$self->in_string() and $href ne '') {
      # attribute to distiguish links to Texinfo manuals from other links
      # and to provide manual name of target
      my $manual_name_attribute = '';
      if ($file) {
        if (not $self->get_conf('NO_CUSTOM_HTML_ATTRIBUTE')) {
          $manual_name_attribute = "data-manual=\"".
           &{$self->formatting_function('format_protect_text')}($self, $file)."\" ";
        }
      }
      if ($name ne '') {
        $reference = "<a ${manual_name_attribute}href=\"$href\">$name</a>";
      } elsif ($book ne '') {
        $book_reference = "<a ${manual_name_attribute}href=\"$href\">$book</a>";
      }
    }
    if ($cmdname eq 'pxref') {
      if (($book ne '') and ($href ne '') and ($reference ne '')) {
        $tree = $self->gdt('see {reference} in @cite{{book}}',
            { 'reference' => {'type' => '_converted', 'text' => $reference},
              'book' => {'type' => '_converted', 'text' => $book }});
      } elsif ($book_reference ne '') {
        $tree = $self->gdt('see @cite{{book_reference}}',
            { 'book_reference' => {'type' => '_converted',
                                   'text' => $book_reference }});
      } elsif (($book ne '') and ($reference ne '')) {
        $tree = $self->gdt('see `{section}\' in @cite{{book}}',
            { 'section' => {'type' => '_converted', 'text' => $reference},
              'book' => {'type' => '_converted', 'text' => $book }});
      } elsif ($book ne '') { # should seldom or even never happen
        $tree = $self->gdt('see @cite{{book}}',
              {'book' => {'type' => '_converted', 'text' => $book }});
      } elsif ($href ne '') {
        $tree = $self->gdt('see {reference}',
             { 'reference' => {'type' => '_converted', 'text' => $reference} });
      } elsif ($reference ne '') {
        $tree = $self->gdt('see `{section}\'', {
              'section' => {'type' => '_converted', 'text' => $reference} });
      }
    } elsif ($cmdname eq 'xref' or $cmdname eq 'inforef') {
      if (($book ne '') and ($href ne '') and ($reference ne '')) {
        $tree = $self->gdt('See {reference} in @cite{{book}}',
            { 'reference' => {'type' => '_converted', 'text' => $reference},
              'book' => {'type' => '_converted', 'text' => $book }});
      } elsif ($book_reference ne '') {
        $tree = $self->gdt('See @cite{{book_reference}}',
            { 'book_reference' => {'type' => '_converted',
                                   'text' => $book_reference }});
      } elsif (($book ne '') and ($reference ne '')) {
        $tree = $self->gdt('See `{section}\' in @cite{{book}}',
            { 'section' => {'type' => '_converted', 'text' => $reference},
              'book' => {'type' => '_converted', 'text' => $book }});
      } elsif ($book ne '') { # should seldom or even never happen
        $tree = $self->gdt('See @cite{{book}}',
              {'book' => {'type' => '_converted', 'text' => $book }});
      } elsif ($href ne '') {
        $tree = $self->gdt('See {reference}',
             { 'reference' => {'type' => '_converted', 'text' => $reference} });
      } elsif ($reference ne '') {
        $tree = $self->gdt('See `{section}\'', {
              'section' => {'type' => '_converted', 'text' => $reference} });
      }
    } else { # @ref
      if (($book ne '') and ($href ne '') and ($reference ne '')) {
        $tree = $self->gdt('{reference} in @cite{{book}}',
            { 'reference' => {'type' => '_converted', 'text' => $reference},
              'book' => {'type' => '_converted', 'text' => $book }});
      } elsif ($book_reference ne '') {
        $tree = $self->gdt('@cite{{book_reference}}',
            { 'book_reference' => {'type' => '_converted',
                                   'text' => $book_reference }});
      } elsif (($book ne '') and ($reference ne '')) {
        $tree = $self->gdt('`{section}\' in @cite{{book}}',
            { 'section' => {'type' => '_converted', 'text' => $reference},
              'book' => {'type' => '_converted', 'text' => $book }});
      } elsif ($book ne '') { # should seldom or even never happen
        $tree = $self->gdt('@cite{{book}}',
              {'book' => {'type' => '_converted', 'text' => $book }});
      } elsif ($href ne '') {
        $tree = $self->gdt('{reference}',
             { 'reference' => {'type' => '_converted', 'text' => $reference} });
      } elsif ($reference ne '') {
        $tree = $self->gdt('`{section}\'', {
              'section' => {'type' => '_converted', 'text' => $reference} });
      }
    }
    if (!defined($tree)) {
      # May happen if there is no argument
      #die "external: $cmdname, ($args), '$name' '$file' '$book' '$href' '$reference'. tree undef";
      return '';
    }
  }
  return $self->convert_tree($tree, "convert xref $cmdname");
}
foreach my $command(keys(%ref_commands)) {
  $default_commands_conversion{$command} = \&_convert_xref_commands;
}

sub _convert_printindex_command($$$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;

  my $index_name;
  if ($command->{'extra'} and $command->{'extra'}->{'misc_args'}
      and defined($command->{'extra'}->{'misc_args'}->[0])) {
    $index_name = $command->{'extra'}->{'misc_args'}->[0];
  } else {
    return '';
  }
  my $index_entries_by_letter = $self->get_info('index_entries_by_letter');
  if (!defined($index_entries_by_letter)
      or !$index_entries_by_letter->{$index_name}
      or !@{$index_entries_by_letter->{$index_name}}) {
    return '';
  }

  #foreach my $letter_entry (@{$index_entries_by_letter->{$index_name}}) {
  #  print STDERR "IIIIIII $letter_entry->{'letter'}\n";
  #  foreach my $index_entry (@{$letter_entry->{'entries'}}) {
  #    print STDERR "   ".join('|', keys(%$index_entry))."||| $index_entry->{'key'}\n";
  #  }
  #}
  return '' if ($self->in_string());

  my %letter_id;
  my %letter_is_symbol;
  # First collect the links that are used in entries and in letter summaries
  my $symbol_idx = 0;
  foreach my $letter_entry (@{$index_entries_by_letter->{$index_name}}) {
    my $letter = $letter_entry->{'letter'};
    my $index_element_id = $self->from_element_direction('This', 'target');
    if (!defined($index_element_id)) {
      my ($root_element, $root_command)
          = $self->get_element_root_command_element($command);
      if ($root_command) {
        $index_element_id = $self->command_id($root_command);
      }
      if (not defined($index_element_id)) {
        # to avoid duplicate names, use a prefix that cannot happen in anchors
        my $target_prefix = 't_i';
        $index_element_id = $target_prefix;
      }
    }
    my $is_symbol = $letter !~ /^\p{Alpha}/;
    $letter_is_symbol{$letter} = $is_symbol;
    my $identifier;
    if ($is_symbol) {
      $symbol_idx++;
      $identifier = $index_element_id . "_${index_name}_symbol-$symbol_idx";
    } else {
      $identifier = $index_element_id . "_${index_name}_letter-${letter}";
    }
    $letter_id{$letter} = $identifier;
  }

  $self->_new_document_context($cmdname);

  # Next do the entries to determine the letters that are not empty
  my @letter_entries;
  my $result_index_entries = '';
  my $formatted_index_entries
    = $self->shared_conversion_state('formatted_index_entries', {});
  foreach my $letter_entry (@{$index_entries_by_letter->{$index_name}}) {
    my $letter = $letter_entry->{'letter'};
    my $entries_text = '';
    my $entry_nr = -1;
    # since we normalize, a different formatting will not trigger a new
    # formatting of the main entry or a subentry level.  This is the
    # same for Texinfo TeX
    my $normalized_entry_levels = [];
    foreach my $index_entry_ref (@{$letter_entry->{'entries'}}) {
      $entry_nr++;
      my $main_entry_element = $index_entry_ref->{'entry_element'};
      next if ($self->get_conf('NO_TOP_NODE_OUTPUT')
               and defined($main_entry_element->{'extra'}->{'element_node'})
               and $main_entry_element->{'extra'}->{'element_node'}->{'extra'}
               and $main_entry_element->{'extra'}->{'element_node'}
                                               ->{'extra'}->{'normalized'}
               and $main_entry_element->{'extra'}->{'element_node'}
                                       ->{'extra'}->{'normalized'} eq 'Top');

      # to avoid double error messages, call convert_tree_new_formatting_context
      # below with a multiple_pass argument if an entry was already formatted once,
      # for example if there are multiple printindex.
      if (!$formatted_index_entries->{$index_entry_ref}) {
        $formatted_index_entries->{$index_entry_ref} = 1;
      } else {
        $formatted_index_entries->{$index_entry_ref}++;
      }

      my $entry_content_element
          = Texinfo::Common::index_content_element($main_entry_element);

      my $in_code = 0;
      my $indices_information = $self->get_info('indices_information');
      $in_code = 1
       if ($indices_information->{$index_entry_ref->{'index_name'}}->{'in_code'});
      my $entry_ref_tree = {'contents' => [$entry_content_element]};
      $entry_ref_tree->{'type'} = '_code' if ($in_code);

      # index entry with @seeentry or @seealso
      if ($main_entry_element->{'extra'}
            and ($main_entry_element->{'extra'}->{'seeentry'}
              or $main_entry_element->{'extra'}->{'seealso'})) {
        my $referred_entry;
        my $seenentry = 1;
        if ($main_entry_element->{'extra'}->{'seeentry'}) {
          $referred_entry = $main_entry_element->{'extra'}->{'seeentry'};
        } else {
          $referred_entry = $main_entry_element->{'extra'}->{'seealso'};
          $seenentry = 0;
        }
        my @referred_contents;
        if ($referred_entry->{'args'} and $referred_entry->{'args'}->[0]
            and $referred_entry->{'args'}->[0]->{'contents'}) {
          @referred_contents
             = @{$referred_entry->{'args'}->[0]->{'contents'}};
        }
        my $referred_tree = {'contents' => \@referred_contents};
        $referred_tree->{'type'} = '_code' if ($in_code);
        my $entry;
        # for @seealso, to appear where chapter/node ususally appear
        my $reference = '';
        my $delimiter = '';
        my $entry_class;
        my $section_class;
        if ($seenentry) {
          my $result_tree;
          if ($in_code) {
            $result_tree
          # TRANSLATORS: redirect to another index entry
        = $self->gdt('@code{{main_index_entry}}, @emph{See} @code{{seenentry}}',
                                        {'main_index_entry' => $entry_ref_tree,
                                         'seenentry' => $referred_tree});
          } else {
            $result_tree
                 # TRANSLATORS: redirect to another index entry
               = $self->gdt('{main_index_entry}, @emph{See} {seenentry}',
                                        {'main_index_entry' => $entry_ref_tree,
                                         'seenentry' => $referred_tree});
          }
          if ($formatted_index_entries->{$index_entry_ref} > 1) {
            # call with multiple_pass argument
            $entry = $self->convert_tree_new_formatting_context($result_tree,
                 "index $index_name l $letter index entry $entry_nr seenentry",
                 "index formatted $formatted_index_entries->{$index_entry_ref}")
          } else {
            $entry = $self->convert_tree($result_tree,
                  "index $index_name l $letter index entry $entry_nr seenentry");
          }
          $entry_class = "$cmdname-index-see-entry";
          $section_class = "$cmdname-index-see-entry-section";
        } else {
          # TRANSLATORS: refer to another index entry
          my $reference_tree = $self->gdt('@emph{See also} {see_also_entry}',
                                       {'see_also_entry' => $referred_tree});
          if ($formatted_index_entries->{$index_entry_ref} > 1) {
            # call with multiple_pass argument
            $entry = $self->convert_tree_new_formatting_context($entry_ref_tree,
               "index $index_name l $letter index entry $entry_nr (with seealso)",
               "index formatted $formatted_index_entries->{$index_entry_ref}");
            $reference
               = $self->convert_tree_new_formatting_context($reference_tree,
                "index $index_name l $letter index entry $entry_nr seealso",
                 "index formatted $formatted_index_entries->{$index_entry_ref}");
          } else {
            $entry = $self->convert_tree($entry_ref_tree,
             "index $index_name l $letter index entry $entry_nr (with seealso)");
            $reference
               = $self->convert_tree_new_formatting_context($reference_tree,
                  "index $index_name l $letter index entry $entry_nr seealso");
          }
          $entry = '<code>' .$entry .'</code>' if ($in_code);
          $delimiter = $self->get_conf('INDEX_ENTRY_COLON');
          # TODO add the information that it is associated with see also?
          $entry_class = "$cmdname-index-entry";
          $section_class = "$cmdname-index-see-also";
        }

        $entries_text .= '<tr><td></td>'
         .$self->html_attribute_class('td', [$entry_class]).'>'
         . $entry .
          $delimiter . '</td>'
        .$self->html_attribute_class('td', [$section_class]).'>';
        $entries_text .= $reference;
        $entries_text .= "</td></tr>\n";

        $normalized_entry_levels = [];
        next;
      }

      # determine the trees and normalized main entry and subentries, to be
      # compared with the previous line normalized entries to determine
      # what is already formatted as part of the previous lines and
      # what levels should be added.  The last level is always formatted.
      my @new_normalized_entry_levels;
      my @entry_trees;
      $new_normalized_entry_levels[0]
        = uc(Texinfo::Convert::NodeNameNormalization::convert_to_normalized(
             $entry_ref_tree));
      $entry_trees[0] = $entry_ref_tree;
      my $subentry = $index_entry_ref->{'entry_element'};
      my $subentry_level = 1;
      my $subentries_max_level = 2;
      while ($subentry->{'extra'} and $subentry->{'extra'}->{'subentry'}
             and $subentry_level <= $subentries_max_level) {
        $subentry = $subentry->{'extra'}->{'subentry'};
        my @subentry_contents;
        if ($subentry->{'args'} and $subentry->{'args'}->[0]
            and $subentry->{'args'}->[0]->{'contents'}) {
          @subentry_contents = @{$subentry->{'args'}->[0]->{'contents'}};
        }
        my $subentry_tree = {'contents' => \@subentry_contents};
        $subentry_tree->{'type'} = '_code' if ($in_code);
        if ($subentry_level >= $subentries_max_level) {
          # at the max, concatenate the remaining subentries
          my $other_subentries_tree = $self->comma_index_subentries_tree($subentry);
          push @{$subentry_tree->{'contents'}},
             @{$other_subentries_tree->{'contents'}}
                if defined($other_subentries_tree);
        } else {
          push @new_normalized_entry_levels,
            uc(Texinfo::Convert::NodeNameNormalization::convert_to_normalized(
              $subentry_tree));
        }
        push @entry_trees, $subentry_tree;
        $subentry_level ++;
      }
      #print STDERR join('|', @new_normalized_entry_levels)."\n";
      # last entry, always converted, associated to chapter/node and
      # with an hyperlinh
      my $entry_tree = pop @entry_trees;
      # indentation level of the last entry
      my $entry_level = 0;

      # format the leading entries when there are subentries.
      # Each on a line with increasing indentation, no hyperlink.
      if (scalar(@entry_trees) > 0) {
        # find the level not already formatted as part of the previous lines
        my $starting_subentry_level = 0;
        foreach my $subentry_tree (@entry_trees) {
          if ((scalar(@$normalized_entry_levels) > $starting_subentry_level)
               and $normalized_entry_levels->[$starting_subentry_level]
                 eq $new_normalized_entry_levels[$starting_subentry_level]) {
          } else {
            last;
          }
          $starting_subentry_level ++;
        }
        $entry_level = $starting_subentry_level;
        foreach my $level ($starting_subentry_level .. scalar(@entry_trees)-1) {
          my $entry;
          if ($formatted_index_entries->{$index_entry_ref} > 1) {
            # call with multiple_pass argument
            $entry = $self->convert_tree_new_formatting_context($entry_trees[$level],
                   "index $index_name l $letter index entry $entry_nr subentry $level",
                   "index formatted $formatted_index_entries->{$index_entry_ref}")
          } else {
            $entry = $self->convert_tree($entry_trees[$level],
                  "index $index_name l $letter index entry $entry_nr subentry $level");
          }
          $entry = '<code>' .$entry .'</code>' if ($in_code);
          my @td_entry_classes = ("$cmdname-index-entry");
          # indent
          if ($level > 0) {
            push @td_entry_classes, "index-entry-level-$level";
          }
          $entries_text .= '<tr><td></td>'
           # FIXME same class used for leading element of the entry and
           # last element of the entry.  Could be different.
           .$self->html_attribute_class('td', \@td_entry_classes).'>'
           . $entry . '</td>'
           # empty cell, no section for this line
            . "<td></td></tr>\n";

          $entry_level = $level+1;
        }
      }

      my $entry;
      if ($formatted_index_entries->{$index_entry_ref} > 1) {
        # call with multiple_pass argument
        $entry = $self->convert_tree_new_formatting_context($entry_tree,
                       "index $index_name l $letter index entry $entry_nr",
                   "index formatted $formatted_index_entries->{$index_entry_ref}")
      } else {
        $entry = $self->convert_tree($entry_tree,
                            "index $index_name l $letter index entry $entry_nr");
      }

      next if ($entry !~ /\S/ and $entry_level == 0);

      $normalized_entry_levels = [@new_normalized_entry_levels];
      $entry = '<code>' .$entry .'</code>' if ($in_code);
      my $target_element = $index_entry_ref->{'entry_element'};
      $target_element = $index_entry_ref->{'entry_associated_element'}
         if ($index_entry_ref->{'entry_associated_element'});
      my $entry_href = $self->command_href($target_element);
      my $formatted_entry = "<a href=\"$entry_href\">$entry</a>";
      my @td_entry_classes = ("$cmdname-index-entry");
      # subentry
      if ($entry_level > 0) {
        push @td_entry_classes, "index-entry-level-$entry_level";
      }

      my $associated_command;
      if ($self->get_conf('NODE_NAME_IN_INDEX')) {
        $associated_command = $main_entry_element->{'extra'}->{'element_node'};
        if (!defined($associated_command)) {
          $associated_command
            = $self->command_node($target_element);
        }
        if (!defined($associated_command)
            # do not warn if the entry is in a special region, like titlepage
            and not $main_entry_element->{'extra'}->{'element_region'}
            and $formatted_index_entries->{$index_entry_ref} == 1) {
          # NOTE _noticed_line_warn is not used as printindex should not
          # happen in multiple tree parsing that lead to ignore_notice being set,
          # but the error message is printed only for the first entry formatting.
          $self->line_warn($self,
                           sprintf(
           __("entry for index `%s' for \@printindex %s outside of any node"),
                                   $index_entry_ref->{'index_name'},
                                   $index_name),
                           $main_entry_element->{'source_info'});
        }
      }
      if (!$associated_command) {
        $associated_command
          = $self->command_root_element_command($target_element);
        if (!$associated_command) {
          # Use Top if not associated command found
          $associated_command
            = $self->tree_unit_element_command(
                                   $self->global_direction_element('Top'));
          # NOTE the warning here catches the most relevant cases of
          # index entry that is not associated to the right command, which
          # are very few in the test suite.  There is also a warning in the
          # parser with a much broader scope with possible overlap, but the
          # overlap is not a problem.
          # NODE_NAME_IN_INDEX may be undef even with USE_NODES set if the
          # converter is called as convert() as in the test suite
          if (defined($self->get_conf('NODE_NAME_IN_INDEX'))
              and not $self->get_conf('NODE_NAME_IN_INDEX')
              # do not warn if the entry is in a special region, like titlepage
              and not $main_entry_element->{'extra'}->{'element_region'}
              and $formatted_index_entries->{$index_entry_ref} == 1) {
            # NOTE _noticed_line_warn is not used as printindex should not
            # happen in multiple tree parsing that lead to ignore_notice being set,
            # but the error message is printed only for the first entry formatting.
            # NOTE the index entry may be associated to a node in that case.
            $self->line_warn($self,
                             sprintf(
        __("entry for index `%s' for \@printindex %s outside of any section"),
                                     $index_entry_ref->{'index_name'},
                                     $index_name),
                             $main_entry_element->{'source_info'});
          }
        }
      }
      my ($associated_command_href, $associated_command_text);
      if ($associated_command) {
        $associated_command_href = $self->command_href($associated_command);
        $associated_command_text = $self->command_text($associated_command);
      }

      $entries_text .= '<tr><td></td>'
        .$self->html_attribute_class('td', \@td_entry_classes).'>'
         . $formatted_entry . $self->get_conf('INDEX_ENTRY_COLON') . '</td>'
        .$self->html_attribute_class('td', ["$cmdname-index-section"]).'>';
      $entries_text
        .= "<a href=\"$associated_command_href\">$associated_command_text</a>"
         if ($associated_command_href);
      $entries_text .= "</td></tr>\n";
    }
    # a letter and associated indice entries
    if ($entries_text ne '') {
      $result_index_entries .= '<tr>' .
        "<th id=\"$letter_id{$letter}\">".
        &{$self->formatting_function('format_protect_text')}($self, $letter)
        . "</th></tr>\n" . $entries_text
        . "<tr><td colspan=\"3\">".$self->get_conf('DEFAULT_RULE')."</td></tr>\n";
      push @letter_entries, $letter_entry;
    }
  }

  # Do the summary letters linking to the letters done above
  my @non_alpha = ();
  my @alpha = ();
  foreach my $letter_entry (@letter_entries) {
    my $letter = $letter_entry->{'letter'};
    my $summary_letter_link
      = $self->html_attribute_class('a',["summary-letter-$cmdname"])
       ." href=\"#$letter_id{$letter}\"><b>".
          &{$self->formatting_function('format_protect_text')}($self, $letter)
           .'</b></a>';
    if ($letter_is_symbol{$letter}) {
      push @non_alpha, $summary_letter_link;
    } else {
      push @alpha, $summary_letter_link;
    }
  }

  if (scalar(@non_alpha) + scalar(@alpha) == 0) {
    $self->_pop_document_context();
    return '';
  }

  my $non_breaking_space = $self->get_info('non_breaking_space');

  # Format the summary letters
  my $join = '';
  my $non_alpha_text = '';
  my $alpha_text = '';
  if (scalar(@non_alpha) + scalar(@alpha) > 1) {
    $join = " $non_breaking_space \n".$self->get_info('line_break_element')."\n"
      if (scalar(@non_alpha) and scalar(@alpha));
    if (scalar(@non_alpha)) {
      $non_alpha_text = join("\n $non_breaking_space \n", @non_alpha) . "\n";
    }
    if (scalar(@alpha)) {
      $alpha_text = join("\n $non_breaking_space \n", @alpha)
                    . "\n $non_breaking_space \n";
    }
  }
  my $result = $self->html_attribute_class('div',
                           [$cmdname, "$index_name-$cmdname"]).">\n";
  # format the summary
  if (scalar(@non_alpha) + scalar(@alpha) > 1) {
    my $summary_header = $self->html_attribute_class('table',
            ["$index_name-letters-header-$cmdname"]).'><tr><th>'
        # TRANSLATORS: before list of letters and symbols grouping index entries
      . $self->convert_tree($self->gdt('Jump to')) .": $non_breaking_space </th><td>" .
      $non_alpha_text . $join . $alpha_text . "</td></tr></table>\n";

    $result .= $summary_header;
  }

  # now format the index entries
  $result
   .= $self->html_attribute_class('table', ["$index_name-entries-$cmdname"])
    ." border=\"0\">\n" . '<tr><td></td>'
    . $self->html_attribute_class('th', ["entries-header-$cmdname"]).'>'
      # TRANSLATORS: index entries column header in index formatting
    . $self->convert_tree($self->gdt('Index Entry')) .'</th>'
    . $self->html_attribute_class('th', ["sections-header-$cmdname"]).'>'
      # TRANSLATORS: section of index entry column header in index formatting
    . $self->convert_tree($self->gdt('Section')) . "</th></tr>\n"
    . "<tr><td colspan=\"3\">".$self->get_conf('DEFAULT_RULE')
    ."</td></tr>\n";
  $result .= $result_index_entries;
  $result .= "</table>\n";

  $self->_pop_document_context();

  if (scalar(@non_alpha) + scalar(@alpha) > 1) {
    my $summary_footer = $self->html_attribute_class('table',
                 ["$index_name-letters-footer-$cmdname"]).'><tr><th>'
        # TRANSLATORS: before list of letters and symbols grouping index entries
      . $self->convert_tree($self->gdt('Jump to'))
      . ": $non_breaking_space </th><td>"
      . $non_alpha_text . $join . $alpha_text . "</td></tr></table>\n";
    $result .= $summary_footer
  }
  return $result . "</div>\n";
}
$default_commands_conversion{'printindex'} = \&_convert_printindex_command;

sub _contents_inline_element($$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;

  print STDERR "CONTENTS_INLINE $cmdname\n" if ($self->get_conf('DEBUG'));
  my $content = &{$self->formatting_function('format_contents')}($self,
                                                          $cmdname, $command);
  if ($content) {
    my ($special_element_variety, $special_element, $class_base,
        $special_element_direction)
          = $self->command_name_special_element_information($cmdname);
    # FIXME is element- the best prefix?
    my $result = $self->html_attribute_class('div', ["element-${class_base}"]);
    my $heading;
    if ($special_element) {
      my $id = $self->command_id($special_element);
      if (defined($id) and $id ne '') {
        $result .= " id=\"$id\"";
      }
      $heading = $self->command_text($special_element);
    } else {
      # happens when called as convert() and not output()
      #cluck "$cmdname special element not defined";
      my $heading_tree = $self->special_element_info('heading_tree',
                                             $special_element_variety);
      if (defined($heading_tree)) {
        $heading = $self->convert_tree($heading_tree,
                                       "convert $cmdname special heading");
      } else {
        $heading = '';
      }
    }
    $result .= ">\n";
    $result .= &{$self->formatting_function('format_heading_text')}($self,
                                  $cmdname, [$class_base.'-heading'], $heading,
                                  $self->get_conf('CHAPTER_HEADER_LEVEL'))."\n";
    $result .= $content . "</div>\n";
    return $result;
  }
  return '';
}

sub _convert_informative_command($$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;

  return '' if ($self->in_string());

  Texinfo::Common::set_informative_command_value($self, $command);

  return '';
}

foreach my $informative_command (@informative_global_commands) {
  $default_commands_conversion{$informative_command}
    = \&_convert_informative_command;
}

sub _convert_contents_command($$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;

  return '' if ($self->in_string());
  $cmdname = 'shortcontents' if ($cmdname eq 'summarycontents');

  Texinfo::Common::set_informative_command_value($self, $command);

  my $structuring = $self->get_info('structuring');
  if ($self->get_conf('CONTENTS_OUTPUT_LOCATION') eq 'inline'
      and ($cmdname eq 'contents' or $cmdname eq 'shortcontents')
      and $self->get_conf($cmdname)
      and $structuring and $structuring->{'sectioning_root'}
      and scalar(@{$structuring->{'sections_list'}}) > 1) {
    return $self->_contents_inline_element($cmdname, $command);
  }
  return '';
}

foreach my $contents_comand (@contents_commands) {
  $default_commands_conversion{$contents_comand} = \&_convert_contents_command;
}

# associate same formatting function for @small* command
# as for the associated @-command
foreach my $small_command (keys(%small_block_associated_command)) {
  $default_commands_conversion{$small_command}
    = $default_commands_conversion{$small_block_associated_command{$small_command}};
}

sub _open_quotation_command($$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;

  my $formatted_quotation_arg_to_prepend;
  if ($command->{'args'} and $command->{'args'}->[0]
      and $command->{'args'}->[0]->{'contents'}
      and @{$command->{'args'}->[0]->{'contents'}}) {
    $formatted_quotation_arg_to_prepend
     = $self->convert_tree($self->gdt('@b{{quotation_arg}:} ',
             {'quotation_arg' => $command->{'args'}->[0]->{'contents'}}),
                           "open $cmdname prepended arg");
  }
  $self->register_pending_formatted_inline_content($cmdname,
                                 $formatted_quotation_arg_to_prepend);
  return '';
}

$default_commands_open{'quotation'} = \&_open_quotation_command;

# associate same opening function for @small* command
# as for the associated @-command
foreach my $small_command (keys(%small_block_associated_command)) {
  if (exists($default_commands_open{$small_block_associated_command{$small_command}})) {
    $default_commands_open{$small_command}
      = $default_commands_open{$small_block_associated_command{$small_command}};
  }
}

# Keys are tree element types, values are function references to convert
# elements of that type.  Can be overridden accessing
# Texinfo::Config::GNUT_get_types_conversion, setup by
# Texinfo::Config::texinfo_register_type_formatting()
my %default_types_conversion;

sub default_type_conversion($$)
{
  my $self = shift;
  my $type = shift;
  return $default_types_conversion{$type};
}

sub type_conversion($$)
{
  my $self = shift;
  my $type = shift;
  return $self->{'types_conversion'}->{$type};
}

my %default_types_open;

sub default_type_open($$)
{
  my $self = shift;
  my $type = shift;
  return $default_types_open{$type};
}


# Ignored commands
foreach my $type ('ignorable_spaces_after_command', 'postamble_after_end',
            'preamble_before_beginning',
            'preamble_before_setfilename',
            'spaces_at_end',
            'spaces_before_paragraph',
            'spaces_after_close_brace') {
  $default_types_conversion{$type} = undef;
}

sub _convert_paragraph_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  $content = $self->get_associated_formatted_inline_content($element).$content;

  if ($self->paragraph_number() == 1) {
    my $in_format = $self->top_block_command();
    if ($in_format) {
      # no first paragraph in those environment to avoid extra spacing
      if ($in_format eq 'itemize'
          or $in_format eq 'enumerate'
          or $in_format eq 'multitable') {
        return $content;
      }
    }
  }
  return $content if ($self->in_string());

  if ($content =~ /\S/) {
    my $align = $self->in_align();
    if ($align and $HTML_align_commands{$align}) {
      return $self->html_attribute_class('p', [$align.'-paragraph']).">"
                             .$content."</p>";
    } else {
      return "<p>".$content."</p>";
    }
  } else {
    return '';
  }
}

$default_types_conversion{'paragraph'} = \&_convert_paragraph_type;


sub _open_inline_container_type($$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;

  my $pending_formatted = $self->get_pending_formatted_inline_content();

  if (defined($pending_formatted)) {
    $self->associate_pending_formatted_inline_content($element, $pending_formatted);
  }
  return '';
}

$default_types_open{'paragraph'} = \&_open_inline_container_type;
$default_types_open{'preformatted'} = \&_open_inline_container_type;


sub _preformatted_class()
{
  my $self = shift;
  my $pre_class;
  my @pre_classes = $self->preformatted_classes_stack();
  foreach my $class (@pre_classes) {
    # FIXME maybe add   or $pre_class eq 'menu'  to override
    # 'menu' with 'menu-comment'?
    $pre_class = $class unless ($pre_class
                           and $preformatted_code_commands{$pre_class}
                           and !($preformatted_code_commands{$class}
                                 or $class eq 'menu'));
  }
  return $pre_class.'-preformatted';
}

sub _convert_preformatted_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  if (!defined($content)) {
    cluck "content undef in _convert_preformatted_type "
       .Texinfo::Common::debug_print_element($element, 1);
  }

  $content = $self->get_associated_formatted_inline_content($element).$content;

  return '' if ($content eq '');

  my $pre_class = $self->_preformatted_class();

  if ($self->top_block_command() eq 'multitable') {
    $content =~ s/^\s*//;
    $content =~ s/\s*$//;
  }

  # menu_entry_description is always in a preformatted container
  # in the tree, as the whole menu is meant to be an
  # environment where spaces and newlines are preserved.
  if ($element->{'parent'}->{'type'}
      and $element->{'parent'}->{'type'} eq 'menu_entry_description') {
    if (!$self->_in_preformatted_in_menu()) {
      # If not in preformatted block command (nor in SIMPLE_MENU),
      # we don't preserve spaces and newlines in menu_entry_description,
      # instead the whole menu_entry is in a table, so no <pre> in that situation
      return $content;
    } else {
      # if directly in description, we want to avoid the linebreak that
      # comes with pre being a block level element, so set a special class
      $pre_class = 'menu-entry-description-preformatted';
    }
  }

  if ($self->in_string()) {
    return $content;
  }
  $content =~ s/^\n/\n\n/; # a newline immediately after a <pre> is ignored.
  my $result = $self->html_attribute_class('pre', [$pre_class]).'>'
                                                   . $content . '</pre>';

  # this may happen with lines without textual content
  # between a def* and def*x.
  if ($element->{'parent'}->{'cmdname'}
      and $element->{'parent'}->{'cmdname'} =~ /^def/) {
    $result = '<dd>'.$result.'</dd>';
  }
  return $result;
}

$default_types_conversion{'preformatted'} = \&_convert_preformatted_type;

sub _convert_balanced_braces_type($$$$) {
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  return $content;
}

$default_types_conversion{'balanced_braces'} = \&_convert_balanced_braces_type;

# use the type and not the index commands names, as they are diverse and
# can be dynamically added, so it is difficult to use as selector for output
# formatting.  The command name can be obtained here as $element->{'cmdname'}.
sub _convert_index_entry_command_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  my $index_id = $self->command_id($element);
  if (defined($index_id) and $index_id ne ''
      and !$self->in_multi_expanded()
      and !$self->in_string()) {
    my $result = &{$self->formatting_function('format_separate_anchor')}($self,
                                                   $index_id, 'index-entry-id');
    $result .= "\n" unless ($self->in_preformatted());
    return $result;
  }
  return '';
}
$default_types_conversion{'index_entry_command'} = \&_convert_index_entry_command_type;

sub _convert_definfoenclose_type($$$$) {
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  # FIXME add a span to mark the original command as a class?
  return &{$self->formatting_function('format_protect_text')}($self,
                                      $element->{'extra'}->{'begin'})
     . $content .
    &{$self->formatting_function('format_protect_text')}($self,
                                      $element->{'extra'}->{'end'});
}

$default_types_conversion{'definfoenclose_command'}
  = \&_convert_definfoenclose_type;

# Note: has an XS override
sub _entity_text
{
  my $text = shift;

  $text =~ s/---/\&mdash\;/g;
  $text =~ s/--/\&ndash\;/g;
  $text =~ s/``/\&ldquo\;/g;
  $text =~ s/''/\&rdquo\;/g;
  $text =~ s/'/\&rsquo\;/g;
  $text =~ s/`/\&lsquo\;/g;

  return $text;
}

sub _convert_text($$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $text = shift;

  my $context = $self->{'document_context'}->[-1];

  # API info: in_verbatim() API code conforming would be:
  #if ($self->in_verbatim()) {
  if ($context->{'verbatim'}) { # inline these calls for speed
    # API info: using the API to allow for customization would be:
    #return &{$self->formatting_function('format_protect_text')}($self, $text);
    return $self->_default_format_protect_text($text);
  }
  return $text if $context->{'raw'};
  # API info: in_raw() API code conforming would be:
  #return $text if ($self->in_raw());

  my $formatting_context = $context->{'formatting_context'}->[-1];
  $text = uc($text) if $formatting_context->{'upper_case'};
  # API info: in_upper_case() API code conforming would be:
  #$text = uc($text) if ($self->in_upper_case());

  # API info: using the API to allow for customization would be:
  #$text = &{$self->formatting_function('format_protect_text')}($self, $text);
  $text = _default_format_protect_text($self, $text);

  # API info: get_conf() API code conforming would be:
  #if ($self->get_conf('OUTPUT_CHARACTERS')
  #    and $self->get_conf('OUTPUT_ENCODING_NAME')
  #    and $self->get_conf('OUTPUT_ENCODING_NAME') eq 'utf-8') {
  if ($self->{'conf'}->{'OUTPUT_CHARACTERS'}
      and $self->{'conf'}->{'OUTPUT_ENCODING_NAME'}
      and $self->{'conf'}->{'OUTPUT_ENCODING_NAME'} eq 'utf-8') {
    $text = Texinfo::Convert::Unicode::unicode_text($text,
                                        (in_code($self) or in_math($self)));
  # API info: in_code() API code conforming and
  # API info: in_math() API code conforming would be:
  #} elsif (!$self->in_code() and !$self->in_math()) {
  } elsif (!$context->{'monospace'}->[-1] and !$context->{'math'}) {
    # API info: get_conf() API code conforming would be:
    #if ($self->get_conf('USE_NUMERIC_ENTITY')) {
    if ($self->{'conf'}->{'USE_NUMERIC_ENTITY'}) {
      $text = $self->xml_format_text_with_numeric_entities($text);
    # API info: get_conf() API code conforming would be:
    #} elsif ($self->get_conf('USE_ISO')) {
    } elsif ($self->{'conf'}->{'USE_ISO'}) {
      $text = _entity_text($text);
    } else {
      $text =~ s/``/&quot;/g;
      $text =~ s/''/&quot;/g;
      $text =~ s/---/\x{1F}/g;
      $text =~ s/--/-/g;
      $text =~ s/\x{1F}/--/g;
    }
  }

  return $text if (in_preformatted($self));

  # API info: in_non_breakable_space() API code conforming would be:
  #if ($self->in_non_breakable_space()) {
  if ($formatting_context->{'no_break'}) {
    my $non_breaking_space = $self->get_info('non_breaking_space');
    $text =~ s/\n/ /g;
    $text =~ s/ +/$non_breaking_space/g;
  # API info: in_space_protected() API code conforming would be:
  #} elsif ($self->in_space_protected()) {
  } elsif ($formatting_context->{'space_protected'}) {
    if (chomp($text)) {
      # API info: API code conforming would be:
      # $self->get_info('line_break_element')
      my $line_break_element = $self->{'line_break_element'};
      # protect spaces in line_break_element formatting.
      # Note that this case is theoretical right now, as it is not possible
      # to redefine $self->{'line_break_element'} and there are no spaces
      # in the possible values.  However this is a deficiency of the API,
      # it would be better to be able to redefine $self->{'line_break_element'}
      $line_break_element =~ s/ /\x{1F}/g;
      $text .= $line_break_element;
    }
    # Protect spaces within text
    my $non_breaking_space = $self->get_info('non_breaking_space');
    $text =~ s/ /$non_breaking_space/g;
    # Revert protected spaces in leading html attribute
    $text =~ s/\x{1F}/ /g;
  }
  return $text;
}

$default_types_conversion{'text'} = \&_convert_text;

sub _css_string_convert_text($$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $text = shift;

  $text = uc($text) if ($self->in_upper_case());

  # need to hide \ otherwise it is protected in protect_text
  if (!$self->in_code() and !$self->in_math()) {
    $text =~ s/---/\x{1F}2014 /g;
    $text =~ s/--/\x{1F}2013 /g;
    $text =~ s/``/\x{1F}201C /g;
    $text =~ s/''/\x{1F}201D /g;
    $text =~ s/'/\x{1F}2019 /g;
    $text =~ s/`/\x{1F}2018 /g;
  }

  $text
   = &{$self->formatting_function('format_protect_text')}($self, $text);
  $text =~ s/\x{1F}/\\/g;

  return $text;
}
$default_css_string_types_conversion{'text'} = \&_css_string_convert_text;

sub _simplify_text_for_comparison($)
{
  my $text = shift;
  $text =~ s/[^\p{Word}]//g;
  return $text;
}

sub _convert_row_type($$$$) {
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  return $content if ($self->in_string());
  if ($content =~ /\S/) {
    my $result = '<tr>' . $content . '</tr>';
    if ($element->{'contents'}
        and scalar(@{$element->{'contents'}})
        and $element->{'contents'}->[0]->{'cmdname'} ne 'headitem') {
      # if headitem, end of line added in _convert_multitable_head_type
      $result .= "\n";
    }
    return $result;
  } else {
    return '';
  }
}
$default_types_conversion{'row'} = \&_convert_row_type;

sub _convert_multitable_head_type($$$$) {
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  return $content if ($self->in_string());
  if ($content =~ /\S/) {
    return '<thead>' . $content . '</thead>' . "\n";
  } else {
    return '';
  }
}

$default_types_conversion{'multitable_head'} = \&_convert_multitable_head_type;

sub _convert_multitable_body_type($$$$) {
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  return $content if ($self->in_string());
  if ($content =~ /\S/) {
    return '<tbody>' . $content . '</tbody>' . "\n";
  } else {
    return '';
  }
}

$default_types_conversion{'multitable_body'} = \&_convert_multitable_body_type;

sub _convert_menu_entry_type($$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;

  my $name_entry;
  my $menu_description;
  my $menu_entry_node;
  my $menu_entry_leading_text;
  my @menu_entry_separators;

  foreach my $arg (@{$element->{'contents'}}) {
    if ($arg->{'type'} eq 'menu_entry_leading_text') {
      $menu_entry_leading_text = $arg;
    } elsif ($arg->{'type'} eq 'menu_entry_name') {
      $name_entry = $arg;
    } elsif ($arg->{'type'} eq 'menu_entry_description') {
      $menu_description = $arg;
    } elsif ($arg->{'type'} eq 'menu_entry_separator') {
      push @menu_entry_separators, $arg;
    } elsif ($arg->{'type'} eq 'menu_entry_node') {
      $menu_entry_node = $arg;
    }
  }

  my $href = '';
  my $rel = '';
  my $section;
  my $label_info = $menu_entry_node->{'extra'};

  # external node
  my $external_node;
  if ($label_info and $label_info->{'manual_content'}) {
    $href = $self->command_href($label_info, undef, $element);
    $external_node = 1;
  # may not be defined in case of menu entry node consisting only of spaces
  } elsif ($label_info and defined($label_info->{'normalized'})) {
    my $node = $self->label_command($label_info->{'normalized'});
    if ($node) {
      # if !NODE_NAME_IN_MENU, we pick the associated section, except if
      # the node is the element command
      if ($node->{'extra'}
          and $node->{'extra'}->{'associated_section'}
          and !$self->get_conf('NODE_NAME_IN_MENU')
          and !($self->command_root_element_command($node) eq $node)) {
        $section = $node->{'extra'}->{'associated_section'};
        $href = $self->command_href($section, undef, $element);
      } else {
        $href = $self->command_href($node, undef, $element);
      }
      if ($node->{'extra'} and $node->{'extra'}->{'isindex'}) {
        # Mark the target as an index.  See
        # http://microformats.org/wiki/existing-rel-values#HTML5_link_type_extensions
        $rel = ' rel="index"';
      }
    }
  }

  my $html_menu_entry_index
    = $self->shared_conversion_state('html_menu_entry_index', 0);
  ${$html_menu_entry_index}++;
  my $accesskey = '';
  $accesskey = " accesskey=\"$$html_menu_entry_index\""
    if ($self->get_conf('USE_ACCESSKEY') and $$html_menu_entry_index < 10);

  my $MENU_SYMBOL = $self->get_conf('MENU_SYMBOL');
  my $MENU_ENTRY_COLON = $self->get_conf('MENU_ENTRY_COLON');

  my $in_string = $self->in_string();
  if ($self->_in_preformatted_in_menu() or $in_string) {
    my $leading_text = $menu_entry_leading_text->{'text'};
    $leading_text =~ s/\*/$MENU_SYMBOL/;
    my $result_name_node = $leading_text;

    if ($name_entry) {
      $result_name_node
        .= $self->convert_tree($name_entry,
                               "menu_arg menu_entry_name preformatted");
      my $name_separator = shift @menu_entry_separators;
      $result_name_node
        .= $self->convert_tree($name_separator,
                               "menu_arg name separator preformatted");
    }

    if ($menu_entry_node) {
      # 'contents' seems to always be defined.  If it is
      # not the case, it should not be an issue as an undefined
      # 'contents' is ignored.
      my $name = $self->convert_tree(
         {'type' => '_code',
          'contents' => $menu_entry_node->{'contents'}},
                       "menu_arg menu_entry_node preformatted");
      if ($href ne '' and !$in_string) {
        $result_name_node .= "<a href=\"$href\"$rel$accesskey>".$name."</a>";
      } else {
        $result_name_node .= $name;
      }
    }
    if (scalar(@menu_entry_separators)) {
      my $node_separator = shift @menu_entry_separators;
      $result_name_node
        .= $self->convert_tree($node_separator,
                               "menu_arg node separator preformatted");
    }

    if (!$self->get_conf('SIMPLE_MENU') and not $in_string) {
      my $pre_class = $self->_preformatted_class();
      $result_name_node = $self->html_attribute_class('pre', [$pre_class]).'>'
                                               . $result_name_node . '</pre>';
    }

    my $description = '';
    if ($menu_description) {
      $description .= $self->convert_tree($menu_description,
                                          "menu_arg description preformatted");
    }

    return $result_name_node . $description;
  }

  my $name;
  my $name_no_number;
  if ($section) {
    $name = $self->command_text($section);
    $name_no_number = $self->command_text($section, 'text_nonumber');
    if ($href ne '' and $name ne '') {
      $name = "<a href=\"$href\"$rel$accesskey>".$name."</a>";
    }
  }
  if (!defined($name) or $name eq '') {
    if ($name_entry) {
      $name = $self->convert_tree($name_entry, 'convert menu_entry_name');
    }
    if (!defined($name) or $name eq '') {
      if ($label_info and $label_info->{'manual_content'}) {
        $name = $self->command_text($label_info);
      } elsif ($label_info) {
        $name = $self->convert_tree({'type' => '_code',
                          'contents' => $label_info->{'node_content'}},
                          'menu_arg name');
      } else {
        $name = '';
      }
    }
    $name =~ s/^\s*//;
    $name_no_number = $name;
    if ($href ne '') {
      $name = "<a href=\"$href\"$rel$accesskey>".$name."</a>";
    }
    $name = "$MENU_SYMBOL ".$name;
  }
  my $description = '';
  if ($menu_description) {
    $description = $self->convert_tree($menu_description,
                                       'menu_arg description');
    if ($self->get_conf('AVOID_MENU_REDUNDANCY')) {
      $description = '' if (_simplify_text_for_comparison($name_no_number)
                           eq _simplify_text_for_comparison($description));
    }
  }
  my $non_breaking_space = $self->get_info('non_breaking_space');
  return '<tr>'
     .$self->html_attribute_class('td', ['menu-entry-destination']).'>'
                                           ."$name$MENU_ENTRY_COLON</td>"
    ."<td>${non_breaking_space}${non_breaking_space}</td>"
    .$self->html_attribute_class('td', ['menu-entry-description']).'>'
                                ."$description</td></tr>\n";
}

$default_types_conversion{'menu_entry'} = \&_convert_menu_entry_type;

sub _convert_menu_comment_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  if ($self->_in_preformatted_in_menu() or $self->in_string()) {
    return $content;
  } else {
    return '<tr>'.$self->html_attribute_class('th', ['menu-comment'])
      . ' colspan="3">'.$content .'</th></tr>';
  }
}

$default_types_conversion{'menu_comment'} = \&_convert_menu_comment_type;

sub _convert_before_item_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  return '' if ($content !~ /\S/);
  return $content if ($self->in_string());
  my $top_block_command = $self->top_block_command();
  if ($top_block_command eq 'itemize' or $top_block_command eq 'enumerate') {
    return '<li>'. $content .'</li>';
  } elsif ($top_block_command eq 'table' or $top_block_command eq 'vtable'
           or $top_block_command eq 'ftable') {
    return '<dd>'. $content .'</dd>'."\n";
  } elsif ($top_block_command eq 'multitable') {
    $content =~ s/^\s*//;
    $content =~ s/\s*$//;

    return '<tr><td>'.$content.'</td></tr>'."\n";
  }
}

$default_types_conversion{'before_item'} = \&_convert_before_item_type;

sub _convert_table_term_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  return '<dt>'.$content;
}

$default_types_conversion{'table_term'} = \&_convert_table_term_type;

sub _convert_def_line_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  if ($self->in_string()) {
    # should probably never happen
    return &{$self->formatting_function('format_protect_text')}($self,
     Texinfo::Convert::Text::convert_to_text(
      $element, Texinfo::Convert::Text::copy_options_for_convert_text($self)));
  }

  my $index_label = '';
  my $index_id = $self->command_id($element);
  if (defined($index_id) and $index_id ne '' and !$self->in_multi_expanded()) {
    $index_label = " id=\"$index_id\"";
  }
  my ($category_element, $class_element,
      $type_element, $name_element, $arguments)
         = Texinfo::Convert::Utils::definition_arguments_content($element);

  my @classes = ();
  my $command_name;
  if ($Texinfo::Common::def_aliases{$element->{'extra'}->{'def_command'}}) {
    $command_name
        = $Texinfo::Common::def_aliases{$element->{'extra'}->{'def_command'}};
  } else {
    $command_name = $element->{'extra'}->{'def_command'};
  }
  my $original_command_name;
  if ($Texinfo::Common::def_aliases{$element->{'extra'}->{'original_def_cmdname'}}) {
    my $original_def_cmdname = $element->{'extra'}->{'original_def_cmdname'};
    $original_command_name = $Texinfo::Common::def_aliases{$original_def_cmdname};
    push @classes, "$original_def_cmdname-alias-$original_command_name";
  } else {
    $original_command_name = $element->{'extra'}->{'original_def_cmdname'};
  }
  if ($command_name ne $original_command_name) {
    push @classes, "def-cmd-$command_name";
  }
  unshift @classes, $original_command_name;

  my $result_type = '';
  if ($type_element) {
    my $type_text = $self->_convert({'type' => '_code',
       'contents' => [$type_element]});
    if ($type_text ne '') {
      $result_type = $self->html_attribute_class('code', ['def-type']).'>'.
         $type_text .'</code>';
    }
    if ($self->get_conf('deftypefnnewline') eq 'on'
        and ($command_name eq 'deftypefn' or $command_name eq 'deftypeop')) {
      $result_type .= $self->get_info('line_break_element');
    }
  }

  my $result_name = '';
  if ($name_element) {
    $result_name = $self->html_attribute_class('strong', ['def-name']).'>'.
       $self->_convert({'type' => '_code', 'contents' => [$name_element]})
       .'</strong>';
  }

  my $def_space = ' ';
  if ($element->{'extra'}->{'omit_def_name_space'}) {
    $def_space = '';
  }

  my $result_arguments = '';
  if ($arguments) {
  # arguments not only metasyntactic variables
  # (deftypefn, deftypevr, deftypeop, deftypecv)
    if ($Texinfo::Common::def_no_var_arg_commands{$command_name}) {
      my $arguments_formatted = $self->_convert({'type' => '_code',
                                                 'contents' => $arguments});
      $result_arguments = $self->html_attribute_class('code',
                                      ['def-code-arguments']).'>'
                          . $arguments_formatted.'</code>'
          if ($arguments_formatted =~ /\S/);
    } else {
      # only metasyntactic variable arguments (deffn, defvr, deftp, defop, defcv)
      push @{$self->{'document_context'}->[-1]->{'monospace'}}, 0;
      my $arguments_formatted = $self->_convert({'contents' => $arguments});
      pop @{$self->{'document_context'}->[-1]->{'monospace'}};
      if ($arguments_formatted =~ /\S/) {
        $result_arguments = $self->html_attribute_class('var',
                               ['def-var-arguments']).'>'
              . $arguments_formatted .'</var>';
      }
    }
  }

  my $def_call = '';
  $def_call .= $result_type . ' ' if ($result_type ne '');
  $def_call .= $result_name;
  $def_call .= $def_space . $result_arguments if ($result_arguments ne '');

  if ($self->get_conf('DEF_TABLE')) {
    my $category_result = '';
    my $definition_category_tree
      = Texinfo::Convert::Utils::definition_category_tree($self, $element);
    $category_result
      = $self->convert_tree({'contents' => [$definition_category_tree]})
        if (defined($definition_category_tree));

    return $self->html_attribute_class('tr', \@classes)
      . "$index_label>".$self->html_attribute_class('td', ['call-def']).'>'
      . $def_call . '</td>'.$self->html_attribute_class('td', ['category-def'])
      . '>' . '[' . $category_result . ']' . "</td></tr>\n";
  }

  my $category_result = '';
  my $category_tree;
  if ($category_element) {
    if ($class_element) {
      if ($command_name eq 'deftypeop'
          and $type_element
          and $self->get_conf('deftypefnnewline') eq 'on') {
        $category_tree = $self->gdt('{category} on @code{{class}}:@* ',
              {'category' => $category_element,
              'class' => $class_element});
      } elsif ($command_name eq 'defop' or $command_name eq 'deftypeop') {
        $category_tree = $self->gdt('{category} on @code{{class}}: ',
              {'category' => $category_element,
              'class' => $class_element});
      } elsif ($command_name eq 'defcv' or $command_name eq 'deftypecv') {
        $category_tree = $self->gdt('{category} of @code{{class}}: ',
              {'category' => $category_element,
              'class' => $class_element});
      }
    } elsif ($type_element
            and ($command_name eq 'deftypefn' or $command_name eq 'deftypeop')
            and $self->get_conf('deftypefnnewline') eq 'on') {
        # FIXME if in @def* in @example and with @deftypefnnewline
        # on there is no effect of @deftypefnnewline on, as @* in
        # preformatted environment becomes an end of line, but the def*
        # line is not in a preformatted environment.  There should be
        # an explicit <br> in that case.  Probably requires changing
        # the conversion of @* in a @def* line in preformatted, nothing
        # really specific of @deftypefnnewline on.
        $category_tree = $self->gdt('{category}:@* ',
                                    {'category' => $category_element});
    } else {
      $category_tree = $self->gdt('{category}: ', {'category' => $category_element});
    }
    $category_result = $self->convert_tree($category_tree);
  }

  if ($category_result ne '') {
    my $open = $self->html_attribute_class('span', ['category-def']);
    if ($open ne '') {
      $category_result = $open.'>'.$category_result.'</span>';
    }
  }
  my $anchor_span_open = '';
  my $anchor_span_close = '';
  my $anchor = $self->_get_copiable_anchor($index_id);
  if ($anchor ne '') {
    $anchor_span_open = '<span>';
    $anchor_span_close = '</span>';
  }
  return $self->html_attribute_class('dt', \@classes)
       . "$index_label>" . $category_result . $anchor_span_open
       . $def_call
       . "$anchor$anchor_span_close</dt>\n";
}

sub _get_copiable_anchor {
  my ($self, $id) = @_;
  my $result = '';
  if ($id and $self->get_conf('COPIABLE_LINKS')) {
    my $paragraph_symbol = $self->get_info('paragraph_symbol');
    $result = $self->html_attribute_class('a', ['copiable-link'])
        ." href=\"#$id\"> $paragraph_symbol</a>";
  }
  return $result;
}

$default_types_conversion{'def_line'} = \&_convert_def_line_type;

sub _convert_def_item_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  return $content if ($self->in_string());
  if ($content =~ /\S/) {
    if (! $self->get_conf('DEF_TABLE')) {
      return '<dd>' . $content . '</dd>';
    } else {
      return '<tr><td colspan="2">' . $content . '</td></tr>';
    }
  }
}

$default_types_conversion{'def_item'} = \&_convert_def_item_type;
$default_types_conversion{'inter_def_item'} = \&_convert_def_item_type;

sub _convert_def_command($$$$$) {
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $args = shift;
  my $content = shift;

  return $content if ($self->in_string());

  my @classes;
  my $command_name;
  if ($Texinfo::Common::def_aliases{$cmdname}) {
    $command_name = $Texinfo::Common::def_aliases{$cmdname};
    push @classes, "first-$cmdname-alias-first-$command_name";
  } else {
    $command_name = $cmdname;
  }
  unshift @classes, "first-$command_name";

  if (!$self->get_conf('DEF_TABLE')) {
    return $self->html_attribute_class('dl', \@classes).">\n"
                                        . $content ."</dl>\n";
  } else {
    return $self->html_attribute_class('table', \@classes)." width=\"100%\">\n"
                                                     . $content . "</table>\n";
  }
}

foreach my $command (keys(%def_commands), 'defblock') {
  $default_commands_conversion{$command} = \&_convert_def_command;
}

sub _convert_table_definition_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  return $content if ($self->in_string());
  if ($content =~ /\S/) {
    return '<dd>' . $content . '</dd>'."\n";
  }
}

$default_types_conversion{'table_definition'}
                                  = \&_convert_table_definition_type;
$default_types_conversion{'inter_item'}
                                  = \&_convert_table_definition_type;

sub _contents_shortcontents_in_title($)
{
  my $self = shift;

  my $result = '';

  my $structuring = $self->get_info('structuring');
  if ($structuring and $structuring->{'sectioning_root'}
      and scalar(@{$structuring->{'sections_list'}}) > 1
      and $self->get_conf('CONTENTS_OUTPUT_LOCATION') eq 'after_title') {
    foreach my $cmdname ('shortcontents', 'contents') {
      if ($self->get_conf($cmdname)) {
        my $contents_text = $self->_contents_inline_element($cmdname, undef);
        if ($contents_text ne '') {
          $result .= $contents_text . $self->get_conf('DEFAULT_RULE')."\n";
        }
      }
    }
  }
  return $result;
}

# Convert @titlepage.  Falls back to simpletitle.
sub _default_format_titlepage($)
{
  my $self = shift;

  my $titlepage_text;
  my $global_commands = $self->get_info('global_commands');
  if ($global_commands->{'titlepage'}) {
    $titlepage_text = $self->convert_tree({'contents'
               => $global_commands->{'titlepage'}->{'contents'}},
                                          'convert titlepage');
  } else {
    my $simpletitle_tree = $self->get_info('simpletitle_tree');
    if ($simpletitle_tree) {
      my $simpletitle_command_name = $self->get_info('simpletitle_command_name');
      my $title_text = $self->convert_tree_new_formatting_context(
        $simpletitle_tree, "$simpletitle_command_name simpletitle");
      $titlepage_text = &{$self->formatting_function('format_heading_text')}($self,
                                  $simpletitle_command_name,
                          [$simpletitle_command_name], $title_text, 0);
    }
  }
  my $result = '';
  $result .= $titlepage_text.$self->get_conf('DEFAULT_RULE')."\n"
    if (defined($titlepage_text));
  $result .= $self->_contents_shortcontents_in_title();
  return $result;
}

sub _default_format_title_titlepage($)
{
  my $self = shift;

  my $result = '';
  if ($self->get_conf('SHOW_TITLE')) {
    if ($self->get_conf('USE_TITLEPAGE_FOR_TITLE')) {
      $result .= &{$self->formatting_function('format_titlepage')}($self);
    } else {
      my $simpletitle_tree = $self->get_info('simpletitle_tree');
      if ($simpletitle_tree) {
        my $simpletitle_command_name = $self->get_info('simpletitle_command_name');
        my $title_text = $self->convert_tree_new_formatting_context(
         $simpletitle_tree, "$simpletitle_command_name simpletitle");
        $result .= &{$self->formatting_function('format_heading_text')}($self,
                       $simpletitle_command_name,
                       [$simpletitle_command_name], $title_text, 0);
      }
      $result .= $self->_contents_shortcontents_in_title();
    }
  }
  return $result;
}

# Function for converting special elements
sub _convert_special_element_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  if ($self->in_string()) {
    return '';
  }

  my $result = '';

  my $special_element_variety = $element->{'extra'}->{'special_element_variety'};
  $result .= join('', $self->close_registered_sections_level(0));

  my $special_element_body
    .= &{$self->special_element_body_formatting($special_element_variety)}($self,
                                          $special_element_variety, $element);

  # This may happen with footnotes in regions that are not expanded,
  # like @copying or @titlepage
  if ($special_element_body eq '') {
    return '';
  }

  my $id = $self->command_id($element);
  my $class_base
    = $self->special_element_info('class', $special_element_variety);
  $result .= $self->html_attribute_class('div', ["element-${class_base}"]);
  if ($id ne '') {
    $result .= " id=\"$id\"";
  }
  $result .= ">\n";
  if ($self->get_conf('HEADERS')
      # first in page
      or $self->count_elements_in_filename('current',
                  $element->{'structure'}->{'unit_filename'}) == 1) {
    $result .= &{$self->formatting_function('format_navigation_header')}($self,
                             $self->get_conf('MISC_BUTTONS'), undef, $element);
  }
  my $heading = $self->command_text($element);
  my $level = $self->get_conf('CHAPTER_HEADER_LEVEL');
  if ($special_element_variety eq 'footnotes') {
    $level = $self->get_conf('FOOTNOTE_SEPARATE_HEADER_LEVEL');
  }
  $result .= &{$self->formatting_function('format_heading_text')}($self,
                           undef, [$class_base.'-heading'], $heading, $level)."\n";


  $result .= $special_element_body . '</div>';
  $result .= &{$self->formatting_function('format_element_footer')}($self, $type,
                                                             $element, $content);
  return $result;
}

$default_types_conversion{'special_element'} = \&_convert_special_element_type;

# Function for converting the top-level elements in the conversion corresponding to
# a section or a node.  The node and associated section appear together in
# the tree unit top-level element.  $ELEMENT was created in this module (in
# _prepare_conversion_tree_units), with type 'unit' (it's not a tree element created
# by the parser).  $CONTENT is the contents of the node/section, already converted.
sub _convert_tree_unit_type($$$$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;

  if ($self->in_string()) {
    if (defined($content)) {
      return $content;
    } else {
      return '';
    }
  }
  my $result = '';
  my $tree_unit = $element;
  if (not $tree_unit->{'structure'}
      or not $tree_unit->{'structure'}->{'unit_prev'}) {
    $result .= $self->get_info('title_titlepage');
    if (not $tree_unit->{'structure'}
        or not $tree_unit->{'structure'}->{'unit_next'}) {
      # only one unit, use simplified formatting
      $result .= $content;
      # if there is one unit it also means that there is no formatting
      # of footnotes in a separate unit.  And if footnotestyle is end
      # the footnotes won't be done in format_element_footer either.
      $result .= &{$self->formatting_function('format_footnotes_segment')}($self);
      $result .= $self->get_conf('DEFAULT_RULE') ."\n"
        if ($self->get_conf('PROGRAM_NAME_IN_FOOTER')
          and defined($self->get_conf('DEFAULT_RULE')));
      # do it here, as it is won't be done at end of page in format_element_footer
      $result .= join('', $self->close_registered_sections_level(0));
      return $result;
    }
  }
  $result .= $content;
  my $command;
  if ($element->{'extra'} and $element->{'extra'}->{'unit_command'}) {
    $command = $element->{'extra'}->{'unit_command'};
  }
  $result .= &{$self->formatting_function('format_element_footer')}($self, $type,
                                                              $element, $content, $command);

  return $result;
}

$default_types_conversion{'unit'} = \&_convert_tree_unit_type;

# for tree unit elements and special elements
sub _default_format_element_footer($$$$;$)
{
  my $self = shift;
  my $type = shift;
  my $element = shift;
  my $content = shift;
  my $command = shift;

  my $result = '';
  my $is_top = $self->element_is_tree_unit_top($element);
  my $next_is_top = ($element->{'structure'}->{'unit_next'}
                     and $self->element_is_tree_unit_top($element->{'structure'}->{'unit_next'}));
  my $next_is_special = (defined($element->{'structure'}->{'unit_next'})
                   and defined($element->{'structure'}->{'unit_next'}->{'type'})
                   and $element->{'structure'}->{'unit_next'}->{'type'} eq 'special_element');

  my $end_page = (!$element->{'structure'}->{'unit_next'}
       or (defined($element->{'structure'}->{'unit_filename'})
           and $element->{'structure'}->{'unit_filename'}
               ne $element->{'structure'}->{'unit_next'}->{'structure'}->{'unit_filename'}
           and $self->count_elements_in_filename('remaining',
                         $element->{'structure'}->{'unit_filename'}) == 1));

  my $is_special = (defined($element->{'type'})
                    and $element->{'type'} eq 'special_element');

  if (($end_page or $next_is_top or $next_is_special or $is_top)
       and $self->get_conf('VERTICAL_HEAD_NAVIGATION')
       and ($self->get_conf('SPLIT') ne 'node'
            or $self->get_conf('HEADERS') or $is_special or $is_top)) {
   $result .= "</td>
</tr>
</table>"."\n";
  }

  my $rule = '';
  my $buttons;

  if ($end_page) {
    $result .= join('', $self->close_registered_sections_level(0));

    # setup buttons for navigation footer
    if (($is_top or $is_special)
        and ($self->get_conf('SPLIT') or !$self->get_conf('MONOLITHIC'))
        and (($self->get_conf('HEADERS')
              or ($self->get_conf('SPLIT')
                  and $self->get_conf('SPLIT') ne 'node')))) {
      if ($is_top) {
        $buttons = $self->get_conf('TOP_BUTTONS');
      } else {
        $buttons = $self->get_conf('MISC_BUTTONS');
      }
    } elsif ($self->get_conf('SPLIT') eq 'section') {
      $buttons = $self->get_conf('SECTION_FOOTER_BUTTONS');
    } elsif ($self->get_conf('SPLIT') eq 'chapter') {
      $buttons = $self->get_conf('CHAPTER_FOOTER_BUTTONS');
    } elsif ($self->get_conf('SPLIT') eq 'node') {
      if ($self->get_conf('HEADERS')) {
        my $no_footer_word_count;
        if ($self->get_conf('WORDS_IN_PAGE')) {
          # FIXME it seems that NO-BREAK SPACE and NEXT LINE (NEL) may
          # not be in \h and \v in some case, but not sure which case it is
          my @cnt = split(/\P{Word}*[\h\v]+\P{Word}*/, $content);
          if (scalar(@cnt) < $self->get_conf('WORDS_IN_PAGE')) {
            $no_footer_word_count = 1;
          }
        }
        $buttons = $self->get_conf('NODE_FOOTER_BUTTONS')
           unless ($no_footer_word_count);
      }
    }
  }
  # FIXME the following condition is almost a duplication of the
  # condition appearing in end_page except that the file counter
  # needs not to be 1
  if ((!$element->{'structure'}->{'unit_next'}
       or (defined($element->{'structure'}->{'unit_filename'})
           and $element->{'structure'}->{'unit_filename'}
               ne $element->{'structure'}->{'unit_next'}->{'structure'}->{'unit_filename'}))
      and $self->get_conf('footnotestyle') eq 'end') {
    $result .= &{$self->formatting_function('format_footnotes_segment')}($self);
  }

  if (!$buttons or $is_top or $is_special
     or ($end_page and ($self->get_conf('SPLIT') eq 'chapter'
                       or $self->get_conf('SPLIT') eq 'section'))
     or ($self->get_conf('SPLIT') eq 'node' and $self->get_conf('HEADERS'))) {
    $rule = $self->get_conf('DEFAULT_RULE');
  }

  if (!$end_page and ($is_top or $next_is_top or ($next_is_special
                                                  and !$is_special))) {
    $rule = $self->get_conf('BIG_RULE');
  }

  if ($buttons or !$end_page or $self->get_conf('PROGRAM_NAME_IN_FOOTER')) {
    $result .= "$rule\n" if ($rule);
  }
  if ($buttons) {
    my $cmdname;
    $cmdname = $command->{'cmdname'} if ($command and $command->{'cmdname'});
    $result .= &{$self->formatting_function('format_navigation_panel')}($self,
                                                    $buttons, $cmdname, $command);
  }
  return $result;
}

# if $document_global_context is set, it means that the formatting
# is not done within the document formatting flow, but the formatted
# output may still end up in the document.  In particular for
# command_text() which caches its computations.
sub _new_document_context($;$$)
{
  my $self = shift;
  my $context = shift;
  my $document_global_context = shift;

  push @{$self->{'document_context'}},
          {'context' => $context,
           'formatting_context' => [{'context_name' => $context}],
           'composition_context' => [''],
           'formats' => [],
           'monospace' => [0],
           'document_global_context' => $document_global_context,
           'block_commands' => [],
          };
  if (defined($document_global_context)) {
    $self->{'document_global_context'}++;
  }
}

sub _pop_document_context($)
{
  my $self = shift;

  my $context = pop @{$self->{'document_context'}};
  if (defined($context->{'document_global_context'})) {
    $self->{'document_global_context'}--;
  }
}

# can be set through Texinfo::Config::texinfo_register_file_id_setting_function
my %customizable_file_id_setting_references;
foreach my $customized_reference ('label_target_name', 'node_file_name',
                'sectioning_command_target_name', 'tree_unit_file_name',
                'special_element_target_file_name') {
  $customizable_file_id_setting_references{$customized_reference} = 1;
}

# Functions accessed with e.g. 'format_heading_text'.
# used in Texinfo::Config
%default_formatting_references = (
     'format_begin_file' => \&_default_format_begin_file,
     'format_button' => \&_default_format_button,
     'format_button_icon_img' => \&_default_format_button_icon_img,
     'format_css_lines' => \&_default_format_css_lines,
     'format_comment' => \&_default_format_comment,
     'format_contents' => \&_default_format_contents,
     'format_element_header' => \&_default_format_element_header,
     'format_element_footer' => \&_default_format_element_footer,
     'format_end_file' => \&_default_format_end_file,
     'format_frame_files' => \&_default_format_frame_files,
     'format_footnotes_segment' => \&_default_format_footnotes_segment,
     'format_footnotes_sequence' => \&_default_format_footnotes_sequence,
     'format_heading_text' => \&_default_format_heading_text,
     'format_navigation_header' => \&_default_format_navigation_header,
     'format_navigation_panel' => \&_default_format_navigation_panel,
     'format_node_redirection_page' => \&_default_format_node_redirection_page,
     'format_program_string' => \&_default_format_program_string,
     'format_protect_text' => \&_default_format_protect_text,
     'format_separate_anchor' => \&_default_format_separate_anchor,
     'format_titlepage' => \&_default_format_titlepage,
     'format_title_titlepage' => \&_default_format_title_titlepage,
     'format_translate_string' => undef,
);

# not up for customization
%default_css_string_formatting_references = (
  'format_protect_text' => \&_default_css_string_format_protect_text,
);

%defaults_format_special_body_contents = (
  'contents' => \&_default_format_special_body_contents,
  'about' => \&_default_format_special_body_about,
  'footnotes' => \&_default_format_special_body_footnotes,
  'shortcontents' => \&_default_format_special_body_shortcontents,
);

sub _reset_unset_no_arg_commands_formatting_context($$$$;$)
{
  my $self = shift;
  my $cmdname = shift;
  my $reset_context = shift;
  my $ref_context = shift;
  my $translate = shift;

  # should never happen as unset is set at configuration
  if (!defined ($self->{'no_arg_commands_formatting'}->{$reset_context}->{$cmdname})) {
    $self->{'no_arg_commands_formatting'}->{$reset_context}->{$cmdname}->{'unset'} = 1;
  }
  my $no_arg_command_context
     = $self->{'no_arg_commands_formatting'}->{$reset_context}->{$cmdname};
  if (defined($ref_context)) {
    if ($no_arg_command_context->{'unset'}) {
      foreach my $key (keys(%{$self->{'no_arg_commands_formatting'}->{$ref_context}->{$cmdname}})) {
        # both 'translated_converted' and (possibly translated) 'text' are
        # reused
        $no_arg_command_context->{$key}
          = $self->{'no_arg_commands_formatting'}->{$ref_context}->{$cmdname}->{$key}
      }
    }
  }
  if ($translate
      and $no_arg_command_context->{'tree'}
      and not defined($no_arg_command_context->{'translated_converted'})) {
    my $translated_tree
      = $no_arg_command_context->{'tree'};
    my $translation_result;
    if ($reset_context eq 'normal') {
      $translation_result
        = $self->convert_tree($translated_tree, "no arg $cmdname translated");
    } elsif ($reset_context eq 'preformatted') {
      # there does not seems to be anything simpler...
      my $preformatted_command_name = 'example';
      $self->_new_document_context();
      push @{$self->{'document_context'}->[-1]->{'composition_context'}},
          $preformatted_command_name;
      # should not be needed for at commands no brace translation strings
      push @{$self->{'document_context'}->[-1]->{'preformatted_classes'}},
          $pre_class_commands{$preformatted_command_name};
      $translation_result
        = $self->convert_tree($translated_tree, "no arg $cmdname translated");
      # only pop the main context
      $self->_pop_document_context();
    } elsif ($reset_context eq 'string') {
      $translation_result = $self->convert_tree_new_formatting_context({'type' => '_string',
                                                           'contents' => [$translated_tree]},
                                     'translated_string', "string no arg $cmdname translated");
    } elsif ($reset_context eq 'css_string') {
      $translation_result = $self->html_convert_css_string($translated_tree);
    }
    $no_arg_command_context->{'text'}
      = $translation_result;
  }
}
sub _complete_no_arg_commands_formatting($$;$)
{
  my $self = shift;
  my $cmdname = shift;
  my $translate = shift;

  _reset_unset_no_arg_commands_formatting_context($self, $cmdname,
                                            'normal', undef, $translate);
  _reset_unset_no_arg_commands_formatting_context($self, $cmdname,
                                   'preformatted', 'normal', $translate);
  _reset_unset_no_arg_commands_formatting_context($self, $cmdname,
                                    'string', 'preformatted', $translate);
  _reset_unset_no_arg_commands_formatting_context($self, $cmdname,
                                   'css_string', 'string', $translate);
}

sub _set_non_breaking_space($$)
{
  my $self = shift;
  my $non_breaking_space = shift;
  $self->{'non_breaking_space'} = $non_breaking_space;
}

# transform <hr> to <hr/>
sub _xhtml_re_close_lone_element($)
{
  my $element = shift;
  $element =~ s/^(<[a-zA-Z]+[^>]*)>$/$1\/>/;
  return $element;
}

my %htmlxref_entries = (
 'node' => [ 'node', 'section', 'chapter', 'mono' ],
 'section' => [ 'section', 'chapter','node', 'mono' ],
 'chapter' => [ 'chapter', 'section', 'node', 'mono' ],
 'mono' => [ 'mono', 'chapter', 'section', 'node' ],
);

# $FILES is an array reference of file names binary strings.
sub _parse_htmlxref_files($$)
{
  my $self = shift;
  my $files = shift;
  my $htmlxref = {};

  foreach my $file (@$files) {
    my $fname = $file;
    if ($self->get_conf('TEST')) {
      my ($volume, $directories);
      # strip directories for out-of-source builds reproducible file names
      ($volume, $directories, $fname) = File::Spec->splitpath($file);
    }
    print STDERR "html refs config file: $file\n" if ($self->get_conf('DEBUG'));
    unless (open(HTMLXREF, $file)) {
      my $htmlxref_file_name = $file;
      my $encoding = $self->get_conf('COMMAND_LINE_ENCODING');
      if (defined($encoding)) {
        $htmlxref_file_name = decode($encoding, $htmlxref_file_name);
      }
      $self->document_warn($self,
        sprintf(__("could not open html refs config file %s: %s"),
          $htmlxref_file_name, $!));
      next;
    }
    my $line_nr = 0;
    my %variables;
    while (my $hline = <HTMLXREF>) {
      my $line = $hline;
      $line_nr++;
      next if $hline =~ /^\s*#/;
      #$hline =~ s/[#]\s.*//;
      $hline =~ s/^\s*//;
      next if $hline =~ /^\s*$/;
      chomp ($hline);
      if ($hline =~ s/^\s*(\w+)\s*=\s*//) {
        # handle variables
        my $var = $1;
        my $re = join '|', map { quotemeta $_ } keys %variables;
        $hline =~ s/\$\{($re)\}/defined $variables{$1} ? $variables{$1}
                                                       : "\${$1}"/ge;
        $variables{$var} = $hline;
        next;
      }
      my @htmlxref = split /\s+/, $hline;
      my $manual = shift @htmlxref;
      my $split_or_mono = shift @htmlxref;
      #print STDERR "$split_or_mono $Texi2HTML::Config::htmlxref_entries{$split_or_mono} $line_nr\n";
      if (!defined($split_or_mono)) {
        $self->line_warn($self, __("missing type"),
                 {'file_name' => $fname, 'line_nr' => $line_nr});
        next;
      } elsif (!defined($htmlxref_entries{$split_or_mono})) {
        $self->line_warn($self, sprintf(__("unrecognized type: %s"),
                                        $split_or_mono),
                    {'file_name' => $fname, 'line_nr' => $line_nr});
        next;
      }
      my $href = shift @htmlxref;
      next if ($htmlxref->{$manual}
               and exists($htmlxref->{$manual}->{$split_or_mono}));

      if (defined($href)) { # substitute 'variables'
        my $re = join '|', map { quotemeta $_ } keys %variables;
        $href =~ s/\$\{($re)\}/defined $variables{$1} ? $variables{$1}
                                                      : "\${$1}"/ge;
        $href =~ s/\/*$// if ($split_or_mono ne 'mono');
      }
      $htmlxref->{$manual} = {} if (!$htmlxref->{$manual});
      $htmlxref->{$manual}->{$split_or_mono} = $href;
    }
    if (!close (HTMLXREF)) {
      $self->document_warn($self, sprintf(__(
                       "error on closing html refs config file %s: %s"),
                             $file, $!));
    }
  }
  return $htmlxref;
}

sub _load_htmlxref_files {
  my ($self) = @_;

  my @htmlxref_files;
  my $htmlxref_mode = $self->get_conf('HTMLXREF_MODE');
  return if (defined($htmlxref_mode) and $htmlxref_mode eq 'none');
  my $htmlxref_file_name = 'htmlxref.cnf';
  if (defined($htmlxref_mode) and $htmlxref_mode eq 'file') {
    if (defined($self->get_conf('HTMLXREF_FILE'))) {
      $htmlxref_file_name = $self->get_conf('HTMLXREF_FILE');
    }
    my ($encoded_htmlxref_file_name, $htmlxref_file_encoding)
      = $self->encoded_output_file_name($htmlxref_file_name);
    if (-e $encoded_htmlxref_file_name and -r $encoded_htmlxref_file_name) {
      @htmlxref_files = ($encoded_htmlxref_file_name);
    } else {
      $self->document_warn($self,
        sprintf(__("could not find html refs config file %s"),
          $htmlxref_file_name));
    }
  } else {
    my @htmlxref_dirs = ();
    if ($self->get_conf('TEST')) {
      my $curdir = File::Spec->curdir();
      # to have reproducible tests, do not use system or user
      # directories if TEST is set.
      @htmlxref_dirs = File::Spec->catdir($curdir, '.texinfo');

      if (defined($self->{'parser_info'})
          and defined($self->{'parser_info'}->{'input_directory'})) {
        my $input_directory = $self->{'parser_info'}->{'input_directory'};
        if ($input_directory ne '.' and $input_directory ne '') {
          unshift @htmlxref_dirs, $input_directory;
        }
      }
    } elsif ($self->{'language_config_dirs'}
             and @{$self->{'language_config_dirs'}}) {
      @htmlxref_dirs = @{$self->{'language_config_dirs'}};
    }
    unshift @htmlxref_dirs, '.';

    # no htmlxref for tests, unless explicitly specified
    if ($self->get_conf('TEST')) {
      if (defined($self->get_conf('HTMLXREF_FILE'))) {
        $htmlxref_file_name = $self->get_conf('HTMLXREF_FILE');
      } else {
        $htmlxref_file_name = undef;
      }
    } elsif (defined($self->get_conf('HTMLXREF_FILE'))) {
      $htmlxref_file_name = $self->get_conf('HTMLXREF_FILE');
    }

    if (defined($htmlxref_file_name)) {
      my ($encoded_htmlxref_file_name, $htmlxref_file_encoding)
        = $self->encoded_output_file_name($htmlxref_file_name);
      @htmlxref_files
        = Texinfo::Common::locate_init_file($encoded_htmlxref_file_name,
                                          \@htmlxref_dirs, 1);
    }
  }

  $self->{'htmlxref'} = {};
  if (scalar(@htmlxref_files)) {
    $self->{'htmlxref'} = _parse_htmlxref_files($self,
                                                \@htmlxref_files);
  }
}

# converter state
#
#  output_init_conf
#
#     API exists
#  shared_conversion_state
#   Set through the shared_conversion_state API (among others):
#  explained_commands         # used only in an @-command conversion function
#  element_explanation_contents    # same as above
#
#     API exists
#  current_filename
#  document_name
#  destination_directory
#  paragraph_symbol
#  line_break_element
#  non_breaking_space
#  simpletitle_tree
#  simpletitle_command_name
#  title_string
#  title_tree
#  documentdescription_string
#  copying_comment
#  index_entries
#  index_entries_by_letter
#  jslicenses
#
#    API exists
#  css_element_class_styles
#  css_import_lines
#  css_rule_lines
#
#    API exists
#  file_id_setting
#  commands_conversion
#  commands_open
#  types_conversion
#  types_open
#  no_arg_commands_formatting
#  style_commands_formatting
#  code_types
#  pre_class_types
#
#    API exists
#  document_context
#
#    API exists
#  pending_closes
#
#    API exists
#  pending_footnotes
#
#    API exists
#  pending_inline_content
#  associated_inline_content
#
#    API exists
#  targets         for directions.  Keys are elements references, values are
#                  target information hash references described above before
#                  the API functions used to access this information.
#  special_targets
#  special_elements_targets
#  special_elements_directions
#  global_target_elements_directions
#
#    API exists
#  directions_strings
#  translated_direction_strings
#
#    API exists
#  special_element_info
#  translated_special_element_info
#
#    API exists
#  elements_in_file_count    # the number of tree unit elements in file
#  file_counters             # begin at elements_in_file_count decrease
#                            # each time the tree unit element is closed
#
#     API exists
#  document_global_context_css
#  file_css
#
#     API exists
#  files_information
#
#     No API, converter internals
#  tree_units
#  out_filepaths          (partially common with Texinfo::Converter)
#  current_root_element
#  seen_ids
#  ignore_notice
#  options_latex_math
#  htmlxref
#  check_htmlxref_already_warned
#  referred_command_stack
#
#    from Converter
#  labels

my %special_characters = (
  'paragraph_symbol' => ['&para;', '00B6'],
  'left_quote' => ['&lsquo;', '2018'],
  'right_quote' => ['&rsquo;', '2019'],
  'bullet' => ['&bull;', '2022'],
  'non_breaking_space' => [undef, '00A0'],
);

sub converter_initialize($)
{
  my $self = shift;

  %{$self->{'css_element_class_styles'}} = %css_element_class_styles;

  _load_htmlxref_files($self);

  # duplicate such as not to modify the defaults
  my $conf_default_no_arg_commands_formatting_normal
    = Storable::dclone($default_no_arg_commands_formatting{'normal'});

  my %special_characters_set;

  my $output_encoding = $self->get_conf('OUTPUT_ENCODING_NAME');

  foreach my $special_character (keys(%special_characters)) {
    my ($default_entity, $unicode_point) = @{$special_characters{$special_character}};
    if ($self->get_conf('OUTPUT_CHARACTERS')
        and Texinfo::Convert::Unicode::unicode_point_decoded_in_encoding(
                                         $output_encoding, $unicode_point)) {
      $special_characters_set{$special_character} = chr(hex($unicode_point));
    } elsif ($self->get_conf('USE_NUMERIC_ENTITY')) {
      $special_characters_set{$special_character} = '&#'.hex($unicode_point).';';
    } else {
      $special_characters_set{$special_character} = $default_entity;
    }
  }

  if (defined($special_characters_set{'non_breaking_space'})) {
    my $non_breaking_space = $special_characters_set{'non_breaking_space'};
    $self->_set_non_breaking_space($non_breaking_space);
    foreach my $space_command (' ', "\t", "\n") {
      $conf_default_no_arg_commands_formatting_normal->{$space_command}->{'text'}
        = $self->{'non_breaking_space'};
    }
    $conf_default_no_arg_commands_formatting_normal->{'tie'}->{'text'}
      = $self->substitute_html_non_breaking_space(
           $default_no_arg_commands_formatting{'normal'}->{'tie'}->{'text'});
  } else {
    $self->_set_non_breaking_space($xml_named_entity_nbsp);
  }
  $self->{'paragraph_symbol'} = $special_characters_set{'paragraph_symbol'};

  if (not defined($self->get_conf('OPEN_QUOTE_SYMBOL'))) {
    $self->set_conf('OPEN_QUOTE_SYMBOL', $special_characters_set{'left_quote'});
  }
  if (not defined($self->get_conf('CLOSE_QUOTE_SYMBOL'))) {
    $self->set_conf('CLOSE_QUOTE_SYMBOL', $special_characters_set{'right_quote'});
  }
  if (not defined($self->get_conf('MENU_SYMBOL'))) {
    $self->set_conf('MENU_SYMBOL', $special_characters_set{'bullet'});
  }

  if ($self->get_conf('USE_NUMERIC_ENTITY')) {
    foreach my $command (keys(%Texinfo::Convert::Unicode::unicode_entities)) {
      $conf_default_no_arg_commands_formatting_normal->{$command}->{'text'}
       = $Texinfo::Convert::Unicode::unicode_entities{$command};
    }
  }

  if ($self->get_conf('USE_XML_SYNTAX')) {
    foreach my $customization_variable ('BIG_RULE', 'DEFAULT_RULE') {
      my $variable_value = $self->get_conf($customization_variable);
      if (defined($variable_value)) {
        my $closed_lone_element = _xhtml_re_close_lone_element($variable_value);
        if ($closed_lone_element ne $variable_value) {
          $self->force_conf($customization_variable, $closed_lone_element);
        }
      }
    }
    $self->{'line_break_element'} = '<br/>';
  } else {
    $self->{'line_break_element'} = '<br>';
  }
  $conf_default_no_arg_commands_formatting_normal->{'*'}->{'text'}
    = $self->{'line_break_element'};

  # three types of direction strings:
  # * strings not translated, already converted
  # * strings translated
  #   - strings already converted
  #   - strings not already converted
  $self->{'directions_strings'} = {};

  my $customized_direction_strings
      = Texinfo::Config::GNUT_get_direction_string_info();
  foreach my $string_type (keys(%default_converted_directions_strings)) {
    $self->{'directions_strings'}->{$string_type} = {};
    foreach my $direction
            (keys(%{$default_converted_directions_strings{$string_type}})) {
      $self->{'directions_strings'}->{$string_type}->{$direction} = {};
      my $string_contexts;
      if ($customized_direction_strings->{$string_type}
          and $customized_direction_strings->{$string_type}->{$direction}) {
        if (defined($customized_direction_strings->{$string_type}
                                              ->{$direction}->{'converted'})) {
          $string_contexts
            = $customized_direction_strings->{$string_type}
                                          ->{$direction}->{'converted'};
        }
      } else {
        my $string
          = $default_converted_directions_strings{$string_type}->{$direction};
        $string_contexts
          = {'normal' => $string};
      }
      $string_contexts->{'string'} = $string_contexts->{'normal'}
        if (not defined($string_contexts->{'string'}));
      foreach my $context (keys(%$string_contexts)) {
        $self->{'directions_strings'}->{$string_type}->{$direction}->{$context}
          = $self->substitute_html_non_breaking_space(
                                                  $string_contexts->{$context});
      }
    }
  }
  $self->{'translated_direction_strings'} = {};
  foreach my $string_type (keys(%default_translated_directions_strings)) {
    $self->{'translated_direction_strings'}->{$string_type} = {};
    foreach my $direction
           (keys(%{$default_translated_directions_strings{$string_type}})) {
      if ($customized_direction_strings->{$string_type}
            and $customized_direction_strings->{$string_type}->{$direction}) {
        $self->{'translated_direction_strings'}->{$string_type}->{$direction}
          = $customized_direction_strings->{$string_type}->{$direction};
      } else {
        if ($default_translated_directions_strings{$string_type}->{$direction}
                                                              ->{'converted'}) {
          $self->{'translated_direction_strings'}->{$string_type}
                  ->{$direction} = {'converted' => {}};
          foreach my $context ('normal', 'string') {
            $self->{'translated_direction_strings'}->{$string_type}
                     ->{$direction}->{'converted'}->{$context}
               = $default_translated_directions_strings{$string_type}
                                                 ->{$direction}->{'converted'};
          }
        } else {
          $self->{'translated_direction_strings'}->{$string_type}->{$direction}
            = $default_translated_directions_strings{$string_type}->{$direction};
        }
      }
    }
  }

  $self->{'types_conversion'} = {};
  my $customized_types_conversion = Texinfo::Config::GNUT_get_types_conversion();
  foreach my $type (keys(%default_types_conversion)) {
    if (exists($customized_types_conversion->{$type})) {
      $self->{'types_conversion'}->{$type}
          = $customized_types_conversion->{$type};
    } else {
      $self->{'types_conversion'}->{$type}
          = $default_types_conversion{$type};
    }
  }

  $self->{'types_open'} = {};
  my $customized_types_open
     = Texinfo::Config::GNUT_get_types_open();
  foreach my $type (keys(%default_types_conversion)) {
    if (exists($customized_types_open->{$type})) {
      $self->{'types_open'}->{$type}
          = $customized_types_open->{$type};
    } elsif (exists($default_types_open{$type})) {
      $self->{'types_open'}->{$type}
           = $default_types_open{$type};
    }
  }

  $self->{'code_types'} = {};
  foreach my $type (keys(%default_code_types)) {
    $self->{'code_types'}->{$type} = $default_code_types{$type};
  }
  $self->{'pre_class_types'} = {};
  foreach my $type (keys(%default_pre_class_types)) {
    $self->{'pre_class_types'}->{$type} = $default_pre_class_types{$type};
  }
  my $customized_type_formatting
    = Texinfo::Config::GNUT_get_types_formatting_info();
  foreach my $type (keys(%$customized_type_formatting)) {
    # Used in cvs.init.
    $self->{'code_types'}->{$type}
     = $customized_type_formatting->{$type}->{'code'};
    $self->{'pre_class_types'}->{$type}
     = $customized_type_formatting->{$type}->{'pre_class'};
  }

  $self->{'commands_conversion'} = {};
  # FIXME put value in a category in Texinfo::Common?
  my $customized_commands_conversion
     = Texinfo::Config::GNUT_get_commands_conversion();
  foreach my $command (keys(%line_commands), keys(%brace_commands),
     keys (%block_commands), keys(%nobrace_commands)) {
    if (exists($customized_commands_conversion->{$command})) {
      $self->{'commands_conversion'}->{$command}
          = $customized_commands_conversion->{$command};
    } else {
      if ($self->get_conf('FORMAT_MENU') ne 'menu'
           and ($command eq 'menu' or $command eq 'detailmenu')) {
        $self->{'commands_conversion'}->{$command} = undef;
      } elsif ($format_raw_commands{$command}
               and !$self->{'expanded_formats_hash'}->{$command}) {
      } elsif (exists($default_commands_conversion{$command})) {
        $self->{'commands_conversion'}->{$command}
           = $default_commands_conversion{$command};
      }
    }
  }

  $self->{'commands_open'} = {};
  my $customized_commands_open
     = Texinfo::Config::GNUT_get_commands_open();
  foreach my $command (keys(%line_commands), keys(%brace_commands),
     keys (%block_commands), keys(%nobrace_commands)) {
    if (exists($customized_commands_open->{$command})) {
      $self->{'commands_open'}->{$command}
          = $customized_commands_open->{$command};
    } elsif (exists($default_commands_open{$command})) {
      $self->{'commands_open'}->{$command}
           = $default_commands_open{$command};
    }
  }

  $self->{'no_arg_commands_formatting'} = {};
  foreach my $context ('normal', 'preformatted', 'string', 'css_string') {
    $self->{'no_arg_commands_formatting'}->{$context} = {};
    foreach my $command (keys(%{$default_no_arg_commands_formatting{'normal'}})) {
      my $no_arg_command_customized_formatting
        = Texinfo::Config::GNUT_get_no_arg_command_formatting($command, $context);
      if (defined($no_arg_command_customized_formatting)) {
        $self->{'no_arg_commands_formatting'}->{$context}->{$command}
           = $no_arg_command_customized_formatting;
      } else {
        my $context_default_default_no_arg_commands_formatting
          = $default_no_arg_commands_formatting{$context};
        if ($context eq 'normal') {
          $context_default_default_no_arg_commands_formatting
           = $conf_default_no_arg_commands_formatting_normal;
        }
        if (defined($context_default_default_no_arg_commands_formatting->{$command})) {
          if ($self->get_conf('OUTPUT_CHARACTERS')
              and Texinfo::Convert::Unicode::brace_no_arg_command(
                             $command, $self->get_conf('OUTPUT_ENCODING_NAME'))) {
            $self->{'no_arg_commands_formatting'}->{$context}->{$command}
              = { 'text' => Texinfo::Convert::Unicode::brace_no_arg_command(
                           $command, $self->get_conf('OUTPUT_ENCODING_NAME'))};
            # reset CSS for itemize command arguments
            if ($context eq 'css_string'
                and exists($brace_commands{$command})
                and $command ne 'bullet' and $command ne 'w'
                and not $special_list_mark_css_string_no_arg_command{$command}) {
              my $css_string
                = $self->{'no_arg_commands_formatting'}
                                    ->{$context}->{$command}->{'text'};
              $css_string = '"'.$css_string.'"';
              $self->{'css_element_class_styles'}->{"ul.mark-$command"}
                = "list-style-type: $css_string";
            }
          } else {
            $self->{'no_arg_commands_formatting'}->{$context}->{$command}
              = $context_default_default_no_arg_commands_formatting->{$command};
          }
        } else {
          $self->{'no_arg_commands_formatting'}->{$context}->{$command}
            = {'unset' => 1};
        }
      }
    }
  }

  # set sane defaults in case there is none and the default formatting
  # function is used
  foreach my $command (keys(%{$default_no_arg_commands_formatting{'normal'}})) {
    if ($self->{'commands_conversion'}->{$command}
        and $self->{'commands_conversion'}->{$command}
            eq $default_commands_conversion{$command}) {
      $self->_complete_no_arg_commands_formatting($command);
    }
  }

  $self->{'style_commands_formatting'} = {};
  foreach my $context (keys(%style_commands_formatting)) {
    $self->{'style_commands_formatting'}->{$context} = {};
    foreach my $command (keys(%{$style_commands_formatting{$context}})) {
      my $style_commands_formatting_info
        = Texinfo::Config::GNUT_get_style_command_formatting($command, $context);
      if (defined($style_commands_formatting_info)) {
        $self->{'style_commands_formatting'}->{$context}->{$command}
           = $style_commands_formatting_info;
      } elsif (exists($style_commands_formatting{$context}->{$command})) {
        $self->{'style_commands_formatting'}->{$context}->{$command}
           = $style_commands_formatting{$context}->{$command};
      }
    }
  }

  $self->{'accent_entities'} = {};
  foreach my $accent_command
     (keys(%Texinfo::Convert::Converter::xml_accent_entities)) {
    $self->{'accent_entities'}->{$accent_command} = [];
    my ($accent_command_entity, $accent_command_text_with_entities)
      = Texinfo::Config::GNUT_get_accent_command_formatting($accent_command);
    if (not defined($accent_command_entity)
        and defined($Texinfo::Convert::Converter::xml_accent_text_with_entities{
                                                              $accent_command})) {
      $accent_command_entity
       = $Texinfo::Convert::Converter::xml_accent_entities{$accent_command};
    }
    if (not defined($accent_command_text_with_entities)
        and defined($Texinfo::Convert::Converter::xml_accent_text_with_entities{
                                                             $accent_command})) {
      $accent_command_text_with_entities
  = $Texinfo::Convert::Converter::xml_accent_text_with_entities{$accent_command};
    }
    # an empty string means no formatting
    if (defined($accent_command_entity)) {
      $self->{'accent_entities'}->{$accent_command} = [$accent_command_entity,
                                           $accent_command_text_with_entities];
    }
  }
  #print STDERR Data::Dumper->Dump([$self->{'accent_entities'}]);

  $self->{'file_id_setting'} = {};
  my $customized_file_id_setting_references
    = Texinfo::Config::GNUT_get_file_id_setting_references();
  # first check the validity of the names
  foreach my $customized_file_id_setting_ref
       (sort(keys(%{$customized_file_id_setting_references}))) {
    if (!$customizable_file_id_setting_references{$customized_file_id_setting_ref}) {
      $self->document_warn($self,
                           sprintf(__("Unknown file and id setting function: %s"),
                                   $customized_file_id_setting_ref));
    } else {
      $self->{'file_id_setting'}->{$customized_file_id_setting_ref}
        = $customized_file_id_setting_references->{$customized_file_id_setting_ref};
    }
  }

  my $customized_formatting_references
       = Texinfo::Config::GNUT_get_formatting_references();
  # first check that all the customized_formatting_references
  # are in default_formatting_references
  foreach my $customized_formatting_reference
       (sort(keys(%{$customized_formatting_references}))) {
    if (!exists($default_formatting_references{$customized_formatting_reference})) {
      $self->document_warn($self, sprintf(__("Unknown formatting function: %s"),
                                          $customized_formatting_reference));
    }
  }

  $self->{'formatting_function'} = {};
  foreach my $formatting_reference (keys(%default_formatting_references)) {
    if (defined($customized_formatting_references->{$formatting_reference})) {
      $self->{'formatting_function'}->{$formatting_reference}
       = $customized_formatting_references->{$formatting_reference};
    } else {
      $self->{'formatting_function'}->{$formatting_reference}
       = $default_formatting_references{$formatting_reference};
    }
  }

  my $customized_special_element_info
    = Texinfo::Config::GNUT_get_special_element_info();

  $self->{'special_element_info'} = {};
  foreach my $type (keys(%default_special_element_info)) {
    $self->{'special_element_info'}->{$type} = {};
    foreach my $special_element_variety
                      (keys(%{$default_special_element_info{$type}})) {
      if (exists($customized_special_element_info->{$type})
          and exists($customized_special_element_info
                          ->{$type}->{$special_element_variety})) {
        $self->{'special_element_info'}->{$type}->{$special_element_variety}
         = $customized_special_element_info->{$type}->{$special_element_variety};
      } else {
        $self->{'special_element_info'}->{$type}->{$special_element_variety}
          = $default_special_element_info{$type}->{$special_element_variety};
      }
    }
  }

  $self->{'translated_special_element_info'} = {};
  foreach my $type (keys(%default_translated_special_element_info)) {
    $self->{'special_element_info'}->{$type} = {};
    $self->{'special_element_info'}->{$type.'_tree'} = {};
    $self->{'translated_special_element_info'}->{$type.'_tree'} = [$type, {}];
    foreach my $special_element_variety
                 (keys(%{$default_translated_special_element_info{$type}})) {
      if (exists($customized_special_element_info->{$type})
          and exists($customized_special_element_info
                          ->{$type}->{$special_element_variety})) {
        $self->{'translated_special_element_info'}->{$type.'_tree'}
                                               ->[1]->{$special_element_variety}
         = $customized_special_element_info->{$type}->{$special_element_variety};
      } else {
        $self->{'translated_special_element_info'}->{$type.'_tree'}
                                               ->[1]->{$special_element_variety}
          = $default_translated_special_element_info{$type}
                                                   ->{$special_element_variety};
      }
    }
  }

  my $customized_special_element_body
     = Texinfo::Config::GNUT_get_formatting_special_element_body_references();

  $self->{'special_element_body'} = {};
  foreach my $special_element_variety (keys(%defaults_format_special_body_contents)) {
    $self->{'special_element_body'}->{$special_element_variety}
      = $defaults_format_special_body_contents{$special_element_variety};
  }
  foreach my $special_element_variety (keys(%$customized_special_element_body)) {
    $self->{'special_element_body'}->{$special_element_variety}
      = $customized_special_element_body->{$special_element_variety};
  }

  $self->{'document_context'} = [];
  $self->{'multiple_pass'} = [];
  $self->{'pending_closes'} = [];
  $self->_new_document_context('_toplevel_context');

  if ($self->get_conf('SPLIT') and $self->get_conf('SPLIT') ne 'chapter'
      and $self->get_conf('SPLIT') ne 'section'
      and $self->get_conf('SPLIT') ne 'node') {
    $self->force_conf('SPLIT', 'node');
  }

  return $self;
}

# the entry point for _convert
sub convert_tree($$;$)
{
  my $self = shift;
  my $tree = shift;
  my $explanation = shift;

  # when formatting accents, goes through xml_accent without
  # explanation, as explanation is not in the standard API, but
  # otherwise the coverage of explanations should be pretty good
  #cluck if (! defined($explanation));
  #print STDERR "CONVERT_TREE".(defined($explanation) ? " ".$explanation : '')."\n"
  #    if ($self->get_conf('DEBUG'));
  return $self->_convert($tree, $explanation);
}

# FIXME document as part of the API.  Make it a mandatory called function?
# a format_* function?
# protect an url, in which characters with specific meaning in url are considered
# to have their specific meaning
sub url_protect_url_text($$)
{
  my $self = shift;
  my $input_string = shift;
  # percent encode character string.  It is better use UTF-8 irrespective
  # of the actual charset of the HTML output file, according to the tests done.
  my $href = encode("UTF-8", $input_string);
  # protect 'ligntly', do not protect unreserved and reserved characters + the % itself
  $href =~ s/([^^A-Za-z0-9\-_.!~*'()\$&+,\/:;=\?@\[\]\#%])/ sprintf "%%%02x", ord $1 /eg;
  return &{$self->formatting_function('format_protect_text')}($self, $href);
}

# FIXME document as part of the API.  Make it a mandatory called function?
# a format_* function?
# protect a file path used in an url.  Characters appearing in file paths
# are not protected.   All the other characters that can be percent
# protected are protected, including characters with specific meaning in url.
sub url_protect_file_text($$)
{
  my $self = shift;
  my $input_string = shift;
  # percent encode character string.  It is better use UTF-8 irrespective
  # of the actual charset of the HTML output file, according to the tests done.
  my $href = encode("UTF-8", $input_string);
  # protect everything that can be special in url except ~, / and : that could
  # appear in file names and does not have much risk in being incorrectly
  # interpreted (for :, the interpretation as a scheme delimiter may be possible).
  $href =~ s/([^^A-Za-z0-9\-_.~\/:])/ sprintf "%%%02x", ord $1 /eg;
  return &{$self->formatting_function('format_protect_text')}($self, $href);
}

sub _normalized_to_id($)
{
  my $id = shift;
  if (!defined($id)) {
    cluck "_normalized_to_id id not defined";
    return '';
  }
  $id =~ s/^([0-9_])/g_t$1/;
  return $id;
}

sub _default_format_css_lines($;$)
{
  my $self = shift;
  my $filename = shift;

  return '' if ($self->get_conf('NO_CSS'));

  my $css_refs = $self->get_conf('CSS_REFS');
  my @css_element_classes = $self->html_get_css_elements_classes($filename);
  my @css_import_lines = $self->css_get_info('imports');
  my @css_rule_lines = $self->css_get_info('rules');

  return '' if !@css_import_lines and !@css_element_classes
                 and !@css_rule_lines
                 and (!defined($css_refs) or !@$css_refs);

  my $css_text = "<style type=\"text/css\">\n<!--\n";
  $css_text .= join('', @css_import_lines) . "\n"
    if (@css_import_lines);
  foreach my $element_class (@css_element_classes) {
    my $css_style = $self->css_get_info('style', $element_class);
    $css_text .= "$element_class {$css_style}\n"
      if defined($css_style );
  }
  $css_text .= join('', @css_rule_lines) . "\n"
    if (@css_rule_lines);
  $css_text .= "-->\n</style>\n";
  foreach my $ref (@$css_refs) {
    $css_text .= $self->close_html_lone_element(
         '<link rel="stylesheet" type="text/css" href="'.
                $self->url_protect_url_text($ref).'"')."\n";
  }
  return $css_text;
}

sub _process_css_file($$$)
{
  my $self = shift;
  my $fh =shift;
  my $file = shift;

  my $in_rules = 0;
  my $in_comment = 0;
  my $in_import = 0;
  my $in_string = 0;
  my $rules = [];
  my $imports = [];
  my $line_nr = 0;
  while (my $line = <$fh>) {
    $line_nr++;
    if ($line_nr == 1) {
      # the rule is to assume utf-8.  There could also be a BOM, and
      # the Content-Type: HTTP header but it is not relevant here.
      # https://developer.mozilla.org/en-US/docs/Web/CSS/@charset
      my $charset = 'utf-8';
      my $charset_line;
      # should always be the first line
      if ($line =~ /^\@charset  *"([^"]+)" *; *$/) {
        $charset = $1;
        $charset_line = 1;
      }
      my $Encode_encoding_object = find_encoding($charset);
      if (defined($Encode_encoding_object)) {
        my $input_perl_encoding = $Encode_encoding_object->name();
        if ($input_perl_encoding eq 'utf-8') {
          binmode($fh, ":utf8");
        } else {
          binmode($fh, ":encoding($input_perl_encoding)");
        }
      }
      next if ($charset_line);
    }
    #print STDERR "Line: $line";
    if ($in_rules) {
      push @$rules, $line;
      next;
    }
    my $text = '';
    while (1) {
      #sleep 1;
      #print STDERR "${text}!in_comment $in_comment in_rules $in_rules in_import $in_import in_string $in_string: $line";
      if ($in_comment) {
        if ($line =~ s/^(.*?\*\/)//) {
          $text .= $1;
          $in_comment = 0;
        } else {
          push @$imports, $text . $line;
          last;
        }
      } elsif (!$in_string and $line =~ s/^\///) {
        if ($line =~ s/^\*//) {
          $text .= '/*';
          $in_comment = 1;
        } else {
          push (@$imports, $text. "\n") if ($text ne '');
          push (@$rules, '/' . $line);
          $in_rules = 1;
          last;
        }
      } elsif (!$in_string and $in_import and $line =~ s/^([\"\'])//) {
        # strings outside of import start rules
        $text .= "$1";
        $in_string = quotemeta("$1");
      } elsif ($in_string and $line =~ s/^(\\$in_string)//) {
        $text .= $1;
      } elsif ($in_string and $line =~ s/^($in_string)//) {
        $text .= $1;
        $in_string = 0;
      } elsif ((! $in_string and !$in_import)
              and ($line =~ s/^([\\]?\@import)$//
                   or $line =~ s/^([\\]?\@import\s+)//)) {
        $text .= $1;
        $in_import = 1;
      } elsif (!$in_string and $in_import and $line =~ s/^\;//) {
        $text .= ';';
        $in_import = 0;
      } elsif (($in_import or $in_string) and $line =~ s/^(.)//) {
        $text .= $1;
      } elsif (!$in_import and $line =~ s/^([^\s])//) {
        push (@$imports, $text. "\n") if ($text ne '');
        push (@$rules, $1 . $line);
        $in_rules = 1;
        last;
      } elsif ($line =~ s/^(\s)//) {
        $text .= $1;
      } elsif ($line eq '') {
        push (@$imports, $text);
        last;
      }
    }
  }
  $self->line_warn($self, __("string not closed in css file"),
                 {'file_name' => $file, 'line_nr' => $line_nr}) if ($in_string);
  $self->line_warn($self, __("--css-include ended in comment"),
                 {'file_name' => $file, 'line_nr' => $line_nr}) if ($in_comment);
  $self->line_warn($self, __("\@import not finished in css file"),
                 {'file_name' => $file, 'line_nr' => $line_nr})
    if ($in_import and !$in_comment and !$in_string);
  return ($imports, $rules);
}

sub _prepare_css($)
{
  my $self = shift;

  return if ($self->get_conf('NO_CSS'));

  my @css_import_lines;
  my @css_rule_lines;

  my $css_files = $self->get_conf('CSS_FILES');
  foreach my $file (@$css_files) {
    my $css_file_fh;
    my $css_file;
    if ($file eq '-') {
      $css_file_fh = \*STDIN;
      $css_file = '-';
    } else {
      $css_file = $self->Texinfo::Common::locate_include_file($file);
      unless (defined($css_file)) {
        my $input_file_name = $file;
        my $encoding = $self->get_conf('COMMAND_LINE_ENCODING');
        if (defined($encoding)) {
          $input_file_name = decode($encoding, $input_file_name);
        }
        $self->document_warn($self, sprintf(
               __("CSS file %s not found"), $input_file_name));
        next;
      }
      unless (open (CSSFILE, $css_file)) {
        my $css_file_name = $css_file;
        my $encoding = $self->get_conf('COMMAND_LINE_ENCODING');
        if (defined($encoding)) {
          $css_file_name = decode($encoding, $css_file_name);
        }
        $self->document_warn($self, sprintf(__(
             "could not open --include-file %s: %s"),
              $css_file_name, $!));
        next;
      }
      $css_file_fh = \*CSSFILE;
    }
    my ($import_lines, $rules_lines);
    ($import_lines, $rules_lines)
      = $self->_process_css_file($css_file_fh, $css_file);
    if (!close($css_file_fh)) {
      my $css_file_name = $css_file;
      my $encoding = $self->get_conf('COMMAND_LINE_ENCODING');
      if (defined($encoding)) {
        $css_file_name = decode($encoding, $css_file_name);
      }
      $self->document_warn($self,
            sprintf(__("error on closing CSS file %s: %s"),
                                   $css_file_name, $!));
    }
    push @css_import_lines, @$import_lines;
    push @css_rule_lines, @$rules_lines;

  }
  if ($self->get_conf('DEBUG')) {
    if (@css_import_lines) {
      print STDERR "# css import lines\n";
      foreach my $line (@css_import_lines) {
        print STDERR "$line";
      }
    }
    if (@css_rule_lines) {
      print STDERR "# css rule lines\n";
      foreach my $line (@css_rule_lines) {
        print STDERR "$line";
      }
    }
  }
  $self->{'css_import_lines'} = \@css_import_lines;
  $self->{'css_rule_lines'} = \@css_rule_lines;
}

# Get the name of a file containing a label, as well as the identifier within
# that file to link to that label.  $normalized is the normalized label name
# and $node_contents is the label contents.  Labels are typically associated
# to @node, @anchor or @float and to external nodes.
sub _normalized_label_id_file($$$)
{
  my $self = shift;
  my $normalized = shift;
  my $node_contents = shift;

  my $target;
  if (!defined($normalized)) {
    $normalized = Texinfo::Convert::NodeNameNormalization::normalize_node(
      { 'contents' => $node_contents });
  }

  if (defined($normalized)) {
    $target = _normalized_to_id($normalized);
  } else {
    $target = '';
  }
  # to find out the Top node, one could check $normalized
  if (defined($self->{'file_id_setting'}->{'label_target_name'})) {
    $target = &{$self->{'file_id_setting'}->{'label_target_name'}}($self,
                             $normalized, $node_contents, $target);
  }

  my $filename = $self->node_information_filename($normalized,
                                                  $node_contents);

  return ($filename, $target);
}

sub _new_sectioning_command_target($$)
{
  my $self = shift;
  my $command = shift;

  my ($normalized_name, $filename)
    = $self->normalized_sectioning_command_filename($command);

  my $target_base = _normalized_to_id($normalized_name);
  if ($target_base !~ /\S/ and $command->{'cmdname'} eq 'top') {
    # @top is allowed to be empty.  In that case it gets this target name
    $target_base = 'SEC_Top';
  }
  my $nr=1;
  my $target = $target_base;
  if ($target ne '') {
    while ($self->{'seen_ids'}->{$target}) {
      $target = $target_base.'-'.$nr;
      $nr++;
      # Avoid integer overflow
      die if ($nr == 0);
    }
  }

  # These are undefined if the $target is set to ''.
  my $target_contents;
  my $target_shortcontents;
  if ($sectioning_heading_commands{$command->{'cmdname'}}) {
    if ($target ne '') {
      my $target_base_contents = $target;
      $target_base_contents =~ s/^g_t//;
      $target_contents = 'toc-'.$target_base_contents;
      my $toc_nr = $nr -1;
      while ($self->{'seen_ids'}->{$target_contents}) {
        $target_contents = 'toc-'.$target_base_contents.'-'.$toc_nr;
        $toc_nr++;
        # Avoid integer overflow
        die if ($toc_nr == 0);
      }

      $target_shortcontents = 'stoc-'.$target_base_contents;
      my $target_base_shortcontents = $target_base;
      $target_base_shortcontents =~ s/^g_t//;
      my $stoc_nr = $nr -1;
      while ($self->{'seen_ids'}->{$target_shortcontents}) {
        $target_shortcontents = 'stoc-'.$target_base_shortcontents
                                   .'-'.$stoc_nr;
        $stoc_nr++;
        # Avoid integer overflow
        die if ($stoc_nr == 0);
      }
    }
  }

  if (defined($self->{'file_id_setting'}->{'sectioning_command_target_name'})) {
    ($target, $target_contents,
     $target_shortcontents, $filename)
      = &{$self->{'file_id_setting'}->{'sectioning_command_target_name'}}($self,
                                     $command, $target,
                                     $target_contents,
                                     $target_shortcontents,
                                     $filename);
  }
  if ($self->get_conf('DEBUG')) {
    print STDERR "Register $command->{'cmdname'} $target\n";
  }
  $self->{'targets'}->{$command} = {
                           'target' => $target,
                           'section_filename' => $filename,
                          };
  $self->{'seen_ids'}->{$target} = 1;
  if (defined($target_contents)) {
    $self->{'targets'}->{$command}->{'contents_target'} = $target_contents;
  } else {
    $self->{'targets'}->{$command}->{'contents_target'} = '';
  }
  if (defined($target_shortcontents)) {
    $self->{'targets'}->{$command}->{'shortcontents_target'}
       = $target_shortcontents;
  } else {
    $self->{'targets'}->{$command}->{'shortcontents_target'} = '';
  }
  return $self->{'targets'}->{$command};
}

# This set with two different codes
#  * the target information, id and normalized filename of 'labels',
#    ie everything that may be the target of a ref, @node, @float label,
#    @anchor.
#  * The target information of sectioning elements by going through tree units
# @node and section commands targets are therefore both set.
#
# conversion to HTML is done on-demand, upon call to command_text
# and similar functions.
# Note that 'node_filename', which is set here for Top target information
# too, is not used later for Top anchors or links, see the NOTE below
# associated with setting TOP_NODE_FILE_TARGET.
sub _set_root_commands_targets_node_files($$)
{
  my $self = shift;
  my $tree_units = shift;

  my $no_unidecode;
  $no_unidecode = 1 if (defined($self->get_conf('USE_UNIDECODE'))
                        and !$self->get_conf('USE_UNIDECODE'));

  my $extension = '';
  $extension = '.'.$self->get_conf('EXTENSION')
            if (defined($self->get_conf('EXTENSION'))
                and $self->get_conf('EXTENSION') ne '');
  if ($self->{'labels'}) {
    foreach my $label (sort(keys(%{$self->{'labels'}}))) {
      my $target_element = $self->{'labels'}->{$label};
      my $label_element = Texinfo::Common::get_label_element($target_element);
      my ($node_filename, $target)
        = $self->_normalized_label_id_file($target_element->{'extra'}->{'normalized'},
                                           $label_element->{'contents'});
      $node_filename .= $extension;
      if (defined($self->{'file_id_setting'}->{'node_file_name'})) {
        # a non defined filename is ok if called with convert, but not
        # if output in files.  We reset if undef, silently unless verbose
        # in case called by convert.
        my $user_node_filename
              = &{$self->{'file_id_setting'}->{'node_file_name'}}(
                                       $self, $target_element, $node_filename);
        if (defined($user_node_filename)) {
          $node_filename = $user_node_filename;
        } elsif ($self->get_conf('VERBOSE')) {
          $self->document_warn($self, sprintf(__(
              "user-defined node file name not set for `%s'"),
              $node_filename));

        } elsif ($self->get_conf('DEBUG')) {
          warn "user-defined node file name undef for `$node_filename'\n";
        }
      }
      if ($self->get_conf('DEBUG')) {
        print STDERR 'Label'
         # uncomment to get the perl object names
         #."($target_element)"
          ." \@$target_element->{'cmdname'} $target, $node_filename\n";
      }
      $self->{'targets'}->{$target_element} = {'target' => $target,
                                           'node_filename' => $node_filename};
      $self->{'seen_ids'}->{$target} = 1;
    }
  }

  if ($tree_units) {
    foreach my $tree_unit (@$tree_units) {
      foreach my $root_element(@{$tree_unit->{'contents'}}) {
        # this happens for types which would precede the root commands.
        # The target may already be set for the top node tree unit.
        next if (!defined($root_element->{'cmdname'})
                 or $self->{'targets'}->{$root_element});
        if ($sectioning_heading_commands{$root_element->{'cmdname'}}) {
          $self->_new_sectioning_command_target($root_element);
        }
      }
    }
  }
}

sub _html_get_tree_root_element($$;$);

# If $find_container is set, the element that holds the command output
# is found, otherwise the element that holds the command is found.  This is
# mostly relevant for footnote only.
# If no known root element type is found, the returned root element is undef,
# and not set to the element at the tree root
sub _html_get_tree_root_element($$;$)
{
  my $self = shift;
  my $command = shift;
  my $find_container = shift;

  # can be used to debug/understand what is going on
  #my $debug = 1;

  my $current = $command;
  #print STDERR "START ".Texinfo::Common::debug_print_element($current)."\n" if ($debug);

  my ($root_element, $root_command);
  while (1) {
    if ($current->{'type'}) {
      if ($current->{'type'} eq 'unit'
          or $current->{'type'} eq 'special_element') {
        #print STDERR "ROOT ELEMENT $current->{'type'}\n" if ($debug);
        return ($current, $root_command);
      }
    }
    if ($current->{'cmdname'}) {
      if ($root_commands{$current->{'cmdname'}}) {
        $root_command = $current;
        #print STDERR "CMD ROOT $current->{'cmdname'}\n" if ($debug);
        return ($root_element, $root_command) if defined($root_element);
      } elsif ($block_commands{$current->{'cmdname'}}
               and $block_commands{$current->{'cmdname'}} eq 'region') {
        if ($current->{'cmdname'} eq 'copying'
            and $self->{'global_commands'}
            and $self->{'global_commands'}->{'insertcopying'}) {
          foreach my $insertcopying(@{$self->{'global_commands'}
                                                        ->{'insertcopying'}}) {
            #print STDERR "INSERTCOPYING\n" if ($debug);
            my ($root_element, $root_command)
              = $self->_html_get_tree_root_element($insertcopying,
                                                   $find_container);
            return ($root_element, $root_command)
              if (defined($root_element) or defined($root_command));
          }
        } elsif ($current->{'cmdname'} eq 'titlepage'
                 and $self->get_conf('USE_TITLEPAGE_FOR_TITLE')
                 and $self->get_conf('SHOW_TITLE')
                 and $self->{'tree_units'}->[0]) {
          #print STDERR "FOR titlepage tree_units [0]\n" if ($debug);
          return ($self->{'tree_units'}->[0],
                  $self->{'tree_units'}->[0]->{'extra'}->{'unit_command'});
        }
        die "Problem $root_element, $root_command" if (defined($root_element)
                                                  or defined($root_command));
        return (undef, undef);
      } elsif ($find_container) {
        # @footnote and possibly @*contents when a separate element is set
        my ($special_element_variety, $special_element, $class_base,
            $special_element_direction)
         = $self->command_name_special_element_information($current->{'cmdname'});
        if ($special_element) {
          #print STDERR "SPECIAL $current->{'cmdname'}: $special_element_variety ($special_element_direction)\n" if ($debug);
          return ($special_element);
        }
      }
    }
    if ($current->{'structure'}
        and $current->{'structure'}->{'associated_unit'}) {
      #print STDERR "ASSOCIATED_UNIT ".Texinfo::Common::debug_print_element($current->{'structure'}->{'associated_unit'})."\n" if ($debug);
      $current = $current->{'structure'}->{'associated_unit'};
    } elsif ($current->{'parent'}) {
      #print STDERR "PARENT ".Texinfo::Common::debug_print_element($current->{'parent'})."\n" if ($debug);
      $current = $current->{'parent'};
    } else {
      #print STDERR "UNKNOWN ROOT ".Texinfo::Common::debug_print_element($current)."\n" if ($debug);
      return (undef, $root_command);
    }
  }
}

sub _html_set_pages_files($$$$$$$$)
{
  my $self = shift;
  my $tree_units = shift;
  my $special_elements = shift;
  my $output_file = shift;
  my $destination_directory = shift;
  my $output_filename = shift;
  my $document_name = shift;

  # Ensure that the document has pages
  return undef if (!defined($tree_units) or !@$tree_units);

  $self->initialize_tree_units_files();

  my $extension = '';
  $extension = '.'.$self->get_conf('EXTENSION')
            if (defined($self->get_conf('EXTENSION'))
                and $self->get_conf('EXTENSION') ne '');

  my %filenames_paths;
  my %unit_file_name_paths;
  # associate a file to the source information leading to set the file
  # name.  Use the first element source information associated to a file
  # The source information can be either a tree element associated to
  # the 'file_info_element' key, with a 'file_info_type' 'node' or
  # 'section'... or a specific source associated to the 'file_info_name'
  # key with 'file_info_type' 'special_file', or a source set if
  # nothing was found, with 'file_info_type' 'stand_in_file' and a
  # 'file_info_name'.  Redirection files are added in the output()
  # function.
  my %files_source_info = ();
  if (!$self->get_conf('SPLIT')) {
    $filenames_paths{$output_filename} = $output_file;
    foreach my $tree_unit (@$tree_units) {
      $unit_file_name_paths{$tree_unit} = $output_filename;
    }
    $files_source_info{$output_filename}
      = {'file_info_type' => 'special_file',
         'file_info_name' => 'non_split'};
  } else {
    my $node_top;
    $node_top = $self->{'labels'}->{'Top'} if ($self->{'labels'});

    my $top_node_filename = $self->top_node_filename($document_name);
    my $node_top_tree_unit;
    # first determine the top node file name.
    if ($node_top and defined($top_node_filename)) {
      my ($node_top_tree_unit) = $self->_html_get_tree_root_element($node_top);
      die "BUG: No element for top node" if (!defined($node_top_tree_unit));
      $filenames_paths{$top_node_filename} = undef;
      $unit_file_name_paths{$node_top_tree_unit} = $top_node_filename;
      $files_source_info{$top_node_filename}
         = {'file_info_type' => 'special_file',
            'file_info_name' => 'Top'};
    }
    my $file_nr = 0;
    my $previous_page;
    foreach my $tree_unit (@$tree_units) {
      # For Top node.
      next if (exists($unit_file_name_paths{$tree_unit}));
      my $file_tree_unit = $tree_unit->{'extra'}->{'first_in_page'};
      if (!$file_tree_unit) {
        cluck ("No first_in_page for $tree_unit\n");
      }
      if (not exists($unit_file_name_paths{$file_tree_unit})) {
        foreach my $root_command (@{$file_tree_unit->{'contents'}}) {
          if ($root_command->{'cmdname'}
              and $root_command->{'cmdname'} eq 'node') {
            my $node_filename;
            # double node are not normalized, they are handled here
            if (!defined($root_command->{'extra'}->{'normalized'})
                or !defined($self->{'labels'}->{
                            $root_command->{'extra'}->{'normalized'}})) {
              $node_filename = 'unknown_node';
              $node_filename .= $extension;
              my $file_source_info = {'file_info_type' => 'stand_in_file',
                                      'file_info_name' => 'unknown_node'};
              $files_source_info{$node_filename} = $file_source_info
                unless ($files_source_info{$node_filename});
            } else {
              # Nodes with {'extra'}->{'normalized'} should always be in
              # 'labels', and thus in targets.  It is a bug otherwise.
              $node_filename
                = $self->{'targets'}->{$root_command}->{'node_filename'};
              my $file_source_info = {'file_info_type' => 'node',
                                      'file_info_element' => $root_command};
              $files_source_info{$node_filename} = $file_source_info
                unless ($files_source_info{$node_filename}
                        and $files_source_info{$node_filename}
                              ->{'file_info_type'} ne 'stand_in_file');
            }
            $filenames_paths{$node_filename} = undef;
            $unit_file_name_paths{$file_tree_unit} = $node_filename;
            last;
          }
        }
        if (not defined($unit_file_name_paths{$file_tree_unit})) {
          # use section to do the file name if there is no node
          my $command = $self->tree_unit_element_command($file_tree_unit);
          if ($command) {
            if ($command->{'cmdname'} eq 'top' and !$node_top
                and defined($top_node_filename)) {
              $filenames_paths{$top_node_filename} = undef;
              $unit_file_name_paths{$file_tree_unit} = $top_node_filename;
              $files_source_info{$top_node_filename}
                  = {'file_info_type' => 'special_file',
                     'file_info_name' => 'Top'};
            } else {
              my $section_filename
                   = $self->{'targets'}->{$command}->{'section_filename'};
              $filenames_paths{$section_filename} = undef;
              $unit_file_name_paths{$file_tree_unit} = $section_filename;
              $files_source_info{$section_filename}
                = {'file_info_type' => 'section',
                   'file_info_element' => $command}
                 unless($files_source_info{$section_filename}
                        and $files_source_info{$section_filename}
                                ->{'file_info_type'} ne 'stand_in_file');
            }
          } else {
            # when everything else has failed
            if ($file_nr == 0 and !$node_top
                and defined($top_node_filename)) {
              $filenames_paths{$top_node_filename} = undef;
              $unit_file_name_paths{$file_tree_unit} = $top_node_filename;
              $files_source_info{$top_node_filename}
                = {'file_info_type' => 'stand_in_file',
                   'file_info_name' => 'Top'}
                      unless($files_source_info{$top_node_filename});
            } else {
              my $filename = $document_name . "_$file_nr";
              $filename .= $extension;
              $filenames_paths{$filename} = undef;
              $unit_file_name_paths{$file_tree_unit} = $filename;
              $files_source_info{$filename}
                 = {'file_info_type' => 'stand_in_file',
                    'file_info_name' => 'unknown'}
                unless($files_source_info{$filename});
            }
            $file_nr++;
          }
        }
      }
      if (not exists($unit_file_name_paths{$tree_unit})) {
        $unit_file_name_paths{$tree_unit}
           = $unit_file_name_paths{$file_tree_unit}
      }
    }
  }

  foreach my $tree_unit (@$tree_units) {
    my $filename = $unit_file_name_paths{$tree_unit};
    # check
    if (!$files_source_info{$filename}) {
      print STDERR "BUG: no files_source_info: $filename\n";
    }
    my $filepath = $filenames_paths{$filename};
    if (defined($self->{'file_id_setting'}->{'tree_unit_file_name'})) {
      # NOTE the information that it is associated with @top or @node Top
      # may be determined with $self->element_is_tree_unit_top($tree_unit);
      my ($user_filename, $user_filepath)
         = &{$self->{'file_id_setting'}->{'tree_unit_file_name'}}(
               $self, $tree_unit, $filename, $filepath);
      if (defined($user_filename)) {
        $filename = $user_filename;
        $filenames_paths{$filename} = $user_filepath;
        $files_source_info{$filename} = {'file_info_type' => 'special_file',
                                         'file_info_name' => 'user_defined'};
      }
    }
    $self->set_tree_unit_file($tree_unit, $filename);
    my $tree_unit_filename = $tree_unit->{'structure'}->{'unit_filename'};
    $self->{'file_counters'}->{$tree_unit_filename} = 0
       if (!exists($self->{'file_counters'}->{$tree_unit_filename}));
    $self->{'file_counters'}->{$tree_unit_filename}++;
    print STDERR 'Page '
      # uncomment for perl object name
      #."$tree_unit "
      .Texinfo::Structuring::root_or_external_element_cmd_texi($tree_unit)
      .": $tree_unit_filename($self->{'file_counters'}->{$tree_unit_filename})\n"
             if ($self->get_conf('DEBUG'));
  }
  if ($special_elements) {
    foreach my $special_element (@$special_elements) {
      my $filename
       = $self->{'targets'}->{$special_element}->{'special_element_filename'};
      if (defined($filename)) {
        $filenames_paths{$filename} = undef;
        $self->set_tree_unit_file($special_element, $filename);
        $self->{'file_counters'}->{$filename} = 0
           if (!exists($self->{'file_counters'}->{$filename}));
        $self->{'file_counters'}->{$filename}++;
        print STDERR 'Special page'
           # uncomment for perl object name
           #." $special_element"
           .": $filename($self->{'file_counters'}->{$filename})\n"
                 if ($self->get_conf('DEBUG'));
        my $file_source_info = {'file_info_element' => $special_element,
                                'file_info_type' => 'special_element'};
        $files_source_info{$filename} = $file_source_info
          unless($files_source_info{$filename}
                 and $files_source_info{$filename}->{'file_info_type'}
                       ne 'stand_in_file');
      }
    }
  }
  foreach my $filename (keys(%filenames_paths)) {
    $self->set_file_path($filename, $destination_directory,
                         $filenames_paths{$filename});
  }
  return %files_source_info;
}

# $ROOT is a parsed Texinfo tree.  Return a list of the "elements" we need to
# output in the HTML file(s).  Each "element" is what can go in one HTML file,
# such as the content between @node lines in the Texinfo source.
# Also do some conversion setup that is to be done in both convert() and output().
sub _prepare_conversion_tree_units($$$$)
{
  my $self = shift;
  my $root = shift;
  my $destination_directory = shift;
  my $document_name = shift;

  my $tree_units;

  if ($self->get_conf('USE_NODES')) {
    $tree_units = Texinfo::Structuring::split_by_node($root);
  } else {
    $tree_units = Texinfo::Structuring::split_by_section($root);
  }

  $self->{'tree_units'} = $tree_units
    if (defined($tree_units));

  # This may be done as soon as tree units are available.
  $self->_prepare_tree_units_global_targets($tree_units);

  # the presence of contents elements in the document is used in diverse
  # places, set it once for all here
  my @contents_elements_options
                  = grep {Texinfo::Common::valid_customization_option($_)}
                        sort(keys(%contents_command_special_element_variety));
  $self->set_global_document_commands('last', \@contents_elements_options);

  # configuration used to determine if a special element is to be done
  # (in addition to contents)
  my @conf_for_special_elements = ('footnotestyle');
  $self->set_global_document_commands('last', \@conf_for_special_elements);
  # Do that before the other elements, to be sure that special page ids
  # are registered before elements id are.
  # NOTE if the last value of footnotestyle is separate, all the footnotes
  # formatted text are set to the special element set in _prepare_special_elements
  # as _html_get_tree_root_element uses the Footnote direction for every footnote.
  # Therefore if @footnotestyle separate is set late in the document the current
  # value may not be consistent with the link obtained for the footnote
  # formatted text.  This is not an issue, as the manual says that
  # @footnotestyle should only appear in the preamble, and it makes sense
  # to have something consistent in the whole document for footnotes position.
  my $special_elements
    = $self->_prepare_special_elements($tree_units, $destination_directory,
                                       $document_name);
  # reset to the default
  $self->set_global_document_commands('before', \@conf_for_special_elements);

  if ($special_elements and defined($tree_units) and scalar(@$tree_units)) {
    my $previous_tree_unit = $tree_units->[-1];
    foreach my $special_element (@$special_elements) {
      $special_element->{'structure'}->{'unit_prev'} = $previous_tree_unit;
      $previous_tree_unit->{'structure'}->{'unit_next'} = $special_element;
      $previous_tree_unit = $special_element;
    }
  }

  #if ($tree_units) {
  #  foreach my $element(@{$tree_units}) {
  #    print STDERR "ELEMENT $element->{'type'}: $element\n";
  #  }
  #}

  $self->_set_root_commands_targets_node_files($tree_units);

  # setup untranslated strings
  $self->_translate_names();

  return ($tree_units, $special_elements);
}

sub _prepare_special_elements($$$$)
{
  my $self = shift;
  my $tree_units = shift;
  my $destination_directory = shift;
  my $document_name = shift;

  my %do_special;
  if ($self->{'structuring'} and $self->{'structuring'}->{'sectioning_root'}
      and scalar(@{$self->{'structuring'}->{'sections_list'}}) > 1) {
    if ($self->get_conf('CONTENTS_OUTPUT_LOCATION') eq 'separate_element') {
      foreach my $cmdname ('shortcontents', 'contents') {
        my $special_element_variety
            = $contents_command_special_element_variety{$cmdname};
        if ($self->get_conf($cmdname)) {
            $do_special{$special_element_variety} = 1;
        }
      }
    }
  }
  if ($self->{'global_commands'}->{'footnote'}
      and $self->get_conf('footnotestyle') eq 'separate'
      and $tree_units and scalar(@$tree_units) > 1) {
    $do_special{'footnotes'} = 1;
  }

  if ((!defined($self->get_conf('DO_ABOUT'))
       and $tree_units and scalar(@$tree_units) > 1
           and ($self->get_conf('SPLIT') or $self->get_conf('HEADERS')))
       or ($self->get_conf('DO_ABOUT'))) {
    $do_special{'about'} = 1;
  }

  my $extension = '';
  $extension = $self->get_conf('EXTENSION')
    if (defined($self->get_conf('EXTENSION')));

  my $special_elements = [];
  # sort special elements according to their index order from
  # special_element_info 'order'.
  # First reverse the hash, using arrays in case some elements are at the
  # same index, and sort to get alphabetically sorted special element
  # varieties that are at the same index.
  my %special_elements_indices;
  foreach my $special_element_variety
      (sort($self->special_element_info('order'))) {
    my $index = $self->special_element_info('order', $special_element_variety);
    $special_elements_indices{$index} = []
      if (not exists ($special_elements_indices{$index}));
    push @{$special_elements_indices{$index}}, $special_element_variety;
  }
  # now sort according to indices
  my @sorted_elements_varieties;
  foreach my $index (sort { $a <=> $b } (keys(%special_elements_indices))) {
    push @sorted_elements_varieties, @{$special_elements_indices{$index}};
  }

  foreach my $special_element_variety (@sorted_elements_varieties) {
    next unless ($do_special{$special_element_variety});

    my $element = {'type' => 'special_element',
                   'extra' => {'special_element_variety'
                                   => $special_element_variety,},
                   'structure' => {'directions' => {}}};
    $element->{'structure'}->{'directions'}->{'This'} = $element;
    my $special_element_direction
     = $self->special_element_info('direction', $special_element_variety);
    $self->{'special_elements_directions'}->{$special_element_direction}
     = $element;
    push @$special_elements, $element;

    my $target
        = $self->special_element_info('target', $special_element_variety);
    my $default_filename;
    if ($self->get_conf('SPLIT') or !$self->get_conf('MONOLITHIC')
        # in general $document_name not defined means called through convert
        and defined($document_name)) {
      my $special_element_file_string =
         $self->special_element_info('file_string', $special_element_variety);
      $special_element_file_string = '' if (!defined($special_element_file_string));
      $default_filename = $document_name . $special_element_file_string;
      $default_filename .= '.'.$extension if (defined($extension));
    } else {
      $default_filename = undef;
    }

    my $filename;
    if (defined($self->{'file_id_setting'}->{'special_element_target_file_name'})) {
      ($target, $filename)
         = &{$self->{'file_id_setting'}->{'special_element_target_file_name'}}(
                                                            $self,
                                                            $element,
                                                            $target,
                                                            $default_filename);
    }
    $filename = $default_filename if (!defined($filename));

    if ($self->get_conf('DEBUG')) {
      my $fileout = $filename;
      $fileout = 'UNDEF' if (!defined($fileout));
      print STDERR 'Add special'
        # uncomment for the perl object name
        #." $element"
        ." $special_element_variety: target $target,\n".
        "    filename $fileout\n";
    }
    $self->{'targets'}->{$element} = {'target' => $target,
                                      'special_element_filename' => $filename,
                                     };
    $self->{'seen_ids'}->{$target} = 1;
  }
  if ($self->get_conf('FRAMES')) {
    $self->{'frame_pages_filenames'} = {};
    foreach my $special_element_variety (keys(%{$self->{'frame_pages_file_string'}})) {
      my $default_filename;
      $default_filename = $document_name.
        $self->{'frame_pages_file_string'}->{$special_element_variety};
      $default_filename .= '.'.$extension if (defined($extension));

      my $element = {'type' => 'special_element',
                   'extra' => {'special_element_variety'
                                  => $special_element_variety, }};

      # only the filename is used
      my ($target, $filename);
      if (defined($self->{'file_id_setting'}->{'special_element_target_file_name'})) {
        ($target, $filename)
          = &{$self->{'file_id_setting'}->{'special_element_target_file_name'}}(
                                                            $self,
                                                            $element,
                                                            $target,
                                                            $default_filename);
      }
      $filename = $default_filename if (!defined($filename));
      $self->{'frame_pages_filenames'}->{$special_element_variety} = $filename;
    }
  }
  return $special_elements;
}

sub _prepare_contents_elements($)
{
  my $self = shift;

  if ($self->{'structuring'} and $self->{'structuring'}->{'sectioning_root'}
      and scalar(@{$self->{'structuring'}->{'sections_list'}}) > 1) {
    foreach my $cmdname ('contents', 'shortcontents') {
      my $special_element_variety
           = $contents_command_special_element_variety{$cmdname};
      if ($self->get_conf($cmdname)) {
        my $default_filename;
        if ($self->get_conf('CONTENTS_OUTPUT_LOCATION') eq 'after_title') {
          if ($self->{'tree_units'} and $self->{'tree_units'}->[0]->{'structure'}
              and exists($self->{'tree_units'}->[0]->{'structure'}->{'unit_filename'})) {
            $default_filename
              = $self->{'tree_units'}->[0]->{'structure'}->{'unit_filename'};
          }
        } elsif ($self->get_conf('CONTENTS_OUTPUT_LOCATION') eq 'after_top') {
          my $section_top = undef;
          if ($self->{'global_commands'} and $self->{'global_commands'}->{'top'}) {
            $section_top = $self->{'global_commands'}->{'top'};
            $default_filename = $self->command_filename($section_top);
          }
        } elsif ($self->get_conf('CONTENTS_OUTPUT_LOCATION') eq 'inline') {
          if ($self->{'global_commands'}
              and $self->{'global_commands'}->{$cmdname}) {
            foreach my $command(@{$self->{'global_commands'}->{$cmdname}}) {
              my ($root_element, $root_command)
                = $self->_html_get_tree_root_element($command);
              if (defined($root_element) and $root_element->{'structure'}
                  and exists($root_element->{'structure'}->{'unit_filename'})) {
                $default_filename
                   = $root_element->{'structure'}->{'unit_filename'};
                last;
              }
            }
          } else {
            next;
          }
        } else { # in this case, there should already be a special element
                 # if needed, done together with the other special elements.
          next;
        }

        my $contents_element = {'type' => 'special_element',
                        'extra' => {'special_element_variety'
                                             => $special_element_variety}};
        my $special_element_direction
         = $self->special_element_info('direction', $special_element_variety);
        $self->{'special_elements_directions'}->{$special_element_direction}
         = $contents_element;
        my $target
         = $self->special_element_info('target', $special_element_variety);
        my $filename;
        if (defined($self->{'file_id_setting'}->{'special_element_target_file_name'})) {
          ($target, $filename)
            = &{$self->{'file_id_setting'}->{'special_element_target_file_name'}}(
                                                          $self,
                                                          $contents_element,
                                                          $target,
                                                          $default_filename);
        }
        $filename = $default_filename if (!defined($filename));
        if ($self->get_conf('DEBUG')) {
          my $str_filename = $filename;
          $str_filename = 'UNDEF' if (not defined($str_filename));
          print STDERR 'Add content'
            # uncomment to get the perl obect name
            #." $contents_element"
            ." $special_element_variety: target $target,\n".
             "    filename $str_filename\n";
        }
        $self->{'targets'}->{$contents_element}
                               = {'target' => $target,
                                  'special_element_filename' => $filename,
                                  'filename' => $filename,
                                 };
      }
    }
  }
}

# Associate tree units with the global targets, First, Last, Top, Index.
sub _prepare_tree_units_global_targets($$)
{
  my $self = shift;
  my $tree_units = shift;

  $self->{'global_target_elements_directions'} = {};
  $self->{'global_target_elements_directions'}->{'First'} = $tree_units->[0];
  $self->{'global_target_elements_directions'}->{'Last'} = $tree_units->[-1];
  # It is always the first printindex, even if it is not output (for example
  # it is in @copying and @titlepage, which are certainly wrong constructs).
  if ($self->{'global_commands'} and $self->{'global_commands'}->{'printindex'}) {
    # Here root_element can only be a tree unit, or maybe undef if there
    # are no tree unit at all
    my ($root_element, $root_command)
     = $self->_html_get_tree_root_element($self->{'global_commands'}->{'printindex'}->[0]);
    if (defined($root_element)) {
      if ($root_command and $root_command->{'cmdname'} eq 'node'
          and $root_command->{'extra'}->{'associated_section'}) {
        $root_command = $root_command->{'extra'}->{'associated_section'};
      }
      # find the first level 1 sectioning element to associate the printindex with
      if ($root_command and $root_command->{'cmdname'} ne 'node') {
        while ($root_command->{'structure'}->{'section_level'} > 1
               and $root_command->{'structure'}->{'section_up'}
               and $root_command->{'structure'}->{'section_up'}
                                        ->{'structure'}->{'associated_unit'}) {
          $root_command = $root_command->{'structure'}->{'section_up'};
          $root_element = $root_command->{'structure'}->{'associated_unit'};
        }
      }
      $self->{'global_target_elements_directions'}->{'Index'} = $root_element;
    }
  }

  my $node_top;
  $node_top = $self->{'labels'}->{'Top'} if ($self->{'labels'});
  my $section_top;
  $section_top = $self->{'global_commands'}->{'top'} if ($self->{'global_commands'});
  if ($section_top) {
    $self->{'global_target_elements_directions'}->{'Top'}
            = $section_top->{'structure'}->{'associated_unit'};
  } elsif ($node_top) {
    my $tree_unit_top = $node_top->{'structure'}->{'associated_unit'};
    if (!$tree_unit_top) {
      die "No associated unit for node_top: "
         .Texinfo::Common::debug_print_element($node_top, 1);
    }
    $self->{'global_target_elements_directions'}->{'Top'} = $tree_unit_top;
  } else {
    $self->{'global_target_elements_directions'}->{'Top'} = $tree_units->[0];
  }

  if ($self->get_conf('DEBUG')) {
    print STDERR "GLOBAL DIRECTIONS:\n";
    foreach my $global_direction (@global_directions) {
      if (defined($self->global_direction_element($global_direction))) {
        my $global_element = $self->global_direction_element($global_direction);
        print STDERR "$global_direction"
            # uncomment to get the perl object name
            # ."($global_element)"
     .': '. Texinfo::Structuring::root_or_external_element_cmd_texi($global_element)."\n";
      }
    }
  }
}

sub _prepare_index_entries($)
{
  my $self = shift;

  my $indices_information = $self->{'indices_information'};
  if ($indices_information) {
    my $no_unidecode;
    $no_unidecode = 1 if (defined($self->get_conf('USE_UNIDECODE'))
                          and !$self->get_conf('USE_UNIDECODE'));

    my $merged_index_entries
        = Texinfo::Structuring::merge_indices($indices_information);
    my $index_entries_sort_strings;
    ($self->{'index_entries_by_letter'}, $index_entries_sort_strings)
            = Texinfo::Structuring::sort_indices($self,
                                    $self, $merged_index_entries,
                                    $indices_information,
                                    'by_letter');
    $self->{'index_entries'} = $merged_index_entries;

    foreach my $index_name (sort(keys(%$indices_information))) {
      foreach my $index_entry (@{$indices_information->{$index_name}
                                                    ->{'index_entries'}}) {
        my $main_entry_element = $index_entry->{'entry_element'};
        # does not refer to the document
        next if ($main_entry_element->{'extra'}
                 and ($main_entry_element->{'extra'}->{'seeentry'}
                      or $main_entry_element->{'extra'}->{'seealso'}));
        my $region = '';
        $region = "$main_entry_element->{'extra'}->{'element_region'}-"
          if (defined($main_entry_element->{'extra'}->{'element_region'}));
        my $entry_reference_content_element
          = Texinfo::Common::index_content_element($main_entry_element, 1);
        my @contents = ($entry_reference_content_element);
        my $subentries_tree
         = $self->comma_index_subentries_tree($main_entry_element,
                                              ' ');
        if (defined($subentries_tree)) {
          push @contents, @{$subentries_tree->{'contents'}};
        }
        my $trimmed_contents
          = Texinfo::Common::trim_spaces_comment_from_content(\@contents);
        my $normalized_index =
          Texinfo::Convert::NodeNameNormalization::normalize_transliterate_texinfo(
            {'contents' => \@contents}, $no_unidecode);
        my $target_base = "index-" . $region .$normalized_index;
        my $nr=1;
        my $target = $target_base;
        while ($self->{'seen_ids'}->{$target}) {
          $target = $target_base.'-'.$nr;
          $nr++;
          # Avoid integer overflow
          die if ($nr == 0);
        }
        $self->{'seen_ids'}->{$target} = 1;
        my $target_element = $main_entry_element;
        $target_element = $index_entry->{'entry_associated_element'}
          if ($index_entry->{'entry_associated_element'});
        $self->{'targets'}->{$target_element} = {'target' => $target, };
      }
    }
  }
}

sub _prepare_footnotes($)
{
  my $self = shift;

  my $footid_base = 'FOOT';
  my $docid_base = 'DOCF';

  $self->{'pending_footnotes'} = [];

  if ($self->{'global_commands'}->{'footnote'}) {
    my $footnote_nr = 0;
    foreach my $footnote (@{$self->{'global_commands'}->{'footnote'}}) {
      $footnote_nr++;
      my $nr = $footnote_nr;
      # anchor for the footnote text
      my $footid = $footid_base.$nr;
      # anchor for the location of the @footnote in the document
      my $docid = $docid_base.$nr;
      while ($self->{'seen_ids'}->{$docid} or $self->{'seen_ids'}->{$footid}) {
        $nr++;
        $footid = $footid_base.$nr;
        $docid = $docid_base.$nr;
        # Avoid integer overflow
        die if ($nr == 0);
      }
      $self->{'seen_ids'}->{$footid} = 1;
      $self->{'seen_ids'}->{$docid} = 1;
      $self->{'targets'}->{$footnote} = { 'target' => $footid };
      $self->{'special_targets'}->{'footnote_location'}->{$footnote}
         = { 'target' => $docid };
      print STDERR 'Enter footnote'
        # uncomment for the perl object name
        #." $footnote"
        .": target $footid, nr $footnote_nr\n"
       .Texinfo::Convert::Texinfo::convert_to_texinfo($footnote)."\n"
        if ($self->get_conf('DEBUG'));
    }
  }
}

sub _external_node_href($$$;$)
{
  my $self = shift;
  my $external_node = shift;
  my $filename = shift;
  # for messages only
  my $source_command = shift;

  #print STDERR "external_node: ".join('|', keys(%$external_node))."\n";
  my ($target_filebase, $target)
      = $self->_normalized_label_id_file($external_node->{'normalized'},
                                         $external_node->{'node_content'});

  my $xml_target = _normalized_to_id($target);

  my $default_target_split = $self->get_conf('EXTERNAL_CROSSREF_SPLIT');

  my $external_file_extension = '';
  my $external_extension = $self->get_conf('EXTERNAL_CROSSREF_EXTENSION');
  $external_extension = $self->get_conf('EXTENSION')
    if (not defined($external_extension));
  $external_file_extension = '.' . $external_extension
    if (defined($external_extension) and $external_extension ne '');

  my $target_split;
  my $file;
  if ($external_node->{'manual_content'}) {
    my $manual_name = Texinfo::Convert::Text::convert_to_text(
       {'contents' => $external_node->{'manual_content'}},
       { 'code' => 1,
         Texinfo::Convert::Text::copy_options_for_convert_text($self)});
    if ($self->get_conf('IGNORE_REF_TO_TOP_NODE_UP') and $xml_target eq '') {
      my $top_node_up = $self->get_conf('TOP_NODE_UP');
      if (defined($top_node_up) and "($manual_name)" eq $top_node_up) {
        return '';
      }
    }
    my $manual_base = $manual_name;
    $manual_base =~ s/\.info*$//;
    $manual_base =~ s/^.*\///;
    my $document_split = $self->get_conf('SPLIT');
    $document_split = 'mono' if (!$document_split);
    my $split_found;
    my $href;
    my $htmlxref_info = $self->{'htmlxref'}->{$manual_base};
    if ($htmlxref_info) {
      foreach my $split_ordered (@{$htmlxref_entries{$document_split}}) {
        if (defined($htmlxref_info->{$split_ordered})) {
          $split_found = $split_ordered;
          $href = $self->url_protect_url_text($htmlxref_info->{$split_ordered});
          last;
        }
      }
    }
    if (defined($split_found)) {
      $target_split = 1 unless ($split_found eq 'mono');
    } else { # nothing specified for that manual, use default
      $target_split = $default_target_split;
      if ($self->get_conf('CHECK_HTMLXREF')) {
        if (defined($source_command) and $source_command->{'source_info'}) {
          my $node_manual_key = $source_command.'-'.$manual_name;
          if (!$self->{'check_htmlxref_already_warned'}->{$node_manual_key}) {
            $self->line_warn($self, sprintf(__(
                    "no htmlxref.cnf entry found for `%s'"), $manual_name),
                             $source_command->{'source_info'});
            $self->{'check_htmlxref_already_warned'}->{$node_manual_key} = 1;
          }
        } else {
          if (!$self->{'check_htmlxref_already_warned'}->{'UNDEF-'.$manual_name}) {
            $self->document_warn($self, sprintf(__(
              "no htmlxref.cnf entry found for `%s'"), $manual_name),
              );
            $self->{'check_htmlxref_already_warned'}->{'UNDEF-'.$manual_name} = 1;
            cluck;
          }
        }
      }
    }

    if ($target_split) {
      if (defined($href)) {
        $file = $href;
      } else {
        my $manual_dir = $manual_base;
        if (defined($self->{'output_format'}) and $self->{'output_format'} ne '') {
          $manual_dir .= '_'.$self->{'output_format'};
        }
        if (defined($self->get_conf('EXTERNAL_DIR'))) {
          $file = $self->get_conf('EXTERNAL_DIR')."/$manual_dir";
        } elsif ($self->get_conf('SPLIT')) {
          $file = "../$manual_dir";
        }
        $file = $self->url_protect_file_text($file);
      }
      $file .= "/";
    } else {# target not split
      if (defined($href)) {
        $file = $href;
      } else {
        my $manual_file_name = $manual_base . $external_file_extension;
        if (defined($self->get_conf('EXTERNAL_DIR'))) {
          $file = $self->get_conf('EXTERNAL_DIR')."/$manual_file_name";
        } elsif ($self->get_conf('SPLIT')) {
          $file = "../$manual_file_name";
        } else {
          $file = $manual_file_name;
        }
        $file = $self->url_protect_file_text($file);
      }
    }
  } else {
    $file = '';
    $target_split = $default_target_split;
  }

  if ($target eq '') {
    if ($target_split) {
      if (defined($self->get_conf('TOP_NODE_FILE_TARGET'))) {
        return $file . $self->get_conf('TOP_NODE_FILE_TARGET');
      } else {
        return $file;
      }
    } else {
      return $file . '#Top';
    }
  }

  if (! $target_split) {
    return $file . '#' . $xml_target;
  } else {
    my $file_name;
    if ($target eq 'Top' and defined($self->get_conf('TOP_NODE_FILE_TARGET'))) {
      $file_name = $self->get_conf('TOP_NODE_FILE_TARGET');
    } else {
      $file_name = $target_filebase . $external_file_extension;
    }
    return $file . $file_name . '#' . $xml_target;
  }
}

# Output a list of the nodes immediately below this one
sub _mini_toc
{
  my ($self, $command) = @_;

  my $result = '';
  my $entry_index = 0;

  if ($command->{'structure'}
      and $command->{'structure'}->{'section_childs'}
      and @{$command->{'structure'}->{'section_childs'}}) {
    $result .= $self->html_attribute_class('ul', ['mini-toc']).">\n";

    foreach my $section (@{$command->{'structure'}->{'section_childs'}}) {
      my $tree = $self->command_text($section, 'tree_nonumber');
      my $text = $self->convert_tree($tree, "mini_toc \@$section->{'cmdname'}");

      $entry_index++;
      my $accesskey = '';
      $accesskey = " accesskey=\"$entry_index\""
        if ($self->get_conf('USE_ACCESSKEY') and $entry_index < 10);

      my $href = $self->command_href($section);
      if ($text ne '') {
        if ($href ne '') {
          my $href_attribute = '';
          if ($href ne '') {
            $href_attribute = " href=\"$href\"";
          }
          $result .= "<li><a${href_attribute}$accesskey>$text</a>";
        } else {
          $result .= "<li>$text";
        }
        $result .= "</li>\n";
      }
    }
    $result .= "</ul>\n";
  }
  return $result;
}

sub _default_format_contents($$;$$)
{
  my $self = shift;
  my $cmdname = shift;
  my $command = shift;
  my $filename = shift;

  $filename = $self->get_info('current_filename') if (!defined($filename));

  my $structuring = $self->get_info('structuring');
  return ''
   if (!$structuring or !$structuring->{'sectioning_root'});

  my $section_root = $structuring->{'sectioning_root'};
  my $contents;
  $contents = 1 if ($cmdname eq 'contents');

  my $min_root_level = $section_root->{'structure'}->{'section_childs'}->[0]
                                             ->{'structure'}->{'section_level'};
  my $max_root_level = $section_root->{'structure'}->{'section_childs'}->[0]
                                              ->{'structure'}->{'section_level'};
  foreach my $top_section (@{$section_root->{'structure'}->{'section_childs'}}) {
    $min_root_level = $top_section->{'structure'}->{'section_level'}
      if ($top_section->{'structure'}->{'section_level'} < $min_root_level);
    $max_root_level = $top_section->{'structure'}->{'section_level'}
      if ($top_section->{'structure'}->{'section_level'} > $max_root_level);
  }
  # chapter level elements are considered top-level here.
  $max_root_level = 1 if ($max_root_level < 1);
  #print STDERR "ROOT_LEVEL Max: $max_root_level, Min: $min_root_level\n";
  my @toc_ul_classes;
  push @toc_ul_classes, 'toc-numbered-mark'
            if ($self->get_conf('NUMBER_SECTIONS'));

  my $result = '';
  if ($contents and !defined($self->get_conf('BEFORE_TOC_LINES'))
      or (!$contents and !defined($self->get_conf('BEFORE_SHORT_TOC_LINES')))) {
    $result .= $self->html_attribute_class('div', [$cmdname]).">\n";
  } elsif($contents) {
    $result .= $self->get_conf('BEFORE_TOC_LINES');
  } else {
    $result .= $self->get_conf('BEFORE_SHORT_TOC_LINES');
  }

  my $toplevel_contents;
  if (@{$section_root->{'structure'}->{'section_childs'}} > 1) {
    $result .= $self->html_attribute_class('ul', \@toc_ul_classes) .">\n";
    $toplevel_contents = 1;
  }

  my $link_to_toc = (!$contents and $self->get_conf('SHORT_TOC_LINK_TO_TOC')
                     and ($self->get_conf('contents'))
                     and ($self->get_conf('CONTENTS_OUTPUT_LOCATION') ne 'inline'
                          or $self->_has_contents_or_shortcontents()));
  foreach my $top_section (@{$section_root->{'structure'}->{'section_childs'}}) {
    my $section = $top_section;
 SECTION:
    while ($section) {
      if ($section->{'cmdname'} ne 'top') {
        my $text = $self->command_text($section);
        my $href;
        if ($link_to_toc) {
          $href = $self->command_contents_href($section, 'contents', $filename);
        } else {
          $href = $self->command_href($section, $filename);
        }
        my $toc_id = $self->command_contents_target($section, $cmdname);
        if ($text ne '') {
          # no indenting for shortcontents
          $result .= (' ' x
            (2*($section->{'structure'}->{'section_level'} - $min_root_level)))
              if ($contents);
          if ($toc_id ne '' or $href ne '') {
            my $toc_name_attribute = '';
            if ($toc_id ne '') {
              $toc_name_attribute = " id=\"$toc_id\"";
            }
            my $href_attribute = '';
            if ($href ne '') {
              $href_attribute = " href=\"$href\"";
            }
            my $rel = '';
            if ($section->{'extra'}
                and $section->{'extra'}->{'associated_node'}
                and $section->{'extra'}->{'associated_node'}->{'extra'}
                and $section->{'extra'}->{'associated_node'}->{'extra'}->{'isindex'}) {
              $rel = ' rel="index"';
            }
            $result .= "<li><a${toc_name_attribute}${href_attribute}$rel>$text</a>";
          } else {
            $result .= "<li>$text";
          }
        }
      } elsif ($section->{'structure'}->{'section_childs'}
               and @{$section->{'structure'}->{'section_childs'}}
               and $toplevel_contents) {
        $result .= "<li>";
      }
      # for shortcontents don't do child if child is not toplevel
      if ($section->{'structure'}->{'section_childs'}
          and ($contents
               or $section->{'structure'}->{'section_level'} < $max_root_level)) {
        # no indenting for shortcontents
        $result .= "\n"
         . ' ' x (2*($section->{'structure'}->{'section_level'} - $min_root_level))
            if ($contents);
        $result .= $self->html_attribute_class('ul', \@toc_ul_classes) .">\n";
        $section = $section->{'structure'}->{'section_childs'}->[0];
      } elsif ($section->{'structure'}->{'section_next'}
               and $section->{'cmdname'} ne 'top') {
        $result .= "</li>\n";
        last if ($section eq $top_section);
        $section = $section->{'structure'}->{'section_next'};
      } else {
        #last if ($section eq $top_section);
        if ($section eq $top_section) {
          $result .= "</li>\n" unless ($section->{'cmdname'} eq 'top');
          last;
        }
        while ($section->{'structure'}->{'section_up'}) {
          $section = $section->{'structure'}->{'section_up'};
          $result .= "</li>\n"
           . ' ' x (2*($section->{'structure'}->{'section_level'} - $min_root_level))
            . "</ul>";
          if ($section eq $top_section) {
            $result .= "</li>\n" if ($toplevel_contents);
            last SECTION;
          }
          if ($section->{'structure'}->{'section_next'}) {
            $result .= "</li>\n";
            $section = $section->{'structure'}->{'section_next'};
            last;
          }
        }
      }
    }
  }
  if (@{$section_root->{'structure'}->{'section_childs'}} > 1) {
    $result .= "\n</ul>";
  }
  if ($contents and !defined($self->get_conf('AFTER_TOC_LINES'))
      or (!$contents and !defined($self->get_conf('AFTER_SHORT_TOC_LINES')))) {
    $result .= "\n</div>\n";
  } elsif($contents) {
    $result .= $self->get_conf('AFTER_TOC_LINES');
  } else {
    $result .= $self->get_conf('AFTER_SHORT_TOC_LINES');
  }
  return $result;
}

sub _default_format_program_string($)
{
  my $self = shift;
  if (defined($self->get_conf('PROGRAM'))
      and $self->get_conf('PROGRAM') ne ''
      and defined($self->get_conf('PACKAGE_URL'))) {
    return $self->convert_tree(
      $self->gdt('This document was generated on @emph{@today{}} using @uref{{program_homepage}, @emph{{program}}}.',
         { 'program_homepage' => $self->get_conf('PACKAGE_URL'),
           'program' => $self->get_conf('PROGRAM') }));
  } else {
    return $self->convert_tree(
      $self->gdt('This document was generated on @emph{@today{}}.'));
  }
}

sub _default_format_end_file($$)
{
  my $self = shift;
  my $filename = shift;

  my $program_text = '';
  if ($self->get_conf('PROGRAM_NAME_IN_FOOTER')) {
    my $program_string
      = &{$self->formatting_function('format_program_string')}($self);
    my $open = $self->html_attribute_class('span', ['program-in-footer']);
    if ($open ne '') {
      $program_string = $open.'>'.$program_string.'</span>';
    }
    $program_text .= "<p>
  $program_string
</p>";
  }

  my $pre_body_close = $self->get_conf('PRE_BODY_CLOSE');
  $pre_body_close = '' if (!defined($pre_body_close));

  my $jslicenses = $self->get_info('jslicenses');
  if ($jslicenses
      and (($jslicenses->{'infojs'}
            and scalar(keys %{$jslicenses->{'infojs'}}))
           or (($self->get_file_information('mathjax', $filename)
                or !$self->get_conf('SPLIT'))
               and ($jslicenses->{'mathjax'}
                    and scalar(keys %{$jslicenses->{'mathjax'}}))))) {
    my $js_setting = $self->get_conf('JS_WEBLABELS');
    my $js_path = $self->get_conf('JS_WEBLABELS_FILE');
    if (defined($js_setting) and defined($js_path)
        and ($js_setting eq 'generate' or $js_setting eq 'reference')) {
      $pre_body_close .=
        '<a href="'.$self->url_protect_url_text($js_path).'" rel="jslicense"><small>'
        .$self->convert_tree($self->gdt('JavaScript license information'))
        .'</small></a>';
    }
  }

  return "${program_text}

$pre_body_close
</body>
</html>
";
}

sub _root_html_element_attributes_string($)
{
  my $self = shift;
  if (defined($self->get_conf('HTML_ROOT_ELEMENT_ATTRIBUTES'))
      and $self->get_conf('HTML_ROOT_ELEMENT_ATTRIBUTES') ne '') {
    return ' '.$self->get_conf('HTML_ROOT_ELEMENT_ATTRIBUTES');
  }
  return '';
}

# This is used for normal output files and other files, like
# redirection file headers.  $COMMAND is the tree element for
# a @node that is being output in the file.
sub _file_header_information($$;$)
{
  my $self = shift;
  my $command = shift;
  my $filename = shift;

  my $title;
  if ($command) {
    my $command_string = $self->command_text($command, 'string');
    if (defined($command_string)
        and $command_string ne $self->get_info('title_string')) {
      my $element_tree;
      if ($self->get_conf('SECTION_NAME_IN_TITLE')
          and $command->{'extra'}
          and $command->{'extra'}->{'associated_section'}
          and $command->{'extra'}->{'associated_section'}->{'args'}
          and $command->{'extra'}->{'associated_section'}->{'args'}->[0]) {
        $element_tree = $command->{'extra'}->{'associated_section'}->{'args'}->[0];
      } else {
        $element_tree = $self->command_text($command, 'tree');
      }
      # TRANSLATORS: sectioning element title for the page header
      my $title_tree = $self->gdt('{element_text} ({title})',
                   { 'title' => $self->get_info('title_tree'),
                     'element_text' => $element_tree });
      $title = $self->convert_tree_new_formatting_context(
          {'type' => '_string', 'contents' => [$title_tree]},
          $command->{'cmdname'}, 'element_title');
    }
  }
  $title = $self->get_info('title_string') if (!defined($title));

  my $description = $self->get_info('documentdescription_string');
  $description = $title
    if (not defined($description) or $description eq '');
  $description = $self->close_html_lone_element(
    "<meta name=\"description\" content=\"$description\"" )
      if ($description ne '');
  my $encoding = '';
  $encoding
     = $self->close_html_lone_element(
        "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=".
          $self->get_conf('OUTPUT_ENCODING_NAME')."\"" )
    if (defined($self->get_conf('OUTPUT_ENCODING_NAME'))
        and ($self->get_conf('OUTPUT_ENCODING_NAME') ne ''));

  my $date = '';
  if ($self->get_conf('DATE_IN_HEADER')) {
    my $today = $self->convert_tree_new_formatting_context(
            {'cmdname' => 'today'}, 'DATE_IN_HEADER');
    $date =
      $self->close_html_lone_element(
        "<meta name=\"date\" content=\"$today\"")."\n";
  }

  my $css_lines = &{$self->formatting_function('format_css_lines')}($self,
                                                                  $filename);

  my $doctype = $self->get_conf('DOCTYPE');
  my $root_html_element_attributes = $self->_root_html_element_attributes_string();
  my $bodytext = $self->get_conf('BODYTEXT');
  if ($self->get_conf('HTML_MATH') and $self->get_conf('HTML_MATH') eq 'mathjax'
      and $self->get_file_information('mathjax', $filename)) {
    $bodytext .= ' class="tex2jax_ignore"';
  }
  my $copying_comment = $self->get_info('copying_comment');
  $copying_comment = ''
       if (not defined($copying_comment));
  my $after_body_open = '';
  $after_body_open = $self->get_conf('AFTER_BODY_OPEN')
    if (defined($self->get_conf('AFTER_BODY_OPEN')));
  my $extra_head = '';
  $extra_head = $self->get_conf('EXTRA_HEAD')
    if (defined($self->get_conf('EXTRA_HEAD')));
  my $program_and_version = $self->get_conf('PACKAGE_AND_VERSION');
  my $program_homepage = $self->get_conf('PACKAGE_URL');
  my $program = $self->get_conf('PROGRAM');
  my $generator = '';
  if (defined($program) and $program ne '') {
    $generator =
      $self->close_html_lone_element(
        "<meta name=\"Generator\" content=\"$program\"") . "\n";
  }

  if (defined($self->get_conf('INFO_JS_DIR'))) {
    if (!$self->get_conf('SPLIT')) {
      $self->document_error($self,
        sprintf(__("%s not meaningful for non-split output"),
                   'INFO_JS_DIR'));
    } else {
      my $jsdir = $self->get_conf('INFO_JS_DIR');
      if ($jsdir eq '.') {
        $jsdir = '';
      } else {
        $jsdir =~ s,/*$,/,; # append a single slash
      }

      $extra_head .= $self->close_html_lone_element(
        '<link rel="stylesheet" type="text/css" href="'.
                     $self->url_protect_url_text($jsdir).'info.css"')."\n".
'<script src="'.$self->url_protect_url_text($jsdir)
                      .'modernizr.js" type="text/javascript"></script>
<script src="'.$self->url_protect_url_text($jsdir)
                      .'info.js" type="text/javascript"></script>';
    }
  }
  if ((defined($self->get_conf('HTML_MATH'))
       and $self->get_conf('HTML_MATH') eq 'mathjax')
      and ($self->get_file_information('mathjax', $filename)
            # FIXME do we really want the script element if no math was seen?
            or !$self->get_conf('SPLIT'))) {
    my $mathjax_script = $self->get_conf('MATHJAX_SCRIPT');

    $extra_head .=
"<script type='text/javascript'>
MathJax = {
  options: {
    skipHtmlTags: {'[-]': ['pre']},
    ignoreHtmlClass: 'tex2jax_ignore',
    processHtmlClass: 'tex2jax_process'
  },
};
</script>"
.'<script type="text/javascript" id="MathJax-script" async
  src="'.$self->url_protect_url_text($mathjax_script).'">
</script>';

  }

  return ($title, $description, $encoding, $date, $css_lines,
          $doctype, $root_html_element_attributes, $bodytext, $copying_comment,
          $after_body_open, $extra_head, $program_and_version, $program_homepage,
          $program, $generator);
}

sub _get_links($$$$)
{
  my $self = shift;
  my $filename = shift;
  my $element = shift;
  my $node_command = shift;

  my $links = '';
  if ($self->get_conf('USE_LINKS')) {
    my $link_buttons = $self->get_conf('LINKS_BUTTONS');
    foreach my $link (@$link_buttons) {
      my $link_href = $self->from_element_direction($link, 'href', $element,
                                                    $filename, $node_command);
      #print STDERR "$link -> $link_href \n";
      if ($link_href and $link_href ne '') {
        my $link_string = $self->from_element_direction($link, 'string',
                                                        $element);
        my $link_title = '';
        $link_title = " title=\"$link_string\"" if (defined($link_string));
        my $rel = '';
        my $button_rel = $self->direction_string($link, 'rel', 'string');
        $rel = " rel=\"".$button_rel.'"' if (defined($button_rel));
        $links .= $self->close_html_lone_element(
                    "<link href=\"$link_href\"${rel}${link_title}")."\n";
      }
    }
  }
  return $links;
}

sub _default_format_begin_file($$$)
{
  my $self = shift;
  my $filename = shift;
  my $element = shift;

  my ($element_command, $node_command, $command_for_title);
  if ($element) {
    $element_command = $self->tree_unit_element_command($element);
    $node_command = $element_command;
    if ($element_command and $element_command->{'cmdname'}
        and $element_command->{'cmdname'} ne 'node'
        and $element_command->{'extra'}
        and $element_command->{'extra'}->{'associated_node'}) {
      $node_command = $element_command->{'extra'}->{'associated_node'};
    }

    $command_for_title = $element_command if ($self->get_conf('SPLIT'));
  }

  my ($title, $description, $encoding, $date, $css_lines,
          $doctype, $root_html_element_attributes, $bodytext, $copying_comment,
          $after_body_open, $extra_head, $program_and_version, $program_homepage,
          $program, $generator) = $self->_file_header_information($command_for_title,
                                                                  $filename);

  my $links = $self->_get_links($filename, $element, $node_command);

  my $result = "$doctype
<html${root_html_element_attributes}>
<!-- Created by $program_and_version, $program_homepage -->
<head>
$encoding
$copying_comment<title>$title</title>

$description\n".
    $self->close_html_lone_element(
      "<meta name=\"keywords\" content=\"$title\"")."\n".
    $self->close_html_lone_element(
      "<meta name=\"resource-type\" content=\"document\"")."\n".
     $self->close_html_lone_element(
      "<meta name=\"distribution\" content=\"global\"") . "\n" .
    ${generator} . ${date} .
    $self->close_html_lone_element(
      "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"")."\n".
"
${links}$css_lines
$extra_head
</head>

<body $bodytext>
$after_body_open";

  return $result;
}

sub _default_format_node_redirection_page($$)
{
  my $self = shift;
  my $command = shift;

  my ($title, $description, $encoding, $date, $css_lines,
          $doctype, $root_html_element_attributes, $bodytext, $copying_comment,
          $after_body_open, $extra_head, $program_and_version, $program_homepage,
          $program, $generator) = $self->_file_header_information($command);

  my $name = $self->command_text($command);
  my $href = $self->command_href($command);
  my $direction = "<a href=\"$href\">$name</a>";
  my $string = $self->convert_tree(
    $self->gdt('The node you are looking for is at {href}.',
      { 'href' => {'type' => '_converted', 'text' => $direction }}));
  my $result = "$doctype
<html${root_html_element_attributes}>
<!-- Created by $program_and_version, $program_homepage -->
<!-- This file redirects to the location of a node or anchor -->
<head>
$encoding
$copying_comment<title>$title</title>

$description\n".
   $self->close_html_lone_element(
     "<meta name=\"keywords\" content=\"$title\"")."\n".
   $self->close_html_lone_element(
     "<meta name=\"resource-type\" content=\"document\"")."\n".
   $self->close_html_lone_element(
     "<meta name=\"distribution\" content=\"global\"") . "\n" .
   ${generator} . ${date} . "$css_lines\n".
   $self->close_html_lone_element(
     "<meta http-equiv=\"Refresh\" content=\"0; url=$href\"")."\n".
   $self->close_html_lone_element(
     "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"")."\n".
"$extra_head
</head>

<body $bodytext>
$after_body_open
<p>$string</p>
</body>
";
  return $result;
}

sub _default_format_footnotes_sequence($)
{
  my $self = shift;

  my @pending_footnotes = $self->get_pending_footnotes();
  my $result = '';
  foreach my $pending_footnote_info_array (@pending_footnotes) {
    my ($command, $footid, $docid, $number_in_doc,
        $footnote_location_filename, $multi_expanded_region)
          = @$pending_footnote_info_array;
    my $footnote_location_href = $self->footnote_location_href($command, undef,
                                           $docid, $footnote_location_filename);
    # NOTE the @-commands in @footnote that are formatted differently depending
    # on $self->in_multi_expanded() cannot know that the original context
    # of the @footnote in the main document was $multi_expanded_region.
    # We do not want to set multi_expanded in customizable code.  However, it
    # could be possible to set a shared_conversion_state based on $multi_expanded_region
    # and have all the conversion functions calling $self->in_multi_expanded()
    # also check the shared_conversion_state.  The special situations
    # with those @-commands in @footnote in multi expanded
    # region do not justify this additional code and complexity.  The consequences
    # should only be redundant anchors HTML elements.
    my $footnote_text
        = $self->convert_tree_new_formatting_context($command->{'args'}->[0],
                              "$command->{'cmdname'} $number_in_doc $footid");
    chomp ($footnote_text);
    $footnote_text .= "\n";

    $result .= $self->html_attribute_class('h5', ['footnote-body-heading']) . '>'.
     "<a id=\"$footid\" href=\"$footnote_location_href\">($number_in_doc)</a></h5>\n"
     . $footnote_text;
  }
  return $result;
}

sub _default_format_footnotes_segment($)
{
  my $self = shift;
  my $foot_lines
    = &{$self->formatting_function('format_footnotes_sequence')}($self);
  return '' if ($foot_lines eq '');
  my $class = $self->special_element_info('class', 'footnotes');
  my $result = $self->html_attribute_class('div', [$class.'-segment']).">\n";
  $result .= $self->get_conf('DEFAULT_RULE') . "\n"
     if (defined($self->get_conf('DEFAULT_RULE'))
         and $self->get_conf('DEFAULT_RULE') ne '');
  my $footnote_heading_tree = $self->special_element_info('heading_tree',
                                                          'footnotes');
  my $footnote_heading;
  if (defined($footnote_heading_tree)) {
    $footnote_heading
      = $self->convert_tree($footnote_heading_tree,
                            'convert footnotes special heading');
  } else {
    $footnote_heading = '';
  }
  my $level = $self->get_conf('FOOTNOTE_END_HEADER_LEVEL');
  $result .= &{$self->formatting_function('format_heading_text')}($self, undef,
                          [$class.'-heading'], $footnote_heading, $level)."\n";
  $result .= $foot_lines;
  $result .= "</div>\n";
  return $result;
}

sub _default_format_special_body_about($$$)
{
  my $self = shift;
  my $special_type = shift;
  my $element = shift;

  my $about = '';
  if ($self->get_conf('PROGRAM_NAME_IN_ABOUT')) {
    $about .= "<p>\n";
    $about .= '  '.&{$self->formatting_function('format_program_string')}($self) ."\n";
    $about .= "</p>\n";
  }
  $about .= <<EOT;
<p>
EOT
  $about .= $self->convert_tree(
    $self->gdt('  The buttons in the navigation panels have the following meaning:'))
            . "\n";
  $about .= <<EOT;
</p>
<table border="1">
  <tr>
EOT
   # TRANSLATORS: direction column header in the navigation help
  $about .= '    <th> ' . $self->convert_tree($self->gdt('Button')) . " </th>\n" .
   # TRANSLATORS: button label column header in the navigation help
   '    <th> ' . $self->convert_tree($self->gdt('Name')) . " </th>\n" .
   # TRANSLATORS: direction description column header in the navigation help
   '    <th> ' . $self->convert_tree($self->gdt('Go to')) . " </th>\n" .
   # TRANSLATORS: section reached column header in the navigation help
   '    <th> ' . $self->convert_tree($self->gdt('From 1.2.3 go to')) . "</th>\n"
 . "  </tr>\n";

  foreach my $button_spec (@{$self->get_conf('SECTION_BUTTONS')}) {
    next if ($button_spec eq ' ' or ref($button_spec) eq 'CODE'
             or ref($button_spec) eq 'SCALAR'
             or (ref($button_spec) eq 'ARRAY' and scalar(@$button_spec) != 2));
    my $button;
    if (ref($button_spec) eq 'ARRAY') {
      $button = $button_spec->[0];
    } else {
      $button = $button_spec;
    }
    $about .= "  <tr>\n    ".$self->html_attribute_class('td',
                                          ['button-direction-about']) .'>';
    # if the button spec is an array we do not knwow what the button
    # looks like, so we do not show the button but still show explanations.
    if (ref($button_spec) ne 'ARRAY') {
      my $button_name_string
          = $self->direction_string($button, 'button', 'string');
      # FIXME strip FirstInFile from $button to get active icon file?
      $about .=
        (($self->get_conf('ICONS') &&
           $self->get_conf('ACTIVE_ICONS')->{$button}) ?
            &{$self->formatting_function('format_button_icon_img')}($self,
               $button_name_string, $self->get_conf('ACTIVE_ICONS')->{$button})
        : ' [' . $self->direction_string($button, 'text') . '] ');
    }
    $about .= "</td>\n";
    my $button_name
          = $self->direction_string($button, 'button');
    $about .=
'    '.$self->html_attribute_class('td', ['name-direction-about']).'>'
    .$button_name."</td>
    <td>".$self->direction_string($button, 'description')."</td>
    <td>".$self->direction_string($button, 'example')."</td>
  </tr>
";
  }

  $about .= <<EOT;
</table>

<p>
EOT
  $about .= $self->convert_tree($self->gdt('  where the @strong{ Example } assumes that the current position is at @strong{ Subsubsection One-Two-Three } of a document of the following structure:')) . "\n";

#  where the <strong> Example </strong> assumes that the current position
#  is at <strong> Subsubsection One-Two-Three </strong> of a document of
#  the following structure:
  $about .= <<EOT;
</p>

<ul>
EOT
  my $non_breaking_space = $self->get_info('non_breaking_space');
  # TRANSLATORS: example name of section for section 1
  $about .= '  <li> 1. ' . $self->convert_tree($self->gdt('Section One')) . "\n" .
"    <ul>\n" .
       # TRANSLATORS: example name of section for section 1.1
'      <li>1.1 ' . $self->convert_tree($self->gdt('Subsection One-One')) . "\n";
  $about .= <<EOT;
        <ul>
          <li>...</li>
        </ul>
      </li>
EOT
                 # TRANSLATORS: example name of section for section 1.2
  $about .= '      <li>1.2 ' . $self->convert_tree($self->gdt('Subsection One-Two')) . "\n" .
"        <ul>\n" .
                 # TRANSLATORS: example name of section for section 1.2.1
'          <li>1.2.1 ' . $self->convert_tree($self->gdt('Subsubsection One-Two-One')) . "</li>\n" .
                 # TRANSLATORS: example name of section for section 1.2.2
'          <li>1.2.2 ' . $self->convert_tree($self->gdt('Subsubsection One-Two-Two')) . "</li>\n" .
                 # TRANSLATORS: example name of section for section 1.2.3
'          <li>1.2.3 ' . $self->convert_tree($self->gdt('Subsubsection One-Two-Three'))
                  . " $non_breaking_space $non_breaking_space\n"
.
'            <strong>&lt;== ' . $self->convert_tree($self->gdt('Current Position')) . " </strong></li>\n" .
                 # TRANSLATORS: example name of section for section 1.2.3
'          <li>1.2.4 ' . $self->convert_tree($self->gdt('Subsubsection One-Two-Four')) . "</li>\n" .
"        </ul>\n" .
"      </li>\n" .
                 # TRANSLATORS: example name of section for section 1.3
'      <li>1.3 ' . $self->convert_tree($self->gdt('Subsection One-Three')) . "\n";
  $about .= <<EOT;
        <ul>
          <li>...</li>
        </ul>
      </li>
EOT
                 # TRANSLATORS: example name of section for section 1.4
  $about .= '      <li>1.4 ' . $self->convert_tree($self->gdt('Subsection One-Four')) . "</li>\n";

  $about .= <<EOT;
    </ul>
  </li>
</ul>
EOT
  return $about;
}

sub _default_format_special_body_contents($$$)
{
  my $self = shift;
  my $special_type = shift;
  my $element = shift;

  return &{$self->formatting_function('format_contents')}($self, 'contents');
}

sub _default_format_special_body_shortcontents($$$)
{
  my $self = shift;
  my $special_type = shift;
  my $element = shift;

  return &{$self->formatting_function('format_contents')}($self, 'shortcontents');
}

sub _default_format_special_body_footnotes($$$)
{
  my $self = shift;
  my $special_type = shift;
  my $element = shift;

  return &{$self->formatting_function('format_footnotes_sequence')}($self);
}

sub _do_jslicenses_file {
  my $self = shift;
  my $destination_directory = shift;

  my $setting = $self->get_conf('JS_WEBLABELS');
  my $path = $self->get_conf('JS_WEBLABELS_FILE');

  # Possible settings:
  #   'generate' - create file at JS_WEBLABELS_FILE
  #   'reference' - reference file at JS_WEBLABELS_FILE but do not create it
  #   'omit' - do nothing
  return if (!$setting or $setting ne 'generate');

  my $doctype = $self->get_conf('DOCTYPE');
  my $root_html_element_attributes = $self->_root_html_element_attributes_string();
  my $a = $doctype . "\n" .
"<html${root_html_element_attributes}>".'<head><title>jslicense labels</title></head>
<body>
<table id="jslicense-labels1">
';

  my $jslicenses = $self->get_info('jslicenses');
  foreach my $category (sort(keys %$jslicenses)) {
    foreach my $file (sort(keys %{$jslicenses->{$category}})) {
      my $file_info = $jslicenses->{$category}->{$file};
      $a .= "<tr>\n";
      $a .= '<td><a href="'.$self->url_protect_url_text($file)."\">$file</a></td>\n";
      $a .= '<td><a href="'.$self->url_protect_url_text($file_info->[1])
                                         ."\">$file_info->[0]</a></td>\n";
      $a .= '<td><a href="'.$self->url_protect_url_text($file_info->[2])
                                         ."\">$file_info->[2]</a></td>\n";
      $a .= "</tr>\n";
    }
  }

  $a .= "</table>\n</body></html>\n";

  if (File::Spec->file_name_is_absolute($path) or $path =~ /^[A-Za-z]*:/) {
    $self->document_warn($self, sprintf(
__("cannot use absolute path or URL `%s' for JS_WEBLABELS_FILE when generating web labels file"), $path));
    return;
  }
  my $license_file = File::Spec->catdir($destination_directory,
                                        $path);
  # sequence of bytes
  my ($licence_file_path, $path_encoding)
     = $self->encoded_output_file_name($license_file);
  my ($fh, $error_message_licence_file)
         = Texinfo::Common::output_files_open_out(
                         $self->output_files_information(), $self,
                         $licence_file_path);
  if (defined($fh)) {
    print $fh $a;
    Texinfo::Common::output_files_register_closed(
                  $self->output_files_information(), $licence_file_path);
    if (!close ($fh)) {
      $self->document_error($self,
               sprintf(__("error on closing %s: %s"),
                                    $license_file, $!));
    }
  } else {
    $self->document_error($self,
           sprintf(__("could not open %s for writing: %s"),
                   $license_file, $error_message_licence_file));
  }
}

# FIXME the file opening should be done in main program, only
# the formatting should be done in customization function.  Frames
# are deprecated in HTML, however, and therefore there is no point
# in investing time in better code to produce them.
sub _default_format_frame_files($$)
{
  my $self = shift;
  my $destination_directory = shift;

  my $frame_file = $self->{'frame_pages_filenames'}->{'Frame'};
  my $frame_outfile;
  if (defined($destination_directory) and $destination_directory ne '') {
    $frame_outfile = File::Spec->catfile($destination_directory,
                                         $frame_file);
  } else {
    $frame_outfile = $frame_file;
  }

  my $toc_frame_file = $self->{'frame_pages_filenames'}->{'Toc_Frame'};
  my $toc_frame_outfile;
  if (defined($destination_directory) and $destination_directory ne '') {
    $toc_frame_outfile = File::Spec->catfile($destination_directory,
                                             $toc_frame_file);
  } else {
    $toc_frame_outfile = $toc_frame_file;
  }
  # sequence of bytes
  my ($frame_file_path, $frame_path_encoding)
     = $self->encoded_output_file_name($frame_outfile);
  my ($frame_fh, $error_message_frame) = Texinfo::Common::output_files_open_out(
                     $self->output_files_information(), $self, $frame_file_path);
  if (defined($frame_fh)) {
    my $doctype = $self->get_conf('FRAMESET_DOCTYPE');
    my $root_html_element_attributes = $self->_root_html_element_attributes_string();
    my $top_file = '';
    my $top_element = $self->global_direction_element('Top');
    if ($top_element) {
      $top_file = $top_element->{'structure'}->{'unit_filename'};
    }
    my $title = $self->{'title_string'};
    print $frame_fh <<EOT;
$doctype
<html${root_html_element_attributes}>
<head><title>$title</title></head>
<frameset cols="140,*">
  <frame name="toc" src="$toc_frame_file">
  <frame name="main" src="$top_file">
</frameset>
</html>
EOT

    Texinfo::Common::output_files_register_closed(
                  $self->output_files_information(), $frame_file_path);
    if (!close ($frame_fh)) {
      $self->document_error($self,
          sprintf(__("error on closing frame file %s: %s"),
                                    $frame_outfile, $!));
      return 0;
    }
  } else {
    $self->document_error($self,
           sprintf(__("could not open %s for writing: %s"),
                                  $frame_outfile, $error_message_frame));
    return 0;
  }
  # sequence of bytes
  my ($toc_frame_path, $toc_frame_path_encoding)
       = $self->encoded_output_file_name($toc_frame_outfile);
  my ($toc_frame_fh, $toc_frame_error_message)
           = Texinfo::Common::output_files_open_out(
                               $self->output_files_information(), $self,
                               $toc_frame_path);
  if (defined($toc_frame_fh)) {

    # this is needed to collect CSS rules.
    $self->{'current_filename'} = $toc_frame_file;
    my $shortcontents =
      &{$self->formatting_function('format_contents')}($self, 'shortcontents');
    $shortcontents =~ s/\bhref=/target="main" href=/g;
    my $header = &{$self->formatting_function('format_begin_file')}($self,
                                                        $toc_frame_file, undef);
    print $toc_frame_fh $header;
    print $toc_frame_fh '<h2>Content</h2>'."\n";
    print $toc_frame_fh $shortcontents;
    print $toc_frame_fh "</body></html>\n";

    $self->{'current_filename'} = undef;

    Texinfo::Common::output_files_register_closed(
                  $self->output_files_information(), $toc_frame_path);
    if (!close ($toc_frame_fh)) {
      $self->document_error($self,
            sprintf(__("error on closing TOC frame file %s: %s"),
                                    $toc_frame_outfile, $!));
      return 0;
    }
  } else {
    $self->document_error($self,
           sprintf(__("could not open %s for writing: %s"),
                   $toc_frame_outfile, $toc_frame_error_message));
    return 0;
  }
  return 1;
}

sub _has_contents_or_shortcontents($)
{
  my $self = shift;
  my $global_commands = $self->get_info('global_commands');
  foreach my $cmdname ('contents', 'shortcontents') {
    if ($global_commands and $global_commands->{$cmdname}) {
      return 1;
    }
  }
  return 0;
}

# to be called before starting conversion.
# NOTE not called directly by convert_tree, which means that convert_tree
# needs to be called from a converter which would have had this function
# called already.
sub _initialize_output_state($)
{
  my $self = shift;

  # for diverse API used in conversion
  $self->{'shared_conversion_state'} = {};

  $self->{'associated_inline_content'} = {};

  # even if there is no actual file, this is needed if the API is used.
  $self->{'files_information'} = {};

  # Needed for CSS gathering, even if nothing related to CSS is output
  $self->{'document_global_context_css'} = {};
  $self->{'file_css'} = {};

  # direction strings
  foreach my $string_type (keys(%default_translated_directions_strings)) {
    # those will be determined from translatable strings
    $self->{'directions_strings'}->{$string_type} = {};
  };

  # targets and directions

  # used for diverse elements: tree units, indices, footnotes, special
  # elements, contents elements...
  $self->{'targets'} = {};
  $self->{'seen_ids'} = {};

  # to avoid infinite recursions when a section refers to itself, possibly
  # indirectly
  $self->{'referred_command_stack'} = [];

  # for directions to special elements, only filled if special
  # elements are actually used.
  $self->{'special_elements_directions'} = {};

  # for footnotes
  $self->{'special_targets'} = {'footnote_location' => {}};

  # other

  $self->{'check_htmlxref_already_warned'} = {}
    if ($self->get_conf('CHECK_HTMLXREF'));
}

sub convert($$)
{
  my $self = shift;
  my $root = shift;

  my $result = '';

  $self->_initialize_output_state();

  # needed for CSS rules gathering
  $self->{'current_filename'} = '';

  # call before _prepare_conversion_tree_units, which calls _translate_names.
  # Some information is not available yet.
  $self->_reset_info();

  my ($tree_units, $special_elements)
    = $self->_prepare_conversion_tree_units($root, undef, undef);

  $self->_prepare_index_entries();
  $self->_prepare_footnotes();

  # title
  $self->{'title_titlepage'}
    = &{$self->formatting_function('format_title_titlepage')}($self);

  # complete information should be available.
  $self->_reset_info();

  if (!defined($tree_units)) {
    print STDERR "\nC NO UNIT\n" if ($self->get_conf('DEBUG'));
    $result = $self->_convert($root, 'convert no unit');
    $result .= &{$self->formatting_function('format_footnotes_segment')}($self);
  } else {
    my $unit_nr = 0;
    # TODO there is no rule before the footnotes special element in
    # case of separate footnotes in the default formatting style.
    # Not sure if it is an issue.
    foreach my $tree_unit (@$tree_units, @$special_elements) {
      print STDERR "\nC UNIT $unit_nr\n" if ($self->get_conf('DEBUG'));
      my $tree_unit_text = $self->_convert($tree_unit, "convert unit $unit_nr");
      $result .= $tree_unit_text;
      $unit_nr++;
    }
  }

  return $result;
}

# This is called from the main program on the converter.
sub output_internal_links($)
{
  my $self = shift;
  my $out_string = '';
  if ($self->{'tree_units'}) {
    foreach my $tree_unit (@{$self->{'tree_units'}}) {
      my $text;
      my $href;
      my $command = $self->tree_unit_element_command($tree_unit);
      if (defined($command)) {
        # Use '' for filename, to force a filename in href.
        $href = $self->command_href($command, '');
        my $tree = $self->command_text($command, 'tree');
        if ($tree) {
          $text = Texinfo::Convert::Text::convert_to_text($tree,
             {Texinfo::Convert::Text::copy_options_for_convert_text($self)});
        }
      }
      if (defined($href) or defined($text)) {
        $out_string .= $href if (defined($href));
        $out_string .= "\ttoc\t";
        $out_string .= $text if (defined($text));
        $out_string .= "\n";
      }
    }
  }
  my $index_entries_by_letter = $self->get_info('index_entries_by_letter');
  if ($index_entries_by_letter) {
    my %options = Texinfo::Convert::Text::copy_options_for_convert_text($self);
    foreach my $index_name (sort(keys (%{$index_entries_by_letter}))) {
      foreach my $letter_entry (@{$index_entries_by_letter->{$index_name}}) {
        foreach my $index_entry (@{$letter_entry->{'entries'}}) {
          my $main_entry_element = $index_entry->{'entry_element'};
          my $in_code
            = $self->{'indices_information'}->{$index_entry->{'index_name'}}
                                                                 ->{'in_code'};
          # does not refer to the document
          next if ($main_entry_element->{'extra'}
                   and ($main_entry_element->{'extra'}->{'seeentry'}
                        or $main_entry_element->{'extra'}->{'seealso'}));
          my $href;
          $href = $self->command_href($main_entry_element, '');
          # Obtain term by converting to text
          my $converter_options = {%options};
          $converter_options->{'code'} = $in_code;
          my $entry_reference_content_element
            = Texinfo::Common::index_content_element($main_entry_element);
          my @contents = ($entry_reference_content_element);
          my $subentries_tree
            = $self->comma_index_subentries_tree($main_entry_element);
          if (defined($subentries_tree)) {
            push @contents, @{$subentries_tree->{'contents'}};
          }
          my $index_term = Texinfo::Convert::Text::convert_to_text(
                               {'contents' => \@contents}, $converter_options);
          if (defined($index_term) and $index_term =~ /\S/) {
            $out_string .= $href if (defined($href));
            $out_string .= "\t$index_name\t";
            $out_string .= $index_term;
            $out_string .= "\n";
          }
        }
      }
    }
  }
  if ($out_string ne '') {
    return $out_string;
  } else {
    return undef;
  }
}

sub run_stage_handlers($$$)
{
  my $converter = shift;
  my $root = shift;
  my $stage = shift;

  my $stage_handlers = Texinfo::Config::GNUT_get_stage_handlers();
  return 0 if (!defined($stage_handlers->{$stage}));

  my @sorted_priorities = sort keys(%{$stage_handlers->{$stage}});
  foreach my $priority (@sorted_priorities) {
    foreach my $handler (@{$stage_handlers->{$stage}->{$priority}}) {
      if ($converter->get_conf('DEBUG')) {
        print STDERR "HANDLER($stage) , priority $priority: $handler\n";
      }
      my $status = &{$handler}($converter, $root, $stage);
      if ($status != 0) {
        if ($status < 0) {
          $converter->document_error($converter,
             sprintf(__("handler %s of stage %s priority %s failed"),
                        $handler, $stage, $priority));
        } else {
          # the handler is supposed to have output an error message
          # already if $status > 0
          if ($converter->get_conf('VERBOSE') or $converter->get_conf('DEBUG')) {
            print STDERR "Handler $handler of $stage($priority) failed\n";
          }
        }
        return $status;
      }
    }
  }
  return 0;
}

sub _reset_info()
{
  my $self = shift;

  # reset to be sure that there is no stale information
  $self->{'converter_info'} = {};
  foreach my $converter_info (keys(%available_converter_info)) {
    if (exists($self->{$converter_info})) {
      if (ref($self->{$converter_info}) eq '') {
        # for scalar, use references in case it may change
        $self->{'converter_info'}->{$converter_info} = \$self->{$converter_info};
      } else {
        $self->{'converter_info'}->{$converter_info} = $self->{$converter_info};
      }
    }
  }
}

# Main function for outputting a manual in HTML.
# $SELF is the output converter object of class Texinfo::Convert::HTML (this
# module), and $ROOT is the Texinfo tree from the parser.
sub output($$)
{
  my $self = shift;
  my $root = shift;

  $self->{'current_filename'} = undef;

  $self->_initialize_output_state();

  # no splitting when writing to the null device or to stdout or returning
  # a string
  if (defined($self->get_conf('OUTFILE'))
      and ($Texinfo::Common::null_device_file{$self->get_conf('OUTFILE')}
           or $self->get_conf('OUTFILE') eq '-'
           or $self->get_conf('OUTFILE') eq '')) {
    $self->force_conf('SPLIT', 0);
    $self->force_conf('MONOLITHIC', 1);
    $self->force_conf('FRAMES', 0);
  }
  if ($self->get_conf('SPLIT')) {
    $self->set_conf('NODE_FILES', 1);
  }
  if ($self->get_conf('FRAMES')) {
    $self->set_conf('shortcontents', 1);
  }
  $self->set_conf('EXTERNAL_CROSSREF_SPLIT', $self->get_conf('SPLIT'));

  if (not defined($self->get_conf('NODE_NAME_IN_INDEX'))) {
    $self->set_conf('NODE_NAME_IN_INDEX', $self->get_conf('USE_NODES'));
  }

  my $handler_fatal_error_level = $self->get_conf('HANDLER_FATAL_ERROR_LEVEL');

  if ($self->get_conf('HTML_MATH')
        and $self->get_conf('HTML_MATH') eq 'mathjax') {
    # See https://www.gnu.org/licenses/javascript-labels.html
    #
    # The link to the source for mathjax does not strictly follow the advice
    # there: instead we link to instructions for obtaining the full source in
    # its preferred form of modification.

    my $mathjax_script = $self->get_conf('MATHJAX_SCRIPT');
    if (! defined($mathjax_script)) {
      $mathjax_script = 'https://cdn.jsdelivr.net/npm/mathjax@3/es5/tex-svg.js';
      $self->set_conf('MATHJAX_SCRIPT', $mathjax_script);
    }

    my $mathjax_source = $self->get_conf('MATHJAX_SOURCE');
    if (! defined($mathjax_source)) {
      $mathjax_source = 'http://docs.mathjax.org/en/latest/web/hosting.html#getting-mathjax-via-git';
      $self->set_conf('MATHJAX_SOURCE', $mathjax_source);
    }
  }

  if ($self->get_conf('HTML_MATH')
      and not defined($self->get_conf('CONVERT_TO_LATEX_IN_MATH'))) {
    $self->set_conf('CONVERT_TO_LATEX_IN_MATH', 1);
  }

  if ($self->get_conf('CONVERT_TO_LATEX_IN_MATH')) {
    $self->{'options_latex_math'}
     = { Texinfo::Convert::LaTeX::copy_options_for_convert_to_latex_math($self) };
  }

  if ($self->get_conf('NO_TOP_NODE_OUTPUT')
      and not defined($self->get_conf('SHOW_TITLE'))) {
    $self->set_conf('SHOW_TITLE', 1);
  }

  # set information, to have some information for run_stage_handlers.
  # Some information is not available yet.
  $self->_reset_info();

  my $setup_status = $self->run_stage_handlers($root, 'setup');
  return undef unless ($setup_status < $handler_fatal_error_level
                       and $setup_status > -$handler_fatal_error_level);

  # the configuration has potentially been modified for
  # this output file especially.  Set a corresponding initial
  # configuration.
  $self->{'output_init_conf'} = { %{$self->{'conf'}} };

  $self->{'jslicenses'} = {};
  if ($self->get_conf('HTML_MATH')
        and $self->get_conf('HTML_MATH') eq 'mathjax') {
    # See https://www.gnu.org/licenses/javascript-labels.html

    my $mathjax_script = $self->get_conf('MATHJAX_SCRIPT');
    my $mathjax_source = $self->get_conf('MATHJAX_SOURCE');

    $self->{'jslicenses'}->{'mathjax'} = {
      $mathjax_script =>
        [ 'Apache License, Version 2.0.',
          'https://www.apache.org/licenses/LICENSE-2.0',
          $mathjax_source ]};
  }
  if ($self->get_conf('INFO_JS_DIR')) {
    $self->{'jslicenses'}->{'infojs'} = {
      'js/info.js' =>
         [ 'GNU General Public License 3.0 or later',
           'http://www.gnu.org/licenses/gpl-3.0.html',
           'js/info.js' ],
       'js/modernizr.js' =>
          [ 'Expat',
            'http://www.jclark.com/xml/copying.txt',
            'js/modernizr.js' ]};
  }
  $self->_prepare_css();

  # this sets OUTFILE, to be used if not split, but also
  # 'destination_directory' and 'output_filename' that are useful when split.
  my ($output_file, $destination_directory, $output_filename,
              $document_name) = $self->determine_files_and_directory();
  my ($encoded_destination_directory, $dir_encoding)
    = $self->encoded_output_file_name($destination_directory);
  my $succeeded
    = $self->create_destination_directory($encoded_destination_directory,
                                          $destination_directory);
  return undef unless $succeeded;

  # set for init files
  $self->{'document_name'} = $document_name;
  $self->{'destination_directory'} = $destination_directory;

  # set information, to have it available for the conversions below,
  # in translate_names called by _prepare_conversion_tree_units and in
  # titles formatting.
  # Some information is not available yet.
  $self->_reset_info();

  # Get the list of "elements" to be processed, i.e. nodes or sections.
  # This should return undef if called on a tree without node or sections.
  my ($tree_units, $special_elements)
    = $self->_prepare_conversion_tree_units($root, $destination_directory,
                                            $document_name);

  Texinfo::Structuring::split_pages($tree_units, $self->get_conf('SPLIT'));

  # determine file names associated with the different pages, and setup
  # the counters for special element pages.
  my %files_source_info;
  if ($output_file ne '') {
    %files_source_info =
      $self->_html_set_pages_files($tree_units, $special_elements, $output_file,
                    $destination_directory, $output_filename, $document_name);
  }

  $self->_prepare_contents_elements();

  # do tree units directions.
  Texinfo::Structuring::elements_directions($self, $self->{'labels'}, $tree_units);

  # do element directions related to files.
  # FIXME do it here or before?  Here it means that
  # PrevFile and NextFile can be set.
  Texinfo::Structuring::elements_file_directions($tree_units);

  # Associate the special elements that have no page with the main page.
  # This may only happen if not split.
  if ($special_elements
      and $tree_units and $tree_units->[0]
      and $tree_units->[0]->{'structure'}
      and defined($tree_units->[0]->{'structure'}->{'unit_filename'})) {
    foreach my $special_element (@$special_elements) {
      if (!defined($special_element->{'structure'}->{'unit_filename'})) {
        $special_element->{'structure'}->{'unit_filename'}
           = $tree_units->[0]->{'structure'}->{'unit_filename'};
        $self->{'file_counters'}->{$special_element->{'structure'}->{'unit_filename'}}++;
      }
    }
  }

  $self->_prepare_index_entries();
  $self->_prepare_footnotes();

  # only in HTML, not in Texinfo::Convert::Converter
  $self->{'elements_in_file_count'} = {};
  # condition could also be based on $output_file ne ''
  if ($self->{'file_counters'}) {
    # 'file_counters' is dynamic, decreased when the element is encountered
    # 'elements_in_file_count' is not modified afterwards
    foreach my $filename (keys(%{$self->{'file_counters'}})) {
      $self->{'elements_in_file_count'}->{$filename}
                            = $self->{'file_counters'}->{$filename};
    }
  }

  # set information, to have it ready for
  # run_stage_handlers.  Some information is not available yet.
  $self->_reset_info();

  my $structure_status = $self->run_stage_handlers($root, 'structure');
  return undef unless ($structure_status < $handler_fatal_error_level
                       and $structure_status > -$handler_fatal_error_level);

  my $default_document_language = $self->get_conf('documentlanguage');

  $self->set_global_document_commands('preamble', ['documentlanguage']);

  my $preamble_document_language = $self->get_conf('documentlanguage');
  $self->set_conf('BODYTEXT',
                  'lang="' . $preamble_document_language . '"');

  if ($default_document_language ne $preamble_document_language) {
    $self->_translate_names();
  }

  # prepare title.  fulltitle uses more possibility than simpletitle for
  # title, including @-commands found in @titlepage only.  Therefore
  # simpletitle is more in line with what makeinfo in C does.
  my $fulltitle;
  foreach my $fulltitle_command('settitle', 'title', 'shorttitlepage', 'top') {
    if ($self->{'global_commands'}->{$fulltitle_command}) {
      my $command = $self->{'global_commands'}->{$fulltitle_command};
      next if (!$command->{'args'}
               or (!$command->{'args'}->[0]->{'contents'}
                   or ($command->{'extra'}
                       and $command->{'extra'}->{'missing_argument'})));
      print STDERR "Using $fulltitle_command as title\n"
        if ($self->get_conf('DEBUG'));
      $fulltitle = {'contents' => $command->{'args'}->[0]->{'contents'}};
      last;
    }
  }
  if (!$fulltitle and $self->{'global_commands'}->{'titlefont'}
      and $self->{'global_commands'}->{'titlefont'}->[0]->{'args'}
      and defined($self->{'global_commands'}->{'titlefont'}->[0]->{'args'}->[0])
      and $self->{'global_commands'}->{'titlefont'}->[0]
                                                ->{'args'}->[0]->{'contents'}
      and @{$self->{'global_commands'}->{'titlefont'}->[0]
                                                ->{'args'}->[0]->{'contents'}}) {
    $fulltitle = $self->{'global_commands'}->{'titlefont'}->[0];
  }
  # prepare simpletitle
  foreach my $simpletitle_command ('settitle', 'shorttitlepage') {
    if ($self->{'global_commands'}->{$simpletitle_command}) {
      my $command = $self->{'global_commands'}->{$simpletitle_command};
      next if (!$command->{'args'}
               or !$command->{'args'}->[0]->{'contents'}
               or ($command->{'extra'}
                   and $command->{'extra'}->{'missing_argument'}));
      $self->{'simpletitle_tree'} =
         {'contents' => $command->{'args'}->[0]->{'contents'}};
      $self->{'simpletitle_command_name'} = $simpletitle_command;
      last;
    }
  }

  my $html_title_string;
  if ($fulltitle) {
    $self->{'title_tree'} = $fulltitle;
    $html_title_string = $self->convert_tree_new_formatting_context(
          {'type' => '_string', 'contents' => [$self->{'title_tree'}]},
          'title_string');
  }
  if (!defined($html_title_string) or $html_title_string !~ /\S/) {
    my $default_title = $self->gdt('Untitled Document');
    $self->{'title_tree'} = $default_title;
    $self->{'title_string'} = $self->convert_tree_new_formatting_context(
          {'type' => '_string', 'contents' => [$self->{'title_tree'}]},
          'title_string');
    $self->line_warn($self, __(
                         "must specify a title with a title command or \@top"),
                     {'file_name' => $self->{'parser_info'}->{'input_file_name'}});
  } else {
    $self->{'title_string'} = $html_title_string;
  }

  # copying comment
  if ($self->{'global_commands'}->{'copying'}) {
    my $copying_comment = Texinfo::Convert::Text::convert_to_text(
     {'contents' => $self->{'global_commands'}->{'copying'}->{'contents'}},
     {Texinfo::Convert::Text::copy_options_for_convert_text($self)});
    if ($copying_comment ne '') {
      $self->{'copying_comment'}
       = &{$self->formatting_function('format_comment')}($self, $copying_comment);
    }
  }
  $self->set_global_document_commands('before', ['documentlanguage']);

  if ($default_document_language ne $preamble_document_language) {
    $self->_translate_names();
  }

  # documentdescription
  if (defined($self->get_conf('documentdescription'))) {
    $self->{'documentdescription_string'}
      = $self->get_conf('documentdescription');
  } elsif ($self->{'global_commands'}->{'documentdescription'}) {
    $self->{'documentdescription_string'}
      = $self->convert_tree_new_formatting_context(
       {'type' => '_string',
        'contents' =>
            $self->{'global_commands'}->{'documentdescription'}->{'contents'}},
       'documentdescription');
    chomp($self->{'documentdescription_string'});
  }

  # set information, to have it ready for run_stage_handlers.
  # Some information is not available yet.
  $self->_reset_info();

  my $init_status = $self->run_stage_handlers($root, 'init');
  return undef unless ($init_status < $handler_fatal_error_level
                       and $init_status > -$handler_fatal_error_level);

  if ($self->get_conf('FRAMES')) {
    my $status = &{$self->formatting_function('format_frame_files')}($self,
                                                      $destination_directory);
    return undef if (!$status);
  }

  # determine first file name
  if (!$tree_units
      or !defined($tree_units->[0]->{'structure'}->{'unit_filename'})) {
    # no page
    if ($output_file ne '') {
      my $no_page_output_filename;
      my $no_page_out_filepath;
      if ($self->get_conf('SPLIT')) {
        $no_page_output_filename = $self->top_node_filename($document_name);
        if (defined($destination_directory) and $destination_directory ne '') {
          $no_page_out_filepath = File::Spec->catfile($destination_directory,
                                                    $no_page_output_filename);
        } else {
          $no_page_out_filepath = $no_page_output_filename;
        }
      } else {
        $no_page_out_filepath = $output_file;
        $no_page_output_filename = $output_filename;
      }
      $self->{'out_filepaths'}->{$no_page_output_filename} = $no_page_out_filepath;

      $self->{'current_filename'} = $no_page_output_filename;
    } else {
      $self->{'current_filename'} = $output_filename;
    }
  } else {
    $self->{'current_filename'}
      = $tree_units->[0]->{'structure'}->{'unit_filename'};
  }
  # title
  $self->{'title_titlepage'}
    = &{$self->formatting_function('format_title_titlepage')}($self);

  # complete information should be available.
  $self->_reset_info();

  my $output = '';

  if (!$tree_units or !$tree_units->[0]->{'structure'}
      or !defined($tree_units->[0]->{'structure'}->{'unit_filename'})) {
    my $fh;
    my $encoded_no_page_out_filepath;
    my $no_page_out_filepath;
    if ($self->{'current_filename'} ne ''
        and $self->{'out_filepaths'}
        and defined($self->{'out_filepaths'}->{$self->{'current_filename'}})) {
      my $path_encoding;
      $no_page_out_filepath
         = $self->{'out_filepaths'}->{$self->{'current_filename'}};
      ($encoded_no_page_out_filepath, $path_encoding)
        = $self->encoded_output_file_name($no_page_out_filepath);
      my $error_message;
      ($fh, $error_message) = Texinfo::Common::output_files_open_out(
                                 $self->output_files_information(), $self,
                                 $encoded_no_page_out_filepath);
      if (!$fh) {
        $self->document_error($self,
              sprintf(__("could not open %s for writing: %s"),
                                      $no_page_out_filepath, $error_message));
        return undef;
      }
    }
    my $body = '';
    if ($tree_units and @$tree_units) {
      my $unit_nr = 0;
      # TODO there is no rule before the footnotes special element in
      # case of separate footnotes in the default formatting style.
      # Not sure if it is an issue.
      foreach my $tree_unit (@$tree_units, @$special_elements) {
        print STDERR "\nUNIT NO-PAGE $unit_nr\n" if ($self->get_conf('DEBUG'));
        my $tree_unit_text
          = $self->_convert($tree_unit, "no-page output unit $unit_nr");
        $body .= $tree_unit_text;
        $unit_nr++;
      }
    } else {
      $body .= $self->get_info('title_titlepage');
      print STDERR "\nNO UNIT NO PAGE\n" if ($self->get_conf('DEBUG'));
      $body .= $self->_convert($root, 'no-page output no unit');
      $body .= &{$self->formatting_function('format_footnotes_segment')}($self);
    }

    # do end file first, in case it needs some CSS
    my $footer = &{$self->formatting_function('format_end_file')}($self,
                                                       $output_filename);
    my $header = &{$self->formatting_function('format_begin_file')}($self,
                                                  $output_filename, undef);
    $output .= $self->write_or_return($header, $fh);
    $output .= $self->write_or_return($body, $fh);
    $output .= $self->write_or_return($footer, $fh);

    # NOTE do not close STDOUT now to avoid a perl warning.
    if ($fh and $no_page_out_filepath ne '-') {
      Texinfo::Common::output_files_register_closed(
            $self->output_files_information(), $encoded_no_page_out_filepath);
      if (!close($fh)) {
        $self->document_error($self,
              sprintf(__("error on closing %s: %s"),
                                      $no_page_out_filepath, $!));
      }
    }
    $self->{'current_filename'} = undef;
    return $output if ($output_file eq '');
  } else {
    # output with pages
    print STDERR "DO Elements with filenames\n"
      if ($self->get_conf('DEBUG'));
    my %files;

    my $unit_nr = -1;
    # Now do the output, converting each tree units and special elements in turn
    $special_elements = [] if (!defined($special_elements));
    foreach my $element (@$tree_units, @$special_elements) {
      my $element_filename = $element->{'structure'}->{'unit_filename'};
      my $out_filepath = $self->{'out_filepaths'}->{$element_filename};
      $self->{'current_filename'} = $element_filename;

      $unit_nr++;
      # First do the special pages, to avoid outputting these if they are
      # empty.
      my $special_element_content;
      if (defined($element->{'type'})
          and $element->{'type'} eq 'special_element') {
        print STDERR "\nUNIT SPECIAL\n" if ($self->get_conf('DEBUG'));
        $special_element_content
                       .= $self->_convert($element, "output s-unit $unit_nr");
        if ($special_element_content eq '') {
          $self->{'file_counters'}->{$element_filename}--;
          next ;
        }
      }

      # convert body before header in case this affects the header
      my $body = '';
      if (defined($special_element_content)) {
        $body = $special_element_content;
      } else {
        print STDERR "\nUNIT $unit_nr\n" if ($self->get_conf('DEBUG'));
        $body = $self->_convert($element, "output unit $unit_nr");
      }

      # register the element but do not print anything. Printing
      # only when file_counters reach 0, to be sure that all the
      # elements have been converted.
      if (!exists($files{$element_filename})) {
        $files{$element_filename} = {'first_element' => $element,
                                     'body' => ''};
      }
      $files{$element_filename}->{'body'} .= $body;
      $self->{'file_counters'}->{$element_filename}--;
      if ($self->{'file_counters'}->{$element_filename} == 0) {
        my $file_element = $files{$element_filename}->{'first_element'};
        my ($encoded_out_filepath, $path_encoding)
          = $self->encoded_output_file_name($out_filepath);
        my ($file_fh, $error_message)
                = Texinfo::Common::output_files_open_out(
                         $self->output_files_information(), $self,
                         $encoded_out_filepath);
        if (!$file_fh) {
          $self->document_error($self,
               sprintf(__("could not open %s for writing: %s"),
                                    $out_filepath, $error_message));
          return undef;
        }
        # do end file first in case it requires some CSS
        my $end_file = &{$self->formatting_function('format_end_file')}($self,
                                                           $element_filename);
        print $file_fh "".&{$self->formatting_function('format_begin_file')}(
                                       $self, $element_filename, $file_element);
        print $file_fh "".$files{$element_filename}->{'body'};
        # end file
        print $file_fh "". $end_file;

        # NOTE do not close STDOUT here to avoid a perl warning
        if ($out_filepath ne '-') {
          Texinfo::Common::output_files_register_closed(
             $self->output_files_information(), $encoded_out_filepath);
          if (!close($file_fh)) {
            $self->document_error($self,
                       sprintf(__("error on closing %s: %s"),
                                  $out_filepath, $!));
            return undef;
          }
        }
      }
    }
    delete $self->{'current_filename'};
    if ($self->get_conf('INFO_JS_DIR')) {
      my $jsdir = File::Spec->catdir($destination_directory,
                                     $self->get_conf('INFO_JS_DIR'));
      if (!-d $jsdir) {
        if (-f $jsdir) {
          $self->document_error($self,
            sprintf(__("%s already exists but is not a directory"), $jsdir));
        } else {
          mkdir $jsdir;
        }
      }
      # Copy JS files.
      if (-d $jsdir) {
        if (!$self->get_conf('TEST')) {
          my $jssrcdir;
          if (!$Texinfo::ModulePath::texinfo_uninstalled) {
            $jssrcdir = File::Spec->catdir(
              $Texinfo::ModulePath::pkgdatadir, 'js');
          } else {
            $jssrcdir = File::Spec->catdir(
              $Texinfo::ModulePath::top_srcdir, 'js');
          }
          for my $f ('info.js', 'modernizr.js', 'info.css') {
            my $from = File::Spec->catfile($jssrcdir, $f);

            if (!copy($from, $jsdir)) {
              $self->document_error($self,
                sprintf(__("error on copying %s into %s"), $from, $jsdir));
            }
          }
        } else {
        # create empty files for tests to keep results stable.
          for my $f ('info.js', 'modernizr.js', 'info.css') {
            my $filename = File::Spec->catfile($jsdir, $f);
            if (!open (FH, '>', $filename)) {
              $self->document_error($self,
                sprintf(__("error on creating empty %s: %s"),
                        $filename, $!));
            }
            if (!close(FH)) {
              $self->document_error($self,
                sprintf(__("error on closing empty %s: %s"),
                        $filename, $!));
            }
          }
        }
      }
    }
  }

  my $jslicenses = $self->get_info('jslicenses');
  if ($jslicenses and scalar(%$jslicenses)) {
    $self->_do_jslicenses_file($destination_directory);
  }

  my $finish_status = $self->run_stage_handlers($root, 'finish');
  return undef unless ($finish_status < $handler_fatal_error_level
                       and $finish_status > -$handler_fatal_error_level);

  my $extension = '';
  $extension = '.'.$self->get_conf('EXTENSION')
            if (defined($self->get_conf('EXTENSION'))
                and $self->get_conf('EXTENSION') ne '');
  # do node redirection pages
  $self->{'current_filename'} = undef;
  if ($self->get_conf('NODE_FILES')
      and $self->{'labels'} and $output_file ne '') {
    my %redirection_filenames;
    foreach my $label (sort(keys (%{$self->{'labels'}}))) {
      my $target_element = $self->{'labels'}->{$label};
      my $label_element = Texinfo::Common::get_label_element($target_element);
      my $label_contents = $label_element->{'contents'};
      my $target = $self->_get_target($target_element);
      # filename may not be defined in case of an @anchor or similar in
      # @titlepage, and @titlepage is not used.
      my $filename = $self->command_filename($target_element);
      my $node_filename;
      # NOTE 'node_filename' is not used for Top, TOP_NODE_FILE_TARGET
      # is.  The other manual must use the same convention to get it
      # right.  We do not do 'node_filename' as a redirection file
      # either.
      if ($target_element->{'extra'}
          and $target_element->{'extra'}->{'normalized'}
          and $target_element->{'extra'}->{'normalized'} eq 'Top'
          and defined($self->get_conf('TOP_NODE_FILE_TARGET'))) {
        $node_filename = $self->get_conf('TOP_NODE_FILE_TARGET');
      } else {
        $node_filename = $target->{'node_filename'};
      }

      if (defined($filename) and $node_filename ne $filename) {
        my $redirection_filename
          = $self->register_normalize_case_filename($node_filename);
        # first condition finds conflict with tree elements
        if ($self->{'elements_in_file_count'}->{$redirection_filename}
            or $redirection_filenames{$redirection_filename}) {
          $self->line_warn($self,
             sprintf(__("\@%s `%s' file %s for redirection exists"),
               $target_element->{'cmdname'},
               Texinfo::Convert::Texinfo::convert_to_texinfo({'contents'
                                                   => $label_contents}),
               $redirection_filename),
            $target_element->{'source_info'});
          my $file_source = $files_source_info{$redirection_filename};
          my $file_info_type = $file_source->{'file_info_type'};
          if ($file_info_type eq 'special_file'
              or $file_info_type eq 'stand_in_file') {
            my $name = $file_source->{'file_info_name'};
            if ($name eq 'non_split') {
              # This cannot actually happen, as the @anchor/@node/@float
              # with potentially conflicting name will also be in the
              # non-split output document and therefore does not need
              # a redirection.
              $self->document_warn($self,
                            __("conflict with whole document file"), 1);
            } elsif ($name eq 'Top') {
              $self->document_warn($self,
                           __("conflict with Top file"), 1);
            } elsif ($name eq 'user_defined') {
              $self->document_warn($self,
                            __("conflict with user-defined file"), 1);
           } elsif ($name eq 'unknown_node') {
              $self->document_warn($self,
                           __("conflict with unknown node file"), 1);
            } elsif ($name eq 'unknown') {
              $self->document_warn($self,
                            __("conflict with file without known source"), 1);
            }
          } elsif ($file_info_type eq 'node') {
            my $conflicting_node = $file_source->{'file_info_element'};
            $self->line_warn($self,
         sprintf(__p('conflict of redirection file with file based on node name',
                     "conflict with \@%s `%s' file"),
                 $conflicting_node->{'cmdname'},
                 Texinfo::Convert::Texinfo::convert_to_texinfo({'contents'
                   => $conflicting_node->{'args'}->[0]->{'contents'}}),
                 ),
              $conflicting_node->{'source_info'}, 1);
          } elsif ($file_info_type eq 'redirection') {
            my $conflicting_node = $file_source->{'file_info_element'};
            my $conflicting_label_contents
                 = $file_source->{'file_info_label_contents'};
            $self->line_warn($self,
               sprintf(__("conflict with \@%s `%s' redirection file"),
                 $conflicting_node->{'cmdname'},
                 Texinfo::Convert::Texinfo::convert_to_texinfo({'contents'
                                            => $conflicting_label_contents}),
                 ),
              $conflicting_node->{'source_info'}, 1);
          } elsif ($file_info_type eq 'section') {
            my $conflicting_section = $file_source->{'file_info_element'};
            $self->line_warn($self,
         sprintf(__p('conflict of redirection file with file based on section name',
                     "conflict with \@%s `%s' file"),
                 $conflicting_section->{'cmdname'},
                 Texinfo::Convert::Texinfo::convert_to_texinfo({'contents'
                   => $conflicting_section->{'args'}->[0]->{'contents'}}),
                 ),
              $conflicting_section->{'source_info'}, 1);
          } elsif ($file_info_type eq 'special_element') {
            my $special_element = $file_source->{'file_info_element'};
            my $element_variety
              = $special_element->{'extra'}->{'special_element_variety'};
            $self->document_warn($self,
               sprintf(__("conflict with %s special element"),
                       $element_variety), 1);
          }
          next;
        }
        $redirection_filenames{$redirection_filename} = $target_element;
        $files_source_info{$redirection_filename}
          = {'file_info_type' => 'redirection',
             'file_info_element' => $target_element,
             'file_info_label_contents' => $label_contents};
        my $redirection_page
          = &{$self->formatting_function('format_node_redirection_page')}($self,
                                                               $target_element);
        my $out_filename;
        if (defined($destination_directory) and $destination_directory ne '') {
          $out_filename = File::Spec->catfile($destination_directory,
                                              $redirection_filename);
        } else {
          $out_filename = $redirection_filename;
        }
        my ($encoded_out_filename, $path_encoding)
          = $self->encoded_output_file_name($out_filename);
        my ($file_fh, $error_message)
               = Texinfo::Common::output_files_open_out(
                             $self->output_files_information(), $self,
                             $encoded_out_filename);
        if (!$file_fh) {
         $self->document_error($self, sprintf(__(
                                    "could not open %s for writing: %s"),
                                    $out_filename, $error_message));
        } else {
          print $file_fh $redirection_page;
          Texinfo::Common::output_files_register_closed(
                  $self->output_files_information(), $encoded_out_filename);
          if (!close ($file_fh)) {
            $self->document_error($self, sprintf(__(
                             "error on closing redirection node file %s: %s"),
                                    $out_filename, $!));
            return undef;
          }
        }
      }
    }
  }
  return undef;
}

#my $characters_replaced_from_class_names = quotemeta('[](),~#:/\\@+=!;.,?* ');
# FIXME not clear what character should be allowed and which ones replaced
# besides space
my $characters_replaced_from_class_names = quotemeta(' ');
sub _protect_class_name($$)
{
  my $self = shift;
  my $class_name = shift;
  $class_name =~ s/[$characters_replaced_from_class_names]/-/g;

  # API info: using the API to allow for customization would be:
  # return &{$self->formatting_function('format_protect_text')}($self, $class_name);
  return _default_format_protect_text($self, $class_name);
}

my $debug;  # whether to print debugging output

# Convert tree element $ELEMENT, and return HTML text for the output files.
sub _convert($$;$);
sub _convert($$;$)
{
  my $self = shift;
  my $element = shift;
  if (!defined($element)) {
    cluck('BUG: _convert: element UNDEF');
    return '';
  }
  # only used for debug
  my $explanation = shift;

  # to help debug and trace
  my $command_type = '';
  if ($element->{'cmdname'}) {
    $command_type = "\@$element->{'cmdname'} ";
  }
  if (defined($element->{'type'})) {
    $command_type .= $element->{'type'};
  }

  $debug = $self->get_conf('DEBUG') if !defined($debug);
  # cache return value of get_conf for speed

  if ($debug) {
    $explanation = 'NO EXPLANATION' if (!defined($explanation));
    my @contexts_names = map {defined($_->{'context_name'})
                                 ? $_->{'context_name'}: 'UNDEF'}
         @{$self->{'document_context'}->[-1]->{'formatting_context'}};
    print STDERR "ELEMENT($explanation) (".join('|',@contexts_names)."), ->";
    print STDERR " cmd: $element->{'cmdname'}," if ($element->{'cmdname'});
    print STDERR " type: $element->{'type'}" if ($element->{'type'});
    my $text = $element->{'text'};
    if (defined($text)) {
      $text =~ s/\n/\\n/;
      print STDERR " text: $text";
    }
    # uncomment to show perl objects
    #print STDERR " $element (".join('|',@{$self->{'document_context'}->[-1]->{'formatting_context'}}).")";
    print STDERR "\n";
  }

  if (ref($element) ne 'HASH') {
    cluck "_convert: tree element not a HASH\n";
    return '';
  }

  if (($element->{'type'}
        and exists ($self->{'types_conversion'}->{$element->{'type'}})
        and !defined($self->{'types_conversion'}->{$element->{'type'}}))
       or ($element->{'cmdname'}
            and exists($self->{'commands_conversion'}->{$element->{'cmdname'}})
            and !defined($self->{'commands_conversion'}->{$element->{'cmdname'}}))) {
    if ($debug) {
      my $string = 'IGNORED';
      $string .= " \@$element->{'cmdname'}" if ($element->{'cmdname'});
      $string .= " $element->{'type'}" if ($element->{'type'});
      print STDERR "$string\n";
    }
    return '';
  }

  # Process text
  if (defined($element->{'text'})) {
    # already converted to html, keep it as is
    if ($element->{'type'} and $element->{'type'} eq '_converted') {
      return $element->{'text'};
    }
    if ($element->{'type'} and $element->{'type'} eq 'untranslated') {
      my $translated = $self->gdt($element->{'text'});
      my $result = $self->_convert($translated, 'translated TEXT');
      return $result;
    }
    my $result = &{$self->{'types_conversion'}->{'text'}} ($self,
                                                      $element->{'type'},
                                                      $element,
                                                      $element->{'text'});
    print STDERR "DO TEXT => `$result'\n" if $debug;
    return $result;
  }

  if ($element->{'extra'} and $element->{'extra'}->{'missing_argument'}
             and (!$element->{'contents'} or !@{$element->{'contents'}})) {
    print STDERR "MISSING_ARGUMENT\n" if $debug;
    return '';
  }

  # commands like @deffnx have both a cmdname and a def_line type.  It is
  # better to consider them as a def_line type, as the whole point of the
  # def_line type is to handle the same the def*x and def* line formatting.
  if ($element->{'cmdname'}
      and !(($element->{'type'} and $element->{'type'} eq 'def_line')
             or ($element->{'type'}
                 and $element->{'type'} eq 'definfoenclose_command')
             or ($element->{'type'}
                 and $element->{'type'} eq 'index_entry_command'))) {
    my $command_name = $element->{'cmdname'};
    if ($root_commands{$command_name}) {
      $self->{'current_root_command'} = $element;
    }
    if (exists($self->{'commands_conversion'}->{$command_name})) {
      my $convert_to_latex;
      if (exists($brace_commands{$command_name})
          and $brace_commands{$command_name} eq 'context') {
        $self->_new_document_context($command_name);
      }
      if (exists($format_context_commands{$command_name})) {
        push @{$self->{'document_context'}->[-1]->{'formatting_context'}},
                                      {'context_name' => '@'.$command_name};
      }
      if (exists($block_commands{$command_name})) {
        push @{$self->{'document_context'}->[-1]->{'block_commands'}},
                                                          $command_name;
      }
      if (exists ($composition_context_commands{$command_name})) {
        push @{$self->{'document_context'}->[-1]->{'composition_context'}},
                                                               $command_name;
      }
      if ($pre_class_commands{$command_name}) {
        push @{$self->{'document_context'}->[-1]->{'preformatted_classes'}},
          $pre_class_commands{$command_name};
      }
      if ($format_raw_commands{$command_name}) {
        $self->{'document_context'}->[-1]->{'raw'}++;
      } elsif ($command_name eq 'verbatim') {
        $self->{'document_context'}->[-1]->{'verbatim'}++;
      }
      if ($brace_code_commands{$command_name} or
          $preformatted_code_commands{$command_name}) {
        push @{$self->{'document_context'}->[-1]->{'monospace'}}, 1;
      } elsif ($brace_commands{$command_name}
               and $brace_commands{$command_name} eq 'style_no_code') {
        push @{$self->{'document_context'}->[-1]->{'monospace'}}, 0;
      } elsif ($upper_case_commands{$command_name}) {
        $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                             ->{'upper_case'}++;
      } elsif ($math_commands{$command_name}) {
        $self->{'document_context'}->[-1]->{'math'}++;
        $convert_to_latex = 1 if ($self->get_conf('CONVERT_TO_LATEX_IN_MATH'));
      }
      if ($command_name eq 'verb') {
        $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                        ->{'space_protected'}++;
      } elsif ($command_name eq 'w') {
        $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                   ->{'no_break'}++;
      }
      my $result = '';
      if (defined($self->{'commands_open'}->{$command_name})) {
        $result .= &{$self->{'commands_open'}->{$command_name}}($self,
                                                 $command_name, $element);
      }
      my $content_formatted = '';
      if ($element->{'contents'}) {
        if ($convert_to_latex) {
          $content_formatted
           = Texinfo::Convert::LaTeX::convert_to_latex_math(undef,
                                {'contents' => $element->{'contents'}},
                                         $self->{'options_latex_math'});
        } else {
          my $content_idx = 0;
          foreach my $content (@{$element->{'contents'}}) {
            $content_formatted
                .= _convert($self, $content, "$command_type c[$content_idx]");
            $content_idx++;
          }
        }
      }
      my $args_formatted;
      if ($brace_commands{$command_name}
          or ($line_commands{$command_name}
              and $line_commands{$command_name} eq 'line')
          or (($command_name eq 'item' or $command_name eq 'itemx')
               and ($element->{'parent'}->{'type'}
                    and $element->{'parent'}->{'type'} eq 'table_term'))
          or ($command_name eq 'quotation'
              or $command_name eq 'smallquotation')
          or $command_name eq 'float'
          or $command_name eq 'cartouche') {
        $args_formatted = [];
        if ($element->{'args'}) {
          my @args_specification;
          @args_specification = @{$default_commands_args{$command_name}}
            if (defined($default_commands_args{$command_name}));
          my $arg_idx = 0;
          foreach my $arg (@{$element->{'args'}}) {
            my $arg_spec = shift @args_specification;
            $arg_spec = ['normal'] if (!defined($arg_spec));
            my $arg_formatted = {'tree' => $arg};
            foreach my $arg_type (@$arg_spec) {
              my $explanation = "$command_type A[$arg_idx]$arg_type";
              if ($arg_type eq 'normal') {
                if ($convert_to_latex) {
                  $arg_formatted->{'normal'}
                   = Texinfo::Convert::LaTeX::convert_to_latex_math(undef, $arg,
                                                  $self->{'options_latex_math'});
                } else {
                  $arg_formatted->{'normal'} = $self->_convert($arg, $explanation);
                }
              } elsif ($arg_type eq 'monospace') {
                push @{$self->{'document_context'}->[-1]->{'monospace'}}, 1;
                $arg_formatted->{$arg_type} = $self->_convert($arg, $explanation);
                pop @{$self->{'document_context'}->[-1]->{'monospace'}};
              } elsif ($arg_type eq 'string') {
                $self->_new_document_context($command_type);
                $self->{'document_context'}->[-1]->{'string'}++;
                $arg_formatted->{$arg_type} = $self->_convert($arg, $explanation);
                $self->_pop_document_context();
              } elsif ($arg_type eq 'monospacestring') {
                $self->_new_document_context($command_type);
                $self->{'document_context'}->[-1]->{'monospace'}->[-1] = 1;
                $self->{'document_context'}->[-1]->{'string'}++;
                $arg_formatted->{$arg_type} = $self->_convert($arg, $explanation);
                $self->_pop_document_context();
              } elsif ($arg_type eq 'monospacetext') {
                $arg_formatted->{$arg_type}
                  = Texinfo::Convert::Text::convert_to_text($arg,
                         {'code' => 1,
                 Texinfo::Convert::Text::copy_options_for_convert_text($self)});
              } elsif ($arg_type eq 'filenametext') {
                # Always use encoded characters for file names
                $arg_formatted->{$arg_type}
                  = Texinfo::Convert::Text::convert_to_text($arg,
                         {'code' => 1,
               Texinfo::Convert::Text::copy_options_for_convert_text($self, 1)});
              } elsif ($arg_type eq 'url') {
                # set the encoding to UTF-8 to always have a string that is suitable
                # for percent encoding.
                my $text_conversion_options = {'code' => 1,
                  Texinfo::Convert::Text::copy_options_for_convert_text($self, 1)};
                $text_conversion_options->{'enabled_encoding'} = 'utf-8';
                $arg_formatted->{$arg_type}
                   = Texinfo::Convert::Text::convert_to_text($arg,
                                                   $text_conversion_options);
              } elsif ($arg_type eq 'raw') {
                $self->{'document_context'}->[-1]->{'raw'}++;
                $arg_formatted->{$arg_type} = $self->_convert($arg, $explanation);
                $self->{'document_context'}->[-1]->{'raw'}--;
              }
            }
            push @$args_formatted, $arg_formatted;
            $arg_idx++;
          }
        }
      }
      if (exists ($composition_context_commands{$command_name})) {
        pop @{$self->{'document_context'}->[-1]->{'composition_context'}};
      }
      if ($pre_class_commands{$command_name}) {
        pop @{$self->{'document_context'}->[-1]->{'preformatted_classes'}};
      }
      if ($preformatted_code_commands{$command_name}
          or ($brace_commands{$command_name}
              and $brace_commands{$command_name} eq 'style_no_code')
          or $brace_code_commands{$command_name}) {
        pop @{$self->{'document_context'}->[-1]->{'monospace'}};
      } elsif ($upper_case_commands{$command_name}) {
        $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                        ->{'upper_case'}--;
      } elsif ($math_commands{$command_name}) {
        $self->{'document_context'}->[-1]->{'math'}--;
      }
      if ($command_name eq 'verb') {
        $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                   ->{'space_protected'}--;
      } elsif ($command_name eq 'w') {
        $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                   ->{'no_break'}--;
      }
      if ($format_raw_commands{$command_name}) {
        $self->{'document_context'}->[-1]->{'raw'}--;
      } elsif ($command_name eq 'verbatim') {
        $self->{'document_context'}->[-1]->{'verbatim'}--;
      }
      if (exists($block_commands{$command_name})) {
        pop @{$self->{'document_context'}->[-1]->{'block_commands'}};
      }
      if (exists($format_context_commands{$command_name})) {
        pop @{$self->{'document_context'}->[-1]->{'formatting_context'}};
      }
      if (exists($brace_commands{$command_name})
          and $brace_commands{$command_name} eq 'context') {
        $self->_pop_document_context();
      }

      if ($element->{'cmdname'} eq 'node') {
        $self->{'current_node'} = $element;
      }
      # args are formatted, now format the command itself
      if ($args_formatted) {
        if (!defined($self->{'commands_conversion'}->{$command_name})) {
          print STDERR "No command_conversion for $command_name\n";
        } else {
          $result .= &{$self->{'commands_conversion'}->{$command_name}}($self,
                  $command_name, $element, $args_formatted, $content_formatted);
        }
      } else {
        $result .= &{$self->{'commands_conversion'}->{$command_name}}($self,
                $command_name, $element, undef, $content_formatted);
      }
      if ($command_name eq 'documentlanguage') {
        $self->_translate_names();
      }
      return $result;
    } else {
      print STDERR "Command not converted: $command_name\n"
       if ($self->get_conf('VERBOSE') or $self->get_conf('DEBUG'));
      return '';
    }
    if ($root_commands{$command_name}) {
      delete $self->{'current_root_command'};
    }
  } elsif ($element->{'type'}) {

    my $result = '';
    my $type_name = $element->{'type'};
    if (defined($self->{'types_open'}->{$type_name})) {
      $result .= &{$self->{'types_open'}->{$type_name}}($self,
                                               $type_name, $element);
    }
    if ($type_name eq 'paragraph') {
      $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                      ->{'paragraph_number'}++;
    } elsif ($type_name eq 'preformatted'
             or $type_name eq 'rawpreformatted') {
      $self->{'document_context'}->[-1]->{'formatting_context'}->[-1]
                                                   ->{'preformatted_number'}++;
    } elsif ($type_name eq 'unit'
             or $type_name eq 'special_element') {
      $self->{'current_root_element'} = $element;
    } elsif ($self->{'pre_class_types'}->{$type_name}) {
      push @{$self->{'document_context'}->[-1]->{'preformatted_classes'}},
        $self->{'pre_class_types'}->{$type_name};
      push @{$self->{'document_context'}->[-1]->{'composition_context'}},
        $type_name;
    }

    if ($self->{'code_types'}->{$type_name}) {
      push @{$self->{'document_context'}->[-1]->{'monospace'}}, 1;
    }
    if ($type_name eq '_string') {
      $self->{'document_context'}->[-1]->{'string'}++;
    }

    my $content_formatted = '';
    if ($type_name eq 'definfoenclose_command') {
      if ($element->{'args'}) {
        $content_formatted = $self->_convert($element->{'args'}->[0]);
      }
    } elsif ($element->{'contents'}) {
      my $content_idx = 0;
      foreach my $content (@{$element->{'contents'}}) {
        $content_formatted
          .= _convert($self, $content, "$command_type c[$content_idx]");
        $content_idx++;
      }
    }

    if (exists($self->{'types_conversion'}->{$type_name})) {
      $result .= &{$self->{'types_conversion'}->{$type_name}} ($self,
                                                 $type_name,
                                                 $element,
                                                 $content_formatted);
    } elsif (defined($content_formatted)) {
      $result .= $content_formatted;
    }
    if ($self->{'code_types'}->{$type_name}) {
      pop @{$self->{'document_context'}->[-1]->{'monospace'}};
    }
    if ($type_name eq '_string') {
      $self->{'document_context'}->[-1]->{'string'}--;
    }
    if ($type_name eq 'unit' or $type_name eq 'special_element') {
      delete $self->{'current_root_element'};
    } elsif ($self->{'pre_class_types'}->{$type_name}) {
      pop @{$self->{'document_context'}->[-1]->{'preformatted_classes'}};
      pop @{$self->{'document_context'}->[-1]->{'composition_context'}};
    }
    print STDERR "DO type ($type_name) => `$result'\n" if $debug;
    return $result;
    # no type, no cmdname, but contents.
  } elsif ($element->{'contents'}) {
    # this happens inside accents, for section/node names, for @images.
    my $content_formatted = '';
    my $i = 0;
    foreach my $content (@{$element->{'contents'}}) {
      $content_formatted .= $self->_convert($content, "$command_type C[$i]");
      $i++;
    }
    print STDERR "UNNAMED HOLDER => `$content_formatted'\n" if $debug;
    return $content_formatted;
  } else {
    print STDERR "UNNAMED empty\n" if $debug;
    if ($self->{'types_conversion'}->{''}) {
      return &{$self->{'types_conversion'}->{''}} ($self, $element);
    } else {
      return '';
    }
  }
  print STDERR "DEBUG: HERE!($element)\n";
}

sub _set_variables_texi2html($)
{
  my $options = shift;
  my @texi2html_options = (
  # added hopefully temporarily to be able to validate with W3C validator
  #['DOCTYPE', '<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">'],
  #['DOCTYPE', '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">'],
  ['FORMAT_MENU', 'menu'],
  ['NO_USE_SETFILENAME', 1],
  ['USE_SETFILENAME_EXTENSION', 0],
  ['footnotestyle', 'separate'],
  ['CONTENTS_OUTPUT_LOCATION', 'separate_element'],
  ['FORCE', 1],
  ['AVOID_MENU_REDUNDANCY', 1],
  ['USE_ACCESSKEY', 0],
  ['NODE_NAME_IN_MENU', 0],
  ['SHORT_TOC_LINK_TO_TOC', 0],
  ['SHOW_TITLE', 1],
  ['USE_UP_NODE_FOR_ELEMENT_UP', 1],
  ['USE_REL_REV', 0],
  ['USE_LINKS', 0],
  ['USE_NODES', 0],
  ['SPLIT', ''],
  ['PROGRAM_NAME_IN_FOOTER', 1],
  ['PROGRAM_NAME_IN_ABOUT', 1],
  ['HEADER_IN_TABLE', 1],
  ['MENU_ENTRY_COLON', ''],
  ['INDEX_ENTRY_COLON', ''],
  ['DO_ABOUT', undef],
  ['CHAPTER_HEADER_LEVEL', 1],
  ['BIG_RULE', '<hr style="height: 6px;">'],
  ['FOOTNOTE_END_HEADER_LEVEL', 3],
  ['FOOTNOTE_SEPARATE_HEADER_LEVEL', 1],
  ['SECTION_BUTTONS', ['FastBack', 'Back', 'Up', 'Forward', 'FastForward',
                             ' ', ' ', ' ', ' ',
                             'Top', 'Contents', 'Index', 'About' ]],
  ['TOP_BUTTONS', ['Back', 'Forward', ' ',
                             'Contents', 'Index', 'About']],

  ['MISC_BUTTONS', [ 'Top', 'Contents', 'Index', 'About' ]],
  ['CHAPTER_BUTTONS', [ 'FastBack', 'FastForward', ' ',
                              ' ', ' ', ' ', ' ',
                              'Top', 'Contents', 'Index', 'About', ]],
  ['SECTION_FOOTER_BUTTONS', [ 'FastBack', 'FirstInFileBack', 'FirstInFileUp',
                                               'Forward', 'FastForward' ]],
  ['CHAPTER_FOOTER_BUTTONS', [ 'FastBack', 'FastForward', ' ',
                              ' ', ' ', ' ', ' ',
                              'Top', 'Contents', 'Index', 'About', ]],
  ['NODE_FOOTER_BUTTONS', [ 'FastBack', 'Back',
                                            'Up', 'Forward', 'FastForward',
                             ' ', ' ', ' ', ' ',
                             'Top', 'Contents', 'Index', 'About' ]],
  );
  foreach my $option (@texi2html_options) {
    #no warnings 'once';
    #$defaults{$option->[0]} = $option->[1];
    $options->{$option->[0]} = $option->[1];
  }
}

1;

# The documentation of the customization API is in the customization_api
# Texinfo manual.  POD format is not suitable for such a documentation, because
# of the module documentation style, the language limitations, and also because
# the customization API involves multiple modules as well as the main program.

__END__
# Automatically generated from maintain/template.pod

=head1 NAME

Texinfo::Convert::HTML - Convert Texinfo tree to HTML

=head1 SYNOPSIS

  my $converter
    = Texinfo::Convert::HTML->converter({'parser' => $parser});

  $converter->output($tree);
  $converter->convert($tree);
  $converter->convert_tree($tree);
  $converter->output_internal_links(); # HTML only

=head1 NOTES

The Texinfo Perl module main purpose is to be used in C<texi2any> to convert
Texinfo to other formats.  There is no promise of API stability.

=head1 DESCRIPTION

Texinfo::Convert::HTML converts a Texinfo tree to HTML.

=head1 METHODS

=over

=item $converter = Texinfo::Convert::HTML->converter($options)

Initialize converter from Texinfo to HTML.

The I<$options> hash reference holds options for the converter.  In
this option hash reference a L<parser object|Texinfo::Parser>
may be associated with the I<parser> key.  The other options
are Texinfo customization options and a few other options that can
be passed to the converter. Most of the customization options are described in
the Texinfo manual.  Those customization options, when appropriate, override
the document content.  The parser should not be available directly anymore
after getting the associated information.

See L<Texinfo::Convert::Converter> for more information.

=item $converter->output($tree)

Convert a Texinfo tree I<$tree> and output the result in files as
described in the Texinfo manual.

=item $result = $converter->convert($tree)

Convert a Texinfo tree I<$tree> and return the resulting output.

=item $result = $converter->convert_tree($tree)

Convert a Texinfo tree portion I<$tree> and return the resulting
output.  This function does not try to output a full document but only
portions.  For a full document use C<convert>.

=item $result = $converter->output_internal_links()
X<C<output_internal_links>>

Returns text representing the links in the document.  The format should
follow the C<--internal-links> option of the C<texi2any>
specification.  This is only supported in (and relevant for) HTML.

=back

=head1 AUTHOR

Patrice Dumas, E<lt>pertusus@free.frE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright 2010- Free Software Foundation, Inc.  See the source file for
all copyright years.

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or (at
your option) any later version.

=cut
