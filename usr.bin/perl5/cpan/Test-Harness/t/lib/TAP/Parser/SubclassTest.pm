# subclass for testing subclassing

package TAP::Parser::SubclassTest;

use strict;
use warnings;

use MySourceHandler;
use MyPerlSourceHandler;
use MyGrammar;
use MyResultFactory;

use base qw( TAP::Parser MyCustom );

sub _default_source_class         {'MySourceHandler'}        # deprecated
sub _default_perl_source_class    {'MyPerlSourceHandler'}    # deprecated
sub _default_grammar_class        {'MyGrammar'}
sub _default_result_factory_class {'MyResultFactory'}

sub make_source { shift->SUPER::make_source(@_)->custom }    # deprecated

sub make_perl_source {
    shift->SUPER::make_perl_source(@_)->custom;
}                                                            # deprecated
sub make_grammar  { shift->SUPER::make_grammar(@_)->custom }
sub make_iterator { shift->SUPER::make_iterator(@_)->custom }    # deprecated
sub make_result   { shift->SUPER::make_result(@_)->custom }

sub _initialize {
    my $self = shift;
    $self->SUPER::_initialize(@_);
    $main::INIT{ ref($self) }++;
    $self->{initialized} = 1;
    return $self;
}

1;
