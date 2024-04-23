	.section .text

	.global _objc_msgSendv

_objc_msgSendv:
	// Prologue
	stp    x29, x30, [sp, #-16]!    // Save frame pointer and link register
	mov    x29, sp                   // Set up frame pointer

	// Save callee-saved registers
	stp    x19, x20, [sp, #0]        // Save x19 and x20
	stp    x21, x22, [sp, #16]       // Save x21 and x22
	stp    x23, x24, [sp, #32]       // Save x23 and x24
	stp    x25, x26, [sp, #48]       // Save x25 and x26
	stp    x27, x28, [sp, #64]       // Save x27 and x28

	// Move the method pointer (selector) into x0
	mov    x0, x1                    // Assuming the selector is passed in x1

	// Move the receiver object pointer into x19
	mov    x19, x2                   // Assuming the receiver object pointer is passed in x2

	// Call objc_msg_lookup() to get the IMP (method implementation pointer)
	bl     _objc_msg_lookup

	// Restore callee-saved registers
	ldp    x19, x20, [sp, #0]        // Restore x19 and x20
	ldp    x21, x22, [sp, #16]       // Restore x21 and x22
	ldp    x23, x24, [sp, #32]       // Restore x23 and x24
	ldp    x25, x26, [sp, #48]       // Restore x25 and x26
	ldp    x27, x28, [sp, #64]       // Restore x27 and x28

	// Epilogue
	ldp    x29, x30, [sp], #16       // Restore frame pointer and link register
	ret                              // Return from function
