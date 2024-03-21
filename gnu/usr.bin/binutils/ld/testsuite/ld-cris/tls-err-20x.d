#source: init.s
#source: tls-ie-9.s --pic
#source: tls-ld-6.s --pic
#source: tls-ie-10.s --pic
#source: tls-hx.s --pic
#as: --no-underscore --em=criself
#ld: -m crislinux
#error: \A[^\n]*: warning: cannot find entry symbol _start; defaulting to [0-9a-f]*\n[^\n]*: in function `tlsdsofn9':\n[^\n]*: undefined reference to `x1'\n[^\n]*: undefined reference to `x2'\Z

# Code coverage case similar to tls-e-20.d, except with an undefined
# reference.
