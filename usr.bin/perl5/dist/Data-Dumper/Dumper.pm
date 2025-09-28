#
# Data/Dumper.pm
#
# convert perl data structures into perl syntax suitable for both printing
# and eval
#
# Documentation at the __END__
#

package Data::Dumper;

use strict;
use warnings;

#$| = 1;

use 5.008_001;
require Exporter;

use constant IS_PRE_516_PERL => $] < 5.016;
use constant SUPPORTS_CORE_BOOLS => defined &builtin::is_bool;

use Carp ();

# Globals people alter.
our ( $Indent, $Trailingcomma, $Purity, $Pad, $Varname, $Useqq, $Terse, $Freezer,
      $Toaster, $Deepcopy, $Quotekeys, $Bless, $Maxdepth, $Pair, $Sortkeys,
      $Deparse, $Sparseseen, $Maxrecurse, $Useperl );

our ( @ISA, @EXPORT, @EXPORT_OK, $VERSION );

BEGIN {
    $VERSION = '2.188'; # Don't forget to set version and release
                        # date in POD below!

    @ISA = qw(Exporter);
    @EXPORT = qw(Dumper);
    @EXPORT_OK = qw(DumperX);

    # if run under miniperl, or otherwise lacking dynamic loading,
    # XSLoader should be attempted to load, or the pure perl flag
    # toggled on load failure.
    eval {
        require XSLoader;
        XSLoader::load( 'Data::Dumper' );
        1
    }
    or $Useperl = 1;
}

my $IS_ASCII  = ord 'A' ==  65;

# module vars and their defaults
$Indent     = 2         unless defined $Indent;
$Trailingcomma = 0      unless defined $Trailingcomma;
$Purity     = 0         unless defined $Purity;
$Pad        = ""        unless defined $Pad;
$Varname    = "VAR"     unless defined $Varname;
$Useqq      = 0         unless defined $Useqq;
$Terse      = 0         unless defined $Terse;
$Freezer    = ""        unless defined $Freezer;
$Toaster    = ""        unless defined $Toaster;
$Deepcopy   = 0         unless defined $Deepcopy;
$Quotekeys  = 1         unless defined $Quotekeys;
$Bless      = "bless"   unless defined $Bless;
#$Expdepth   = 0         unless defined $Expdepth;
$Maxdepth   = 0         unless defined $Maxdepth;
$Pair       = ' => '    unless defined $Pair;
$Useperl    = 0         unless defined $Useperl;
$Sortkeys   = 0         unless defined $Sortkeys;
$Deparse    = 0         unless defined $Deparse;
$Sparseseen = 0         unless defined $Sparseseen;
$Maxrecurse = 1000      unless defined $Maxrecurse;

#
# expects an arrayref of values to be dumped.
# can optionally pass an arrayref of names for the values.
# names must have leading $ sign stripped. begin the name with *
# to cause output of arrays and hashes rather than refs.
#
sub new {
  my($c, $v, $n) = @_;

  Carp::croak("Usage:  PACKAGE->new(ARRAYREF, [ARRAYREF])")
    unless (defined($v) && (ref($v) eq 'ARRAY'));
  $n = [] unless (defined($n) && (ref($n) eq 'ARRAY'));

  my($s) = {
        level      => 0,           # current recursive depth
        indent     => $Indent,     # various styles of indenting
        trailingcomma => $Trailingcomma, # whether to add comma after last elem
        pad        => $Pad,        # all lines prefixed by this string
        xpad       => "",          # padding-per-level
        apad       => "",          # added padding for hash keys n such
        sep        => "",          # list separator
        pair       => $Pair,    # hash key/value separator: defaults to ' => '
        seen       => {},          # local (nested) refs (id => [name, val])
        todump     => $v,          # values to dump []
        names      => $n,          # optional names for values []
        varname    => $Varname,    # prefix to use for tagging nameless ones
        purity     => $Purity,     # degree to which output is evalable
        useqq      => $Useqq,      # use "" for strings (backslashitis ensues)
        terse      => $Terse,      # avoid name output (where feasible)
        freezer    => $Freezer,    # name of Freezer method for objects
        toaster    => $Toaster,    # name of method to revive objects
        deepcopy   => $Deepcopy,   # do not cross-ref, except to stop recursion
        quotekeys  => $Quotekeys,  # quote hash keys
        'bless'    => $Bless,    # keyword to use for "bless"
#        expdepth   => $Expdepth,   # cutoff depth for explicit dumping
        maxdepth   => $Maxdepth,   # depth beyond which we give up
	maxrecurse => $Maxrecurse, # depth beyond which we abort
        useperl    => $Useperl,    # use the pure Perl implementation
        sortkeys   => $Sortkeys,   # flag or filter for sorting hash keys
        deparse    => $Deparse,    # use B::Deparse for coderefs
        noseen     => $Sparseseen, # do not populate the seen hash unless necessary
       };

  if ($Indent > 0) {
    $s->{xpad} = "  ";
    $s->{sep} = "\n";
  }
  return bless($s, $c);
}

# Packed numeric addresses take less memory. Plus pack is faster than sprintf

sub format_refaddr {
    require Scalar::Util;
    pack "J", Scalar::Util::refaddr(shift);
};

#
# add-to or query the table of already seen references
#
sub Seen {
  my($s, $g) = @_;
  if (defined($g) && (ref($g) eq 'HASH'))  {
    my($k, $v, $id);
    while (($k, $v) = each %$g) {
      if (defined $v) {
        if (ref $v) {
          $id = format_refaddr($v);
          if ($k =~ /^[*](.*)$/) {
            $k = (ref $v eq 'ARRAY') ? ( "\\\@" . $1 ) :
                 (ref $v eq 'HASH')  ? ( "\\\%" . $1 ) :
                 (ref $v eq 'CODE')  ? ( "\\\&" . $1 ) :
                 (   "\$" . $1 ) ;
          }
          elsif ($k !~ /^\$/) {
            $k = "\$" . $k;
          }
          $s->{seen}{$id} = [$k, $v];
        }
        else {
          Carp::carp("Only refs supported, ignoring non-ref item \$$k");
        }
      }
      else {
        Carp::carp("Value of ref must be defined; ignoring undefined item \$$k");
      }
    }
    return $s;
  }
  else {
    return map { @$_ } values %{$s->{seen}};
  }
}

#
# set or query the values to be dumped
#
sub Values {
  my($s, $v) = @_;
  if (defined($v)) {
    if (ref($v) eq 'ARRAY')  {
      $s->{todump} = [@$v];        # make a copy
      return $s;
    }
    else {
      Carp::croak("Argument to Values, if provided, must be array ref");
    }
  }
  else {
    return @{$s->{todump}};
  }
}

#
# set or query the names of the values to be dumped
#
sub Names {
  my($s, $n) = @_;
  if (defined($n)) {
    if (ref($n) eq 'ARRAY') {
      $s->{names} = [@$n];         # make a copy
      return $s;
    }
    else {
      Carp::croak("Argument to Names, if provided, must be array ref");
    }
  }
  else {
    return @{$s->{names}};
  }
}

sub DESTROY {}

sub Dump {
  return &Dumpxs
    unless $Data::Dumper::Useperl || (ref($_[0]) && $_[0]->{useperl})
            # Use pure perl version on earlier releases on EBCDIC platforms
        || (! $IS_ASCII && $] lt 5.021_010);
  return &Dumpperl;
}

#
# dump the refs in the current dumper object.
# expects same args as new() if called via package name.
#
our @post;
sub Dumpperl {
  my($s) = shift;
  my(@out, $val, $name);
  my($i) = 0;
  local(@post);

  $s = $s->new(@_) unless ref $s;

  for $val (@{$s->{todump}}) {
    @post = ();
    $name = $s->{names}[$i++];
    $name = $s->_refine_name($name, $val, $i);

    my $valstr;
    {
      local($s->{apad}) = $s->{apad};
      $s->{apad} .= ' ' x (length($name) + 3) if $s->{indent} >= 2 and !$s->{terse};
      $valstr = $s->_dump($val, $name);
    }

    $valstr = "$name = " . $valstr . ';' if @post or !$s->{terse};
    my $out = $s->_compose_out($valstr, \@post);

    push @out, $out;
  }
  return wantarray ? @out : join('', @out);
}

# wrap string in single quotes (escaping if needed)
sub _quote {
    my $val = shift;
    $val =~ s/([\\\'])/\\$1/g;
    return  "'" . $val .  "'";
}

# Old Perls (5.14-) have trouble resetting vstring magic when it is no
# longer valid.
use constant _bad_vsmg => defined &_vstring && (_vstring(~v0)||'') eq "v0";

#
# twist, toil and turn;
# and recurse, of course.
# sometimes sordidly;
# and curse if no recourse.
#
sub _dump {
  my($s, $val, $name) = @_;
  my($out, $type, $id, $sname);

  $type = ref $val;
  $out = "";

  if ($type) {

    # Call the freezer method if it's specified and the object has the
    # method.  Trap errors and warn() instead of die()ing, like the XS
    # implementation.
    my $freezer = $s->{freezer};
    if ($freezer and UNIVERSAL::can($val, $freezer)) {
      eval { $val->$freezer() };
      warn "WARNING(Freezer method call failed): $@" if $@;
    }

    require Scalar::Util;
    my $realpack = Scalar::Util::blessed($val);
    my $realtype = $realpack ? Scalar::Util::reftype($val) : ref $val;
    $id = format_refaddr($val);

    # Note: By this point $name is always defined and of non-zero length.
    # Keep a tab on it so that we do not fall into recursive pit.
    if (exists $s->{seen}{$id}) {
      if ($s->{purity} and $s->{level} > 0) {
        $out = ($realtype eq 'HASH')  ? '{}' :
               ($realtype eq 'ARRAY') ? '[]' :
               'do{my $o}' ;
        push @post, $name . " = " . $s->{seen}{$id}[0];
      }
      else {
        $out = $s->{seen}{$id}[0];
        if ($name =~ /^([\@\%])/) {
          my $start = $1;
          if ($out =~ /^\\$start/) {
            $out = substr($out, 1);
          }
          else {
            $out = $start . '{' . $out . '}';
          }
        }
      }
      return $out;
    }
    else {
      # store our name
      $s->{seen}{$id} = [ (
          ($name =~ /^[@%]/)
            ? ('\\' . $name )
            : ($realtype eq 'CODE' and $name =~ /^[*](.*)$/)
              ? ('\\&' . $1 )
              : $name
        ), $val ];
    }
    my $no_bless = 0;
    my $is_regex = 0;
    if ( $realpack and ($] >= 5.009005 ? re::is_regexp($val) : $realpack eq 'Regexp') ) {
        $is_regex = 1;
        $no_bless = $realpack eq 'Regexp';
    }

    # If purity is not set and maxdepth is set, then check depth:
    # if we have reached maximum depth, return the string
    # representation of the thing we are currently examining
    # at this depth (i.e., 'Foo=ARRAY(0xdeadbeef)').
    if (!$s->{purity}
      and defined($s->{maxdepth})
      and $s->{maxdepth} > 0
      and $s->{level} >= $s->{maxdepth})
    {
      return qq['$val'];
    }

    # avoid recursing infinitely [perl #122111]
    if ($s->{maxrecurse} > 0
        and $s->{level} >= $s->{maxrecurse}) {
        die "Recursion limit of $s->{maxrecurse} exceeded";
    }

    # we have a blessed ref
    my ($blesspad);
    if ($realpack and !$no_bless) {
      $out = $s->{'bless'} . '( ';
      $blesspad = $s->{apad};
      $s->{apad} .= '       ' if ($s->{indent} >= 2);
    }

    $s->{level}++;
    my $ipad = $s->{xpad} x $s->{level};

    if ($is_regex) {
        my $pat;
        my $flags = "";
        if (defined(*re::regexp_pattern{CODE})) {
          ($pat, $flags) = re::regexp_pattern($val);
        }
        else {
          $pat = "$val";
        }
        $pat =~ s <
                     (\\.)           # anything backslash escaped
                   | (\$)(?![)|]|\z) # any unescaped $, except $| $) and end
                   | /               # any unescaped /
                  >
                  {
                      $1 ? $1
                          : $2 ? '${\q($)}'
                          : '\\/'
                  }gex;
        $out .= "qr/$pat/$flags";
    }
    elsif ($realtype eq 'SCALAR' || $realtype eq 'REF'
    || $realtype eq 'VSTRING') {
      if ($realpack) {
        $out .= 'do{\\(my $o = ' . $s->_dump($$val, "\${$name}") . ')}';
      }
      else {
        $out .= '\\' . $s->_dump($$val, "\${$name}");
      }
    }
    elsif ($realtype eq 'GLOB') {
      $out .= '\\' . $s->_dump($$val, "*{$name}");
    }
    elsif ($realtype eq 'ARRAY') {
      my($pad, $mname);
      my($i) = 0;
      $out .= ($name =~ /^\@/) ? '(' : '[';
      $pad = $s->{sep} . $s->{pad} . $s->{apad};
      ($name =~ /^\@(.*)$/) ? ($mname = "\$" . $1) :
    # omit -> if $foo->[0]->{bar}, but not ${$foo->[0]}->{bar}
        ($name =~ /^\\?[\%\@\*\$][^{].*[]}]$/) ? ($mname = $name) :
        ($mname = $name . '->');
      $mname .= '->' if $mname =~ /^\*.+\{[A-Z]+\}$/;
      for my $v (@$val) {
        $sname = $mname . '[' . $i . ']';
        $out .= $pad . $ipad . '#' . $i
          if $s->{indent} >= 3;
        $out .= $pad . $ipad . $s->_dump($v, $sname);
        $out .= ","
            if $i++ < $#$val
            || ($s->{trailingcomma} && $s->{indent} >= 1);
      }
      $out .= $pad . ($s->{xpad} x ($s->{level} - 1)) if $i;
      $out .= ($name =~ /^\@/) ? ')' : ']';
    }
    elsif ($realtype eq 'HASH') {
      my ($k, $v, $pad, $lpad, $mname, $pair);
      $out .= ($name =~ /^\%/) ? '(' : '{';
      $pad = $s->{sep} . $s->{pad} . $s->{apad};
      $lpad = $s->{apad};
      $pair = $s->{pair};
      ($name =~ /^\%(.*)$/) ? ($mname = "\$" . $1) :
    # omit -> if $foo->[0]->{bar}, but not ${$foo->[0]}->{bar}
        ($name =~ /^\\?[\%\@\*\$][^{].*[]}]$/) ? ($mname = $name) :
        ($mname = $name . '->');
      $mname .= '->' if $mname =~ /^\*.+\{[A-Z]+\}$/;
      my $sortkeys = defined($s->{sortkeys}) ? $s->{sortkeys} : '';
      my $keys = [];
      if ($sortkeys) {
        if (ref($s->{sortkeys}) eq 'CODE') {
          $keys = $s->{sortkeys}($val);
          unless (ref($keys) eq 'ARRAY') {
            Carp::carp("Sortkeys subroutine did not return ARRAYREF");
            $keys = [];
          }
        }
        else {
          $keys = [ sort keys %$val ];
        }
      }

      # Ensure hash iterator is reset
      keys(%$val);

      my $key;
      while (($k, $v) = ! $sortkeys ? (each %$val) :
         @$keys ? ($key = shift(@$keys), $val->{$key}) :
         () )
      {
        my $nk = $s->_dump($k, "");

        # _dump doesn't quote numbers of this form
        if ($s->{quotekeys} && $nk =~ /^(?:0|-?[1-9][0-9]{0,8})\z/) {
          $nk = $s->{useqq} ? qq("$nk") : qq('$nk');
        }
        elsif (!$s->{quotekeys} and $nk =~ /^[\"\']([A-Za-z_]\w*)[\"\']$/) {
          $nk = $1
        }

        $sname = $mname . '{' . $nk . '}';
        $out .= $pad . $ipad . $nk . $pair;

        # temporarily alter apad
        $s->{apad} .= (" " x (length($nk) + 4))
          if $s->{indent} >= 2;
        $out .= $s->_dump($val->{$k}, $sname) . ",";
        $s->{apad} = $lpad
          if $s->{indent} >= 2;
      }
      if (substr($out, -1) eq ',') {
        chop $out if !$s->{trailingcomma} || !$s->{indent};
        $out .= $pad . ($s->{xpad} x ($s->{level} - 1));
      }
      $out .= ($name =~ /^\%/) ? ')' : '}';
    }
    elsif ($realtype eq 'CODE') {
      if ($s->{deparse}) {
        require B::Deparse;
        my $sub =  'sub ' . (B::Deparse->new)->coderef2text($val);
        my $pad =  $s->{sep} . $s->{pad} . $s->{apad} . $s->{xpad} x ($s->{level} - 1);
        $sub    =~ s/\n/$pad/gs;
        $out   .=  $sub;
      }
      else {
        $out .= 'sub { "DUMMY" }';
        Carp::carp("Encountered CODE ref, using dummy placeholder") if $s->{purity};
      }
    }
    else {
      Carp::croak("Can't handle '$realtype' type");
    }

    if ($realpack and !$no_bless) { # we have a blessed ref
      $out .= ', ' . _quote($realpack) . ' )';
      $out .= '->' . $s->{toaster} . '()'
        if $s->{toaster} ne '';
      $s->{apad} = $blesspad;
    }
    $s->{level}--;
  }
  else {                                 # simple scalar

    my $ref = \$_[1];
    my $v;
    # first, catalog the scalar
    if ($name ne '') {
      $id = format_refaddr($ref);
      if (exists $s->{seen}{$id}) {
        if ($s->{seen}{$id}[2]) {
          $out = $s->{seen}{$id}[0];
          #warn "[<$out]\n";
          return "\${$out}";
        }
      }
      else {
        #warn "[>\\$name]\n";
        $s->{seen}{$id} = ["\\$name", $ref];
      }
    }
    $ref = \$val;
    if (ref($ref) eq 'GLOB') {  # glob
      my $name = substr($val, 1);
      $name =~ s/^main::(?!\z)/::/;
      if ($name =~ /\A(?:[A-Z_a-z][0-9A-Z_a-z]*)?::(?:[0-9A-Z_a-z]+::)*[0-9A-Z_a-z]*\z/ && $name ne 'main::') {
        $sname = $name;
      }
      else {
        local $s->{useqq} = IS_PRE_516_PERL && ($s->{useqq} || $name =~ /[^\x00-\x7f]/) ? 1 : $s->{useqq};
        $sname = $s->_dump(
          $name eq 'main::'
            ? ''
            : $name,
          "",
        );
        $sname = '{' . $sname . '}';
      }
      if ($s->{purity}) {
        my $k;
        local ($s->{level}) = 0;
        for $k (qw(SCALAR ARRAY HASH)) {
          my $gval = *$val{$k};
          next unless defined $gval;
          next if $k eq "SCALAR" && ! defined $$gval;  # always there

          # _dump can push into @post, so we hold our place using $postlen
          my $postlen = scalar @post;
          $post[$postlen] = "\*$sname = ";
          local ($s->{apad}) = " " x length($post[$postlen]) if $s->{indent} >= 2;
          $post[$postlen] .= $s->_dump($gval, "\*$sname\{$k\}");
        }
      }
      $out .= '*' . $sname;
    }
    elsif (!defined($val)) {
      $out .= "undef";
    }
    elsif (SUPPORTS_CORE_BOOLS && do {
      BEGIN { SUPPORTS_CORE_BOOLS and warnings->unimport("experimental::builtin") }
      builtin::is_bool($val)
    }) {
      $out .= $val ? '!!1' : '!!0';
    }
    # This calls the XSUB _vstring (if the XS code is loaded). I'm not *sure* if
    # if belongs in the "Pure Perl" implementation. It sort of depends on what
    # was meant by "Pure Perl", as this subroutine already relies Scalar::Util
    # loading, which means that it has an XS dependency. De facto, it's the
    # "Pure Perl" implementation of dumping (which uses XS helper code), as
    # opposed to the C implementation (which calls out to Perl helper code).
    # So in that sense this is fine - it just happens to be a local XS helper.
    elsif (defined &_vstring and $v = _vstring($val)
      and !_bad_vsmg || eval $v eq $val) {
      $out .= $v;
    }
    # However the confusion comes here - if we *can't* find our XS helper, we
    # fall back to this code, which generates different (worse) results. That's
    # better than nothing, *but* it means that if you run the regression tests
    # with Dumper.so missing, the test for "vstrings" fails, because this code
    # here generates a different result. So there are actually "three" different
    # implementations of Data::Dumper (kind of sort of) but we only test two.
    elsif (!defined &_vstring
       and ref $ref eq 'VSTRING' || eval{Scalar::Util::isvstring($val)}) {
      $out .= sprintf "v%vd", $val;
    }
    # \d here would treat "1\x{660}" as a safe decimal number
    elsif ($val =~ /^(?:0|-?[1-9][0-9]{0,8})\z/) { # safe decimal number
      $out .= $val;
    }
    else {                 # string
      if ($s->{useqq} or $val =~ tr/\0-\377//c) {
        # Fall back to qq if there's Unicode
        $out .= qquote($val, $s->{useqq});
      }
      else {
        $out .= _quote($val);
      }
    }
  }
  if ($id) {
    # if we made it this far, $id was added to seen list at current
    # level, so remove it to get deep copies
    if ($s->{deepcopy}) {
      delete($s->{seen}{$id});
    }
    elsif ($name) {
      $s->{seen}{$id}[2] = 1;
    }
  }
  return $out;
}

#
# non-OO style of earlier version
#
sub Dumper {
  return Data::Dumper->Dump([@_]);
}

# compat stub
sub DumperX {
  return Data::Dumper->Dumpxs([@_], []);
}

#
# reset the "seen" cache
#
sub Reset {
  my($s) = shift;
  $s->{seen} = {};
  return $s;
}

sub Indent {
  my($s, $v) = @_;
  if (@_ >= 2) {
    if ($v == 0) {
      $s->{xpad} = "";
      $s->{sep} = "";
    }
    else {
      $s->{xpad} = "  ";
      $s->{sep} = "\n";
    }
    $s->{indent} = $v;
    return $s;
  }
  else {
    return $s->{indent};
  }
}

sub Trailingcomma {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{trailingcomma} = $v), return $s) : $s->{trailingcomma};
}

sub Pair {
    my($s, $v) = @_;
    @_ >= 2 ? (($s->{pair} = $v), return $s) : $s->{pair};
}

sub Pad {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{pad} = $v), return $s) : $s->{pad};
}

sub Varname {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{varname} = $v), return $s) : $s->{varname};
}

sub Purity {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{purity} = $v), return $s) : $s->{purity};
}

sub Useqq {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{useqq} = $v), return $s) : $s->{useqq};
}

sub Terse {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{terse} = $v), return $s) : $s->{terse};
}

sub Freezer {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{freezer} = $v), return $s) : $s->{freezer};
}

sub Toaster {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{toaster} = $v), return $s) : $s->{toaster};
}

sub Deepcopy {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{deepcopy} = $v), return $s) : $s->{deepcopy};
}

sub Quotekeys {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{quotekeys} = $v), return $s) : $s->{quotekeys};
}

sub Bless {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{'bless'} = $v), return $s) : $s->{'bless'};
}

sub Maxdepth {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{'maxdepth'} = $v), return $s) : $s->{'maxdepth'};
}

sub Maxrecurse {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{'maxrecurse'} = $v), return $s) : $s->{'maxrecurse'};
}

sub Useperl {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{'useperl'} = $v), return $s) : $s->{'useperl'};
}

sub Sortkeys {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{'sortkeys'} = $v), return $s) : $s->{'sortkeys'};
}

sub Deparse {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{'deparse'} = $v), return $s) : $s->{'deparse'};
}

sub Sparseseen {
  my($s, $v) = @_;
  @_ >= 2 ? (($s->{'noseen'} = $v), return $s) : $s->{'noseen'};
}

# used by qquote below
my %esc = (
    "\a" => "\\a",
    "\b" => "\\b",
    "\t" => "\\t",
    "\n" => "\\n",
    "\f" => "\\f",
    "\r" => "\\r",
    "\e" => "\\e",
);

# The low controls are considered to be everything below SPACE, plus the
# outlier \c? control (but that wasn't properly in existence in early perls,
# so reconstruct its value here.  This abandons EBCDIC support for this
# character for perls below 5.8)
my $low_controls = join "", map { quotemeta chr $_ } 0.. (ord(" ") - 1);
$low_controls .= ($] < 5.008 || $IS_ASCII)
                 ? "\x7f"
                 : chr utf8::unicode_to_native(0x9F);
my $low_controls_re = qr/[$low_controls]/;

# put a string value in double quotes
sub qquote {
  local($_) = shift;
  s/([\\\"\@\$])/\\$1/g;

  # This efficiently changes the high ordinal characters to \x{} if the utf8
  # flag is on.  On ASCII platforms, the high ordinals are all the
  # non-ASCII's.  On EBCDIC platforms, we don't include in these the non-ASCII
  # controls.
  my $bytes; { use bytes; $bytes = length }
  s/([^[:ascii:]$low_controls])/sprintf("\\x{%x}",ord($1))/ge
    if $bytes > length;

  return qq("$_") unless /[[:^print:]]/;  # fast exit if only printables

  # Here, there is at least one non-printable to output.  First, translate the
  # escapes.
  s/([\a\b\t\n\f\r\e])/$esc{$1}/g;

  # no need for 3 digits in escape for octals not followed by a digit.
  s/($low_controls_re)(?!\d)/'\\'.sprintf('%o',ord($1))/eg;

  # But otherwise use 3 digits
  s/($low_controls_re)/'\\'.sprintf('%03o',ord($1))/eg;

    # all but last branch below not supported --BEHAVIOR SUBJECT TO CHANGE--
  my $high = shift || "";
    if ($high eq "iso8859") {   # Doesn't escape the Latin1 printables
      # Could use /u and [:cntrl:] etc, if khw were confident it worked in
      # early early perls
      s/([\200-\240])/'\\'.sprintf('%o',ord($1))/eg if $IS_ASCII;
    } elsif ($high eq "utf8") {
#     Some discussion of what to do here is in
#       https://rt.perl.org/Ticket/Display.html?id=113088
#     use utf8;
#     $str =~ s/([^\040-\176])/sprintf "\\x{%04x}", ord($1)/ge;
    } elsif ($high eq "8bit") {
        # leave it as it is
    } else {
      s/([[:^ascii:]])/'\\'.sprintf('%03o',ord($1))/eg;
      #s/([^\040-\176])/sprintf "\\x{%04x}", ord($1)/ge;
    }

  return qq("$_");
}

sub _refine_name {
    my $s = shift;
    my ($name, $val, $i) = @_;
    if (defined $name) {
      if ($name =~ /^[*](.*)$/) {
        if (defined $val) {
            $name = (ref $val eq 'ARRAY') ? ( "\@" . $1 ) :
              (ref $val eq 'HASH')  ? ( "\%" . $1 ) :
              (ref $val eq 'CODE')  ? ( "\*" . $1 ) :
              ( "\$" . $1 ) ;
        }
        else {
          $name = "\$" . $1;
        }
      }
      elsif ($name !~ /^\$/) {
        $name = "\$" . $name;
      }
    }
    else { # no names provided
      $name = "\$" . $s->{varname} . $i;
    }
    return $name;
}

sub _compose_out {
    my $s = shift;
    my ($valstr, $postref) = @_;
    my $out = "";
    $out .= $s->{pad} . $valstr . $s->{sep};
    if (@{$postref}) {
        $out .= $s->{pad} .
            join(';' . $s->{sep} . $s->{pad}, @{$postref}) .
            ';' .
            $s->{sep};
    }
    return $out;
}

1;
__END__

=head1 NAME

Data::Dumper - stringified perl data structures, suitable for both printing and C<eval>

=head1 SYNOPSIS

    use Data::Dumper;

    # simple procedural interface
    print Dumper($foo, $bar);

    # extended usage with names
    print Data::Dumper->Dump([$foo, $bar], [qw(foo *ary)]);

    # configuration variables
    {
      local $Data::Dumper::Purity = 1;
      eval Data::Dumper->Dump([$foo, $bar], [qw(foo *ary)]);
    }

    # OO usage
    my $d = Data::Dumper->new([$foo, $bar], [qw(foo *ary)]);
       ...
    print $d->Dump;
       ...
    $d->Purity(1)->Terse(1)->Deepcopy(1);
    eval $d->Dump;


=head1 DESCRIPTION

Given a list of scalars or reference variables, writes out their contents in
perl syntax. The references can also be objects.  The content of each
variable is output in a single Perl statement.  Handles self-referential
structures correctly.

The return value can be C<eval>ed to get back an identical copy of the
original reference structure.  (Please do consider the security implications
of eval'ing code from untrusted sources!)

Any references that are the same as one of those passed in will be named
C<$VAR>I<n> (where I<n> is a numeric suffix), and other duplicate references
to substructures within C<$VAR>I<n> will be appropriately labeled using arrow
notation.  You can specify names for individual values to be dumped if you
use the C<Dump()> method, or you can change the default C<$VAR> prefix to
something else.  See C<$Data::Dumper::Varname> and C<$Data::Dumper::Terse>
in L</Configuration Variables or Methods> below.

The default output of self-referential structures can be C<eval>ed, but the
nested references to C<$VAR>I<n> will be undefined, since a recursive
structure cannot be constructed using one Perl statement.  You should set the
C<Purity> flag to 1 to get additional statements that will correctly fill in
these references.  Moreover, if C<eval>ed when strictures are in effect,
you need to ensure that any variables it accesses are previously declared.

In the extended usage form, the references to be dumped can be given
user-specified names.  If a name begins with a C<*>, the output will
describe the dereferenced type of the supplied reference for hashes and
arrays, and coderefs.  Output of names will be avoided where possible if
the C<Terse> flag is set.

In many cases, methods that are used to set the internal state of the
object will return the object itself, so method calls can be conveniently
chained together.

Several styles of output are possible, all controlled by setting
the C<Indent> flag.  See L<Configuration Variables or Methods> below
for details.


=head2 Methods

=over 4

=item I<PACKAGE>->new(I<ARRAYREF [>, I<ARRAYREF]>)

Returns a newly created C<Data::Dumper> object.  The first argument is an
anonymous array of values to be dumped.  The optional second argument is an
anonymous array of names for the values.  The names need not have a leading
C<$> sign, and must be comprised of alphanumeric characters.  You can begin
a name with a C<*> to specify that the dereferenced type must be dumped
instead of the reference itself, for ARRAY and HASH references.

The prefix specified by C<$Data::Dumper::Varname> will be used with a
numeric suffix if the name for a value is undefined.

Data::Dumper will catalog all references encountered while dumping the
values. Cross-references (in the form of names of substructures in perl
syntax) will be inserted at all possible points, preserving any structural
interdependencies in the original set of values.  Structure traversal is
depth-first,  and proceeds in order from the first supplied value to
the last.

=item I<$OBJ>->Dump  I<or>  I<PACKAGE>->Dump(I<ARRAYREF [>, I<ARRAYREF]>)

Returns the stringified form of the values stored in the object (preserving
the order in which they were supplied to C<new>), subject to the
configuration options below.  In a list context, it returns a list
of strings corresponding to the supplied values.

The second form, for convenience, simply calls the C<new> method on its
arguments before dumping the object immediately.

=item I<$OBJ>->Seen(I<[HASHREF]>)

Queries or adds to the internal table of already encountered references.
You must use C<Reset> to explicitly clear the table if needed.  Such
references are not dumped; instead, their names are inserted wherever they
are encountered subsequently.  This is useful especially for properly
dumping subroutine references.

Expects an anonymous hash of name => value pairs.  Same rules apply for names
as in C<new>.  If no argument is supplied, will return the "seen" list of
name => value pairs, in a list context.  Otherwise, returns the object
itself.

=item I<$OBJ>->Values(I<[ARRAYREF]>)

Queries or replaces the internal array of values that will be dumped.  When
called without arguments, returns the values as a list.  When called with a
reference to an array of replacement values, returns the object itself.  When
called with any other type of argument, dies.

=item I<$OBJ>->Names(I<[ARRAYREF]>)

Queries or replaces the internal array of user supplied names for the values
that will be dumped.  When called without arguments, returns the names.  When
called with an array of replacement names, returns the object itself.  If the
number of replacement names exceeds the number of values to be named, the
excess names will not be used.  If the number of replacement names falls short
of the number of values to be named, the list of replacement names will be
exhausted and remaining values will not be renamed.  When
called with any other type of argument, dies.

=item I<$OBJ>->Reset

Clears the internal table of "seen" references and returns the object
itself.

=back

=head2 Functions

=over 4

=item Dumper(I<LIST>)

Returns the stringified form of the values in the list, subject to the
configuration options below.  The values will be named C<$VAR>I<n> in the
output, where I<n> is a numeric suffix.  Will return a list of strings
in a list context.

=back

=head2 Configuration Variables or Methods

Several configuration variables can be used to control the kind of output
generated when using the procedural interface.  These variables are usually
C<local>ized in a block so that other parts of the code are not affected by
the change.

These variables determine the default state of the object created by calling
the C<new> method, but cannot be used to alter the state of the object
thereafter.  The equivalent method names should be used instead to query
or set the internal state of the object.

The method forms return the object itself when called with arguments,
so that they can be chained together nicely.

=over 4

=item *

$Data::Dumper::Indent  I<or>  I<$OBJ>->Indent(I<[NEWVAL]>)

Controls the style of indentation.  It can be set to 0, 1, 2 or 3.  Style 0
spews output without any newlines, indentation, or spaces between list items.
It is the most compact format possible that can still be called valid perl.
Style 1 outputs a readable form with newlines but no fancy indentation (each
level in the structure is simply indented by a fixed amount of whitespace).
Style 2 (the default) outputs a very readable form which lines up the hash
keys.  Style 3 is like style 2, but also annotates the elements of arrays with
their index (but the comment is on its own line, so array output consumes
twice the number of lines).  Style 2 is the default.

=item *

$Data::Dumper::Trailingcomma  I<or>  I<$OBJ>->Trailingcomma(I<[NEWVAL]>)

Controls whether a comma is added after the last element of an array or
hash. Even when true, no comma is added between the last element of an array
or hash and a closing bracket when they appear on the same line. The default
is false.

=item *

$Data::Dumper::Purity  I<or>  I<$OBJ>->Purity(I<[NEWVAL]>)

Controls the degree to which the output can be C<eval>ed to recreate the
supplied reference structures.  Setting it to 1 will output additional perl
statements that will correctly recreate nested references.  The default is
0.

=item *

$Data::Dumper::Pad  I<or>  I<$OBJ>->Pad(I<[NEWVAL]>)

Specifies the string that will be prefixed to every line of the output.
Empty string by default.

=item *

$Data::Dumper::Varname  I<or>  I<$OBJ>->Varname(I<[NEWVAL]>)

Contains the prefix to use for tagging variable names in the output. The
default is "VAR".

=item *

$Data::Dumper::Useqq  I<or>  I<$OBJ>->Useqq(I<[NEWVAL]>)

When set, enables the use of double quotes for representing string values.
Whitespace other than space will be represented as C<[\n\t\r]>, "unsafe"
characters will be backslashed, and unprintable characters will be output as
quoted octal integers.  The default is 0.

=item *

$Data::Dumper::Terse  I<or>  I<$OBJ>->Terse(I<[NEWVAL]>)

When set, Data::Dumper will emit single, non-self-referential values as
atoms/terms rather than statements.  This means that the C<$VAR>I<n> names
will be avoided where possible, but be advised that such output may not
always be parseable by C<eval>.

=item *

$Data::Dumper::Freezer  I<or>  $I<OBJ>->Freezer(I<[NEWVAL]>)

Can be set to a method name, or to an empty string to disable the feature.
Data::Dumper will invoke that method via the object before attempting to
stringify it.  This method can alter the contents of the object (if, for
instance, it contains data allocated from C), and even rebless it in a
different package.  The client is responsible for making sure the specified
method can be called via the object, and that the object ends up containing
only perl data types after the method has been called.  Defaults to an empty
string.

If an object does not support the method specified (determined using
UNIVERSAL::can()) then the call will be skipped.  If the method dies a
warning will be generated.

=item *

$Data::Dumper::Toaster  I<or>  $I<OBJ>->Toaster(I<[NEWVAL]>)

Can be set to a method name, or to an empty string to disable the feature.
Data::Dumper will emit a method call for any objects that are to be dumped
using the syntax C<bless(DATA, CLASS)-E<gt>METHOD()>.  Note that this means that
the method specified will have to perform any modifications required on the
object (like creating new state within it, and/or reblessing it in a
different package) and then return it.  The client is responsible for making
sure the method can be called via the object, and that it returns a valid
object.  Defaults to an empty string.

=item *

$Data::Dumper::Deepcopy  I<or>  $I<OBJ>->Deepcopy(I<[NEWVAL]>)

Can be set to a boolean value to enable deep copies of structures.
Cross-referencing will then only be done when absolutely essential
(i.e., to break reference cycles).  Default is 0.

=item *

$Data::Dumper::Quotekeys  I<or>  $I<OBJ>->Quotekeys(I<[NEWVAL]>)

Can be set to a boolean value to control whether hash keys are quoted.
A defined false value will avoid quoting hash keys when it looks like a simple
string.  Default is 1, which will always enclose hash keys in quotes.

=item *

$Data::Dumper::Bless  I<or>  $I<OBJ>->Bless(I<[NEWVAL]>)

Can be set to a string that specifies an alternative to the C<bless>
builtin operator used to create objects.  A function with the specified
name should exist, and should accept the same arguments as the builtin.
Default is C<bless>.

=item *

$Data::Dumper::Pair  I<or>  $I<OBJ>->Pair(I<[NEWVAL]>)

Can be set to a string that specifies the separator between hash keys
and values. To dump nested hash, array and scalar values to JavaScript,
use: C<$Data::Dumper::Pair = ' : ';>. Implementing C<bless> in JavaScript
is left as an exercise for the reader.
A function with the specified name exists, and accepts the same arguments
as the builtin.

Default is: C< =E<gt> >.

=item *

$Data::Dumper::Maxdepth  I<or>  $I<OBJ>->Maxdepth(I<[NEWVAL]>)

Can be set to a positive integer that specifies the depth beyond which
we don't venture into a structure.  Has no effect when
C<Data::Dumper::Purity> is set.  (Useful in debugger when we often don't
want to see more than enough).  Default is 0, which means there is
no maximum depth.

=item *

$Data::Dumper::Maxrecurse  I<or>  $I<OBJ>->Maxrecurse(I<[NEWVAL]>)

Can be set to a positive integer that specifies the depth beyond which
recursion into a structure will throw an exception.  This is intended
as a security measure to prevent perl running out of stack space when
dumping an excessively deep structure.  Can be set to 0 to remove the
limit.  Default is 1000.

=item *

$Data::Dumper::Useperl  I<or>  $I<OBJ>->Useperl(I<[NEWVAL]>)

Can be set to a boolean value which controls whether the pure Perl
implementation of C<Data::Dumper> is used. The C<Data::Dumper> module is
a dual implementation, with almost all functionality written in both
pure Perl and also in XS ('C'). Since the XS version is much faster, it
will always be used if possible. This option lets you override the
default behavior, usually for testing purposes only. Default is 0, which
means the XS implementation will be used if possible.

=item *

$Data::Dumper::Sortkeys  I<or>  $I<OBJ>->Sortkeys(I<[NEWVAL]>)

Can be set to a boolean value to control whether hash keys are dumped in
sorted order. A true value will cause the keys of all hashes to be
dumped in Perl's default sort order. Can also be set to a subroutine
reference which will be called for each hash that is dumped. In this
case C<Data::Dumper> will call the subroutine once for each hash,
passing it the reference of the hash. The purpose of the subroutine is
to return a reference to an array of the keys that will be dumped, in
the order that they should be dumped. Using this feature, you can
control both the order of the keys, and which keys are actually used. In
other words, this subroutine acts as a filter by which you can exclude
certain keys from being dumped. Default is 0, which means that hash keys
are not sorted.

=item *

$Data::Dumper::Deparse  I<or>  $I<OBJ>->Deparse(I<[NEWVAL]>)

Can be set to a boolean value to control whether code references are
turned into perl source code. If set to a true value, C<B::Deparse>
will be used to get the source of the code reference. In older versions,
using this option imposed a significant performance penalty when dumping
parts of a data structure other than code references, but that is no
longer the case.

Caution : use this option only if you know that your coderefs will be
properly reconstructed by C<B::Deparse>.

=item *

$Data::Dumper::Sparseseen I<or>  $I<OBJ>->Sparseseen(I<[NEWVAL]>)

By default, Data::Dumper builds up the "seen" hash of scalars that
it has encountered during serialization. This is very expensive.
This seen hash is necessary to support and even just detect circular
references. It is exposed to the user via the C<Seen()> call both
for writing and reading.

If you, as a user, do not need explicit access to the "seen" hash,
then you can set the C<Sparseseen> option to allow Data::Dumper
to eschew building the "seen" hash for scalars that are known not
to possess more than one reference. This speeds up serialization
considerably if you use the XS implementation.

Note: If you turn on C<Sparseseen>, then you must not rely on the
content of the seen hash since its contents will be an
implementation detail!

=back

=head2 Exports

=over 4

=item Dumper

=back

=head1 EXAMPLES

Run these code snippets to get a quick feel for the behavior of this
module.  When you are through with these examples, you may want to
add or change the various configuration variables described above,
to see their behavior.  (See the testsuite in the Data::Dumper
distribution for more examples.)


    use Data::Dumper;

    package Foo;
    sub new {bless {'a' => 1, 'b' => sub { return "foo" }}, $_[0]};

    package Fuz;                       # a weird REF-REF-SCALAR object
    sub new {bless \($_ = \ 'fu\'z'), $_[0]};

    package main;
    $foo = Foo->new;
    $fuz = Fuz->new;
    $boo = [ 1, [], "abcd", \*foo,
             {1 => 'a', 023 => 'b', 0x45 => 'c'},
             \\"p\q\'r", $foo, $fuz];

    ########
    # simple usage
    ########

    $bar = eval(Dumper($boo));
    print($@) if $@;
    print Dumper($boo), Dumper($bar);  # pretty print (no array indices)

    $Data::Dumper::Terse = 1;        # don't output names where feasible
    $Data::Dumper::Indent = 0;       # turn off all pretty print
    print Dumper($boo), "\n";

    $Data::Dumper::Indent = 1;       # mild pretty print
    print Dumper($boo);

    $Data::Dumper::Indent = 3;       # pretty print with array indices
    print Dumper($boo);

    $Data::Dumper::Useqq = 1;        # print strings in double quotes
    print Dumper($boo);

    $Data::Dumper::Pair = " : ";     # specify hash key/value separator
    print Dumper($boo);


    ########
    # recursive structures
    ########

    @c = ('c');
    $c = \@c;
    $b = {};
    $a = [1, $b, $c];
    $b->{a} = $a;
    $b->{b} = $a->[1];
    $b->{c} = $a->[2];
    print Data::Dumper->Dump([$a,$b,$c], [qw(a b c)]);


    $Data::Dumper::Purity = 1;         # fill in the holes for eval
    print Data::Dumper->Dump([$a, $b], [qw(*a b)]); # print as @a
    print Data::Dumper->Dump([$b, $a], [qw(*b a)]); # print as %b


    $Data::Dumper::Deepcopy = 1;       # avoid cross-refs
    print Data::Dumper->Dump([$b, $a], [qw(*b a)]);


    $Data::Dumper::Purity = 0;         # avoid cross-refs
    print Data::Dumper->Dump([$b, $a], [qw(*b a)]);

    ########
    # deep structures
    ########

    $a = "pearl";
    $b = [ $a ];
    $c = { 'b' => $b };
    $d = [ $c ];
    $e = { 'd' => $d };
    $f = { 'e' => $e };
    print Data::Dumper->Dump([$f], [qw(f)]);

    $Data::Dumper::Maxdepth = 3;       # no deeper than 3 refs down
    print Data::Dumper->Dump([$f], [qw(f)]);


    ########
    # object-oriented usage
    ########

    $d = Data::Dumper->new([$a,$b], [qw(a b)]);
    $d->Seen({'*c' => $c});            # stash a ref without printing it
    $d->Indent(3);
    print $d->Dump;
    $d->Reset->Purity(0);              # empty the seen cache
    print join "----\n", $d->Dump;


    ########
    # persistence
    ########

    package Foo;
    sub new { bless { state => 'awake' }, shift }
    sub Freeze {
        my $s = shift;
        print STDERR "preparing to sleep\n";
        $s->{state} = 'asleep';
        return bless $s, 'Foo::ZZZ';
    }

    package Foo::ZZZ;
    sub Thaw {
        my $s = shift;
        print STDERR "waking up\n";
        $s->{state} = 'awake';
        return bless $s, 'Foo';
    }

    package main;
    use Data::Dumper;
    $a = Foo->new;
    $b = Data::Dumper->new([$a], ['c']);
    $b->Freezer('Freeze');
    $b->Toaster('Thaw');
    $c = $b->Dump;
    print $c;
    $d = eval $c;
    print Data::Dumper->Dump([$d], ['d']);


    ########
    # symbol substitution (useful for recreating CODE refs)
    ########

    sub foo { print "foo speaking\n" }
    *other = \&foo;
    $bar = [ \&other ];
    $d = Data::Dumper->new([\&other,$bar],['*other','bar']);
    $d->Seen({ '*foo' => \&foo });
    print $d->Dump;


    ########
    # sorting and filtering hash keys
    ########

    $Data::Dumper::Sortkeys = \&my_filter;
    my $foo = { map { (ord, "$_$_$_") } 'I'..'Q' };
    my $bar = { %$foo };
    my $baz = { reverse %$foo };
    print Dumper [ $foo, $bar, $baz ];

    sub my_filter {
        my ($hash) = @_;
        # return an array ref containing the hash keys to dump
        # in the order that you want them to be dumped
        return [
          # Sort the keys of %$foo in reverse numeric order
            $hash eq $foo ? (sort {$b <=> $a} keys %$hash) :
          # Only dump the odd number keys of %$bar
            $hash eq $bar ? (grep {$_ % 2} keys %$hash) :
          # Sort keys in default order for all other hashes
            (sort keys %$hash)
        ];
    }

=head1 BUGS

Due to limitations of Perl subroutine call semantics, you cannot pass an
array or hash.  Prepend it with a C<\> to pass its reference instead.  This
will be remedied in time, now that Perl has subroutine prototypes.
For now, you need to use the extended usage form, and prepend the
name with a C<*> to output it as a hash or array.

C<Data::Dumper> cheats with CODE references.  If a code reference is
encountered in the structure being processed (and if you haven't set
the C<Deparse> flag), an anonymous subroutine that
contains the string '"DUMMY"' will be inserted in its place, and a warning
will be printed if C<Purity> is set.  You can C<eval> the result, but bear
in mind that the anonymous sub that gets created is just a placeholder.
Even using the C<Deparse> flag will in some cases produce results that
behave differently after being passed to C<eval>; see the documentation
for L<B::Deparse>.

SCALAR objects have the weirdest looking C<bless> workaround.

=head2 NOTE

Different runs of Perl will have different
ordering of hash keys.  The change was done for greater security,
see L<perlsec/"Algorithmic Complexity Attacks">.  This means that
different runs of Perl will have different Data::Dumper outputs if
the data contains hashes.  If you need to have identical Data::Dumper
outputs from different runs of Perl, use the environment variable
PERL_HASH_SEED, see L<perlrun/PERL_HASH_SEED>.  Using this restores
the old (platform-specific) ordering: an even prettier solution might
be to use the C<Sortkeys> filter of Data::Dumper.

=head1 AUTHOR

Gurusamy Sarathy        gsar@activestate.com

Copyright (c) 1996-2019 Gurusamy Sarathy. All rights reserved.
This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

=head1 VERSION

Version 2.188

=head1 SEE ALSO

perl(1)

=cut
