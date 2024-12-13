// msgSendv-aarch64: glue to NSInvocation target calls
// Copyright (C) 2024 Zoe Knox <zoe@ravynsoft.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// The prototype of this function is:
//     id objc_msgSendv(id self, SEL selector, unsigned arg_size, void *arg_frame);
// So x0 = self, x1 = selector, x2 = arg_size, and x3 = arg_frame

	.section .text
	.global objc_msgSendv
	.type	objc_msgSendv, @function

objc_msgSendv:
	stp    x29, x30, [sp, #-128]!    // Save frame pointer and link register
	mov    x29, sp                   // Set up frame pointer

	// Save callee-saved registers
	stp    x19, x20, [sp, #16]       // Save x19 and x20
	stp    x21, x22, [sp, #32]       // Save x21 and x22
	stp    x23, x24, [sp, #48]       // Save x23 and x24
	stp    x25, x26, [sp, #64]       // Save x25 and x26
	stp    x27, x28, [sp, #80]       // Save x27 and x28

	// Our target object is in x0 and our selector is in x1 on entry
	stp    x0, x1, [sp, #96]	// Stash them on the stack
	mov    x19, x2			// Arg count
	mov    x20, x3			// Arg frame addr

	bl     object_getClass 		// Get the class of the object -> x0
	ldp    x9, x1, [sp, #96]	// Restore our clobbered x1
	bl     class_getMethodImplementation // Look up the IMP

	mov    x9, x0			// IMP -> x9 for safekeeping
	ldp    x0, x1, [sp, #96]	// Restore self, _cmd
	cmp    x19, #0
	beq    L2
	ldr    x2, [x20, x19]		// Load arg to reg
	sub    x19, x19, #8		// Decrement arg count
	cmp    x19, #0
	beq    L2
	ldr    x3, [x20, x19]
	sub    x19, x19, #8
	cmp    x19, #0
	beq    L2
	ldr    x4, [x20, x19]
	sub    x19, x19, #8
	cmp    x19, #0
	beq    L2
	ldr    x5, [x20, x19]
	sub    x19, x19, #8
	cmp    x19, #0
	beq    L2

L2:
	adr    lr, L3
	br     x9				// Call the IMP

L3:
	mov    x19, #0
	// Restore callee-saved registers
	ldp    x19, x20, [sp, #16]       // Restore x19 and x20
	ldp    x21, x22, [sp, #32]       // Restore x21 and x22
	ldp    x23, x24, [sp, #48]       // Restore x23 and x24
	ldp    x25, x26, [sp, #64]       // Restore x25 and x26
	ldp    x27, x28, [sp, #80]       // Restore x27 and x28

	ldp    x29, x30, [sp], #128      // Restore frame pointer and link register
	ret                              // Return from function

