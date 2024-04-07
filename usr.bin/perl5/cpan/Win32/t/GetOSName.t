use strict;
use Test::More;
use Win32;

# The "description" value is extracted from the $pretty field:
#
#     "2000 [Server]" => "Server"
#     "{Home Server}" => "Windows Home Server" (prefixed with "Windows ")
#     "Anything R2"   => "R2 Anything"         (R2 moved to front)
#
# The "display name" value is the same as the $pretty field,
# prefixed by "Windows ", with all "[]{}" characters removed.

# $pretty, $os $id, $major, $minor, $sm, $pt, $metric

my @intel_tests = (
["Win32s",                          "Win32s",  0                     ],

["95",                              "95",      1, 4, 0               ],
["98",                              "98",      1, 4, 10              ],
["Me",                              "Me",      1, 4, 90              ],

["NT 3.51",                         "NT3.51",  2, 3, 51              ],
["NT 4",                            "NT4",     2, 4, 0               ],

["2000 [Professional]",             "2000",    2, 5, 0, 0x0000, 1,  0],
["2000 [Server]",                   "2000",    2, 5, 0, 0x0000, 2,  0],
["[Small Business Server] 2000",    "2000",    2, 5, 0, 0x0020, 2,  0],
["2000 [Advanced Server]",          "2000",    2, 5, 0, 0x0002, 2,  0],
["2000 [Datacenter Server]",        "2000",    2, 5, 0, 0x0080, 2,  0],

["XP [Home Edition]",               "XP/.Net", 2, 5, 1, 0x0200, 1,  0],
["XP [Professional]",               "XP/.Net", 2, 5, 1, 0x0000, 1,  0],
["XP [Tablet PC Edition]",          "XP/.Net", 2, 5, 1, 0x0000, 1, 86],
["XP [Media Center Edition]",       "XP/.Net", 2, 5, 1, 0x0000, 1, 87],
["XP [Starter Edition]",            "XP/.Net", 2, 5, 1, 0x0000, 1, 88],

["2003 [Standard Edition]",         "2003",    2, 5, 2, 0x0000, 2,  0],
["[Small Business Server] 2003",    "2003",    2, 5, 2, 0x0020, 2,  0],
["{Storage Server} 2003",           "2003",    2, 5, 2, 0x2000, 2,  0],
["{Home Server}",                   "2003",    2, 5, 2, 0x8000, 2,  0],

["{Compute Cluster Server} 2003",   "2003",    2, 5, 2, 0x4000, 2,  0],
["2003 [Datacenter Edition]",       "2003",    2, 5, 2, 0x0080, 2,  0],
["2003 [Enterprise Edition]",       "2003",    2, 5, 2, 0x0002, 2,  0],
["2003 [Web Edition]",              "2003",    2, 5, 2, 0x0400, 2,  0],

["2003 [R2 Standard Edition]",      "2003",    2, 5, 2, 0x0000, 2, 89],
["[Small Business Server] 2003 R2", "2003",    2, 5, 2, 0x0020, 2, 89],
["{Storage Server} 2003 R2",        "2003",    2, 5, 2, 0x2000, 2, 89],
# ??? test for more R2 versions?
);

my @amd64_tests = (
["{XP Professional x64 Edition}",   "2003",    2, 5, 2, 0x0000, 1,  0],
["2003 [Datacenter x64 Edition]",   "2003",    2, 5, 2, 0x0080, 2,  0],
["2003 [Enterprise x64 Edition]",   "2003",    2, 5, 2, 0x0002, 2,  0],
["2003 [Standard x64 Edition]",     "2003",    2, 5, 2, 0x0000, 2,  0],
);

my @dual_tests = (
["Vista",                           "Vista",   2, 6, 0               ],

["Vista [Starter]",                 "Vista",   2, 6, 0, 0x0b         ],
["Vista [Home Basic]",              "Vista",   2, 6, 0, 0x02         ],
["Vista [Home Premium]",            "Vista",   2, 6, 0, 0x03         ],
["Vista [Business]",                "Vista",   2, 6, 0, 0x06         ],
["Vista [Enterprise]",              "Vista",   2, 6, 0, 0x04         ],
["Vista [Ultimate]",                "Vista",   2, 6, 0, 0x01         ],

#["Vista Business for Embedded Systems", "Vista",   2, 6, 0           ],
#["Vista Ultimate for Embedded Systems", "Vista",   2, 6, 0           ],

["2008 [Standard]",                 "2008",    2, 6, 0, 0x07, 2      ],
["2008 [Enterprise]",               "2008",    2, 6, 0, 0x04, 2      ],
["[HPC Server] 2008",               "2008",    2, 6, 0, 0x12, 2      ],
["[Web Server] 2008",               "2008",    2, 6, 0, 0x11, 2      ],
#["[Storage Server] 2008",           "2008",    2, 6, 0, ????, 2      ],
["[Small Business Server] 2008",    "2008",    2, 6, 0, 0x09, 2,  0  ],

#    * Windows Server 2008 Standard (x86 and x86-64)
#    * Windows Server 2008 Enterprise (x86 and x86-64)
#    * Windows HPC Server 2008 (replacing Windows Compute Cluster Server 2003)
#    * Windows Web Server 2008 (x86 and x86-64)
#    * Windows Storage Server 2008 (x86 and x86-64)
#    * Windows Small Business Server 2008 (Codenamed "Cougar") (x86-64) for small businesses
#    * Windows Essential Business Server 2008 (Codenamed "Centro") (x86-64) for medium-sized businesses [25]
#    * Windows Server 2008 for Itanium-based Systems
#    * Windows Server 2008 Foundation
#
# Server Core is available in the Web, Standard, Enterprise and Datacenter editions.

["7",                               "7",       2, 6, 1               ],
["7 [Starter]",                     "7",       2, 6, 1, 0x0b         ],
["7 [Home Basic]",                  "7",       2, 6, 1, 0x02         ],
["7 [Home Premium]",                "7",       2, 6, 1, 0x03         ],
["7 [Professional]",                "7",       2, 6, 1, 0x06         ],
["7 [Professional]",                "7",       2, 6, 1, 0x30         ],
["7 [Enterprise]",                  "7",       2, 6, 1, 0x04         ],
["7 [Ultimate]",                    "7",       2, 6, 1, 0x01         ],

["8",                               "8",       2, 6, 2               ],
["2008 [R2 Standard]",              "2008",    2, 6, 1, 0x07, 2, 89  ],
["2012 [Standard]",                 "2012",    2, 6, 2, 0x07, 2, 89  ],
["[Small Business Server] 2008 R2", "2008",    2, 6, 1, 0x09, 2, 89  ],

["8.1",                             "8.1",     2, 6, 3               ],
["2012 [R2]",                       "2012",    2, 6, 3, 0x00, 2, 89  ],
);

my @win10_tests = (
["10 [Build 9840]",                                                "10", 2, 10, 0, 0x00, 0, 0, 9840],

["10 [Version 1507 (Preview Build 9841)]",                         "10", 2, 10, 0, 0x00, 0, 0, 9841],
["10 [Version 1507 (RTM)]",                                        "10", 2, 10, 0, 0x00, 0, 0, 10240],

["10 [Version 1511 (November Update) (Preview Build 10525)]",      "10", 2, 10, 0, 0x00, 0, 0, 10525],
["10 [Version 1511 (November Update)]",                            "10", 2, 10, 0, 0x00, 0, 0, 10586],

["10 [Version 1607 (Anniversary Update) (Preview Build 11082)]",   "10", 2, 10, 0, 0x00, 0, 0, 11082],
["10 [Version 1607 (Anniversary Update)]",                         "10", 2, 10, 0, 0x00, 0, 0, 14393],

["10 [Version 1703 (Creators Update) (Preview Build 14901)]",      "10", 2, 10, 0, 0x00, 0, 0, 14901],
["10 [Version 1703 (Creators Update)]",                            "10", 2, 10, 0, 0x00, 0, 0, 15063],

["10 [Version 1709 (Fall Creators Update) (Preview Build 16170)]", "10", 2, 10, 0, 0x00, 0, 0, 16170],
["10 [Version 1709 (Fall Creators Update)]",                       "10", 2, 10, 0, 0x00, 0, 0, 16299],

["10 [Version 1803 (April 2018 Update) (Preview Build 16353)]",    "10", 2, 10, 0, 0x00, 0, 0, 16353],
["10 [Version 1803 (April 2018 Update)]",                          "10", 2, 10, 0, 0x00, 0, 0, 17134],

["10 [Version 1809 (October 2018 Update) (Preview Build 17604)]",  "10", 2, 10, 0, 0x00, 0, 0, 17604],
["10 [Version 1809 (October 2018 Update)]",                        "10", 2, 10, 0, 0x00, 0, 0, 17763],

["10 [Version 1903 (May 2019 Update) (Preview Build 18204)]",      "10", 2, 10, 0, 0x00, 0, 0, 18204],
["10 [Version 1903 (May 2019 Update)]",                            "10", 2, 10, 0, 0x00, 0, 0, 18362],

["2016 [Version 1607]",                                    "2016",    2, 10, 0, 0x07, 2, 0, 14393],
["2019 [Version 1809]",                                    "2019",    2, 10, 0, 0x07, 2, 0, 17763],

["Server [Version 1709]",                                  "Server",  2, 10, 0, 0x07, 2, 0, 16299],
["Server [Version 1803]",                                  "Server",  2, 10, 0, 0x07, 2, 0, 17134],
# The 1809 version from the semi-annual channel will identify as "Windows Server 2019 Version 1809"
#["Server [Version 1809]",                                 "Server",  2, 10, 0, 0x07, 2, 0, 17763],
["Server [Version 1903]",                                  "Server",  2, 10, 0, 0x07, 2, 0, 18362],
["Server [Build 12345]",                                   "Server",  2, 10, 0, 0x07, 2, 0, 12345],

);

my @ia64_tests = (
["2003 [Datacenter Edition for Itanium-based Systems]", "2003", 2, 5, 2, 0x0080, 2, 0],
["2003 [Enterprise Edition for Itanium-based Systems]", "2003", 2, 5, 2, 0x0002, 2, 0],
);

plan tests => 6 * (@intel_tests + @amd64_tests + 2*@dual_tests + @ia64_tests) + 3 * @win10_tests;

# Test internal implementation function
sub check {
    my($test, $arch) = @_;
    my($pretty, $expect, $id, $major, $minor, $sm, $pt, $metrics, $build) = @$test;
    $metrics = [$metrics] if defined($metrics) && not ref $metrics;

    my $tag = "";
    ($pretty, $tag) = ("$1$2$3", "$2") if $pretty =~ /^(.*)\[(.*)\](.*)$/;
    ($pretty, $tag) = ("$1$2$3", "Windows $2") if $pretty =~ /^(.*)\{(.*)\}(.*)$/;
    $tag = "R2 $tag" if $tag !~ /R2/ && $pretty =~ /R2$/;

    # All display names start with "Windows";
    # and 2003/2008 start with "Windows Server"
    unless ($pretty eq "Win32s") {
	my $prefix = "Windows";
	$prefix .= " Server" if $pretty =~ /^20(03|08|12|16|19)/;
	$pretty = "$prefix $pretty";
    }

    # @dual_tests: Vista and later all come in both 32-bit and 64-bit versions
    if ($id == 2 && $major >= 6) {
	my $suffix = "";
	$suffix = " (32-bit)" if $arch == Win32::PROCESSOR_ARCHITECTURE_INTEL;
	$suffix = " (64-bit)" if $arch == Win32::PROCESSOR_ARCHITECTURE_AMD64;
	$_ .= $suffix for $pretty, $tag;
	$tag =~ s/^\s*//;
    }

    # We pass the same value for $suitemask and $productinfo.  The former is
    # used for Windows up to 2003, the latter is used for Vista and later.
    my($os, $desc) = Win32::_GetOSName("", $major||0, $minor||0, $build,
				       $id, $sm||0, $pt||1, $sm||0, $arch, $metrics);
    my $display = Win32::GetOSDisplayName($os, $desc);

    note($pretty) if defined &note;
    is($display, $pretty);
    is($os, "Win$expect", "os:   $os");
    is($desc, $tag, "desc: $desc");

    next if $major == 10;

    my $sp = "Service Pack 42";
    ($os, $desc) = Win32::_GetOSName($sp, $major||0, $minor||0, 0,
				     $id, $sm||0, $pt||1, $sm||0, $arch, $metrics);
    $display = Win32::GetOSDisplayName($os, $desc);

    is($display, "$pretty $sp", "display: $display");
    is($os,      "Win$expect",  "os:      $os");
    $expect = length($tag) ? "$tag $sp" : $sp;
    is($desc,    $expect,       "desc:    $desc");
}

check($_, Win32::PROCESSOR_ARCHITECTURE_INTEL) for @intel_tests, @dual_tests, @win10_tests;
check($_, Win32::PROCESSOR_ARCHITECTURE_AMD64) for @amd64_tests, @dual_tests;
check($_, Win32::PROCESSOR_ARCHITECTURE_IA64)  for @ia64_tests;

