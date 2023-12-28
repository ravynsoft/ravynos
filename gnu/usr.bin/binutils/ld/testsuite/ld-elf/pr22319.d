#name: PR 22319 - required undefined symbols in output
#ld: -u undefined_symbol -e 0
#nm: -u

[ 	]+U+[ 	]+undefined_symbol
#pass

