// syscall.c – Определяет реализацию механизма системных вызовов.
// Написано для  руководств по разработке ядра - автор James Molloy

#include "syscall.h"
#include "isr.h"

#include "monitor.h"

static void syscall_handler(registers_t *regs);

static void *syscalls[3] =
{
   &monitor_write,
   &monitor_write_hex,
   &monitor_write_dec,
};
u32int num_syscalls = 3;

void initialise_syscalls()
{
   // Регистрируем наш обработчик системных вызовов.
   register_interrupt_handler (0x80, &syscall_handler);
}

void syscall_handler(registers_t *regs)
{
   // Сначала проверяем, является ли допустимым запрашиваемый номер системного вызова.
   // Номер системного вызова находится в EAX.
   if (regs->eax >= num_syscalls)
       return;

   // Вычисляем место, где находится запрашиваемый системный вызов.
   void *location = syscalls[regs->eax];

   // Нам неизвестно, сколько параметров необходимо функции, поэтому мы  просто
   // помещаем их в стек в правильном порядке. Функция может использовать все эти 
   // параметры, если они потребуются, а затем мы можем убрать их из стека.
   int ret;
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
   " : "=a" (ret) : "r" (regs->edi), "r" (regs->esi), "r" (regs->edx), "r" (regs->ecx), "r" (regs->ebx), "r" (location));
   regs->eax = ret;
}