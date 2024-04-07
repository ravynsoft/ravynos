# This will put installed perl files into some other location
# Note that we cannot put hashbang to be extproc to make Configure work.

use Config;
use File::Compare;

$dir = shift;
$dir =~ s|/|\\|g ;
$nowarn = 1, $dir = shift if $dir eq '-n';

die <<EOU unless defined $dir and -d $dir;
usage:	$^X $0 [-n] directory-to-install
  -n	do not check whether the directory is not on path
EOU

@path = split /;/, $ENV{PATH};
$idir = $Config{installbin};
$indir =~ s|\\|/|g ;

my %seen;

foreach $file (<$idir/*>) {
  next if $file =~ /\.(exe|bak)/i;
  $base = $file;
  $base =~ s/\.$//;		# just in case...
  $base =~ s|.*/||;
  $base =~ s|\.pl$||;
  #$file =~ s|/|\\|g ;
  warn "Clashing output name for $file, skipping" if $seen{$base}++;
  my $new = (-f "$dir/$base.cmd" ? '' : ' (new file)');
  print "Processing $file => $dir/$base.cmd$new\n";
  my $ext = ($new ? '.cmd' : '.tcm');
  open IN, '<', $file or warn, next;
  open OUT, '>', "$dir/$base$ext" or warn, next;
  my $firstline = <IN>;
  my $flags = '';
  $flags = $2 if $firstline =~ /^#!\s*(\S+)\s+-([^#]+?)\s*(#|$)/;
  print OUT "extproc perl -S$flags\n$firstline";
  print OUT $_ while <IN>;
  close IN or warn, next;
  close OUT or warn, next;
  chmod 0444, "$dir/$base$ext";
  next if $new;
  if (compare "$dir/$base$ext", "$dir/$base.cmd") {	# different
    chmod 0666, "$dir/$base.cmd";
    unlink "$dir/$base.cmd";
    rename "$dir/$base$ext", "$dir/$base.cmd";
  } else {
    chmod 0666, "$dir/$base$ext";
    unlink "$dir/$base$ext";
    print "...unchanged...\n";
  }
}

