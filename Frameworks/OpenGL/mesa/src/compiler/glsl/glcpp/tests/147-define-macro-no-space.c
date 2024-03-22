/* The GLSL specification is not specific about how to handle a non-space
 * character separating a macro identifier from the replacement list. It says
 * only "as is standard for C++ preprocessors". GCC accepts these and warns of
 * "missing whitespace". So we'll accept these, (though we don't warn).
 *
 * Note: 'O' is intentionally omitted to leave room for "octothorpe" if we
 *       decide it should be legal to use a hash here, (in fact, hash has no
 *       legal use as the first token in a macro replacement list, but one
 *       could argue that that could still be allowed if the macro were never
 *       instantiated).
 */
#define A& ampersand
#define B! bang
#define C, comma
#define D/ divider
#define E= equals
#define F. full stop
#define G> greater than
#define H- hyphen
#define I+ incrementor
#define J[ JSON array
#define K} kurly brace?
#define L< less than
#define M{ moustache
#define N^ nose
#define P) parenthesis (right)
#define Q? question mark
#define R% ratio indicator
#define S] square bracket (right)
#define T~ tilde
#define U: umlaut?
#define V| vertical bar
#define W; wink
#define X* X (as multiplication)
A
B
C
D
E
F
G
H
I
J
K
L
M
N
P
Q
R
S
T
U
V
W
X

