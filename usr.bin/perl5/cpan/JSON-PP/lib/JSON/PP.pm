package JSON::PP;

# JSON-2.0

use 5.008;
use strict;

use Exporter ();
BEGIN { our @ISA = ('Exporter') }

use overload ();
use JSON::PP::Boolean;

use Carp ();
use Scalar::Util qw(blessed reftype refaddr);
#use Devel::Peek;

our $VERSION = '4.16';

our @EXPORT = qw(encode_json decode_json from_json to_json);

# instead of hash-access, i tried index-access for speed.
# but this method is not faster than what i expected. so it will be changed.

use constant P_ASCII                => 0;
use constant P_LATIN1               => 1;
use constant P_UTF8                 => 2;
use constant P_INDENT               => 3;
use constant P_CANONICAL            => 4;
use constant P_SPACE_BEFORE         => 5;
use constant P_SPACE_AFTER          => 6;
use constant P_ALLOW_NONREF         => 7;
use constant P_SHRINK               => 8;
use constant P_ALLOW_BLESSED        => 9;
use constant P_CONVERT_BLESSED      => 10;
use constant P_RELAXED              => 11;

use constant P_LOOSE                => 12;
use constant P_ALLOW_BIGNUM         => 13;
use constant P_ALLOW_BAREKEY        => 14;
use constant P_ALLOW_SINGLEQUOTE    => 15;
use constant P_ESCAPE_SLASH         => 16;
use constant P_AS_NONBLESSED        => 17;

use constant P_ALLOW_UNKNOWN        => 18;
use constant P_ALLOW_TAGS           => 19;

use constant USE_B => $ENV{PERL_JSON_PP_USE_B} || 0;
use constant CORE_BOOL => defined &builtin::is_bool;

my $invalid_char_re;

BEGIN {
    $invalid_char_re = "[";
    for my $i (0 .. 0x01F, 0x22, 0x5c) { # '/' is ok
        $invalid_char_re .= quotemeta chr utf8::unicode_to_native($i);
    }

    $invalid_char_re = qr/$invalid_char_re]/;
}

BEGIN {
    if (USE_B) {
        require B;
    }
}

BEGIN {
    my @xs_compati_bit_properties = qw(
            latin1 ascii utf8 indent canonical space_before space_after allow_nonref shrink
            allow_blessed convert_blessed relaxed allow_unknown
            allow_tags
    );
    my @pp_bit_properties = qw(
            allow_singlequote allow_bignum loose
            allow_barekey escape_slash as_nonblessed
    );

    for my $name (@xs_compati_bit_properties, @pp_bit_properties) {
        my $property_id = 'P_' . uc($name);

        eval qq/
            sub $name {
                my \$enable = defined \$_[1] ? \$_[1] : 1;

                if (\$enable) {
                    \$_[0]->{PROPS}->[$property_id] = 1;
                }
                else {
                    \$_[0]->{PROPS}->[$property_id] = 0;
                }

                \$_[0];
            }

            sub get_$name {
                \$_[0]->{PROPS}->[$property_id] ? 1 : '';
            }
        /;
    }

}



# Functions

my $JSON; # cache

sub encode_json ($) { # encode
    ($JSON ||= __PACKAGE__->new->utf8)->encode(@_);
}


sub decode_json { # decode
    ($JSON ||= __PACKAGE__->new->utf8)->decode(@_);
}

# Obsoleted

sub to_json($) {
   Carp::croak ("JSON::PP::to_json has been renamed to encode_json.");
}


sub from_json($) {
   Carp::croak ("JSON::PP::from_json has been renamed to decode_json.");
}


# Methods

sub new {
    my $class = shift;
    my $self  = {
        max_depth   => 512,
        max_size    => 0,
        indent_length => 3,
    };

    $self->{PROPS}[P_ALLOW_NONREF] = 1;

    bless $self, $class;
}


sub encode {
    return $_[0]->PP_encode_json($_[1]);
}


sub decode {
    return $_[0]->PP_decode_json($_[1], 0x00000000);
}


sub decode_prefix {
    return $_[0]->PP_decode_json($_[1], 0x00000001);
}


# accessor


# pretty printing

sub pretty {
    my ($self, $v) = @_;
    my $enable = defined $v ? $v : 1;

    if ($enable) { # indent_length(3) for JSON::XS compatibility
        $self->indent(1)->space_before(1)->space_after(1);
    }
    else {
        $self->indent(0)->space_before(0)->space_after(0);
    }

    $self;
}

# etc

sub max_depth {
    my $max  = defined $_[1] ? $_[1] : 0x80000000;
    $_[0]->{max_depth} = $max;
    $_[0];
}


sub get_max_depth { $_[0]->{max_depth}; }


sub max_size {
    my $max  = defined $_[1] ? $_[1] : 0;
    $_[0]->{max_size} = $max;
    $_[0];
}


sub get_max_size { $_[0]->{max_size}; }

sub boolean_values {
    my $self = shift;
    if (@_) {
        my ($false, $true) = @_;
        $self->{false} = $false;
        $self->{true} = $true;
        if (CORE_BOOL) {
            BEGIN { CORE_BOOL and warnings->unimport(qw(experimental::builtin)) }
            if (builtin::is_bool($true) && builtin::is_bool($false) && $true && !$false) {
                $self->{core_bools} = !!1;
            }
            else {
                delete $self->{core_bools};
            }
        }
    } else {
        delete $self->{false};
        delete $self->{true};
        delete $self->{core_bools};
    }
    return $self;
}

sub core_bools {
    my $self = shift;
    my $core_bools = defined $_[0] ? $_[0] : 1;
    if ($core_bools) {
        $self->{true} = !!1;
        $self->{false} = !!0;
        $self->{core_bools} = !!1;
    }
    else {
        $self->{true} = $JSON::PP::true;
        $self->{false} = $JSON::PP::false;
        $self->{core_bools} = !!0;
    }
    return $self;
}

sub get_core_bools {
    my $self = shift;
    return !!$self->{core_bools};
}

sub unblessed_bool {
    my $self = shift;
    return $self->core_bools(@_);
}

sub get_unblessed_bool {
    my $self = shift;
    return $self->get_core_bools(@_);
}

sub get_boolean_values {
    my $self = shift;
    if (exists $self->{true} and exists $self->{false}) {
        return @$self{qw/false true/};
    }
    return;
}

sub filter_json_object {
    if (defined $_[1] and ref $_[1] eq 'CODE') {
        $_[0]->{cb_object} = $_[1];
    } else {
        delete $_[0]->{cb_object};
    }
    $_[0]->{F_HOOK} = ($_[0]->{cb_object} or $_[0]->{cb_sk_object}) ? 1 : 0;
    $_[0];
}

sub filter_json_single_key_object {
    if (@_ == 1 or @_ > 3) {
        Carp::croak("Usage: JSON::PP::filter_json_single_key_object(self, key, callback = undef)");
    }
    if (defined $_[2] and ref $_[2] eq 'CODE') {
        $_[0]->{cb_sk_object}->{$_[1]} = $_[2];
    } else {
        delete $_[0]->{cb_sk_object}->{$_[1]};
        delete $_[0]->{cb_sk_object} unless %{$_[0]->{cb_sk_object} || {}};
    }
    $_[0]->{F_HOOK} = ($_[0]->{cb_object} or $_[0]->{cb_sk_object}) ? 1 : 0;
    $_[0];
}

sub indent_length {
    if (!defined $_[1] or $_[1] > 15 or $_[1] < 0) {
        Carp::carp "The acceptable range of indent_length() is 0 to 15.";
    }
    else {
        $_[0]->{indent_length} = $_[1];
    }
    $_[0];
}

sub get_indent_length {
    $_[0]->{indent_length};
}

sub sort_by {
    $_[0]->{sort_by} = defined $_[1] ? $_[1] : 1;
    $_[0];
}

sub allow_bigint {
    Carp::carp("allow_bigint() is obsoleted. use allow_bignum() instead.");
    $_[0]->allow_bignum;
}

###############################

###
### Perl => JSON
###


{ # Convert

    my $max_depth;
    my $indent;
    my $ascii;
    my $latin1;
    my $utf8;
    my $space_before;
    my $space_after;
    my $canonical;
    my $allow_blessed;
    my $convert_blessed;

    my $indent_length;
    my $escape_slash;
    my $bignum;
    my $as_nonblessed;
    my $allow_tags;

    my $depth;
    my $indent_count;
    my $keysort;


    sub PP_encode_json {
        my $self = shift;
        my $obj  = shift;

        $indent_count = 0;
        $depth        = 0;

        my $props = $self->{PROPS};

        ($ascii, $latin1, $utf8, $indent, $canonical, $space_before, $space_after, $allow_blessed,
            $convert_blessed, $escape_slash, $bignum, $as_nonblessed, $allow_tags)
         = @{$props}[P_ASCII .. P_SPACE_AFTER, P_ALLOW_BLESSED, P_CONVERT_BLESSED,
                    P_ESCAPE_SLASH, P_ALLOW_BIGNUM, P_AS_NONBLESSED, P_ALLOW_TAGS];

        ($max_depth, $indent_length) = @{$self}{qw/max_depth indent_length/};

        $keysort = $canonical ? sub { $a cmp $b } : undef;

        if ($self->{sort_by}) {
            $keysort = ref($self->{sort_by}) eq 'CODE' ? $self->{sort_by}
                     : $self->{sort_by} =~ /\D+/       ? $self->{sort_by}
                     : sub { $a cmp $b };
        }

        encode_error("hash- or arrayref expected (not a simple scalar, use allow_nonref to allow this)")
             if(!ref $obj and !$props->[ P_ALLOW_NONREF ]);

        my $str  = $self->object_to_json($obj);

        $str .= "\n" if ( $indent ); # JSON::XS 2.26 compatible

        return $str;
    }


    sub object_to_json {
        my ($self, $obj) = @_;
        my $type = ref($obj);

        if($type eq 'HASH'){
            return $self->hash_to_json($obj);
        }
        elsif($type eq 'ARRAY'){
            return $self->array_to_json($obj);
        }
        elsif ($type) { # blessed object?
            if (blessed($obj)) {

                return $self->value_to_json($obj) if ( $obj->isa('JSON::PP::Boolean') );

                if ( $allow_tags and $obj->can('FREEZE') ) {
                    my $obj_class = ref $obj || $obj;
                    $obj = bless $obj, $obj_class;
                    my @results = $obj->FREEZE('JSON');
                    if ( @results and ref $results[0] ) {
                        if ( refaddr( $obj ) eq refaddr( $results[0] ) ) {
                            encode_error( sprintf(
                                "%s::FREEZE method returned same object as was passed instead of a new one",
                                ref $obj
                            ) );
                        }
                    }
                    return '("'.$obj_class.'")['.join(',', @results).']';
                }

                if ( $convert_blessed and $obj->can('TO_JSON') ) {
                    my $result = $obj->TO_JSON();
                    if ( defined $result and ref( $result ) ) {
                        if ( refaddr( $obj ) eq refaddr( $result ) ) {
                            encode_error( sprintf(
                                "%s::TO_JSON method returned same object as was passed instead of a new one",
                                ref $obj
                            ) );
                        }
                    }

                    return $self->object_to_json( $result );
                }

                return "$obj" if ( $bignum and _is_bignum($obj) );

                if ($allow_blessed) {
                    return $self->blessed_to_json($obj) if ($as_nonblessed); # will be removed.
                    return 'null';
                }
                encode_error( sprintf("encountered object '%s', but neither allow_blessed, convert_blessed nor allow_tags settings are enabled (or TO_JSON/FREEZE method missing)", $obj)
                );
            }
            else {
                return $self->value_to_json($obj);
            }
        }
        else{
            return $self->value_to_json($obj);
        }
    }


    sub hash_to_json {
        my ($self, $obj) = @_;
        my @res;

        encode_error("json text or perl structure exceeds maximum nesting level (max_depth set too low?)")
                                         if (++$depth > $max_depth);

        my ($pre, $post) = $indent ? $self->_up_indent() : ('', '');
        my $del = ($space_before ? ' ' : '') . ':' . ($space_after ? ' ' : '');

        for my $k ( _sort( $obj ) ) {
            push @res, $self->string_to_json( $k )
                          .  $del
                          . ( ref $obj->{$k} ? $self->object_to_json( $obj->{$k} ) : $self->value_to_json( $obj->{$k} ) );
        }

        --$depth;
        $self->_down_indent() if ($indent);

        return '{}' unless @res;
        return '{' . $pre . join( ",$pre", @res ) . $post . '}';
    }


    sub array_to_json {
        my ($self, $obj) = @_;
        my @res;

        encode_error("json text or perl structure exceeds maximum nesting level (max_depth set too low?)")
                                         if (++$depth > $max_depth);

        my ($pre, $post) = $indent ? $self->_up_indent() : ('', '');

        for my $v (@$obj){
            push @res, ref($v) ? $self->object_to_json($v) : $self->value_to_json($v);
        }

        --$depth;
        $self->_down_indent() if ($indent);

        return '[]' unless @res;
        return '[' . $pre . join( ",$pre", @res ) . $post . ']';
    }

    sub _looks_like_number {
        my $value = shift;
        if (USE_B) {
            my $b_obj = B::svref_2object(\$value);
            my $flags = $b_obj->FLAGS;
            return 1 if $flags & ( B::SVp_IOK() | B::SVp_NOK() ) and !( $flags & B::SVp_POK() );
            return;
        } else {
            no warnings 'numeric';
            # if the utf8 flag is on, it almost certainly started as a string
            return if utf8::is_utf8($value);
            # detect numbers
            # string & "" -> ""
            # number & "" -> 0 (with warning)
            # nan and inf can detect as numbers, so check with * 0
            return unless length((my $dummy = "") & $value);
            return unless 0 + $value eq $value;
            return 1 if $value * 0 == 0;
            return -1; # inf/nan
        }
    }

    sub value_to_json {
        my ($self, $value) = @_;

        return 'null' if(!defined $value);

        my $type = ref($value);

        if (!$type) {
            BEGIN { CORE_BOOL and warnings->unimport('experimental::builtin') }
            if (CORE_BOOL && builtin::is_bool($value)) {
                return $value ? 'true' : 'false';
            }
            elsif (_looks_like_number($value)) {
                return $value;
            }
            return $self->string_to_json($value);
        }
        elsif( blessed($value) and  $value->isa('JSON::PP::Boolean') ){
            return $$value == 1 ? 'true' : 'false';
        }
        else {
            if ((overload::StrVal($value) =~ /=(\w+)/)[0]) {
                return $self->value_to_json("$value");
            }

            if ($type eq 'SCALAR' and defined $$value) {
                return   $$value eq '1' ? 'true'
                       : $$value eq '0' ? 'false'
                       : $self->{PROPS}->[ P_ALLOW_UNKNOWN ] ? 'null'
                       : encode_error("cannot encode reference to scalar");
            }

            if ( $self->{PROPS}->[ P_ALLOW_UNKNOWN ] ) {
                return 'null';
            }
            else {
                if ( $type eq 'SCALAR' or $type eq 'REF' ) {
                    encode_error("cannot encode reference to scalar");
                }
                else {
                    encode_error("encountered $value, but JSON can only represent references to arrays or hashes");
                }
            }

        }
    }


    my %esc = (
        "\n" => '\n',
        "\r" => '\r',
        "\t" => '\t',
        "\f" => '\f',
        "\b" => '\b',
        "\"" => '\"',
        "\\" => '\\\\',
        "\'" => '\\\'',
    );


    sub string_to_json {
        my ($self, $arg) = @_;

        $arg =~ s/(["\\\n\r\t\f\b])/$esc{$1}/g;
        $arg =~ s/\//\\\//g if ($escape_slash);

        # On ASCII platforms, matches [\x00-\x08\x0b\x0e-\x1f]
        $arg =~ s/([^\n\t\c?[:^cntrl:][:^ascii:]])/'\\u00' . unpack('H2', $1)/eg;

        if ($ascii) {
            $arg = _encode_ascii($arg);
        }

        if ($latin1) {
            $arg = _encode_latin1($arg);
        }

        if ($utf8) {
            utf8::encode($arg);
        }

        return '"' . $arg . '"';
    }


    sub blessed_to_json {
        my $reftype = reftype($_[1]) || '';
        if ($reftype eq 'HASH') {
            return $_[0]->hash_to_json($_[1]);
        }
        elsif ($reftype eq 'ARRAY') {
            return $_[0]->array_to_json($_[1]);
        }
        else {
            return 'null';
        }
    }


    sub encode_error {
        my $error  = shift;
        Carp::croak "$error";
    }


    sub _sort {
        defined $keysort ? (sort $keysort (keys %{$_[0]})) : keys %{$_[0]};
    }


    sub _up_indent {
        my $self  = shift;
        my $space = ' ' x $indent_length;

        my ($pre,$post) = ('','');

        $post = "\n" . $space x $indent_count;

        $indent_count++;

        $pre = "\n" . $space x $indent_count;

        return ($pre,$post);
    }


    sub _down_indent { $indent_count--; }


    sub PP_encode_box {
        {
            depth        => $depth,
            indent_count => $indent_count,
        };
    }

} # Convert


sub _encode_ascii {
    join('',
        map {
            chr($_) =~ /[[:ascii:]]/ ?
                chr($_) :
            $_ <= 65535 ?
                sprintf('\u%04x', $_) : sprintf('\u%x\u%x', _encode_surrogates($_));
        } unpack('U*', $_[0])
    );
}


sub _encode_latin1 {
    join('',
        map {
            $_ <= 255 ?
                chr($_) :
            $_ <= 65535 ?
                sprintf('\u%04x', $_) : sprintf('\u%x\u%x', _encode_surrogates($_));
        } unpack('U*', $_[0])
    );
}


sub _encode_surrogates { # from perlunicode
    my $uni = $_[0] - 0x10000;
    return ($uni / 0x400 + 0xD800, $uni % 0x400 + 0xDC00);
}


sub _is_bignum {
    $_[0]->isa('Math::BigInt') or $_[0]->isa('Math::BigFloat');
}



#
# JSON => Perl
#

my $max_intsize;

BEGIN {
    my $checkint = 1111;
    for my $d (5..64) {
        $checkint .= 1;
        my $int   = eval qq| $checkint |;
        if ($int =~ /[eE]/) {
            $max_intsize = $d - 1;
            last;
        }
    }
}

{ # PARSE 

    my %escapes = ( #  by Jeremy Muhlich <jmuhlich [at] bitflood.org>
        b    => "\b",
        t    => "\t",
        n    => "\n",
        f    => "\f",
        r    => "\r",
        '\\' => '\\',
        '"'  => '"',
        '/'  => '/',
    );

    my $text; # json data
    my $at;   # offset
    my $ch;   # first character
    my $len;  # text length (changed according to UTF8 or NON UTF8)
    # INTERNAL
    my $depth;          # nest counter
    my $encoding;       # json text encoding
    my $is_valid_utf8;  # temp variable
    my $utf8_len;       # utf8 byte length
    # FLAGS
    my $utf8;           # must be utf8
    my $max_depth;      # max nest number of objects and arrays
    my $max_size;
    my $relaxed;
    my $cb_object;
    my $cb_sk_object;

    my $F_HOOK;

    my $allow_bignum;   # using Math::BigInt/BigFloat
    my $singlequote;    # loosely quoting
    my $loose;          # 
    my $allow_barekey;  # bareKey
    my $allow_tags;

    my $alt_true;
    my $alt_false;

    sub _detect_utf_encoding {
        my $text = shift;
        my @octets = unpack('C4', $text);
        return 'unknown' unless defined $octets[3];
        return ( $octets[0] and  $octets[1]) ? 'UTF-8'
             : (!$octets[0] and  $octets[1]) ? 'UTF-16BE'
             : (!$octets[0] and !$octets[1]) ? 'UTF-32BE'
             : ( $octets[2]                ) ? 'UTF-16LE'
             : (!$octets[2]                ) ? 'UTF-32LE'
             : 'unknown';
    }

    sub PP_decode_json {
        my ($self, $want_offset);

        ($self, $text, $want_offset) = @_;

        ($at, $ch, $depth) = (0, '', 0);

        if ( !defined $text or ref $text ) {
            decode_error("malformed JSON string, neither array, object, number, string or atom");
        }

        my $props = $self->{PROPS};

        ($utf8, $relaxed, $loose, $allow_bignum, $allow_barekey, $singlequote, $allow_tags)
            = @{$props}[P_UTF8, P_RELAXED, P_LOOSE .. P_ALLOW_SINGLEQUOTE, P_ALLOW_TAGS];

        ($alt_true, $alt_false) = @$self{qw/true false/};

        if ( $utf8 ) {
            $encoding = _detect_utf_encoding($text);
            if ($encoding ne 'UTF-8' and $encoding ne 'unknown') {
                require Encode;
                Encode::from_to($text, $encoding, 'utf-8');
            } else {
                utf8::downgrade( $text, 1 ) or Carp::croak("Wide character in subroutine entry");
            }
        }
        else {
            utf8::encode( $text );
        }

        $len = length $text;

        ($max_depth, $max_size, $cb_object, $cb_sk_object, $F_HOOK)
             = @{$self}{qw/max_depth  max_size cb_object cb_sk_object F_HOOK/};

        if ($max_size > 1) {
            use bytes;
            my $bytes = length $text;
            decode_error(
                sprintf("attempted decode of JSON text of %s bytes size, but max_size is set to %s"
                    , $bytes, $max_size), 1
            ) if ($bytes > $max_size);
        }

        white(); # remove head white space

        decode_error("malformed JSON string, neither array, object, number, string or atom") unless defined $ch; # Is there a first character for JSON structure?

        my $result = value();

        if ( !$props->[ P_ALLOW_NONREF ] and !ref $result ) {
                decode_error(
                'JSON text must be an object or array (but found number, string, true, false or null,'
                       . ' use allow_nonref to allow this)', 1);
        }

        Carp::croak('something wrong.') if $len < $at; # we won't arrive here.

        my $consumed = defined $ch ? $at - 1 : $at; # consumed JSON text length

        white(); # remove tail white space

        return ( $result, $consumed ) if $want_offset; # all right if decode_prefix

        decode_error("garbage after JSON object") if defined $ch;

        $result;
    }


    sub next_chr {
        return $ch = undef if($at >= $len);
        $ch = substr($text, $at++, 1);
    }


    sub value {
        white();
        return          if(!defined $ch);
        return object() if($ch eq '{');
        return array()  if($ch eq '[');
        return tag()    if($ch eq '(');
        return string() if($ch eq '"' or ($singlequote and $ch eq "'"));
        return number() if($ch =~ /[0-9]/ or $ch eq '-');
        return word();
    }

    sub string {
        my $utf16;
        my $is_utf8;

        ($is_valid_utf8, $utf8_len) = ('', 0);

        my $s = ''; # basically UTF8 flag on

        if($ch eq '"' or ($singlequote and $ch eq "'")){
            my $boundChar = $ch;

            OUTER: while( defined(next_chr()) ){

                if($ch eq $boundChar){
                    next_chr();

                    if ($utf16) {
                        decode_error("missing low surrogate character in surrogate pair");
                    }

                    utf8::decode($s) if($is_utf8);

                    return $s;
                }
                elsif($ch eq '\\'){
                    next_chr();
                    if(exists $escapes{$ch}){
                        $s .= $escapes{$ch};
                    }
                    elsif($ch eq 'u'){ # UNICODE handling
                        my $u = '';

                        for(1..4){
                            $ch = next_chr();
                            last OUTER if($ch !~ /[0-9a-fA-F]/);
                            $u .= $ch;
                        }

                        # U+D800 - U+DBFF
                        if ($u =~ /^[dD][89abAB][0-9a-fA-F]{2}/) { # UTF-16 high surrogate?
                            $utf16 = $u;
                        }
                        # U+DC00 - U+DFFF
                        elsif ($u =~ /^[dD][c-fC-F][0-9a-fA-F]{2}/) { # UTF-16 low surrogate?
                            unless (defined $utf16) {
                                decode_error("missing high surrogate character in surrogate pair");
                            }
                            $is_utf8 = 1;
                            $s .= _decode_surrogates($utf16, $u) || next;
                            $utf16 = undef;
                        }
                        else {
                            if (defined $utf16) {
                                decode_error("surrogate pair expected");
                            }

                            my $hex = hex( $u );
                            if ( chr $u =~ /[[:^ascii:]]/ ) {
                                $is_utf8 = 1;
                                $s .= _decode_unicode($u) || next;
                            }
                            else {
                                $s .= chr $hex;
                            }
                        }

                    }
                    else{
                        unless ($loose) {
                            $at -= 2;
                            decode_error('illegal backslash escape sequence in string');
                        }
                        $s .= $ch;
                    }
                }
                else{

                    if ( $ch =~ /[[:^ascii:]]/ ) {
                        unless( $ch = is_valid_utf8($ch) ) {
                            $at -= 1;
                            decode_error("malformed UTF-8 character in JSON string");
                        }
                        else {
                            $at += $utf8_len - 1;
                        }

                        $is_utf8 = 1;
                    }

                    if (!$loose) {
                        if ($ch =~ $invalid_char_re)  { # '/' ok
                            if (!$relaxed or $ch ne "\t") {
                                $at--;
                                decode_error(sprintf "invalid character 0x%X"
                                   . " encountered while parsing JSON string",
                                   ord $ch);
                            }
                        }
                    }

                    $s .= $ch;
                }
            }
        }

        decode_error("unexpected end of string while parsing JSON string");
    }


    sub white {
        while( defined $ch  ){
            if($ch eq '' or $ch =~ /\A[ \t\r\n]\z/){
                next_chr();
            }
            elsif($relaxed and $ch eq '/'){
                next_chr();
                if(defined $ch and $ch eq '/'){
                    1 while(defined(next_chr()) and $ch ne "\n" and $ch ne "\r");
                }
                elsif(defined $ch and $ch eq '*'){
                    next_chr();
                    while(1){
                        if(defined $ch){
                            if($ch eq '*'){
                                if(defined(next_chr()) and $ch eq '/'){
                                    next_chr();
                                    last;
                                }
                            }
                            else{
                                next_chr();
                            }
                        }
                        else{
                            decode_error("Unterminated comment");
                        }
                    }
                    next;
                }
                else{
                    $at--;
                    decode_error("malformed JSON string, neither array, object, number, string or atom");
                }
            }
            else{
                if ($relaxed and $ch eq '#') { # correctly?
                    pos($text) = $at;
                    $text =~ /\G([^\n]*(?:\r\n|\r|\n|$))/g;
                    $at = pos($text);
                    next_chr;
                    next;
                }

                last;
            }
        }
    }


    sub array {
        my $a  = $_[0] || []; # you can use this code to use another array ref object.

        decode_error('json text or perl structure exceeds maximum nesting level (max_depth set too low?)')
                                                    if (++$depth > $max_depth);

        next_chr();
        white();

        if(defined $ch and $ch eq ']'){
            --$depth;
            next_chr();
            return $a;
        }
        else {
            while(defined($ch)){
                push @$a, value();

                white();

                if (!defined $ch) {
                    last;
                }

                if($ch eq ']'){
                    --$depth;
                    next_chr();
                    return $a;
                }

                if($ch ne ','){
                    last;
                }

                next_chr();
                white();

                if ($relaxed and $ch eq ']') {
                    --$depth;
                    next_chr();
                    return $a;
                }

            }
        }

        $at-- if defined $ch and $ch ne '';
        decode_error(", or ] expected while parsing array");
    }

    sub tag {
        decode_error('malformed JSON string, neither array, object, number, string or atom') unless $allow_tags;

        next_chr();
        white();

        my $tag = value();
        return unless defined $tag;
        decode_error('malformed JSON string, (tag) must be a string') if ref $tag;

        white();

        if (!defined $ch or $ch ne ')') {
            decode_error(') expected after tag');
        }

        next_chr();
        white();

        my $val = value();
        return unless defined $val;
        decode_error('malformed JSON string, tag value must be an array') unless ref $val eq 'ARRAY';

        if (!eval { $tag->can('THAW') }) {
             decode_error('cannot decode perl-object (package does not exist)') if $@;
             decode_error('cannot decode perl-object (package does not have a THAW method)');
        }
        $tag->THAW('JSON', @$val);
    }

    sub object {
        my $o = $_[0] || {}; # you can use this code to use another hash ref object.
        my $k;

        decode_error('json text or perl structure exceeds maximum nesting level (max_depth set too low?)')
                                                if (++$depth > $max_depth);
        next_chr();
        white();

        if(defined $ch and $ch eq '}'){
            --$depth;
            next_chr();
            if ($F_HOOK) {
                return _json_object_hook($o);
            }
            return $o;
        }
        else {
            while (defined $ch) {
                $k = ($allow_barekey and $ch ne '"' and $ch ne "'") ? bareKey() : string();
                white();

                if(!defined $ch or $ch ne ':'){
                    $at--;
                    decode_error("':' expected");
                }

                next_chr();
                $o->{$k} = value();
                white();

                last if (!defined $ch);

                if($ch eq '}'){
                    --$depth;
                    next_chr();
                    if ($F_HOOK) {
                        return _json_object_hook($o);
                    }
                    return $o;
                }

                if($ch ne ','){
                    last;
                }

                next_chr();
                white();

                if ($relaxed and $ch eq '}') {
                    --$depth;
                    next_chr();
                    if ($F_HOOK) {
                        return _json_object_hook($o);
                    }
                    return $o;
                }

            }

        }

        $at-- if defined $ch and $ch ne '';
        decode_error(", or } expected while parsing object/hash");
    }


    sub bareKey { # doesn't strictly follow Standard ECMA-262 3rd Edition
        my $key;
        while($ch =~ /[\$\w[:^ascii:]]/){
            $key .= $ch;
            next_chr();
        }
        return $key;
    }


    sub word {
        my $word =  substr($text,$at-1,4);

        if($word eq 'true'){
            $at += 3;
            next_chr;
            return defined $alt_true ? $alt_true : $JSON::PP::true;
        }
        elsif($word eq 'null'){
            $at += 3;
            next_chr;
            return undef;
        }
        elsif($word eq 'fals'){
            $at += 3;
            if(substr($text,$at,1) eq 'e'){
                $at++;
                next_chr;
                return defined $alt_false ? $alt_false : $JSON::PP::false;
            }
        }

        $at--; # for decode_error report

        decode_error("'null' expected")  if ($word =~ /^n/);
        decode_error("'true' expected")  if ($word =~ /^t/);
        decode_error("'false' expected") if ($word =~ /^f/);
        decode_error("malformed JSON string, neither array, object, number, string or atom");
    }


    sub number {
        my $n    = '';
        my $v;
        my $is_dec;
        my $is_exp;

        if($ch eq '-'){
            $n = '-';
            next_chr;
            if (!defined $ch or $ch !~ /\d/) {
                decode_error("malformed number (no digits after initial minus)");
            }
        }

        # According to RFC4627, hex or oct digits are invalid.
        if($ch eq '0'){
            my $peek = substr($text,$at,1);
            if($peek =~ /^[0-9a-dfA-DF]/){ # e may be valid (exponential)
                decode_error("malformed number (leading zero must not be followed by another digit)");
            }
            $n .= $ch;
            next_chr;
        }

        while(defined $ch and $ch =~ /\d/){
            $n .= $ch;
            next_chr;
        }

        if(defined $ch and $ch eq '.'){
            $n .= '.';
            $is_dec = 1;

            next_chr;
            if (!defined $ch or $ch !~ /\d/) {
                decode_error("malformed number (no digits after decimal point)");
            }
            else {
                $n .= $ch;
            }

            while(defined(next_chr) and $ch =~ /\d/){
                $n .= $ch;
            }
        }

        if(defined $ch and ($ch eq 'e' or $ch eq 'E')){
            $n .= $ch;
            $is_exp = 1;
            next_chr;

            if(defined($ch) and ($ch eq '+' or $ch eq '-')){
                $n .= $ch;
                next_chr;
                if (!defined $ch or $ch =~ /\D/) {
                    decode_error("malformed number (no digits after exp sign)");
                }
                $n .= $ch;
            }
            elsif(defined($ch) and $ch =~ /\d/){
                $n .= $ch;
            }
            else {
                decode_error("malformed number (no digits after exp sign)");
            }

            while(defined(next_chr) and $ch =~ /\d/){
                $n .= $ch;
            }

        }

        $v .= $n;

        if ($is_dec or $is_exp) {
            if ($allow_bignum) {
                require Math::BigFloat;
                return Math::BigFloat->new($v);
            }
        } else {
            if (length $v > $max_intsize) {
                if ($allow_bignum) { # from Adam Sussman
                    require Math::BigInt;
                    return Math::BigInt->new($v);
                }
                else {
                    return "$v";
                }
            }
        }

        return $is_dec ? $v/1.0 : 0+$v;
    }

    # Compute how many bytes are in the longest legal official Unicode
    # character
    my $max_unicode_length = do {
      no warnings 'utf8';
      chr 0x10FFFF;
    };
    utf8::encode($max_unicode_length);
    $max_unicode_length = length $max_unicode_length;

    sub is_valid_utf8 {

        # Returns undef (setting $utf8_len to 0) unless the next bytes in $text
        # comprise a well-formed UTF-8 encoded character, in which case,
        # return those bytes, setting $utf8_len to their count.

        my $start_point = substr($text, $at - 1);

        # Look no further than the maximum number of bytes in a single
        # character
        my $limit = $max_unicode_length;
        $limit = length($start_point) if $limit > length($start_point);

        # Find the number of bytes comprising the first character in $text
        # (without having to know the details of its internal representation).
        # This loop will iterate just once on well-formed input.
        while ($limit > 0) {    # Until we succeed or exhaust the input
            my $copy = substr($start_point, 0, $limit);

            # decode() will return true if all bytes are valid; false
            # if any aren't.
            if (utf8::decode($copy)) {

                # Is valid: get the first character, convert back to bytes,
                # and return those bytes.
                $copy = substr($copy, 0, 1);
                utf8::encode($copy);
                $utf8_len = length $copy;
                return substr($start_point, 0, $utf8_len);
            }

            # If it didn't work, it could be that there is a full legal character
            # followed by a partial or malformed one.  Narrow the window and
            # try again.
            $limit--;
        }

        # Failed to find a legal UTF-8 character.
        $utf8_len = 0;
        return;
    }


    sub decode_error {
        my $error  = shift;
        my $no_rep = shift;
        my $str    = defined $text ? substr($text, $at) : '';
        my $mess   = '';
        my $type   = 'U*';

        for my $c ( unpack( $type, $str ) ) { # emulate pv_uni_display() ?
            my $chr_c = chr($c);
            $mess .=  $chr_c eq '\\' ? '\\\\'
                    : $chr_c =~ /[[:print:]]/ ? $chr_c
                    : $chr_c eq '\a' ? '\a'
                    : $chr_c eq '\t' ? '\t'
                    : $chr_c eq '\n' ? '\n'
                    : $chr_c eq '\r' ? '\r'
                    : $chr_c eq '\f' ? '\f'
                    : sprintf('\x{%x}', $c)
                    ;
            if ( length $mess >= 20 ) {
                $mess .= '...';
                last;
            }
        }

        unless ( length $mess ) {
            $mess = '(end of string)';
        }

        Carp::croak (
            $no_rep ? "$error" : "$error, at character offset $at (before \"$mess\")"
        );

    }


    sub _json_object_hook {
        my $o    = $_[0];
        my @ks = keys %{$o};

        if ( $cb_sk_object and @ks == 1 and exists $cb_sk_object->{ $ks[0] } and ref $cb_sk_object->{ $ks[0] } ) {
            my @val = $cb_sk_object->{ $ks[0] }->( $o->{$ks[0]} );
            if (@val == 0) {
                return $o;
            }
            elsif (@val == 1) {
                return $val[0];
            }
            else {
                Carp::croak("filter_json_single_key_object callbacks must not return more than one scalar");
            }
        }

        my @val = $cb_object->($o) if ($cb_object);
        if (@val == 0) {
            return $o;
        }
        elsif (@val == 1) {
            return $val[0];
        }
        else {
            Carp::croak("filter_json_object callbacks must not return more than one scalar");
        }
    }


    sub PP_decode_box {
        {
            text    => $text,
            at      => $at,
            ch      => $ch,
            len     => $len,
            depth   => $depth,
            encoding      => $encoding,
            is_valid_utf8 => $is_valid_utf8,
        };
    }

} # PARSE


sub _decode_surrogates { # from perlunicode
    my $uni = 0x10000 + (hex($_[0]) - 0xD800) * 0x400 + (hex($_[1]) - 0xDC00);
    my $un  = pack('U*', $uni);
    utf8::encode( $un );
    return $un;
}


sub _decode_unicode {
    my $un = pack('U', hex shift);
    utf8::encode( $un );
    return $un;
}

sub incr_parse {
    local $Carp::CarpLevel = 1;
    ( $_[0]->{_incr_parser} ||= JSON::PP::IncrParser->new )->incr_parse( @_ );
}


sub incr_skip {
    ( $_[0]->{_incr_parser} ||= JSON::PP::IncrParser->new )->incr_skip;
}


sub incr_reset {
    ( $_[0]->{_incr_parser} ||= JSON::PP::IncrParser->new )->incr_reset;
}

sub incr_text : lvalue {
    $_[0]->{_incr_parser} ||= JSON::PP::IncrParser->new;

    if ( $_[0]->{_incr_parser}->{incr_pos} ) {
        Carp::croak("incr_text cannot be called when the incremental parser already started parsing");
    }
    $_[0]->{_incr_parser}->{incr_text};
}


###############################
# Utilities
#

# shamelessly copied and modified from JSON::XS code.

$JSON::PP::true  = do { bless \(my $dummy = 1), "JSON::PP::Boolean" };
$JSON::PP::false = do { bless \(my $dummy = 0), "JSON::PP::Boolean" };

sub is_bool {
  if (blessed $_[0]) {
    return (
      $_[0]->isa("JSON::PP::Boolean")
      or $_[0]->isa("Types::Serialiser::BooleanBase")
      or $_[0]->isa("JSON::XS::Boolean")
    );
  }
  elsif (CORE_BOOL) {
    BEGIN { CORE_BOOL and warnings->unimport('experimental::builtin') }
    return builtin::is_bool($_[0]);
  }
  return !!0;
}

sub true  { $JSON::PP::true  }
sub false { $JSON::PP::false }
sub null  { undef; }

###############################

package JSON::PP::IncrParser;

use strict;

use constant INCR_M_WS   => 0; # initial whitespace skipping
use constant INCR_M_STR  => 1; # inside string
use constant INCR_M_BS   => 2; # inside backslash
use constant INCR_M_JSON => 3; # outside anything, count nesting
use constant INCR_M_C0   => 4;
use constant INCR_M_C1   => 5;
use constant INCR_M_TFN  => 6;
use constant INCR_M_NUM  => 7;

our $VERSION = '1.01';

sub new {
    my ( $class ) = @_;

    bless {
        incr_nest    => 0,
        incr_text    => undef,
        incr_pos     => 0,
        incr_mode    => 0,
    }, $class;
}


sub incr_parse {
    my ( $self, $coder, $text ) = @_;

    $self->{incr_text} = '' unless ( defined $self->{incr_text} );

    if ( defined $text ) {
        $self->{incr_text} .= $text;
    }

    if ( defined wantarray ) {
        my $max_size = $coder->get_max_size;
        my $p = $self->{incr_pos};
        my @ret;
        {
            do {
                unless ( $self->{incr_nest} <= 0 and $self->{incr_mode} == INCR_M_JSON ) {
                    $self->_incr_parse( $coder );

                    if ( $max_size and $self->{incr_pos} > $max_size ) {
                        Carp::croak("attempted decode of JSON text of $self->{incr_pos} bytes size, but max_size is set to $max_size");
                    }
                    unless ( $self->{incr_nest} <= 0 and $self->{incr_mode} == INCR_M_JSON ) {
                        # as an optimisation, do not accumulate white space in the incr buffer
                        if ( $self->{incr_mode} == INCR_M_WS and $self->{incr_pos} ) {
                            $self->{incr_pos} = 0;
                            $self->{incr_text} = '';
                        }
                        last;
                    }
                }

                unless ( $coder->get_utf8 ) {
                    utf8::decode( $self->{incr_text} );
                }

                my ($obj, $offset) = $coder->PP_decode_json( $self->{incr_text}, 0x00000001 );
                push @ret, $obj;
                use bytes;
                $self->{incr_text} = substr( $self->{incr_text}, $offset || 0 );
                $self->{incr_pos} = 0;
                $self->{incr_nest} = 0;
                $self->{incr_mode} = 0;
                last unless wantarray;
            } while ( wantarray );
        }

        if ( wantarray ) {
            return @ret;
        }
        else { # in scalar context
            return defined $ret[0] ? $ret[0] : undef;
        }
    }
}


sub _incr_parse {
    my ($self, $coder) = @_;
    my $text = $self->{incr_text};
    my $len = length $text;
    my $p = $self->{incr_pos};

INCR_PARSE:
    while ( $len > $p ) {
        my $s = substr( $text, $p, 1 );
        last INCR_PARSE unless defined $s;
        my $mode = $self->{incr_mode};

        if ( $mode == INCR_M_WS ) {
            while ( $len > $p ) {
                $s = substr( $text, $p, 1 );
                last INCR_PARSE unless defined $s;
                if ( ord($s) > ord " " ) {
                    if ( $s eq '#' ) {
                        $self->{incr_mode} = INCR_M_C0;
                        redo INCR_PARSE;
                    } else {
                        $self->{incr_mode} = INCR_M_JSON;
                        redo INCR_PARSE;
                    }
                }
                $p++;
            }
        } elsif ( $mode == INCR_M_BS ) {
            $p++;
            $self->{incr_mode} = INCR_M_STR;
            redo INCR_PARSE;
        } elsif ( $mode == INCR_M_C0 or $mode == INCR_M_C1 ) {
            while ( $len > $p ) {
                $s = substr( $text, $p, 1 );
                last INCR_PARSE unless defined $s;
                if ( $s eq "\n" ) {
                    $self->{incr_mode} = $self->{incr_mode} == INCR_M_C0 ? INCR_M_WS : INCR_M_JSON;
                    last;
                }
                $p++;
            }
            next;
        } elsif ( $mode == INCR_M_TFN ) {
            last INCR_PARSE if $p >= $len && $self->{incr_nest};
            while ( $len > $p ) {
                $s = substr( $text, $p++, 1 );
                next if defined $s and $s =~ /[rueals]/;
                last;
            }
            $p--;
            $self->{incr_mode} = INCR_M_JSON;

            last INCR_PARSE unless $self->{incr_nest};
            redo INCR_PARSE;
        } elsif ( $mode == INCR_M_NUM ) {
            last INCR_PARSE if $p >= $len && $self->{incr_nest};
            while ( $len > $p ) {
                $s = substr( $text, $p++, 1 );
                next if defined $s and $s =~ /[0-9eE.+\-]/;
                last;
            }
            $p--;
            $self->{incr_mode} = INCR_M_JSON;

            last INCR_PARSE unless $self->{incr_nest};
            redo INCR_PARSE;
        } elsif ( $mode == INCR_M_STR ) {
            while ( $len > $p ) {
                $s = substr( $text, $p, 1 );
                last INCR_PARSE unless defined $s;
                if ( $s eq '"' ) {
                    $p++;
                    $self->{incr_mode} = INCR_M_JSON;

                    last INCR_PARSE unless $self->{incr_nest};
                    redo INCR_PARSE;
                }
                elsif ( $s eq '\\' ) {
                    $p++;
                    if ( !defined substr($text, $p, 1) ) {
                        $self->{incr_mode} = INCR_M_BS;
                        last INCR_PARSE;
                    }
                }
                $p++;
            }
        } elsif ( $mode == INCR_M_JSON ) {
            while ( $len > $p ) {
                $s = substr( $text, $p++, 1 );
                if ( $s eq "\x00" ) {
                    $p--;
                    last INCR_PARSE;
                } elsif ( $s =~ /^[\t\n\r ]$/) {
                    if ( !$self->{incr_nest} ) {
                        $p--; # do not eat the whitespace, let the next round do it
                        last INCR_PARSE;
                    }
                    next;
                } elsif ( $s eq 't' or $s eq 'f' or $s eq 'n' ) {
                    $self->{incr_mode} = INCR_M_TFN;
                    redo INCR_PARSE;
                } elsif ( $s =~ /^[0-9\-]$/ ) {
                    $self->{incr_mode} = INCR_M_NUM;
                    redo INCR_PARSE;
                } elsif ( $s eq '"' ) {
                    $self->{incr_mode} = INCR_M_STR;
                    redo INCR_PARSE;
                } elsif ( $s eq '[' or $s eq '{' ) {
                    if ( ++$self->{incr_nest} > $coder->get_max_depth ) {
                        Carp::croak('json text or perl structure exceeds maximum nesting level (max_depth set too low?)');
                    }
                    next;
                } elsif ( $s eq ']' or $s eq '}' ) {
                    if ( --$self->{incr_nest} <= 0 ) {
                        last INCR_PARSE;
                    }
                } elsif ( $s eq '#' ) {
                    $self->{incr_mode} = INCR_M_C1;
                    redo INCR_PARSE;
                }
            }
        }
    }

    $self->{incr_pos} = $p;
    $self->{incr_parsing} = $p ? 1 : 0; # for backward compatibility
}


sub incr_text {
    if ( $_[0]->{incr_pos} ) {
        Carp::croak("incr_text cannot be called when the incremental parser already started parsing");
    }
    $_[0]->{incr_text};
}


sub incr_skip {
    my $self  = shift;
    $self->{incr_text} = substr( $self->{incr_text}, $self->{incr_pos} );
    $self->{incr_pos}     = 0;
    $self->{incr_mode}    = 0;
    $self->{incr_nest}    = 0;
}


sub incr_reset {
    my $self = shift;
    $self->{incr_text}    = undef;
    $self->{incr_pos}     = 0;
    $self->{incr_mode}    = 0;
    $self->{incr_nest}    = 0;
}

###############################


1;
__END__
=pod

=head1 NAME

JSON::PP - JSON::XS compatible pure-Perl module.

=head1 SYNOPSIS

 use JSON::PP;

 # exported functions, they croak on error
 # and expect/generate UTF-8

 $utf8_encoded_json_text = encode_json $perl_hash_or_arrayref;
 $perl_hash_or_arrayref  = decode_json $utf8_encoded_json_text;

 # OO-interface

 $json = JSON::PP->new->ascii->pretty->allow_nonref;
 
 $pretty_printed_json_text = $json->encode( $perl_scalar );
 $perl_scalar = $json->decode( $json_text );
 
 # Note that JSON version 2.0 and above will automatically use
 # JSON::XS or JSON::PP, so you should be able to just:
 
 use JSON;


=head1 DESCRIPTION

JSON::PP is a pure perl JSON decoder/encoder, and (almost) compatible to much
faster L<JSON::XS> written by Marc Lehmann in C. JSON::PP works as
a fallback module when you use L<JSON> module without having
installed JSON::XS.

Because of this fallback feature of JSON.pm, JSON::PP tries not to
be more JavaScript-friendly than JSON::XS (i.e. not to escape extra
characters such as U+2028 and U+2029, etc),
in order for you not to lose such JavaScript-friendliness silently
when you use JSON.pm and install JSON::XS for speed or by accident.
If you need JavaScript-friendly RFC7159-compliant pure perl module,
try L<JSON::Tiny>, which is derived from L<Mojolicious> web
framework and is also smaller and faster than JSON::PP.

JSON::PP has been in the Perl core since Perl 5.14, mainly for
CPAN toolchain modules to parse META.json.

=head1 FUNCTIONAL INTERFACE

This section is taken from JSON::XS almost verbatim. C<encode_json>
and C<decode_json> are exported by default.

=head2 encode_json

    $json_text = encode_json $perl_scalar

Converts the given Perl data structure to a UTF-8 encoded, binary string
(that is, the string contains octets only). Croaks on error.

This function call is functionally identical to:

    $json_text = JSON::PP->new->utf8->encode($perl_scalar)

Except being faster.

=head2 decode_json

    $perl_scalar = decode_json $json_text

The opposite of C<encode_json>: expects an UTF-8 (binary) string and tries
to parse that as an UTF-8 encoded JSON text, returning the resulting
reference. Croaks on error.

This function call is functionally identical to:

    $perl_scalar = JSON::PP->new->utf8->decode($json_text)

Except being faster.

=head2 JSON::PP::is_bool

    $is_boolean = JSON::PP::is_bool($scalar)

Returns true if the passed scalar represents either JSON::PP::true or
JSON::PP::false, two constants that act like C<1> and C<0> respectively
and are also used to represent JSON C<true> and C<false> in Perl strings.

On perl 5.36 and above, will also return true when given one of perl's
standard boolean values, such as the result of a comparison.

See L<MAPPING>, below, for more information on how JSON values are mapped to
Perl.

=head1 OBJECT-ORIENTED INTERFACE

This section is also taken from JSON::XS.

The object oriented interface lets you configure your own encoding or
decoding style, within the limits of supported formats.

=head2 new

    $json = JSON::PP->new

Creates a new JSON::PP object that can be used to de/encode JSON
strings. All boolean flags described below are by default I<disabled>
(with the exception of C<allow_nonref>, which defaults to I<enabled> since
version C<4.0>).

The mutators for flags all return the JSON::PP object again and thus calls can
be chained:

   my $json = JSON::PP->new->utf8->space_after->encode({a => [1,2]})
   => {"a": [1, 2]}

=head2 ascii

    $json = $json->ascii([$enable])
    
    $enabled = $json->get_ascii

If C<$enable> is true (or missing), then the C<encode> method will not
generate characters outside the code range C<0..127> (which is ASCII). Any
Unicode characters outside that range will be escaped using either a
single \uXXXX (BMP characters) or a double \uHHHH\uLLLLL escape sequence,
as per RFC4627. The resulting encoded JSON text can be treated as a native
Unicode string, an ascii-encoded, latin1-encoded or UTF-8 encoded string,
or any other superset of ASCII.

If C<$enable> is false, then the C<encode> method will not escape Unicode
characters unless required by the JSON syntax or other flags. This results
in a faster and more compact format.

See also the section I<ENCODING/CODESET FLAG NOTES> later in this document.

The main use for this flag is to produce JSON texts that can be
transmitted over a 7-bit channel, as the encoded JSON texts will not
contain any 8 bit characters.

  JSON::PP->new->ascii(1)->encode([chr 0x10401])
  => ["\ud801\udc01"]

=head2 latin1

    $json = $json->latin1([$enable])
    
    $enabled = $json->get_latin1

If C<$enable> is true (or missing), then the C<encode> method will encode
the resulting JSON text as latin1 (or iso-8859-1), escaping any characters
outside the code range C<0..255>. The resulting string can be treated as a
latin1-encoded JSON text or a native Unicode string. The C<decode> method
will not be affected in any way by this flag, as C<decode> by default
expects Unicode, which is a strict superset of latin1.

If C<$enable> is false, then the C<encode> method will not escape Unicode
characters unless required by the JSON syntax or other flags.

See also the section I<ENCODING/CODESET FLAG NOTES> later in this document.

The main use for this flag is efficiently encoding binary data as JSON
text, as most octets will not be escaped, resulting in a smaller encoded
size. The disadvantage is that the resulting JSON text is encoded
in latin1 (and must correctly be treated as such when storing and
transferring), a rare encoding for JSON. It is therefore most useful when
you want to store data structures known to contain binary data efficiently
in files or databases, not when talking to other JSON encoders/decoders.

  JSON::PP->new->latin1->encode (["\x{89}\x{abc}"]
  => ["\x{89}\\u0abc"]    # (perl syntax, U+abc escaped, U+89 not)

=head2 utf8

    $json = $json->utf8([$enable])
    
    $enabled = $json->get_utf8

If C<$enable> is true (or missing), then the C<encode> method will encode
the JSON result into UTF-8, as required by many protocols, while the
C<decode> method expects to be handled an UTF-8-encoded string.  Please
note that UTF-8-encoded strings do not contain any characters outside the
range C<0..255>, they are thus useful for bytewise/binary I/O. In future
versions, enabling this option might enable autodetection of the UTF-16
and UTF-32 encoding families, as described in RFC4627.

If C<$enable> is false, then the C<encode> method will return the JSON
string as a (non-encoded) Unicode string, while C<decode> expects thus a
Unicode string.  Any decoding or encoding (e.g. to UTF-8 or UTF-16) needs
to be done yourself, e.g. using the Encode module.

See also the section I<ENCODING/CODESET FLAG NOTES> later in this document.

Example, output UTF-16BE-encoded JSON:

  use Encode;
  $jsontext = encode "UTF-16BE", JSON::PP->new->encode ($object);

Example, decode UTF-32LE-encoded JSON:

  use Encode;
  $object = JSON::PP->new->decode (decode "UTF-32LE", $jsontext);

=head2 pretty

    $json = $json->pretty([$enable])

This enables (or disables) all of the C<indent>, C<space_before> and
C<space_after> (and in the future possibly more) flags in one call to
generate the most readable (or most compact) form possible.

=head2 indent

    $json = $json->indent([$enable])
    
    $enabled = $json->get_indent

If C<$enable> is true (or missing), then the C<encode> method will use a multiline
format as output, putting every array member or object/hash key-value pair
into its own line, indenting them properly.

If C<$enable> is false, no newlines or indenting will be produced, and the
resulting JSON text is guaranteed not to contain any C<newlines>.

This setting has no effect when decoding JSON texts.

The default indent space length is three.
You can use C<indent_length> to change the length.

=head2 space_before

    $json = $json->space_before([$enable])
    
    $enabled = $json->get_space_before

If C<$enable> is true (or missing), then the C<encode> method will add an extra
optional space before the C<:> separating keys from values in JSON objects.

If C<$enable> is false, then the C<encode> method will not add any extra
space at those places.

This setting has no effect when decoding JSON texts. You will also
most likely combine this setting with C<space_after>.

Example, space_before enabled, space_after and indent disabled:

   {"key" :"value"}

=head2 space_after

    $json = $json->space_after([$enable])
    
    $enabled = $json->get_space_after

If C<$enable> is true (or missing), then the C<encode> method will add an extra
optional space after the C<:> separating keys from values in JSON objects
and extra whitespace after the C<,> separating key-value pairs and array
members.

If C<$enable> is false, then the C<encode> method will not add any extra
space at those places.

This setting has no effect when decoding JSON texts.

Example, space_before and indent disabled, space_after enabled:

   {"key": "value"}

=head2 relaxed

    $json = $json->relaxed([$enable])
    
    $enabled = $json->get_relaxed

If C<$enable> is true (or missing), then C<decode> will accept some
extensions to normal JSON syntax (see below). C<encode> will not be
affected in anyway. I<Be aware that this option makes you accept invalid
JSON texts as if they were valid!>. I suggest only to use this option to
parse application-specific files written by humans (configuration files,
resource files etc.)

If C<$enable> is false (the default), then C<decode> will only accept
valid JSON texts.

Currently accepted extensions are:

=over 4

=item * list items can have an end-comma

JSON I<separates> array elements and key-value pairs with commas. This
can be annoying if you write JSON texts manually and want to be able to
quickly append elements, so this extension accepts comma at the end of
such items not just between them:

   [
      1,
      2, <- this comma not normally allowed
   ]
   {
      "k1": "v1",
      "k2": "v2", <- this comma not normally allowed
   }

=item * shell-style '#'-comments

Whenever JSON allows whitespace, shell-style comments are additionally
allowed. They are terminated by the first carriage-return or line-feed
character, after which more white-space and comments are allowed.

  [
     1, # this comment not allowed in JSON
        # neither this one...
  ]

=item * C-style multiple-line '/* */'-comments (JSON::PP only)

Whenever JSON allows whitespace, C-style multiple-line comments are additionally
allowed. Everything between C</*> and C<*/> is a comment, after which
more white-space and comments are allowed.

  [
     1, /* this comment not allowed in JSON */
        /* neither this one... */
  ]

=item * C++-style one-line '//'-comments (JSON::PP only)

Whenever JSON allows whitespace, C++-style one-line comments are additionally
allowed. They are terminated by the first carriage-return or line-feed
character, after which more white-space and comments are allowed.

  [
     1, // this comment not allowed in JSON
        // neither this one...
  ]

=item * literal ASCII TAB characters in strings

Literal ASCII TAB characters are now allowed in strings (and treated as
C<\t>).

  [
     "Hello\tWorld",
     "Hello<TAB>World", # literal <TAB> would not normally be allowed
  ]

=back

=head2 canonical

    $json = $json->canonical([$enable])
    
    $enabled = $json->get_canonical

If C<$enable> is true (or missing), then the C<encode> method will output JSON objects
by sorting their keys. This is adding a comparatively high overhead.

If C<$enable> is false, then the C<encode> method will output key-value
pairs in the order Perl stores them (which will likely change between runs
of the same script, and can change even within the same run from 5.18
onwards).

This option is useful if you want the same data structure to be encoded as
the same JSON text (given the same overall settings). If it is disabled,
the same hash might be encoded differently even if contains the same data,
as key-value pairs have no inherent ordering in Perl.

This setting has no effect when decoding JSON texts.

This setting has currently no effect on tied hashes.

=head2 allow_nonref

    $json = $json->allow_nonref([$enable])
    
    $enabled = $json->get_allow_nonref

Unlike other boolean options, this opotion is enabled by default beginning
with version C<4.0>.

If C<$enable> is true (or missing), then the C<encode> method can convert a
non-reference into its corresponding string, number or null JSON value,
which is an extension to RFC4627. Likewise, C<decode> will accept those JSON
values instead of croaking.

If C<$enable> is false, then the C<encode> method will croak if it isn't
passed an arrayref or hashref, as JSON texts must either be an object
or array. Likewise, C<decode> will croak if given something that is not a
JSON object or array.

Example, encode a Perl scalar as JSON value without enabled C<allow_nonref>,
resulting in an error:

   JSON::PP->new->allow_nonref(0)->encode ("Hello, World!")
   => hash- or arrayref expected...

=head2 allow_unknown

    $json = $json->allow_unknown([$enable])
    
    $enabled = $json->get_allow_unknown

If C<$enable> is true (or missing), then C<encode> will I<not> throw an
exception when it encounters values it cannot represent in JSON (for
example, filehandles) but instead will encode a JSON C<null> value. Note
that blessed objects are not included here and are handled separately by
c<allow_blessed>.

If C<$enable> is false (the default), then C<encode> will throw an
exception when it encounters anything it cannot encode as JSON.

This option does not affect C<decode> in any way, and it is recommended to
leave it off unless you know your communications partner.

=head2 allow_blessed

    $json = $json->allow_blessed([$enable])
    
    $enabled = $json->get_allow_blessed

See L<OBJECT SERIALISATION> for details.

If C<$enable> is true (or missing), then the C<encode> method will not
barf when it encounters a blessed reference that it cannot convert
otherwise. Instead, a JSON C<null> value is encoded instead of the object.

If C<$enable> is false (the default), then C<encode> will throw an
exception when it encounters a blessed object that it cannot convert
otherwise.

This setting has no effect on C<decode>.

=head2 convert_blessed

    $json = $json->convert_blessed([$enable])
    
    $enabled = $json->get_convert_blessed

See L<OBJECT SERIALISATION> for details.

If C<$enable> is true (or missing), then C<encode>, upon encountering a
blessed object, will check for the availability of the C<TO_JSON> method
on the object's class. If found, it will be called in scalar context and
the resulting scalar will be encoded instead of the object.

The C<TO_JSON> method may safely call die if it wants. If C<TO_JSON>
returns other blessed objects, those will be handled in the same
way. C<TO_JSON> must take care of not causing an endless recursion cycle
(== crash) in this case. The name of C<TO_JSON> was chosen because other
methods called by the Perl core (== not by the user of the object) are
usually in upper case letters and to avoid collisions with any C<to_json>
function or method.

If C<$enable> is false (the default), then C<encode> will not consider
this type of conversion.

This setting has no effect on C<decode>.

=head2 allow_tags

    $json = $json->allow_tags([$enable])

    $enabled = $json->get_allow_tags

See L<OBJECT SERIALISATION> for details.

If C<$enable> is true (or missing), then C<encode>, upon encountering a
blessed object, will check for the availability of the C<FREEZE> method on
the object's class. If found, it will be used to serialise the object into
a nonstandard tagged JSON value (that JSON decoders cannot decode).

It also causes C<decode> to parse such tagged JSON values and deserialise
them via a call to the C<THAW> method.

If C<$enable> is false (the default), then C<encode> will not consider
this type of conversion, and tagged JSON values will cause a parse error
in C<decode>, as if tags were not part of the grammar.

=head2 boolean_values

    $json->boolean_values([$false, $true])

    ($false,  $true) = $json->get_boolean_values

By default, JSON booleans will be decoded as overloaded
C<$JSON::PP::false> and C<$JSON::PP::true> objects.

With this method you can specify your own boolean values for decoding -
on decode, JSON C<false> will be decoded as a copy of C<$false>, and JSON
C<true> will be decoded as C<$true> ("copy" here is the same thing as
assigning a value to another variable, i.e. C<$copy = $false>).

This is useful when you want to pass a decoded data structure directly
to other serialisers like YAML, Data::MessagePack and so on.

Note that this works only when you C<decode>. You can set incompatible
boolean objects (like L<boolean>), but when you C<encode> a data structure
with such boolean objects, you still need to enable C<convert_blessed>
(and add a C<TO_JSON> method if necessary).

Calling this method without any arguments will reset the booleans
to their default values.

C<get_boolean_values> will return both C<$false> and C<$true> values, or
the empty list when they are set to the default.

=head2 core_bools

    $json->core_bools([$enable]);

If C<$enable> is true (or missing), then C<decode>, will produce standard
perl boolean values. Equivalent to calling:

    $json->boolean_values(!!1, !!0)

C<get_core_bools> will return true if this has been set. On perl 5.36, it will
also return true if the boolean values have been set to perl's core booleans
using the C<boolean_values> method.

The methods C<unblessed_bool> and C<get_unblessed_bool> are provided as aliases
for compatibility with L<Cpanel::JSON::XS>.

=head2 filter_json_object

    $json = $json->filter_json_object([$coderef])

When C<$coderef> is specified, it will be called from C<decode> each
time it decodes a JSON object. The only argument is a reference to
the newly-created hash. If the code references returns a single scalar
(which need not be a reference), this value (or rather a copy of it) is
inserted into the deserialised data structure. If it returns an empty
list (NOTE: I<not> C<undef>, which is a valid scalar), the original
deserialised hash will be inserted. This setting can slow down decoding
considerably.

When C<$coderef> is omitted or undefined, any existing callback will
be removed and C<decode> will not change the deserialised hash in any
way.

Example, convert all JSON objects into the integer 5:

   my $js = JSON::PP->new->filter_json_object(sub { 5 });
   # returns [5]
   $js->decode('[{}]');
   # returns 5
   $js->decode('{"a":1, "b":2}');

=head2 filter_json_single_key_object

    $json = $json->filter_json_single_key_object($key [=> $coderef])

Works remotely similar to C<filter_json_object>, but is only called for
JSON objects having a single key named C<$key>.

This C<$coderef> is called before the one specified via
C<filter_json_object>, if any. It gets passed the single value in the JSON
object. If it returns a single value, it will be inserted into the data
structure. If it returns nothing (not even C<undef> but the empty list),
the callback from C<filter_json_object> will be called next, as if no
single-key callback were specified.

If C<$coderef> is omitted or undefined, the corresponding callback will be
disabled. There can only ever be one callback for a given key.

As this callback gets called less often then the C<filter_json_object>
one, decoding speed will not usually suffer as much. Therefore, single-key
objects make excellent targets to serialise Perl objects into, especially
as single-key JSON objects are as close to the type-tagged value concept
as JSON gets (it's basically an ID/VALUE tuple). Of course, JSON does not
support this in any way, so you need to make sure your data never looks
like a serialised Perl hash.

Typical names for the single object key are C<__class_whatever__>, or
C<$__dollars_are_rarely_used__$> or C<}ugly_brace_placement>, or even
things like C<__class_md5sum(classname)__>, to reduce the risk of clashing
with real hashes.

Example, decode JSON objects of the form C<< { "__widget__" => <id> } >>
into the corresponding C<< $WIDGET{<id>} >> object:

   # return whatever is in $WIDGET{5}:
   JSON::PP
      ->new
      ->filter_json_single_key_object (__widget__ => sub {
            $WIDGET{ $_[0] }
         })
      ->decode ('{"__widget__": 5')

   # this can be used with a TO_JSON method in some "widget" class
   # for serialisation to json:
   sub WidgetBase::TO_JSON {
      my ($self) = @_;

      unless ($self->{id}) {
         $self->{id} = ..get..some..id..;
         $WIDGET{$self->{id}} = $self;
      }

      { __widget__ => $self->{id} }
   }

=head2 shrink

    $json = $json->shrink([$enable])
    
    $enabled = $json->get_shrink

If C<$enable> is true (or missing), the string returned by C<encode> will
be shrunk (i.e. downgraded if possible).

The actual definition of what shrink does might change in future versions,
but it will always try to save space at the expense of time.

If C<$enable> is false, then JSON::PP does nothing.

=head2 max_depth

    $json = $json->max_depth([$maximum_nesting_depth])
    
    $max_depth = $json->get_max_depth

Sets the maximum nesting level (default C<512>) accepted while encoding
or decoding. If a higher nesting level is detected in JSON text or a Perl
data structure, then the encoder and decoder will stop and croak at that
point.

Nesting level is defined by number of hash- or arrayrefs that the encoder
needs to traverse to reach a given point or the number of C<{> or C<[>
characters without their matching closing parenthesis crossed to reach a
given character in a string.

Setting the maximum depth to one disallows any nesting, so that ensures
that the object is only a single hash/object or array.

If no argument is given, the highest possible setting will be used, which
is rarely useful.

See L<JSON::XS/SECURITY CONSIDERATIONS> for more info on why this is useful.

=head2 max_size

    $json = $json->max_size([$maximum_string_size])
    
    $max_size = $json->get_max_size

Set the maximum length a JSON text may have (in bytes) where decoding is
being attempted. The default is C<0>, meaning no limit. When C<decode>
is called on a string that is longer then this many bytes, it will not
attempt to decode the string but throw an exception. This setting has no
effect on C<encode> (yet).

If no argument is given, the limit check will be deactivated (same as when
C<0> is specified).

See L<JSON::XS/SECURITY CONSIDERATIONS> for more info on why this is useful.

=head2 encode

    $json_text = $json->encode($perl_scalar)

Converts the given Perl value or data structure to its JSON
representation. Croaks on error.

=head2 decode

    $perl_scalar = $json->decode($json_text)

The opposite of C<encode>: expects a JSON text and tries to parse it,
returning the resulting simple scalar or reference. Croaks on error.

=head2 decode_prefix

    ($perl_scalar, $characters) = $json->decode_prefix($json_text)

This works like the C<decode> method, but instead of raising an exception
when there is trailing garbage after the first JSON object, it will
silently stop parsing there and return the number of characters consumed
so far.

This is useful if your JSON texts are not delimited by an outer protocol
and you need to know where the JSON text ends.

   JSON::PP->new->decode_prefix ("[1] the tail")
   => ([1], 3)

=head1 FLAGS FOR JSON::PP ONLY

The following flags and properties are for JSON::PP only. If you use
any of these, you can't make your application run faster by replacing
JSON::PP with JSON::XS. If you need these and also speed boost,
you might want to try L<Cpanel::JSON::XS>, a fork of JSON::XS by
Reini Urban, which supports some of these (with a different set of
incompatibilities). Most of these historical flags are only kept
for backward compatibility, and should not be used in a new application.

=head2 allow_singlequote

    $json = $json->allow_singlequote([$enable])
    $enabled = $json->get_allow_singlequote

If C<$enable> is true (or missing), then C<decode> will accept
invalid JSON texts that contain strings that begin and end with
single quotation marks. C<encode> will not be affected in any way.
I<Be aware that this option makes you accept invalid JSON texts
as if they were valid!>. I suggest only to use this option to
parse application-specific files written by humans (configuration
files, resource files etc.)

If C<$enable> is false (the default), then C<decode> will only accept
valid JSON texts.

    $json->allow_singlequote->decode(qq|{"foo":'bar'}|);
    $json->allow_singlequote->decode(qq|{'foo':"bar"}|);
    $json->allow_singlequote->decode(qq|{'foo':'bar'}|);

=head2 allow_barekey

    $json = $json->allow_barekey([$enable])
    $enabled = $json->get_allow_barekey

If C<$enable> is true (or missing), then C<decode> will accept
invalid JSON texts that contain JSON objects whose names don't
begin and end with quotation marks. C<encode> will not be affected
in any way. I<Be aware that this option makes you accept invalid JSON
texts as if they were valid!>. I suggest only to use this option to
parse application-specific files written by humans (configuration
files, resource files etc.)

If C<$enable> is false (the default), then C<decode> will only accept
valid JSON texts.

    $json->allow_barekey->decode(qq|{foo:"bar"}|);

=head2 allow_bignum

    $json = $json->allow_bignum([$enable])
    $enabled = $json->get_allow_bignum

If C<$enable> is true (or missing), then C<decode> will convert
big integers Perl cannot handle as integer into L<Math::BigInt>
objects and convert floating numbers into L<Math::BigFloat>
objects. C<encode> will convert C<Math::BigInt> and C<Math::BigFloat>
objects into JSON numbers.

   $json->allow_nonref->allow_bignum;
   $bigfloat = $json->decode('2.000000000000000000000000001');
   print $json->encode($bigfloat);
   # => 2.000000000000000000000000001

See also L<MAPPING>.

=head2 loose

    $json = $json->loose([$enable])
    $enabled = $json->get_loose

If C<$enable> is true (or missing), then C<decode> will accept
invalid JSON texts that contain unescaped [\x00-\x1f\x22\x5c]
characters. C<encode> will not be affected in any way.
I<Be aware that this option makes you accept invalid JSON texts
as if they were valid!>. I suggest only to use this option to
parse application-specific files written by humans (configuration
files, resource files etc.)

If C<$enable> is false (the default), then C<decode> will only accept
valid JSON texts.

    $json->loose->decode(qq|["abc
                                   def"]|);

=head2 escape_slash

    $json = $json->escape_slash([$enable])
    $enabled = $json->get_escape_slash

If C<$enable> is true (or missing), then C<encode> will explicitly
escape I<slash> (solidus; C<U+002F>) characters to reduce the risk of
XSS (cross site scripting) that may be caused by C<< </script> >>
in a JSON text, with the cost of bloating the size of JSON texts.

This option may be useful when you embed JSON in HTML, but embedding
arbitrary JSON in HTML (by some HTML template toolkit or by string
interpolation) is risky in general. You must escape necessary
characters in correct order, depending on the context.

C<decode> will not be affected in any way.

=head2 indent_length

    $json = $json->indent_length($number_of_spaces)
    $length = $json->get_indent_length

This option is only useful when you also enable C<indent> or C<pretty>.

JSON::XS indents with three spaces when you C<encode> (if requested
by C<indent> or C<pretty>), and the number cannot be changed.
JSON::PP allows you to change/get the number of indent spaces with these
mutator/accessor. The default number of spaces is three (the same as
JSON::XS), and the acceptable range is from C<0> (no indentation;
it'd be better to disable indentation by C<indent(0)>) to C<15>.

=head2 sort_by

    $json = $json->sort_by($code_ref)
    $json = $json->sort_by($subroutine_name)

If you just want to sort keys (names) in JSON objects when you
C<encode>, enable C<canonical> option (see above) that allows you to
sort object keys alphabetically.

If you do need to sort non-alphabetically for whatever reasons,
you can give a code reference (or a subroutine name) to C<sort_by>,
then the argument will be passed to Perl's C<sort> built-in function.

As the sorting is done in the JSON::PP scope, you usually need to
prepend C<JSON::PP::> to the subroutine name, and the special variables
C<$a> and C<$b> used in the subrontine used by C<sort> function.

Example:

   my %ORDER = (id => 1, class => 2, name => 3);
   $json->sort_by(sub {
       ($ORDER{$JSON::PP::a} // 999) <=> ($ORDER{$JSON::PP::b} // 999)
       or $JSON::PP::a cmp $JSON::PP::b
   });
   print $json->encode([
       {name => 'CPAN', id => 1, href => 'http://cpan.org'}
   ]);
   # [{"id":1,"name":"CPAN","href":"http://cpan.org"}]

Note that C<sort_by> affects all the plain hashes in the data structure.
If you need finer control, C<tie> necessary hashes with a module that
implements ordered hash (such as L<Hash::Ordered> and L<Tie::IxHash>).
C<canonical> and C<sort_by> don't affect the key order in C<tie>d
hashes.

   use Hash::Ordered;
   tie my %hash, 'Hash::Ordered',
       (name => 'CPAN', id => 1, href => 'http://cpan.org');
   print $json->encode([\%hash]);
   # [{"name":"CPAN","id":1,"href":"http://cpan.org"}] # order is kept

=head1 INCREMENTAL PARSING

This section is also taken from JSON::XS.

In some cases, there is the need for incremental parsing of JSON
texts. While this module always has to keep both JSON text and resulting
Perl data structure in memory at one time, it does allow you to parse a
JSON stream incrementally. It does so by accumulating text until it has
a full JSON object, which it then can decode. This process is similar to
using C<decode_prefix> to see if a full JSON object is available, but
is much more efficient (and can be implemented with a minimum of method
calls).

JSON::PP will only attempt to parse the JSON text once it is sure it
has enough text to get a decisive result, using a very simple but
truly incremental parser. This means that it sometimes won't stop as
early as the full parser, for example, it doesn't detect mismatched
parentheses. The only thing it guarantees is that it starts decoding as
soon as a syntactically valid JSON text has been seen. This means you need
to set resource limits (e.g. C<max_size>) to ensure the parser will stop
parsing in the presence if syntax errors.

The following methods implement this incremental parser.

=head2 incr_parse

    $json->incr_parse( [$string] ) # void context
    
    $obj_or_undef = $json->incr_parse( [$string] ) # scalar context
    
    @obj_or_empty = $json->incr_parse( [$string] ) # list context

This is the central parsing function. It can both append new text and
extract objects from the stream accumulated so far (both of these
functions are optional).

If C<$string> is given, then this string is appended to the already
existing JSON fragment stored in the C<$json> object.

After that, if the function is called in void context, it will simply
return without doing anything further. This can be used to add more text
in as many chunks as you want.

If the method is called in scalar context, then it will try to extract
exactly I<one> JSON object. If that is successful, it will return this
object, otherwise it will return C<undef>. If there is a parse error,
this method will croak just as C<decode> would do (one can then use
C<incr_skip> to skip the erroneous part). This is the most common way of
using the method.

And finally, in list context, it will try to extract as many objects
from the stream as it can find and return them, or the empty list
otherwise. For this to work, there must be no separators (other than
whitespace) between the JSON objects or arrays, instead they must be
concatenated back-to-back. If an error occurs, an exception will be
raised as in the scalar context case. Note that in this case, any
previously-parsed JSON texts will be lost.

Example: Parse some JSON arrays/objects in a given string and return
them.

    my @objs = JSON::PP->new->incr_parse ("[5][7][1,2]");

=head2 incr_text

    $lvalue_string = $json->incr_text

This method returns the currently stored JSON fragment as an lvalue, that
is, you can manipulate it. This I<only> works when a preceding call to
C<incr_parse> in I<scalar context> successfully returned an object. Under
all other circumstances you must not call this function (I mean it.
although in simple tests it might actually work, it I<will> fail under
real world conditions). As a special exception, you can also call this
method before having parsed anything.

That means you can only use this function to look at or manipulate text
before or after complete JSON objects, not while the parser is in the
middle of parsing a JSON object.

This function is useful in two cases: a) finding the trailing text after a
JSON object or b) parsing multiple JSON objects separated by non-JSON text
(such as commas).

=head2 incr_skip

    $json->incr_skip

This will reset the state of the incremental parser and will remove
the parsed text from the input buffer so far. This is useful after
C<incr_parse> died, in which case the input buffer and incremental parser
state is left unchanged, to skip the text parsed so far and to reset the
parse state.

The difference to C<incr_reset> is that only text until the parse error
occurred is removed.

=head2 incr_reset

    $json->incr_reset

This completely resets the incremental parser, that is, after this call,
it will be as if the parser had never parsed anything.

This is useful if you want to repeatedly parse JSON objects and want to
ignore any trailing data, which means you have to reset the parser after
each successful decode.

=head1 MAPPING

Most of this section is also taken from JSON::XS.

This section describes how JSON::PP maps Perl values to JSON values and
vice versa. These mappings are designed to "do the right thing" in most
circumstances automatically, preserving round-tripping characteristics
(what you put in comes out as something equivalent).

For the more enlightened: note that in the following descriptions,
lowercase I<perl> refers to the Perl interpreter, while uppercase I<Perl>
refers to the abstract Perl language itself.

=head2 JSON -> PERL

=over 4

=item object

A JSON object becomes a reference to a hash in Perl. No ordering of object
keys is preserved (JSON does not preserve object key ordering itself).

=item array

A JSON array becomes a reference to an array in Perl.

=item string

A JSON string becomes a string scalar in Perl - Unicode codepoints in JSON
are represented by the same codepoints in the Perl string, so no manual
decoding is necessary.

=item number

A JSON number becomes either an integer, numeric (floating point) or
string scalar in perl, depending on its range and any fractional parts. On
the Perl level, there is no difference between those as Perl handles all
the conversion details, but an integer may take slightly less memory and
might represent more values exactly than floating point numbers.

If the number consists of digits only, JSON::PP will try to represent
it as an integer value. If that fails, it will try to represent it as
a numeric (floating point) value if that is possible without loss of
precision. Otherwise it will preserve the number as a string value (in
which case you lose roundtripping ability, as the JSON number will be
re-encoded to a JSON string).

Numbers containing a fractional or exponential part will always be
represented as numeric (floating point) values, possibly at a loss of
precision (in which case you might lose perfect roundtripping ability, but
the JSON number will still be re-encoded as a JSON number).

Note that precision is not accuracy - binary floating point values cannot
represent most decimal fractions exactly, and when converting from and to
floating point, JSON::PP only guarantees precision up to but not including
the least significant bit.

When C<allow_bignum> is enabled, big integer values and any numeric
values will be converted into L<Math::BigInt> and L<Math::BigFloat>
objects respectively, without becoming string scalars or losing
precision.

=item true, false

These JSON atoms become C<JSON::PP::true> and C<JSON::PP::false>,
respectively. They are overloaded to act almost exactly like the numbers
C<1> and C<0>. You can check whether a scalar is a JSON boolean by using
the C<JSON::PP::is_bool> function.

=item null

A JSON null atom becomes C<undef> in Perl.

=item shell-style comments (C<< # I<text> >>)

As a nonstandard extension to the JSON syntax that is enabled by the
C<relaxed> setting, shell-style comments are allowed. They can start
anywhere outside strings and go till the end of the line.

=item tagged values (C<< (I<tag>)I<value> >>).

Another nonstandard extension to the JSON syntax, enabled with the
C<allow_tags> setting, are tagged values. In this implementation, the
I<tag> must be a perl package/class name encoded as a JSON string, and the
I<value> must be a JSON array encoding optional constructor arguments.

See L<OBJECT SERIALISATION>, below, for details.

=back


=head2 PERL -> JSON

The mapping from Perl to JSON is slightly more difficult, as Perl is a
truly typeless language, so we can only guess which JSON type is meant by
a Perl value.

=over 4

=item hash references

Perl hash references become JSON objects. As there is no inherent
ordering in hash keys (or JSON objects), they will usually be encoded
in a pseudo-random order. JSON::PP can optionally sort the hash keys
(determined by the I<canonical> flag and/or I<sort_by> property), so
the same data structure will serialise to the same JSON text (given
same settings and version of JSON::PP), but this incurs a runtime
overhead and is only rarely useful, e.g. when you want to compare some
JSON text against another for equality.

=item array references

Perl array references become JSON arrays.

=item other references

Other unblessed references are generally not allowed and will cause an
exception to be thrown, except for references to the integers C<0> and
C<1>, which get turned into C<false> and C<true> atoms in JSON. You can
also use C<JSON::PP::false> and C<JSON::PP::true> to improve
readability.

   to_json [\0, JSON::PP::true]      # yields [false,true]

=item JSON::PP::true, JSON::PP::false

These special values become JSON true and JSON false values,
respectively. You can also use C<\1> and C<\0> directly if you want.

=item JSON::PP::null

This special value becomes JSON null.

=item blessed objects

Blessed objects are not directly representable in JSON, but C<JSON::PP>
allows various ways of handling objects. See L<OBJECT SERIALISATION>,
below, for details.

=item simple scalars

Simple Perl scalars (any scalar that is not a reference) are the most
difficult objects to encode: JSON::PP will encode undefined scalars as
JSON C<null> values, scalars that have last been used in a string context
before encoding as JSON strings, and anything else as number value:

   # dump as number
   encode_json [2]                      # yields [2]
   encode_json [-3.0e17]                # yields [-3e+17]
   my $value = 5; encode_json [$value]  # yields [5]

   # used as string, so dump as string
   print $value;
   encode_json [$value]                 # yields ["5"]

   # undef becomes null
   encode_json [undef]                  # yields [null]

You can force the type to be a JSON string by stringifying it:

   my $x = 3.1; # some variable containing a number
   "$x";        # stringified
   $x .= "";    # another, more awkward way to stringify
   print $x;    # perl does it for you, too, quite often
                # (but for older perls)

You can force the type to be a JSON number by numifying it:

   my $x = "3"; # some variable containing a string
   $x += 0;     # numify it, ensuring it will be dumped as a number
   $x *= 1;     # same thing, the choice is yours.

You can not currently force the type in other, less obscure, ways.

Since version 2.91_01, JSON::PP uses a different number detection logic
that converts a scalar that is possible to turn into a number safely.
The new logic is slightly faster, and tends to help people who use older
perl or who want to encode complicated data structure. However, this may
results in a different JSON text from the one JSON::XS encodes (and
thus may break tests that compare entire JSON texts). If you do
need the previous behavior for compatibility or for finer control,
set PERL_JSON_PP_USE_B environmental variable to true before you
C<use> JSON::PP (or JSON.pm).

Note that numerical precision has the same meaning as under Perl (so
binary to decimal conversion follows the same rules as in Perl, which
can differ to other languages). Also, your perl interpreter might expose
extensions to the floating point numbers of your platform, such as
infinities or NaN's - these cannot be represented in JSON, and it is an
error to pass those in.

JSON::PP (and JSON::XS) trusts what you pass to C<encode> method
(or C<encode_json> function) is a clean, validated data structure with
values that can be represented as valid JSON values only, because it's
not from an external data source (as opposed to JSON texts you pass to
C<decode> or C<decode_json>, which JSON::PP considers tainted and
doesn't trust). As JSON::PP doesn't know exactly what you and consumers
of your JSON texts want the unexpected values to be (you may want to
convert them into null, or to stringify them with or without
normalisation (string representation of infinities/NaN may vary
depending on platforms), or to croak without conversion), you're advised
to do what you and your consumers need before you encode, and also not
to numify values that may start with values that look like a number
(including infinities/NaN), without validating.

=back

=head2 OBJECT SERIALISATION

As JSON cannot directly represent Perl objects, you have to choose between
a pure JSON representation (without the ability to deserialise the object
automatically again), and a nonstandard extension to the JSON syntax,
tagged values.

=head3 SERIALISATION

What happens when C<JSON::PP> encounters a Perl object depends on the
C<allow_blessed>, C<convert_blessed>, C<allow_tags> and C<allow_bignum>
settings, which are used in this order:

=over 4

=item 1. C<allow_tags> is enabled and the object has a C<FREEZE> method.

In this case, C<JSON::PP> creates a tagged JSON value, using a nonstandard
extension to the JSON syntax.

This works by invoking the C<FREEZE> method on the object, with the first
argument being the object to serialise, and the second argument being the
constant string C<JSON> to distinguish it from other serialisers.

The C<FREEZE> method can return any number of values (i.e. zero or
more). These values and the paclkage/classname of the object will then be
encoded as a tagged JSON value in the following format:

   ("classname")[FREEZE return values...]

e.g.:

   ("URI")["http://www.google.com/"]
   ("MyDate")[2013,10,29]
   ("ImageData::JPEG")["Z3...VlCg=="]

For example, the hypothetical C<My::Object> C<FREEZE> method might use the
objects C<type> and C<id> members to encode the object:

   sub My::Object::FREEZE {
      my ($self, $serialiser) = @_;

      ($self->{type}, $self->{id})
   }

=item 2. C<convert_blessed> is enabled and the object has a C<TO_JSON> method.

In this case, the C<TO_JSON> method of the object is invoked in scalar
context. It must return a single scalar that can be directly encoded into
JSON. This scalar replaces the object in the JSON text.

For example, the following C<TO_JSON> method will convert all L<URI>
objects to JSON strings when serialised. The fact that these values
originally were L<URI> objects is lost.

   sub URI::TO_JSON {
      my ($uri) = @_;
      $uri->as_string
   }

=item 3. C<allow_bignum> is enabled and the object is a C<Math::BigInt> or C<Math::BigFloat>.

The object will be serialised as a JSON number value.

=item 4. C<allow_blessed> is enabled.

The object will be serialised as a JSON null value.

=item 5. none of the above

If none of the settings are enabled or the respective methods are missing,
C<JSON::PP> throws an exception.

=back

=head3 DESERIALISATION

For deserialisation there are only two cases to consider: either
nonstandard tagging was used, in which case C<allow_tags> decides,
or objects cannot be automatically be deserialised, in which
case you can use postprocessing or the C<filter_json_object> or
C<filter_json_single_key_object> callbacks to get some real objects our of
your JSON.

This section only considers the tagged value case: a tagged JSON object
is encountered during decoding and C<allow_tags> is disabled, a parse
error will result (as if tagged values were not part of the grammar).

If C<allow_tags> is enabled, C<JSON::PP> will look up the C<THAW> method
of the package/classname used during serialisation (it will not attempt
to load the package as a Perl module). If there is no such method, the
decoding will fail with an error.

Otherwise, the C<THAW> method is invoked with the classname as first
argument, the constant string C<JSON> as second argument, and all the
values from the JSON array (the values originally returned by the
C<FREEZE> method) as remaining arguments.

The method must then return the object. While technically you can return
any Perl scalar, you might have to enable the C<allow_nonref> setting to
make that work in all cases, so better return an actual blessed reference.

As an example, let's implement a C<THAW> function that regenerates the
C<My::Object> from the C<FREEZE> example earlier:

   sub My::Object::THAW {
      my ($class, $serialiser, $type, $id) = @_;

      $class->new (type => $type, id => $id)
   }


=head1 ENCODING/CODESET FLAG NOTES

This section is taken from JSON::XS.

The interested reader might have seen a number of flags that signify
encodings or codesets - C<utf8>, C<latin1> and C<ascii>. There seems to be
some confusion on what these do, so here is a short comparison:

C<utf8> controls whether the JSON text created by C<encode> (and expected
by C<decode>) is UTF-8 encoded or not, while C<latin1> and C<ascii> only
control whether C<encode> escapes character values outside their respective
codeset range. Neither of these flags conflict with each other, although
some combinations make less sense than others.

Care has been taken to make all flags symmetrical with respect to
C<encode> and C<decode>, that is, texts encoded with any combination of
these flag values will be correctly decoded when the same flags are used
- in general, if you use different flag settings while encoding vs. when
decoding you likely have a bug somewhere.

Below comes a verbose discussion of these flags. Note that a "codeset" is
simply an abstract set of character-codepoint pairs, while an encoding
takes those codepoint numbers and I<encodes> them, in our case into
octets. Unicode is (among other things) a codeset, UTF-8 is an encoding,
and ISO-8859-1 (= latin 1) and ASCII are both codesets I<and> encodings at
the same time, which can be confusing.

=over 4

=item C<utf8> flag disabled

When C<utf8> is disabled (the default), then C<encode>/C<decode> generate
and expect Unicode strings, that is, characters with high ordinal Unicode
values (> 255) will be encoded as such characters, and likewise such
characters are decoded as-is, no changes to them will be done, except
"(re-)interpreting" them as Unicode codepoints or Unicode characters,
respectively (to Perl, these are the same thing in strings unless you do
funny/weird/dumb stuff).

This is useful when you want to do the encoding yourself (e.g. when you
want to have UTF-16 encoded JSON texts) or when some other layer does
the encoding for you (for example, when printing to a terminal using a
filehandle that transparently encodes to UTF-8 you certainly do NOT want
to UTF-8 encode your data first and have Perl encode it another time).

=item C<utf8> flag enabled

If the C<utf8>-flag is enabled, C<encode>/C<decode> will encode all
characters using the corresponding UTF-8 multi-byte sequence, and will
expect your input strings to be encoded as UTF-8, that is, no "character"
of the input string must have any value > 255, as UTF-8 does not allow
that.

The C<utf8> flag therefore switches between two modes: disabled means you
will get a Unicode string in Perl, enabled means you get an UTF-8 encoded
octet/binary string in Perl.

=item C<latin1> or C<ascii> flags enabled

With C<latin1> (or C<ascii>) enabled, C<encode> will escape characters
with ordinal values > 255 (> 127 with C<ascii>) and encode the remaining
characters as specified by the C<utf8> flag.

If C<utf8> is disabled, then the result is also correctly encoded in those
character sets (as both are proper subsets of Unicode, meaning that a
Unicode string with all character values < 256 is the same thing as a
ISO-8859-1 string, and a Unicode string with all character values < 128 is
the same thing as an ASCII string in Perl).

If C<utf8> is enabled, you still get a correct UTF-8-encoded string,
regardless of these flags, just some more characters will be escaped using
C<\uXXXX> then before.

Note that ISO-8859-1-I<encoded> strings are not compatible with UTF-8
encoding, while ASCII-encoded strings are. That is because the ISO-8859-1
encoding is NOT a subset of UTF-8 (despite the ISO-8859-1 I<codeset> being
a subset of Unicode), while ASCII is.

Surprisingly, C<decode> will ignore these flags and so treat all input
values as governed by the C<utf8> flag. If it is disabled, this allows you
to decode ISO-8859-1- and ASCII-encoded strings, as both strict subsets of
Unicode. If it is enabled, you can correctly decode UTF-8 encoded strings.

So neither C<latin1> nor C<ascii> are incompatible with the C<utf8> flag -
they only govern when the JSON output engine escapes a character or not.

The main use for C<latin1> is to relatively efficiently store binary data
as JSON, at the expense of breaking compatibility with most JSON decoders.

The main use for C<ascii> is to force the output to not contain characters
with values > 127, which means you can interpret the resulting string
as UTF-8, ISO-8859-1, ASCII, KOI8-R or most about any character set and
8-bit-encoding, and still get the same data structure back. This is useful
when your channel for JSON transfer is not 8-bit clean or the encoding
might be mangled in between (e.g. in mail), and works because ASCII is a
proper subset of most 8-bit and multibyte encodings in use in the world.

=back

=head1 BUGS

Please report bugs on a specific behavior of this module to RT or GitHub
issues (preferred):

L<https://github.com/makamaka/JSON-PP/issues>

L<https://rt.cpan.org/Public/Dist/Display.html?Queue=JSON-PP>

As for new features and requests to change common behaviors, please
ask the author of JSON::XS (Marc Lehmann, E<lt>schmorp[at]schmorp.deE<gt>)
first, by email (important!), to keep compatibility among JSON.pm backends.

Generally speaking, if you need something special for you, you are advised
to create a new module, maybe based on L<JSON::Tiny>, which is smaller and
written in a much cleaner way than this module.

=head1 SEE ALSO

The F<json_pp> command line utility for quick experiments.

L<JSON::XS>, L<Cpanel::JSON::XS>, and L<JSON::Tiny> for faster alternatives.
L<JSON> and L<JSON::MaybeXS> for easy migration.

L<JSON::PP::Compat5005> and L<JSON::PP::Compat5006> for older perl users.

RFC4627 (L<http://www.ietf.org/rfc/rfc4627.txt>)

RFC7159 (L<http://www.ietf.org/rfc/rfc7159.txt>)

RFC8259 (L<http://www.ietf.org/rfc/rfc8259.txt>)

=head1 AUTHOR

Makamaka Hannyaharamitu, E<lt>makamaka[at]cpan.orgE<gt>

=head1 CURRENT MAINTAINER

Kenichi Ishigaki, E<lt>ishigaki[at]cpan.orgE<gt>

=head1 COPYRIGHT AND LICENSE

Copyright 2007-2016 by Makamaka Hannyaharamitu

Most of the documentation is taken from JSON::XS by Marc Lehmann

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut
