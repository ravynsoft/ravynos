#!./perl

BEGIN {
    chdir 't' if -d 't';
    require "./test.pl";
    set_up_inc('../lib');
    require "./charset_tools.pl";
}

plan(tests => 3);

# It is important that the script contains at least one newline character
# that can be expanded to \r\n on DOSish systems.
fresh_perl_is(byte_utf8a_to_utf8n("\xEF\xBB\xBF") . "print 1;\nprint 2", "12", {}, "script starts with a BOM" );

# Big- and little-endian UTF-16
for my $end (0, 1) {
	my $encoding = $end ? 'UTF-16LE' : 'UTF-16BE';
	my $prog = join '', map chr($_), map {
		$end ? @$_[0, 1] : @$_[1, 0]
	} (
                # Create UTF-16.
		[ 0xFE, 0xFF ], map [ 0, ord($_) ], split //, "print 1;\nprint 2"
	);
	fresh_perl_is($prog, "12", {}, "BOM indicates $encoding");
}
