.global fn

.text

fn:
    .cfi_startproc
    mov     $0x1234, %rax
    mov     %rsp, %r10
    push    %rax
    mov     %rsp, %r11
    mov     (%rsp), %r12
    pop     %rax
    mov     (%rsp), %r12
    ret
    .cfi_endproc

.cfi_startproc
.cfi_sections .eh_frame
fn2:
    push %rsp
    .cfi_def_cfa_offset 16
    sub $0x1000, %rsp
    movq $0x1234, (%rsp)
    add 0x1000, %rsp
    pop %rsp
    .cfi_def_cfa_offset 8
    ret
.cfi_endproc

