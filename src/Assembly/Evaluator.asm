bits 64
default rel

; External Functions
extern printf

; Account for Operating System
%ifidn __OUTPUT_FORMAT__, win64
    %idefine Arg1 rcx        ; '&Operations' (std::vector<Operation>*)
    %idefine Arg1_32 ecx
    %idefine Arg1_8 cl
    %idefine Arg2 rdx        ; '&Operations' (std::vector<Operation>*)
    %idefine Arg2_32 edx
    %idefine Arg2_8 dl
%elifidn __OUTPUT_FORMAT__, elf64
    %idefine Arg1 rdi        ; '&Operations' (std::vector<Operation>*)
    %idefine Arg1_32 edi
    %idefine Arg1_8 dil
    %idefine Arg2 rsi        ; '&Operations' (std::vector<Operation>*)
    %idefine Arg2_32 esi
    %idefine Arg2_8 sil
%else
    %error "Unsupported output format: expected win64 or elf64"
%endif

; Macros
%define Offset(base, idx, scale) [base + idx * scale]

%macro PushAll 1-*  ; '*' means any amount
    %rep %0         ; Repeat these instructions for every Argument passed into the macro
        push %1     ; Push the Argument currently in Argument #1
        %rotate 1   ; Rotates 1 left
    %endrep
%endmacro

%macro PopAll 1-*   ; '*' means any amount
    %rep %0         ; Repeat these instructions for every Argument passed into the macro
        %rotate -1  ; Push the Argument currently in Argument #1
        pop %1      ; Rotates 1 left
    %endrep
%endmacro

; Data
section .data
    OPCodePrintMsg db "OPCode: %d", 10, 0
    InputPrintMsg db "Input: %d", 10, 0

; Main Code
section .text
    ; static bool LogicEvaluator::EvaluateByteCode(vector<Operation, std::allocator<Operation>>& Operations)
    ; _Z - Manadator C++ Prefix (Linux & Windows, add extra underscore for MacOS)
    ; N - Start of Nested Namespace
    ; 14LogicEvaluator - Class Name prefixed by length
    ; 16EvaluateByteCode - Function Name prefixed by length
    ; E - End of nested namespace
    ; R - Passed in by reference
    ; St - std::
    ; 6vector - The class name prefixed by length
    ; I - Start of template Argument list
    ; 9Operation - Class Name prefixed by length
    ; Sa - std::allocator
    ; I - Start of template Argument list
    ; S1_ - Backreference (Index = 2) to 9Operation
    ; EE - End of both template Argument list
    global _ZN14LogicEvaluator16EvaluateByteCodeERSt6vectorI9OperationSaIS1_EE
    global _ZN14LogicEvaluator26EvaluateAndCompareByteCodeERSt6vectorI9OperationSaIS1_EES4_
    global HandleOP
    
    _ZN14LogicEvaluator16EvaluateByteCodeERSt6vectorI9OperationSaIS1_EE:
        ; Arg - vector<Operation, std::allocator<Operation>> Operations
        PushAll r12, r14, r15
        
        %ifidn __OUTPUT_FORMAT__, win64
            sub rsp, 288        ; 32B shadow + 256B eval stack (3 pushes = 24B + 8B ret = 32B, aligned)
            lea r12, [rsp + 32] ; r12 = evaluation stack pointer
        %elifidn __OUTPUT_FORMAT__, elf64
            sub rsp, 256        ; 256B eval stack (3 pushes = 24B + 8B ret = 32B, aligned)
            mov r12, rsp        ; r12 = evaluation stack pointer
        %endif
        
        ; Set Variables
        mov r14, [Arg1]         ; r14 = Operations._M_start (Operation* array base)
        mov r15, [Arg1 + 8]     ; r15 = Operations._M_finish (one-past-end pointer)
        
        ; Loop through the Operations
        OperationsLoop:
            cmp r14, r15                    ; Check if the source is at the destination
            je OperationsLoopDone           ; Jump to the done section if at destination
            
            mov Arg1_32, [r14]              ; Arg1_32 = CurrentOp.OPCode (4 bytes at offset 0)
            movzx Arg2_32, byte [r14 + 4]   ; Arg2_32 = CurrentOp.Input (1 byte at offset 4, zero-extended)
            call HandleOP
            
            ; Move the source ptr 8 bytes (4 Bytes for OPCode + 1 Byte for bool Input + 3 bytes for alignment)
            add r14, 8 
            jmp OperationsLoop
        OperationsLoopDone:
            movzx eax, byte [r12 - 1]           ; Return top boolean from eval stack in AL
            %ifidn __OUTPUT_FORMAT__, win64
                add rsp, 288
            %elifidn __OUTPUT_FORMAT__, elf64
                add rsp, 256
            %endif
            
            PopAll r12, r14, r15
            ret
    
    _ZN14LogicEvaluator26EvaluateAndCompareByteCodeERSt6vectorI9OperationSaIS1_EES4_:
        ; Arg1 - vector<Operation>& OperationsA
        ; Arg2 - vector<Operation>& OperationsB
        PushAll rbx, r12, r13, r14, r15
        
        %ifidn __OUTPUT_FORMAT__, win64
            sub rsp, 544            ; 32B shadow + 256B Stack A + 256B Stack B (5 pushes = 40B + 8B ret = 48B, aligned)
            lea r8, [rsp + 32]      ; r8 = current Stack A pointer
            lea r9, [rsp + 288]     ; r9 = current Stack B pointer
        %elifidn __OUTPUT_FORMAT__, elf64
            sub rsp, 512            ; 256B Stack A + 256B Stack B (5 pushes = 40B + 8B ret = 48B, aligned)
            mov r8, rsp             ; r8 = current Stack A pointer
            lea r9, [rsp + 256]     ; r9 = current Stack B pointer
        %endif
        
        mov r14, [Arg1]             ; r14 = OperationsA._M_start
        mov r15, [Arg1 + 8]         ; r15 = OperationsA._M_finish
        mov rbx, [Arg2]             ; rbx = OperationsB._M_start
        mov r13, [Arg2 + 8]         ; r13 = OperationsB._M_finish
        
        ; Single loop stepping through both OperationsA and OperationsB
        .CompareLoop:
            ; Step OperationsA if not finished
            cmp r14, r15
            je .CheckB
            
            mov Arg1_32, [r14]
            movzx Arg2_32, byte [r14 + 4]
            mov r12, r8             ; Load Stack A pointer for HandleOP
            call HandleOP
            mov r8, r12             ; Save updated Stack A pointer
            add r14, 8
            
        .CheckB:
            ; Step OperationsB if not finished
            cmp rbx, r13
            je .CheckBothDone
            
            mov Arg1_32, [rbx]
            movzx Arg2_32, byte [rbx + 4]
            mov r12, r9             ; Load Stack B pointer for HandleOP
            call HandleOP
            mov r9, r12             ; Save updated Stack B pointer
            add rbx, 8
            
        .CheckBothDone:
            cmp r14, r15
            jne .CompareLoop
            cmp rbx, r13
            jne .CompareLoop
            
        ; Compare top boolean on Stack A ([r8 - 1]) with top boolean on Stack B ([r9 - 1])
        xor eax, eax
        mov dl, [r8 - 1]
        cmp dl, [r9 - 1]
        sete al
        
        %ifidn __OUTPUT_FORMAT__, win64
            add rsp, 544
        %elifidn __OUTPUT_FORMAT__, elf64
            add rsp, 512
        %endif
        
        PopAll rbx, r12, r13, r14, r15
        ret
    
    HandleOP:
        cmp Arg1_32, 7
        je .push
        ja .ret                     ; unknown opcodes ignored
        cmp Arg1_32, 2
        je .not

        ; Binary ops: pop two values
        movzx eax,  byte [r12 - 1]
        movzx r10d, byte [r12 - 2]
        sub r12, 2

        cmp Arg1_32, 0
        je .and
        cmp Arg1_32, 1
        je .or
        cmp Arg1_32, 3
        je .xor
        cmp Arg1_32, 4
        je .xnor
        cmp Arg1_32, 6
        je .nor
        .nand:
            and eax, r10d
            xor eax, 1
            jmp .store
        .nor:
            or eax, r10d
            xor eax, 1
            jmp .store
        .and:
            and eax, r10d
            jmp .store
        .or:
            or eax, r10d
            jmp .store
        .xor:
            xor eax, r10d
            jmp .store
        .xnor:
            xor eax, r10d
            xor eax, 1
        .store:
            mov [r12], al
            inc r12
            ret
        .not:
            xor byte [r12 - 1], 1
            ret
        .push:
            mov [r12], Arg2_8
            inc r12
        .ret:
            ret
