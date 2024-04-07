VMS::DCLsym is an extension to Perl 5 which allows it to manipulate DCL symbols
via an object-oriented or tied-hash interface.

In order to build the extension, just say

$ Perl Makefile.PL
$ MMK

in the directory containing the source files.  Once it's built, you can run the
test script by saying

$ Perl "-Iblib" test.pl

Finally, if you want to make it part of your regular Perl library, you can say
$ MMK install

If you have any problems or suggestions, please feel free to let me know.

Regards,
Charles Bailey  bailey@newman.upenn.edu
17-Aug-1995
