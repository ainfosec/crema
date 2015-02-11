/**
   @file stdlib.c
   @brief Funcutionality for standard C routines that support Crema programs
   @copyright 2014 Assured Information Security, Inc.
   @author Jacob Torrey <torreyj@ainfosec.com>

   A source file defining functions and other structures needed to support
   in the execution of Crema programs
 */
#include "stdlib.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

//#define KLEE

#ifdef KLEE
#include "klee/klee.h"
#endif

static void list_resize(list_t * list, size_t new_sz)
{
  if (list == NULL)
    {
      return;
    }
  if (new_sz > list->len)
    {
      list->arr = realloc(list->arr, new_sz * list->elem_sz);
      list->cap = new_sz;
    }
}

list_t * list_create(int64_t es)
{
  list_t * l = malloc(sizeof(list_t));
  if (l)
    {
      l->elem_sz = es;
      l->cap = 0;
      l->len = 0;
      l->arr = NULL;
    }
  return l;
}

void list_free(list_t * list)
{
  if (list == NULL)
    {
      return;
    }
  if (list->arr != NULL)
    {
      free(list->arr);
    }
  free(list);
}

void list_delete(list_t * list, unsigned int idx)
{
  if (list == NULL)
    {
      return;
    }
  if (idx < list->len)
    {
      memmove(list->arr + (idx * list->elem_sz), list->arr + ((idx + 1) * list->elem_sz), list->len - idx - 1);
      list->len--;
    }
}

int64_t list_length(list_t * list)
{
  return list->len;
}

void str_delete(string_t * str, unsigned int idx)
{
  list_delete(str, idx);
  ((char *) str->arr)[str->len] = '\0';
}

void list_insert(list_t * list, unsigned int idx, void * elem)
{
  if (list == NULL)
    {
      return;
    }
  if (idx < list->len)
    {
      memcpy(list->arr + (idx * list->elem_sz), elem, list->elem_sz);
    }
}

void * list_retrieve(list_t * list, unsigned int idx)
{
  if (list == NULL)
    {
      return NULL;
    }
  if (idx >= list->len)
    {
      return NULL;
    }
  return list->arr + (idx * list->elem_sz);
}

void list_append(list_t * list, void * elem)
{
  if (list == NULL)
    {
      return;
    }
  // use len+1 to save space for a terminating entry (e.g. '\0')
  if (list->len+1 >= list->cap)
    {
      list_resize(list, list->cap + DEFAULT_RESIZE_AMT);
    }
  list->len++;
  list_insert(list, list->len - 1, elem);
}

void list_concat(list_t * list1, list_t * list2)
{
  int i;
  if (list1->elem_sz != list2->elem_sz)
    {
      return;
    }
  list_resize(list1, list1->cap + list2->len);
  for (i = 0; i < list2->len; i++)
    {
      list_append(list1, list_retrieve(list2, i));
    }
}

string_t * str_create()
{
  return list_create(sizeof(char));
}

string_t * str_from_cstring(char * s)
{
  string_t * str = list_create(sizeof(char));
  list_resize(str, strlen(s) + 1);
  strncpy(str->arr, s, strlen(s));
  str->len = strlen(s);
  str_insert(str, strlen(s), '\0');
  return str;
}

void str_free(string_t * str)
{
  list_free(str);
}

void str_insert(string_t * str, unsigned int idx, char elem)
{
  list_insert(str, idx, (void *) &elem);
}

char str_retrieve(string_t * str, unsigned int idx)
{
  char * p = list_retrieve(str, idx);
  if (p == NULL)
    {
      fprintf(stderr, "ERROR: Retrieving out of bounds list element!\n");
      exit(-1);
    }
  return (char) *p;
}

void str_append(string_t * str, char elem)
{
  list_append(str, (void *) &elem);
  ((char*)str->arr)[str->len] = '\0';
}

void str_concat(string_t * str1, string_t * str2)
{
  list_concat(str1, str2);
  ((char*)str1->arr)[str1->len] = '\0';
}

void str_print(string_t * str)
{
  printf("%s", (char *) str->arr);
}

void str_println(string_t * str)
{
  printf("%s\n", (char *) str->arr);
}

list_t * int_list_create()
{
  return list_create(sizeof(int64_t));
}

void int_list_insert(list_t * list, int64_t idx, int64_t val)
{
  list_insert(list, idx, (void *) &val);
}

int64_t int_list_retrieve(list_t * list, int64_t idx)
{
  int64_t *p = list_retrieve(list, idx);
  if (p == NULL)
    {
      fprintf(stderr, "ERROR: Retrieving out of bounds list element!\n");
      exit(-1);
    }
  return (int64_t) *p;
}

void int_list_append(list_t * list, int64_t elem)
{
  list_append(list, (void *) &elem);
}

list_t * double_list_create()
{
  return list_create(sizeof(double));
}

void double_list_insert(list_t * list, unsigned int idx, double val)
{
  list_insert(list, idx, (void *) &val);
}

double double_list_retrieve(list_t * list, unsigned int idx)
{
  double *p = list_retrieve(list, idx);
  if (p != NULL)
    {
      fprintf(stderr, "ERROR: Retrieving out of bounds list element!\n");
      exit(-1);
    }
  return (double) *p;
}

void double_list_append(list_t * list, double elem)
{
  list_append(list, (void *) &elem);
}

list_t * crema_seq(int64_t start, int64_t end)
{
  list_t * l;
  int64_t i;
  if (end <= start)
    {
      return NULL;
    }
  l = int_list_create();
  list_resize(l, end - start);
  for (i = start; i <= end; i++)
    {
      int_list_append(l, i);
    }
  return l;
}

void double_print(double val)
{
  printf("%lf", val);
}

void double_println(double val)
{
  printf("%lf\n", val);
}

void int_print(int64_t val)
{
    printf("%ld", val);
}

void int_println(int64_t val)
{
    printf("%ld\n", val);
}

void make_symbolic(list_t * list) 
{
#ifdef KLEE
  uint64_t x = klee_int("testint");
  *(uint64_t *)list->arr = x;
#endif
}

char **main_args = NULL;
int64_t main_argc = 0;

void save_args(int64_t argc, char ** argv)
{
  main_args = argv;
  main_argc = argc;
}

int64_t prog_arg_count()
{
  return main_argc;
}

list_t * prog_argument(int64_t idx)
{
  if (idx >= main_argc)
    return str_from_cstring("null cstring");

  return str_from_cstring(main_args[idx]);
}

// ************************ Math Functions ***************************** //

double double_floor(double val)
{
  // return val >= 0 ? (double)((long)val) : (double)((long)(val - 1));
  return floor(val);
}

double double_ceiling(double val)
{
  return val > 0 ? (double)((long)(val + 1)) : (double)((long)val);
}

double double_round(double val)
{
  if(val >= 0)
  {
    return val - (long)val >= 0.5 ? (double)((long)(val + 1)) : (double)((long)val);
  }
  else
  {
    return val - (long)val >= -0.5 ? (double)((long)val) : (double)((long)(val - 1));
  }
}

double double_square(double val)
{
  return val*val;
}

int64_t int_square(int64_t val)
{
  return val*val;
}

double double_pow(double base, double power)
{
  return pow(base, power);
}

int64_t int_pow(int64_t base, int64_t power)
{
  return (int64_t)pow(base, power);
}

double double_sin(double val)
{
  return sin(val);
}

double double_sqrt(double val)
{
  return sqrt(val);
}

double double_abs(double val)
{
  return abs(val);
}

int64_t int_abs(int64_t val)
{
  return abs(val);
}