;============================================================================
;Description:   Pop number from stack and print it to stdout.
;----------------------------------------------------------------------------
;Params:        [RSP + 8]      - number to translate.
;----------------------------------------------------------------------------
;Returns:       [RSI]          - adress of string, containing translated number.
;----------------------------------------------------------------------------
;Destroy:       [RAX], [RCX], [RDX]
;============================================================================

section .text

stdOUT:

; Translate number from stack to decimal ASCII reprsentation.
; Not using call to avoid make faster.

            mov rax, [rsp + 8]
            mov rsi, 10                             ; number system base
            mov rcx, 20
            push rbp
            mov rbp, rsp                            ; reserve buffer
            sub rsp, 20

.Next       xor rdx, rdx
            div rsi
            mov dl, [ASCIIstr + rdx]                ; generate ASCII
            mov [rsp + rcx - 1], dl                 ; mov ASCII to buffer
            loop .Next

            mov rsi, rsp
            mov rdx, 20
.Clear      cmp byte [rsi], '0'
            jne .Return
            dec rdx
            inc rsi
            jmp .Clear

.Return     mov rsp, rbp
            pop rbp

            mov rax, 1
            mov rdi, 1
            syscall


section .data
ASCIIstr: db "0123456789"
