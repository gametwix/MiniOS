# Module for system calls in MiniOS

Controls switching between user mode and kernel mode. This system operates on the basis of system interrupts and is presented as a system call.

## Mode switching control algorithm user and kernel (secure)

![](/images/alg.jpg)

### GDT table

<table>
<tr>
<td>

</td>
<td>
Entry number in gdt
</td>
<td>
Base
</td>
<td>
Limit
</td>
<td>
Access
</td>
<td>
Flags + Limit
</td>
</tr>

<tr>
<td>
Null segment
</td>
<td>
0
</td>
<td>
0
</td>
<td>
0
</td>
<td>
0
</td>
<td>
0
</td>
</tr>

<tr bgcolor="#cb4335">
<td>
Code segment
</td>
<td>
1
</td>
<td>
0
</td>
<td>
0xFFFFFFFF
</td>
<td>
0x9A
</td>
<td>
0xCF
</td>
</tr>

<tr bgcolor="#cb4335">
<td>
Data segment
</td>
<td>
2
</td>
<td>
0
</td>
<td>
0xFFFFFFFF
</td>
<td>
0x92
</td>
<td>
0xCF
</td>
</tr>

<tr bgcolor="#52be80">
<td>
User mode code segment
</td>
<td>
3
</td>
<td>
0
</td>
<td>
0xFFFFFFFF
</td>
<td>
0xFA
</td>
<td>
0xCF
</td>
</tr>

<tr bgcolor="#52be80">
<td>
User mode data segment
</td>
<td>
4
</td>
<td>
0
</td>
<td>
0xFFFFFFFF
</td>
<td>
0xF2
</td>
<td>
0xCF
</td>
</tr>

<tr>
<td>
TSS
</td>
<td>
5
</td>
<td>
*TSS
</td>
<td>
size of TSS
</td>
<td>
0x9A
</td>
<td>
0x00
</td>
</tr>

</table>

### Structure GDT entry

![](/images/struct_gdt1.png)
![](/images/struct_gdt2.png)

### Highlights of the algorithm

<table>
<tr>
<td>

</td>
<td>
Code
</td>
<td>
Comments
</td>
</tr>

<tr>
<td>
1
</td>
<td>
<code>

init_descriptor_tables();\
asm volatile("sti");\
init_timer(50);\
initialise_paging();\
initialise_tasking();\
initialise_syscalls();\
</code>
</td>
<td>
Initialization of gdt and idt tables, timer, memory paging, multitasking, system calls. 
</td>
</tr>

<tr>
<td>
2
</td>
<td>
<code>

set_kernel_stack(current_task->kernel_stack+KERNEL_STACK_SIZE);
</code>
</td>
<td>
Set the start of the kernel stack right after the process stack. 
</td>
</tr>

<tr>
<td>
3.1
</td>
<td>
<code>

asm volatile(“cli”);
</code>
</td>
<td>
The processor stops processing interrupts.
</td>
</tr>

<tr>
<td>
3.2
</td>
<td>
<code>

asm volatile( \
      mov $0x23, %ax; \
      mov %ax, %ds; \
      mov %ax, %es; \
      mov %ax, %fs; \
      mov %ax, %gs; \
      mov %esp, %eax; \
      pushl $0x23; \
      pushl %esp; \
      pushf; \
      pushl $0x1B; \
      push $1f; \
      iret; \
    1: \
      ");

</code>
</td>
<td>
Set registers to run state in user segment
</td>
</tr>

<tr>
<td>
3.3
</td>
<td>
<code>

pop %eax; \
or $0x200, %eax; \
push %eax; 

</code>
</td>
<td>
Enable interrupts
</td>
</tr>

<tr>
<td>
4
</td>
<td>
<code>

isr_common_stub:
    pusha \
    \
    mov ax, ds \
    push eax  \
    \
    mov ax, 0x10 \
    mov ds, ax \
    mov es, ax \
    mov fs, ax \
    mov gs, ax \
    \
    call irq_handler

</code>
</td>
<td>
TSS switch cs and esp. Handling an interrupt.
</td>
</tr>

<tr>
<td>
5
</td>
<td>
<code>

void syscall_handler(registers_t *regs) { \
    if (regs->eax >= num_syscalls) \
        return; \
    void *location = syscalls[regs->eax]; \
    int ret; \
    asm volatile (" \
      push %1; \
      push %2; \
      push %3; \
      push %4; \
      push %5; \
      call *%6; \
      pop %%ebx; \
      pop %%ebx; \
      pop %%ebx; \
      pop %%ebx; \
      pop %%ebx; \
    " : "=a" (ret) : "r" (regs->edi), "r"  (regs->esi), "r" (regs->edx), "r" (regs->ecx), "r" (regs->ebx), "r" (location));\
    regs->eax = ret;\
}


</code>
</td>
<td>
Processing the system call (using the necessary pointer to the system function)
</td>
</tr>

<tr>
<td>
6
</td>
<td>
<code>

pop ebx \
    mov ds, bx \
    mov es, bx \
    mov fs, bx \
    mov gs, bx \
    \
    popa \
    add esp, 8 \
    sti \
    iret \

</code>
</td>
<td>
We return the old state of the registers. With the help of iret we return the old cs and esp.
</td>
</tr>
</table>

