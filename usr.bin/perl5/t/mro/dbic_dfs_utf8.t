#!./perl

use strict;
use warnings;
use utf8;
use open qw( :utf8 :std );

require q(./test.pl); plan(tests => 1);

=pod

=encoding UTF-8

This example is taken from the inheritance graph of DBIx::Class::Coﾚ in DBIx::Class v0.07002:
(No ASCII art this time, this graph is insane)

The xx:: prefixes are just to be sure these bogus declarations never stomp on real ones

=cut

{
    package Ẋẋ::ḐʙIＸ::Cl았::Coﾚ; use mro 'dfs';
    our @ISA = qw/
      Ẋẋ::ḐʙIＸ::Cl았::ᓭᚱi알ḭźɜ::Sᑐ랍lえ
      Ẋẋ::ḐʙIＸ::Cl았::Ĭⁿᰒ텣올움ᶮ
      Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ
      Ẋẋ::ḐʙIＸ::Cl았::ᛕķ::Ạuต
      Ẋẋ::ḐʙIＸ::Cl았::ᛕķ
      Ẋẋ::ḐʙIＸ::Cl았::ﾛẈ
      Ẋẋ::ḐʙIＸ::Cl았::ResultSourceProxy::탑lẹ
      Ẋẋ::ḐʙIＸ::Cl았::ᚪc엤ȭઋᶢऋouꩇ
    /;

    package Ẋẋ::ḐʙIＸ::Cl았::Ĭⁿᰒ텣올움ᶮ; use mro 'dfs';
    our @ISA = qw/ Ẋẋ::ḐʙIＸ::Cl았::ﾛẈ /;

    package Ẋẋ::ḐʙIＸ::Cl았::ﾛẈ; use mro 'dfs';
    our @ISA = qw/ Ẋẋ::ḐʙIＸ::Cl았 /;

    package Ẋẋ::ḐʙIＸ::Cl았; use mro 'dfs';
    our @ISA = qw/
      Ẋẋ::ḐʙIＸ::Cl았::촘폰en팃엗
      xx::Cl았::닽Ӕ::앛쳇sᚖ
    /;

    package Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ; use mro 'dfs';
    our @ISA = qw/
      Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::헬pḜrＳ
      Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::앛쳇sᚖ
      Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::찻찯eᚪtЁnʂ
      Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::Pr오xᐇMeᖪ옫ś
      Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::밧え
      Ẋẋ::ḐʙIＸ::Cl았
    /;

    package Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::헬pḜrＳ; use mro 'dfs';
    our @ISA = qw/
      Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::핫ᛗƳ
      Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::핫ᶱn
      Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::бl옹sTȭ
      Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::ᛗᓆ톰ẰᚿẎ
    /;

    package Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::Pr오xᐇMeᖪ옫ś; use mro 'dfs';
    our @ISA = qw/ Ẋẋ::ḐʙIＸ::Cl았 /;

    package Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::밧え; use mro 'dfs';
    our @ISA = qw/ Ẋẋ::ḐʙIＸ::Cl았 /;

    package Ẋẋ::ḐʙIＸ::Cl았::ᛕķ::Ạuต; use mro 'dfs';
    our @ISA = qw/ Ẋẋ::ḐʙIＸ::Cl았 /;

    package Ẋẋ::ḐʙIＸ::Cl았::ᛕķ; use mro 'dfs';
    our @ISA = qw/ Ẋẋ::ḐʙIＸ::Cl았::ﾛẈ /;

    package Ẋẋ::ḐʙIＸ::Cl았::ResultSourceProxy::탑lẹ; use mro 'dfs';
    our @ISA = qw/
      Ẋẋ::ḐʙIＸ::Cl았::ᚪc엤ȭઋᶢऋouꩇ
      Ẋẋ::ḐʙIＸ::Cl았::ResultSourceProxy
    /;

    package Ẋẋ::ḐʙIＸ::Cl았::ResultSourceProxy; use mro 'dfs';
    our @ISA = qw/ Ẋẋ::ḐʙIＸ::Cl았 /;

    package xx::Cl았::닽Ӕ::앛쳇sᚖ; our @ISA = (); use mro 'dfs';
    package Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::핫ᛗƳ; our @ISA = (); use mro 'dfs';
    package Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::핫ᶱn; our @ISA = (); use mro 'dfs';
    package Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::бl옹sTȭ; our @ISA = (); use mro 'dfs';
    package Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::ᛗᓆ톰ẰᚿẎ; our @ISA = (); use mro 'dfs';
    package Ẋẋ::ḐʙIＸ::Cl았::촘폰en팃엗; our @ISA = (); use mro 'dfs';
    package Ẋẋ::ḐʙIＸ::Cl았::ᚪc엤ȭઋᶢऋouꩇ; our @ISA = (); use mro 'dfs';
    package Ẋẋ::ḐʙIＸ::Cl았::ᓭᚱi알ḭźɜ::Sᑐ랍lえ; our @ISA = (); use mro 'dfs';
    package Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::앛쳇sᚖ; our @ISA = (); use mro 'dfs';
    package Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::찻찯eᚪtЁnʂ; our @ISA = (); use mro 'dfs';
}

ok(eq_array(
    mro::get_linear_isa('Ẋẋ::ḐʙIＸ::Cl았::Coﾚ'),
    [qw/
        Ẋẋ::ḐʙIＸ::Cl았::Coﾚ
        Ẋẋ::ḐʙIＸ::Cl았::ᓭᚱi알ḭźɜ::Sᑐ랍lえ
        Ẋẋ::ḐʙIＸ::Cl았::Ĭⁿᰒ텣올움ᶮ
        Ẋẋ::ḐʙIＸ::Cl았::ﾛẈ
        Ẋẋ::ḐʙIＸ::Cl았
        Ẋẋ::ḐʙIＸ::Cl았::촘폰en팃엗
        xx::Cl았::닽Ӕ::앛쳇sᚖ
        Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ
        Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::헬pḜrＳ
        Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::핫ᛗƳ
        Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::핫ᶱn
        Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::бl옹sTȭ
        Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::ᛗᓆ톰ẰᚿẎ
        Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::앛쳇sᚖ
        Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::찻찯eᚪtЁnʂ
        Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::Pr오xᐇMeᖪ옫ś
        Ẋẋ::ḐʙIＸ::Cl았::렐aチ온ሺṖ::밧え
        Ẋẋ::ḐʙIＸ::Cl았::ᛕķ::Ạuต
        Ẋẋ::ḐʙIＸ::Cl았::ᛕķ
        Ẋẋ::ḐʙIＸ::Cl았::ResultSourceProxy::탑lẹ
        Ẋẋ::ḐʙIＸ::Cl았::ᚪc엤ȭઋᶢऋouꩇ
        Ẋẋ::ḐʙIＸ::Cl았::ResultSourceProxy
    /]
), '... got the right DFS merge order for Ẋẋ::ḐʙIＸ::Cl았::Coﾚ');
