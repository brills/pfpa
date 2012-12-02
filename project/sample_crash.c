#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

struct list1 { struct list1 *next; int data; int array[10][2];};
struct list { struct list *next; int data; struct list1 l[128]; int cold; };

void printlist( struct list1 *L) {
	while (L) {
		printf("%d ", L->data);
		L = L->next;
	}
	printf("\n");
}

struct list *createnode(int data) {
	struct list *new = malloc(sizeof(struct list));
	int i;
	new->data = data;
	for (i = 0; i < 128; i++)
		new->l[i].array[i>>5][1] = 0;
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
	struct list1 *A, *B, *tmp1, *tmp2;
	splitclone(L, &A, &B);
	printlist(A);
	printlist(B);
	while (A) {tmp1 = A->next; free(A); A=tmp1;}
	while (B) {tmp2 = B->next; free(B); B=tmp2;}
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
        mylist->cold = 0;

	return 0;

}
