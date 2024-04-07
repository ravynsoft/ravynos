use warnings;
use strict;

use ExtUtils::MakeMaker;

WriteMakefile(
	NAME => "Carp",
	VERSION_FROM => "lib/Carp.pm",
	ABSTRACT_FROM => "lib/Carp.pm",
	PREREQ_PM => {
		"Config" => 0,
		"Exporter" => 0,
		"ExtUtils::MakeMaker" => 0,
		"IPC::Open3" => "1.0103",
		"Test::More" => "0.47",
		"overload" => 0,
		"strict" => 0,
		"warnings" => 0,
	},
	LICENSE => "perl",
	INSTALLDIRS => "$]" < 5.011 ? "perl" : "site",
);

1;
