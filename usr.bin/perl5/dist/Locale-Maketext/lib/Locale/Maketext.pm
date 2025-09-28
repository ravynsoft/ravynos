package Locale::Maketext;
use strict;
our $USE_LITERALS;
use Carp ();
use I18N::LangTags ();
use I18N::LangTags::Detect ();

#--------------------------------------------------------------------------

BEGIN { unless(defined &DEBUG) { *DEBUG = sub () {0} } }
# define the constant 'DEBUG' at compile-time

# turn on utf8 if we have it (this is what GutsLoader.pm used to do essentially )
#    use if (exists $INC{'utf8.pm'} || eval 'use utf8'), 'utf8';
BEGIN {

    # if we have it || we can load it
    if ( exists $INC{'utf8.pm'} || eval { local $SIG{'__DIE__'};require utf8; } ) {
        utf8->import();
        DEBUG and warn " utf8 on for _compile()\n";
    }
    else {
        DEBUG and warn " utf8 not available for _compile() ($INC{'utf8.pm'})\n$@\n";
    }
}


our $VERSION = '1.33';
our @ISA = ();

our $MATCH_SUPERS = 1;
our $MATCH_SUPERS_TIGHTLY = 1;
our $USING_LANGUAGE_TAGS  = 1;
# Turning this off is somewhat of a security risk in that little or no
# checking will be done on the legality of tokens passed to the
# eval("use $module_name") in _try_use.  If you turn this off, you have
# to do your own taint checking.

$USE_LITERALS = 1 unless defined $USE_LITERALS;
# a hint for compiling bracket-notation things.

my %isa_scan = ();

###########################################################################

sub quant {
    my($handle, $num, @forms) = @_;

    return $num if @forms == 0; # what should this mean?
    return $forms[2] if @forms > 2 and $num == 0; # special zeroth case

    # Normal case:
    # Note that the formatting of $num is preserved.
    return( $handle->numf($num) . ' ' . $handle->numerate($num, @forms) );
    # Most human languages put the number phrase before the qualified phrase.
}


sub numerate {
    # return this lexical item in a form appropriate to this number
    my($handle, $num, @forms) = @_;
    my $s = ($num == 1);

    return '' unless @forms;
    if(@forms == 1) { # only the headword form specified
        return $s ? $forms[0] : ($forms[0] . 's'); # very cheap hack.
    }
    else { # sing and plural were specified
        return $s ? $forms[0] : $forms[1];
    }
}

#--------------------------------------------------------------------------

sub numf {
    my($handle, $num) = @_[0,1];
    if($num < 10_000_000_000 and $num > -10_000_000_000 and $num == int($num)) {
        $num += 0;  # Just use normal integer stringification.
        # Specifically, don't let %G turn ten million into 1E+007
    }
    else {
        $num = CORE::sprintf('%G', $num);
        # "CORE::" is there to avoid confusion with the above sub sprintf.
    }
    while( $num =~ s/^([-+]?\d+)(\d{3})/$1,$2/s ) {1}  # right from perlfaq5
    # The initial \d+ gobbles as many digits as it can, and then we
    #  backtrack so it un-eats the rightmost three, and then we
    #  insert the comma there.

    $num =~ tr<.,><,.> if ref($handle) and $handle->{'numf_comma'};
    # This is just a lame hack instead of using Number::Format
    return $num;
}

sub sprintf {
    no integer;
    my($handle, $format, @params) = @_;
    return CORE::sprintf($format, @params);
    # "CORE::" is there to avoid confusion with myself!
}

#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#=#

use integer; # vroom vroom... applies to the whole rest of the module

sub language_tag {
    my $it = ref($_[0]) || $_[0];
    return undef unless $it =~ m/([^':]+)(?:::)?$/s;
    $it = lc($1);
    $it =~ tr<_><->;
    return $it;
}

sub encoding {
    my $it = $_[0];
    return(
        (ref($it) && $it->{'encoding'})
        || 'iso-8859-1'   # Latin-1
    );
}

#--------------------------------------------------------------------------

sub fallback_languages { return('i-default', 'en', 'en-US') }

sub fallback_language_classes { return () }

#--------------------------------------------------------------------------

sub fail_with { # an actual attribute method!
    my($handle, @params) = @_;
    return unless ref($handle);
    $handle->{'fail'} = $params[0] if @params;
    return $handle->{'fail'};
}

#--------------------------------------------------------------------------

sub _exclude {
    my ( $handle, @methods  ) = @_;

    unless ( defined $handle->{'denylist'} ) {
        no strict 'refs';

        # Don't let people call methods they're not supposed to from maketext.
        # Explicitly exclude all methods in this package that start with an
        # underscore on principle.
        $handle->{'denylist'} = {
            map { $_ => 1 } (
                qw/
                  blacklist
                  denylist
                  encoding
                  fail_with
                  failure_handler_auto
                  fallback_language_classes
                  fallback_languages
                  get_handle
                  init
                  language_tag
                  maketext
                  new
                  whitelist
                  allowlist
                  /, grep { /^_/ } keys %{ __PACKAGE__ . "::" }
            ),
        };
    }

    if ( scalar @methods ) {
        $handle->{'denylist'} = { %{ $handle->{'denylist'} }, map { $_ => 1 } @methods };
    }

    delete $handle->{'_external_lex_cache'};
    return;
}

sub blacklist {
    my ( $handle, @methods  ) = @_;
    _exclude ( $handle, @methods );
    return;
}

sub denylist {
    my ( $handle, @methods  ) = @_;
    _exclude ( $handle, @methods );
    return;
}

sub _include {
    my ( $handle, @methods ) = @_;
    if ( scalar @methods ) {
        $handle->{'allowlist'} = {} unless defined $handle->{'allowlist'};
        $handle->{'allowlist'} = { %{ $handle->{'allowlist'} }, map { $_ => 1 } @methods };
    }

    delete $handle->{'_external_lex_cache'};
    return;
}

sub whitelist {
    my ( $handle, @methods  ) = @_;
    _include ( $handle, @methods );
    return;
}

sub allowlist {
    my ( $handle, @methods  ) = @_;
    _include ( $handle, @methods );
    return;
}

#--------------------------------------------------------------------------

sub failure_handler_auto {
    # Meant to be used like:
    #  $handle->fail_with('failure_handler_auto')

    my $handle = shift;
    my $phrase = shift;

    $handle->{'failure_lex'} ||= {};
    my $lex = $handle->{'failure_lex'};

    my $value ||= ($lex->{$phrase} ||= $handle->_compile($phrase));

    # Dumbly copied from sub maketext:
    return ${$value} if ref($value) eq 'SCALAR';
    return $value    if ref($value) ne 'CODE';
    {
        local $SIG{'__DIE__'};
        eval { $value = &$value($handle, @_) };
    }
    # If we make it here, there was an exception thrown in the
    #  call to $value, and so scream:
    if($@) {
        # pretty up the error message
        $@ =~ s{\s+at\s+\(eval\s+\d+\)\s+line\s+(\d+)\.?\n?}
                 {\n in bracket code [compiled line $1],}s;
        #$err =~ s/\n?$/\n/s;
        Carp::croak "Error in maketexting \"$phrase\":\n$@ as used";
        # Rather unexpected, but suppose that the sub tried calling
        # a method that didn't exist.
    }
    else {
        return $value;
    }
}

#==========================================================================

sub new {
    # Nothing fancy!
    my $class = ref($_[0]) || $_[0];
    my $handle = bless {}, $class;
    $handle->blacklist;
    $handle->denylist;
    $handle->init;
    return $handle;
}

sub init { return } # no-op

###########################################################################

sub maketext {
    # Remember, this can fail.  Failure is controllable many ways.
    Carp::croak 'maketext requires at least one parameter' unless @_ > 1;

    my($handle, $phrase) = splice(@_,0,2);
    Carp::confess('No handle/phrase') unless (defined($handle) && defined($phrase));

    # backup $@ in case it's still being used in the calling code.
    # If no failures, we'll re-set it back to what it was later.
    my $at = $@;

    # Copy @_ case one of its elements is $@.
    @_ = @_;

    # Look up the value:

    my $value;
    if (exists $handle->{'_external_lex_cache'}{$phrase}) {
        DEBUG and warn "* Using external lex cache version of \"$phrase\"\n";
        $value = $handle->{'_external_lex_cache'}{$phrase};
    }
    else {
        foreach my $h_r (
            @{  $isa_scan{ref($handle) || $handle} || $handle->_lex_refs  }
        ) {
            DEBUG and warn "* Looking up \"$phrase\" in $h_r\n";
            if(exists $h_r->{$phrase}) {
                DEBUG and warn "  Found \"$phrase\" in $h_r\n";
                unless(ref($value = $h_r->{$phrase})) {
                    # Nonref means it's not yet compiled.  Compile and replace.
                    if ($handle->{'use_external_lex_cache'}) {
                        $value = $handle->{'_external_lex_cache'}{$phrase} = $handle->_compile($value);
                    }
                    else {
                        $value = $h_r->{$phrase} = $handle->_compile($value);
                    }
                }
                last;
            }
            # extending packages need to be able to localize _AUTO and if readonly can't "local $h_r->{'_AUTO'} = 1;"
            # but they can "local $handle->{'_external_lex_cache'}{'_AUTO'} = 1;"
            elsif($phrase !~ m/^_/s and ($handle->{'use_external_lex_cache'} ? ( exists $handle->{'_external_lex_cache'}{'_AUTO'} ? $handle->{'_external_lex_cache'}{'_AUTO'} : $h_r->{'_AUTO'} ) : $h_r->{'_AUTO'})) {
                # it's an auto lex, and this is an autoable key!
                DEBUG and warn "  Automaking \"$phrase\" into $h_r\n";
                if ($handle->{'use_external_lex_cache'}) {
                    $value = $handle->{'_external_lex_cache'}{$phrase} = $handle->_compile($phrase);
                }
                else {
                    $value = $h_r->{$phrase} = $handle->_compile($phrase);
                }
                last;
            }
            DEBUG>1 and print "  Not found in $h_r, nor automakable\n";
            # else keep looking
        }
    }

    unless(defined($value)) {
        DEBUG and warn "! Lookup of \"$phrase\" in/under ", ref($handle) || $handle, " fails.\n";
        if(ref($handle) and $handle->{'fail'}) {
            DEBUG and warn "WARNING0: maketext fails looking for <$phrase>\n";
            my $fail;
            if(ref($fail = $handle->{'fail'}) eq 'CODE') { # it's a sub reference
                $@ = $at; # Put $@ back in case we altered it along the way.
                return &{$fail}($handle, $phrase, @_);
                # If it ever returns, it should return a good value.
            }
            else { # It's a method name
                $@ = $at; # Put $@ back in case we altered it along the way.
                return $handle->$fail($phrase, @_);
                # If it ever returns, it should return a good value.
            }
        }
        else {
            # All we know how to do is this;
            Carp::croak("maketext doesn't know how to say:\n$phrase\nas needed");
        }
    }

    if(ref($value) eq 'SCALAR'){
        $@ = $at; # Put $@ back in case we altered it along the way.
        return $$value ;
    }
    if(ref($value) ne 'CODE'){
        $@ = $at; # Put $@ back in case we altered it along the way.
        return $value ;
    }

    {
        local $SIG{'__DIE__'};
        eval { $value = &$value($handle, @_) };
    }
    # If we make it here, there was an exception thrown in the
    #  call to $value, and so scream:
    if ($@) {
        # pretty up the error message
        $@ =~ s{\s+at\s+\(eval\s+\d+\)\s+line\s+(\d+)\.?\n?}
                 {\n in bracket code [compiled line $1],}s;
        #$err =~ s/\n?$/\n/s;
        Carp::croak "Error in maketexting \"$phrase\":\n$@ as used";
        # Rather unexpected, but suppose that the sub tried calling
        # a method that didn't exist.
    }
    else {
        $@ = $at; # Put $@ back in case we altered it along the way.
        return $value;
    }
    $@ = $at; # Put $@ back in case we altered it along the way.
}

###########################################################################

sub get_handle {  # This is a constructor and, yes, it CAN FAIL.
    # Its class argument has to be the base class for the current
    # application's l10n files.

    my($base_class, @languages) = @_;
    $base_class = ref($base_class) || $base_class;
    # Complain if they use __PACKAGE__ as a project base class?

    if( @languages ) {
        DEBUG and warn 'Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";
        if($USING_LANGUAGE_TAGS) {   # An explicit language-list was given!
            @languages =
            map {; $_, I18N::LangTags::alternate_language_tags($_) }
            # Catch alternation
            map I18N::LangTags::locale2language_tag($_),
            # If it's a lg tag, fine, pass thru (untainted)
            # If it's a locale ID, try converting to a lg tag (untainted),
            # otherwise nix it.
            @languages;
            DEBUG and warn 'Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";
        }
    }
    else {
        @languages = $base_class->_ambient_langprefs;
    }

    @languages = $base_class->_langtag_munging(@languages);

    my %seen;
    foreach my $module_name ( map { $base_class . '::' . $_ }  @languages ) {
        next unless length $module_name; # sanity
        next if $seen{$module_name}++        # Already been here, and it was no-go
        || !&_try_use($module_name); # Try to use() it, but can't it.
        return($module_name->new); # Make it!
    }

    return undef; # Fail!
}

###########################################################################

sub _langtag_munging {
    my($base_class, @languages) = @_;

    # We have all these DEBUG statements because otherwise it's hard as hell
    # to diagnose if/when something goes wrong.

    DEBUG and warn 'Lgs1: ', map("<$_>", @languages), "\n";

    if($USING_LANGUAGE_TAGS) {
        DEBUG and warn 'Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";
        @languages     = $base_class->_add_supers( @languages );

        push @languages, I18N::LangTags::panic_languages(@languages);
        DEBUG and warn "After adding panic languages:\n",
        ' Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";

        push @languages, $base_class->fallback_languages;
        # You are free to override fallback_languages to return empty-list!
        DEBUG and warn 'Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";

        @languages =  # final bit of processing to turn them into classname things
        map {
            my $it = $_;  # copy
            $it =~ tr<-A-Z><_a-z>; # lc, and turn - to _
            $it =~ tr<_a-z0-9><>cd;  # remove all but a-z0-9_
            $it;
        } @languages
        ;
        DEBUG and warn "Nearing end of munging:\n",
        ' Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";
    }
    else {
        DEBUG and warn "Bypassing language-tags.\n",
        ' Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";
    }

    DEBUG and warn "Before adding fallback classes:\n",
    ' Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";

    push @languages, $base_class->fallback_language_classes;
    # You are free to override that to return whatever.

    DEBUG and warn "Finally:\n",
    ' Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";

    return @languages;
}

###########################################################################

sub _ambient_langprefs {
    return  I18N::LangTags::Detect::detect();
}

###########################################################################

sub _add_supers {
    my($base_class, @languages) = @_;

    if (!$MATCH_SUPERS) {
        # Nothing
        DEBUG and warn "Bypassing any super-matching.\n",
        ' Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";

    }
    elsif( $MATCH_SUPERS_TIGHTLY ) {
        DEBUG and warn "Before adding new supers tightly:\n",
        ' Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";
        @languages = I18N::LangTags::implicate_supers( @languages );
        DEBUG and warn "After adding new supers tightly:\n",
        ' Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";

    }
    else {
        DEBUG and warn "Before adding supers to end:\n",
        ' Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";
        @languages = I18N::LangTags::implicate_supers_strictly( @languages );
        DEBUG and warn "After adding supers to end:\n",
        ' Lgs@', __LINE__, ': ', map("<$_>", @languages), "\n";
    }

    return @languages;
}

###########################################################################
#
# This is where most people should stop reading.
#
###########################################################################

my %tried = ();
# memoization of whether we've used this module, or found it unusable.

sub _try_use {   # Basically a wrapper around "require Modulename"
    # "Many men have tried..."  "They tried and failed?"  "They tried and died."
    return $tried{$_[0]} if exists $tried{$_[0]};  # memoization

    my $module = $_[0];   # ASSUME sane module name!
    { no strict 'refs';
        no warnings 'once';
        return($tried{$module} = 1)
        if %{$module . '::Lexicon'} or @{$module . '::ISA'};
        # weird case: we never use'd it, but there it is!
    }

    DEBUG and warn " About to use $module ...\n";

    local $SIG{'__DIE__'};
    local $@;
    local @INC = @INC;
    pop @INC if $INC[-1] eq '.';
    eval "require $module"; # used to be "use $module", but no point in that.

    if($@) {
        DEBUG and warn "Error using $module \: $@\n";
        return $tried{$module} = 0;
    }
    else {
        DEBUG and warn " OK, $module is used\n";
        return $tried{$module} = 1;
    }
}

#--------------------------------------------------------------------------

sub _lex_refs {  # report the lexicon references for this handle's class
    # returns an arrayREF!
    no strict 'refs';
    no warnings 'once';
    my $class = ref($_[0]) || $_[0];
    DEBUG and warn "Lex refs lookup on $class\n";
    return $isa_scan{$class} if exists $isa_scan{$class};  # memoization!

    my @lex_refs;
    my $seen_r = ref($_[1]) ? $_[1] : {};

    if( defined( *{$class . '::Lexicon'}{'HASH'} )) {
        push @lex_refs, *{$class . '::Lexicon'}{'HASH'};
        DEBUG and warn '%' . $class . '::Lexicon contains ',
            scalar(keys %{$class . '::Lexicon'}), " entries\n";
    }

    # Implements depth(height?)-first recursive searching of superclasses.
    # In hindsight, I suppose I could have just used Class::ISA!
    foreach my $superclass (@{$class . '::ISA'}) {
        DEBUG and warn " Super-class search into $superclass\n";
        next if $seen_r->{$superclass}++;
        push @lex_refs, @{&_lex_refs($superclass, $seen_r)};  # call myself
    }

    $isa_scan{$class} = \@lex_refs; # save for next time
    return \@lex_refs;
}

sub clear_isa_scan { %isa_scan = (); return; } # end on a note of simplicity!

#--------------------------------------------------------------------------

sub _compile {
    # This big scary routine compiles an entry.
    # It returns either a coderef if there's brackety bits in this, or
    #  otherwise a ref to a scalar.

    my $string_to_compile = $_[1]; # There are taint issues using regex on @_ - perlbug 60378,27344

    # The while() regex is more expensive than this check on strings that don't need a compile.
    # this op causes a ~2% speed hit for strings that need compile and a 250% speed improvement
    # on strings that don't need compiling.
    return \"$string_to_compile" if($string_to_compile !~ m/[\[~\]]/ms); # return a string ref if chars [~] are not in the string

    my $handle = $_[0];

    my(@code);
    my(@c) = (''); # "chunks" -- scratch.
    my $call_count = 0;
    my $big_pile = '';
    {
        my $in_group = 0; # start out outside a group
        my($m, @params); # scratch

        while($string_to_compile =~  # Iterate over chunks.
            m/(
                [^\~\[\]]+  # non-~[] stuff (Capture everything else here)
                |
                ~.       # ~[, ~], ~~, ~other
                |
                \[          # [ presumably opening a group
                |
                \]          # ] presumably closing a group
                |
                ~           # terminal ~ ?
                |
                $
            )/xgs
        ) {
            DEBUG>2 and warn qq{  "$1"\n};

            if($1 eq '[' or $1 eq '') {       # "[" or end
                # Whether this is "[" or end, force processing of any
                #  preceding literal.
                if($in_group) {
                    if($1 eq '') {
                        $handle->_die_pointing($string_to_compile, 'Unterminated bracket group');
                    }
                    else {
                        $handle->_die_pointing($string_to_compile, 'You can\'t nest bracket groups');
                    }
                }
                else {
                    if ($1 eq '') {
                        DEBUG>2 and warn "   [end-string]\n";
                    }
                    else {
                        $in_group = 1;
                    }
                    die "How come \@c is empty?? in <$string_to_compile>" unless @c; # sanity
                    if(length $c[-1]) {
                        # Now actually processing the preceding literal
                        $big_pile .= $c[-1];
                        if($USE_LITERALS and (
                                (ord('A') == 65)
                                ? $c[-1] !~ m/[^\x20-\x7E]/s
                                # ASCII very safe chars
                                : $c[-1] !~ m/[^ !"\#\$%&'()*+,\-.\/0-9:;<=>?\@A-Z[\\\]^_`a-z{|}~\x07]/s
                                # EBCDIC very safe chars
                            )) {
                            # normal case -- all very safe chars
                            $c[-1] =~ s/'/\\'/g;
                            push @code, q{ '} . $c[-1] . "',\n";
                            $c[-1] = ''; # reuse this slot
                        }
                        else {
                            $c[-1] =~ s/\\\\/\\/g;
                            push @code, ' $c[' . $#c . "],\n";
                            push @c, ''; # new chunk
                        }
                    }
                    # else just ignore the empty string.
                }

            }
            elsif($1 eq ']') {  # "]"
                # close group -- go back in-band
                if($in_group) {
                    $in_group = 0;

                    DEBUG>2 and warn "   --Closing group [$c[-1]]\n";

                    # And now process the group...

                    if(!length($c[-1]) or $c[-1] =~ m/^\s+$/s) {
                        DEBUG>2 and warn "   -- (Ignoring)\n";
                        $c[-1] = ''; # reset out chink
                        next;
                    }

                    #$c[-1] =~ s/^\s+//s;
                    #$c[-1] =~ s/\s+$//s;
                    ($m,@params) = split(/,/, $c[-1], -1);  # was /\s*,\s*/

                    # A bit of a hack -- we've turned "~,"'s into DELs, so turn
                    #  'em into real commas here.
                    if (ord('A') == 65) { # ASCII, etc
                        foreach($m, @params) { tr/\x7F/,/ }
                    }
                    else {              # EBCDIC (1047, 0037, POSIX-BC)
                        # Thanks to Peter Prymmer for the EBCDIC handling
                        foreach($m, @params) { tr/\x07/,/ }
                    }

                    # Special-case handling of some method names:
                    if($m eq '_*' or $m =~ m/^_(-?\d+)$/s) {
                        # Treat [_1,...] as [,_1,...], etc.
                        unshift @params, $m;
                        $m = '';
                    }
                    elsif($m eq '*') {
                        $m = 'quant'; # "*" for "times": "4 cars" is 4 times "cars"
                    }
                    elsif($m eq '#') {
                        $m = 'numf';  # "#" for "number": [#,_1] for "the number _1"
                    }

                    # Most common case: a simple, legal-looking method name
                    if($m eq '') {
                        # 0-length method name means to just interpolate:
                        push @code, ' (';
                    }
                    elsif($m =~ /^\w+$/s
                        && !$handle->{'blacklist'}{$m}
                        && !$handle->{'denylist'}{$m}
                        && ( !defined $handle->{'whitelist'} || $handle->{'whitelist'}{$m} )
                        && ( !defined $handle->{'allowlist'} || $handle->{'allowlist'}{$m} )
                        # exclude anything fancy and restrict to the allowlist/denylist (and historical whitelist/blacklist).
                    ) {
                        push @code, ' $_[0]->' . $m . '(';
                    }
                    else {
                        # TODO: implement something?  or just too icky to consider?
                        $handle->_die_pointing(
                            $string_to_compile,
                            "Can't use \"$m\" as a method name in bracket group",
                            2 + length($c[-1])
                        );
                    }

                    pop @c; # we don't need that chunk anymore
                    ++$call_count;

                    foreach my $p (@params) {
                        if($p eq '_*') {
                            # Meaning: all parameters except $_[0]
                            $code[-1] .= ' @_[1 .. $#_], ';
                            # and yes, that does the right thing for all @_ < 3
                        }
                        elsif($p =~ m/^_(-?\d+)$/s) {
                            # _3 meaning $_[3]
                            $code[-1] .= '$_[' . (0 + $1) . '], ';
                        }
                        elsif($USE_LITERALS and (
                                (ord('A') == 65)
                                ? $p !~ m/[^\x20-\x7E]/s
                                # ASCII very safe chars
                                : $p !~ m/[^ !"\#\$%&'()*+,\-.\/0-9:;<=>?\@A-Z[\\\]^_`a-z{|}~\x07]/s
                                # EBCDIC very safe chars
                            )) {
                            # Normal case: a literal containing only safe characters
                            $p =~ s/'/\\'/g;
                            $code[-1] .= q{'} . $p . q{', };
                        }
                        else {
                            # Stow it on the chunk-stack, and just refer to that.
                            push @c, $p;
                            push @code, ' $c[' . $#c . '], ';
                        }
                    }
                    $code[-1] .= "),\n";

                    push @c, '';
                }
                else {
                    $handle->_die_pointing($string_to_compile, q{Unbalanced ']'});
                }

            }
            elsif(substr($1,0,1) ne '~') {
                # it's stuff not containing "~" or "[" or "]"
                # i.e., a literal blob
                my $text = $1;
                $text =~ s/\\/\\\\/g;
                $c[-1] .= $text;

            }
            elsif($1 eq '~~') { # "~~"
                $c[-1] .= '~';

            }
            elsif($1 eq '~[') { # "~["
                $c[-1] .= '[';

            }
            elsif($1 eq '~]') { # "~]"
                $c[-1] .= ']';

            }
            elsif($1 eq '~,') { # "~,"
                if($in_group) {
                    # This is a hack, based on the assumption that no-one will actually
                    # want a DEL inside a bracket group.  Let's hope that's it's true.
                    if (ord('A') == 65) { # ASCII etc
                        $c[-1] .= "\x7F";
                    }
                    else {              # EBCDIC (cp 1047, 0037, POSIX-BC)
                        $c[-1] .= "\x07";
                    }
                }
                else {
                    $c[-1] .= '~,';
                }

            }
            elsif($1 eq '~') { # possible only at string-end, it seems.
                $c[-1] .= '~';

            }
            else {
                # It's a "~X" where X is not a special character.
                # Consider it a literal ~ and X.
                my $text = $1;
                $text =~ s/\\/\\\\/g;
                $c[-1] .= $text;
            }
        }
    }

    if($call_count) {
        undef $big_pile; # Well, nevermind that.
    }
    else {
        # It's all literals!  Ahwell, that can happen.
        # So don't bother with the eval.  Return a SCALAR reference.
        return \$big_pile;
    }

    die q{Last chunk isn't null??} if @c and length $c[-1]; # sanity
    DEBUG and warn scalar(@c), " chunks under closure\n";
    if(@code == 0) { # not possible?
        DEBUG and warn "Empty code\n";
        return \'';
    }
    elsif(@code > 1) { # most cases, presumably!
        unshift @code, "join '',\n";
    }
    unshift @code, "use strict; sub {\n";
    push @code, "}\n";

    DEBUG and warn @code;
    my $sub = eval(join '', @code);
    die "$@ while evalling" . join('', @code) if $@; # Should be impossible.
    return $sub;
}

#--------------------------------------------------------------------------

sub _die_pointing {
    # This is used by _compile to throw a fatal error
    my $target = shift;
    $target = ref($target) || $target; # class name
                                       # ...leaving $_[0] the error-causing text, and $_[1] the error message

    my $i = index($_[0], "\n");

    my $pointy;
    my $pos = pos($_[0]) - (defined($_[2]) ? $_[2] : 0) - 1;
    if($pos < 1) {
        $pointy = "^=== near there\n";
    }
    else { # we need to space over
        my $first_tab = index($_[0], "\t");
        if($pos > 2 and ( -1 == $first_tab  or  $first_tab > pos($_[0]))) {
            # No tabs, or the first tab is harmlessly after where we will point to,
            # AND we're far enough from the margin that we can draw a proper arrow.
            $pointy = ('=' x $pos) . "^ near there\n";
        }
        else {
            # tabs screw everything up!
            $pointy = substr($_[0],0,$pos);
            $pointy =~ tr/\t //cd;
            # make everything into whitespace, but preserving tabs
            $pointy .= "^=== near there\n";
        }
    }

    my $errmsg = "$_[1], in\:\n$_[0]";

    if($i == -1) {
        # No newline.
        $errmsg .= "\n" . $pointy;
    }
    elsif($i == (length($_[0]) - 1)  ) {
        # Already has a newline at end.
        $errmsg .= $pointy;
    }
    else {
        # don't bother with the pointy bit, I guess.
    }
    Carp::croak( "$errmsg via $target, as used" );
}

1;
