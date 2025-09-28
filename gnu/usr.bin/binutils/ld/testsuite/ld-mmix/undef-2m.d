#source: undef-2.s
#source: start.s
#as: -x
#ld: -m mmo
#error: \A[^\n\r]*undefined reference to `undefd'\Z
