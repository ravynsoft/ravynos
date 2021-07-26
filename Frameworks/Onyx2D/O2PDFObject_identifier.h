/* Copyright (c) 2007 Christopher J. W. Lloyd

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#import <Onyx2D/O2PDFObject.h>

typedef enum {
    O2PDFIdentifierUnknown,
    O2PDFIdentifier_true,
    O2PDFIdentifier_false,
    O2PDFIdentifier_null,
    O2PDFIdentifier_f,
    O2PDFIdentifier_n,
    O2PDFIdentifier_R,
    O2PDFIdentifier_xref,
    O2PDFIdentifier_trailer,
    O2PDFIdentifier_startxref,
    O2PDFIdentifier_obj,
    O2PDFIdentifier_endobj,
    O2PDFIdentifier_stream,
    O2PDFIdentifier_endstream,

    // postscript calculator
    O2PDFIdentifier_abs,
    O2PDFIdentifier_add,
    O2PDFIdentifier_atan,
    O2PDFIdentifier_ceiling,
    O2PDFIdentifier_cos,
    O2PDFIdentifier_cvi,
    O2PDFIdentifier_cvr,
    O2PDFIdentifier_div,
    O2PDFIdentifier_exp,
    O2PDFIdentifier_floor,
    O2PDFIdentifier_idiv,
    O2PDFIdentifier_ln,
    O2PDFIdentifier_log,
    O2PDFIdentifier_mod,
    O2PDFIdentifier_mul,
    O2PDFIdentifier_neg,
    O2PDFIdentifier_round,
    O2PDFIdentifier_sin,
    O2PDFIdentifier_sqrt,
    O2PDFIdentifier_sub,
    O2PDFIdentifier_truncate,

    O2PDFIdentifier_and,
    O2PDFIdentifier_bitshift,
    O2PDFIdentifier_eq,
    O2PDFIdentifier_ge,
    O2PDFIdentifier_gt,
    O2PDFIdentifier_le,
    O2PDFIdentifier_lt,
    O2PDFIdentifier_ne,
    O2PDFIdentifier_not,
    O2PDFIdentifier_or,
    O2PDFIdentifier_xor,

    O2PDFIdentifier_if,
    O2PDFIdentifier_ifelse,

    O2PDFIdentifier_copy,
    O2PDFIdentifier_dup,
    O2PDFIdentifier_exch,
    O2PDFIdentifier_index,
    O2PDFIdentifier_pop,
    O2PDFIdentifier_roll,
} O2PDFIdentifier;

O2PDFIdentifier O2PDFClassifyIdentifier(const char *bytes, unsigned length);

@interface O2PDFObject_identifier : O2PDFObject {
    O2PDFIdentifier _identifier;
    unsigned _length;
    char *_bytes;
}

+ pdfObjectWithIdentifier:(O2PDFIdentifier)identifier name:(const char *)bytes length:(unsigned)length;

- (O2PDFIdentifier)identifier;

- (const char *)name;

@end
