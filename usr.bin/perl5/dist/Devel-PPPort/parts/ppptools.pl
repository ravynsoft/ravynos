################################################################################
#
#  ppptools.pl -- various utility functions
#
#  WARNING: This will be called by old perls.  You can't use modern constructs
#  in it.
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

require './parts/inc/inctools';

sub cat_file
{
  eval { require File::Spec };
  return $@ ? join('/', @_) : File::Spec->catfile(@_);
}

sub all_files_in_dir
{
  my $dir = shift;
  local *DIR;

  opendir DIR, $dir or die "cannot open directory $dir: $!\n";
  my @files = grep { !-d && !/^\./ } readdir DIR;  # no dirs or hidden files
  closedir DIR;

  return map { cat_file($dir, $_) } sort @files;
}

sub parse_todo
{
  # Creates a hash with the keys being all symbols found in all the files in
  # the input directory (default 'parts/todo'), and the values being each a
  # subhash like so:
  #     'utf8_hop_forward' => {
  #                               'code' => 'U',
  #                               'version' => '5.025007'
  #                           },
  #
  # The input line that generated that was this:
  #
  #     utf8_hop_forward               # U

  my $dir = shift || 'parts/todo';
  local *TODO;
  my %todo;
  my $todo;

  for $todo (all_files_in_dir($dir)) {
    open TODO, $todo or die "cannot open $todo: $!\n";
    my $version = <TODO>;
    chomp $version;
    while (<TODO>) {
      chomp;
      s/#(?: (\w)\b)?.*//;  # 'code' is optional
      my $code = $1;
      s/^\s+//; s/\s+$//;
      /^\s*$/ and next;
      /^\w+$/ or die "parse_todo: invalid identifier in $todo: $_\n";
      exists $todo{$_} and die "parse_todo: duplicate identifier in $todo: $_ ($todo{$_} <=> $version)\n";
      $todo{$_}{'version'} = $version;
      $todo{$_}{'code'} = $code if $code;
    }
    close TODO;
  }

  return \%todo;
}

sub expand_version
{
  my($op, $ver) = @_;
  my($r, $v, $s) = parse_version($ver);
  $r =~ / ^ [57] $ /x  or die "only Perl revisions [57] are supported\n";
  my $bcdver = sprintf "0x%d%03d%03d", $r, $v, $s;
  return "(PERL_BCDVERSION $op $bcdver)";
}

sub parse_partspec
{
  my $file = shift;
  my $section = 'implementation';

  my $vsec = join '|', qw( provides dontwarn implementation
                           xsubs xsinit xsmisc xshead xsboot tests );
  my(%data, %options);
  local *F;

  open F, $file or die "$file: $!\n";
  while (<F>) {
    /[ \t]+$/ and warn "$file:$.: warning: trailing whitespace\n";
    if ($section eq 'implementation') {
      m!//! && !m!(?:=~|s/).*//! && !m!(?:ht|f)tp(?:s)://!
          and warn "$file:$.: warning: potential C++ comment\n";
    }

    /^##/ and next;

    if (/^=($vsec)(?:\s+(.*))?/) {
      $section = $1;
      if (defined $2) {
        my $opt = $2;
        $options{$section} = eval "{ $opt }";
        $@ and die "$file:$.: invalid options ($opt) in section $section: $@\n";
      }
      next;
    }
    push @{$data{$section}}, $_;
  }
  close F;

  for (keys %data) {
    my @v = @{$data{$_}};
    shift @v while @v && $v[0]  =~ /^\s*$/;
    pop   @v while @v && $v[-1] =~ /^\s*$/;
    $data{$_} = join '', @v;
  }

  if (! exists $data{provides}) {
    if ($file =~ /inctools$/) { # This file is special, it doesn't 'provide'
                                # any API, but has subs to use internally
      $data{provides} = "";
    }
    else {
      $data{provides} = ($file =~ /(\w+)\.?$/)[0];
    }
  }
  $data{provides} = [$data{provides} =~ /(\S+)/g];

  if (exists $data{dontwarn}) {
    $data{dontwarn} = [$data{dontwarn} =~ /(\S+)/g];
  }

  my @prov;
  my %proto;

  if (exists $data{tests} && (!exists $data{implementation} || $data{implementation} !~ /\S/)) {
    $data{implementation} = '';
  }
  else {
    $data{implementation} =~ /\S/ or die "Empty implementation in $file\n";

    my $p;

    for $p (@{$data{provides}}) {
      if ($p =~ m#^/.*/\w*$#) {
        my @tmp = eval "\$data{implementation} =~ ${p}gm";
        $@ and die "invalid regex $p in $file\n";
        @tmp or warn "no matches for regex $p in $file\n";
        push @prov, do { my %h; grep !$h{$_}++, @tmp };
      }
      elsif ($p eq '__UNDEFINED__') {
        my @tmp = $data{implementation} =~ /^\s*__UNDEFINED__[^\r\n\S]+(\w+)/gm;
        @tmp or warn "no __UNDEFINED__ macros in $file\n";
        push @prov, @tmp;
      }
      else {
        push @prov, $p;
      }
    }

    for (@prov) {
      if ($data{implementation} !~ /\b\Q$_\E\b/) {
        warn "$file claims to provide $_, but doesn't seem to do so\n";
        next;
      }

      # scan for prototypes
      my($proto) = $data{implementation} =~ /
                   ( ^ (?:[\w*]|[^\S\r\n])+
                       [\r\n]*?
                     ^ \b$_\b \s*
                       \( [^{]* \)
                   )
                       \s* \{
                   /xm or next;

      $proto =~ s/^\s+//;
      $proto =~ s/\s+$//;
      $proto =~ s/\s+/ /g;

      exists $proto{$_} and warn "$file: duplicate prototype for $_\n";
      $proto{$_} = $proto;
    }
  }

  for $section (qw( implementation xsubs xsinit xsmisc xshead xsboot )) {
    if (exists $data{$section}) {
      $data{$section} =~ s/\{\s*version\s*(<|>|==|!=|>=|<=)\s*([\d._]+)\s*\}/expand_version($1, $2)/gei;
    }
  }

  $data{provides}   = \@prov;
  $data{prototypes} = \%proto;
  $data{OPTIONS}    = \%options;

  my %prov     = map { ($_ => 1) } @prov;
  my %dontwarn = exists $data{dontwarn} ? map { ($_ => 1) } @{$data{dontwarn}} : ();
  my @maybeprov = do { my %h;
                       grep {
                         my($nop) = /^Perl_(.*)/;
                         not exists $prov{$_}                         ||
                             exists $dontwarn{$_}                     ||
                             /^D_PPP_/                                ||
                             (defined $nop && exists $prov{$nop}    ) ||
                             (defined $nop && exists $dontwarn{$nop}) ||
                             $h{$_}++;
                       }
                       $data{implementation} =~ /^\s*#\s*define\s+(\w+)/gm };

  if (@maybeprov) {
    warn "$file seems to provide these macros, but doesn't list them:\n  "
         . join("\n  ", @maybeprov) . "\n";
  }

  return \%data;
}

sub compare_prototypes
{
  my($p1, $p2) = @_;
  for ($p1, $p2) {
    s/^\s+//;
    s/\s+$//;
    s/\s+/ /g;
    s/(\w)\s(\W)/$1$2/g;
    s/(\W)\s(\w)/$1$2/g;
  }
  return $p1 cmp $p2;
}

sub ppcond
{
  my $s = shift;
  my @c;
  my $p;

  for $p (@$s) {
    push @c, map "!($_)", @{$p->{pre}};
    defined $p->{cur} and push @c, "($p->{cur})";
  }

  join " && ", @c;
}

sub trim_arg        # Splits the argument into type and name, returning the
                    # pair: (type, name)
{
  my $in = shift;
  my $remove = join '|', qw( NN NULLOK VOL );

  $in eq '...' and return ($in);

  local $_ = $in;
  my $name;                 # Work on the name

  s/[*()]/ /g;              # Get rid of this punctuation
  s/ \[ [^\]]* \] / /xg;    # Get rid of dimensions
  s/\b(?:auto|const|extern|inline|register|static|volatile|restrict)\b//g;
  s/\b(?:$remove)\b//;
  s/^\s+//; s/\s+$//;       # No leading, trailing space

  if( /^\b (?:struct|union|enum) \s+ \w+ (?: \s+ ( \w+ ) )? $/x ) {
    defined $1 and $name = $1;    # Extract the name for one of these declarations
  }
  else {
    if( s/\b(?:char|double|float|int|long|short|signed|unsigned|void)\b//g ) {
      /^ \s* (\w+) \s* $/x and $name = $1;    # Similarly for these
    }
    elsif (/^ \s* " [^"]+ " \s+ (\w+) \s* $/x) { # A literal string (is special)
        $name = $1;
    }
    else {
      /^ \s* \w+ \s+ (\w+) \s* $/x and $name = $1; # Everything else.
    }
  }

  $_ = $in;     # Now work on the type.

  # Get rid of the name if we found one
  defined $name and s/\b$name\b//;

  # these don't matter at all; note that const does matter
  s/\b(?:auto|extern|inline|register|static|volatile|restrict)\b//g;
  s/\b(?:$remove)\b//;

  while (s/ \* \s+ \* /**/xg) {}  # No spaces within pointer sequences
  s/ \s* ( \*+ ) \s* / $1 /xg;    # Normalize pointer sequences to be surrounded
                                  # by a single space
  s/^\s+//; s/\s+$//;             # No leading, trailing spacd
  s/\s+/ /g;                      # Collapse multiple space into one

  return ($_, $name) if defined $name;
  return $_;
}

sub parse_embed
{
  my @files = @_;
  my @func;
  my @pps;
  my $file;
  local *FILE;

  for $file (@files) {
    open FILE, $file or die "$file: $!\n";
    my($line, $l);

    while (defined($line = <FILE>)) {
      while ($line =~ /\\$/ && defined($l = <FILE>)) {
        $line =~ s/\\\s*//;
        $line .= $l;
      }
      next if $line =~ /^\s*:/;
      $line =~ s/^\s+|\s+$//gs;
      my($dir, $args) = ($line =~ /^\s*#\s*(\w+)(?:\s*(.*?)\s*)?$/);
      if (defined $dir and defined $args) {
        for ($dir) {
          /^ifdef$/   and do { push @pps, { pre => [], cur => "defined($args)"  }         ; last };
          /^ifndef$/  and do { push @pps, { pre => [], cur => "!defined($args)" }         ; last };
          /^if$/      and do { push @pps, { pre => [], cur => $args             }         ; last };
          /^elif$/    and do { push @{$pps[-1]{pre}}, $pps[-1]{cur}; $pps[-1]{cur} = $args; last };
          /^else$/    and do { push @{$pps[-1]{pre}}, $pps[-1]{cur}; $pps[-1]{cur} = undef; last };
          /^endif$/   and do { pop @pps                                                   ; last };
          /^include$/ and last;
          /^define$/  and last;
          /^undef$/   and last;
          warn "unhandled preprocessor directive: $dir\n";
        }
      }
      else {
        my @e = split /\s*\|\s*/, $line;
        if( @e >= 3 ) {
          my($flags, $ret, $name, @args) = @e;

          # Skip non-name entries, like
          #    PL_parser-E<gt>linestr
          # which documents a struct entry rather than a function.  We retain
          # all other entries, so that our caller has full information, and
          # may skip things like non-public functions.
          next if $flags =~ /N/;

          # M implies m for the purposes of this module.
          $flags .= 'm' if $flags =~ /M/;

          # An entry marked 'b' is in mathoms, so is effectively deprecated,
          # as it can be removed at anytime.  But if it also has a macro to
          # implement it, that macro stays when mathoms is removed, so the
          # non-'Perl_' form isn't deprecated.  embed.fnc is supposed to have
          # already set this up, but make sure.
          if ($flags =~ /b/ && $flags !~ /m/ && $flags !~ /D/) {
            warn "Expecting D flag for '$name', since it is b without [Mm]";
            $flags .= 'D';
          }

          if ($name =~ /^[^\W\d]\w*$/) {
            my $cond = ppcond(\@pps);
            if ($cond =~ /defined\(PERL_IN_[A-Z0-9_]+_[CH]/ && $flags =~ /A/)
            {
                warn "$name marked as API, but restricted scope: $cond\n";
            }
            #warn "$name: $cond" if length $cond && $flags =~ /A/;
            for (@args) {
              $_ = [trim_arg($_)];
            }
            ($ret) = trim_arg($ret);
            push @func, {
              name  => $name,
              flags => { map { $_, 1 } $flags =~ /./g },
              ret   => $ret,
              args  => \@args,
              cond  => $cond,
            };
            $func[-1]{'ppport_fnc'} = 1 if $file =~ /ppport\.fnc/;
          }
          elsif ($flags !~ /y/) {
            warn "mysterious name [$name] in $file, line $.\n";
          }
        }
      }
    }

    close FILE;
  }

  # Here's what two elements of the array look like:
  # {
  #              'args' => [
  #                          [
  #                            'const nl_item',
  #                            'item'
  #                          ]
  #                        ],
  #              'cond' => '(defined(HAS_NL_LANGINFO) && defined(PERL_LANGINFO_H))',
  #              'flags' => {
  #                           'A' => 1,
  #                           'T' => 1,
  #                           'd' => 1,
  #                           'o' => 1
  #                         },
  #              'name' => 'Perl_langinfo',
  #              'ret' => 'const char *'
  #            },
  #            {
  #              'args' => [
  #                          [
  #                            'const int',
  #                            'item'
  #                          ]
  #                        ],
  #              'cond' => '!(defined(HAS_NL_LANGINFO) && defined(PERL_LANGINFO_H))',
  #              'flags' => {
  #                           'A' => 1,
  #                           'T' => 1,
  #                           'd' => 1,
  #                           'o' => 1
  #                         },
  #              'name' => 'Perl_langinfo',
  #              'ret' => 'const char *'
  #            },

  return @func;
}

sub known_but_hard_to_test_for
{
    # This returns a list of functions/symbols that are in Perl, but the tests
    # for their existence don't work, usually as a result of them being XS,
    # and using XS to test.  Effectively, any XS code that compiles and works
    # is exercising most of these XS-related ones.
    #
    # The values for the keys are each the version that ppport.h makes them
    # work on, and were gleaned by manually looking at the code parts/inc/*.
    # For functions, scanprov will automatically figure out the version
    # they were introduced in.

    my %return;





for (qw(CLASS CPERLscope dMY_CXT_SV dXSI32 END_EXTERN_C EXTERN_C items
        ix PERL_USE_GCC_BRACE_GROUPS PL_hexdigit pTHX_ PTRV
        RETVAL START_EXTERN_C STMT_END STMT_START StructCopy
        STR_WITH_LEN svtype THIS XS XSPROTO))
    {
        # __MIN_PERL__ is this at the time of this commit.  This is the
        # earliest these have been tested to at the time of the commit, but
        # likely go back further.
        $return{$_} = '5.003_07';
    }
    for (qw(_pMY_CXT pMY_CXT_)) {
        $return{$_} = '5.9.0';
    }
    for (qw(PERLIO_FUNCS_DECL)) {
        $return{$_} = '5.9.3';
    }
    for (qw(XopDISABLE XopENABLE XopENTRY XopENTRYCUSTOM XopENTRY_set)) {
        $return{$_} = '5.13.7';
    }
    for (qw(XS_EXTERNAL XS_INTERNAL)) {
        $return{$_} = '5.15.2';
    }

    return \%return;
}

sub normalize_prototype  # So that they can be compared more easily
{
    my $proto = shift;
    $proto =~ s/\s* \* \s* / * /xg;
    return $proto;
}

sub make_prototype
{
  my $f = shift;
  my @args = map { "@$_" } @{$f->{args}};
  my $proto;
  my $pTHX_ = exists $f->{flags}{T} ? "" : "pTHX_ ";
  $proto = "$f->{ret} $f->{name}" . "($pTHX_" . join(', ', @args) . ')';
  return normalize_prototype($proto);
}
1;
