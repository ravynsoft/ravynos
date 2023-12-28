        # eBPF tests for the CALL instruction

        .text
        call 0
        call 1
        call -2
        call 0xa
        call foo
foo:    call foo
        call foo
        call foo
