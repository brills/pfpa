#include <stdlib.h>

struct list { struct list* next; int field1; char field2; int data[128]; };

void generate_list1( struct list ** p, int l) {
	struct list *ptr;
	if (l <= 0) return;
	*p = malloc(sizeof(struct list));
	ptr = *p;
	for (--l; l >= 0; --l) {
		ptr->next = malloc(sizeof(struct list));
		ptr = ptr->next;
	}
	ptr->next = NULL;
}

void generate_list2( struct list ** p, int l) {
	struct list *ptr;
	if (l <= 0) return;
	*p = malloc(sizeof(struct list));
	ptr = *p;
	for (--l; l >= 0; --l) {
		ptr->next = malloc(sizeof(struct list));
		ptr = ptr->next;
	}
	ptr->next = NULL;
}


int main() {
	struct list *l1, *p1, *l2, *p2; int i; 

	// generate two linked lists
	generate_list1(&l1, 128);
	generate_list2(&l2, 128);

	// different visiting pattern: we expect two different data layout
	// of the same type "struct list"
	//
	for (i = 0, p1 = l1; p1->next != NULL; p1 = p1->next, ++i) {
		if (!(i & 15))
			p1->field2 = i;
		p1->field1 = i;
	}

	for (i = 0, p2 = l2; p2->next != NULL; p2 = p2->next, ++i) {
		if (!(i & 15))
			p2->field1 = i;
		p2->field2 = i;
	}

  return 0;
}
