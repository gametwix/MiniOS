// Добавил заголовочные файлы
#include "paging.h"
#include "kheap.h"

// The kernel's page directory
page_directory_t *kernel_directory=0;

// The current page directory
page_directory_t *current_directory=0;

// Набор bitset для фреймов.
u32int *frames;
u32int nframes;

// Определено в kheap.c
extern u32int placement_address;

// В алгоритмах для bitset используются макросы.
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

// Статическая функция для установки бита в наборе bitset для фреймов
static void set_frame(u32int frame_addr)
{
   u32int frame = frame_addr/0x1000;
   u32int idx = INDEX_FROM_BIT(frame);
   u32int off = OFFSET_FROM_BIT(frame);
   frames[idx] |= (0x1 << off);
}

// Статическая функция для сброса бита в наборе bitset для фреймов
static void clear_frame(u32int frame_addr)
{
   u32int frame = frame_addr/0x1000;
   u32int idx = INDEX_FROM_BIT(frame);
   u32int off = OFFSET_FROM_BIT(frame);
   frames[idx] &= ~(0x1 << off);
}

// Статическая функция для проверки, установлен ли бит
static u32int test_frame(u32int frame_addr)
{
   u32int frame = frame_addr/0x1000;
   u32int idx = INDEX_FROM_BIT(frame);
   u32int off = OFFSET_FROM_BIT(frame);
   return (frames[idx] & (0x1 << off));
}

// Статическая функция для поиска первого свободного фрейма
static u32int first_frame()
{
   u32int i, j;
   for (i = 0; i < INDEX_FROM_BIT(nframes); i++)
   {
       if (frames[i] != 0xFFFFFFFF) // нечего не освобождаем, сразу выходим.
       {
           // по меньшей мере, здесь один свободный бит
           for (j = 0; j < 32; j++)
           {
               u32int toTest = 0x1 << j;
               if ( !(frames[i]&toTest) )
               {
                   return i*4*8+j;
               }
           }
       }
   }
}

// Функция выделения фрейма.
void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
   if (page->frame != 0)
   {
       return; // Фрейм уже выделен, сразу возвращаемся.
   }
   else
   {
       u32int idx = first_frame(); // idx теперь является индексом первого свободного фрейма.
       if (idx == (u32int)-1)
       {
           // PANIC это всего лишь макрос, которые выдает на экран сообщение, а затем переходит в бесконечный цикл.
           PANIC("No free frames!");
       }
       set_frame(idx*0x1000); // Этот фрейм теперь наш!
       page->present = 1; // Помечаем его как присутствующий.
       page->rw = (is_writeable)?1:0; // Можно ли для страницы выполнять запись?
       page->user = (is_kernel)?0:1; // Находится ли страница в пользовательском режиме?
       page->frame = idx;
   }
}

// Function to deallocate a frame.
void free_frame(page_t *page)
{
   u32int frame;
   if (!(frame=page->frame))
   {
       return; // Указанной страницы теперь фактически нет в выделенном фрейме!
   }
   else
   {
       clear_frame(frame); // фрейм теперь снова свободен.
       page->frame = 0x0; // Страницы теперь во фрейме нет.
   }
} 

void initialise_paging()
{
   // Размер физической памяти. Сейчас мы предполагаем,
   // что размер равен 16 MB.
   u32int mem_end_page = 0x1000000;

   nframes = mem_end_page / 0x1000;
   frames = (u32int*)kmalloc(INDEX_FROM_BIT(nframes));
   memset(frames, 0, INDEX_FROM_BIT(nframes));

   // Давайте создадим директорий страниц.
   kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
   memset(kernel_directory, 0, sizeof(page_directory_t));
   current_directory = kernel_directory;

   // Нам нужна карта идентичности (физический адрес = виртуальный адрес) с адреса
   // 0x0 до конца используемой памяти с тем, чтобы у нас к ним был прозрачный 
   // доступ как если бы страничная организация памяти не использовалась.
   // ЗАМЕТЬТЕ, что мы преднамеренно используем цикл while.
   // Внутри тела цикла мы фактически изменяем адрес placement_address
   // с помощью вызова функции kmalloc(). Цикл while используется здесь, т.к. выход
   // из цикла динамически, а не один раз после запуска цикла.
   int i = 0;
   while (i < placement_address)
   {
       // Код ядра можно читать из пользовательского режима, но нельзя в него записывать.
       alloc_frame( get_page(i, 1, kernel_directory), 0, 0);
       i += 0x1000;
   }
   // Прежде, чем включить страничное управление памятью, нужно зарегистрировать
   // обработчик некорректного обращения к памяти - page fault.
   register_interrupt_handler(14, page_fault);

   // Теперь включаем страничную организацию памяти!
   switch_page_directory(kernel_directory);
}

void switch_page_directory(page_directory_t *dir)
{
   current_directory = dir;
   asm volatile("mov %0, %%cr3":: "r"(&dir->tablesPhysical));
   u32int cr0;
   asm volatile("mov %%cr0, %0": "=r"(cr0));
   cr0 |= 0x80000000; // Enable paging!
   asm volatile("mov %0, %%cr0":: "r"(cr0));
}

page_t *get_page(u32int address, int make, page_directory_t *dir)
{
   // Помещаем адрес в индекс.
   address /= 0x1000;
   // Находим таблицу страниц, в которой есть этот адрес.
   u32int table_idx = address / 1024;
   if (dir->tables[table_idx]) // Если эта таблица уже назначена
   {
       return &dir->tables[table_idx]->pages[address%1024];
   }
   else if(make)
   {
       u32int tmp;
       dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
       memset(dir->tables[table_idx], 0, 0x1000);
       dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US.
       return &dir->tables[table_idx]->pages[address%1024];
   }
   else
   {
       return 0;
   }
}

void page_fault(registers_t regs)
{
   // Возникло прерывания неверного обращения к странице - page fault.
   // Адрес прерывания запоминается в регистре CR2.
   u32int faulting_address;
   asm volatile("mov %%cr2, %0" : "=r" (faulting_address));

   // Код ошибки подробно сообщит нам о том, что случилось.
   int present   = !(regs.err_code & 0x1); // Страница отсутствует
   int rw = regs.err_code & 0x2;           // Операция записи?
   int us = regs.err_code & 0x4;           // Процессор находится в пользовательском режиме?
   int reserved = regs.err_code & 0x8;     // В записи страницы переписаны биты, зарезервированные для нужд процессора?
   int id = regs.err_code & 0x10;          // Причина во время выборки инструкции?

   // Выдача сообщения об ошибке.
   monitor_write("Page fault! ( ");
   if (present) {monitor_write("present ");}
   if (rw) {monitor_write("read-only ");}
   if (us) {monitor_write("user-mode ");}
   if (reserved) {monitor_write("reserved ");}
   monitor_write(") at 0x");
   monitor_write_hex(faulting_address);
   monitor_write("\n");
   PANIC("Page fault");
}