//
// task.h – Определяются структуры и прототипы, необходимые для многозадачности.
// Написано для  руководств по разработке ядра - автор James Molloy
//

#ifndef TASK_H
#define TASK_H

#include "common.h"
#include "paging.h"

#define KERNEL_STACK_SIZE 2048

// В этой структуре определяется задача 'task' - процесс.
typedef struct task
{
   int id;                // Идентификатор процесса ID.
   u32int esp, ebp;       // Указатели стека и базы.
   u32int eip;            // Указатель инструкции.
   page_directory_t *page_directory; // Директорий страниц.
   u32int kernel_stack; // Kernel stack location.
   struct task *next;     // Следующая задача в связном списке.
} task_t;

// Инициализируется система, поддерживающая многозадачность.
void initialise_tasking();

// Инициализируется таймером, в результате чего происходит смена работающего процесса.
void switch_task();

// Порождение нового процесса из текущего, для нового процесса выделяется другое
// пространство памяти.
int fork();

// В результате стек текущего процесса будет перемещен на новое место.
void move_stack(void *new_stack_start, u32int size);

// Возвращает pid текущего процесса.
int getpid();

void switch_to_user_mode();

#endif