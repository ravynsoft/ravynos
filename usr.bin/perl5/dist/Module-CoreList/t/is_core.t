#!perl -w
use strict;
use Module::CoreList;
use Test::More tests => 44;

BEGIN { require_ok('Module::CoreList'); }

# Check default perl

ok(Module::CoreList::is_core('IO::File', $Module::CoreList::version{$]}{'IO::File'}), "is_core is self-consistent");

ok(!Module::CoreList::is_core('Module::Path'), 'Module::Path has never been in core');
ok(!Module::CoreList::is_core('Module::Path', undef, '5.016003'), 'Module::Path has never been in core');
ok(!Module::CoreList::is_core('Module::Path', undef), 'Module::Path has never been in core');

# List::Util::PP was added in 5.010001 and removed in 5.017001
ok(!Module::CoreList::is_core('List::Util::PP', undef, '5.002'), 'List::Util::PP was added in 5.10.1 so not in core in 5.002');
ok(Module::CoreList::is_core('List::Util::PP', undef, '5.016003'), 'List::Util::PP was in core in 5.16.3');
ok(!Module::CoreList::is_core('List::Util::PP', undef, '5.018001'), 'List::Util::PP was removed in 5.17.1 so not in core in 5.18.1');

# Carp has always been a core module
ok(Module::CoreList::is_core('Carp', undef, '5'), 'Carp was a core module in first release of perl 5');
ok(Module::CoreList::is_core('Carp', undef, '5.019004'), 'Carp was still a core module in 5.19.4');
ok(Module::CoreList::is_core('Carp'), "Carp should be a core module whatever version of perl you're running");

ok(Module::CoreList::is_core('attributes', undef, '5.00503') == 0, "attributes weren't in 5.00503");
ok(Module::CoreList::is_core('attributes', undef, '5.006001') == 1, "attributes were in 5.6.1");
ok(Module::CoreList::is_core('Pod::Plainer', undef, '5.012001') == 1, "Pod::Plainer was core in 5.12.1");
ok(Module::CoreList::is_core('Pod::Plainer', undef, '5.016003') == 0, "Pod::Plainer was removed in 5.13.1");

ok(!Module::CoreList::is_core('File::Temp', 0, '5.006'), 'File::Temp is not in 5.006000');
ok(Module::CoreList::is_core('File::Temp', 0, '5.006001'), 'File::Temp is in 5.006001');
ok(!Module::CoreList::is_core('File::Temp', '0.12', '5.006'), 'File::Temp 0.12 is not in 5.006000');
ok(Module::CoreList::is_core('File::Temp', '0.12', '5.006001'), 'File::Temp 0.12 is in 5.006001');
ok(Module::CoreList::is_core('File::Temp', '0.12', '5.006002'), 'File::Temp 0.12 is in 5.006002');


# history of module 'encoding' in core
#   version 1.00 included in 5.007003
#   version 1.35 included in 5.008
#   version 1.47 included in 5.008001
#   version 1.48 included in 5.008003
#   version 2.00 included in 5.008005
#   version 2.01 included in 5.008006
#   version 2.02 included in 5.008008
#   version 2.6_01 included in 5.008009
#   version 2.04 included in 5.009004
#   version 2.06 included in 5.009005
#   version 2.6_01 included in 5.010001
#   version 2.12 included in 5.019001

ok(!Module::CoreList::is_core('encoding', undef, '5'), "encoding wasn't in core in first release of perl 5");
ok(!Module::CoreList::is_core('encoding', '1.00', '5'), "encoding 1.00 wasn't in core in first release of perl 5");
ok(!Module::CoreList::is_core('encoding', '1.35', '5.007003'), "encoding 1.35 wasn't yet in core in perl 5.007003");
ok(Module::CoreList::is_core('encoding', '1.35', '5.008'), "encoding 1.35 was first included in perl 5.008");
ok(Module::CoreList::is_core('encoding', '1.35', '5.009004'), "encoding 2.04 (>1.35) was included in 5.009004");
ok(Module::CoreList::is_core('encoding', '2.01', '5.008007'), "encoding 2.01 was first in core in perl 5.008006, so was core in 5.8.7");
ok(Module::CoreList->is_core('encoding', '2.01', '5.008007'), "encoding 2.01 was first in core in perl 5.008006, so was core in 5.8.7");

# Module::CoreList (2.17) was first included in 5.008009
ok(!Module::CoreList::is_core('Module::CoreList', undef, '5.007003'), "Module::CoreList wasn't core in perl 5.7.3");
ok(!Module::CoreList->is_core('Module::CoreList', undef, '5.007003'), "Module::CoreList wasn't core in perl 5.7.3 (class method)");

# Test for situations where different branches on the perl
# release tree had different versions of a module, and a naive
# checking of perl release number will trip you up
ok(Module::CoreList->is_core('Text::Soundex', '1.01', '5.008007'), "Text::Soundex 1.01 was first included in 5.007003");
ok(Module::CoreList->is_core('Text::Soundex', '3.03', '5.008009'), "Text::Soundex 3.03 was included in 5.008009");
ok(!Module::CoreList->is_core('Text::Soundex', '3.03', '5.009003'), "5.009003 still had Text::Soundex 1.01");
ok(Module::CoreList->is_core('Text::Soundex', '1.01', '5.009003'), "5.009003 still had Text::Soundex 1.01");
ok(!Module::CoreList->is_core('Text::Soundex', '3.03', '5.009005'), "5.009005 still had Text::Soundex 3.02");
ok(Module::CoreList->is_core('Text::Soundex', '3.02', '5.009005'), "5.009005 had Text::Soundex 3.02");
ok(Module::CoreList->is_core('Text::Soundex', '3.03', '5.01'), "5.01 had Text::Soundex 3.03");

# 5.002 was the first perl release where core modules had a version number
ok(Module::CoreList->is_core('DB_File', '1.01', '5.002'), "DB_File 1.01 was included in 5.002");
ok(!Module::CoreList->is_core('DB_File', '1.03', '5.002'), "DB_File 1.03 wasn't included in 5.002");
ok(Module::CoreList->is_core('DB_File', '1.03', '5.00307'), "DB_File 1.03 was included in 5.00307");

ok(! Module::CoreList->is_core("CGI", undef, 5.021), "CGI not in 5.021");
ok(! Module::CoreList->is_core("CGI", undef, 5.021001), "CGI not in 5.021001");

ok(  Module::CoreList::is_core("Config", 0, "5.020"), "Config v0+ is in core in 5.020");
ok(  Module::CoreList::is_core("Config", undef, "5.020"), "Config v(undef) is in core in 5.020");

eval { Module::CoreList::is_core('Config', 'invalid', '5.020'); };
like( $@, qr/^Invalid version 'invalid' specified\b/, 'invalid version throws');
