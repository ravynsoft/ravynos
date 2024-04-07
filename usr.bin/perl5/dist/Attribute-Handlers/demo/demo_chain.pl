#! /usr/local/bin/perl -w

use Attribute::Handlers;

sub Prefix : ATTR {
  my ($glob, $sub) = @_[1,2];
  no warnings 'redefine';
  *$glob = sub {
                 print "This happens first\n";
                 $sub->(@_);
               };
}

sub Postfix : ATTR {
  my ($glob, $sub) = @_[1,2];
  no warnings 'redefine';
  *$glob = sub {
                 $sub->(@_);
                 print "This happens last\n";
               };
}

sub test : Postfix Prefix {
  print "Hello World\n";
}

test();
