#name: Check require-defined with an undefined symbol
#source: require-defined.s
#ld: -e _start --gc-sections --require-defined=xxx
#error: required symbol `xxx' not defined
