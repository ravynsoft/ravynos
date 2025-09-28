#!/usr/bin/perl

use Test::More tests => 1;

use File::Spec;
use File::Path;
use File::Temp;

rmtree "testing";
mkdir "testing" or die "mkdir failed: $!";
chdir "testing";
mkdir "tmp" or die "mkdir failed: $!";

my $tempdirstr;
{
    my $dir = File::Temp->newdir( DIR => "tmp" );
    $tempdirstr = "$dir";

    mkdir "hide" or die "mkdir failed: $!";
    chdir "hide";
}

chdir File::Spec->updir;
$tempdirstr = File::Spec->rel2abs($tempdirstr);
ok !-d $tempdirstr or diag dircontent("tmp", $tempdirstr);

# cleanup
chdir File::Spec->updir;
rmtree( "testing" );

exit;

sub dircontent {
  my $dir = shift;
  my $tempdirstr = shift;
  my $str = "Contents of $dir (should not contain \"$tempdirstr\"):\n";
  opendir(my $DH, $dir) or die "opendir failed; $!";
  my @contents = grep $_ !~ /^\.+/, readdir($DH);
  closedir($DH);
  for my $ls (@contents) {
    $str .= "  $ls\n";
  }
  return $str;
}
