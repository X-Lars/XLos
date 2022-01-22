#pragma once

#define SYS_MAX_CALLS 2

void SysCall01(void)
{
    uint16 x = 10, y = 6;
    PrintString("SYSTEM CALL: 01", &x, &y);
}

void SysCall02(void)
{
    uint16 x = 10, y = 6;
    PrintString("SYSTEM CALL: 02", &x, &y);
}
void *SysCalls[SYS_MAX_CALLS] = {SysCall01, SysCall02};
// naked: only asm
__attribute__ ((naked)) void SysDispatcher(void)
{
    __asm__ __volatile__(".intel_syntax noprefix\n"
                         ".equ MAX_CALLS, 2\n"
                         "cmp eax, MAX_CALLS-1\n"
                         "ja invalid_syscall\n"
                         "push eax\n"
                         "push gs\n"
                         "push fs\n"
                         "push es\n"
                         "push ds\n"
                         "push ebp\n"
                         "push edi\n"
                         "push esi\n"
                         "push edx\n"
                         "push ecx\n"
                         "push ebx\n"
                         "push esp\n"
                         "call [SysCalls+eax*4]\n"
                         "add esp, 4\n"
                         "pop ebx\n"
                         "pop ecx\n"
                         "pop edx\n"
                         "pop esi\n"
                         "pop edi\n"
                         "pop ebp\n"
                         "pop ds\n"
                         "pop es\n"
                         "pop fs\n"
                         "pop gs\n"
                         "add esp, 4\n"
                         "iretd\n"
                         "invalid_syscall:\n"
                         "iretd\n"
                         ".att_syntax");
    
}

