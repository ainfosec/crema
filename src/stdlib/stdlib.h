/**
   @file stdlib.h
   @brief Header for standard C routines that support Crema programs
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   A header defining functions and other structures needed to support
   in the execution of Crema programs
 */
#ifndef CREMA_STDLIB_H_
#define CREMA_STDLIB_H_

#include <stdint.h>
#include <stdlib.h>

struct list_s {
  unsigned int cap;
  int64_t len;
  size_t elem_sz;
  void * arr;
};

typedef struct list_s list_t;
typedef list_t string_t;

#define DEFAULT_RESIZE_AMT 3

list_t * list_create(int64_t es);
void list_free(list_t * list);
void list_insert(list_t * list, unsigned int idx, void * elem);
void * list_retrieve(list_t * list, unsigned int idx);
void list_append(list_t * list, void * elem);
void list_concat(list_t * list1, list_t * list2);
void list_delete(list_t * list, unsigned int idx);
int64_t list_length(list_t * list);

string_t * str_create();
void str_free(string_t * str);
void str_insert(string_t * str, unsigned int idx, char elem);
char str_retrieve(string_t * str, unsigned int idx);
void str_append(string_t * str, char elem);
void str_concat(string_t * str1, string_t * str2);
void str_print(string_t * str);
void str_delete(string_t * str, unsigned int idx);

list_t * int_list_create();
void int_list_insert(list_t * list, int64_t idx, int64_t val);
int64_t int_list_retrieve(list_t * list, int64_t idx);
void int_list_append(list_t * list, int64_t elem);

list_t * double_list_create();
void double_list_insert(list_t * list, unsigned int idx, double val);
double double_list_retrieve(list_t * list, unsigned int idx);
void double_list_append(list_t * list, double elem);

list_t * crema_seq(int64_t start, int64_t end);

#endif // CREMA_STDLIB_H_
