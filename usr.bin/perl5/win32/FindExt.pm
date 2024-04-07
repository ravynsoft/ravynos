package FindExt;

our $VERSION = '1.03';

use strict;
use warnings;

my $no = join('|',qw(Amiga.* GDBM_File ODBM_File NDBM_File DB_File
                     VMS.* Sys-Syslog IPC-SysV));
$no = qr/^(?:$no)$/i;

sub apply_config {
    my ($config) = @_;
    my @no;

    push @no, 'Sys-Syslog' if $^O eq 'MSWin32';

    # duplicates logic from Configure (mostly)
    push @no, "DB_File" unless $config->{i_db};
    push @no, "GDBM_File" unless $config->{i_gdbm};
    push @no, "IPC-SysV" unless $config->{d_msg} || $config->{d_sem} || $config->{d_shm};
    push @no, "NDBM_File" unless $config->{d_ndbm};
    push @no, "ODBM_File"
      unless ($config->{i_dbm} || $config->{i_rpcsvcdbm}) && !$config->{d_cplusplus};
    push @no, "Amiga.*" unless $^O eq "amigaos";
    push @no, "VMS.*" unless $^O eq "VMS";
    push @no, "Win32.*" unless $^O eq "MSWin32" || $^O eq "cygwin";

    $no = join('|', @no);
    $no = qr/^(?:$no)$/i;
}

my %ext;
my %static;

sub set_static_extensions {
    # adjust results of scan_ext, and also save
    # statics in case scan_ext hasn't been called yet.
    # if '*' is passed then all XS extensions are static
    # (with possible exclusions)
    %static = ();
    my @list = @_;
    if (@_ and $_[0] eq '*') {
	my %excl = map {$_=>1} map {m/^!(.*)$/} @_[1 .. $#_];
	@list = grep {!exists $excl{$_}} keys %ext;
    }
    for (@list) {
        $static{$_} = 1;
        $ext{$_} = 'static' if $ext{$_} && $ext{$_} eq 'dynamic';
    }

    # Encode is a special case.  If we are building Encode as a static
    # extension, we need to explicitly list its subextensions as well.
    # For other nested extensions, this is handled automatically by
    # the appropriate Makefile.PL.
    if ($ext{Encode} && $ext{Encode} eq 'static') {
        require File::Find;
        File::Find::find({
                          no_chdir => 1,
                          wanted => sub {
                              return unless m!\b(Encode/.+)/Makefile\.PL!;
                              $static{$1} = 1;
                              $ext{$1} = 'static';
                          },
                         }, "../cpan/Encode");
    }
}

sub _ext_eq {
    my $key = shift;
    sub {
        sort grep $ext{$_} eq $key, keys %ext;
    }
}

*dynamic_ext = _ext_eq('dynamic');
*static_ext = _ext_eq('static');
*nonxs_ext = _ext_eq('nonxs');

sub extensions {
    sort grep $ext{$_} ne 'known', keys %ext;
}

sub known_extensions {
    sort keys %ext;
}

sub is_static
{
 return $ext{$_[0]} eq 'static'
}

sub has_xs_or_c {
    my $dir = shift;
    opendir my $dh, $dir or die "opendir $dir: $!";
    while (defined (my $item = readdir $dh)) {
        return 1 if $item =~ /\.xs$/;
        return 1 if $item =~ /\.c$/;
    }
    return 0;
}

# Function to find available extensions, ignoring DynaLoader
sub scan_ext
{
    my $ext_dir = shift;
    opendir my $dh, "$ext_dir";
    while (defined (my $item = readdir $dh)) {
        next if $item =~ /^\.\.?$/;
        next if $item eq "DynaLoader";
        next unless -d "$ext_dir/$item";
        my $this_ext = $item;
        my $leaf = $item;

        $this_ext =~ s!-!/!g;
        $leaf =~ s/.*-//;

        # List/Util.xs lives in Scalar-List-Utils, Cwd.xs lives in PathTools
        $this_ext = 'List/Util' if $this_ext eq 'Scalar/List/Utils';
        $this_ext = 'Cwd'       if $this_ext eq 'PathTools';

	# Temporary hack to cope with smokers that are not clearing directories:
        next if $ext{$this_ext};

        if (has_xs_or_c("$ext_dir/$item")) {
            $ext{$this_ext} = $static{$this_ext} ? 'static' : 'dynamic';
        } else {
            $ext{$this_ext} = 'nonxs';
        }
        $ext{$this_ext} = 'known' if $item =~ $no;
    }
}

1;
# ex: set ts=8 sts=4 sw=4 et:
