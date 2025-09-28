use warnings;
use strict;

use Test::More tests => 32;

BEGIN { $^H |= 0x20000; }

my $t;

$t = "";
eval q{
	use XS::APItest qw(labelconst);
	$t .= "a";
	$t .= labelconst b:;
	$t .= "c";
};
is $@, "";
is $t, "abc";

$t = "";
eval q{
	use XS::APItest qw(labelconst);
	$t .= "a";
	$t .= "b" . labelconst FOO: . "c";
	$t .= "d";
};
is $@, "";
is $t, "abFOOcd";

$t = "";
eval q{
	use XS::APItest qw(labelconst);
	$t .= "a";
	$t .= labelconst FOO :;
	$t .= "b";
};
is $@, "";
is $t, "aFOOb";

$t = "";
eval q{
	use XS::APItest qw(labelconst);
	$t .= "a";
	$t .= labelconst F_1B:;
	$t .= "b";
};
is $@, "";
is $t, "aF_1Bb";

$t = "";
eval q{
	use XS::APItest qw(labelconst);
	$t .= "a";
	$t .= labelconst _AB:;
	$t .= "b";
};
is $@, "";
is $t, "a_ABb";

$t = "";
eval q{
	use XS::APItest qw(labelconst);
	no warnings;
	$t .= "a";
	$t .= labelconst 1AB:;
	$t .= "b";
};
isnt $@, "";
is $t, "";

$t = "";
eval q{
	use XS::APItest qw(labelconst);
	$t .= "a";
	$t .= labelconst :;
	$t .= "b";
};
isnt $@, "";
is $t, "";

$t = "";
eval q{
	use XS::APItest qw(labelconst);
	$t .= "a";
	$t .= labelconst ;
	$t .= "b";
};
isnt $@, "";
is $t, "";

$t = "";
$t = do("./t/labelconst.aux");
is $@, "";
is $t, "FOOBARBAZQUUX";

{
    use utf8;
    use open qw( :utf8 :std );
    
    $t = "";
    eval q{
            use XS::APItest qw(labelconst);
            $t .= "ㅏ";
            $t .= labelconst ᛒ:;
            $t .= "ḉ";
    };
    is $@, "";
    is $t, "ㅏᛒḉ";
    
    $t = "";
    eval q{
            use XS::APItest qw(labelconst);
            $t .= "ㅏ";
            $t .= "ᛒ" . labelconst ＦǑǑ: . "ḉ";
            $t .= "ｄ";
    };
    is $@, "";
    is $t, "ㅏᛒＦǑǑḉｄ";
    
    $t = "";
    eval q{
            use XS::APItest qw(labelconst);
            $t .= "ㅏ";
            $t .= labelconst ＦǑǑ :;
            $t .= "ᛒ";
    };
    is $@, "";
    is $t, "ㅏＦǑǑᛒ";
    
    $t = "";
    eval q{
            use XS::APItest qw(labelconst);
            $t .= "ㅏ";
            $t .= labelconst Ｆ_1Ḅ:;
            $t .= "ᛒ";
    };
    is $@, "";
    is $t, "ㅏＦ_1Ḅᛒ";
    
    $t = "";
    eval q{
            use XS::APItest qw(labelconst);
            $t .= "ㅏ";
            $t .= labelconst _AḄ:;
            $t .= "ᛒ";
    };
    is $@, "";
    is $t, "ㅏ_AḄᛒ";
    
    $t = "";
    eval q{
            use XS::APItest qw(labelconst);
            no warnings;
            $t .= "ㅏ";
            $t .= labelconst 1AḄ:;
            $t .= "ᛒ";
    };
    isnt $@, "";
    is $t, "";
    
}

{
    use utf8;
    $t = "";
    $t = do("./t/labelconst_utf8.aux");
    is $@, "";
    is $t, "ＦǑǑBÀRᛒÀZQÙÙX";
}

1;
