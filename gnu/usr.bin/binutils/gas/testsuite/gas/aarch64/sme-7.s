/* SME Extension (LDR and STR instructions).  */
/* Load vector to ZA array.  */
ldr za[w12, 0], [x0]
ldr za[w12, 0], [sp]
ldr za[w12, 0], [x0, #0, mul vl]
ldr za[w12, 0], [sp, #0, mul vl]
ldr za[w15, 0], [x17]
ldr za[w13, 9], [x17, #9, mul vl]
ldr za[w15, 15], [x17, #15, mul vl]
ldr za[w15, 15], [sp, #15, mul vl]

/* Store vector from ZA array.  */
str za[w12, 0], [x0]
str za[w12, 0], [sp]
str za[w12, 0], [x0, #0, mul vl]
str za[w12, 0], [sp, #0, mul vl]
str za[w15, 0], [x17]
str za[w13, 9], [x17, #9, mul vl]
str za[w15, 15], [x17, #15, mul vl]
str za[w15, 15], [sp, #15, mul vl]

/* Register aliases.  */
foo .req w12
bar .req w15

ldr za[foo, 0], [sp, #0, mul vl]
str za[bar, 0], [x17]
