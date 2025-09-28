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

# _internal_method not blocked by default
$res = eval { $lh->maketext('[_internal_method]') };
is( $res, "_internal_method_response", '_internal_method allowed when no allowlist defined' );
is( $@, '', 'no exception thrown by use of _internal_method without allowlist setting' );

# allowlisting sprintf
$lh->whitelist('sprintf');

# _internal_method blocked by allowlist
$res = eval { $lh->maketext('[_internal_method]') };
is( $res, undef, 'no return value from blocked expansion' );
like( $@, qr/Can't use .* as a method name/, '_internal_method blocked in bracket notation by allowlist' );

# sprintf allowed by allowlist
$res = eval { $lh->maketext('[sprintf,%s,hello]') };
is( $res, "hello", 'sprintf allowed in bracket notation by allowlist' );
is( $@,   '',      'no exception thrown by use of sprintf with allowlist' );

# custom_handler blocked by allowlist
$res = eval { $lh->maketext('[custom_handler]') };
is( $res, undef, 'no return value from blocked expansion' );
like( $@, qr/Can't use .* as a method name/, 'custom_handler blocked in bracket notation by allowlist' );

# adding custom_handler to allowlist
$lh->whitelist('custom_handler');

# sprintf still allowed by allowlist
$res = eval { $lh->maketext('[sprintf,%s,hello]') };
is( $res, "hello", 'sprintf allowed in bracket notation by allowlist' );
is( $@,   '',      'no exception thrown by use of sprintf with allowlist' );

# custom_handler allowed by allowlist
$res = eval { $lh->maketext('[custom_handler]') };
is( $res, "custom_handler_response", 'custom_handler allowed in bracket notation by allowlist' );
is( $@, '', 'no exception thrown by use of custom_handler with allowlist' );

# _internal_method blocked by allowlist
$res = eval { $lh->maketext('[_internal_method]') };
is( $res, undef, 'no return value from blocked expansion' );
like( $@, qr/Can't use .* as a method name/, '_internal_method blocked in bracket notation by allowlist' );

# adding fail_with to allowlist
$lh->whitelist('fail_with');

# fail_with still blocked by blacklist
$res = eval { $lh->maketext('[fail_with,xyzzy]') };
is( $res, undef, 'no return value from blocked expansion' );
like( $@, qr/Can't use .* as a method name/, 'fail_with blocked in bracket notation by blacklist even when allowlisted' );

