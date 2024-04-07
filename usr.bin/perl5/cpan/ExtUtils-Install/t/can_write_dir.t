#!/usr/bin/perl -w

# Test the private _can_write_dir() function.

use strict;
use ExtUtils::Install;
use File::Spec;
{ package FS;  our @ISA = qw(File::Spec); }

# Alias it for easier access
*can_write_dir = \&ExtUtils::Install::_can_write_dir;

use Test::More 'no_plan';


my $dne = FS->catdir(qw(does not exist));
ok ! -e $dne;
is_deeply [can_write_dir($dne)],
          [1,
           FS->curdir,
           FS->catdir('does'),
           FS->catdir('does', 'not'),
           FS->catdir('does', 'not', 'exist')
          ];


my $abs_dne = FS->rel2abs($dne);
ok ! -e $abs_dne;
is_deeply [can_write_dir($abs_dne)],
          [1,
           FS->rel2abs(FS->curdir),
           FS->rel2abs(FS->catdir('does')),
           FS->rel2abs(FS->catdir('does', 'not')),
           FS->rel2abs(FS->catdir('does', 'not', 'exist')),
          ];

SKIP: {
    my $exists = FS->catdir(qw(exists));
    my $subdir = FS->catdir(qw(exists subdir));
    
    
    ok mkdir $exists;
    END { rmdir $exists }
    
    ok chmod 0555, $exists, 'make read only';

    skip "Current user or OS cannot create directories that they cannot read", 6
          if -w $exists; # these tests require a directory we cant read

    is_deeply [can_write_dir($exists)], [0, $exists];
    is_deeply [can_write_dir($subdir)], [0, $exists, $subdir];
    
    ok chmod 0777, $exists, 'make writable';
    ok -w $exists;
    is_deeply [can_write_dir($exists)], [1, $exists];
    is_deeply [can_write_dir($subdir)],
              [1,
               $exists,
               $subdir
              ];
}