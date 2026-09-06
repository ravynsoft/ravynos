
                .globl  _NSobjc_msgSendv
                .text
_NSobjc_msgSendv:
                pushq	%rbp
                movq	%rsp, %rbp
                subq    $32, %rsp
                movq    %rdi, (%rbp)            // id target
                movq    %rsi, -8(%rbp)          // SEL selector
                movq    %rdx, -16(%rbp)         // arg frame size
                movq    %rcx, -24(%rbp)         // arg frame pointer

// IMP lookUpImpOrForward(struct objc_object, struct objc_selector,
//      struct objc_class, int flags)

                movq    (%rbp), %rdi            // target
                movq    -8(%rbp), %rsi          // sel
                movq    (%rdi), %rdx            // isa, objc_class
                movq    $11, %rcx               // LOOKUP_{INIT|RESOLVER|NIL}
                call	_lookUpImpOrForward

// on return, rax contains the IMP to call. return early if it's nil.
// otherwise, keep it there for the call

                cmpq    $0, %rax                // check if imp lookup was nil
                jz      Lmarshal
                jmp     Lexit

// object & selector are still in rdi, rsi but we're going to trash them
// and use the values from the argument frame.
// marshal the arg frame into registers for the call.
// if there are more than 6 args, the excess are put on the stack.

Lmarshal:       movq    -16(%rbp), %r10         // arg frame size
                movq    -24(%rbp), %r11         // arg frame ptr
                movq    %r11, %r12
                addq    %r10, %r12              // ptr + size = end of frame

                movq    (%r10), %rdi
                addq    $8, %r10
                cmpq    %r10, %r12
                jle     Lcall

                movq    (%r10), %rsi
                addq    $8, %r10
                cmpq    %r10, %r12
                jle     Lcall

                movq    (%r10), %rdx
                addq    $8, %r10
                cmpq    %r10, %r12
                jle     Lcall

                movq    (%r10), %rcx
                addq    $8, %r10
                cmpq    %r10, %r12
                jle     Lcall

                movq    (%r10), %r8
                addq    $8, %r10
                cmpq    %r10, %r12
                jle     Lcall

                movq    (%r10), %r9
                addq    $8, %r10
                cmpq    %r10, %r12
                jle     Lcall

                subq    %r10, %r12              // end of frame - ptr == 0?
                cmpq    $0, %r12
                jz      Lcall

                subq    %r12, %rsp              // nope, reserve space to push
L0:             pushq   (%r10)                  // seventh and beyond
                addq    $8, %r10
                cmpq    %r10, %r12
                jl      L0

Lcall:          
                pushq   %rax
                pushq   %rdi
                leaq    msg3(%rip), %rdi
                call    _printf
                popq    %rdi
                popq    %rax

                pushq   %rax
                pushq   %rdi
                leaq    msg1(%rip), %rdi
                call    _printf
                popq    %rdi
                popq    %rax

                call    *%rax                   // all good - call the IMP!

Lexit:          addq    $32, %rsp
                addq    %r12, %rsp
                popq    %rbp
                ret

                .data
format1:        .asciz  "[%s %s] size %lx frame %lx\n"
msg1:           .asciz  "rsi %lx rdx %lx rcx %lx r8 %lx r9 %lx\n"
msg2:           .asciz  "pushing args 7+\n"
msg3:           .asciz  "making call\n"
