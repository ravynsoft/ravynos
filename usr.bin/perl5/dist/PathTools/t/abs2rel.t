#!/usr/bin/perl -w

use strict;
use Test::More;

use Cwd qw(cwd getcwd abs_path);
use File::Spec();
use File::Temp qw(tempdir);
use File::Path qw(mkpath);

my $startdir = cwd();
my @files = ( 'anyfile', './anyfile', '../first_sub_dir/anyfile', '../second_sub_dir/second_file' );

for my $file (@files) {
    test_rel2abs($file);
}

sub test_rel2abs {
    my $first_file = shift;
    my $tdir = tempdir( CLEANUP => 1 );
    chdir $tdir or die "Unable to change to $tdir: $!";

    my @subdirs = (
        'first_sub_dir',
        File::Spec->catdir('first_sub_dir',  'sub_sub_dir'),
        'second_sub_dir'
    );
    mkpath(\@subdirs, 0, 0711)
        or die "Unable to mkpath: $!";

    open my $OUT2, '>',
        File::Spec->catfile('second_sub_dir', 'second_file')
        or die "Unable to open 'second_file' for writing: $!";
    print $OUT2 "Attempting to resolve RT #121360\n";
    close $OUT2 or die "Unable to close 'second_file' after writing: $!";

    chdir 'first_sub_dir'
        or die "Unable to change to 'first_sub_dir': $!";
    open my $OUT1, '>', $first_file
        or die "Unable to open $first_file for writing: $!";
    print $OUT1 "Attempting to resolve RT #121360\n";
    close $OUT1 or die "Unable to close $first_file after writing: $!";

    my $rel_path = $first_file;
    my $rel_base = File::Spec->catdir(File::Spec->curdir(), 'sub_sub_dir');
    my $abs_path = File::Spec->rel2abs($rel_path);
    my $abs_base = File::Spec->rel2abs($rel_base);
    ok(-f $rel_path, "'$rel_path' is readable by effective uid/gid");
    ok(-f $abs_path, "'$abs_path' is readable by effective uid/gid");
    is_deeply(
        [ (stat $rel_path)[0..5] ],
        [ (stat $abs_path)[0..5] ],
        "rel_path and abs_path stat same"
    );
    ok(-d $rel_base, "'$rel_base' is a directory");
    ok(-d $abs_base, "'$abs_base' is a directory");
    is_deeply(
        [ (stat $rel_base)[0..5] ],
        [ (stat $abs_base)[0..5] ],
        "rel_base and abs_base stat same"
    );
    my $rr_link = File::Spec->abs2rel($rel_path, $rel_base);
    my $ra_link = File::Spec->abs2rel($rel_path, $abs_base);
    my $ar_link = File::Spec->abs2rel($abs_path, $rel_base);
    my $aa_link = File::Spec->abs2rel($abs_path, $abs_base);
    is($rr_link, $ra_link,
        "rel_path-rel_base '$rr_link' = rel_path-abs_base '$ra_link'");
    is($ar_link, $aa_link,
        "abs_path-rel_base '$ar_link' = abs_path-abs_base '$aa_link'");
    is($rr_link, $aa_link,
        "rel_path-rel_base '$rr_link' = abs_path-abs_base '$aa_link'");

    chdir $startdir or die "Unable to change back to $startdir: $!";
}

done_testing();
