#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

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
  struct timeval t_start, t_end;
  int i;
  double t_diff;

  gettimeofday(&t_start, NULL);
  struct list* l = createList(0);
  for(i=1; i<2000; i++){
    add(l, i);
  }

  gettimeofday(&t_end,NULL);
  t_diff = (t_end.tv_sec - t_start.tv_sec) + (double)(t_end.tv_usec - t_start.tv_usec)/1000000;
  fprintf(stderr, " ---  time spent = %.6f  --- \n", t_diff);

  printf("%d %d\n", get(l, 0), get(l,999));
  return 0;
}
