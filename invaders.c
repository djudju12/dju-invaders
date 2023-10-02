#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "proj_queue.c"

#define overlaps(a, b) SDL_HasIntersection(a, b)
#define PRINT_TARGET(i, target) printf("target = %d alive = %d\n", i, target.alive)

#define FPS 60
#define DELTA_TIME_SEC 1.0 / FPS
#define WIDTH 800
#define HEIGTH 800
#define BG_COLOR 0x181818FF

#define SPACESHIP_WIDTH 60
#define SPACESHIP_HEIGTH 30
#define SPACESHIP_LIVES 3
#define SPACESHIP_VEL 250
#define SPACESHIP_COLOR 0x174515FF
#define SPACESHIP_PROJ_COLOR 0xFFFFFFFF
#define SPACESHIP_LIVE_SIZE 15
#define SPACESHIP_PROJ_SIZE 10
#define SPACESHIP_PROJ_VEL SPACESHIP_VEL*1.5

#define TARGET_COL_LEN 11
#define TARGET_ROW_LEN 5
#define TARGET_WIDTH 30
#define TARGET_HEIGTH 30
#define TARGET_SHOOT_CHANCE 3
#define TARGET_COLOR 0x52276aFF
#define TARGET_PROJ_COLOR 0xa2ab20FF

SDL_Renderer* renderer;
SDL_Window* window;
ProjQueue* spaceship_proj_pool;
ProjQueue* target_proj_pool;

Uint8 running = 1;
int target_d = -1;

void set_color(Uint32 color)
{
   Uint8 r = color >> 8 * 3;
   Uint8 g = color >> 8 * 2;
   Uint8 b = color >> 8 * 1;
   Uint8 a = color >> 8 * 0;
   SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

typedef struct {
   SDL_Rect rect;
   Uint8 lives;
} Spaceship;

Spaceship spaceship = { 0 };

void spaceship_pos(int x)
{
   spaceship.rect.x = x;
}

void spaceship_init()
{
   SDL_Rect rect = {
      .x = WIDTH / 2,
      .y = HEIGTH - SPACESHIP_HEIGTH * 1.5,
      .w = SPACESHIP_WIDTH,
      .h = SPACESHIP_HEIGTH
   };

   spaceship.rect = rect;
   spaceship.lives = SPACESHIP_LIVES;
}

typedef struct {
   SDL_Rect rect;
   Uint8 alive;
} Target;

Target targets[TARGET_ROW_LEN * TARGET_COL_LEN];

void targets_init()
{
   float x_space = 2.0;
   float y_space = 2.0;

   for (size_t i = 0; i < TARGET_ROW_LEN; i++)
   {
      for (size_t j = 0; j < TARGET_COL_LEN; j++)
      {
         SDL_Rect rect = {
            .x = TARGET_WIDTH + j * TARGET_WIDTH * x_space,
            .y = TARGET_HEIGTH * 2 + i * TARGET_HEIGTH * y_space,
            .w = TARGET_WIDTH,
            .h = TARGET_HEIGTH
         };

         int k = i * TARGET_COL_LEN + j;
         targets[k] = (Target){ rect, 1 };
      }
   }
}

void move_left()
{
   int nx = spaceship.rect.x - SPACESHIP_VEL * DELTA_TIME_SEC;
   if (nx <= 0) return;
   spaceship_pos(nx);
}

void move_right()
{
   int nx = spaceship.rect.x + SPACESHIP_VEL * DELTA_TIME_SEC;
   if (nx + spaceship.rect.w >= WIDTH) return;
   spaceship_pos(nx);
}

void shoot(Projectile source, ProjQueue *proj_pool)
{
   SDL_Rect proj_rect = {
      .x = source.rect.x + source.rect.w / 2,
      .y = source.rect.y - SPACESHIP_PROJ_SIZE,
      .w = SPACESHIP_PROJ_SIZE,
      .h = SPACESHIP_PROJ_SIZE
   };

   enqueue(proj_pool, (Projectile){proj_rect, source.direction});
}

void print_lives()
{
   set_color(0xFF0000FF);
   for (size_t i = 0; i < spaceship.lives; i++)
   {
      SDL_Rect r = {
         .x = SPACESHIP_LIVE_SIZE + SPACESHIP_LIVE_SIZE * i * 1.5,
         .y = SPACESHIP_LIVE_SIZE,
         .w = SPACESHIP_LIVE_SIZE,
         .h = SPACESHIP_LIVE_SIZE
      };

      SDL_RenderFillRect(renderer, &r);
   }
}

void random_target_shoot(SDL_Rect source)
{
   int r = (rand() % ((10000 + 1) - 0) + 0);
   if (r > TARGET_SHOOT_CHANCE) return;

   Projectile proj = {
      .rect = source,
      .direction = -1
   };

   shoot(proj, target_proj_pool);
}

void update(float dt)
{
   Node* spaceship_current = spaceship_proj_pool->first;
   int proj_ny;
   Uint8 target_shoot;
   while (spaceship_current != NULL)
   {
      target_shoot = 0;
      for (size_t i = 0; i < TARGET_COL_LEN * TARGET_ROW_LEN; i++)
      {
         if (overlaps(&spaceship_current->value.rect, &targets[i].rect) && 
            (targets[i].alive) &&
            (spaceship_current->value.direction == 1))
         {
            targets[i].alive = 0;
            dequeue(spaceship_proj_pool);
            target_shoot = 1;
            break;
         }
      }

      if (!target_shoot)
      {
         proj_ny = spaceship_current->value.rect.y - SPACESHIP_PROJ_VEL * dt * spaceship_current->value.direction;
         if (spaceship_current->value.rect.y < (0-100)) dequeue(spaceship_proj_pool);
         else spaceship_current->value.rect.y = proj_ny;
      }

      spaceship_current = spaceship_current->next;
   }
   
   Node* target_current = target_proj_pool->first;
   while (target_current != NULL)
   {
      if (overlaps(&target_current->value.rect, &spaceship.rect) &&
         (target_current->value.direction == -1))
      {
         spaceship.lives--;
         dequeue(target_proj_pool);
         target_current = target_current->next;
         continue;
      }

      if (!target_shoot)
      {
         proj_ny = target_current->value.rect.y - SPACESHIP_PROJ_VEL * dt * target_current->value.direction;
         if (target_current->value.rect.y > (HEIGTH+100)) dequeue(target_proj_pool);
         else target_current->value.rect.y = proj_ny;
      }

      target_current = target_current->next;
   }

   int x_last_target = targets[TARGET_COL_LEN-1].rect.x;
   int x_first_target = targets[0].rect.x;

   if (x_last_target + TARGET_WIDTH >= WIDTH) target_d *= -1;
   if (x_first_target <= 0) target_d *= -1;
   
   int target_nx;
   for (size_t i = 0; i < TARGET_COL_LEN * TARGET_ROW_LEN; i++)
   {
      target_nx = targets[i].rect.x + target_d*(SPACESHIP_VEL/3)*dt;
      targets[i].rect.x = target_nx;
      random_target_shoot(targets[i].rect);
   }

   print_lives();
}

void render()
{
   set_color(SPACESHIP_COLOR);
   SDL_RenderFillRect(renderer, &spaceship.rect);

   set_color(SPACESHIP_PROJ_COLOR);
   Node* spaceship_current = spaceship_proj_pool->first;
   for (size_t i = 0; i < spaceship_proj_pool->len; i++)
   {
      SDL_RenderFillRect(renderer, &spaceship_current->value.rect);
      spaceship_current = spaceship_current->next;
   }

   set_color(TARGET_PROJ_COLOR);
   Node* target_current = target_proj_pool->first;
   for (size_t i = 0; i < target_proj_pool->len; i++)
   {
      SDL_RenderFillRect(renderer, &target_current->value.rect);
      target_current = target_current->next;
   }

   set_color(TARGET_COLOR);
   for (int j = 0; j < TARGET_COL_LEN * TARGET_ROW_LEN; j++)
   {
      if (targets[j].alive) {
         SDL_RenderFillRect(renderer, &targets[j].rect);
      }
   }
}

void clean_up()
{
   set_color(BG_COLOR);
   SDL_RenderClear(renderer);
}

int main(void)
{
   if (SDL_Init(SDL_INIT_EVERYTHING) != 0) goto err_SDL_INIT;

   window = SDL_CreateWindow("SPACE INVADERS", 0, 0, WIDTH, HEIGTH, SDL_WINDOW_BORDERLESS);
   if (!window) goto err_SDL_WINDOW;

   renderer = SDL_CreateRenderer(window, -1, 0);
   if (renderer < 0) goto err_SDL_RENDERER;

   srand(time(NULL));

   spaceship_proj_pool = new_queue();
   target_proj_pool = new_queue();
   spaceship_init();
   targets_init();

   const Uint8* keyboard = SDL_GetKeyboardState(NULL);

   while (running)
   {
      SDL_Event event;
      while (SDL_PollEvent(&event))
      {
         if (event.type == SDL_KEYDOWN)
         {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
               running = 0;
               break;

            case SDLK_SPACE:
               Projectile source = {
                  .rect = spaceship.rect,
                  .direction = 1
               };

               shoot(source, spaceship_proj_pool);
               break;

            default:
               break;
            }
         }
      }

      if (keyboard[SDL_SCANCODE_A]) move_left();
      if (keyboard[SDL_SCANCODE_D]) move_right();
      if (spaceship.lives == 0) running = 0;

      clean_up();
      update(DELTA_TIME_SEC);
      render();

      SDL_RenderPresent(renderer);
      SDL_Delay(1000 / FPS);
   }

   printf("[INFO] cleaning up SDL\n");
   SDL_DestroyWindow(window);
   SDL_DestroyRenderer(renderer);

   return 0;

err_SDL_INIT:
   printf("[ERROR] cannot initiate SDL: %s\n", SDL_GetError());

err_SDL_WINDOW:
   printf("[ERROR] cannot create SDL window: %s\n", SDL_GetError());
   SDL_DestroyWindow(window);

err_SDL_RENDERER:
   printf("[ERROR] cannot create SDL renderer: %s\n", SDL_GetError());
   SDL_DestroyWindow(window);
   SDL_DestroyRenderer(renderer);
}