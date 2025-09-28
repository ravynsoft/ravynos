use Test::More;
use File::Spec;
use File::Find;
use strict;

BEGIN { chdir 't' if -d 't' };

eval 'use Test::Pod';
plan skip_all => "Test::Pod v0.95 required for testing POD"
    if $@ || $Test::Pod::VERSION < 0.95;

plan skip_all => "Pod tests disabled under perl core" if $ENV{PERL_CORE};

my @files;
find( sub { push @files, File::Spec->catfile(
                    File::Spec->splitdir( $File::Find::dir ), $_
                ) if /\.p(?:l|m|od)$/ }, File::Spec->catdir(qw(.. blib lib) ));

plan skip_all => "No tests to run" unless scalar @files;

plan tests => scalar @files;
for my $file ( @files ) {
    pod_file_ok( $file );
}


