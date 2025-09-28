#!/usr/bin/perl -Tw

use strict;
use warnings;
use Test::More tests => 17;

BEGIN {
    use_ok("Locale::Maketext");
}

{

    package MyTestLocale;
    no warnings 'once';

    @MyTestLocale::ISA     = qw(Locale::Maketext);
    %MyTestLocale::Lexicon = ();
}

{

    package MyTestLocale::en;
    no warnings 'once';

    @MyTestLocale::en::ISA = qw(MyTestLocale);

    %MyTestLocale::en::Lexicon = ( '_AUTO' => 1 );

    sub custom_handler {
        return "custom_handler_response";
    }

    sub _internal_method {
        return "_internal_method_response";
    }

    sub new {
        my ( $class, @args ) = @_;
        my $lh = $class->SUPER::new(@args);
        $lh->{use_external_lex_cache} = 1;
        return $lh;
    }
}

my $lh = MyTestLocale->get_handle('en');
my $res;

# get_handle blocked by default
$res = eval { $lh->maketext('[get_handle,en]') };
is( $res, undef, 'no return value from blocked expansion' );
like( $@, qr/Can't use .* as a method name/, 'get_handle blocked in bracket notation by default denylist' );

# _ambient_langprefs blocked by default
$res = eval { $lh->maketext('[_ambient_langprefs]') };
is( $res, undef, 'no return value from blocked expansion' );
like( $@, qr/Can't use .* as a method name/, '_ambient_langprefs blocked in bracket notation by default denylist' );

# _internal_method not blocked by default
$res = eval { $lh->maketext('[_internal_method]') };
is( $res, "_internal_method_response", '_internal_method allowed in bracket notation by default denylist' );
is( $@, '', 'no exception thrown by use of _internal_method under default denylist' );

# sprintf not blocked by default
$res = eval { $lh->maketext('[sprintf,%s,hello]') };
is( $res, "hello", 'sprintf allowed in bracket notation by default denylist' );
is( $@,   '',      'no exception thrown by use of sprintf under default denylist' );

# denylisting sprintf and numerate
$lh->denylist( 'sprintf', 'numerate' );

# sprintf blocked by custom denylist
$res = eval { $lh->maketext('[sprintf,%s,hello]') };
is( $res, undef, 'no return value from blocked expansion' );
like( $@, qr/Can't use .* as a method name/, 'sprintf blocked in bracket notation by custom denylist' );

# denylisting numf and _internal_method
$lh->denylist('numf');
$lh->denylist('_internal_method');

# sprintf blocked by custom denylist
$res = eval { $lh->maketext('[sprintf,%s,hello]') };
is( $res, undef, 'no return value from blocked expansion' );
like( $@, qr/Can't use .* as a method name/, 'sprintf blocked in bracket notation by custom denylist after extension of denylist' );

# _internal_method blocked by custom denylist
$res = eval { $lh->maketext('[_internal_method]') };
is( $res, undef, 'no return value from blocked expansion' );
like( $@, qr/Can't use .* as a method name/, 'sprintf blocked in bracket notation by custom denylist after extension of denylist' );

# custom_handler not in default or custom denylist
$res = eval { $lh->maketext('[custom_handler]') };
is( $res, "custom_handler_response", 'custom_handler allowed in bracket notation by default and custom denylist' );
is( $@, '', 'no exception thrown by use of custom_handler under default and custom denylist' );
