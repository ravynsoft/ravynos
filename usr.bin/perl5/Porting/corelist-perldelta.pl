#!perl
use 5.010;
use strict;
use warnings;
use lib 'Porting';
use Maintainers qw/%Modules/;
use lib 'dist/Module-CoreList/lib';
use Module::CoreList;
use Getopt::Long;

=head1 USAGE

  # generate the module changes for the Perl you are currently building
  ./perl -Ilib Porting/corelist-perldelta.pl

  # update the module changes for the Perl you are currently building
  ./perl -Ilib Porting/corelist-perldelta.pl --mode=update pod/perldelta.pod

  # generate a diff between the corelist sections of two perldelta* files:
  perl Porting/corelist-perldelta.pl --mode=check 5.017001 5.017002 <perl5172delta.pod

=head1 ABOUT

corelist-perldelta.pl is a bit schizophrenic. The part to generate the
new Perldelta text does not need Algorithm::Diff, but wants to be
run with the freshly built Perl.

The part to check the diff wants to be run with a Perl that has an up-to-date
L<Module::CoreList>, but needs the outside L<Algorithm::Diff>.

Ideally, the program will be split into two separate programs, one
to generate the text and one to show the diff between the
corelist sections of the last perldelta and the next perldelta.

Currently no information about Removed Modules is displayed in any of the
modes.

=cut

my %sections = (
  new     => qr/New Modules and Pragma(ta)?/,
  updated => qr/Updated Modules and Pragma(ta)?/,
  removed => qr/Removed Modules and Pragma(ta)?/,
);

my %titles = (
  new     => 'New Modules and Pragmata',
  updated => 'Updated Modules and Pragmata',
  removed => 'Removed Modules and Pragmata',
);

my $deprecated;

sub run {
  my %opt = (mode => 'generate');

  GetOptions(\%opt,
    'mode|m:s', # 'generate', 'check', 'update'
  );

  # by default, compare latest two version in CoreList;
  my ($old, $new) = latest_two_perl_versions();

  # use the provided versions if present
  # @ARGV >=2 means [old_version] [new_version] [path/to/file]
  if ( @ARGV >= 2) {
    ($old, $new) = (shift @ARGV, shift @ARGV);
    die "$old is an invalid version\n" if not exists
      $Module::CoreList::version{$old};
    die "$new is an invalid version\n" if not exists
      $Module::CoreList::version{$new};
  }

  if ( $opt{mode} eq 'generate' ) {
    do_generate($old => $new);
  }
  elsif ( $opt{mode} eq 'check' ) {
    do_check(\*ARGV, $old => $new);
  }
  elsif ( $opt{mode} eq 'update' ) {
    do_update_existing(shift @ARGV, $old => $new);
  }
  else {
    die "Unrecognized mode '$opt{mode}'\n";
  }

  exit 0;
}

sub latest_two_perl_versions {

  my @versions = sort keys %Module::CoreList::version;

  my $new = pop @versions;

  # If a fully-padded version number ends in a zero (as in "5.019010"), that
  # version shows up in %Module::CoreList::version both with and without its
  # trailing zeros. So skip all versions that are numerically equal to $new.
  pop @versions while @versions && $versions[-1] == $new;

  die "Too few distinct core versions in %Module::CoreList::version ?!\n"
    if !@versions;

  return $versions[-1], $new;
}

# Given two perl versions, it returns a list describing the core distributions that have changed.
# The first three elements are hashrefs corresponding to new, updated, and removed modules
# and are of the form (mostly, see the special remarks about removed):
#   'Distribution Name' => ['Distribution Name', previous version number, current version number]
# where the version number is undef if the distribution did not exist.
# The fourth element is an arrayref of core distribution names of those distribution for which it
# is unknown whether they have changed and therefore need to be manually checked.
#
# In most cases, the distribution name in %Modules corresponds to the module that is representative
# of the distribution as listed in Module::CoreList. However, there are a few distribution names
# that do not correspond to a module. %distToModules has been created which maps the distribution
# name to a representative module. The representative module was chosen by either looking at the
# Makefile of the distribution or by seeing which module the distribution has been traditionally
# listed under in past perldeltas.
#
# There are a few distributions for which there is no single representative module (e.g. libnet).
# These distributions are returned as the last element of the list.
#
# %Modules contains a final key, _PERLLIB, which contains a list of modules that are owned by p5p.
# This list contains modules and pragmata that may also be present in Module::CoreList.
# A list of modules are in the list @unclaimedModules, which were manually listed based on whether
# they were independent modules and whether they have been listed in past perldeltas.
# The pragmata were found by doing something like:
#   say for sort grep { $_ eq lc $_ and !exists $Modules{$_}}
#     keys %{$Module::CoreList::version{'5.019003'}}
# and manually filtering out pragmata that were already covered.
#
# It is currently not possible to differentiate between a removed module and a removed
# distribution. Therefore, the removed hashref contains every module that has been removed, even if
# the module's corresponding distribution has not been removed.

sub corelist_delta {
  my ($old, $new) = @_;
  my $corelist = \%Module::CoreList::version;
  my %changes = Module::CoreList::changes_between( $old, $new );
  $deprecated = $Module::CoreList::deprecated{$new};

  my $getModifyType = sub {
    my $data = shift;
    if ( exists $data->{left} and exists $data->{right} ) {
      return 'updated';
    }
    elsif ( !exists $data->{left} and exists $data->{right} ) {
      return 'new';
    }
    elsif ( exists $data->{left} and !exists $data->{right} ) {
      return 'removed';
    }
    return undef;
  };

  my @unclaimedModules = qw/AnyDBM_File B B::Concise B::Deparse Benchmark Class::Struct Config::Extensions DB
                            DBM_Filter Devel::Peek DirHandle DynaLoader English Errno ExtUtils::Embed ExtUtils::Miniperl
                            ExtUtils::Typemaps ExtUtils::XSSymSet Fcntl File::Basename File::Compare File::Copy File::DosGlob
                            File::Find File::Glob File::stat FileCache FileHandle FindBin GDBM_File Getopt::Std Hash::Util Hash::Util::FieldHash
                            I18N::Langinfo IPC::Open3 NDBM_File ODBM_File Opcode PerlIO PerlIO::encoding PerlIO::mmap PerlIO::scalar PerlIO::via
                            Pod::Functions Pod::Html POSIX SDBM_File SelectSaver Symbol Sys::Hostname Thread Tie::Array Tie::Handle Tie::Hash
                            Tie::Hash::NamedCapture Tie::Memoize Tie::Scalar Tie::StdHandle Tie::SubstrHash Time::gmtime Time::localtime Time::tm
                            Unicode::UCD UNIVERSAL User::grent User::pwent VMS::DCLsym VMS::Filespec VMS::Stdio XS::Typemap XS::APItest Win32CORE/;
  my @unclaimedPragmata = qw/arybase attributes blib bytes charnames deprecate diagnostics encoding feature fields filetest inc::latest integer less locale mro open ops overload overloading re sigtrap sort strict subs utf8 vars vmsish/;
  my @unclaimed = (@unclaimedModules, @unclaimedPragmata);

  my %distToModules = (
    'IO-Compress' => [
      {
        'name'         => 'IO-Compress',
        'modification' => $getModifyType->( $changes{'IO::Compress::Base'} ),
        'data'         => $changes{'IO::Compress::Base'}
      }
    ],
    'libnet' => [
      {
        'name'         => 'libnet',
        'modification' => $getModifyType->( $changes{'Net::Cmd'} ),
        'data'         => $changes{'Net::Cmd'}
      }
    ],
    'PathTools' => [
      {
        'name'         => 'File::Spec',
        'modification' => $getModifyType->( $changes{'Cwd'} ),
        'data'         => $changes{'Cwd'}
      }
    ],
    'podlators' => [
      {
        'name'         => 'podlators',
        'modification' => $getModifyType->( $changes{'Pod::Text'} ),
        'data'         => $changes{'Pod::Text'}
      }
    ],
    'Scalar-List-Utils' => [
      {
        'name'         => 'List::Util',
        'modification' => $getModifyType->( $changes{'List::Util'} ),
        'data'         => $changes{'List::Util'}
      },
      {
        'name'         => 'Scalar::Util',
        'modification' => $getModifyType->( $changes{'Scalar::Util'} ),
        'data'         => $changes{'Scalar::Util'}
      },
      {
        'name'         => 'Sub::Util',
        'modification' => $getModifyType->( $changes{'Sub::Util'} ),
        'data'         => $changes{'Sub::Util'}
      }
    ],
    'Text-Tabs+Wrap' => [
      {
        'name'         => 'Text::Tabs',
        'modification' => $getModifyType->( $changes{'Text::Tabs'} ),
        'data'         => $changes{'Text::Tabs'}
      },
      {
        'name'         => 'Text::Wrap',
        'modification' => $getModifyType->( $changes{'Text::Wrap'} ),
        'data'         => $changes{'Text::Wrap'}
      }
    ],
  );

  # structure is (new|removed|updated) => [ [ModuleName, previousVersion, newVersion] ]
  my $deltaGrouping = {};

  # list of distributions listed in %Modules that need to be manually checked because there is no module that represents it
  my @manuallyCheck;

  # %Modules defines what is currently in core
  for my $k ( keys %Modules ) {
    next if $k eq '_PERLLIB'; #these are taken care of by being listed in @unclaimed
    next if Module::CoreList::is_core($k) and !exists $changes{$k}; #modules that have not changed

    my ( $distName, $modifyType, $data );

    if ( exists $changes{$k} ) {
      $distName   = $k;
      $modifyType = $getModifyType->( $changes{$k} );
      $data       = $changes{$k};
    }
    elsif ( exists $distToModules{$k} ) {
      # modification will be undef if the distribution has not changed
      my @modules = grep { $_->{modification} } @{ $distToModules{$k} };
      for (@modules) {
        $deltaGrouping->{ $_->{modification} }->{ $_->{name} } = [ $_->{name}, $_->{data}->{left}, $_->{data}->{right} ];
      }
      next;
    }
    else {
      push @manuallyCheck, $k and next;
    }

    $deltaGrouping->{$modifyType}->{$distName} = [ $distName, $data->{left}, $data->{right} ];
  }

  for my $k (@unclaimed) {
    if ( exists $changes{$k} ) {
      $deltaGrouping->{ $getModifyType->( $changes{$k} ) }->{$k} =
        [ $k, $changes{$k}->{left}, $changes{$k}->{right} ];
    }
  }

  # in old corelist, but not this one => removed
  # N.B. This is exhaustive -- not just what's in %Modules, so modules removed from
  # distributions will show up here, too.  Some person will have to review to see what's
  # important. That's the best we can do without a historical Maintainers.pl
  for my $k ( keys %{ $corelist->{$old} } ) {
    if ( ! exists $corelist->{$new}{$k} ) {
      $deltaGrouping->{'removed'}->{$k} = [ $k, $corelist->{$old}{$k}, undef ];
    }
  }

  return (
    \%{ $deltaGrouping->{'new'} },
    \%{ $deltaGrouping->{'removed'} },
    \%{ $deltaGrouping->{'updated'} },
    \@manuallyCheck
  );
}

# currently does not update the Removed Module section
sub do_update_existing {
  my ( $existing, $old, $new ) = @_;

  my ( $added, $removed, $updated, $manuallyCheck ) = corelist_delta( $old => $new );
  if (@{$manuallyCheck}) {
    print "It cannot be determined whether the following distributions have changed.\n";
    print "Please check and list accordingly:\n";
    say "\t* $_" for sort @{$manuallyCheck};
    print "\n";
  }

  my $data = {
    new      => $added,
    updated  => $updated,
    #removed => $removed, ignore removed for now
  };

  my $text = DeltaUpdater::transform_pod( $existing, $data );
  open my $out, '>', $existing or die "can't open perldelta file $existing: $!";
  binmode($out);
  print $out $text;
  close $out;
  say "The New and Updated Modules and Pragmata sections in $existing have been updated";
  say "Please ensure the Removed Modules and Pragmata section is up-to-date";
}

sub do_generate {
  my ($old, $new) = @_;
  my ($added, $removed, $updated, $manuallyCheck) = corelist_delta($old => $new);

  if ($manuallyCheck) {
    print "\nXXXIt cannot be determined whether the following distributions have changed.\n";
    print "Please check and list accordingly:\n";
    say "\t$_" for @{$manuallyCheck};
    print "\n";
  }

  my $data = {
    new      => $added,
    updated  => $updated,
    #removed => $removed, ignore removed for now
  };

  say DeltaUpdater::sections_to_pod($data)
}

sub do_check {
  my ($in, $old, $new) = @_;

  my $delta = DeltaParser->new($in);
  my ($added, $removed, $updated) = corelist_delta($old => $new);

  # because of the difficulty in identifying the distribution for removed modules
  # don't bother checking them
  for my $ck ([ 'new', $delta->new_modules, $added ],
              #[ 'removed', $delta->removed_modules, $removed ],
              [ 'updated', $delta->updated_modules, $updated ] ) {
    my @delta = @{ $ck->[1] };
    my @corelist = sort { lc $a->[0] cmp lc $b->[0] } values %{ $ck->[2] };

    printf $ck->[0] . ":\n";

    require Algorithm::Diff;
    my $diff = Algorithm::Diff->new(map {
      [map { join q{ } => grep defined, @{ $_ } } @{ $_ }]
    } \@delta, \@corelist);

    while ($diff->Next) {
      next if $diff->Same;
      my $sep = '';
      if (!$diff->Items(2)) {
        printf "%d,%dd%d\n", $diff->Get(qw( Min1 Max1 Max2 ));
      } elsif(!$diff->Items(1)) {
        printf "%da%d,%d\n", $diff->Get(qw( Max1 Min2 Max2 ));
      } else {
        $sep = "---\n";
        printf "%d,%dc%d,%d\n", $diff->Get(qw( Min1 Max1 Min2 Max2 ));
      }
      print "Delta< $_\n" for $diff->Items(1);
      print $sep;
      print "Corelist> $_\n" for $diff->Items(2);
    }

    print "\n";
  }
}

{

  package DeltaUpdater;
  use List::Util 'reduce';

  sub get_section_name_from_heading {
    my $heading = shift;
    while (my ($key, $expression) = each %sections) {
      if ($heading =~ $expression) {
        return $titles{$key};
      }
    }
    die "$heading did not match any section";
  }

  sub is_desired_section_name {
    for (values %sections) {
      return 1 if $_[0] =~ $_;
    }
    return 0;
  }

  # verify the module and pragmata in the section, changing the stated version if necessary
  # this subroutine warns if the module name cannot be parsed or if it is not listed in
  # the results returned from corelist_delta()
  #
  # a side-effect of calling this function is that modules present in the section are
  # removed from $data, resulting in $data containing only those modules and pragmata
  # that were not listed in the perldelta file. This means we can then pass $data to
  # add_to_section() without worrying about filtering out duplicates
  sub update_section {
    my ( $section, $data, $title ) = @_;
    my @items = @{ $section->{items} };

    for my $item (@items) {

      my $content = $item->{text};
      my $module  = $item->{name};

      #skip dummy items
      next if !$module and $content =~ /\s*xx*\s*/i;

      say "Could not parse module name; line is:\n\t$content" and next unless $module;

      if ( !$data->{$title}{$module} ) {
        print "$module is not listed as being $title in Module::CoreList.\n";
        print "Ensure Module::CoreList has been updated and\n";
        print "check to see that the distribution is not listed under another name.\n\n";
        next;
      }

      if ( $title eq 'new' ) {
        my ($new) = $content =~ /(\d[^\s]+)\s+has\s+been.*$/m;
        say "Could not parse new version for $module; line is:\n\t$content" and next unless $new;
        if ( $data->{$title}{$module}[2] ne $new ) {
            say "$module: new version differs; version in pod: $new; version in corelist: " . $data->{$title}{$module}[2];
        }
        $content =~ s/\d[^\s]+(\s+has\s+been.*$)/$data->{$title}{$module}[2].$1/me;
      }

      elsif ( $title eq 'updated' ) {
        my ( $prev, $new ) = $content =~ /from\s+(?:version\s+)?(\d[^\s]+)\s+to\s+(?:version\s+)?(\d[^\s,]+?)(?=[\s,]|\.\s|\.$|$).*/s;
        say "Could not parse old and new version for $module; line is:\n\t$content" and next
          unless $prev and $new;
        if ( $data->{$title}{$module}[1] ne $prev ) {
          say "$module: previous version differs; version in pod: $prev; version in corelist: " . $data->{$title}{$module}[1];
        }
        if ( $data->{$title}{$module}[2] ne $new ) {
          say "$module: new version differs; version in pod: $new; version in corelist: " . $data->{$title}{$module}[2];
        }
        $content =~
          s/(from\s+(?:version\s+)?)\d[^\s]+(\s+to\s+(?:version\s+)?)\d[^\s,]+?(?=[\s,]|\.\s|\.$|$)(.*)/$1.$data->{$title}{$module}[1].$2.$data->{$title}{$module}[2].$3/se;
      }

      elsif ( $title eq 'removed' ) {
        my ($prev) = $content =~ /^.*?was\s+(\d[^\s]+?)/m;
        say "Could not parse old version for $module; line is:\n\t$content" and next unless $prev;
        if ( $data->{$title}{$module}[1] ne $prev ) {
          say "$module: previous version differs; $prev " . $data->{$title}{$module}[1];
        }
        $content =~ s/(^.*?was\s+)\d[^\s]+?/$1.$data->{$title}{$module}[1]/me;
      }

      delete $data->{$title}{$module};
      $item->{text} = $content;
    }
    return $section;
  }

  # add modules and pragmata present in $data to the section
  sub add_to_section {
    my ( $section, $data, $title ) = @_;

    #undef is a valid version name in Module::CoreList so suppress warnings about concatenating undef values
    no warnings 'uninitialized';
    for ( values %{ $data->{$title} } ) {
      my ( $mod, $old_v, $new_v ) = @{$_};
      my ( $item, $text );

      $item = { name => $mod, text => "=item *\n" };
      if ( $title eq 'new' ) {
        $text = "L<$mod> $new_v has been added to the Perl core.\n";
      }

      elsif ( $title eq 'updated' ) {
        $text = "L<$mod> has been upgraded from version $old_v to $new_v.\n";
        if ( $deprecated->{$mod} ) {
          $text .= "NOTE: L<$mod> is deprecated and may be removed from a future version of Perl.\n";
        }
      }

      elsif ( $title eq 'removed' ) {
        $text = "C<$mod> has been removed from the Perl core.  Prior version was $old_v.\n";
      }

      $item->{text} .= "\n$text\n";
      push @{ $section->{items} }, $item;
    }
    return $section;
  }

  sub sort_items_in_section {
    my ($section) = @_;

    # if we could not parse the module name, it will be uninitialized
    # in sort. This is not a problem as it will just result in these
    # sections being placed near the beginning of the section
    no warnings 'uninitialized';
    $section->{items} =
      [ sort { lc $a->{name} cmp lc $b->{name} } @{ $section->{items} } ];
    return $section;
  }

  # given a hashref of the form returned by corelist_delta()
  # and a hash structured as documented in transform_pod(), it returns
  # a pod string representation of the sections, creating sections
  # if necessary
  sub sections_to_pod {
    my ( $data, %sections ) = @_;
    my $out = '';

    for (
        (
          [ 'New Modules and Pragmata',     'new' ],
          [ 'Updated Modules and Pragmata', 'updated' ],
          [ 'Removed Modules and Pragmata', 'removed' ]
        )
      )
    {
      my ( $section_name, $title ) = @{$_};

      my $section = $sections{$section_name} // {
          name           => $section_name,
          preceding_text => "=head2 $_->[0]\n=over 4\n",
          following_text => "=back\n",
          items          => [],
          manual         => 1
      };

      $section = update_section( $section, $data, $title );
      $section = add_to_section( $section, $data, $title );
      $section = sort_items_in_section( $section );

      next if $section->{manual} and scalar @{ $section->{items} } == 0;

      my $items = reduce { no warnings 'once'; $a . $b->{text} }
        ( '', @{ $section->{items} } );
      $out .=
        ( $section->{preceding_text} // '' )
        . $items
        . ( $section->{following_text} // '' );
    }
    return $out;
  }

  # given a filename corresponding to an existing perldelta file
  # and a hashref of the form returned by corelist_delta(), it
  # returns a string of the resulting file after the module
  # information has been added.
  sub transform_pod {
    my ( $existing, $data ) = @_;

    # will contain hashrefs corresponding to new, updated and removed
    # modules and pragmata keyed by section name
    # each section is hashref of the structure
    #   preceding_text => Text occurring before and including the over
    #                     region containing the list of modules,
    #   items          => [Arrayref of hashrefs corresponding to a module
    #                      entry],
    #     an entry has the form:
    #       name => Module name or undef if the name could not be determined
    #       text => The text of the entry, including the item heading
    #
    #   following_text => Any text not corresponding to a module
    #                     that occurs after the first module
    #
    # the sections are converted to a pod string by calling sections_to_pod()
    my %sections;

    # we are in the Modules_and_Pragmata's section
    my $in_Modules_and_Pragmata;

    # we are the Modules_and_Pragmata's section but have not
    # encountered any of the desired sections. We use this
    # flag to determine whether we should append the text to $out
    # or we need to delay appending until the module listings are
    # processed and instead append to $append_to_out
    my $in_Modules_and_Pragmata_preamble;

    my $done_processing_Modules_and_Pragmata;

    my $current_section;

    # $nested_element_level == 0 : not in an over region, treat lines as text
    # $nested_element_level == 1 : presumably in the top over region that
    #                              corresponds to the module listing. Treat
    #                              each item as a module
    # $nested_element_level > 1  : we only consider these values when we are in an item
    #                              We treat lines as the text of the current item.
    my $nested_element_level = 0;

    my $current_item;
    my $need_to_parse_module_name;

    my $out = '';
    my $append_to_out = '';

    open my $fh, '<', $existing or die "can't open perldelta file $existing: $!";
    binmode($fh);

    while (<$fh>) {
      # treat the rest of the file as plain text
      if ($done_processing_Modules_and_Pragmata) {
        $out .= $_;
        next;
      }

      elsif ( !$in_Modules_and_Pragmata ) {
        # entering Modules and Pragmata
        if (/^=head1 Modules and Pragmata/) {
          $in_Modules_and_Pragmata          = 1;
          $in_Modules_and_Pragmata_preamble = 1;
        }
        $out .= $_;
        next;
      }

      # leaving Modules and Pragmata
      elsif (/^=head1/) {
        if ($current_section) {
          push @{ $current_section->{items} }, $current_item
            if $current_item;
          $sections{ $current_section->{name} } = $current_section;
        }
        $done_processing_Modules_and_Pragmata = 1;
        $out .=
          sections_to_pod( $data, %sections ) . $append_to_out . $_;
        next;
      }

      # new section in Modules and Pragmata
      elsif (/^=head2 (.*?)$/) {
        my $name = $1;
        if ($current_section) {
          push @{ $current_section->{items} }, $current_item
            if $current_item;
          $sections{ $current_section->{name} } = $current_section;
          undef $current_section;
        }

        if ( is_desired_section_name($name) ) {
          undef $in_Modules_and_Pragmata_preamble;
          if ( $nested_element_level > 0 ) {
            die "Unexpected head2 at line no. $.";
          }
          my $title = get_section_name_from_heading($name);
          if ( exists $sections{$title} ) {
            die "$name occurred twice at line no. $.";
          }
          $current_section                   = {};
          $current_section->{name}           = $title;
          $current_section->{preceding_text} = $_;
          $current_section->{items}          = [];
         $nested_element_level               = 0;
          next;
        }

        # otherwise treat section as plain text
        else {
          if ($in_Modules_and_Pragmata_preamble) {
            $out .= $_;
          }
          else {
            $append_to_out .= $_;
          }
          next;
        }
      }

      elsif ($current_section) {

        # not in an over region
        if ( $nested_element_level == 0 ) {
          if (/^=over/) {
            $nested_element_level++;
          }
          if ( scalar @{ $current_section->{items} } > 0 ) {
            $current_section->{following_text} .= $_;
          }
          else {
            $current_section->{preceding_text} .= $_;
          }
          next;
        }

        if ($current_item) {
          if ($need_to_parse_module_name) {
            # the item may not have a parsable module name, which means that
            # $current_item->{name} will never be defined.
            if (/^(?:L|C)<(.+?)>/) {
              $current_item->{name} = $1;
              undef $need_to_parse_module_name;
            }
            # =item or =back signals the end of an item
            # block, which we handle below
            if ( !/^=(?:item|back)/ ) {
              $current_item->{text} .= $_;
              next;
            }
          }
          # currently in an over region
          # treat text inside region as plain text
          if ( $nested_element_level > 1 ) {
            if (/^=back/) {
              $nested_element_level--;
            }
            elsif (/^=over/) {
              $nested_element_level++;
            }
            $current_item->{text} .= $_;
            next;
          }
          # entering over region
          if (/^=over/) {
            $nested_element_level++;
            $current_item->{text} .= $_;
            next;
          }
          # =item or =back signals the end of an item
          # block, which we handle below
          if ( !/^=(?:item|back)/ ) {
            $current_item->{text} .= $_;
            next;
          }
        }

        if (/^=item \*/) {
          push @{ $current_section->{items} }, $current_item
            if $current_item;
          $current_item = { text => $_ };
          $need_to_parse_module_name = 1;
          next;
        }

        if (/^=back/) {
          push @{ $current_section->{items} }, $current_item
            if $current_item;
          undef $current_item;
          $nested_element_level--;
        }

        if ( scalar @{ $current_section->{items} } == 0 ) {
          $current_section->{preceding_text} .= $_;
        }
        else {
          $current_section->{following_text} .= $_;
        }
        next;
      }

      # text in Modules and Pragmata not in a head2 region
      else {
        if ($in_Modules_and_Pragmata_preamble) {
          $out .= $_;
        }
        else {
          $append_to_out .= $_;
        }
        next;
      }
    }
    close $fh;
    die 'Never saw Modules and Pragmata section' unless $in_Modules_and_Pragmata;
    return $out;
  }

}

{
  package DeltaParser;
  use Pod::Simple::SimpleTree;

  sub new {
    my ($class, $input) = @_;

    my $self = bless {} => $class;

    my $parsed_pod = Pod::Simple::SimpleTree->new->parse_file($input)->root;
    splice @{ $parsed_pod }, 0, 2; # we don't care about the document structure,
                                   # just the nodes within it

    $self->_parse_delta($parsed_pod);

    return $self;
  }

  # creates the accessor methods:
  #   new_modules
  #   updated_modules
  #   removed_modules
  for my $k (keys %sections) {
    no strict 'refs';
    my $m = "${k}_modules";
    *$m = sub { $_[0]->{$m} };
  }

  sub _parse_delta {
    my ($self, $pod) = @_;

    my $new_section     = $self->_look_for_section( $pod, $sections{new} );
    my $updated_section = $self->_look_for_section( $pod, $sections{updated} );
    my $removed_section = $self->_look_for_section( $pod, $sections{removed} );

    $self->_parse_new_section($new_section);
    $self->_parse_updated_section($updated_section);
    $self->_parse_removed_section($removed_section);

    for (qw/new_modules updated_modules removed_modules/) {
      $self->{$_} =
        [ sort { lc $a->[0] cmp lc $b->[0] } @{ $self->{$_} } ];
    }

    return;
  }

  sub _parse_new_section {
    my ($self, $section) = @_;

    $self->{new_modules} = [];
    return unless $section;
    $self->{new_modules} = $self->_parse_section($section => sub {
      my ($el) = @_;

      my ($first, $second) = @{ $el }[2, 3];
      my ($ver) = $second =~ /(\d[^\s]+)\s+has\s+been/;

      return [ $first->[2], undef, $ver ];
    });

    return;
  }

  sub _parse_updated_section {
    my ($self, $section) = @_;

    $self->{updated_modules} = [];
    return unless $section;
    $self->{updated_modules} = $self->_parse_section($section => sub {
      my ($el) = @_;

      my ($first, $second) = @{ $el }[2, 3];
      my $module = $first->[2];

      # the regular expression matches the following:
      #   from VERSION_NUMBER to VERSION_NUMBER
      #   from VERSION_NUMBER to VERSION_NUMBER.
      #   from version VERSION_NUMBER to version VERSION_NUMBER.
      #   from VERSION_NUMBER to VERSION_NUMBER and MODULE from VERSION_NUMBER to VERSION_NUMBER
      #   from VERSION_NUMBER to VERSION_NUMBER, and MODULE from VERSION_NUMBER to VERSION_NUMBER
      #
      # some perldeltas contain more than one module listed in an entry, this only attempts to match the
      # first module
      my ($old, $new) = $second =~
          /from\s+(?:version\s+)?(\d[^\s]+)\s+to\s+(?:version\s+)?(\d[^\s,]+?)(?=[\s,]|\.\s|\.$|$).*/s;

      warn "Unable to extract old or new version of $module from perldelta"
        if !defined $old || !defined $new;

      return [ $module, $old, $new ];
    });

    return;
  }

  sub _parse_removed_section {
    my ($self, $section) = @_;

    $self->{removed_modules} = [];
    return unless $section;
    $self->{removed_modules} = $self->_parse_section($section => sub {
      my ($el) = @_;

      my ($first, $second) = @{ $el }[2, 3];
      my ($old) = $second =~ /was\s+(\d[^\s]+?)\.?$/;

      return [ $first->[2], $old, undef ];
    });

    return;
  }

  sub _parse_section {
    my ($self, $section, $parser) = @_;

    my $items = $self->_look_down($section => sub {
      my ($el) = @_;
      return unless ref $el && $el->[0] =~ /^item-/
          && @{ $el } > 2 && ref $el->[2];
      return unless $el->[2]->[0] =~ /C|L/;

      return 1;
    });

    return [map { $parser->($_) } @{ $items }];
  }

  sub _look_down {
    my ($self, $pod, $predicate) = @_;
    my @pod = @{ $pod };

    my @l;
    while (my $el = shift @pod) {
      push @l, $el if $predicate->($el);
      if (ref $el) {
        my @el = @{ $el };
        splice @el, 0, 2;
        unshift @pod, @el if @el;
      }
    }

    return @l ? \@l : undef;
  }

  sub _look_for_section {
    my ($self, $pod, $section) = @_;

    my $level;
    $self->_look_for_range($pod,
      sub {
        my ($el) = @_;
        my ($heading) = $el->[0] =~ /^head(\d)$/;
        my $f = $heading && $el->[2] =~ /^$section/;
        $level = $heading if $f && !$level;
        return $f;
      },
      sub {
        my ($el) = @_;
        $el->[0] =~ /^head(\d)$/ && $1 <= $level;
      },
    );
  }

  sub _look_for_range {
    my ($self, $pod, $start_predicate, $stop_predicate) = @_;

    my @l;
    for my $el (@{ $pod }) {
      if (@l) {
        return \@l if $stop_predicate->($el);
      }
      else {
        next unless $start_predicate->($el);
      }
      push @l, $el;
    }

    return;
  }
}

run;
