#!./perl

use strict;
use warnings;

require q(./test.pl); plan(tests => 1);

=pod

This example is taken from the inheritance graph of DBIx::Class::Core in DBIx::Class v0.07002:
(No ASCII art this time, this graph is insane)

The xx:: prefixes are just to be sure these bogus declarations never stomp on real ones

=cut

{
    package xx::DBIx::Class::Core; use mro 'dfs';
    our @ISA = qw/
      xx::DBIx::Class::Serialize::Storable
      xx::DBIx::Class::InflateColumn
      xx::DBIx::Class::Relationship
      xx::DBIx::Class::PK::Auto
      xx::DBIx::Class::PK
      xx::DBIx::Class::Row
      xx::DBIx::Class::ResultSourceProxy::Table
      xx::DBIx::Class::AccessorGroup
    /;

    package xx::DBIx::Class::InflateColumn; use mro 'dfs';
    our @ISA = qw/ xx::DBIx::Class::Row /;

    package xx::DBIx::Class::Row; use mro 'dfs';
    our @ISA = qw/ xx::DBIx::Class /;

    package xx::DBIx::Class; use mro 'dfs';
    our @ISA = qw/
      xx::DBIx::Class::Componentised
      xx::Class::Data::Accessor
    /;

    package xx::DBIx::Class::Relationship; use mro 'dfs';
    our @ISA = qw/
      xx::DBIx::Class::Relationship::Helpers
      xx::DBIx::Class::Relationship::Accessor
      xx::DBIx::Class::Relationship::CascadeActions
      xx::DBIx::Class::Relationship::ProxyMethods
      xx::DBIx::Class::Relationship::Base
      xx::DBIx::Class
    /;

    package xx::DBIx::Class::Relationship::Helpers; use mro 'dfs';
    our @ISA = qw/
      xx::DBIx::Class::Relationship::HasMany
      xx::DBIx::Class::Relationship::HasOne
      xx::DBIx::Class::Relationship::BelongsTo
      xx::DBIx::Class::Relationship::ManyToMany
    /;

    package xx::DBIx::Class::Relationship::ProxyMethods; use mro 'dfs';
    our @ISA = qw/ xx::DBIx::Class /;

    package xx::DBIx::Class::Relationship::Base; use mro 'dfs';
    our @ISA = qw/ xx::DBIx::Class /;

    package xx::DBIx::Class::PK::Auto; use mro 'dfs';
    our @ISA = qw/ xx::DBIx::Class /;

    package xx::DBIx::Class::PK; use mro 'dfs';
    our @ISA = qw/ xx::DBIx::Class::Row /;

    package xx::DBIx::Class::ResultSourceProxy::Table; use mro 'dfs';
    our @ISA = qw/
      xx::DBIx::Class::AccessorGroup
      xx::DBIx::Class::ResultSourceProxy
    /;

    package xx::DBIx::Class::ResultSourceProxy; use mro 'dfs';
    our @ISA = qw/ xx::DBIx::Class /;

    package xx::Class::Data::Accessor; our @ISA = (); use mro 'dfs';
    package xx::DBIx::Class::Relationship::HasMany; our @ISA = (); use mro 'dfs';
    package xx::DBIx::Class::Relationship::HasOne; our @ISA = (); use mro 'dfs';
    package xx::DBIx::Class::Relationship::BelongsTo; our @ISA = (); use mro 'dfs';
    package xx::DBIx::Class::Relationship::ManyToMany; our @ISA = (); use mro 'dfs';
    package xx::DBIx::Class::Componentised; our @ISA = (); use mro 'dfs';
    package xx::DBIx::Class::AccessorGroup; our @ISA = (); use mro 'dfs';
    package xx::DBIx::Class::Serialize::Storable; our @ISA = (); use mro 'dfs';
    package xx::DBIx::Class::Relationship::Accessor; our @ISA = (); use mro 'dfs';
    package xx::DBIx::Class::Relationship::CascadeActions; our @ISA = (); use mro 'dfs';
}

ok(eq_array(
    mro::get_linear_isa('xx::DBIx::Class::Core'),
    [qw/
        xx::DBIx::Class::Core
        xx::DBIx::Class::Serialize::Storable
        xx::DBIx::Class::InflateColumn
        xx::DBIx::Class::Row
        xx::DBIx::Class
        xx::DBIx::Class::Componentised
        xx::Class::Data::Accessor
        xx::DBIx::Class::Relationship
        xx::DBIx::Class::Relationship::Helpers
        xx::DBIx::Class::Relationship::HasMany
        xx::DBIx::Class::Relationship::HasOne
        xx::DBIx::Class::Relationship::BelongsTo
        xx::DBIx::Class::Relationship::ManyToMany
        xx::DBIx::Class::Relationship::Accessor
        xx::DBIx::Class::Relationship::CascadeActions
        xx::DBIx::Class::Relationship::ProxyMethods
        xx::DBIx::Class::Relationship::Base
        xx::DBIx::Class::PK::Auto
        xx::DBIx::Class::PK
        xx::DBIx::Class::ResultSourceProxy::Table
        xx::DBIx::Class::AccessorGroup
        xx::DBIx::Class::ResultSourceProxy
    /]
), '... got the right DFS merge order for xx::DBIx::Class::Core');
