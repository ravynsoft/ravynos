#!/usr/bin/perl -w
################################################################################
#
#  mktodo.pl -- generate baseline and todo files
#
# It makes the todo file for the single passed in perl binary.  If --base is
# not specified it compiles with ppport.h.
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

use strict;
use Getopt::Long;
use Data::Dumper;
use IO::File;
use IO::Select;
use Config;
use Time::HiRes qw( gettimeofday tv_interval );

require './devel/devtools.pl';

our %opt = (
  blead     => 0,     # ? Is this perl blead
  debug     => 0,
  base      => 0,     # Don't use ppport.h when generating
  verbose   => 0,
  check     => 1,
  final     => "",
 'todo-dir' => "",
  todo      => "",    # If no --todo, this is a blead perl
  shlib     => 'blib/arch/auto/Devel/PPPort/PPPort.so',
);

GetOptions(\%opt, qw(
perl=s todo=i blead todo-dir=s version=s shlib=s debug=i base final=s verbose check!
          )) or die;

identify();

my $todo_file;
my $todo_version;
if ($opt{todo}) {
    $todo_file = "$opt{'todo-dir'}/$opt{todo}";
    $todo_version = format_version_line($opt{todo});
}

# Pass this through the Make, to apicheck.pl
$ENV{'DPPP_ARGUMENTS'} = "--todo-dir=$opt{'todo-dir'} --todo=$todo_version";

my $test_name_re =   qr/ \b DPPP_test_ (?: \d _ )? (\w+) \b /x;

print "\n", ident_str(), "\n\n";

my $fullperl = `which $opt{perl}`;
chomp $fullperl;

$ENV{SKIP_SLOW_TESTS} = 1;

# Generate the Makefile using the passed in perl
regen_Makefile();

# List of functions that are never considered undefined.  Add to as necessary
my %stdsym = map { ($_ => 1) } qw (
  acos
  acosl
  acosq
  asin
  asinl
  asinq
  atan
  atan2
  atan2l
  atan2q
  atanl
  atanq
  ceil
  ceill
  ceilq
  cos
  cosh
  coshl
  coshq
  cosl
  cosq
  exit
  exp
  expl
  expq
  floor
  floorl
  floorq
  fmod
  fmodl
  fmodq
  log
  log10
  log10l
  log10q
  logl
  logq
  memcmp
  memcpy
  memmove
  memset
  pow
  powl
  powq
  siglongjmp
  sin
  sinh
  sinhl
  sinhq
  sinl
  sinq
  snprintf
  sprintf
  sqrt
  sqrtl
  sqrtq
  strcmp
  strlen
  strncmp
  tan
  tanh
  tanhl
  tanhq
  tanl
  tanq
  tolower
  vsnprintf
);

# Initialize %sym so that the keys are all the Text symbols for this perl,
# output from the system's 'nm'
my %sym;
for (`$Config{nm} $fullperl`) {
  chomp;
  /\s+T\s+(\w+)\s*$/ and $sym{$1}++;
}
keys %sym >= 50 or die "less than 50 symbols found in $fullperl\n";

# %todo is initialized to be the symbols in the current todo file, like so:
# {
#   'UTF8_SAFE_SKIP' => 'U',
#   'newSVsv_flags' => 'U',
#   'newSVsv_nomg' => 'U',
# }
#
# The values are the outputs from nm, plus 'E' from us, for Error
my %todo = %{load_todo($todo_file, $todo_version)} if $opt{todo};

my @recheck;

# Get an exhaustive list from apicheck.i of symbols, what functions contain
# them, and how many in each function.
# symbol        fcn            count
# ------        ---            -----
# 'UV' => {
#             'SvUVX'          => 1,
#             'toFOLD_uvchr'   => 2,
#             'sv_uni_display' => 1,
#             ...
# }
my $symmap = get_apicheck_symbol_map();

# In each iteration of the loop we create an apicheck.c.  This will contain a
# generated wrapper function for each API function and macro.  The wrapper
# contains one or more calls to its API element.  Then we attempt to compile
# apicheck.c into apicheck.o.  If it compiles, then every API element exists
# in this version of perl.  If not, we figure out which ones were undefined,
# and set things up so that in the next iteration of the loop, the wrappers
# for those elements are #ifdef'd out.
for (;;) {
  my $retry = 1;
  my $trynm = 1;

  regen_apicheck();

retry:
  my(@new, @already_in_sym, %seen);

  my $r = run(qw(make));
  $r->{didnotrun} and die "couldn't run make: $!\n" .
        join('', @{$r->{stdout}})."\n---\n".join('', @{$r->{stderr}});

  # If there were warnings, we ask the user before continuing when creating
  # the base files of blead.  This leads to a potential early exit when things
  # aren't working right.
  my $is_blead = 0;
  if ($opt{blead} && $opt{base}) {
    undef $opt{blead};  # Only warn once.
    $is_blead = 1;      # But let the code below know
    if (@{$r->{stderr}}) {
        print STDERR "Warnings and errors from compiling blead:\n";
        print STDERR @{$r->{stderr}};
        ask_or_quit("\nUnexpected warnings when compiling blead can lead to"
                  . " wrong results.  Please examine the above list.\n"
                  . "Shall I proceed?");
    }
    else {
        print STDERR "blead compiled without warnings nor errors.\n"
                   . "Proceeding with everything else\n\n";
    }
  }

  # Examine stderr.  For each wrapper function listed in it, we create an
  # 'E' (for error) entry.   If the function (possibly prefixed by '[Pp]erl')
  # is in %sym, it is added to @already_in_sym.  Otherwise, @new.
  for my $l (@{$r->{stderr}}) {
    if ($l =~ $test_name_re) {
      if (!$seen{$1}++) {
        my @s = grep { exists $sym{$_} } $1, "Perl_$1", "perl_$1";
        if (@s) {
          push @already_in_sym, [$1, "E (@s)"];
        }
        else {
          push @new, [$1, "E"];
        }
      }
    }
  }
  print STDERR __LINE__, ": \@new after make", Dumper \@new if $opt{debug} > 6;

  if ($r->{status} == 0) {
    my @u;
    my @usym;

    # Here, apicheck.o was successfully created.  It likely will need
    # functions from outside it in order to form a complete executable a.out.
    # In the first iteration, look to see if all needed externs are available.
    # (We don't actually try to create an a.out)
    if ($trynm) {
      @u = eval { find_undefined_symbols($fullperl, $opt{shlib}) };
      warn "warning: $@" if $@;
      $trynm = 0;
    }

    # If it didn't find any undefined symbols, everything should be working.
    # Run the test suite.
    unless (@u) {
      $r = run(qw(make test));
      $r->{didnotrun} and die "couldn't run make test: $!\n" .
        join('', @{$r->{stdout}})."\n---\n".join('', @{$r->{stderr}});

      $r->{status} == 0 and last;   # It worked!!

      # Alas, something was wrong.  Add any undefined symbols listed in the
      # output to our list
      for my $l (@{$r->{stderr}}) {
        if ($l =~ /undefined symbol: (\w+)/) {
          push @u, $1;
        }
      }
    }

    # For each undefined symbol
    for my $u (@u) {

      # If this is an API symbol, $symmap->{$u} will exist and be a hash of
      # keys, being all the symbols referred to within it (with their values
      # btw being the count of occurrences in the element).
      for my $m (keys %{$symmap->{$u}}) {

        # pthread_[gs]etspecific() are undefined.  khw doesn't know why; these
        # are Posix functions.  But we have a bunch of things depending on
        # them, so it doesn't work unless we ignore this apparently spurious
        # issue.
        next if $u =~ / ^ pthread_[gs]etspecific $ /x;

        if (!$seen{$m}++) {
          my $pl = $m;
          $pl =~ s/^[Pp]erl_//;
          my @s = grep { exists $sym{$_} } $pl, "Perl_$pl", "perl_$pl";

          # The comment for this entry that goes into the file that gets
          # written includes any [Pp]erl prefix.
          push @new, [$m, @s ? "U (@s)" : "U"];
        }
      }
    }
  }
  print STDERR __LINE__, ": \@new after getting undefs", Dumper \@new
                                                            if $opt{debug} > 6;

  # Remove from @new all the current todo symbols
  @new = grep !$todo{$_->[0]}, @new;
  print STDERR __LINE__, ": \@new after removing current", Dumper \@new
                                                            if $opt{debug} > 6;

  # If none remain, start over with those we know about, minus the todo
  # symbols.  khw doesn't understand why this is necessary
  unless (@new) {
    @new = grep !$todo{$_->[0]}, @already_in_sym;
    print STDERR __LINE__, ": \@new after starting over", Dumper \@new
                                                            if $opt{debug} > 6;
  }

  # This retries once if nothing new was found (khw guesses that is just to
  # be sure, or maybe it's because we ran nm the first time through)
  unless (@new) {
    if ($retry > 0) {
      $retry--;
      regen_Makefile();
      goto retry;
    }
    print Dumper($r);
    die "no new TODO symbols found...";
  }

  # recheck symbols except undefined ones reported by the dynamic linker
  push @recheck, map { $_->[0] } grep { $_->[1] !~ /^U/ } @new;

  # Display each newly found undefined symbol, and add it to the list of todo
  # symbols
  if (@new) {
    for (@new) {
        display_sym('new', @$_);
        $todo{$_->[0]} = $_->[1];
    }

    if ($is_blead) {
        ask_or_quit("\nUndefined symbols in blead indicate a bug in blead\n"
                  . "Shall I proceed?");
    }
  }

  print STDERR __LINE__, ": %todo at end of iteration ", Dumper \%todo
                                                            if $opt{debug} > 6;

  # Write the revised todo, so that apicheck.c when generated in the next
  # iteration will have these #ifdef'd out
  write_todo($todo_file, $todo_version, \%todo);
} # End of loop

# If we are to check our work, do so.  This verifies that each symbol
# identified above is really a problem in this version.  (khw doesn't know
# under what circumstances this becomes an issue)
#
# We go through each symbol on the @recheck list, and create an apicheck.c
# with it enabled.
if ($opt{check}) {

  # Create something like '%3d'
  my $ifmt = '%' . length(scalar @recheck) . 'd';

  my $t0 = [gettimeofday];

  RECHECK: for my $i (0 .. $#recheck) {
    my $sym = $recheck[$i];

    # Assume it will work
    my $cur = delete $todo{$sym};

    # Give a progress report
    display_sym('chk', $sym, $cur, sprintf(" [$ifmt/$ifmt, ETA %s]",
               $i + 1, scalar @recheck, eta($t0, $i, scalar @recheck)));

    # Write out the todo file without this symbol, meaning it will be enabled
    # in the generated apicheck.c file
    write_todo($todo_file, $todo_version, \%todo);

    # E is not an nm symbol, but was added by us to indicate 'Error'
    if ($cur eq "E (Perl_$sym)") {

      # We can try a shortcut here.  Create an apicheck.c file for just this
      # symbol.
      regen_apicheck($sym);

      my $r = run(qw(make test));

      if (!$r->{didnotrun} && $r->{status} == 0) {

        # Shortcut indicated that this function compiles..
        display_sym('del', $sym, $cur);
        next RECHECK;
      }

      # Here, the api file with just this entry failed to compile.  (khw
      # doesn't know why we just don't give up on it now, but we don't.)  We
      # drop down below to generate and compile a full apicheck.c with this
      # symbol enabled.  (XXX Perhaps we could look at stderr and if it
      # contained things about parameter mismatch, (which is a common
      # occurrence), we could skip the steps below.)
    }

    # Either can't shortcut, or the shortcut indicated that the function
    # doesn't compile in isolation.  Create, compile and test with this
    # function/symbol enabled.  (Remember that this should have succeeded
    # above to get to here when this symbol was disabled, so enabling just
    # this one will tell us for sure that it works or doesn't work.  (khw
    # wonders if this is actually a DAG, or perhaps with cycles, so this is
    # under it all, insufficient.)
    regen_Makefile();

    my $r = run(qw(make test));

    # This regenerated apicheck.c
    dump_apicheck() if $opt{debug} > 3;

    $r->{didnotrun} and die "couldn't run make test: $!\n" .
        join('', @{$r->{stdout}})."\n---\n".join('', @{$r->{stderr}});

    if ($r->{status} == 0) {    # This symbol compiles and tests ok, so retain
                                # in this version
      display_sym('del', $sym, $cur);
    }
    else { # Revert to this symbol is bad in this version
      print STDERR __LINE__, ": symbol '$sym' not in this version\n"
                                                            if $opt{debug} > 6;
      $todo{$sym} = $cur;
      write_todo($todo_file, $todo_version, \%todo);
    }
  }
} # End of checking our work

print STDERR __LINE__, ": %todo at end ", Dumper \%todo  if $opt{debug} > 6;
write_todo($todo_file, $todo_version, \%todo);

# If this is the earliest perl being tested, we can extend down our values to
# include it.  (Remember, that we create files for the next higher version,
# but this allows us to create a file for the lowest as well.)  This
# effectively writes out all the known symbols of this earliest version as if
# they came into existence during it.
if ($opt{final}) {
    my $file = "$opt{'todo-dir'}/$opt{final}";
    my $version = format_version_line($opt{final});

    regen_Makefile();
    my $r = run(qw(make));
    $r->{didnotrun} and die "couldn't run make: $!\n" .
        join('', @{$r->{stdout}})."\n---\n".join('', @{$r->{stderr}});

    my $symbols = read_sym(file => $opt{shlib}, options => [qw( --defined-only )]);
    my @stuff = map { $_ =~ $test_name_re } keys %$symbols;
    %todo = map { $_ => 'T' } @stuff;

    print STDERR __LINE__, ": write at ", Dumper $file, $version, \%todo
                                                            if $opt{debug} > 5;
    write_todo($file, $version, \%todo);
}

# Clean up after ourselves
$opt{debug} = 0;    # Don't care about failures
run(qw(make realclean));

exit 0;

sub display_sym
{
  my($what, $sym, $reason, $extra) = @_;
  $extra ||= '';
  my %col = (
    'new' => 'bold red',
    'chk' => 'bold magenta',
    'del' => 'bold green',
  );
  $what = colored("$what symbol", $col{$what});

  printf "[%s] %s %-30s # %s%s\n",
         $todo_version, $what, $sym, $reason, $extra;
}

sub regen_Makefile
{
  # We make sure to add rules for creating apicheck.c
  my @mf_arg = ('--with-apicheck', 'OPTIMIZE=-O0 -w');

  # It doesn't include ppport.h if generating the base files.
  push @mf_arg, qw( DEFINE=-DDPPP_APICHECK_NO_PPPORT_H ) if $opt{base};

  # just to be sure
  my $debug = $opt{debug};
  $opt{debug} = 0;    # Don't care about failures
  run(qw(make realclean));
  $opt{debug} = $debug;

  my $r = run($fullperl, "Makefile.PL", @mf_arg);
  unless ($r->{status} == 0) {
      die "cannot run Makefile.PL: $!\n" .
          join('', @{$r->{stdout}})."\n---\n".join('', @{$r->{stderr}});
  }
}

sub regen_apicheck      # Regeneration can also occur by calling 'make'
{
  unlink qw(apicheck.c apicheck.o);
  runtool({ out => '/dev/null' }, $fullperl, 'apicheck_c.PL', map { "--api=$_" } @_)
      or die "cannot regenerate apicheck.c\n";
  dump_apicheck() if $opt{debug} > 3;
}

sub dump_apicheck
{
    my $apicheck = "apicheck.c";
    my $f = new IO::File $apicheck or die "cannot open $apicheck: $!\n";
    my @lines = <$f>;
    print STDERR __FILE__, ": ", __LINE__, ": $apicheck (",
                                           scalar @lines,
                                           " lines) for $fullperl";
    print STDERR " and '" if @_;
    print STDERR join "', '", @_;
    print STDERR "'" if @_;
    print STDERR ":\n";
    my $n = 1;
    print STDERR $n++, " ", $_ for @lines;
}

sub load_todo   # Return entries from $file; skip if the first line
                # isn't $expver (expected version)
{
  my($file, $expver) = @_;

  if (-e $file) {
    my $f = new IO::File $file or die "cannot open $file: $!\n";
    my $ver = <$f>;
    chomp $ver;
    if ($ver eq $expver) {
      my %sym;
      while (<$f>) {
        chomp;
        /^(\w+)\s+#\s+(.*)/ or goto nuke_file;
        exists $sym{$1} and goto nuke_file;
        $sym{$1} = $2;
      }
      return \%sym;
    }

nuke_file:
    undef $f;
    unlink $file or die "cannot remove $file: $!\n";
  }

  return {};
}

sub write_todo  # Write out the todo file.  The keys of %sym are known to not
                # be in this version, hence are 'todo'
{
  my($file, $ver, $sym) = @_;
  my $f;

  $f = new IO::File ">$file" or die "cannot open $file: $!\n";
  $f->print("$ver\n");

  # Dictionary ordering, with only alphanumerics
  for (sort dictionary_order keys %$sym) {
    $f->print(sprintf "%-30s # %s\n", $_, $sym->{$_});
  }

  $f->close;
}

sub find_undefined_symbols
{
  # returns a list of undefined symbols in $shlib.  To be considered
  # undefined, it must also not be defined in $perl.  Symbols that begin with
  # underscore, or contain '@', or are some libc ones are not returned.
  # Presumably, the list of libc could be expanded if necessary.

  my($perl, $shlib) = @_;

  my $ps = read_sym(file => $perl,  options => [qw( --defined-only   )]);
  my $ls = read_sym(file => $shlib, options => [qw( --undefined-only )]);

  my @undefined;

  for my $sym (keys %$ls) {
    next if $sym =~ /\@/ or $sym =~ /^_/ or exists $stdsym{$sym};
    unless (exists $ps->{$sym}) {
        print STDERR __LINE__, ": , Couldn't find '$sym' in $perl\n"
                                                            if $opt{debug} > 4;
        push @undefined, $sym;
    }
  }

  print STDERR __LINE__, ": find_undef returning ", Dumper \@undefined
                                                            if $opt{debug} > 4;
  return @undefined;
}

sub read_sym
{
  my %opt = ( options => [], @_ );

  my $r = run($Config{nm}, @{$opt{options}}, $opt{file});

  if ($r->{didnotrun} or $r->{status}) {
    die "cannot run $Config{nm}" .
          join('', @{$r->{stdout}})."\n---\n".join('', @{$r->{stderr}});
  }

  my %sym;

  for (@{$r->{stdout}}) {
    chomp;
    my($adr, $fmt, $sym) = /^\s*([[:xdigit:]]+)?\s+([ABCDGINRSTUVW?-])\s+(\S+)\s*$/i
                           or die "cannot parse $Config{nm} output:\n[$_]\n";
    $sym{$sym} = { format => $fmt };
    $sym{$sym}{address} = $adr if defined $adr;
  }

  return \%sym;
}

sub get_apicheck_symbol_map
{
  my $r;

  while (1) {

    # Create apicheck.i
    $r = run(qw(make apicheck.i));

    # Quit the loop if it succeeded
    last unless $r->{didnotrun} or $r->{status};

    # Get the list of macros that had parameter issues.  These are marked as
    # A, for absolute in nm terms
    my $absolute_err = 'A';
    my %sym = map { /error: macro "(\w+)" (?:requires|passed) \d+ argument/
                    ? ($1 => $absolute_err)
                    : ()
                  } @{$r->{stderr}};

    # Display these, and add them to the global %todo.
    if (keys %sym) {
      for my $s (sort dictionary_order keys %sym) {
        if (defined $todo{$s} && $todo{$s} eq $absolute_err) {
            # Otherwise could loop
            die "cannot run make apicheck.i ($r->{didnotrun} / $r->{status}):\n".
                join('', @{$r->{stdout}})."\n---\n".join('', @{$r->{stderr}});
        }
        $todo{$s} = $sym{$s};
        display_sym('new', $s, $sym{$s});
      }

      # And rewrite the todo file, including these new symbols.
      write_todo($todo_file, $todo_version, \%todo);

      # Regenerate apicheck.c for the next iteration
      regen_apicheck();
    }
    else {  # It failed for some other reason than parameter issues: give up
      die "cannot run make apicheck.i ($r->{didnotrun} / $r->{status}):\n".
          join('', @{$r->{stdout}})."\n---\n".join('', @{$r->{stderr}});
    }
  }

  # Here, have an apicheck.i.  Read it in
  my $fh = IO::File->new('apicheck.i')
           or die "cannot open apicheck.i: $!";

  local $_;
  my %symmap;
  my $cur;

  while (<$fh>) {
    print STDERR __LINE__, ": apicheck.i ", $_ if $opt{debug} > 5;
    next if /^#/;

    # We only care about lines within one of our DPPP_test_ functions.  If
    # we're in one, $cur is set to the name of the current one.
    if (! defined $cur) {   # Not within such a function; see if this starts
                            # one
      $_ =~ $test_name_re and $cur = $1;
    }
    else {

      # For anything that looks like a symbol, note it as a key, and as its
      # value, the name of the function.  Actually the value is another key,
      # whose value is the count of this symbol's occurrences, so it looks
      # like:
      # 'UV' => {
      #             'SvUVX' => 1,
      #             'toFOLD_uvchr' => 2,
      #             'sv_uni_display' => 1,
      #             ...
      # }
      for my $sym (/\b([A-Za-z_]\w+)\b/g) {
        $symmap{$sym}{$cur}++;
      }

      # This line marks the end of this function, as constructed by us.
      undef $cur if /^}$/;
    }
  }

  print STDERR __LINE__, ": load_todo returning ", Dumper \%symmap
                                                            if $opt{debug} > 5;
  return \%symmap;
}
