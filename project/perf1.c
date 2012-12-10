#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

struct list { struct list* next; int hotData; int coldData[128]; };

void add(struct list* l, int data){
  int i;
  int hd;
  while(l->next){
    l = l->next;
    hd = l->hotData;
  }

  struct list* node = malloc(sizeof(struct list));
  l->next = node;
  node->hotData = data;
}

struct list* createList(int data){
  struct list* list = malloc(sizeof(struct list));
  list->hotData = data;

  return list;
}

struct list* get(struct list* l, int idx){
  int i;
  int hd;
  struct list* node = l;
  for(i=0; i < idx; i++){
    node = node->next;
    hd = node->hotData;
  }

  return node;
}

int main() {
  struct timeval t_start, t_end;
  int i;
  double t_diff;
  int r1 = 0, r2 = 0;

  gettimeofday(&t_start, NULL);

  struct list* l = createList(0);
  struct list* l2 = createList(0);
  for(i=1; i<2000; i++){
    add(l, i);
    add(l2, i);
  }

  for(i=0;i<200;i++){
    r1 += get(l, 1999)->hotData;
    r2 += get(l2, 1999)->hotData;
  }

  gettimeofday(&t_end, NULL);

  t_diff = (t_end.tv_sec - t_start.tv_sec) + (double)(t_end.tv_usec - t_start.tv_usec)/1000000;
  fprintf(stderr, " ---  time spent = %.6f  --- \n", t_diff);

  printf("%d %d\n", r1, r2);
  return 0;
}
