#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

struct list { struct list *next; int data;};

void printlist( struct list *L) {
	while (L) {
		printf("%d ", L->data);
		L = L->next;
	}
	printf("\n");
}

struct list *createnode(int data) {
	struct list *new = malloc(sizeof(struct list));
	new->data = data;
	return new;
}

void generatelist(struct list **L, int length) {

	struct list *tmp;
	*L = NULL;

	if (!length)
		return;

	*L = createnode(rand() % 32);
	tmp = *L;
	for (--length; length > 0; --length) {
		tmp->next = createnode(rand() % 32);
		tmp = tmp->next;
	}
	tmp->next = NULL;
}

int main() {
	srand(0);
	struct list *mylist1, *mylist2, *ptr;

	generatelist(&mylist1, 20);
	generatelist(&mylist2, 20);
	if (rand() > 50) 
		ptr = mylist1;
	else 
		ptr = mylist2;
	printlist(ptr);
	return 0;

}
