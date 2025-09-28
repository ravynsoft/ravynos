use strict;
use Test::More tests => 21;
BEGIN {use_ok('I18N::LangTags', ':ALL')};

note("Perl v$], I18N::LangTags v$I18N::LangTags::VERSION");
note("Loaded from " . ($INC{'I18N/LangTags.pm'} || "??"));

foreach ('C', 'POSIX') {
    # It seems that the acceptable result is either '' or undef
    my $tag = locale2language_tag($_);
    is($tag, defined $tag ? '' : undef, "locale2language_tag('$_')");
}

foreach (['en', 'en'],
	 ['en_US', 'en-us'],
	 ['en_US.ISO8859-1', 'en-us'],
	 ['eu_mt', 'eu-mt'],
	 ['eu', 'eu'],
	 ['it', 'it'],
	 ['it_IT', 'it-it'],
	 ['it_IT.utf8', 'it-it'],
	 ['it_IT.utf8@euro', 'it-it'],
	 ['it_IT@euro', 'it-it'],
	 ['zh_CN.gb18030', 'zh-cn'],
	 ['zh_CN.gbk', 'zh-cn'],
	 ['zh_CN.utf8', 'zh-cn'],
	 ['zh_HK', 'zh-hk'],
	 ['zh_HK.utf8', 'zh-hk'],
	 ['zh_TW', 'zh-tw'],
	 ['zh_TW.euctw', 'zh-tw'],
	 ['zh_TW.utf8', 'zh-tw'],
	) { 
    my ($tag, $expect) = @$_;
    is(lc locale2language_tag($tag), $expect,
       "locale2language_tag('$tag')");
}
