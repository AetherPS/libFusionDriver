BITS 64
DEFAULT REL

magic: db 'SHEL'
entry: dq shellcode

thr_initial: dq 0
ascePthreadCreate: dq 0
ascePthreadJoin: dq 0
aShellCode: dq 0
ShellCodeComplete: db 0

; Local vars
hThread: dq 0
str_threadName: db 'ShellCode Thread', 0

; Main shellcode function.
shellcode:
	; load thread into fs
	mov rdi, qword [thr_initial]
	mov rsi, qword [rdi]
	mov rdi, qword [rsi + 0x1E0]
	call amd64_set_fsbase

	; create thread
	lea r8, [str_threadName]
	mov rcx, 0
	mov rdx, qword [aShellCode]
	xor rsi, rsi
	lea rdi, [hThread]
	mov r12, qword [ascePthreadCreate]
	call r12
	
	; join thread
	mov rdi, [hThread]
	xor rsi, rsi
	mov r12, qword [ascePthreadJoin]
	call r12

	; Set the Flag we are done.
    mov byte [ShellCodeComplete], 1

	; Shutdown the thread.
	mov rdi, 0
	call sys_thr_exit
	retn

; Sub function
sys_thr_exit:
	mov rax, 431
	mov r10, rcx
	syscall
	retn

; Sub function
sys_sysarch:
	mov rax, 165
	mov r10, rcx
	syscall
	retn

; Sub function
amd64_set_fsbase:
	push rbp
	mov rbp, rsp
	push rbx
	sub rsp, 0x18

	mov [rbp - 0x18], rdi

	lea rsi, [rbp - 0x18]
	mov edi, 129
	call sys_sysarch

	add rsp, 0x18
	pop rbx
	pop rbp
	retn