#ifndef LIST_H
#define LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

enum type { Int, String, Char, Double, Null };

struct list_elem {
  void *data;
  enum type type;
  struct list_elem *next;
  int heap;
};

struct list_elem *list_elem_create();
void list_elem_destroy(struct list_elem *le);
struct list_elem *list_elem_ini(void *data, enum type t);
struct list_elem *list_elem_ini_int(long int *i);
struct list_elem *list_elem_ini_str(wchar_t *str);
struct list_elem *list_elem_ini_dub(double *d);
void list_elem_inspect(struct list_elem *le);
void list_elem_inspect_all(struct list_elem *le);
#endif
