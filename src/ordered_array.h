// ordered_array.h – Интерфейс создания упорядоченного массива и 
// добавления к нему или удаления из него элементов.
// Написано для  руководств по разработке ядра - автор James Molloy

#ifndef ORDERED_ARRAY_H
#define ORDERED_ARRAY_H

#include "common.h"

/**
Сортировка этого массива происходит при вставке элементов — он всегда будет отсортирован 
(между двумя последовательными к нему обращениями)
В нем можно хранить все, что угодно; тип хранящихся объектов определяется как void*,
поэтому можно хранить u32int или какой-нибудь другой указатель .
**/
typedef void* type_t;
/**
 Предикат должен возвращать ненулевое значение в случае, если первый аргумент меньше, 
чем второй. В противном случае должно быть возвращено нулевое значение. 
**/
typedef s8int (*lessthan_predicate_t)(type_t,type_t);
typedef struct
{
   type_t *array;
   u32int size;
   u32int max_size;
   lessthan_predicate_t less_than;
} ordered_array_t;

/**
  Стандартный предикат less than (меньше, чем)
**/
s8int standard_lessthan_predicate(type_t a, type_t b);

/**
 Создание упорядоченного массива
**/
ordered_array_t create_ordered_array(u32int max_size, lessthan_predicate_t less_than);
ordered_array_t place_ordered_array(void *addr, u32int max_size, lessthan_predicate_t less_than);

/**
  Уничтожение упорядоченного массива
**/
void destroy_ordered_array(ordered_array_t *array);

/**
  Добавление элемента в массив
**/
void insert_ordered_array(type_t item, ordered_array_t *array);

/**
Поиск элемента по индексу i.
**/
type_t lookup_ordered_array(u32int i, ordered_array_t *array);

/**
  Удаление из массива элемента, расположенного по индексу i
**/
void remove_ordered_array(u32int i, ordered_array_t *array);

#endif // ORDERED_ARRAY_H