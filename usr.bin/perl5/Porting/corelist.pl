#!perl
# Generates info for Module::CoreList from this perl tree
# run this from the root of a perl tree
#
# Data is on STDOUT.
#
# With an optional arg specifying the root of a CPAN mirror, outputs the
# %upstream and %bug_tracker hashes too.

use autodie;
use strict;
use warnings;
use File::Find;
use ExtUtils::MM_Unix;
use version;
use lib "Porting";
use Maintainers qw(%Modules files_to_modules);
use File::Spec;
use Parse::CPAN::Meta;
use IPC::Cmd 'can_run';
use HTTP::Tiny;
use IO::Uncompress::Gunzip;

my $corelist_file = './dist/Module-CoreList/lib/Module/CoreList.pm';
my $utils_file = './dist/Module-CoreList/lib/Module/CoreList/Utils.pm';

my %lines;
my %module_to_file;
my %modlist;

die "usage: $0 [ cpan-mirror/ ] [ 5.x.y] \n" unless @ARGV <= 2;
my $cpan         = shift;
my $raw_version  = shift || $];
my $perl_version = version->parse("$raw_version");
my $perl_vnum    = $perl_version->numify;
my $perl_vstring = $perl_version->normal; # how do we get version.pm to not give us leading v?
$perl_vstring =~ s/^v//;

if ( !-f 'MANIFEST' ) {
    die "Must be run from the root of a clean perl tree\n";
}

open( my $corelist_fh, '<', $corelist_file );
my $corelist = join( '', <$corelist_fh> );
close $corelist_fh;

unless (
    $corelist =~ /^%released \s* = \s* \(
        .*?
        $perl_vnum \s* => \s* .*?
        \);/ismx
    )
{
    warn "Adding $perl_vnum to the list of released perl versions. Please consider adding a release date.\n";
    $corelist =~ s/^(%released \s* = \s* .*?) ( \) )
                /$1  $perl_vnum => '????-??-??',\n  $2/ismx;
}

if ($cpan) {
    my $modlistfile = File::Spec->catfile( $cpan, 'modules', '02packages.details.txt' );
    my $content;

    my $fh;
    if ( -e $modlistfile ) {
        warn "Reading the module list from $modlistfile";
        open $fh, '<', $modlistfile;
    } elsif ( -e $modlistfile . ".gz" ) {
        my $zcat = can_run('gzcat') || can_run('zcat') or die "Can't find gzcat or zcat";
        warn "Reading the module list from $modlistfile.gz";
        open $fh, '-|', "$zcat $modlistfile.gz";
    } else {
        warn "About to fetch 02packages from www.cpan.org. This may take a few minutes\n";
	my $gzipped_content = fetch_url('http://www.cpan.org/modules/02packages.details.txt.gz');
	unless ($gzipped_content) {
            die "Unable to read 02packages.details.txt from either your CPAN mirror or www.cpan.org";
        }
	IO::Uncompress::Gunzip::gunzip(\$gzipped_content, \$content, Transparent => 0)
	    or die "Can't gunzip content: $IO::Uncompress::Gunzip::GunzipError";
    }

    if ( $fh and !$content ) {
        local $/ = "\n";
        $content = join( '', <$fh> );
    }

    die "Incompatible modlist format"
        unless $content =~ /^Columns: +package name, version, path/m;

    # Converting the file to a hash is about 5 times faster than a regexp flat
    # lookup.
    for ( split( qr/\n/, $content ) ) {
        next unless /^([A-Za-z_:0-9]+) +[-0-9.undefHASHVERSIONvsetwhenloadingbogus]+ +(\S+)/;
        $modlist{$1} = $2;
    }
}

find(
    sub {
        if (-d) {
          my @parts = File::Spec->splitdir($File::Find::name);
          # be careful not to skip inc::latest
          return $File::Find::prune = 1 if @parts == 3 and ($parts[-1] eq 'inc' or $parts[-1] eq 't');
        }

        /(\.pm|_pm\.PL)$/ or return;
        /PPPort\.pm$/ and return;
        my $module = $File::Find::name;
        $module =~ /\b(demo|t|private|corpus)\b/ and return;    # demo or test modules
        my $version = MM->parse_version($_);
        defined $version or $version = 'undef';
        $version =~ /\d/ and $version = "'$version'";

        # some heuristics to figure out the module name from the file name
        $module =~ s{^(lib|cpan|dist|ext|os2/OS2)/}{}
			and $1 ne 'lib'
            and (
            $module =~ s{\b(\w+)/\1\b}{$1},
            $module =~ s{^B/O}{O},
            $module =~ s{^Devel-PPPort}{Devel},
            $module =~ s{^libnet/}{},
            $module =~ s{^PathTools/}{},
            $module =~ s{REXX/DLL}{DLL},
            $module =~ s{^Encode/encoding}{encoding},
            $module =~ s{^IPC-SysV/}{IPC/},
            $module =~ s{^MIME-Base64/QuotedPrint}{MIME/QuotedPrint},
            $module =~ s{^(?:DynaLoader|Errno|Opcode|XSLoader)/}{},
            $module =~ s{^Sys-Syslog/win32}{Sys-Syslog},
            $module =~ s{^Time-Piece/Seconds}{Time/Seconds},
            );
		$module =~ s{^lib/}{}g;
        $module =~ s{/}{::}g;
        $module =~ s{-}{::}g;
		$module =~ s{^.*::lib::}{}; # turns Foo/lib/Foo.pm into Foo.pm
        $module =~ s/(\.pm|_pm\.PL)$//;
        $lines{$module}          = $version;
        $module_to_file{$module} = $File::Find::name;
    },
    'os2/OS2',
    'lib',
    'ext',
	'cpan',
	'dist'
);

-e 'configpm' and $lines{Config} = "$]";

if ( open my $ucdv, "<", "lib/unicore/version" ) {
    chomp( my $ucd = <$ucdv> );
    $lines{Unicode} = "'$ucd'";
    close $ucdv;
}

my $delta_data = make_corelist_delta(
  $perl_vnum,
  \%lines,
  \%Module::CoreList::version
);

my $versions_in_release = "    " . $perl_vnum . " => {\n";
$versions_in_release .= "        delta_from => $delta_data->{delta_from},\n";
$versions_in_release .= "        changed => {\n";
foreach my $key (sort keys $delta_data->{changed}->%*) {
  $versions_in_release .= sprintf "            %-24s=> %s,\n", "'$key'",
      defined $delta_data->{changed}{$key} ? "'"
        . $delta_data->{changed}{$key} . "'" : "undef";
}
$versions_in_release .= "        },\n";
$versions_in_release .= "        removed => {\n";
for my $key (sort keys %{ $delta_data->{removed} || {} }) {
  $versions_in_release .= sprintf "            %-24s=> %s,\n", "'$key'", 1;
}
$versions_in_release .= "        }\n";
$versions_in_release .= "    },\n";

$corelist =~ s/^(%delta\s*=\s*.*?)^\s*$perl_vnum\s*=>\s*{.*?},\s*(^\);)$/$1$2/ism;
$corelist =~ s/^(%delta\s*=\s*.*?)(^\);)$/$1$versions_in_release$2/ism;

exit unless %modlist;

# We have to go through this two stage lookup, given how Maintainers.pl keys its
# data by "Module", which is really a dist.
my $file_to_M = files_to_modules( values %module_to_file );

sub slurp_utf8($) {
    open my $fh, "<:utf8", "$_[0]"
	or die "can't open $_[0] for reading: $!";
    return do { local $/; <$fh> };
}

sub parse_cpan_meta($) {
    return Parse::CPAN::Meta->${
	$_[0] =~ /\A\x7b/ ? \"load_json_string" : \"load_yaml_string"
    }($_[0]);
}

my %module_to_upstream;
my %module_to_dist;
my %dist_to_meta_YAML;
my %module_to_deprecated;
while ( my ( $module, $file ) = each %module_to_file ) {
    my $M = $file_to_M->{$file};
    next unless $M;
    next if $Modules{$M}{MAINTAINER} && $Modules{$M}{MAINTAINER} eq 'P5P';
    $module_to_upstream{$module} = $Modules{$M}{UPSTREAM};
    $module_to_deprecated{$module} = 1 if $Modules{$M}{DEPRECATED};
    next
        if defined $module_to_upstream{$module}
            && $module_to_upstream{$module} eq 'blead';
    my $dist = $modlist{$module};
    unless ($dist) {
        warn "Can't find a distribution for $module\n";
        next;
    }
    $module_to_dist{$module} = $dist;

    next if exists $dist_to_meta_YAML{$dist};

    $dist_to_meta_YAML{$dist} = undef;

    # Like it or lump it, this has to be Unix format.
    my $meta_YAML_path = "authors/id/$dist";
    $meta_YAML_path =~ s/(?:tar\.gz|tar\.bz2|zip|tgz)$/meta/
	or die "ERROR: bad meta YAML path: '$meta_YAML_path'";
    my $meta_YAML_url = 'http://www.cpan.org/' . $meta_YAML_path;

    if ( -e "$cpan/$meta_YAML_path" ) {
        $dist_to_meta_YAML{$dist} = parse_cpan_meta(slurp_utf8( $cpan . "/" . $meta_YAML_path ));
    } elsif ( my $content = fetch_url($meta_YAML_url) ) {
        unless ($content) {
            warn "Failed to fetch $meta_YAML_url\n";
            next;
        }
        eval { $dist_to_meta_YAML{$dist} = parse_cpan_meta($content); };
        if ( my $err = $@ ) {
            warn "$meta_YAML_path: ".$err;
            next;
        }
    } else {
        warn "$meta_YAML_path does not exist for $module\n";

        # I tried code to open the tarballs with Archive::Tar to find and
        # extract META.yml, but only Text-Tabs+Wrap-2006.1117.tar.gz had one,
        # so it's not worth including.
        next;
    }
}

my $upstream_stanza = "%upstream = (\n";
foreach my $module ( sort keys %module_to_upstream ) {
    my $upstream = defined $module_to_upstream{$module} ? "'$module_to_upstream{$module}'" : 'undef';
    $upstream_stanza .= sprintf "    %-24s=> %s,\n", "'$module'", $upstream;
}
$upstream_stanza .= ");";

$corelist =~ s/^%upstream .*? ;$/$upstream_stanza/ismx;

# Deprecation generation
{
  my $delta_data = make_corelist_delta(
    $perl_vnum,
    \%module_to_deprecated,
    do { no warnings 'once'; \%Module::CoreList::deprecated },
  );

  my $deprecated_stanza = "    " . $perl_vnum . " => {\n";
  $deprecated_stanza .= "        delta_from => $delta_data->{delta_from},\n";
  $deprecated_stanza .= "        changed => {\n";
  foreach my $key (sort keys $delta_data->{changed}->%*) {
    $deprecated_stanza .= sprintf "            %-24s=> %s,\n", "'$key'",
        defined $delta_data->{changed}{$key} ? "'"
          . $delta_data->{changed}{$key} . "'" : "undef";
  }
  $deprecated_stanza .= "        },\n";
  $deprecated_stanza .= "        removed => {\n";
  for my $key (sort keys %{ $delta_data->{removed} || {} }) {
    $deprecated_stanza .= sprintf "           %-24s=> %s,\n", "'$key'", 1;
  }
  $deprecated_stanza .= "        }\n";
  $deprecated_stanza .= "    },\n";

  $corelist =~ s/^(%deprecated\s*=\s*.*?)^\s*$perl_vnum\s*=>\s*{.*?},\s*(^\);)$/$1$2/ism;
  $corelist =~ s/^(%deprecated\s*=\s*.*?)(^\);)$/$1$deprecated_stanza$2/xism;
}

my $tracker = "%bug_tracker = (\n";
foreach my $module ( sort keys %module_to_upstream ) {
    my $upstream = defined $module_to_upstream{$module};
    next
        if defined $upstream and $upstream eq 'blead';

    my $bug_tracker;

    my $dist = $module_to_dist{$module};
    $bug_tracker = $dist_to_meta_YAML{$dist}->{resources}{bugtracker}
        if $dist;
    $bug_tracker = $bug_tracker->{web} if ref($bug_tracker) eq "HASH";

    $bug_tracker = defined $bug_tracker ? quote($bug_tracker) : 'undef';
    next if $bug_tracker eq "'https://github.com/Perl/perl5/issues'";
	next if $bug_tracker eq "'http://rt.perl.org/perlbug/'";
	next if $bug_tracker eq "'https://rt.perl.org/perlbug/'";
    $tracker .= sprintf "    %-24s=> %s,\n", "'$module'", $bug_tracker;
}
$tracker .= ");";

$corelist =~ s/^%bug_tracker .*? ;/$tracker/eismx;

write_corelist($corelist,$corelist_file);

open( my $utils_fh, '<', $utils_file );
my $utils = join( '', <$utils_fh> );
close $utils_fh;

my %utils = map { ( $_ => 1 ) } parse_utils_lst();

my $delta_utils = make_coreutils_delta($perl_vnum, \%utils);

my $utilities_in_release = "    " . $perl_vnum . " => {\n";
$utilities_in_release .= "        delta_from => $delta_utils->{delta_from},\n";
$utilities_in_release .= "        changed => {\n";
foreach my $key (sort keys $delta_utils->{changed}->%*) {
  $utilities_in_release .= sprintf "            %-24s=> %s,\n", "'$key'",
      defined $delta_utils->{changed}{$key} ? "'"
        . $delta_utils->{changed}{$key} . "'" : "undef";
}
$utilities_in_release .= "        },\n";
$utilities_in_release .= "        removed => {\n";
for my $key (sort keys %{ $delta_utils->{removed} || {} }) {
  $utilities_in_release .= sprintf "            %-24s=> %s,\n", "'$key'", 1;
}
$utilities_in_release .= "        }\n";
$utilities_in_release .= "    },\n";

$utils =~ s/^(my %delta\s*=\s*.*?)^\s*$perl_vnum\s*=>\s*{.*?},\s*(^\);)$/$1$2/ism;
$utils =~ s/^(my %delta\s*=\s*.*?)(^\);)$/$1$utilities_in_release$2/ism;

write_corelist($utils,$utils_file);

warn "All done. Please check over the following files carefully before committing.\nThanks!\n";
warn "$corelist_file\n$utils_file\n";

sub write_corelist {
    my $content = shift;
    my $filename = shift;
    open (my $clfh, ">", $filename);
    binmode $clfh;
    print $clfh $content;
    close($clfh);
}

sub fetch_url {
    my $url = shift;
    my $http = HTTP::Tiny->new;
    my $response = $http->get($url);
    if ($response->{success}) {
	return $response->{content};
    } else {
	warn "Error fetching $url: $response->{status} $response->{reason}\n";
        return;
    }
}

sub make_corelist_delta {
  my($version, $lines, $existing) = @_;
  # Trust core perl, if someone does use a weird version number the worst that
  # can happen is an extra delta entry for a module.
  my %versions = map { $_ => eval $lines->{$_} } keys %$lines;

  # Ensure we have the corelist data loaded from this perl checkout, not the system one.
  require $corelist_file;

  my %deltas;
  # Search for the release with the least amount of changes (this avoids having
  # to ask for where this perl was branched from).
  for my $previous (reverse sort { $a <=> $b } keys %$existing) {
    # Shouldn't happen, but ensure we don't load weird data...
    next if $previous > $version || $previous == $version;
    my $delta = $deltas{$previous} = {};
    ($delta->{changed}, $delta->{removed}) = calculate_delta(
      $existing->{$previous}, \%versions);
  }

  my $smallest = (sort {
      ((keys($deltas{$a}->{changed}->%*) + keys($deltas{$a}->{removed}->%*)) <=>
       (keys($deltas{$b}->{changed}->%*) + keys($deltas{$b}->{removed}->%*))) ||
      $b <=> $a
    } keys %deltas)[0];

  return {
    delta_from => $smallest,
    changed => $deltas{$smallest}{changed},
    removed => $deltas{$smallest}{removed},
  }
}

sub make_coreutils_delta {
  my($version, $lines) = @_;
  # Trust core perl, if someone does use a weird version number the worst that
  # can happen is an extra delta entry for a module.
  my %utilities = map { $_ => eval $lines->{$_} } keys %$lines;

  # Ensure we have the corelist data loaded from this perl checkout, not the system one.
  require $utils_file;

  my %deltas;
  # Search for the release with the least amount of changes (this avoids having
  # to ask for where this perl was branched from).
  for my $previous (reverse sort { $a <=> $b } keys %Module::CoreList::Utils::utilities) {
    # Shouldn't happen, but ensure we don't load weird data...
    next if $previous > $version || $previous == $version;

    my $delta = $deltas{$previous} = {};
    ($delta->{changed}, $delta->{removed}) = calculate_delta(
      $Module::CoreList::Utils::utilities{$previous}, \%utilities);
  }

  my $smallest = (sort {
      ((keys($deltas{$a}->{changed}->%*) + keys($deltas{$a}->{removed}->%*)) <=>
       (keys($deltas{$b}->{changed}->%*) + keys($deltas{$b}->{removed}->%*))) ||
      $b <=> $a
    } keys %deltas)[0];

  return {
    delta_from => $smallest,
    changed => $deltas{$smallest}{changed},
    removed => $deltas{$smallest}{removed},
  }
}

# Calculate (changed, removed) modules between two versions.
sub calculate_delta {
  my($from, $to) = @_;
  my(%changed, %removed);

  for my $package(keys %$from) {
    if(not exists $to->{$package}) {
      $removed{$package} = 1;
    }
  }

  for my $package(keys %$to) {
    if(!exists $from->{$package}
        || (defined $from->{$package} && !defined $to->{$package})
        || (!defined $from->{$package} && defined $to->{$package})
        || (defined $from->{$package} && defined $to->{$package}
            && $from->{$package} ne $to->{$package})) {
      $changed{$package} = $to->{$package};
    }
  }

  return \%changed, \%removed;
}

sub quote {
    my ($str) = @_;
    # There's gotta be something already doing this properly that we could just
    # reuse, but I can't quite thing of where to look for it, so I'm gonna do
    # the simplest possible thing that'll allow me to release 5.17.7.  --rafl
    $str =~ s/'/\\'/g;
    "'${str}'";
}

sub parse_utils_lst {
  require File::Spec::Unix;
  my @scripts;
  open my $fh, '<', 'utils.lst' or die "$!\n";
  while (<$fh>) {
    chomp;
    my ($file,$extra) = split m!#!;
    $file =~ s!\s+!!g;
    push @scripts, $file;
    $extra =~ s!\s+!!g if $extra;
    if ( $extra and my ($link) = $extra =~ m!^link=(.+?)$! ) {
      push @scripts, $link;
    }
  }
  return map { +( File::Spec::Unix->splitpath( $_ ) )[-1] } @scripts;
}
