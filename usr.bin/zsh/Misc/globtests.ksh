#!/usr/local/bin/zsh -f

setopt kshglob extendedglob

failed=0
while read res str pat; do
  [[ $res = '#' ]] && continue
  [[ $str = ${~pat} ]]
  ts=$?
  [[ $1 = -q ]] || print "$ts:  [[ $str = $pat ]]"
  if [[ ( $ts -gt 0 && $res = t) || ($ts -eq 0 && $res = f) ]]; then
    print "Test failed:  [[ $str = $pat ]]"
    (( failed++ ))
  fi
done <<EOT
t fofo                *(f*(o))
t ffo                 *(f*(o))
t foooofo             *(f*(o))
t foooofof            *(f*(o))
t fooofoofofooo       *(f*(o))
f foooofof            *(f+(o))
f xfoooofof           *(f*(o))
f foooofofx           *(f*(o))
t ofxoofxo            *(*(of*(o)x)o)
f ofooofoofofooo      *(f*(o))
t foooxfooxfoxfooox   *(f*(o)x)
f foooxfooxofoxfooox  *(f*(o)x)
t foooxfooxfxfooox    *(f*(o)x)
t ofxoofxo            *(*(of*(o)x)o)
t ofoooxoofxo         *(*(of*(o)x)o)
t ofoooxoofxoofoooxoofxo            *(*(of*(o)x)o)
t ofoooxoofxoofoooxoofxoo           *(*(of*(o)x)o)
f ofoooxoofxoofoooxoofxofo          *(*(of*(o)x)o)
t ofoooxoofxoofoooxoofxooofxofxo    *(*(of*(o)x)o)
t aac    *(@(a))a@(c)
t ac     *(@(a))a@(c)
f c      *(@(a))a@(c)
t aaac   *(@(a))a@(c)
f baaac  *(@(a))a@(c)
t abcd   ?@(a|b)*@(c)d
t abcd   @(ab|a*@(b))*(c)d
t acd    @(ab|a*(b))*(c)d
t abbcd  @(ab|a*(b))*(c)d
t effgz  @(b+(c)d|e*(f)g?|?(h)i@(j|k))
t efgz   @(b+(c)d|e*(f)g?|?(h)i@(j|k))
t egz    @(b+(c)d|e*(f)g?|?(h)i@(j|k))
t egzefffgzbcdij    *(b+(c)d|e*(f)g?|?(h)i@(j|k))
f egz    @(b+(c)d|e+(f)g?|?(h)i@(j|k))
t ofoofo *(of+(o))
t oxfoxoxfox    *(oxf+(ox))
f oxfoxfox      *(oxf+(ox))
t ofoofo        *(of+(o)|f)
# The following is supposed to match only as fo+ofo+ofo
t foofoofo      @(foo|f|fo)*(f|of+(o))
t oofooofo      *(of|oof+(o))
t fffooofoooooffoofffooofff      *(*(f)*(o))
# If the following is really slow, that's a bug.
f fffooofoooooffoofffooofffx     *(*(f)*(o))
# The following tests backtracking in alternation matches
t fofoofoofofoo *(fo|foo)
# Exclusion
t foo           !(x)
t foo           !(x)*
f foo           !(foo)
t foo           !(foo)*
t foobar        !(foo)
t foobar        !(foo)*
t moo.cow       !(*.*).!(*.*)
f mad.moo.cow   !(*.*).!(*.*)
f mucca.pazza   mu!(*(c))?.pa!(*(z))?
f _foo~         _?(*[^~])
t fff           !(f)
t fff           *(!(f))
t fff           +(!(f))
t ooo           !(f)
t ooo           *(!(f))
t ooo           +(!(f))
t foo           !(f)
t foo           *(!(f))
t foo           +(!(f))
f f             !(f)
f f             *(!(f))
f f             +(!(f))
t foot          @(!(z*)|*x)
f zoot          @(!(z*)|*x)
t foox          @(!(z*)|*x)
t zoox          @(!(z*)|*x)
t foo           *(!(foo))
f foob          !(foo)b*
t foobb         !(foo)b*
t fooxx         (#i)FOOXX
f fooxx         (#l)FOOXX
t FOOXX         (#l)fooxx
f fooxx         (#i)FOO@(#I)X@(#i)X
t fooXx         (#i)FOO@(#I)X@(#i)X
t fooxx         @((#i)FOOX)x
f fooxx         @((#i)FOOX)X
f BAR           @(bar|(#i)foo)
t FOO           @(bar|(#i)foo)
t Modules       (#i)*m*
EOT
print "$failed tests failed."
