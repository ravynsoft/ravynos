################################################################################
#
#  devtools.pl -- various utility functions
#
#  NOTE: This will only be called by the overarching (modern) perl
#
################################################################################
#
#  Version 3.x, Copyright (C) 2004-2013, Marcus Holland-Moritz.
#  Version 2.x, Copyright (C) 2001, Paul Marquess.
#  Version 1.x, Copyright (C) 1999, Kenneth Albanowski.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#
################################################################################

use Data::Dumper;
$Data::Dumper::Sortkeys = 1;
use IO::File;
use warnings;   # Can't use strict because of %opt passed from caller
require "./parts/inc/inctools";

eval "use Term::ANSIColor";
$@ and eval "sub colored { pop; @_ }";

my @argvcopy = @ARGV;

sub verbose
{
  if ($opt{verbose}) {
    my @out = @_;
    s/^(.*)/colored("($0) ", 'bold blue').colored($1, 'blue')/eg for @out;
    print STDERR @out;
  }
}

sub ddverbose
{
  return $opt{verbose} ? ('--verbose') : ();
}

sub runtool
{
  my $opt = ref $_[0] ? shift @_ : {};
  my($prog, @args) = @_;
  my $sysstr = join ' ', map { "'$_'" } $prog, @args;
  $sysstr .= " >$opt->{'out'}"  if exists $opt->{'out'};
  $sysstr .= " 2>$opt->{'err'}" if exists $opt->{'err'};
  verbose("running $sysstr\n");
  my $rv = system $sysstr;
  verbose("$prog => exit code $rv\n");
  return not $rv;
}

sub runperl
{
  my $opt = ref $_[0] ? shift @_ : {};
  runtool($opt, $^X, @_);
}

sub run
{
  my $prog = shift;
  my @args = @_;

  runtool({ 'out' => 'tmp.out', 'err' => 'tmp.err' }, $prog, @args);

  my $out = IO::File->new("tmp.out") or die "tmp.out: $!\n";
  my $err = IO::File->new("tmp.err") or die "tmp.err: $!\n";

  my %rval = (
    status    => $? >> 8,
    stdout    => [<$out>],
    stderr    => [<$err>],
    didnotrun => 0,         # Note that currently this will always be 0
                            # This must have been used in earlier versions
  );

  unlink "tmp.out", "tmp.err";

  $? & 128 and $rval{core}   = 1;
  $? & 127 and $rval{signal} = $? & 127;

  # This is expected and isn't an error.
  @{$rval{stderr}} = grep { $_ !~ /make.*No rule .*realclean/ } @{$rval{stderr}};

  if (    exists $rval{core}
      ||  exists $rval{signal}
      || ($opt{debug} > 2 && @{$rval{stderr}} && $rval{status})
      || ($opt{debug} > 3 && @{$rval{stderr}})
      || ($opt{debug} > 4 && @{$rval{stdout}}))
  {
    print STDERR "Returning\n", Dumper \%rval;

    # Under verbose, runtool already output the call string
    unless ($opt{verbose}) {
        print STDERR "from $prog ", join ", ", @args;
        print STDERR "\n";
    }
  }

  return \%rval;
}

sub ident_str
{
  return "$^X $0 @argvcopy";
}

sub identify
{
  verbose(ident_str() . "\n");
}

sub ask($)
{
  my $q = shift;
  my $a;
  local $| = 1;
  do {
    print "\a\n$q [y/n] ";
    return unless -t;   # Fail if no tty input
    $a = <>; }
  while ($a !~ /^\s*([yn])\s*$/i);
  return lc $1 eq 'y';
}

sub quit_now
{
  print "\nSorry, cannot continue.\a\n\n";
  exit 1;
}

sub ask_or_quit
{
  quit_now unless &ask;
}

sub eta
{
  my($start, $i, $n) = @_;
  return "--:--:--" if $i < 3;
  my $elapsed = tv_interval($start);
  my $h = int($elapsed*($n-$i)/$i);
  my $s = $h % 60; $h /= 60;
  my $m = $h % 60; $h /= 60;
  return sprintf "%02d:%02d:%02d", $h, $m, $s;
}

sub get_and_sort_perls($)
{
    my $opt = shift;

    my $starting;
    $starting = int_parse_version($opt->{'debug-start'})
                                                       if $opt->{'debug-start'};
    my $skip_devels = $opt->{'skip-devels'} // 0;

    # Uses the opt structure parameter to find the perl versions to use this
    # run, and returns an array with a hash representing blead in the 0th
    # element and the oldest in the final one.  Each entry looks like
    #     {
    #       'version' => '5.031002',
    #       'file' => '5031002',
    #       'path' => '/home/khw/devel/bin/perl5.31.2'
    #     },
    #
    # Get blead and all other perls
    my @perls = $opt->{blead};
    for my $dir (split ",", $opt->{install}) {
        push @perls, grep !/-RC\d+/, glob "$dir/bin/perl5.*";
    }

    # Normalize version numbers into 5.xxxyyy, and convert each element
    # describing the perl to be a hash with keys 'version' and 'path'
    for (my $i = 0; $i < @perls; $i++) {
        my $version = `$perls[$i] -e 'print \$]'`;
        my $file = int_parse_version($version);
        $version = format_version($version);

        if ($skip_devels) {
            my ($super, $major, $minor) = parse_version($version);

            # If skipping development releases, we still use blead (0th entry).
            # Devel releases are odd numbered ones 5.6 and above, but use every
            # release for below 5.6
            if ($i != 0 && $major >= 6 && $major % 2 != 0) {
                splice @perls, $i, 1;
                last if $i >= @perls;
                redo;
            }
        }

        # Make this entry a hash with its version, file name, and path
        $perls[$i] = { version =>  $version,
                       file    =>  $file,
                       path    =>  $perls[$i],
                     };
    }

    # Sort in descending order.  We start processing the most recent perl
    # first.
    @perls = sort { $b->{file} <=> $a->{file} } @perls;

    # Override blead's version if specified.
    if (exists $opt->{'blead-version'}) {
        $perls[0]{version} = format_version($opt->{'blead-version'});
    }

    my %seen;

    # blead's todo is its version plus 1.  Otherwise, each todo is the
    # previous one's.  Also get rid of duplicate versions.
    $perls[0]{todo} = $perls[0]{file} + 1;
    $seen{$perls[0]{file}} = 1;
    for my $i (1 .. $#perls) {
        last unless defined $perls[$i];
        if (    exists $seen{$perls[$i]{file}}
            || ($starting && $perls[$i]{file} gt $starting)
        ) {
            splice @perls, $i, 1;
            redo;
        }

        $seen{$perls[$i]{file}} = 1;
        $perls[$i]{todo} = $perls[$i-1]{file};
    }

    # The earliest perl gets a special marker key, consisting of the proper
    # file name
    $perls[$#perls]{final} = $perls[$#perls]{file};

    if ($opt{debug}) {
        print STDERR "The perls returned are: ", Dumper \@perls;
    }

    return \@perls;
}

1;
