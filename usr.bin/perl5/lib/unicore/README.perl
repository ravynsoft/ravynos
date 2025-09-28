# The goal is for perl to compile and reasonably run any version of Unicode.
# Working reasonably well doesn't mean that the test suite will run without
# showing errors.  A few of the very-Unicode specific test files have been
# modified to account for different versions, but most have not.  For example,
# some tests use characters that aren't encoded in all Unicode versions; others
# have hard-coded the General Categories for a code point that were correct at
# the time the test was written.  Perl itself will not compile under Unicode
# releases prior to 3.0 without a simple change to Unicode::Normalize.
# mktables contains instructions for this.

# The *.txt files were copied from

# 	ftp://www.unicode.org/Public/UNIDATA

# (which always points to the latest version) with subdirectories 'extracted' and
# 'auxiliary'.  Older versions are located under Public with an appropriate name.
# They are also available via http at www.unicode.org/versions/
#

# The Unihan files were not included due to space considerations.  Also NOT
# included were any *.html files.  It is possible to add the Unihan files and
# have some properties from them automatically compiled.  By editing mktables
# (see instructions near its beginning) you can add other Unihan properties.

# The file named 'version' should exist and be a single line with the Unicode
# version, like:
#
# 5.2.0
#
# (without the initial '# ')

# To be 8.3 filesystem friendly, the names of some of the input files have been
# changed from the values that are in the Unicode DB.  Not all of the Test
# files are currently used, so may not be present, so some of the mv's can
# fail.  The .html Test files are not touched.

mv PropertyValueAliases.txt PropValueAliases.txt
mv NamedSequencesProv.txt NamedSqProv.txt
mv NormalizationTest.txt NormTest.txt
mv DerivedAge.txt DAge.txt
mv DerivedCoreProperties.txt DCoreProperties.txt
mv DerivedNormalizationProps.txt DNormalizationProps.txt
mv IdentifierStatus.txt IdStatus.txt
mv IdentifierType.txt IdType.txt

# Some early releases don't have the extracted directory, and hence these files
# should be moved to it.
mkdir extracted 2>/dev/null
mv DerivedBidiClass.txt DerivedBinaryProperties.txt extracted 2>/dev/null
mv DerivedCombiningClass.txt DerivedDecompositionType.txt extracted 2>/dev/null
mv DerivedEastAsianWidth.txt DerivedGeneralCategory.txt extracted 2>/dev/null
mv DerivedJoiningGroup.txt DerivedJoiningType.txt extracted 2>/dev/null
mv DerivedLineBreak.txt DerivedNumericType.txt DerivedNumericValues.txt extracted 2>/dev/null

mv extracted/DerivedBidiClass.txt extracted/DBidiClass.txt
mv extracted/DerivedBinaryProperties.txt extracted/DBinaryProperties.txt
mv extracted/DerivedCombiningClass.txt extracted/DCombiningClass.txt
mv extracted/DerivedDecompositionType.txt extracted/DDecompositionType.txt
mv extracted/DerivedEastAsianWidth.txt extracted/DEastAsianWidth.txt
mv extracted/DerivedGeneralCategory.txt extracted/DGeneralCategory.txt
mv extracted/DerivedJoiningGroup.txt extracted/DJoinGroup.txt
mv extracted/DerivedJoiningType.txt extracted/DJoinType.txt
mv extracted/DerivedLineBreak.txt extracted/DLineBreak.txt
mv extracted/DerivedNumericType.txt extracted/DNumType.txt
mv extracted/DerivedNumericValues.txt extracted/DNumValues.txt
mv extracted/DerivedName.txt extracted/DName.txt
rmdir extracted 2>/dev/null     # Will fail if non-empty, but if it is empty
                                # was an early release that didn't have it.

mv auxiliary/GraphemeBreakTest.txt auxiliary/GCBTest.txt
mv auxiliary/LineBreakTest.txt auxiliary/LBTest.txt
mv auxiliary/SentenceBreakTest.txt auxiliary/SBTest.txt
mv auxiliary/WordBreakTest.txt auxiliary/WBTest.txt

# If you have the Unihan database (5.2 and above), you should also do the
# following:

mv Unihan_DictionaryIndices.txt UnihanIndicesDictionary.txt
mv Unihan_DictionaryLikeData.txt UnihanDataDictionaryLike.txt
mv Unihan_IRGSources.txt UnihanIRGSources.txt
mv Unihan_NumericValues.txt UnihanNumericValues.txt
mv Unihan_OtherMappings.txt UnihanOtherMappings.txt
mv Unihan_RadicalStrokeCounts.txt UnihanRadicalStrokeCounts.txt
mv Unihan_Readings.txt UnihanReadings.txt
mv Unihan_Variants.txt UnihanVariants.txt

# If you download everything, the names of files that are not used by mktables
# are not changed by the above, and hence may not work correctly as-is on 8.3
# filesystems.

# mktables is used to generate the tables used by the rest of Perl.  It will
# warn you about any *.txt and *.html files in the directory substructure that
# it doesn't know about.  You should remove any so-identified, or edit mktables
# to add them to its lists to process.  You can run
#
#    mktables -globlist
#
# to have it try to process these tables generically.

# COMPILING ON OLDER UNICODE VERSIONS
#
# To compile perl for use with an older Unicode release, delete everything in
# the lib/unicore directory except mktables and Makefile.  Then download the
# Unicode-supplied files for the desired version to that directory  (A url for
# these is given earlier in this file).  Then create the 'version' file with a
# single line, like '6.1.0'.  Do a 'make test' from the project level.  You
# will get some porting errors for needing to regen.  Regenerate what it tells
# you are needed, and make test again.  If you compile an old enough version,
# you will also have to download a few files from later Unicode versions,
# following the instructions that will be given if warranted.  It should
# compile in any release without warnings, except for some casing conflicts
# in Unicode 2.1.8, and some extraneous files will show up in very early
# releases of the form qr/diff.*\.txt/.  If you add Unihan.txt, one line is in error in
#
# Other glitches are noted in mktables under 'UNICODE VERSIONS NOTES'

# FOR PUMPKINS
#
# The files are inter-related.  If you take the latest UnicodeData.txt, for
# example, but leave the older versions of other files, there can be subtle
# problems.  So get everything available from Unicode, and delete those which
# aren't needed.
#
# When moving to a new version of Unicode, you need to update 'version' by hand
#
#	p4 edit version
# 	...
#
# You should look in the Unicode release notes (which are probably towards the
# bottom of http://www.unicode.org/reports/tr44/) to see if any properties have
# newly been moved to be Obsolete, Deprecated, or Stabilized.  The full names
# for these should be added to the respective lists near the beginning of
# mktables, using an 'if' to add them for just this Unicode version going
# forward, so that mktables can continue to be used for earlier Unicode
# versions.
#
# When putting out a new Perl release, think about if any of the Deprecated
# properties should be moved to Suppressed.
#
# perlrecharclass.pod has a list of all the characters that are white space,
# which needs to be updated if there are changes.  A quick way to check if
# there have been changes would be to see if the number of such characters
# listed in perluniprops.pod (generated by running mktables) for the property
# \p{White_Space} is no longer 25.  Further investigation would then be
# necessary to classify the new characters as horizontal and vertical.
#
# The code in regexec.c for the \X match construct is intimately tied to the
# regular expression in UAX #29 (http://www.unicode.org/reports/tr29/).  You
# should see if it has changed, and if so, regexec.c should be modified.  The
# current one is
# ( CRLF
# | Prepend* ( RI-sequence | Hangul-Syllable | !Control )
#   ( Grapheme_Extend | SpacingMark )*
# | . )
#
# mktables has many checks to warn you if there are unexpected or novel things
# that it doesn't know how to handle.
#
# Module::CoreList should be changed to include the new release
#
# Also, you should regen l1_char_class_tab.h, by
#
# perl regen/mk_L_charclass.pl
#
# and, regen charclass_invlists.h by
#
# perl regen/mk_invlists.pl
#
# Finally:
#
# 	p4 submit
#
# --
# jhi@iki.fi; updated by nick@ccl4.org, public@khwilliamson.com
