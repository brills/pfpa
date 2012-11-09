#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

struct list { struct list *next; int data;};
struct list1 { struct list1 *next; int data;};

void printlist( struct list1 *L) {
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

struct list1 *createnode1(int data) {
	struct list1 *new = malloc(sizeof(struct list1));
	new->data = data;
	return new;
}

void splitclone (struct list *L, struct list1 **R1, struct list1 **R2) { 
	if (L==0) {
		*R1 = *R2 = 0;
		return;
	}
	if (L->data > 16) {
		*R1 = createnode1(L->data);
		splitclone (L->next, &(*R1)->next, R2);
	} else {
		*R2 = createnode1(L->data);
		splitclone (L->next, R1,&(*R2)->next);
	}
}

void processlist(struct list *L) {
	struct list1 *A, *B, *tmp;
	splitclone(L, &A, &B);
	printlist(A);
	printlist(B);
//	while (A) {tmp = A->next; free(A); A=tmp;}
//	while (B) {tmp = B->next; free(B); B=tmp;}
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
	struct list *mylist;
	generatelist(&mylist, 20);
	processlist(mylist);
	return 0;

}