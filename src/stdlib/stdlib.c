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
	memmove(list->arr + (idx * list->elem_sz), list->arr + ((idx + 1) * list->elem_sz), (list->len - idx - 1) * list->elem_sz);
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

string_t * str_substr(string_t * str, unsigned int start, unsigned int len)
{
  string_t * nstr = list_create(sizeof(char));

  if (start >= str->len)
    return NULL;

  if (len > str->len || len == 0)
    len = str->len - start;

  if (start == 0 && (len == 0 || len == str->len))
    return str;

  if (start >= str->len)
    return NULL;

  list_resize(nstr, len + 1);
  strncpy(nstr->arr, (char*)str->arr + start, len);
  nstr->len = len;
  str_insert(nstr, len, '\0');
  return nstr;
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
  if(str->arr == NULL)
  {
    return;
  }
  printf("%s", (char *) str->arr);
}

void str_println(string_t * str)
{
  if(str->arr == NULL)
  {
    printf("\n");
  }
  else
  {
    printf("%s\n", (char *) str->arr);
  }
}

int str_len(string_t * str)
{
  return str->len;
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

int64_t int_list_length(list_t * list)
{
  return (int64_t)list->len;
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

/**
   Prints a double value to stdout.

   @params val A double value to be printed
*/
void double_print(double val)
{
  printf("%lf", val);
}

/**
   Prints a double value to stdout terminated with a new-line character

   @params val A double value to be printed
*/
void double_println(double val)
{
  printf("%lf\n", val);
}

/**
   Prints a int value to stdout.

   @params val An int value to be printed
*/
void int_print(int64_t val)
{
    printf("%ld", val);
}

/**
   Prints an int value to stdout terminated with a new-line character

   @params val An int value to be printed
*/
void int_println(int64_t val)
{
    printf("%ld\n", val);
}

/**
   ???

   @params list ???
*/
void make_symbolic(list_t * list) 
{
#ifdef KLEE
  uint64_t x = klee_int("testint");
  *(uint64_t *)list->arr = x;
#endif
}

// void bool_print(bool val)
// {
//   printf("%s", val ? "true" : "false");
// }

char **main_args = NULL;
int64_t main_argc = 0;

/**
   Stores the command line arguments passed into the running program in
   global variables for the Crema standard lib.

   @params argc The argc (argument count) value to be saved
   @params argv The argv (argument list) values to be saved
*/
void save_args(int64_t argc, char ** argv)
{
  main_args = argv;
  main_argc = argc;
}

/**
   Returns the number of command line arguments passed to the running Crema program

   @return The number of command line arguments passed
*/
int64_t prog_arg_count()
{
  return main_argc;
}

/**
   Retrieves the string representation of a command line argument at a given postion
   from the command line arguments list passed to the running Crema program
  
   @param idx The index of the argument to be retrieved
   @return The command line argument (string) at position idx, or "null cstring"
*/
list_t * prog_argument(int64_t idx)
{
  if (idx >= main_argc)
    return str_from_cstring("null cstring");

  return str_from_cstring(main_args[idx]);
}

// *********************** Type Conversions ***************************** //

/**
   Cast a double value as an int.

   @params val A double value
   @return The given double value cast as an int
*/
int64_t double_to_int(double val)
{
  return (int64_t)val;
}

/**
   Cast an int value as a double.

   @params val An int value
   @return The given int value cast as a double
*/
double int_to_double(int64_t val)
{
  return (double)val;
}

/**
   Convert an int (64-bit) value into a string of characters.

   @params val An 64-bit int value
   @return The given int value cast as a double
*/
string_t * int_to_string(int64_t val)
{
  char str[20];
  string_t * stp;
  
  snprintf(str, 20, "%ld", val);
  stp = str_from_cstring(str);
  return stp;
}

/**
   Converts a string to an int.

   @params str A string to be converted to an int
   @return Int value, or 0.0 if the string could not be parsed as an int
*/
int64_t string_to_int(string_t * str)
{
  if(str->arr == NULL)
  {
    return 0;
  }
  else
  {
    return atoi(str->arr);
  }
}

/**
   Converts a string to a double.

   @params str A string to be converted to a double
   @return Double value, or 0.0 if the string could not be parsed as a double
*/
int64_t string_to_double(string_t * str)
{
  if(str->arr == NULL)
  {
    return 0;
  }
  else
  {
    return atof(str->arr);
  }
}

// ***************************** Maths ********************************* //

/**
   Calculates the nearest integral value less than the given number.

   @params val A double value
   @return The next lowest integral value
*/
double double_floor(double val)
{
  // return val >= 0 ? (double)((long)val) : (double)((long)(val - 1));
  return floor(val);
}

/**
   Calculates the nearest integral value greater than the given number.

   @params val A double value
   @return The next highest integral value
*/
double double_ceiling(double val)
{
  return val > 0 ? (double)((long)(val + 1)) : (double)((long)val);
}

/**
   Calculates the nearest integral value to the given number.
   The result will be rounded up for values with a decimal portion equal
   to .5000...

   @params val A double value
   @return The nearest integral value
*/
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

/**
   Truncates the given double value to an integral number. In other words, all 
   values after the decimal point become 0.

   @params val A double value
   @return The double value after truncation
*/
double double_truncate(double val)
{
  return trunc(val);
}

/**
   Calculates the square of the given double value (i.e. n*n or n^2)

   @params val A double value
   @return The double value squared
*/
double double_square(double val)
{
  return val*val;
}

/**
   Calculates the square of the given int value (i.e. n*n or n^2)

   @params val An int value
   @return The int value squared
*/
int64_t int_square(int64_t val)
{
  return val*val;
}

/**
   Calculates the value of a double number raised to a power (i.e. x^n)

   @params base The base value for the exponential
   @params power The power for the exponential
   @return The base value raised to the given power
*/
double double_pow(double base, double power)
{
  return pow(base, power);
}

/**
   Calculates the value of an int number raised to a power (i.e. x^n)

   @params base The base value for the exponential
   @params power The power for the exponential
   @return The base value raised to the given power
*/
int64_t int_pow(int64_t base, int64_t power)
{
  return (int64_t)pow(base, power);
}

/**
   Calculates the value of the trigonometric sin function of a
   double value (i.e. sin(n))

   @params val A double value
   @return The sin of the given value
*/
double double_sin(double val)
{
  return sin(val);
}

/**
   Calculates the value of the trigonometric cos function of a
   double value (i.e. cos(n))

   @params val A double value
   @return The cos of the given value
*/
double double_cos(double val)
{
  return cos(val);
}

/**
   Calculates the value of the trigonometric tan function of a
   double value (i.e. tan(n))

   @params val A double value
   @return The tan of the given value
*/
double double_tan(double val)
{
  return tan(val);
}

/**
   Calculates the square root for a given double value

   @params val A double value
   @return The square root of the given value
*/
double double_sqrt(double val)
{
  return sqrt(val);
}

/**
   Calculates the absolute value for a given double value

   @params val A double value
   @return The absolute value of the given value
*/
double double_abs(double val)
{
  return abs(val);
}

/**
   Calculates the absolute value for a given int value

   @params val A int value
   @return The absolute value of the given value
*/
int64_t int_abs(int64_t val)
{
  return abs(val);
}
