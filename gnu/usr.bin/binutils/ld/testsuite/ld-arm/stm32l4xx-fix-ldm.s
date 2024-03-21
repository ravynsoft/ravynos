        .syntax unified
        .cpu cortex-m4
        .fpu fpv4-sp-d16
        .text
        .align  1
        .thumb
        .thumb_func
        .global _start
_start:
        @ LDM CASE #1 (used when rx is in upper_list)
        @ ldm rx, {...} ->
        @ ldm rx!, {lower_list}
        @ ldm rx,  {upper_list}
        @ b.w
        ldm.w r9, {r1-r9}

        @ LDM CASE #1 bis (used when rx is in upper_list and pc is
        @ in reglist)
        @ ldm rx, {...} ->
        @ ldm rx!, {lower_list}
        @ ldm rx,  {upper_list}
        ldm.w r9, {r1-r9, pc}

        @ LDM CASE #2 (used when rx is not in upper_list)
        @ ldm rx, {...} ->
        @ mov ry, rx where ry is the lowest register from upper_list
        @ ldm ry!, {lower_list}
        @ ldm ry,  {upper_list}
        @ b.w
        ldm.w r0, {r1-r9}

        @ LDM CASE #2 bis (used when rx is in lower_list)
        @ ldm rx, {...} ->
        @ mov ry, rx where ry is the lowest register from upper_list
        @ ldm ry!, {lower_list}
        @ ldm ry,  {upper_list}
        @ b.w
        ldm.w r1, {r1-r9}

        @ LDM CASE #2 ter (used when rx is not in upper_list and pc is
        @ in reglist)
        @ ldm rx, {...} ->
        @ mov ry, rx where ry is the lowest register from upper_list
        @ ldm ry!, {lower_list}
        @ ldm ry,  {upper_list}
        ldm.w r0, {r1-r9, pc}

        @ LDM CASE #2 quater (used when rx is in lower_list and pc is
        @ in reglist)
        @ ldm rx, {...} ->
        @ mov ry, rx where ry is the lowest register from upper_list
        @ ldm ry!, {lower_list}
        @ ldm ry,  {upper_list}
        ldm.w r1, {r1-r9, pc}

        @ LDM CASE #3 (used when rx is not in upper_list)
        @ ldm rx, {...} ->
        @ ldm rx!, {lower_list}
        @ ldm rx!, {upper_list}
        @ b.w
        @ Write-back variant are unpredictable when rx appears also in
        @ the loaded registers
        ldm.w r0!, {r1-r9}

        @ LDM CASE #3 bis (used when rx is not in upper_list and pc is
        @ in reglist)
        @ ldm rx, {...} ->
        @ ldm rx!, {lower_list}
        @ ldm rx!, {upper_list}
        ldm.w r0!, {r1-r9, pc}

        @ LDM CASE #4 (used when pc is not in reglist and rx is in
        @ lower_list)
        @ ldmb rx, {...} ->
        @ ldmb rx!, {upper_list}
        @ ldmb rx,  {lower_list}
        ldmdb.w r1, {r1-r9}

        @ LDM CASE #5 (used when pc is not in reglist and rx is not in
        @ lower_list)
        @ It looks like it this mean that it could be in upper_list or not
        @ ldmdb rx, {...} ->
        @ mov ry, rx where ry is the lowest register from lower_list
        @ ldmdb ry!, {upper_list}
        @ ldmdb ry , {lower_list}
        @ b.w
        ldmdb.w sl, {r1-r9}

        @ LDM CASE #5 bis (used when pc is not in reglist and rx is in
        @ upper_list)
        @ ldmdb rx, {...} ->
        @ mov ry, rx where ry is the lowest register from lower_list
        @ ldmdb ry!, {upper_list}
        @ ldmdb ry , {lower_list}
        @ b.w
        ldmdb.w r9, {r1-r9}

        @ LDM CASE #6 (used when pc is in reglist and rx is in
        @ upper_list)
        @ ldmdb rx, {...} ->
        @ sub rx, rx, #size (lower_list + upper_list)
        @ ldm rx!, {lower_list}
        @ ldm rx,  {upper_list}
        @ This case reverses the load order
        ldmdb.w r9, {r1-r9, pc}

        @ LDM CASE #6 bis (used when pc is in reglist and rx is in
        @ lower_list)
        @ ldmdb rx, {...} ->
        @ sub rx, rx, #size (lower_list + upper_list)
        @ ldm rx!, {lower_list}
        @ ldm rx,  {upper_list}
        ldmdb.w r1, {r1-r9, pc}

        @ LDM CASE #7 (used when pc is in reglist and rx is not in
        @ upper_list)
        @ ldmdb rx, {...} ->
        @ sub ry, rx, #size (lower_list + upper_list) where ry is the lowest
        @ register of the upper list
        @ ldm ry!, {lower_list}
        @ ldm ry , {upper_list}
        @ This case reverses the load order
        ldmdb.w r0, {r1-r9, pc}

        @ LDM CASE #8 (used when pc is in not in reglist)
        @ ldmdb rx!, {...} ->
        @ ldm rx!, {upper_list}
        @ ldm rx!, {lower_list}
        @ b.w
        ldmdb.w r0!, {r1-r9}

        @ LDM CASE #9 (Used when pc is in reglist)
        @ ldmdb rx!, {...} ->
        @ sub rx, rx, #size (lower_list + upper_list)
        @ mov ry, rx where ry is the lowest register from upper_list
        @ ldm ry!, {lower_list}
        @ ldm ry , {upper_list}
        ldmdb.w r0!, {r1-r9, pc}

        @ POP CASE #1 (list does not include pc)
        @ pop {...} -> pop {lower_list} pop {upper_list}
        @ b.w
        pop {r0-r9}

        @ POP CASE #2 (list includes PC)
        @ pop {...} -> pop {lower_list} pop {upper_list}
        pop {r0-r9, pc}
