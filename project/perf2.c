#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

struct list { struct list* next; int hotData; int coldData[128]; };

void add(struct list* l, int data){
  int i;
  while(l->next)
    l = l->next;

  struct list* node = malloc(sizeof(struct list));
  l->next = node;
  node->hotData = data;

  for(i=0; i<128; i++)
    node->coldData[i] = i;
}

struct list* createList(int data){
  struct list* list = malloc(sizeof(struct list));
  list->hotData = data;

  return list;
}

struct list* get(struct list* l, int idx){
  int i;
  struct list* node = l;
  for(i=0; i < idx; i++)
    node = node->next;

  return node;
}

int main() {
  int i;
  struct list* l = createList(0);
  for(i=1; i<2000; i++){
    add(l, i);
  }

  printf("%d %d\n", get(l, 0)->hotData, get(l,999)->coldData[127]);

  return 0;
}
