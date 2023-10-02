#include <SDL2/SDL.h>
#include <stdio.h>

#define PRINT_VALUE(i, proj) printf("[%d] x = %d y = %d w = %d h = %d\n", i, proj.rect.x, proj.rect.y, proj.rect.w, proj.rect.h)

void exit_msg(char *msg);

typedef struct {
   SDL_Rect rect;
   int direction;
} Projectile; 

typedef struct Node Node;

struct Node {
   Projectile value;
   Node *next;
};

typedef struct {
   Node *first;
   Node *last;
   int len;
} ProjQueue;

ProjQueue* new_queue()
{
   ProjQueue *q = malloc(sizeof (ProjQueue));

   if (!q) exit_msg("BUY MORE RAM!\n");
   q->len = 0;
   q->first = q->last = NULL;

   return q;
}

int empty(ProjQueue* queue)
{
   return queue->len == 0;
}

void print_queue(ProjQueue* queue)
{
   Node *current = queue->first;
   int i = 0;
   while (current != NULL)
   {
      PRINT_VALUE(i, current->value);
      current = current->next;
      i++;
   }
}

void enqueue(ProjQueue* queue, Projectile proj)
{
   Node *node = malloc(sizeof (Node));
   if (!node) exit_msg("BUY MORE RAM!\n");

   node->value = proj;
   node->next = NULL;
   
   if (empty(queue)) {
      queue->first = node;
      queue->last = node;
   }
   else {
      queue->last->next = node;
      queue->last = node;
   }

   queue->len++;
}

void dequeue(ProjQueue* queue)
{
   if (empty(queue)) exit_msg("Attempt to dequeue a empty queue! Exiting...\n");
   
   Node *temp = queue->first;

   queue->first = queue->first->next;
   queue->len--;

   free(temp);
}

void exit_msg(char *msg)
{
   fprintf(stderr, "%s", msg);
   exit(-1);
}