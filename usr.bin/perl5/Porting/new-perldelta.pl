#!/usr/bin/perl -w
use strict;

# This needs to be able to run from a clean checkout, hence assume only system
# perl, which may be too old to have autodie

require './Porting/pod_lib.pl';

my $state = get_pod_metadata(1);
my (undef, $old_major, $old_minor) = @{$state->{delta_version}};
# For now, hard code it for the simple ones...
my $new_major = $old_major;
my $new_minor = $old_minor + 1;
# These two are just for "If you are upgrading from earlier releases..." in
# the perldelta template.
my $was_major = $old_major;
my $was_minor = $old_minor - 1;
# I may have missed some corner cases here:
if ($was_minor < 0) {
    $was_minor = 0;
    --$was_major;
}
my $newdelta_filename = "perl5$new_major${new_minor}delta.pod";

{
    # For now, just tell the user what to add, as it's safer.
    my %add;

    sub git_add_new {
        push @{$add{new}}, shift;
    }

    sub git_add_modified {
        push @{$add{modified}}, shift;
    }

    sub notify_success {
        return unless %add;
        print "Please run:\n";
        foreach (qw(new modified)) {
            print "    git add @{$add{$_}}\n" if $add{$_};
        }
        print "\nBefore committing please check that the build works and make test_porting passes\n";
    }
}

my $filename = 'pod/.gitignore';
my $gitignore = slurp_or_die($filename);

$gitignore =~ s{^/$state->{delta_target}$}
               {/$newdelta_filename}m
    or die "Can't find /$state->{delta_target} in $filename";

write_or_die($filename, $gitignore);
git_add_modified($filename);

my $olddelta = slurp_or_die('pod/perldelta.pod');

$olddelta =~ s{^(perl)(delta - what is new for perl v5.$old_major.$old_minor)$}
              {$1 . "5$old_major$old_minor" . $2}me
    or die "Can't find expected NAME contents in $olddelta";

my $olddeltaname = "pod/perl5$old_major${old_minor}delta.pod";
# in a built tree, $olddeltaname is a symlink to perldelta.pod, make sure
# we don't write through it
unlink($olddeltaname);
write_or_die($olddeltaname, $olddelta);
git_add_new($olddeltaname);

$filename = 'Porting/perldelta_template.pod';
my $newdelta = slurp_or_die($filename);

foreach([rXXX => $was_major],
        [sXXX => $old_major],
        [tXXX => $new_major],
        [aXXX => $was_minor],
        [bXXX => $old_minor],
        [cXXX => $new_minor],
        ['5XXX' => 5 . $old_major . $old_minor]) {
    my ($token, $value) = @$_;
    $newdelta =~ s/$token/$value/g
        or die "Can't find '$token' in $filename";
}

write_or_die('pod/perldelta.pod', $newdelta);
git_add_modified('pod/perldelta.pod');

$filename = 'pod/perl.pod';
my $pod_master = slurp_or_die($filename);

$pod_master =~ s{^(\s*perl5)($was_major$was_minor)(delta\s+Perl changes in version )(5\.\d+\.\d+)(.*)}
    {$1 . $old_major . $old_minor .$3 . "5.$old_major.$old_minor" . $5 . "\n" .
         "$1$2$3$4$5"}me
    or warn "Couldn't find perldelta line (for perl5$was_major${was_minor}delta) in $filename";

write_or_die($filename, $pod_master);
git_add_modified($filename);

my $command = "$^X Porting/pod_rules.pl";
system $command
    and die "Could not run '$command', \$? = $?";
git_add_modified(map {chomp $_; $_} `$^X Porting/pod_rules.pl --showfiles`);

notify_success();

# ex: set ts=8 sts=4 sw=4 et:
