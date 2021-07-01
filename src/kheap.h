#ifndef KHEAP_H
#define KHEAP_H

#include "common.h"
#include "ordered_array.h"

#define KHEAP_START         0xC0000000
#define KHEAP_INITIAL_SIZE  0x100000
#define HEAP_INDEX_SIZE   0x20000
#define HEAP_MAGIC        0x123890AB
#define HEAP_MIN_SIZE     0x70000

/**
  Информация о размере неиспользуемого фрагмента/используемого блока памяти
**/
typedef struct
{
   u32int magic;   // Магическое число, используемое для контроля ошибок и индентификации.
   u8int is_hole;   // 1 — если это неиспользуемый фрагмент памяти; 0 — если используемый блок
   u32int size;    // Размер блока, в том числе завершающая запись блока.
} header_t;

typedef struct
{
   u32int magic;     // Магическое число, такое же самое, как и в header_t.
   header_t *header; // Указатель на заголовок блока.
} footer_t;

typedef struct
{
   ordered_array_t index;
   u32int start_address; // Начало выделяемого пространства памяти.
   u32int end_address;   // Конец  выделяемого пространства памяти. Может быть до значения max_address.
   u32int max_address;   // Максимальный адрес, до которого куча может расширяться.
   u8int supervisor;     // Должны ли дополнительные страницы, запрашиваемые вами, использоваться только в режиме супервизора?
   u8int readonly;       // Должны ли дополнительные страницы, запрашиваемые вами, использоваться только в режиме чтения?
} heap_t;

/**
  Создаем новую кучу.
**/
heap_t *create_heap(u32int start, u32int end, u32int max, u8int supervisor, u8int readonly);
/**
  Allocates a contiguous region of memory 'size' in size. If page_align==1, it creates that block starting
  on a page boundary.
**/
void *alloc(u32int size, u8int page_align, heap_t *heap);
/**
  Releases a block allocated with 'alloc'.
**/
void free(void *p, heap_t *heap);

u32int kmalloc_int(u32int sz, int align, u32int *phys);

u32int kmalloc_a(u32int sz);  // выделяет страницу.
u32int kmalloc_p(u32int sz, u32int *phys); // возвращает физический адрес.
u32int kmalloc_ap(u32int sz, u32int *phys); // выделяет страницу и возвращает физический адрес.
u32int kmalloc(u32int sz); // Обычная функция. 

#endif // KHEAP_H