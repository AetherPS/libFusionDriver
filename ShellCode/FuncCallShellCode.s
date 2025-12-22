BITS 64
DEFAULT REL

; ============================================================================
; Header Structure (maps to C++ struct FuncCallHeader)
; ============================================================================
magic:              db 'CALL'           ; 0x00: Magic identifier (4 bytes)
entry:              dq shellcode        ; 0x04: Entry point (8 bytes)
funcPtr:            dq 0                ; 0x0C: Function to call (8 bytes)
returnValue:        dq 0                ; 0x14: Return value storage (8 bytes)
argCount:           dq 0                ; 0x1C: Number of arguments (8 bytes)
argBuffer:          times 16 dq 0       ; 0x24: Argument buffer - 16 args max (128 bytes)
scratchOffset:      dq 0                ; 0xA4: Current offset into scratch memory (8 bytes)
scratchMemory:      times 4096 db 0     ; 0xAC: Scratch memory for strings/data (4096 bytes)
                                        ; Total: 0x10AC (4268 bytes)

; ============================================================================
; call_func - Generic function caller
;
; Reads from header structure:
;   funcPtr   - Function to call
;   argCount  - Number of arguments (0-16)
;   argBuffer - Arguments in order [arg1, arg2, arg3, arg4, arg5, arg6, ...]
;
; System V AMD64 ABI:
;   Args 1-6: rdi, rsi, rdx, rcx, r8, r9
;   Args 7+:  pushed on stack (right to left)
;
; Returns: rax (also stored in returnValue)
; ============================================================================
call_func:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    
    ; Load parameters from header
    mov r15, qword [funcPtr]
    mov r14, qword [argCount]
    lea rbx, [argBuffer]
    
    ; Clear all arg registers
    xor rdi, rdi
    xor rsi, rsi
    xor rdx, rdx
    xor rcx, rcx
    xor r8, r8
    xor r9, r9
    
    ; No args? Just call
    test r14, r14
    jz .do_call
    
    ; arg1 -> rdi
    mov rdi, qword [rbx]
    cmp r14, 1
    je .do_call
    
    ; arg2 -> rsi
    mov rsi, qword [rbx + 8]
    cmp r14, 2
    je .do_call
    
    ; arg3 -> rdx
    mov rdx, qword [rbx + 16]
    cmp r14, 3
    je .do_call
    
    ; arg4 -> rcx
    mov rcx, qword [rbx + 24]
    cmp r14, 4
    je .do_call
    
    ; arg5 -> r8
    mov r8, qword [rbx + 32]
    cmp r14, 5
    je .do_call
    
    ; arg6 -> r9
    mov r9, qword [rbx + 40]
    cmp r14, 6
    jbe .do_call
    
    ; Handle stack arguments (args 7+)
    mov r12, r14
    sub r12, 6              ; r12 = number of stack args
    
    ; Align stack to 16 bytes if odd number of stack args
    mov r13, r12
    and r13, 1
    shl r13, 3
    sub rsp, r13
    
    ; Push stack args in reverse order (arg N, arg N-1, ..., arg 7)
    lea r13, [rbx + r14*8 - 8]
.push_loop:
    push qword [r13]
    sub r13, 8
    dec r12
    jnz .push_loop

.do_call:
    ; Ensure 16-byte alignment before call
    mov r12, rsp
    and rsp, -16
    push r12
    sub rsp, 8
    
    call r15
    
    add rsp, 8
    pop rsp
    
    ; Store return value in header
    mov qword [returnValue], rax
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; ============================================================================
; Main shellcode entry
; ============================================================================
shellcode:
    call call_func
    ret