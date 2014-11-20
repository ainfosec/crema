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

static void list_resize(list_t * list, size_t new_sz)
{
  if (list == NULL)
    {
      return;
    }
  if (new_sz > list->len)
    {
      list->arr = realloc(list->arr, new_sz * list->elem_sz);
    }
}

list_t * list_create(size_t es)
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
  if (list->len == list->cap)
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

list_t * int_list_create()
{
  return list_create(sizeof(int64_t));
}

void int_list_insert(list_t * list, unsigned int idx, int64_t val)
{
  list_insert(list, idx, (void *) &val);
}

int64_t int_list_retrieve(list_t * list, unsigned int idx)
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
void double_list_insert(list_t * list, unsigned int idx, double val);
double double_list_retrieve(list_t * list, unsigned int idx);
void double_list_append(list_t * list, double elem);

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
  for (i = start; i < end; i++)
    {
      int_list_append(l, i);
    }
  return l;
}
