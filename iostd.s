section .text

Base            equ     10
BuffLen         equ     23
LF              equ     0x0A
Accurracy       equ     3

;============================================================================
;Description:   Read number from stdin and store it in RAX.
;----------------------------------------------------------------------------
;Params:        None
;----------------------------------------------------------------------------
;Returns:       [RAX]          - number * 1000 
;----------------------------------------------------------------------------
;Destroy:       [RBX], [RCX], [RDX], [RDI], [RSI], [R11], [R12]
;============================================================================

stdIN:          mov r12, rsp
                sub rsp, BuffLen        ; allocate memory
                xor rax, rax
                xor rdi, rdi
                mov rsi, rsp
                mov rdx, BuffLen
                syscall

                mov rcx, rax            ; input size
                xor rax, rax
                xor rbx, rbx
.Cycle:         dec rcx
.Next:          mov bl, byte [rsi]
                inc rsi
                cmp bl, '.'
                je .Dot
                cmp bl, '-'
                je .Minus
                sub bl, '0'
                mov rdx, 10
                mul rdx
                add rax, rbx
.Minus:         loop .Next
                mov rdx, 1000
                mul rdx
                jmp .Negative

.Dot:           dec rcx
                mov rdi, rcx
                cmp rcx, Accurracy
                jbe .DotNext
                mov rcx , Accurracy
.DotNext        mov bl, byte [rsi]
                inc rsi
                sub bl, '0'
                mov rdx, 10
                mul rdx
                add rax, rbx
                loop .DotNext
                cmp rdi, Accurracy
                jae .Return
                mov rcx, Accurracy
                sub rcx, rdi
.Align:         mov rdx, 10
                mul rdx
                loop .Align

.Negative       mov bl, '-'
                cmp byte [rsp], bl
                jne .Return
                neg rax

.Return:        mov rsp, r12
                ret

;============================================================================
;Description:   Print number from stack to stdout.
;----------------------------------------------------------------------------
;Params:        [RSP + 8]      - number * 1000.
;----------------------------------------------------------------------------
;Returns:       None
;----------------------------------------------------------------------------
;Destroy:       [RAX], [RBX], [RCX], [RDX], [RDI], [RSI], [R11], [R12]
;============================================================================

stdOUT:         mov rax, [rsp + 8]
                mov rsi, Base                           ; number system base
                mov rcx, BuffLen - 3
                mov r12, rsp                            ; save rsp
                sub rsp, BuffLen                        ; allocate memory
                mov byte [rsp + BuffLen - 1], LF

                cmp rax, 0
                je .Zero
                jl .Minus
                mov byte [rsp], 0
                ja .Next

.Zero:          mov byte [rsp], '0'
                mov byte [rsp + 1], LF
                mov rdx, 2
                mov rsi, rsp
                jmp .Return 

.Minus:         mov byte [rsp], '-'
                neg rax

.Next:          xor rdx, rdx
                div rsi
                add dl, '0'                             ; generate ASCII
                mov byte [rsp + rcx], dl                ; mov ASCII to buffer
                cmp rcx, BuffLen - 5                    ; 3 spec symbols + 3 numbers
                je .PutDot
                loop .Next
                jmp .PostProc
.PutDot:        dec rcx
                mov byte [rsp + rcx], '.'
                loop .Next

.PostProc:      cld
                mov al, '0'
                mov rdi, rsp
                inc rdi
                mov rcx, BuffLen - 8
.Clear:         scasb
                jne .ClearDot
                mov byte [rdi - 1], 0
                loop .Clear

.ClearDot:      std
                mov rdi, rsp
                add rdi, BuffLen - 3
                mov rcx, 3
.Cycle:         scasb
                jne .Print
                mov byte [rdi + 1], 0
                loop .Cycle
                mov byte [rsp + BuffLen - 6], 0              ; delete dot if .000

.Print:         mov rsi, rsp
                mov rdx, BuffLen
.Return:        mov rax, 1
                mov rdi, 1
                syscall
                mov rsp, r12
                ret 8
