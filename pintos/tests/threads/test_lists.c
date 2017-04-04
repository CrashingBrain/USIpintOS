#include "../../lib/kernel/list.h"
#include <stdio.h>
#include "listpop.h"
#include "lib/stdlib.h"


/*Luca Dolfi, Renzo Cotti   - Assignment 1*/


struct item {
	struct list_elem elem;
	int priority;
};

void test_lists(void);
void populate (struct list * l, int * a, int n);
static bool compare_items(const struct list_elem* a, const struct list_elem* b, UNUSED void* aux);
static void print_all(struct list* l);

static bool compare_items(const struct list_elem* a, const struct list_elem* b, UNUSED void* aux) {
  struct item *left, *right;
  left = list_entry(a, struct item, elem);
  right = list_entry(b, struct item, elem);
  return left->priority < right->priority;
}

static void print_all(struct list* l){
  struct list_elem* pos;
  struct item*    item;

  for (pos = list_begin(l); pos != list_end(l); pos = list_next(pos)) {
    item = list_entry(pos, struct item, elem);
    printf("%d\n", item->priority);
  }

  printf("\n");
}

void populate (struct list * l, int * a, int n){
	int i = 0;
	while (i < n){
		struct item * item=malloc(sizeof(struct item));
		item->priority = *(a+i);
		list_push_back(l, &item->elem);
		i++;
	}
}



void test_lists() {
	struct list item_list;
	list_init(&item_list);
	populate (&item_list, ITEMARRAY, ITEMCOUNT);
	// create and populate item_list
	print_all(&item_list);
	list_sort(&item_list, compare_items, NULL);
	print_all(&item_list);
}
