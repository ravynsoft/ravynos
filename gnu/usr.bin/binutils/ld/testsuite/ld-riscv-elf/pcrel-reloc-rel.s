.data
# Makes sure "sym" doesn't end up at the beginning of ".data", as that makes it
# tough to then later detect it from scripts.
.global buf
buf:
    .fill 8192, 4, 1
.global sym
sym:
    .fill 8192, 4, 2
