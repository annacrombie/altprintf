#include "list.h"
#include "log.h"

struct list_elem *list_elem_create() {
  void *null = NULL;
  struct list_elem *le = malloc(sizeof(struct list_elem));
  LOG("allocatted %zu@%p\n", sizeof(struct list_elem), le);
  le->data = null;
  le->type = Null;
  le->next = null;
  le->heap = 1;

  return le;
}

void list_elem_destroy(struct list_elem *le) {
  if (le->heap && le->data != NULL) {
    LOG("freeing %p\n", le->data);
    free(le->data);
  }
  if (le->next != NULL) { list_elem_destroy(le->next); }

  LOG("freeing %p\n", le);
  free(le);
}

struct list_elem *list_elem_ini(void *data, enum type t) {
  struct list_elem *le = list_elem_create();
  le->data = data;
  le->type = t;
  return le;
}

struct list_elem *list_elem_ini_str(wchar_t *str) {
  struct list_elem *le = list_elem_create();
  le->data = str;
  le->type = String;
  return le;
}

struct list_elem *list_elem_ini_int(long int *i) {
  struct list_elem  *le = list_elem_create();
  le->data = i;
  le->type = Int;
  return le;
}

struct list_elem *list_elem_ini_dub(double *d) {
  struct list_elem  *le = list_elem_create();
  le->data = d;
  le->type = Double;
  return le;
}

void list_elem_inspect(struct list_elem *le) {
  switch (le->type) {
    case Int:
      printf("->Int %ld\n", *(long int *)le->data);
      break;
    case String:
      printf("->String '%ls'\n", (wchar_t *)le->data);
      break;
    case Char:
      printf("->Char %lc\n", *(wint_t *)le->data);
      break;
    case Double:
      printf("->Double %f\n", *(double *)le->data);
      break;
    case Null:
      printf("->Null\n");
      break;
  }
}

void list_elem_inspect_all(struct list_elem *le) {
  while(1) {
    list_elem_inspect(le);
    if (le->next == NULL) { break; }
    le = le->next;
  }
}
