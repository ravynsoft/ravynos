/* Legal characters in GLSL are:
 *
 * Identifier characters:
 *
 *     Letters a-z
 *     Letters A-Z
 *     Underscore
 *     Numbers 0-9
 *
 * Punctuation:
 *
 *     Period, plus, dash, slash, asterisk, percent, angled brackets,
 *     square brackets, parentheses, braces, caret, vertical bar,
 *     ampersand, tilde, equals, exclamation point, colon, semicolon,
 *     comma, and question mark
 *
 * Special:
 *
 *     Number sign (as used in preprocessor)
 *
 *     Backslash just before newline as line continuation
 *
 * White space:
 *
 *     Space, horizontal tab, vertical tab, form feed, carriage-return,
 *     and line-feed.
 *
 * [GLSL Language Specficiation 4.30.6, section 3.1]
 *
 * In this file, we test each of these in turn as follows:
 *
 *    Identifier characters: All pass through unchanged
 *    Punctuation: All pass through unchanged
 *    Special: Empty directive replaced with blank line
 *             Line continuation merges two lines, then a blank line
 *    Whitespace: 4 horizontal space characters each replaced with space
 *                2 newline characters each replaced with a newline
 *
 */
abcdefghijklmnopqrstuvwxyz
ABCDEFGHIJKMLNOPQRSTUVWXYZ
_
0123456789
.
+
-
/
*
%
<
>
[
]
(
)
{
}
^
|
&
~
=
!
:
;
,
?
#
.\
.
. .
.	.
..
..
..
.
.
