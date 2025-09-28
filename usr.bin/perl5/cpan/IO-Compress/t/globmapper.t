BEGIN {
    if ($ENV{PERL_CORE}) {
	chdir 't' if -d 't';
	@INC = ("../lib", "lib/compress");
    }
}

use lib qw(t t/compress);
use strict ;
use warnings ;

use Test::More ;
use CompTestUtils;


BEGIN
{
    plan(skip_all => "File::GlobMapper needs Perl 5.005 or better - you have
Perl $]" )
        if $] < 5.005 ;

    # use Test::NoWarnings, if available
    my $extra = 0 ;
    $extra = 1
        if eval { require Test::NoWarnings ;  import Test::NoWarnings; 1 };

    plan tests => 68 + $extra ;

    use_ok('File::GlobMapper') ;
}

{
    title "Error Cases" ;

    my $gm;

    for my $delim ( qw/ ( ) { } [ ] / )
    {
        $gm = File::GlobMapper->new("${delim}abc", '*.X');
        ok ! $gm, "  new failed" ;
        is $File::GlobMapper::Error, "Unmatched $delim in input fileglob",
            "  catch unmatched $delim";
    }

    for my $delim ( qw/ ( ) [ ] / )
    {
        $gm = File::GlobMapper->new("{${delim}abc}", '*.X');
        ok ! $gm, "  new failed" ;
        is $File::GlobMapper::Error, "Unmatched $delim in input fileglob",
            "  catch unmatched $delim inside {}";
    }


}

{
    title "input glob matches zero files";

    #my $tmpDir = 'td';
    my $tmpDir ;
    my $lex = LexDir->new( $tmpDir );
    my $d = quotemeta $tmpDir;

    my $gm = File::GlobMapper->new("$d/Z*", '*.X');
    ok $gm, "  created GlobMapper object" ;

    my $map = $gm->getFileMap() ;
    is @{ $map }, 0, "  returned 0 maps";
    is_deeply $map, [], " zero maps" ;

    my $hash = $gm->getHash() ;
    is_deeply $hash, {}, "  zero maps" ;
}

{
    title 'test wildcard mapping of * in destination';

    #my $tmpDir = 'td';
    my $tmpDir ;
    my $lex = LexDir->new( $tmpDir );
    #mkdir $tmpDir, 0777 ;

    touch map { "$tmpDir/$_.tmp" } qw( abc1 abc2 abc3 ) ;

    my $gm = File::GlobMapper->new("$tmpDir/ab*.tmp", "*X");
    ok $gm, "  created GlobMapper object" ;

    my $map = $gm->getFileMap() ;
    is @{ $map }, 3, "  returned 3 maps";
    is_deeply $map,
        [ [map { "$tmpDir/$_" } qw(abc1.tmp abc1.tmpX)],
          [map { "$tmpDir/$_" } qw(abc2.tmp abc2.tmpX)],
          [map { "$tmpDir/$_" } qw(abc3.tmp abc3.tmpX)],
        ], "  got mapping";

    my $hash = $gm->getHash() ;
    is_deeply $hash,
        { map { "$tmpDir/$_" } qw(abc1.tmp abc1.tmpX
                                  abc2.tmp abc2.tmpX
                                  abc3.tmp abc3.tmpX),
        }, "  got mapping";
}

{
    title 'no wildcards in input or destination';

    #my $tmpDir = 'td';
    my $tmpDir ;
    my $lex = LexDir->new( $tmpDir );
    #mkdir $tmpDir, 0777 ;

    touch map { "$tmpDir/$_.tmp" } qw( abc1 abc2 abc3 ) ;

    my $gm = File::GlobMapper->new("$tmpDir/abc2.tmp", "$tmpDir/abc2.tmp");
    ok $gm, "  created GlobMapper object" ;

    my $map = $gm->getFileMap() ;
    is @{ $map }, 1, "  returned 1 maps";
    is_deeply $map,
        [ [map { "$tmpDir/$_.tmp" } qw(abc2 abc2)],
        ], "  got mapping";

    my $hash = $gm->getHash() ;
    is_deeply $hash,
        { map { "$tmpDir/$_.tmp" } qw(abc2 abc2),
        }, "  got mapping";
}

{
    title 'test wildcard mapping of {} in destination';

    my $tmpDir ;#= 'td';
    my $lex = LexDir->new( $tmpDir );
    #mkdir $tmpDir, 0777 ;

    touch map { "$tmpDir/$_.tmp" } qw( abc1 abc2 abc3 ) ;

    my $gm = File::GlobMapper->new("$tmpDir/abc{1,3}.tmp", "*.X");
    #diag "Input pattern is $gm->{InputPattern}";
    ok $gm, "  created GlobMapper object" ;

    my $map = $gm->getFileMap() ;
    is @{ $map }, 2, "  returned 2 maps";
    is_deeply $map,
        [ [map { "$tmpDir/$_" } qw(abc1.tmp abc1.tmp.X)],
          [map { "$tmpDir/$_" } qw(abc3.tmp abc3.tmp.X)],
        ], "  got mapping";

    $gm = File::GlobMapper->new("$tmpDir/abc{1,3}.tmp", "$tmpDir/X.#1.X")
        or diag $File::GlobMapper::Error ;
    #diag "Input pattern is $gm->{InputPattern}";
    ok $gm, "  created GlobMapper object" ;

    $map = $gm->getFileMap() ;
    is @{ $map }, 2, "  returned 2 maps";
    is_deeply $map,
        [ [map { "$tmpDir/$_" } qw(abc1.tmp X.1.X)],
          [map { "$tmpDir/$_" } qw(abc3.tmp X.3.X)],
        ], "  got mapping";

}


{
    title 'test wildcard mapping of multiple * to #';

    my $tmpDir ;#= 'td';
    my $lex = LexDir->new( $tmpDir );
    #mkdir $tmpDir, 0777 ;

    touch map { "$tmpDir/$_.tmp" } qw( abc1 abc2 abc3 ) ;

    my $gm = File::GlobMapper->new("$tmpDir/*b(*).tmp", "$tmpDir/X-#2-#1-X");
    ok $gm, "  created GlobMapper object"
        or diag $File::GlobMapper::Error ;

    my $map = $gm->getFileMap() ;
    is @{ $map }, 3, "  returned 3 maps";
    is_deeply $map,
        [ [map { "$tmpDir/$_" } qw(abc1.tmp X-c1-a-X)],
          [map { "$tmpDir/$_" } qw(abc2.tmp X-c2-a-X)],
          [map { "$tmpDir/$_" } qw(abc3.tmp X-c3-a-X)],
        ], "  got mapping";
}

{
    title 'test wildcard mapping of multiple ? to #';

    my $tmpDir ;#= 'td';
    my $lex = LexDir->new( $tmpDir );
    #mkdir $tmpDir, 0777 ;

    touch map { "$tmpDir/$_.tmp" } qw( abc1 abc2 abc3 ) ;

    my $gm = File::GlobMapper->new("$tmpDir/?b(*).tmp", "$tmpDir/X-#2-#1-X");
    ok $gm, "  created GlobMapper object" ;

    my $map = $gm->getFileMap() ;
    is @{ $map }, 3, "  returned 3 maps";
    is_deeply $map,
        [ [map { "$tmpDir/$_" } qw(abc1.tmp X-c1-a-X)],
          [map { "$tmpDir/$_" } qw(abc2.tmp X-c2-a-X)],
          [map { "$tmpDir/$_" } qw(abc3.tmp X-c3-a-X)],
        ], "  got mapping";
}

{
    title 'test wildcard mapping of multiple ?,* and [] to #';

    my $tmpDir ;#= 'td';
    my $lex = LexDir->new( $tmpDir );
    #mkdir $tmpDir, 0777 ;

    touch map { "$tmpDir/$_.tmp" } qw( abc1 abc2 abc3 ) ;

    my $gm = File::GlobMapper->new("$tmpDir/?b[a-z]*.tmp", "$tmpDir/X-#3-#2-#1-X");
    ok $gm, "  created GlobMapper object" ;

    #diag "Input pattern is $gm->{InputPattern}";
    my $map = $gm->getFileMap() ;
    is @{ $map }, 3, "  returned 3 maps";
    is_deeply $map,
        [ [map { "$tmpDir/$_" } qw(abc1.tmp X-1-c-a-X)],
          [map { "$tmpDir/$_" } qw(abc2.tmp X-2-c-a-X)],
          [map { "$tmpDir/$_" } qw(abc3.tmp X-3-c-a-X)],
        ], "  got mapping";
}

{
    title 'input glob matches a file multiple times';

    my $tmpDir ;#= 'td';
    my $lex = LexDir->new( $tmpDir );
    #mkdir $tmpDir, 0777 ;

    touch "$tmpDir/abc.tmp";

    my $gm = File::GlobMapper->new("$tmpDir/{a*,*c}.tmp", '*.X');
    ok $gm, "  created GlobMapper object" ;

    my $map = $gm->getFileMap() ;
    is @{ $map }, 1, "  returned 1 maps";
    is_deeply $map,
        [ [map { "$tmpDir/$_" } qw(abc.tmp abc.tmp.X)], ], "  got mapping";

    my $hash = $gm->getHash() ;
    is_deeply $hash,
        { map { "$tmpDir/$_" } qw(abc.tmp abc.tmp.X) }, "  got mapping";

}

{
    title 'multiple input files map to one output file';

    my $tmpDir ;#= 'td';
    my $lex = LexDir->new( $tmpDir );
    #mkdir $tmpDir, 0777 ;

    touch map { "$tmpDir/$_.tmp" } qw( abc def) ;

    my $gm = File::GlobMapper->new("$tmpDir/*.tmp", "$tmpDir/fred");
    ok ! $gm, "  did not create GlobMapper object" ;

    is $File::GlobMapper::Error, 'multiple input files map to one output file', "  Error is expected" ;

    #my $map = $gm->getFileMap() ;
    #is @{ $map }, 1, "  returned 1 maps";
    #is_deeply $map,
    #[ [map { "$tmpDir/$_" } qw(abc1 abc.X)], ], "  got mapping";
}

{
    title "globmap" ;

    my $tmpDir ;#= 'td';
    my $lex = LexDir->new( $tmpDir );
    #mkdir $tmpDir, 0777 ;

    touch map { "$tmpDir/$_.tmp" } qw( abc1 abc2 abc3 ) ;

    my $map = File::GlobMapper::globmap("$tmpDir/*b*.tmp", "$tmpDir/X-#2-#1-X");
    ok $map, "  got map"
        or diag $File::GlobMapper::Error ;

    is @{ $map }, 3, "  returned 3 maps";
    is_deeply $map,
        [ [map { "$tmpDir/$_" } qw(abc1.tmp X-c1-a-X)],
          [map { "$tmpDir/$_" } qw(abc2.tmp X-c2-a-X)],
          [map { "$tmpDir/$_" } qw(abc3.tmp X-c3-a-X)],
        ], "  got mapping";
}

# TODO
# test each of the wildcard metacharacters can be mapped to the output filename
#
#   ~ [] {} . *

# input & output glob with no wildcards is ok
# input with no wild or output with no wild is bad
# input wild has concatenated *'s
# empty string for either both from & to
# escaped chars within [] and {}, including the chars []{}
# escaped , within {}
# missing ] and missing }
# {} and {,} are special cases
# {ab*,de*}
# {abc,{},{de,f}} => abc {} de f
