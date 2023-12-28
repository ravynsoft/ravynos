# Test .arch .sse
.arch generic32
.arch .3dnowa
pswapd %mm1,%mm0
pminub %mm1,%mm0
