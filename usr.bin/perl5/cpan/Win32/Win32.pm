package Win32;

# BEGIN {
    use strict;
    use vars qw|$VERSION $XS_VERSION @ISA @EXPORT @EXPORT_OK|;

    require Exporter;
    require DynaLoader;

    @ISA = qw|Exporter DynaLoader|;
    $VERSION = '0.59';
    $XS_VERSION = $VERSION;
    $VERSION = eval $VERSION;

    @EXPORT = qw(
	NULL
	WIN31_CLASS
	OWNER_SECURITY_INFORMATION
	GROUP_SECURITY_INFORMATION
	DACL_SECURITY_INFORMATION
	SACL_SECURITY_INFORMATION
	MB_ICONHAND
	MB_ICONQUESTION
	MB_ICONEXCLAMATION
	MB_ICONASTERISK
	MB_ICONWARNING
	MB_ICONERROR
	MB_ICONINFORMATION
	MB_ICONSTOP
    );
    @EXPORT_OK = qw(
        GetOSName
        SW_HIDE
        SW_SHOWNORMAL
        SW_SHOWMINIMIZED
        SW_SHOWMAXIMIZED
        SW_SHOWNOACTIVATE

        CSIDL_DESKTOP
        CSIDL_PROGRAMS
        CSIDL_PERSONAL
        CSIDL_FAVORITES
        CSIDL_STARTUP
        CSIDL_RECENT
        CSIDL_SENDTO
        CSIDL_STARTMENU
        CSIDL_MYMUSIC
        CSIDL_MYVIDEO
        CSIDL_DESKTOPDIRECTORY
        CSIDL_NETHOOD
        CSIDL_FONTS
        CSIDL_TEMPLATES
        CSIDL_COMMON_STARTMENU
        CSIDL_COMMON_PROGRAMS
        CSIDL_COMMON_STARTUP
        CSIDL_COMMON_DESKTOPDIRECTORY
        CSIDL_APPDATA
        CSIDL_PRINTHOOD
        CSIDL_LOCAL_APPDATA
        CSIDL_COMMON_FAVORITES
        CSIDL_INTERNET_CACHE
        CSIDL_COOKIES
        CSIDL_HISTORY
        CSIDL_COMMON_APPDATA
        CSIDL_WINDOWS
        CSIDL_SYSTEM
        CSIDL_PROGRAM_FILES
        CSIDL_MYPICTURES
        CSIDL_PROFILE
        CSIDL_PROGRAM_FILES_COMMON
        CSIDL_COMMON_TEMPLATES
        CSIDL_COMMON_DOCUMENTS
        CSIDL_COMMON_ADMINTOOLS
        CSIDL_ADMINTOOLS
        CSIDL_COMMON_MUSIC
        CSIDL_COMMON_PICTURES
        CSIDL_COMMON_VIDEO
        CSIDL_RESOURCES
        CSIDL_RESOURCES_LOCALIZED
        CSIDL_CDBURN_AREA
    );
# }

# We won't bother with the constant stuff, too much of a hassle.  Just hard
# code it here.

sub NULL 				{ 0 }
sub WIN31_CLASS 			{ &NULL }

sub OWNER_SECURITY_INFORMATION		{ 0x00000001 }
sub GROUP_SECURITY_INFORMATION		{ 0x00000002 }
sub DACL_SECURITY_INFORMATION		{ 0x00000004 }
sub SACL_SECURITY_INFORMATION		{ 0x00000008 }

sub MB_ICONHAND				{ 0x00000010 }
sub MB_ICONQUESTION			{ 0x00000020 }
sub MB_ICONEXCLAMATION			{ 0x00000030 }
sub MB_ICONASTERISK			{ 0x00000040 }
sub MB_ICONWARNING			{ 0x00000030 }
sub MB_ICONERROR			{ 0x00000010 }
sub MB_ICONINFORMATION			{ 0x00000040 }
sub MB_ICONSTOP				{ 0x00000010 }

#
# Newly added constants.  These have an empty prototype, unlike the
# the ones above, which aren't prototyped for compatibility reasons.
#
sub SW_HIDE           ()		{ 0 }
sub SW_SHOWNORMAL     ()		{ 1 }
sub SW_SHOWMINIMIZED  ()		{ 2 }
sub SW_SHOWMAXIMIZED  ()		{ 3 }
sub SW_SHOWNOACTIVATE ()		{ 4 }

sub CSIDL_DESKTOP              ()       { 0x0000 }     # <desktop>
sub CSIDL_PROGRAMS             ()       { 0x0002 }     # Start Menu\Programs
sub CSIDL_PERSONAL             ()       { 0x0005 }     # "My Documents" folder
sub CSIDL_FAVORITES            ()       { 0x0006 }     # <user name>\Favorites
sub CSIDL_STARTUP              ()       { 0x0007 }     # Start Menu\Programs\Startup
sub CSIDL_RECENT               ()       { 0x0008 }     # <user name>\Recent
sub CSIDL_SENDTO               ()       { 0x0009 }     # <user name>\SendTo
sub CSIDL_STARTMENU            ()       { 0x000B }     # <user name>\Start Menu
sub CSIDL_MYMUSIC              ()       { 0x000D }     # "My Music" folder
sub CSIDL_MYVIDEO              ()       { 0x000E }     # "My Videos" folder
sub CSIDL_DESKTOPDIRECTORY     ()       { 0x0010 }     # <user name>\Desktop
sub CSIDL_NETHOOD              ()       { 0x0013 }     # <user name>\nethood
sub CSIDL_FONTS                ()       { 0x0014 }     # windows\fonts
sub CSIDL_TEMPLATES            ()       { 0x0015 }
sub CSIDL_COMMON_STARTMENU     ()       { 0x0016 }     # All Users\Start Menu
sub CSIDL_COMMON_PROGRAMS      ()       { 0x0017 }     # All Users\Start Menu\Programs
sub CSIDL_COMMON_STARTUP       ()       { 0x0018 }     # All Users\Startup
sub CSIDL_COMMON_DESKTOPDIRECTORY ()    { 0x0019 }     # All Users\Desktop
sub CSIDL_APPDATA              ()       { 0x001A }     # Application Data, new for NT4
sub CSIDL_PRINTHOOD            ()       { 0x001B }     # <user name>\PrintHood
sub CSIDL_LOCAL_APPDATA        ()       { 0x001C }     # non roaming, user\Local Settings\Application Data
sub CSIDL_COMMON_FAVORITES     ()       { 0x001F }
sub CSIDL_INTERNET_CACHE       ()       { 0x0020 }
sub CSIDL_COOKIES              ()       { 0x0021 }
sub CSIDL_HISTORY              ()       { 0x0022 }
sub CSIDL_COMMON_APPDATA       ()       { 0x0023 }     # All Users\Application Data
sub CSIDL_WINDOWS              ()       { 0x0024 }     # GetWindowsDirectory()
sub CSIDL_SYSTEM               ()       { 0x0025 }     # GetSystemDirectory()
sub CSIDL_PROGRAM_FILES        ()       { 0x0026 }     # C:\Program Files
sub CSIDL_MYPICTURES           ()       { 0x0027 }     # "My Pictures", new for Win2K
sub CSIDL_PROFILE              ()       { 0x0028 }     # USERPROFILE
sub CSIDL_PROGRAM_FILES_COMMON ()       { 0x002B }     # C:\Program Files\Common
sub CSIDL_COMMON_TEMPLATES     ()       { 0x002D }     # All Users\Templates
sub CSIDL_COMMON_DOCUMENTS     ()       { 0x002E }     # All Users\Documents
sub CSIDL_COMMON_ADMINTOOLS    ()       { 0x002F }     # All Users\Start Menu\Programs\Administrative Tools
sub CSIDL_ADMINTOOLS           ()       { 0x0030 }     # <user name>\Start Menu\Programs\Administrative Tools
sub CSIDL_COMMON_MUSIC         ()       { 0x0035 }     # All Users\My Music
sub CSIDL_COMMON_PICTURES      ()       { 0x0036 }     # All Users\My Pictures
sub CSIDL_COMMON_VIDEO         ()       { 0x0037 }     # All Users\My Video
sub CSIDL_RESOURCES            ()       { 0x0038 }     # %windir%\Resources\, For theme and other windows resources.
sub CSIDL_RESOURCES_LOCALIZED  ()       { 0x0039 }     # %windir%\Resources\<LangID>, for theme and other windows specific resources.
sub CSIDL_CDBURN_AREA          ()       { 0x003B }     # <user name>\Local Settings\Application Data\Microsoft\CD Burning

sub VER_NT_DOMAIN_CONTROLLER () { 0x0000002 } # The system is a domain controller and the operating system is Windows Server 2008, Windows Server 2003, or Windows 2000 Server.
sub VER_NT_SERVER () { 0x0000003 } # The operating system is Windows Server 2008, Windows Server 2003, or Windows 2000 Server.
# Note that a server that is also a domain controller is reported as VER_NT_DOMAIN_CONTROLLER, not VER_NT_SERVER.
sub VER_NT_WORKSTATION () { 0x0000001 } # The operating system is Windows Vista, Windows XP Professional, Windows XP Home Edition, or Windows 2000 Professional.


sub VER_SUITE_BACKOFFICE               () { 0x00000004 } # Microsoft BackOffice components are installed.
sub VER_SUITE_BLADE                    () { 0x00000400 } # Windows Server 2003, Web Edition is installed.
sub VER_SUITE_COMPUTE_SERVER           () { 0x00004000 } # Windows Server 2003, Compute Cluster Edition is installed.
sub VER_SUITE_DATACENTER               () { 0x00000080 } # Windows Server 2008 Datacenter, Windows Server 2003, Datacenter Edition, or Windows 2000 Datacenter Server is installed.
sub VER_SUITE_ENTERPRISE               () { 0x00000002 } # Windows Server 2008 Enterprise, Windows Server 2003, Enterprise Edition, or Windows 2000 Advanced Server is installed. Refer to the Remarks section for more information about this bit flag.
sub VER_SUITE_EMBEDDEDNT               () { 0x00000040 } # Windows XP Embedded is installed.
sub VER_SUITE_PERSONAL                 () { 0x00000200 } # Windows Vista Home Premium, Windows Vista Home Basic, or Windows XP Home Edition is installed.
sub VER_SUITE_SINGLEUSERTS             () { 0x00000100 } # Remote Desktop is supported, but only one interactive session is supported. This value is set unless the system is running in application server mode.
sub VER_SUITE_SMALLBUSINESS            () { 0x00000001 } # Microsoft Small Business Server was once installed on the system, but may have been upgraded to another version of Windows. Refer to the Remarks section for more information about this bit flag.
sub VER_SUITE_SMALLBUSINESS_RESTRICTED () { 0x00000020 } # Microsoft Small Business Server is installed with the restrictive client license in force. Refer to the Remarks section for more information about this bit flag.
sub VER_SUITE_STORAGE_SERVER           () { 0x00002000 } # Windows Storage Server 2003 R2 or Windows Storage Server 2003 is installed.
sub VER_SUITE_TERMINAL                 () { 0x00000010 } # Terminal Services is installed. This value is always set.
# If VER_SUITE_TERMINAL is set but VER_SUITE_SINGLEUSERTS is not set, the system is running in application server mode.
sub VER_SUITE_WH_SERVER                () { 0x00008000 } # Windows Home Server is installed.
sub VER_SUITE_MULTIUSERTS              () { 0x00020000 } # AppServer mode is enabled.


sub SM_TABLETPC                ()       { 86 }
sub SM_MEDIACENTER             ()       { 87 }
sub SM_STARTER                 ()       { 88 }
sub SM_SERVERR2                ()       { 89 }

sub PRODUCT_UNDEFINED                        () { 0x000 } # An unknown product
sub PRODUCT_ULTIMATE                         () { 0x001 } # Ultimate
sub PRODUCT_HOME_BASIC                       () { 0x002 } # Home Basic
sub PRODUCT_HOME_PREMIUM                     () { 0x003 } # Home Premium
sub PRODUCT_ENTERPRISE                       () { 0x004 } # Enterprise
sub PRODUCT_HOME_BASIC_N                     () { 0x005 } # Home Basic N
sub PRODUCT_BUSINESS                         () { 0x006 } # Business
sub PRODUCT_STANDARD_SERVER                  () { 0x007 } # Server Standard (full installation)
sub PRODUCT_DATACENTER_SERVER                () { 0x008 } # Server Datacenter (full installation)
sub PRODUCT_SMALLBUSINESS_SERVER             () { 0x009 } # Windows Small Business Server
sub PRODUCT_ENTERPRISE_SERVER                () { 0x00A } # Server Enterprise (full installation)
sub PRODUCT_STARTER                          () { 0x00B } # Starter
sub PRODUCT_DATACENTER_SERVER_CORE           () { 0x00C } # Server Datacenter (core installation)
sub PRODUCT_STANDARD_SERVER_CORE             () { 0x00D } # Server Standard (core installation)
sub PRODUCT_ENTERPRISE_SERVER_CORE           () { 0x00E } # Server Enterprise (core installation)
sub PRODUCT_ENTERPRISE_SERVER_IA64           () { 0x00F } # Server Enterprise for Itanium-based Systems
sub PRODUCT_BUSINESS_N                       () { 0x010 } # Business N
sub PRODUCT_WEB_SERVER                       () { 0x011 } # Web Server (full installation)
sub PRODUCT_CLUSTER_SERVER                   () { 0x012 } # HPC Edition
sub PRODUCT_HOME_SERVER                      () { 0x013 } # Home Server Edition
sub PRODUCT_STORAGE_EXPRESS_SERVER           () { 0x014 } # Storage Server Express
sub PRODUCT_STORAGE_STANDARD_SERVER          () { 0x015 } # Storage Server Standard
sub PRODUCT_STORAGE_WORKGROUP_SERVER         () { 0x016 } # Storage Server Workgroup
sub PRODUCT_STORAGE_ENTERPRISE_SERVER        () { 0x017 } # Storage Server Enterprise
sub PRODUCT_SERVER_FOR_SMALLBUSINESS         () { 0x018 } # Windows Server 2008 for Windows Essential Server Solutions
sub PRODUCT_SMALLBUSINESS_SERVER_PREMIUM     () { 0x019 } # Windows Small Business Server Premium
sub PRODUCT_HOME_PREMIUM_N                   () { 0x01A } # Home Premium N
sub PRODUCT_ENTERPRISE_N                     () { 0x01B } # Enterprise N
sub PRODUCT_ULTIMATE_N                       () { 0x01C } # Ultimate N
sub PRODUCT_WEB_SERVER_CORE                  () { 0x01D } # Web Server (core installation)
sub PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT () { 0x01E } # Windows Essential Business Server Management Server
sub PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY   () { 0x01F } # Windows Essential Business Server Security Server
sub PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING  () { 0x020 } # Windows Essential Business Server Messaging Server
sub PRODUCT_SERVER_FOUNDATION                () { 0x021 } # Server Foundation
#define PRODUCT_HOME_PREMIUM_SERVER                 0x00000022
sub PRODUCT_SERVER_FOR_SMALLBUSINESS_V       () { 0x023 } # Windows Server 2008 without Hyper-V for Windows Essential Server Solutions
sub PRODUCT_STANDARD_SERVER_V                () { 0x024 } # Server Standard without Hyper-V (full installation)
sub PRODUCT_DATACENTER_SERVER_V              () { 0x025 } # Server Datacenter without Hyper-V (full installation)
sub PRODUCT_ENTERPRISE_SERVER_V              () { 0x026 } # Server Enterprise without Hyper-V (full installation)
sub PRODUCT_DATACENTER_SERVER_CORE_V         () { 0x027 } # Server Datacenter without Hyper-V (core installation)
sub PRODUCT_STANDARD_SERVER_CORE_V           () { 0x028 } # Server Standard without Hyper-V (core installation)
sub PRODUCT_ENTERPRISE_SERVER_CORE_V         () { 0x029 } # Server Enterprise without Hyper-V (core installation)
sub PRODUCT_HYPERV                           () { 0x02A } # Microsoft Hyper-V Server
#define PRODUCT_STORAGE_EXPRESS_SERVER_CORE         0x0000002B
#define PRODUCT_STORAGE_STANDARD_SERVER_CORE        0x0000002C
#define PRODUCT_STORAGE_WORKGROUP_SERVER_CORE       0x0000002D
#define PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE      0x0000002E
sub PRODUCT_STARTER_N                        () { 0x02F } # Starter N
sub PRODUCT_PROFESSIONAL                     () { 0x030 } # Professional
sub PRODUCT_PROFESSIONAL_N                   () { 0x031 } # Professional N
#define PRODUCT_SB_SOLUTION_SERVER                  0x00000032
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS             0x00000033
#define PRODUCT_STANDARD_SERVER_SOLUTIONS           0x00000034
#define PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE      0x00000035
#define PRODUCT_SB_SOLUTION_SERVER_EM               0x00000036
#define PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM          0x00000037
#define PRODUCT_SOLUTION_EMBEDDEDSERVER             0x00000038
#define PRODUCT_SOLUTION_EMBEDDEDSERVER_CORE        0x00000039
#define PRODUCT_PROFESSIONAL_EMBEDDED               0x0000003A
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT       0x0000003B
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL       0x0000003C
#define PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC    0x0000003D
#define PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC    0x0000003E
#define PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE   0x0000003F
#define PRODUCT_CLUSTER_SERVER_V                    0x00000040
#define PRODUCT_EMBEDDED                            0x00000041
sub PRODUCT_STARTER_E                        () { 0x042 } # Starter E
sub PRODUCT_HOME_BASIC_E                     () { 0x043 } # Home Basic E
sub PRODUCT_HOME_PREMIUM_E                   () { 0x044 } # Home Premium E
sub PRODUCT_PROFESSIONAL_E                   () { 0x045 } # Professional E
sub PRODUCT_ENTERPRISE_E                     () { 0x046 } # Enterprise E
sub PRODUCT_ULTIMATE_E                       () { 0x047 } # Ultimate E
#define PRODUCT_ENTERPRISE_EVALUATION               0x00000048
#define PRODUCT_MULTIPOINT_STANDARD_SERVER          0x0000004C
#define PRODUCT_MULTIPOINT_PREMIUM_SERVER           0x0000004D
#define PRODUCT_STANDARD_EVALUATION_SERVER          0x0000004F
#define PRODUCT_DATACENTER_EVALUATION_SERVER        0x00000050
#define PRODUCT_ENTERPRISE_N_EVALUATION             0x00000054
#define PRODUCT_EMBEDDED_AUTOMOTIVE                 0x00000055
#define PRODUCT_EMBEDDED_INDUSTRY_A                 0x00000056
#define PRODUCT_THINPC                              0x00000057
#define PRODUCT_EMBEDDED_A                          0x00000058
#define PRODUCT_EMBEDDED_INDUSTRY                   0x00000059
#define PRODUCT_EMBEDDED_E                          0x0000005A
#define PRODUCT_EMBEDDED_INDUSTRY_E                 0x0000005B
#define PRODUCT_EMBEDDED_INDUSTRY_A_E               0x0000005C
#define PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER 0x0000005F
#define PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER  0x00000060
#define PRODUCT_CORE_ARM                            0x00000061
sub PRODUCT_CORE_N                           () { 0x62 } # Windows 10 Home N
sub PRODUCT_CORE_COUNTRYSPECIFIC             () { 0x63 } # Windows 10 Home China
sub PRODUCT_CORE_SINGLELANGUAGE              () { 0x64 } # Windows 10 Home Single Language
sub PRODUCT_CORE                             () { 0x65 } # Windows 10 Home
#define PRODUCT_PROFESSIONAL_WMC                    0x00000067
#define PRODUCT_MOBILE_CORE                         0x00000068
#define PRODUCT_EMBEDDED_INDUSTRY_EVAL              0x00000069
#define PRODUCT_EMBEDDED_INDUSTRY_E_EVAL            0x0000006A
#define PRODUCT_EMBEDDED_EVAL                       0x0000006B
#define PRODUCT_EMBEDDED_E_EVAL                     0x0000006C
#define PRODUCT_NANO_SERVER                         0x0000006D
#define PRODUCT_CLOUD_STORAGE_SERVER                0x0000006E
#define PRODUCT_CORE_CONNECTED                      0x0000006F
#define PRODUCT_PROFESSIONAL_STUDENT                0x00000070
#define PRODUCT_CORE_CONNECTED_N                    0x00000071
#define PRODUCT_PROFESSIONAL_STUDENT_N              0x00000072
#define PRODUCT_CORE_CONNECTED_SINGLELANGUAGE       0x00000073
#define PRODUCT_CORE_CONNECTED_COUNTRYSPECIFIC      0x00000074
#define PRODUCT_CONNECTED_CAR                       0x00000075
#define PRODUCT_INDUSTRY_HANDHELD                   0x00000076
#define PRODUCT_PPI_PRO                             0x00000077
#define PRODUCT_ARM64_SERVER                        0x00000078
sub PRODUCT_EDUCATION                        () { 0x79 } # Windows 10 Education
sub PRODUCT_EDUCATION_N                      () { 0x7A } # Windows 10 Education N
#define PRODUCT_IOTUAP                              0x0000007B
#define PRODUCT_CLOUD_HOST_INFRASTRUCTURE_SERVER    0x0000007C
#define PRODUCT_ENTERPRISE_S                        0x0000007D
#define PRODUCT_ENTERPRISE_S_N                      0x0000007E
#define PRODUCT_PROFESSIONAL_S                      0x0000007F
#define PRODUCT_PROFESSIONAL_S_N                    0x00000080
#define PRODUCT_ENTERPRISE_S_EVALUATION             0x00000081
#define PRODUCT_ENTERPRISE_S_N_EVALUATION           0x00000082

sub PRODUCT_UNLICENSED                       () { 0xABCDABCD } # product has not been activated and is no longer in the grace period

sub PROCESSOR_ARCHITECTURE_ARM64   ()   { 12 }     # ARM64
sub PROCESSOR_ARCHITECTURE_ARM     ()   { 5 }      # ARM
sub PROCESSOR_ARCHITECTURE_AMD64   ()   { 9 }      # x64 (AMD or Intel)
sub PROCESSOR_ARCHITECTURE_IA64    ()   { 6 }      # Intel Itanium Processor Family (IPF)
sub PROCESSOR_ARCHITECTURE_INTEL   ()   { 0 }      # x86
sub PROCESSOR_ARCHITECTURE_UNKNOWN ()   { 0xffff } # Unknown architecture.

sub _GetProcessorArchitecture {
    my $arch = {
	 386 => PROCESSOR_ARCHITECTURE_INTEL,
	 486 => PROCESSOR_ARCHITECTURE_INTEL,
	 586 => PROCESSOR_ARCHITECTURE_INTEL,
	2200 => PROCESSOR_ARCHITECTURE_IA64,
	8664 => PROCESSOR_ARCHITECTURE_AMD64,
    }->{Win32::GetChipName()};

    if (!defined($arch)) {
        $arch = {
            5 => PROCESSOR_ARCHITECTURE_ARM,
            12 => PROCESSOR_ARCHITECTURE_ARM64,
        }->{Win32::GetChipArch()};
    }

    return defined($arch) ? $arch : PROCESSOR_ARCHITECTURE_UNKNOWN;
}

### This method is just a simple interface into GetOSVersion().  More
### specific or demanding situations should use that instead.

my ($cached_os, $cached_desc);

sub GetOSName {
    unless (defined $cached_os) {
	my($desc, $major, $minor, $build, $id, undef, undef, $suitemask, $producttype)
	    = Win32::GetOSVersion();
	my $arch = _GetProcessorArchitecture();
	my $productinfo = Win32::GetProductInfo(6, 0, 0, 0);
	($cached_os, $cached_desc) = _GetOSName($desc, $major, $minor, $build, $id,
						$suitemask, $producttype, $productinfo, $arch);
    }
    return wantarray ? ($cached_os, $cached_desc) : $cached_os;
}

sub GetOSDisplayName {
    # Calling GetOSDisplayName() with arguments is for the test suite only!
    my($name,$desc) = @_ ? @_ : GetOSName();
    $name =~ s/^Win//;
    if ($desc =~ /^Windows Home Server\b/ || $desc =~ /^Windows XP Professional x64 Edition\b/) {
	($name, $desc) = ($desc, "");
    }
    elsif ($desc =~ s/\s*(Windows (.*) Server( \d+)?)//) {
	$name = "$1 $name";
	$desc =~ s/^\s+//;
    }
    else {
	for ($name) {
	    s/^/Windows / unless /^Win32s$/;
	    s/\/.Net//;
	    s/NT(\d)/NT $1/;
	    if ($desc =~ s/\s*(HPC|Small Business|Web) Server//) {
		my $name = $1;
		$desc =~ s/^\s*//;
		s/(200.)/$name Server $1/;
	    }
	    s/^Windows (20(03|08|12|16|19))/Windows Server $1/;
            s/^Windows SAC/Windows Server/;
	}
    }
    $name .= " $desc" if length $desc;
    return $name;
}

sub _GetSystemMetrics {
    my($index,$metrics) = @_;
    return Win32::GetSystemMetrics($index) unless ref $metrics;
    return $metrics->{$index} if ref $metrics eq "HASH" && defined $metrics->{$index};
    return 1 if ref $metrics eq "ARRAY" && grep $_ == $index, @$metrics;
    return 0;
}

sub _GetOSName {
    # The $metrics argument only exists for the benefit of t/GetOSName.t
    my($csd, $major, $minor, $build, $id, $suitemask, $producttype, $productinfo, $arch, $metrics) = @_;

    my($os,@tags);
    my $desc = "";
    if ($id == 0) {
	$os = "Win32s";
    }
    elsif ($id == 1) {
	if ($minor == 0) {
	    $os = "95";
	}
	elsif ($minor == 10) {
	    $os = "98";
	}
	elsif ($minor == 90) {
	    $os = "Me";
	}
    }
    elsif ($id == 2) {
	if ($major == 3) {
	    $os = "NT3.51";
	}
	elsif ($major == 4) {
	    $os = "NT4";
	}
	elsif ($major == 5) {
	    if ($minor == 0) {
		$os = "2000";
		if ($producttype == VER_NT_WORKSTATION) {
		    $desc = "Professional";
		}
		else {
		    if ($suitemask & VER_SUITE_DATACENTER) {
			$desc = "Datacenter Server";
		    }
		    elsif ($suitemask & VER_SUITE_ENTERPRISE) {
			$desc = "Advanced Server";
		    }
		    elsif ($suitemask & VER_SUITE_SMALLBUSINESS_RESTRICTED) {
			$desc = "Small Business Server";
		    }
		    else {
			$desc = "Server";
		    }
		}
		# XXX ignoring "Windows 2000 Advanced Server Limited Edition" for Itanium
		# XXX and "Windows 2000 Datacenter Server Limited Edition" for Itanium
	    }
	    elsif ($minor == 1) {
		$os = "XP/.Net";
		if (_GetSystemMetrics(SM_MEDIACENTER, $metrics)) {
		    $desc = "Media Center Edition";
		}
		elsif (_GetSystemMetrics(SM_TABLETPC, $metrics)) {
		    # Tablet PC Edition is based on XP Pro
		    $desc = "Tablet PC Edition";
		}
		elsif (_GetSystemMetrics(SM_STARTER, $metrics)) {
		    $desc = "Starter Edition";
		}
		elsif ($suitemask & VER_SUITE_PERSONAL) {
		    $desc = "Home Edition";
		}
		else {
		    $desc = "Professional";
		}
		# XXX ignoring all Windows XP Embedded and Fundamentals versions
	    }
	    elsif ($minor == 2) {
		$os = "2003";

		if (_GetSystemMetrics(SM_SERVERR2, $metrics)) {
		    # XXX R2 was released for all x86 and x64 versions,
		    # XXX but only Enterprise Edition for Itanium.
		    $desc = "R2";
		}

		if ($suitemask == VER_SUITE_STORAGE_SERVER) {
		    $desc .= " Windows Storage Server";
		}
		elsif ($suitemask == VER_SUITE_WH_SERVER) {
		    $desc .= " Windows Home Server";
		}
		elsif ($producttype == VER_NT_WORKSTATION && $arch == PROCESSOR_ARCHITECTURE_AMD64) {
		    $desc .= " Windows XP Professional x64 Edition";
		}

		# Test for the server type.
		if ($producttype != VER_NT_WORKSTATION) {
		    if ($arch == PROCESSOR_ARCHITECTURE_IA64) {
			if ($suitemask & VER_SUITE_DATACENTER) {
			    $desc .= " Datacenter Edition for Itanium-based Systems";
			}
			elsif ($suitemask & VER_SUITE_ENTERPRISE) {
			    $desc .= " Enterprise Edition for Itanium-based Systems";
			}
		    }
		    elsif ($arch == PROCESSOR_ARCHITECTURE_AMD64) {
			if ($suitemask & VER_SUITE_DATACENTER) {
			    $desc .= " Datacenter x64 Edition";
			}
			elsif ($suitemask & VER_SUITE_ENTERPRISE) {
			    $desc .= " Enterprise x64 Edition";
			}
			else {
			    $desc .= " Standard x64 Edition";
			}
		    }
		    else {
			if ($suitemask & VER_SUITE_COMPUTE_SERVER) {
			    $desc .= " Windows Compute Cluster Server";
			}
			elsif ($suitemask & VER_SUITE_DATACENTER) {
			    $desc .= " Datacenter Edition";
			}
			elsif ($suitemask & VER_SUITE_ENTERPRISE) {
			    $desc .= " Enterprise Edition";
			}
			elsif ($suitemask & VER_SUITE_BLADE) {
			    $desc .= " Web Edition";
			}
			elsif ($suitemask & VER_SUITE_SMALLBUSINESS_RESTRICTED) {
			    $desc .= " Small Business Server";
			}
			else {
			    if ($desc !~ /Windows (Home|Storage) Server/) {
				$desc .= " Standard Edition";
			    }
			}
		    }
		}
	    }
	}
	elsif ($major == 6) {
	    if ($minor == 0) {
		if ($producttype == VER_NT_WORKSTATION) {
		    $os = "Vista";
		}
		else {
		    $os = "2008";
		}
	    }
	    elsif ($minor == 1) {
		if ($producttype == VER_NT_WORKSTATION) {
		    $os = "7";
		}
		else {
		    $os = "2008";
		    $desc = "R2";
		}
	    }
	    elsif ($minor == 2) {
                if ($producttype == VER_NT_WORKSTATION) {
                    $os = "8";
                }
                else {
                    $os = "2012";
                }
	    }
	    elsif ($minor == 3) {
		if ($producttype == VER_NT_WORKSTATION) {
		    $os = "8.1";
		}
		else {
		    $os = "2012";
		    $desc = "R2";
		}
	    }
        }
	elsif ($major == 10) {
            if ($producttype == VER_NT_WORKSTATION) {
                # Build numbers from https://en.wikipedia.org/wiki/Windows_10_version_history
                $os = '10';
                if (9841 <= $build && $build <= 10240) {
                    $desc = " Version 1507";
                    $desc .= " (Preview Build $build)" if $build < 10240;
                    $desc .= " (RTM)" if $build == 10240;
                }
                elsif (10525 <= $build && $build <= 10586) {
                    $desc = " Version 1511 (November Update)";
                    $desc .= " (Preview Build $build)" if $build < 10586;
                }
                elsif (11082 <= $build && $build <= 14393) {
                    $desc = " Version 1607 (Anniversary Update)";
                    $desc .= " (Preview Build $build)" if $build < 14393;
                }
                elsif (14901 <= $build && $build <= 15063) {
                    $desc = " Version 1703 (Creators Update)";
                    $desc .= " (Preview Build $build)" if $build < 15063;
                }
                elsif (16170 <= $build && $build <= 16299) {
                    $desc = " Version 1709 (Fall Creators Update)";
                    $desc .= " (Preview Build $build)" if $build < 16299;
                }
                elsif (16353 <= $build && $build <= 17134) {
                    $desc = " Version 1803 (April 2018 Update)";
                    $desc .= " (Preview Build $build)" if $build < 17134;
                }
                elsif (17604 <= $build && $build <= 17763) {
                    $desc = " Version 1809 (October 2018 Update)";
                    $desc .= " (Preview Build $build)" if $build < 17763;
                }
                elsif (18204 <= $build && $build <= 18362) {
                    $desc = " Version 1903 (May 2019 Update)";
                    $desc .= " (Preview Build $build)" if $build < 18362;
                }
                else {
                    $desc = " Build $build";
                }
            }
            else {
                if ($build == 14393) {
                    $os = "2016";
                    $desc = "Version 1607";
                }
                elsif ($build == 17763) {
                    $os = "2019";
                    $desc = "Version 1809";
                }
                else {
                    $os = "Server";
                    if ($build == 16299) {
                        $desc = "Version 1709";
                    }
                    elsif ($build == 17134) {
                        $desc = "Version 1803";
                    }
                    elsif ($build == 18362) {
                        $desc = "Version 1903";
                    }
                    else {
                        $desc = "Build $build";
                    }
                }
            }
        }

        if ($major >= 6) {
            if ($major == 6) {
                if ($productinfo == PRODUCT_ULTIMATE) {
                    $desc .= " Ultimate";
                }
                elsif ($productinfo == PRODUCT_HOME_PREMIUM) {
                    $desc .= " Home Premium";
                }
                elsif ($productinfo == PRODUCT_HOME_BASIC) {
                    $desc .= " Home Basic";
                }
                elsif ($productinfo == PRODUCT_ENTERPRISE) {
                    $desc .= " Enterprise";
                }
                elsif ($productinfo == PRODUCT_BUSINESS) {
                    # "Windows 7 Business" had a name change to "Windows 7 Professional"
                    $desc .= $minor == 0 ? " Business" : " Professional";
                }
                elsif ($productinfo == PRODUCT_STARTER) {
                    $desc .= " Starter";
                }
                elsif ($productinfo == PRODUCT_CLUSTER_SERVER) {
                    $desc .= " HPC Server";
                }
                elsif ($productinfo == PRODUCT_DATACENTER_SERVER) {
                    $desc .= " Datacenter";
                }
                elsif ($productinfo == PRODUCT_DATACENTER_SERVER_CORE) {
                    $desc .= " Datacenter Edition (core installation)";
                }
                elsif ($productinfo == PRODUCT_ENTERPRISE_SERVER) {
                    $desc .= " Enterprise";
                }
                elsif ($productinfo == PRODUCT_ENTERPRISE_SERVER_CORE) {
                    $desc .= " Enterprise Edition (core installation)";
                }
                elsif ($productinfo == PRODUCT_ENTERPRISE_SERVER_IA64) {
                    $desc .= " Enterprise Edition for Itanium-based Systems";
                }
                elsif ($productinfo == PRODUCT_SMALLBUSINESS_SERVER) {
                    $desc .= " Small Business Server";
                }
                elsif ($productinfo == PRODUCT_SMALLBUSINESS_SERVER_PREMIUM) {
                    $desc .= " Small Business Server Premium Edition";
                }
                elsif ($productinfo == PRODUCT_STANDARD_SERVER) {
                    $desc .= " Standard";
                }
                elsif ($productinfo == PRODUCT_STANDARD_SERVER_CORE) {
                    $desc .= " Standard Edition (core installation)";
                }
                elsif ($productinfo == PRODUCT_WEB_SERVER) {
                    $desc .= " Web Server";
                }
                elsif ($productinfo == PRODUCT_PROFESSIONAL) {
                    $desc .= " Professional";
                }
            }

	    if ($arch == PROCESSOR_ARCHITECTURE_INTEL) {
		$desc .= " (32-bit)";
	    }
	    elsif ($arch == PROCESSOR_ARCHITECTURE_AMD64) {
		$desc .= " (64-bit)";
	    }
	}
    }

    unless (defined $os) {
	warn "Unknown Windows version [$id:$major:$minor]";
	return;
    }

    for ($desc) {
	s/\s\s+/ /g;
	s/^\s//;
	s/\s$//;
    }

    # XXX What about "Small Business Server"? NT, 200, 2003, 2008 editions...

    if ($major >= 5) {
	# XXX XP, Vista, 7 all have starter editions
	#push(@tags, "Starter Edition") if _GetSystemMetrics(SM_STARTER, $metrics);
    }

    if (@tags) {
	unshift(@tags, $desc) if length $desc;
	$desc = join(" ", @tags);
    }

    if (length $csd) {
	$desc .= " " if length $desc;
	$desc .= $csd;
    }
    return ("Win$os", $desc);
}

sub IsSymlinkCreationAllowed {
    my(undef, $major, $minor, $build) = GetOSVersion();

    # Vista was the first Windows version with symlink support
    return !!0 if $major < 6;

    # Since Windows 10 1703, enabling the developer mode allows to create
    # symlinks regardless of process privileges
    if ($major > 10 || ($major == 10 && ($minor > 0 || $build > 15063))) {
        return !!1 if IsDeveloperModeEnabled();
    }

    my $privs = GetProcessPrivileges();

    return !!0 unless $privs;

    # It doesn't matter if the permission is enabled or not, it just has to
    # exist. CreateSymbolicLink() will automatically enable it when needed.
    return exists $privs->{SeCreateSymbolicLinkPrivilege};
}

# "no warnings 'redefine';" doesn't work for 5.8.7 and earlier
local $^W = 0;
bootstrap Win32;

1;

__END__

=head1 NAME

Win32 - Interfaces to some Win32 API Functions

=head1 DESCRIPTION

The Win32 module contains functions to access Win32 APIs.

=head2 Alphabetical Listing of Win32 Functions

It is recommended to C<use Win32;> before any of these functions;
however, for backwards compatibility, those marked as [CORE] will
automatically do this for you.

In the function descriptions below the term I<Unicode string> is used
to indicate that the string may contain characters outside the system
codepage.  The caveat I<If supported by the core Perl version>
generally means Perl 5.8.9 and later, though some Unicode pathname
functionality may work on earlier versions.

=over

=item Win32::AbortSystemShutdown(MACHINE)

Aborts a system shutdown (started by the
InitiateSystemShutdown function) on the specified MACHINE.

=item Win32::BuildNumber()

[CORE] Returns the ActivePerl build number.  This function is
only available in the ActivePerl binary distribution.

=item Win32::CopyFile(FROM, TO, OVERWRITE)

[CORE] The Win32::CopyFile() function copies an existing file to a new
file.  All file information like creation time and file attributes will
be copied to the new file.  However it will B<not> copy the security
information.  If the destination file already exists it will only be
overwritten when the OVERWRITE parameter is true.  But even this will
not overwrite a read-only file; you have to unlink() it first
yourself.

=item Win32::CreateDirectory(DIRECTORY)

Creates the DIRECTORY and returns a true value on success.  Check $^E
on failure for extended error information.

DIRECTORY may contain Unicode characters outside the system codepage.
Once the directory has been created you can use
Win32::GetANSIPathName() to get a name that can be passed to system
calls and external programs.

=item Win32::CreateFile(FILE)

Creates the FILE and returns a true value on success.  Check $^E on
failure for extended error information.

FILE may contain Unicode characters outside the system codepage.  Once
the file has been created you can use Win32::GetANSIPathName() to get
a name that can be passed to system calls and external programs.

=item Win32::DomainName()

[CORE] Returns the name of the Microsoft Network domain or workgroup
that the owner of the current perl process is logged into.  The
"Workstation" service must be running to determine this
information.  This function does B<not> work on Windows 9x.

=item Win32::ExpandEnvironmentStrings(STRING)

Takes STRING and replaces all referenced environment variable
names with their defined values.  References to environment variables
take the form C<%VariableName%>.  Case is ignored when looking up the
VariableName in the environment.  If the variable is not found then the
original C<%VariableName%> text is retained.  Has the same effect
as the following:

	$string =~ s/%([^%]*)%/$ENV{$1} || "%$1%"/eg

However, this function may return a Unicode string if the environment
variable being expanded hasn't been assigned to via %ENV.  Access
to %ENV is currently always using byte semantics.

=item Win32::FormatMessage(ERRORCODE)

[CORE] Converts the supplied Win32 error number (e.g. returned by
Win32::GetLastError()) to a descriptive string.  Analogous to the
perror() standard-C library function.  Note that C<$^E> used
in a string context has much the same effect.

	C:\> perl -e "$^E = 26; print $^E;"
	The specified disk or diskette cannot be accessed

=item Win32::FsType()

[CORE] Returns the name of the filesystem of the currently active
drive (like 'FAT' or 'NTFS').  In list context it returns three values:
(FSTYPE, FLAGS, MAXCOMPLEN).  FSTYPE is the filesystem type as
before.  FLAGS is a combination of values of the following table:

	0x00000001  supports case-sensitive filenames
	0x00000002  preserves the case of filenames
	0x00000004  supports Unicode in filenames
	0x00000008  preserves and enforces ACLs
	0x00000010  supports file-based compression
	0x00000020  supports disk quotas
	0x00000040  supports sparse files
	0x00000080  supports reparse points
	0x00000100  supports remote storage
	0x00008000  is a compressed volume (e.g. DoubleSpace)
	0x00010000  supports object identifiers
	0x00020000  supports the Encrypted File System (EFS)

MAXCOMPLEN is the maximum length of a filename component (the part
between two backslashes) on this file system.

=item Win32::FreeLibrary(HANDLE)

Unloads a previously loaded dynamic-link library.  The HANDLE is
no longer valid after this call.  See L<LoadLibrary|Win32::LoadLibrary(LIBNAME)>
for information on dynamically loading a library.

=item Win32::GetACP()

Returns the current Windows ANSI code page identifier for the operating
system.  See also GetOEMCP(), GetConsoleCP() and GetConsoleOutputCP().

=item Win32::GetANSIPathName(FILENAME)

Returns an ANSI version of FILENAME.  This may be the short name
if the long name cannot be represented in the system codepage.

While not currently implemented, it is possible that in the future
this function will convert only parts of the path to FILENAME to a
short form.

If FILENAME doesn't exist on the filesystem, or if the filesystem
doesn't support short ANSI filenames, then this function will
translate the Unicode name into the system codepage using replacement
characters.

=item Win32::GetArchName()

Use of this function is deprecated.  It is equivalent with
$ENV{PROCESSOR_ARCHITECTURE}.  This might not work on Win9X.

=item Win32::GetChipName()

Returns the processor type: 386, 486 or 586 for x86 processors, 8664 for the x64
processor and 2200 for the Itanium. For arm/arm64 processor, the value is marked
as "Reserved" (not specified, but usually 0) in Microsoft documentation, so it's
better to use GetChipArch(). Since it returns the native processor type it will
return a 64-bit processor type even when called from a 32-bit Perl running on
64-bit Windows.

=item Win32::GetChipArch()

Returns the processor architecture: 0 for x86 processors, 5 for arm, 6 for
Itanium, 9 for x64 and 12 for arm64, and 0xFFFF for unknown architecture.

=item Win32::GetConsoleCP()

Returns the input code page used by the console associated with the
calling process.  To set the console's input code page, see
SetConsoleCP().  See also GetConsoleOutputCP(), GetACP() and
GetOEMCP().

=item Win32::GetConsoleOutputCP()

Returns the output code page used by the console associated with the
calling process.  To set the console's output code page, see
SetConsoleOutputCP().  See also GetConsoleCP(), GetACP(), and
GetOEMCP().

=item Win32::GetCwd()

[CORE] Returns the current active drive and directory.  This function
does not return a UNC path, since the functionality required for such
a feature is not available under Windows 95.

If supported by the core Perl version, this function will return an
ANSI path name for the current directory if the long pathname cannot
be represented in the system codepage.

=item Win32::GetCurrentProcessId()

Returns the process identifier of the current process.  Until the
process terminates, the process identifier uniquely identifies the
process throughout the system.

The current process identifier is normally also available via the
predefined $$ variable.  Under fork() emulation however $$ may contain
a pseudo-process identifier that is only meaningful to the Perl
kill(), wait() and waitpid() functions.  The
Win32::GetCurrentProcessId() function will always return the regular
Windows process id, even when called from inside a pseudo-process.

=item Win32::GetCurrentThreadId()

Returns the thread identifier of the calling thread.  Until the thread
terminates, the thread identifier uniquely identifies the thread
throughout the system.

=item Win32::GetFileVersion(FILENAME)

Returns the file version number from the VERSIONINFO resource of
the executable file or DLL.  This is a tuple of four 16 bit numbers.
In list context these four numbers will be returned.  In scalar context
they are concatenated into a string, separated by dots.

=item Win32::GetFolderPath(FOLDER [, CREATE])

Returns the full pathname of one of the Windows special folders.
The folder will be created if it doesn't exist and the optional CREATE
argument is true.  The following FOLDER constants are defined by the
Win32 module, but only exported on demand:

        CSIDL_ADMINTOOLS
        CSIDL_APPDATA
        CSIDL_CDBURN_AREA
        CSIDL_COMMON_ADMINTOOLS
        CSIDL_COMMON_APPDATA
        CSIDL_COMMON_DESKTOPDIRECTORY
        CSIDL_COMMON_DOCUMENTS
        CSIDL_COMMON_FAVORITES
        CSIDL_COMMON_MUSIC
        CSIDL_COMMON_PICTURES
        CSIDL_COMMON_PROGRAMS
        CSIDL_COMMON_STARTMENU
        CSIDL_COMMON_STARTUP
        CSIDL_COMMON_TEMPLATES
        CSIDL_COMMON_VIDEO
        CSIDL_COOKIES
        CSIDL_DESKTOP
        CSIDL_DESKTOPDIRECTORY
        CSIDL_FAVORITES
        CSIDL_FONTS
        CSIDL_HISTORY
        CSIDL_INTERNET_CACHE
        CSIDL_LOCAL_APPDATA
        CSIDL_MYMUSIC
        CSIDL_MYPICTURES
        CSIDL_MYVIDEO
        CSIDL_NETHOOD
        CSIDL_PERSONAL
        CSIDL_PRINTHOOD
        CSIDL_PROFILE
        CSIDL_PROGRAMS
        CSIDL_PROGRAM_FILES
        CSIDL_PROGRAM_FILES_COMMON
        CSIDL_RECENT
        CSIDL_RESOURCES
        CSIDL_RESOURCES_LOCALIZED
        CSIDL_SENDTO
        CSIDL_STARTMENU
        CSIDL_STARTUP
        CSIDL_SYSTEM
        CSIDL_TEMPLATES
        CSIDL_WINDOWS

Note that not all folders are defined on all versions of Windows.

Please refer to the MSDN documentation of the CSIDL constants,
currently available at:

http://msdn.microsoft.com/library/default.asp?url=/library/en-us/shellcc/platform/shell/reference/enums/csidl.asp

This function will return an ANSI folder path if the long name cannot
be represented in the system codepage.  Use Win32::GetLongPathName()
on the result of Win32::GetFolderPath() if you want the Unicode
version of the folder name.

=item Win32::GetFullPathName(FILENAME)

[CORE] GetFullPathName combines the FILENAME with the current drive
and directory name and returns a fully qualified (aka, absolute)
path name.  In list context it returns two elements: (PATH, FILE) where
PATH is the complete pathname component (including trailing backslash)
and FILE is just the filename part.  Note that no attempt is made to
convert 8.3 components in the supplied FILENAME to longnames or
vice-versa.  Compare with Win32::GetShortPathName() and
Win32::GetLongPathName().

If supported by the core Perl version, this function will return an
ANSI path name if the full pathname cannot be represented in the
system codepage.

=item Win32::GetLastError()

[CORE] Returns the last error value generated by a call to a Win32 API
function.  Note that C<$^E> used in a numeric context amounts to the
same value.

=item Win32::GetLongPathName(PATHNAME)

[CORE] Returns a representation of PATHNAME composed of longname
components (if any).  The result may not necessarily be longer
than PATHNAME.  No attempt is made to convert PATHNAME to the
absolute path.  Compare with Win32::GetShortPathName() and
Win32::GetFullPathName().

This function may return the pathname in Unicode if it cannot be
represented in the system codepage.  Use Win32::GetANSIPathName()
before passing the path to a system call or another program.

=item Win32::GetNextAvailDrive()

[CORE] Returns a string in the form of "<d>:" where <d> is the first
available drive letter.

=item Win32::GetOEMCP()

Returns the current original equipment manufacturer (OEM) code page
identifier for the operating system.  See also GetACP(), GetConsoleCP()
and GetConsoleOutputCP().

=item Win32::GetOSDisplayName()

Returns the "marketing" name of the Windows operating system version
being used.  It returns names like these (random samples):

   Windows 2000 Datacenter Server
   Windows XP Professional
   Windows XP Tablet PC Edition
   Windows Home Server
   Windows Server 2003 Enterprise Edition for Itanium-based Systems
   Windows Vista Ultimate (32-bit)
   Windows Small Business Server 2008 R2 (64-bit)

The display name describes the native Windows version, so even on a
32-bit Perl this function may return a "Windows ... (64-bit)" name
when running on a 64-bit Windows.

This function should only be used to display the actual OS name to the
user; it should not be used to determine the class of operating systems
this system belongs to.  The Win32::GetOSName(), Win32::GetOSVersion,
Win32::GetProductInfo() and Win32::GetSystemMetrics() functions provide
the base information to check for certain capabilities, or for families
of OS releases.

=item Win32::GetOSName()

In scalar context returns the name of the Win32 operating system
being used.  In list context returns a two element list of the OS name
and whatever edition information is known about the particular build
(for Win9X boxes) and whatever service packs have been installed.
The latter is roughly equivalent to the first item returned by
GetOSVersion() in list context.

The description will also include tags for other special editions,
like "R2", "Media Center", "Tablet PC", or "Starter Edition".

In the Windows 10 / Server Semi-Annual Channel era, the description may
contain the relevant ReleaseId value, but this is only inferred from
the build number, not determined absolutely.

Currently the possible values for the OS name are

    WinWin32s
    Win95
    Win98
    WinMe
    WinNT3.51
    WinNT4
    Win2000
    WinXP/.Net
    Win2003
    WinHomeSvr
    WinVista
    Win2008
    Win7
    Win8
    Win8.1
    Win10
    Win2016
    Win2019
    WinSAC

This routine is just a simple interface into GetOSVersion().  More
specific or demanding situations should use that instead.  Another
option would be to use POSIX::uname(), however the latter appears to
report only the OS family name and not the specific OS.  In scalar
context it returns just the ID.

The name "WinXP/.Net" is used for historical reasons only, to maintain
backwards compatibility of the Win32 module.  Windows .NET Server has
been renamed as Windows 2003 Server before final release and uses a
different major/minor version number than Windows XP.

Similarly the name "WinWin32s" should have been "Win32s" but has been
kept as-is for backwards compatibility reasons too.

=item Win32::GetOSVersion()

[CORE] Returns the list (STRING, MAJOR, MINOR, BUILD, ID), where the
elements are, respectively: An arbitrary descriptive string, the major
version number of the operating system, the minor version number, the
build number, and a digit indicating the actual operating system.
For the ID, the values are 0 for Win32s, 1 for Windows 9X/Me and 2 for
Windows NT/2000/XP/2003/Vista/2008/7.  In scalar context it returns just
the ID.

Currently known values for ID MAJOR MINOR and BUILD are as follows:

    OS                      ID    MAJOR   MINOR   BUILD
    Win32s                   0      -       -       -
    Windows 95               1      4       0       -
    Windows 98               1      4      10       -
    Windows Me               1      4      90       -

    Windows NT 3.51          2      3      51       -
    Windows NT 4             2      4       0       -

    Windows 2000             2      5       0       -
    Windows XP               2      5       1       -
    Windows Server 2003      2      5       2       -
    Windows Server 2003 R2   2      5       2       -
    Windows Home Server      2      5       2       -

    Windows Vista            2      6       0       -
    Windows Server 2008      2      6       0       -
    Windows 7                2      6       1       -
    Windows Server 2008 R2   2      6       1       -
    Windows 8                2      6       2       -
    Windows Server 2012      2      6       2       -
    Windows 8.1              2      6       2       -
    Windows Server 2012 R2   2      6       2       -
    
    Windows 10               2     10       0       -
    Windows Server 2016      2     10       0   14393
    Windows Server 2019      2     10       0   17677
    
On Windows NT 4 SP6 and later this function returns the following
additional values: SPMAJOR, SPMINOR, SUITEMASK, PRODUCTTYPE.

The version numbers for Windows 2003 and Windows Home Server are
identical; the SUITEMASK field must be used to differentiate between
them.

The version numbers for Windows Vista and Windows Server 2008 are
identical; the PRODUCTTYPE field must be used to differentiate between
them.

The version numbers for Windows 7 and Windows Server 2008 R2 are
identical; the PRODUCTTYPE field must be used to differentiate between
them.

The version numbers for Windows 8 and Windows Server 2012 are
identical; the PRODUCTTYPE field must be used to differentiate between
them.

For modern Windows releases, the major and minor version numbers are
identical. The PRODUCTTYPE field must be used to differentiate between
Windows 10 and Server releases. The BUILD field is used to
differentiate Windows Server versions: currently 2016, 2019, and
Semi-Annual Channel releases.

SPMAJOR and SPMINOR are the version numbers of the latest
installed service pack. (In the Windows 10 era, these are unused.)

SUITEMASK is a bitfield identifying the product suites available on
the system.  Known bits are:

    VER_SUITE_SMALLBUSINESS             0x00000001
    VER_SUITE_ENTERPRISE                0x00000002
    VER_SUITE_BACKOFFICE                0x00000004
    VER_SUITE_COMMUNICATIONS            0x00000008
    VER_SUITE_TERMINAL                  0x00000010
    VER_SUITE_SMALLBUSINESS_RESTRICTED  0x00000020
    VER_SUITE_EMBEDDEDNT                0x00000040
    VER_SUITE_DATACENTER                0x00000080
    VER_SUITE_SINGLEUSERTS              0x00000100
    VER_SUITE_PERSONAL                  0x00000200
    VER_SUITE_BLADE                     0x00000400
    VER_SUITE_EMBEDDED_RESTRICTED       0x00000800
    VER_SUITE_SECURITY_APPLIANCE        0x00001000
    VER_SUITE_STORAGE_SERVER            0x00002000
    VER_SUITE_COMPUTE_SERVER            0x00004000
    VER_SUITE_WH_SERVER                 0x00008000
    VER_SUITE_MULTIUSERTS               0x00020000

The VER_SUITE_xxx names are listed here to cross reference the Microsoft
documentation.  The Win32 module does not provide symbolic names for these
constants.

PRODUCTTYPE provides additional information about the system.  It should
be one of the following integer values:

    1 - Workstation (NT 4, 2000 Pro, XP Home, XP Pro, Vista, etc)
    2 - Domaincontroller
    3 - Server (2000 Server, Server 2003, Server 2008, etc)

Note that a server that is also a domain controller is reported as
PRODUCTTYPE 2 (Domaincontroller) and not PRODUCTTYPE 3 (Server).

=item Win32::GetShortPathName(PATHNAME)

[CORE] Returns a representation of PATHNAME that is composed of short
(8.3) path components where available.  For path components where the
file system has not generated the short form the returned path will
use the long form, so this function might still for instance return a
path containing spaces.  Returns C<undef> when the PATHNAME does not
exist. Compare with Win32::GetFullPathName() and
Win32::GetLongPathName().

=item Win32::GetSystemMetrics(INDEX)

Retrieves the specified system metric or system configuration setting.
Please refer to the Microsoft documentation of the GetSystemMetrics()
function for a reference of available INDEX values.  All system
metrics return integer values.

=item Win32::GetProcAddress(INSTANCE, PROCNAME)

Returns the address of a function inside a loaded library.  The
information about what you can do with this address has been lost in
the mist of time.  Use the Win32::API module instead of this deprecated
function.

=item Win32::GetProcessPrivileges([PID])

Returns a reference to a hash holding the information about the privileges
held by the specified process. The keys are privilege names, and the values
are booleans indicating whether a given privilege is currently enabled or not.

If the optional PID parameter is omitted, the function queries the current
process.

Example return value:

    {
        SeTimeZonePrivilege => 0,
        SeShutdownPrivilege => 0,
        SeUndockPrivilege => 0,
        SeIncreaseWorkingSetPrivilege => 0,
        SeChangeNotifyPrivilege => 1
    }

=item Win32::GetProductInfo(OSMAJOR, OSMINOR, SPMAJOR, SPMINOR)

Retrieves the product type for the operating system on the local
computer, and maps the type to the product types supported by the
specified operating system.  Please refer to the Microsoft
documentation of the GetProductInfo() function for more information
about the parameters and return value.  This function requires Windows
Vista or later.

See also the Win32::GetOSName() and Win32::GetOSDisplayName()
functions which provide a higher level abstraction of the data
returned by this function.

=item Win32::GetTickCount()

[CORE] Returns the number of milliseconds elapsed since the last
system boot.  Resolution is limited to system timer ticks (about 10ms
on WinNT and 55ms on Win9X).

=item Win32::GuidGen()

Creates a globally unique 128 bit integer that can be used as a
persistent identifier in a distributed setting. To a very high degree
of certainty this function returns a unique value. No other
invocation, on the same or any other system (networked or not), should
return the same value.

The return value is formatted according to OLE conventions, as groups
of hex digits with surrounding braces.  For example:

    {09531CF1-D0C7-4860-840C-1C8C8735E2AD}

=item Win32::HttpGetFile(URL, FILENAME [, IGNORE_CERT_ERRORS])

Uses the WinHttp library to download the file specified by the URL
parameter to the local file specified by FILENAME. The optional third
parameter, if true, indicates that certficate errors are to be ignored
for https connections; please use with caution in a safe environment,
such as when testing locally using a self-signed certificate.

Only http and https protocols are supported.  Authentication is not
supported.  The function is not available when building with gcc prior to
4.8.0 because the WinHttp library is not available.

In scalar context returns a boolean success or failure, and in list
context also returns, in addition to the boolean status, a second
value containing message text related to the status.

If the call fails, C<Win32::GetLastError()> will return a numeric
error code, which may be a system error, a WinHttp error, or a
user-defined error composed of 1e9 plus the HTTP status code.

Scalar context example:

    print Win32::GetLastError()
        unless Win32::HttpGetFile('http://example.com/somefile.tar.gz',
                                  '.\file.tgz');

List context example:

    my ($ok, $msg) = Win32::HttpGetFile('http://example.com/somefile.tar.gz',
                                        '.\file.tgz');
    if ($ok) {
        print "Success!: $msg\n";
    }
    else {
        print "Failure!: $msg\n";
        my $err = Win32::GetLastError();
        if ($err > 1e9) {
            printf "HTTP status: %d\n", ($err - 1e9);
        }
    }

=item Win32::InitiateSystemShutdown

(MACHINE, MESSAGE, TIMEOUT, FORCECLOSE, REBOOT)

Shuts down the specified MACHINE, notifying users with the
supplied MESSAGE, within the specified TIMEOUT interval.  Forces
closing of all documents without prompting the user if FORCECLOSE is
true, and reboots the machine if REBOOT is true.  This function works
only on WinNT.

=item Win32::IsAdminUser()

Returns non zero if the account in whose security context the
current process/thread is running belongs to the local group of
Administrators in the built-in system domain; returns 0 if not.
On Windows Vista it will only return non-zero if the process is
actually running with elevated privileges.  Returns C<undef>
and prints a warning if an error occurred.  This function always
returns 1 on Win9X.

=item Win32::IsDeveloperModeEnabled()

Returns true if the developer mode is currently enabled. It always returns
false on Windows versions older than Windows 10.

=item Win32::IsSymlinkCreationAllowed()

Returns true if the current process is allowed to create symbolic links. This
function is a convenience wrapper around Win32::GetProcessPrivileges() and
Win32::IsDeveloperModeEnabled().

=item Win32::IsWinNT()

[CORE] Returns non zero if the Win32 subsystem is Windows NT.

=item Win32::IsWin95()

[CORE] Returns non zero if the Win32 subsystem is Windows 95.

=item Win32::LoadLibrary(LIBNAME)

Loads a dynamic link library into memory and returns its module
handle.  This handle can be used with Win32::GetProcAddress() and
Win32::FreeLibrary().  This function is deprecated.  Use the Win32::API
module instead.

=item Win32::LoginName()

[CORE] Returns the username of the owner of the current perl process.
The return value may be a Unicode string.

=item Win32::LookupAccountName(SYSTEM, ACCOUNT, DOMAIN, SID, SIDTYPE)

Looks up ACCOUNT on SYSTEM and returns the domain name the SID and
the SID type.

=item Win32::LookupAccountSID(SYSTEM, SID, ACCOUNT, DOMAIN, SIDTYPE)

Looks up SID on SYSTEM and returns the account name, domain name,
and the SID type.

=item Win32::MsgBox(MESSAGE [, FLAGS [, TITLE]])

Create a dialog box containing MESSAGE.  FLAGS specifies the
required icon and buttons according to the following table:

	0 = OK
	1 = OK and Cancel
	2 = Abort, Retry, and Ignore
	3 = Yes, No and Cancel
	4 = Yes and No
	5 = Retry and Cancel

	MB_ICONSTOP          "X" in a red circle
	MB_ICONQUESTION      question mark in a bubble
	MB_ICONEXCLAMATION   exclamation mark in a yellow triangle
	MB_ICONINFORMATION   "i" in a bubble

TITLE specifies an optional window title.  The default is "Perl".

The function returns the menu id of the selected push button:

	0  Error

	1  OK
	2  Cancel
	3  Abort
	4  Retry
	5  Ignore
	6  Yes
	7  No

=item Win32::NodeName()

[CORE] Returns the Microsoft Network node-name of the current machine.

=item Win32::OutputDebugString(STRING)

Sends a string to the application or system debugger for display.
The function does nothing if there is no active debugger.

Alternatively one can use the I<Debug Viewer> application to
watch the OutputDebugString() output:

http://www.microsoft.com/technet/sysinternals/utilities/debugview.mspx

=item Win32::RegisterServer(LIBRARYNAME)

Loads the DLL LIBRARYNAME and calls the function DllRegisterServer.

=item Win32::SetChildShowWindow(SHOWWINDOW)

[CORE] Sets the I<ShowMode> of child processes started by system().
By default system() will create a new console window for child
processes if Perl itself is not running from a console.  Calling
SetChildShowWindow(0) will make these new console windows invisible.
Calling SetChildShowWindow() without arguments reverts system() to the
default behavior.  The return value of SetChildShowWindow() is the
previous setting or C<undef>.

The following symbolic constants for SHOWWINDOW are available
(but not exported) from the Win32 module: SW_HIDE, SW_SHOWNORMAL,
SW_SHOWMINIMIZED, SW_SHOWMAXIMIZED and SW_SHOWNOACTIVATE.

=item Win32::SetConsoleCP(ID)

Sets the input code page used by the console associated with the
calling process.  The return value of SetConsoleCP() is nonzero on
success or zero on failure.  To get the console's input code page, see
GetConsoleCP().

=item Win32::SetConsoleOutputCP(ID)

Sets the output code page used by the console associated with the
calling process.  The return value of SetConsoleOutputCP() is nonzero on
success or zero on failure.  To get the console's output code page, see
GetConsoleOutputCP().

=item Win32::SetCwd(NEWDIRECTORY)

[CORE] Sets the current active drive and directory.  This function does not
work with UNC paths, since the functionality required to required for
such a feature is not available under Windows 95.

=item Win32::SetLastError(ERROR)

[CORE] Sets the value of the last error encountered to ERROR.  This is
that value that will be returned by the Win32::GetLastError()
function.

=item Win32::Sleep(TIME)

[CORE] Pauses for TIME milliseconds.  The timeslices are made available
to other processes and threads.

=item Win32::Spawn(COMMAND, ARGS, PID)

[CORE] Spawns a new process using the supplied COMMAND, passing in
arguments in the string ARGS.  The pid of the new process is stored in
PID.  This function is deprecated.  Please use the Win32::Process module
instead.

=item Win32::UnregisterServer(LIBRARYNAME)

Loads the DLL LIBRARYNAME and calls the function
DllUnregisterServer.

=back

=head1 CAVEATS

=head2 Short Path Names

There are many situations in which modern Windows systems will not have
the L<short path name|https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#short-vs-long-names>
(also called 8.3 or MS-DOS) alias for long file names available.

Short path support can be configured system-wide via the registry,
but the default on modern systems is to configure short path usage per
volume. The configuration for a volume can be queried in a number of ways,
but these may either be unreliable or require elevated (administrator)
privileges.

Typically, the configuration for a volume can be queried using the C<fsutil>
utility, e.g. C<fsutil 8dot3name query d:>. On the C level, it can be queried
with a C<FSCTL_QUERY_PERSISTENT_VOLUME_STATE> request to the
C<DeviceIOControl> API call, as described in
L<this article|https://www.codeproject.com/Articles/304374/Query-Volume-Setting-for-State-Windows>.
However, both of these methods require administrator privileges to work.

The Win32 module does not perform any per-volume check and simply fetches
short path names in the same manner as the underlying Windows API call it
uses: If short path names are disabled, the call will still succeed but the
long name will actually be returned.

Note that on volumes where this happens, C<GetANSIPathName> usually cannot be
used to return useful filenames for files that contain unicode characters.
(In code page 65001, this may still work.) Handling unicode filenames in this
legacy manner relies upon C<GetShortPathName> returning 8.3 filenames, but
without short name support, it will return the filename with all unicode
characters replaced by question mark characters.

=cut
