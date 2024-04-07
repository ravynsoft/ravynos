#!./perl

use strict;
use warnings;
use Test::More;
use_ok('Tie::Hash');

tie my %tied, 'Tie::ExtraHash';
%tied = (apple => 'tree', cow => 'field');
my %hash = (apple => 'tree', cow => 'field');

# TIEHASH
is_deeply(\%hash, \%tied, "TIEHASH");
ok(tied(%tied), "TIEHASH really does tie");

# FIRST/NEXTKEY
is_deeply([sort keys %hash], [sort keys %tied], "FIRSTKEY/NEXTKEY");
is_deeply([sort values %hash], [sort values %tied], "FIRSTKEY/NEXTKEY");

# EXISTS
ok(exists($tied{apple}) && exists($hash{apple}),
   'EXISTS works when it exists');

# DELETE and !EXISTS
delete($tied{apple}); delete($hash{apple});
ok(!exists($tied{apple}) && !exists($hash{apple}),
   'EXISTS works when it doesn\'t exist (as does DELETE)');

# STORE and FETCH
$tied{house} = $hash{house} = 'town';
ok($tied{house} eq 'town' && $tied{house} eq $hash{house},
   'STORE and FETCH');

# CLEAR
%tied = (); %hash = ();
ok(tied(%tied), "still tied after CLEAR");
is_deeply(\%tied, \%hash, "CLEAR");

# SCALAR
is(scalar(%tied), scalar(%hash), "SCALAR");

done_testing();
