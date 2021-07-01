// main.c -- Определяет точку входа Cи-кода ядра

#include "monitor.h"
#include "multiboot.h"
#include "descriptor_tables.h"
#include "timer.h"
#include "task.h"
#include "fs.h"
#include "paging.h"
#include "initrd.h"
#include "vfs.h"

u32int initial_esp;


int main(struct multiboot *mboot_ptr)
{
    // // Initialise all the ISRs and segmentation
    // init_descriptor_tables();
    // // Initialise the screen (by clearing it)
    // monitor_clear();
    // // Write out a sample string
    // monitor_write("Hello, world!\n");

    // asm volatile("int $0x3");
    // asm volatile("int $0x4");

    // asm volatile("sti");
    // init_timer(10);

    // return 0;

    //initial_esp = initial_stack;
//    // Инициализируем все ISR и сегментацию
//    init_descriptor_tables();
//    // Инициализируем экран (очищаем его)
//    monitor_clear();
//    // Инициализируем PIT значением 100Hz
//    asm volatile("sti");
//    init_timer(50);

//    // Находим место размещения нашего диска initial ramdisk.
//    ASSERT(mboot_ptr->mods_count > 0);
//    u32int initrd_location = *((u32int*)mboot_ptr->mods_addr);
//    u32int initrd_end = *(u32int*)(mboot_ptr->mods_addr+4);
//    // Пожалуйста, не затрите наш модуля при доступе к адресам размещения!
//    placement_address = initrd_end;

    init_descriptor_tables();
    // Initialise the screen (by clearing it)
    monitor_clear();
    // Write out a sample string
    monitor_write("Hello, world!\n");

    u32int initrd_location = *((u32int*)mboot_ptr->mods_addr);
    // Запускам страничную организацию памяти.
    initialise_paging();

    // Запускаем многозадачность.
    initialise_tasking();

    // Инициализируем initial ramdisk и указываем его как корневую файловую систему.
    fs_root = initialise_initrd(initrd_location);

    initialise_syscalls();

    switch_to_user_mode();

    syscall_monitor_write("Hello, user world!\n");

    return 0; 

}