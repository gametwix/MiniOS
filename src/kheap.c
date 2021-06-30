#include "kheap.h"
#include "paging.h"

// end is defined in the linker script.
extern u32int end;
u32int placement_address = (u32int)&end;


static s32int find_smallest_hole(u32int size, u8int page_align, heap_t *heap)
{
   // Находим наименьший свободный фрагмент, который подходит.
   u32int iterator = 0;
   while (iterator < heap->index.size)
   {
       header_t *header = (header_t *)lookup_ordered_array(iterator, &heap->index);
       // Если пользователь запросил память, которая выровнена по границе
       if (page_align > 0)
       {
           // Выравниваем по границе начало заголовка.
           u32int location = (u32int)header;
           s32int offset = 0;
           if ((location+sizeof(header_t)) & 0xFFFFF000 != 0)
               offset = 0x1000 /* размер страницы */  - (location+sizeof(header_t))%0x1000;
           s32int hole_size = (s32int)header->size - offset;
           // Теперь подходит?
           if (hole_size >= (s32int)size)
               break;
       }
       else if (header->size >= size)
           break;
       iterator++;
   }
   // Когда выходить из цикла?
   if (iterator == heap->index.size)
       return -1; // Мы дошли до конца и ничего не нашли. 
   else
       return iterator;
}

static s8int header_t_less_than(void*a, void *b)
{
   return (((header_t*)a)->size < ((header_t*)b)->size)?1:0;
}

heap_t *create_heap(u32int start, u32int end_addr, u32int max, u8int supervisor, u8int readonly)
{
   heap_t *heap = (heap_t*)kmalloc(sizeof(heap_t));

   // Мы предполагаем, что startAddress и endAddress выровнены по границе страниц.
   ASSERT(start%0x1000 == 0);
   ASSERT(end_addr%0x1000 == 0);

   // Инициализируем список индексов.
   heap->index = place_ordered_array( (void*)start, HEAP_INDEX_SIZE, &header_t_less_than);

   // Сдвигаем начальный адрес вперед, куда мы можем начать помещать данные.
   start += sizeof(type_t)*HEAP_INDEX_SIZE;

   // Обесчьте, чтобы начальный адрес был выровнен по границе страниц.
   if (start & 0xFFFFF000 != 0)
   {
       start &= 0xFFFFF000;
       start += 0x1000;
   }
   // Запишите начальный, конечный и максимальный адреса в структуру памяти типа куча. 
   heap->start_address = start;
   heap->end_address = end_addr;
   heap->max_address = max;
   heap->supervisor = supervisor;
   heap->readonly = readonly;

   // Мы начинаем с одного большого фрагмента свободной памяти, указанной в списке индексов.
   header_t *hole = (header_t *)start;
   hole->size = end_addr-start;
   hole->magic = HEAP_MAGIC;
   hole->is_hole = 1;
   insert_ordered_array((void*)hole, &heap->index);

   return heap;
} 

u32int kmalloc(u32int sz)
{
    u32int tmp = placement_address;
    placement_address += sz;
    return tmp;
} 

// u32int kmalloc(u32int sz, int align)
// {
//     if (align == 1 && (placement_address & 0xFFFFF000)) // Если адрес еще не выровнен по границе страниц
//     {
//         // Align it.
//         placement_address &= 0xFFFFF000;
//         placement_address += 0x1000;
//     }
//     u32int tmp = placement_address;
//     placement_address += sz;
//     return tmp;
// }

// u32int kmalloc(u32int sz, int align, u32int *phys)
// {
//     if (align == 1 && (placement_address & 0xFFFFF000)) // Если адрес еще не выровнен по границе страниц
//     {
//         // Align it.
//         placement_address &= 0xFFFFF000;
//         placement_address += 0x1000;
//     }
//     if (phys)
//     {
//         *phys = placement_address;
//     }
//     u32int tmp = placement_address;
//     placement_address += sz;
//     return tmp;
// } 

void kfree(void *p)
{
    free(p, kheap);
}

u32int kmalloc_a(u32int sz)
{
    return kmalloc_int(sz, 1, 0);
}

u32int kmalloc_p(u32int sz, u32int *phys)
{
    return kmalloc_int(sz, 0, phys);
}

u32int kmalloc_ap(u32int sz, u32int *phys)
{
    return kmalloc_int(sz, 1, phys);
}


