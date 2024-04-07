# copied over from JSON::XS and modified to use JSON::PP

# use the testsuite from http://www.json.org/JSON_checker/
# except for fail18.json, as we do not support a depth of 20 (but 16 and 32).

use strict;
no warnings;
use Test::More;
BEGIN { plan tests => 38 };

BEGIN { $ENV{PERL_JSON_BACKEND} = 0; }

use JSON::PP;

# emulate JSON_checker default config
my $json = JSON::PP->new->utf8->max_depth(32)->canonical;

my $vax_float = (pack("d",1) =~ /^[\x80\x10]\x40/);

binmode DATA;

for (;;) {
   $/ = "\n# ";
   chomp (my $test = <DATA>)
      or last;
   $/ = "\n";
   my $name = <DATA>;
   if ($vax_float && $name =~ /pass1.json/) {
       $test =~ s/\b23456789012E66\b/23456789012E20/;
   }

   if (my $perl = eval { $json->decode ($test) }) {
      ok ($name =~ /^pass/, $name);
      is ($json->encode ($json->decode ($json->encode ($perl))), $json->encode ($perl));
   } else {
      ok ($name =~ /^fail/, "$name ($@)");
   }
}

__DATA__
{"Extra value after close": true} "misplaced quoted value"
# fail10.json
{"Illegal expression": 1 + 2}
# fail11.json
{"Illegal invocation": alert()}
# fail12.json
{"Numbers cannot have leading zeroes": 013}
# fail13.json
{"Numbers cannot be hex": 0x14}
# fail14.json
["Illegal backslash escape: \x15"]
# fail15.json
[\naked]
# fail16.json
["Illegal backslash escape: \017"]
# fail17.json
[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[["Too deep"]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
# fail18.json
{"Missing colon" null}
# fail19.json
["Unclosed array"
# fail2.json
{"Double colon":: null}
# fail20.json
{"Comma instead of colon", null}
# fail21.json
["Colon instead of comma": false]
# fail22.json
["Bad value", truth]
# fail23.json
['single quote']
# fail24.json
["	tab	character	in	string	"]
# fail25.json
["tab\   character\   in\  string\  "]
# fail26.json
["line
break"]
# fail27.json
["line\
break"]
# fail28.json
[0e]
# fail29.json
{unquoted_key: "keys must be quoted"}
# fail3.json
[0e+]
# fail30.json
[0e+-1]
# fail31.json
{"Comma instead if closing brace": true,
# fail32.json
["mismatch"}
# fail33.json
["extra comma",]
# fail4.json
["double extra comma",,]
# fail5.json
[   , "<-- missing value"]
# fail6.json
["Comma after the close"],
# fail7.json
["Extra close"]]
# fail8.json
{"Extra comma": true,}
# fail9.json
[
    "JSON Test Pattern pass1",
    {"object with 1 member":["array with 1 element"]},
    {},
    [],
    -42,
    true,
    false,
    null,
    {
        "integer": 1234567890,
        "real": -9876.543210,
        "e": 0.123456789e-12,
        "E": 1.234567890E+34,
        "":  23456789012E66,
        "zero": 0,
        "one": 1,
        "space": " ",
        "quote": "\"",
        "backslash": "\\",
        "controls": "\b\f\n\r\t",
        "slash": "/ & \/",
        "alpha": "abcdefghijklmnopqrstuvwyz",
        "ALPHA": "ABCDEFGHIJKLMNOPQRSTUVWYZ",
        "digit": "0123456789",
        "0123456789": "digit",
        "special": "`1~!@#$%^&*()_+-={':[,]}|;.</>?",
        "hex": "\u0123\u4567\u89AB\uCDEF\uabcd\uef4A",
        "true": true,
        "false": false,
        "null": null,
        "array":[  ],
        "object":{  },
        "address": "50 St. James Street",
        "url": "http://www.JSON.org/",
        "comment": "// /* <!-- --",
        "# -- --> */": " ",
        " s p a c e d " :[1,2 , 3

,

4 , 5        ,          6           ,7        ],"compact":[1,2,3,4,5,6,7],
        "jsontext": "{\"object with 1 member\":[\"array with 1 element\"]}",
        "quotes": "&#34; \u0022 %22 0x22 034 &#x22;",
        "\/\\\"\uCAFE\uBABE\uAB98\uFCDE\ubcda\uef4A\b\f\n\r\t`1~!@#$%^&*()_+-=[]{}|;:',./<>?"
: "A key can be any string"
    },
    0.5 ,98.6
,
99.44
,

1066,
1e1,
0.1e1,
1e-1,
1e00,2e+00,2e-00
,"rosebud"]
# pass1.json
[[[[[[[[[[[[[[[[[[["Not too deep"]]]]]]]]]]]]]]]]]]]
# pass2.json
{
    "JSON Test Pattern pass3": {
        "The outermost value": "must be an object or array.",
        "In this test": "It is an object."
    }
}

# pass3.json
