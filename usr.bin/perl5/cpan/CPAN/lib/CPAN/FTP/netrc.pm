package CPAN::FTP::netrc;
use strict;

$CPAN::FTP::netrc::VERSION = $CPAN::FTP::netrc::VERSION = "1.01";

# package CPAN::FTP::netrc;
sub new {
    my($class) = @_;
    my $file = File::Spec->catfile($ENV{HOME},".netrc");

    my($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
       $atime,$mtime,$ctime,$blksize,$blocks)
        = stat($file);
    $mode ||= 0;
    my $protected = 0;

    my($fh,@machines,$hasdefault);
    $hasdefault = 0;
    $fh = FileHandle->new or die "Could not create a filehandle";

    if($fh->open($file)) {
        $protected = ($mode & 077) == 0;
        local($/) = "";
      NETRC: while (<$fh>) {
            my(@tokens) = split " ", $_;
          TOKEN: while (@tokens) {
                my($t) = shift @tokens;
                if ($t eq "default") {
                    $hasdefault++;
                    last NETRC;
                }
                last TOKEN if $t eq "macdef";
                if ($t eq "machine") {
                    push @machines, shift @tokens;
                }
            }
        }
    } else {
        $file = $hasdefault = $protected = "";
    }

    bless {
        'mach' => [@machines],
        'netrc' => $file,
        'hasdefault' => $hasdefault,
        'protected' => $protected,
    }, $class;
}

# CPAN::FTP::netrc::hasdefault;
sub hasdefault { shift->{'hasdefault'} }
sub netrc      { shift->{'netrc'}      }
sub protected  { shift->{'protected'}  }
sub contains {
    my($self,$mach) = @_;
    for ( @{$self->{'mach'}} ) {
        return 1 if $_ eq $mach;
    }
    return 0;
}

1;
