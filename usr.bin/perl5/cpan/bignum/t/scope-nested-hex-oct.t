# -*- mode: perl; -*-

use strict;
use warnings;

use Test::More;

plan skip_all => 'Need at least Perl v5.10.1' if $] < "5.010001";

plan tests => 96;

note "\nbigint -> bigfloat -> bigrat\n\n";

{
    note "use bigint;";
    use bigint;
    is(ref(hex("1")), "Math::BigInt", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigInt", 'ref(oct("1"))');

    {
        note "use bigfloat;";
        use bigfloat;
        is(ref(hex("1")), "Math::BigFloat", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigFloat", 'ref(oct("1"))');

        {
            note "use bigrat;";
            use bigrat;
            is(ref(hex("1")), "Math::BigRat", 'ref(hex("1"))');
            is(ref(oct("1")), "Math::BigRat", 'ref(oct("1"))');

            note "no bigrat;";
            no bigrat;
            is(ref(hex("1")), "", 'ref(hex("1"))');
            is(ref(oct("1")), "", 'ref(oct("1"))');
        }

        is(ref(hex("1")), "Math::BigFloat", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigFloat", 'ref(oct("1"))');

        note "no bigfloat;";
        no bigfloat;
        is(ref(hex("1")), "", 'ref(hex("1"))');
        is(ref(oct("1")), "", 'ref(oct("1"))');
    }

    is(ref(hex("1")), "Math::BigInt", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigInt", 'ref(oct("1"))');

    note "no bigint;";
    no bigint;
    is(ref(hex("1")), "", 'ref(hex("1"))');
    is(ref(oct("1")), "", 'ref(oct("1"))');
}

note "\nbigint -> bigrat -> bigfloat\n\n";

{
    note "use bigint;";
    use bigint;
    is(ref(hex("1")), "Math::BigInt", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigInt", 'ref(oct("1"))');

    {
        note "use bigrat;";
        use bigrat;
        is(ref(hex("1")), "Math::BigRat", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigRat", 'ref(oct("1"))');

        {
            note "use bigfloat;";
            use bigfloat;
            is(ref(hex("1")), "Math::BigFloat", 'ref(hex("1"))');
            is(ref(oct("1")), "Math::BigFloat", 'ref(oct("1"))');

            note "no bigfloat;";
            no bigfloat;
            is(ref(hex("1")), "", 'ref(hex("1"))');
            is(ref(oct("1")), "", 'ref(oct("1"))');
        }

        is(ref(hex("1")), "Math::BigRat", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigRat", 'ref(oct("1"))');

        note "no bigrat;";
        no bigrat;
        is(ref(hex("1")), "", 'ref(hex("1"))');
        is(ref(oct("1")), "", 'ref(oct("1"))');
    }

    is(ref(hex("1")), "Math::BigInt", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigInt", 'ref(oct("1"))');

    note "no bigint;";
    no bigint;
    is(ref(hex("1")), "", 'ref(hex("1"))');
    is(ref(oct("1")), "", 'ref(oct("1"))');
}

note "\nbigfloat -> bigint -> bigrat\n\n";

{
    note "use bigfloat;";
    use bigfloat;
    is(ref(hex("1")), "Math::BigFloat", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigFloat", 'ref(oct("1"))');

    {
        note "use bigint;";
        use bigint;
        is(ref(hex("1")), "Math::BigInt", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigInt", 'ref(oct("1"))');

        {
            note "use bigrat;";
            use bigrat;
            is(ref(hex("1")), "Math::BigRat", 'ref(hex("1"))');
            is(ref(oct("1")), "Math::BigRat", 'ref(oct("1"))');

            note "no bigrat;";
            no bigrat;
            is(ref(hex("1")), "", 'ref(hex("1"))');
            is(ref(oct("1")), "", 'ref(oct("1"))');
        }

        is(ref(hex("1")), "Math::BigInt", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigInt", 'ref(oct("1"))');

        note "no bigint;";
        no bigint;
        is(ref(hex("1")), "", 'ref(hex("1"))');
        is(ref(oct("1")), "", 'ref(oct("1"))');
    }

    is(ref(hex("1")), "Math::BigFloat", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigFloat", 'ref(oct("1"))');

    note "no bigfloat;";
    no bigfloat;
    is(ref(hex("1")), "", 'ref(hex("1"))');
    is(ref(oct("1")), "", 'ref(oct("1"))');
}

note "\nbigfloat -> bigrat -> bigint\n\n";

{
    note "use bigfloat;";
    use bigfloat;
    is(ref(hex("1")), "Math::BigFloat", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigFloat", 'ref(oct("1"))');

    {
        note "use bigrat;";
        use bigrat;
        is(ref(hex("1")), "Math::BigRat", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigRat", 'ref(oct("1"))');

        {
            note "use bigint;";
            use bigint;
            is(ref(hex("1")), "Math::BigInt", 'ref(hex("1"))');
            is(ref(oct("1")), "Math::BigInt", 'ref(oct("1"))');

            note "no bigint;";
            no bigint;
            is(ref(hex("1")), "", 'ref(hex("1"))');
            is(ref(oct("1")), "", 'ref(oct("1"))');
        }

        is(ref(hex("1")), "Math::BigRat", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigRat", 'ref(oct("1"))');

        note "no bigrat;";
        no bigrat;
        is(ref(hex("1")), "", 'ref(hex("1"))');
        is(ref(oct("1")), "", 'ref(oct("1"))');
    }

    is(ref(hex("1")), "Math::BigFloat", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigFloat", 'ref(oct("1"))');

    note "no bigfloat;";
    no bigfloat;
    is(ref(hex("1")), "", 'ref(hex("1"))');
    is(ref(oct("1")), "", 'ref(oct("1"))');
}

note "\nbigrat -> bigint -> bigfloat\n\n";

{
    note "use bigrat;";
    use bigrat;
    is(ref(hex("1")), "Math::BigRat", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigRat", 'ref(oct("1"))');

    {
        note "use bigint;";
        use bigint;
        is(ref(hex("1")), "Math::BigInt", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigInt", 'ref(oct("1"))');

        {
            note "use bigfloat;";
            use bigfloat;
            is(ref(hex("1")), "Math::BigFloat", 'ref(hex("1"))');
            is(ref(oct("1")), "Math::BigFloat", 'ref(oct("1"))');

            note "no bigfloat;";
            no bigfloat;
            is(ref(hex("1")), "", 'ref(hex("1"))');
            is(ref(oct("1")), "", 'ref(oct("1"))');
        }

        is(ref(hex("1")), "Math::BigInt", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigInt", 'ref(oct("1"))');

        note "no bigint;";
        no bigint;
        is(ref(hex("1")), "", 'ref(hex("1"))');
        is(ref(oct("1")), "", 'ref(oct("1"))');
    }

    is(ref(hex("1")), "Math::BigRat", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigRat", 'ref(oct("1"))');

    note "no bigrat;";
    no bigrat;
    is(ref(hex("1")), "", 'ref(hex("1"))');
    is(ref(oct("1")), "", 'ref(oct("1"))');
}

note "\nbigrat -> bigfloat -> bigint\n\n";

{
    note "use bigrat;";
    use bigrat;
    is(ref(hex("1")), "Math::BigRat", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigRat", 'ref(oct("1"))');

    {
        note "use bigfloat;";
        use bigfloat;
        is(ref(hex("1")), "Math::BigFloat", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigFloat", 'ref(oct("1"))');

        {
            note "use bigint;";
            use bigint;
            is(ref(hex("1")), "Math::BigInt", 'ref(hex("1"))');
            is(ref(oct("1")), "Math::BigInt", 'ref(oct("1"))');

            note "no bigint;";
            no bigint;
            is(ref(hex("1")), "", 'ref(hex("1"))');
            is(ref(oct("1")), "", 'ref(oct("1"))');
        }

        is(ref(hex("1")), "Math::BigFloat", 'ref(hex("1"))');
        is(ref(oct("1")), "Math::BigFloat", 'ref(oct("1"))');

        note "no bigfloat;";
        no bigfloat;
        is(ref(hex("1")), "", 'ref(hex("1"))');
        is(ref(oct("1")), "", 'ref(oct("1"))');
    }

    is(ref(hex("1")), "Math::BigRat", 'ref(hex("1"))');
    is(ref(oct("1")), "Math::BigRat", 'ref(oct("1"))');

    note "no bigrat;";
    no bigrat;
    is(ref(hex("1")), "", 'ref(hex("1"))');
    is(ref(oct("1")), "", 'ref(oct("1"))');
}
