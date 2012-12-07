#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

struct list { struct list* next; int hotData; int coldData[1024]; };

void add(struct list* l, int data){
  while(l->next)
    l = l->next;

  struct list* node = malloc(sizeof(struct list));
  l->next = node;
  node->hotData = data;
}

struct list* createList(int data){
  struct list* list = malloc(sizeof(struct list));
  list->hotData = data;

  return list;
}

int get(struct list* l, int idx){
  int i;
  struct list* node = l;
  for(i=0; i < idx; i++)
    node = node->next;

  return node->hotData;
}

int main() {
  int i;
  struct list* l = createList(0);
  for(i=1; i<2000; i++){
    add(l, i);
  }

  printf("%d %d\n", get(l, 0), get(l,999));

  return 0;
}
