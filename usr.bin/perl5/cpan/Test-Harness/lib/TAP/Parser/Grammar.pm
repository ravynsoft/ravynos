package TAP::Parser::Grammar;

use strict;
use warnings;

use TAP::Parser::ResultFactory   ();
use TAP::Parser::YAMLish::Reader ();

use base 'TAP::Object';

=head1 NAME

TAP::Parser::Grammar - A grammar for the Test Anything Protocol.

=head1 VERSION

Version 3.44

=cut

our $VERSION = '3.44';

=head1 SYNOPSIS

  use TAP::Parser::Grammar;
  my $grammar = $self->make_grammar({
    iterator => $tap_parser_iterator,
    parser   => $tap_parser,
    version  => 12,
  });

  my $result = $grammar->tokenize;

=head1 DESCRIPTION

C<TAP::Parser::Grammar> tokenizes lines from a L<TAP::Parser::Iterator> and
constructs L<TAP::Parser::Result> subclasses to represent the tokens.

Do not attempt to use this class directly.  It won't make sense.  It's mainly
here to ensure that we will be able to have pluggable grammars when TAP is
expanded at some future date (plus, this stuff was really cluttering the
parser).

=head1 METHODS

=head2 Class Methods

=head3 C<new>

  my $grammar = TAP::Parser::Grammar->new({
      iterator => $iterator,
      parser   => $parser,
      version  => $version,
  });

Returns L<TAP::Parser> grammar object that will parse the TAP stream from the
specified iterator.  Both C<iterator> and C<parser> are required arguments.
If C<version> is not set it defaults to C<12> (see L</set_version> for more
details).

=cut

# new() implementation supplied by TAP::Object
sub _initialize {
    my ( $self, $args ) = @_;
    $self->{iterator} = $args->{iterator};    # TODO: accessor
    $self->{iterator} ||= $args->{stream};    # deprecated
    $self->{parser} = $args->{parser};        # TODO: accessor
    $self->set_version( $args->{version} || 12 );
    return $self;
}

my %language_for;

{

    # XXX the 'not' and 'ok' might be on separate lines in VMS ...
    my $ok  = qr/(?:not )?ok\b/;
    my $num = qr/\d+/;

    my %v12 = (
        version => {
            syntax  => qr/^TAP\s+version\s+(\d+)\s*\z/i,
            handler => sub {
                my ( $self, $line ) = @_;
                my $version = $1;
                return $self->_make_version_token( $line, $version, );
            },
        },
        plan => {
            syntax  => qr/^1\.\.(\d+)\s*(.*)\z/,
            handler => sub {
                my ( $self, $line ) = @_;
                my ( $tests_planned, $tail ) = ( $1, $2 );
                my $explanation = undef;
                my $skip        = '';

                if ( $tail =~ /^todo((?:\s+\d+)+)/ ) {
                    my @todo = split /\s+/, _trim($1);
                    return $self->_make_plan_token(
                        $line, $tests_planned, 'TODO',
                        '',    \@todo
                    );
                }
                elsif ( 0 == $tests_planned ) {
                    $skip = 'SKIP';

                    # If we can't match # SKIP the directive should be undef.
                    ($explanation) = $tail =~ /^#\s*SKIP\S*\s+(.*)/i;
                }
                elsif ( $tail !~ /^\s*$/ ) {
                    return $self->_make_unknown_token($line);
                }

                $explanation = '' unless defined $explanation;

                return $self->_make_plan_token(
                    $line, $tests_planned, $skip,
                    $explanation, []
                );

            },
        },

        # An optimization to handle the most common test lines without
        # directives.
        simple_test => {
            syntax  => qr/^($ok) \ ($num) (?:\ ([^#]+))? \z/x,
            handler => sub {
                my ( $self, $line ) = @_;
                my ( $ok, $num, $desc ) = ( $1, $2, $3 );

                return $self->_make_test_token(
                    $line, $ok, $num,
                    $desc
                );
            },
        },
        test => {
            syntax  => qr/^($ok) \s* ($num)? \s* (.*) \z/x,
            handler => sub {
                my ( $self, $line ) = @_;
                my ( $ok, $num, $desc ) = ( $1, $2, $3 );
                my ( $dir, $explanation ) = ( '', '' );
                if ($desc =~ m/^ ( [^\\\#]* (?: \\. [^\\\#]* )* )
                       \# \s* (SKIP|TODO) \b \s* (.*) $/ix
                  )
                {
                    ( $desc, $dir, $explanation ) = ( $1, $2, $3 );
                }
                return $self->_make_test_token(
                    $line, $ok, $num, $desc,
                    $dir,  $explanation
                );
            },
        },
        comment => {
            syntax  => qr/^#(.*)/,
            handler => sub {
                my ( $self, $line ) = @_;
                my $comment = $1;
                return $self->_make_comment_token( $line, $comment );
            },
        },
        bailout => {
            syntax  => qr/^\s*Bail out!\s*(.*)/,
            handler => sub {
                my ( $self, $line ) = @_;
                my $explanation = $1;
                return $self->_make_bailout_token(
                    $line,
                    $explanation
                );
            },
        },
    );

    my %v13 = (
        %v12,
        plan => {
            syntax  => qr/^1\.\.(\d+)\s*(?:\s*#\s*SKIP\b(.*))?\z/i,
            handler => sub {
                my ( $self, $line ) = @_;
                my ( $tests_planned, $explanation ) = ( $1, $2 );
                my $skip
                  = ( 0 == $tests_planned || defined $explanation )
                  ? 'SKIP'
                  : '';
                $explanation = '' unless defined $explanation;
                return $self->_make_plan_token(
                    $line, $tests_planned, $skip,
                    $explanation, []
                );
            },
        },
        yaml => {
            syntax  => qr/^ (\s+) (---.*) $/x,
            handler => sub {
                my ( $self, $line ) = @_;
                my ( $pad, $marker ) = ( $1, $2 );
                return $self->_make_yaml_token( $pad, $marker );
            },
        },
        pragma => {
            syntax =>
              qr/^ pragma \s+ ( [-+] \w+ \s* (?: , \s* [-+] \w+ \s* )* ) $/x,
            handler => sub {
                my ( $self, $line ) = @_;
                my $pragmas = $1;
                return $self->_make_pragma_token( $line, $pragmas );
            },
        },
    );

    %language_for = (
        '12' => {
            tokens => \%v12,
        },
        '13' => {
            tokens => \%v13,
            setup  => sub {
                shift->{iterator}->handle_unicode;
            },
        },
    );
}

##############################################################################

=head2 Instance Methods

=head3 C<set_version>

  $grammar->set_version(13);

Tell the grammar which TAP syntax version to support. The lowest
supported version is 12. Although 'TAP version' isn't valid version 12
syntax it is accepted so that higher version numbers may be parsed.

=cut

sub set_version {
    my $self    = shift;
    my $version = shift;

    if ( my $language = $language_for{$version} ) {
        $self->{version} = $version;
        $self->{tokens}  = $language->{tokens};

        if ( my $setup = $language->{setup} ) {
            $self->$setup();
        }

        $self->_order_tokens;
    }
    else {
        require Carp;
        Carp::croak("Unsupported syntax version: $version");
    }
}

# Optimization to put the most frequent tokens first.
sub _order_tokens {
    my $self = shift;

    my %copy = %{ $self->{tokens} };
    my @ordered_tokens = grep {defined}
      map { delete $copy{$_} } qw( simple_test test comment plan );
    push @ordered_tokens, values %copy;

    $self->{ordered_tokens} = \@ordered_tokens;
}

##############################################################################

=head3 C<tokenize>

  my $token = $grammar->tokenize;

This method will return a L<TAP::Parser::Result> object representing the
current line of TAP.

=cut

sub tokenize {
    my $self = shift;

    my $line = $self->{iterator}->next;
    unless ( defined $line ) {
        delete $self->{parser};    # break circular ref
        return;
    }

    my $token;

    for my $token_data ( @{ $self->{ordered_tokens} } ) {
        if ( $line =~ $token_data->{syntax} ) {
            my $handler = $token_data->{handler};
            $token = $self->$handler($line);
            last;
        }
    }

    $token = $self->_make_unknown_token($line) unless $token;

    return $self->{parser}->make_result($token);
}

##############################################################################

=head3 C<token_types>

  my @types = $grammar->token_types;

Returns the different types of tokens which this grammar can parse.

=cut

sub token_types {
    my $self = shift;
    return keys %{ $self->{tokens} };
}

##############################################################################

=head3 C<syntax_for>

  my $syntax = $grammar->syntax_for($token_type);

Returns a pre-compiled regular expression which will match a chunk of TAP
corresponding to the token type.  For example (not that you should really pay
attention to this, C<< $grammar->syntax_for('comment') >> will return
C<< qr/^#(.*)/ >>.

=cut

sub syntax_for {
    my ( $self, $type ) = @_;
    return $self->{tokens}->{$type}->{syntax};
}

##############################################################################

=head3 C<handler_for>

  my $handler = $grammar->handler_for($token_type);

Returns a code reference which, when passed an appropriate line of TAP,
returns the lexed token corresponding to that line.  As a result, the basic
TAP parsing loop looks similar to the following:

 my @tokens;
 my $grammar = TAP::Grammar->new;
 LINE: while ( defined( my $line = $parser->_next_chunk_of_tap ) ) {
     for my $type ( $grammar->token_types ) {
         my $syntax  = $grammar->syntax_for($type);
         if ( $line =~ $syntax ) {
             my $handler = $grammar->handler_for($type);
             push @tokens => $grammar->$handler($line);
             next LINE;
         }
     }
     push @tokens => $grammar->_make_unknown_token($line);
 }

=cut

sub handler_for {
    my ( $self, $type ) = @_;
    return $self->{tokens}->{$type}->{handler};
}

sub _make_version_token {
    my ( $self, $line, $version ) = @_;
    return {
        type    => 'version',
        raw     => $line,
        version => $version,
    };
}

sub _make_plan_token {
    my ( $self, $line, $tests_planned, $directive, $explanation, $todo ) = @_;

    if (   $directive eq 'SKIP'
        && 0 != $tests_planned
        && $self->{version} < 13 )
    {
        warn
          "Specified SKIP directive in plan but more than 0 tests ($line)\n";
    }

    return {
        type          => 'plan',
        raw           => $line,
        tests_planned => $tests_planned,
        directive     => $directive,
        explanation   => _trim($explanation),
        todo_list     => $todo,
    };
}

sub _make_test_token {
    my ( $self, $line, $ok, $num, $desc, $dir, $explanation ) = @_;
    return {
        ok          => $ok,

        # forcing this to be an integer (and not a string) reduces memory
        # consumption. RT #84939
        test_num    => ( defined $num ? 0 + $num : undef ),
        description => _trim($desc),
        directive   => ( defined $dir ? uc $dir : '' ),
        explanation => _trim($explanation),
        raw         => $line,
        type        => 'test',
    };
}

sub _make_unknown_token {
    my ( $self, $line ) = @_;
    return {
        raw  => $line,
        type => 'unknown',
    };
}

sub _make_comment_token {
    my ( $self, $line, $comment ) = @_;
    return {
        type    => 'comment',
        raw     => $line,
        comment => _trim($comment)
    };
}

sub _make_bailout_token {
    my ( $self, $line, $explanation ) = @_;
    return {
        type    => 'bailout',
        raw     => $line,
        bailout => _trim($explanation)
    };
}

sub _make_yaml_token {
    my ( $self, $pad, $marker ) = @_;

    my $yaml = TAP::Parser::YAMLish::Reader->new;

    my $iterator = $self->{iterator};

    # Construct a reader that reads from our input stripping leading
    # spaces from each line.
    my $leader = length($pad);
    my $strip  = qr{ ^ (\s{$leader}) (.*) $ }x;
    my @extra  = ($marker);
    my $reader = sub {
        return shift @extra if @extra;
        my $line = $iterator->next;
        return $2 if $line =~ $strip;
        return;
    };

    my $data = $yaml->read($reader);

    # Reconstitute input. This is convoluted. Maybe we should just
    # record it on the way in...
    chomp( my $raw = $yaml->get_raw );
    $raw =~ s/^/$pad/mg;

    return {
        type => 'yaml',
        raw  => $raw,
        data => $data
    };
}

sub _make_pragma_token {
    my ( $self, $line, $pragmas ) = @_;
    return {
        type    => 'pragma',
        raw     => $line,
        pragmas => [ split /\s*,\s*/, _trim($pragmas) ],
    };
}

sub _trim {
    my $data = shift;

    return '' unless defined $data;

    $data =~ s/^\s+//;
    $data =~ s/\s+$//;
    return $data;
}

1;

=head1 TAP GRAMMAR

B<NOTE:>  This grammar is slightly out of date.  There's still some discussion
about it and a new one will be provided when we have things better defined.

The L<TAP::Parser> does not use a formal grammar because TAP is essentially a
stream-based protocol.  In fact, it's quite legal to have an infinite stream.
For the same reason that we don't apply regexes to streams, we're not using a
formal grammar here.  Instead, we parse the TAP in lines.

For purposes for forward compatibility, any result which does not match the
following grammar is currently referred to as
L<TAP::Parser::Result::Unknown>.  It is I<not> a parse error.

A formal grammar would look similar to the following:

 (*
     For the time being, I'm cheating on the EBNF by allowing
     certain terms to be defined by POSIX character classes by
     using the following syntax:

       digit ::= [:digit:]

     As far as I am aware, that's not valid EBNF.  Sue me.  I
     didn't know how to write "char" otherwise (Unicode issues).
     Suggestions welcome.
 *)

 tap            ::= version? { comment | unknown } leading_plan lines
                    |
                    lines trailing_plan {comment}

 version        ::= 'TAP version ' positiveInteger {positiveInteger} "\n"

 leading_plan   ::= plan skip_directive? "\n"

 trailing_plan  ::= plan "\n"

 plan           ::= '1..' nonNegativeInteger

 lines          ::= line {line}

 line           ::= (comment | test | unknown | bailout ) "\n"

 test           ::= status positiveInteger? description? directive?

 status         ::= 'not '? 'ok '

 description    ::= (character - (digit | '#')) {character - '#'}

 directive      ::= todo_directive | skip_directive

 todo_directive ::= hash_mark 'TODO' ' ' {character}

 skip_directive ::= hash_mark 'SKIP' ' ' {character}

 comment        ::= hash_mark {character}

 hash_mark      ::= '#' {' '}

 bailout        ::= 'Bail out!' {character}

 unknown        ::= { (character - "\n") }

 (* POSIX character classes and other terminals *)

 digit              ::= [:digit:]
 character          ::= ([:print:] - "\n")
 positiveInteger    ::= ( digit - '0' ) {digit}
 nonNegativeInteger ::= digit {digit}

=head1 SUBCLASSING

Please see L<TAP::Parser/SUBCLASSING> for a subclassing overview.

If you I<really> want to subclass L<TAP::Parser>'s grammar the best thing to
do is read through the code.  There's no easy way of summarizing it here.

=head1 SEE ALSO

L<TAP::Object>,
L<TAP::Parser>,
L<TAP::Parser::Iterator>,
L<TAP::Parser::Result>,

=cut
