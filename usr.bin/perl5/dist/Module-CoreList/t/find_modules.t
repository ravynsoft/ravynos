#!perl -w
use strict;
use Module::CoreList;
use Test::More tests => 6;

BEGIN { require_ok('Module::CoreList'); }

is_deeply([ Module::CoreList->find_modules(qr/warnings/) ],
          [ qw(encoding::warnings warnings warnings::register) ],
          'qr/warnings/');

is_deeply([ Module::CoreList->find_modules(qr/IPC::Open/) ],
          [ qw(IPC::Open2 IPC::Open3) ],
          'qr/IPC::Open/');

is_deeply([ Module::CoreList->find_modules(qr/Module::/, 5.008008) ], [], 'qr/Module::/ at 5.008008');

is_deeply([ Module::CoreList->find_modules(qr/Test::H.*::.*s/, 5.006001, 5.007003) ],
          [ qw(Test::Harness::Assert Test::Harness::Straps) ],
          'qr/Test::H.*::.*s/ at 5.006001 and 5.007003');

is_deeply([ Module::CoreList::find_modules(qr/Module::CoreList/) ],
          [ qw(Module::CoreList Module::CoreList::TieHashDelta Module::CoreList::Utils) ],
          'Module::CoreList functional' );
