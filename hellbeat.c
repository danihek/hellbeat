#include <stdio.h>
#include <raylib.h>
#include <stddef.h>
#include <stdlib.h>

#define LA_IMPLEMENTATION
#include "la.h"

#define push(xs, x) \
    do {\
        if (xs.count >= xs.capacity)\
        {\
            if (xs.capacity == 0) xs.capacity = sizeof(x)+256;\
            else xs.capacity *= 2;\
            xs.items = realloc(xs.items, xs.capacity*sizeof(*xs.items));\
        }\
        xs.items[xs.count++] = x;\
    } while(0)

#define delete(xs, x) \
    do {\
        if (xs.count > 0) {\
            size_t index = -1;\
            for (size_t i = 0; i < xs.count; ++i) {\
                if (&xs.items[i] == &x) {\
                    index = i;\
                    break;\
                }\
            }\
            if (index != (size_t)-1) {\
                for (size_t i = index; i < xs.count - 1; ++i) {\
                    xs.items[i] = xs.items[i + 1];\
                }\
                --xs.count;\
                if (xs.count * 2 < xs.capacity && xs.capacity > 256) {\
                    xs.capacity /= 2;\
                    xs.items = realloc(xs.items, xs.capacity * sizeof(*xs.items));\
                }\
            }\
        }\
    } while(0)

#define delete_by_idx(xs, index) \
    do {\
        if (xs.count > 0 && index < xs.count) {\
            for (size_t i = index; i < xs.count - 1; ++i) {\
                xs.items[i] = xs.items[i + 1];\
            }\
            --xs.count;\
            if (xs.count * 2 < xs.capacity && xs.capacity > 256) {\
                xs.capacity /= 2;\
                xs.items = realloc(xs.items, xs.capacity * sizeof(*xs.items));\
            }\
        }\
    } while(0)


/* structures etc */
enum MOTIONS
{
    Motion_F, // left  up    | ↑ | - |
    Motion_D, // left  down  | ↓ | - |
    Motion_J, // right up    | - | ↑ |
    Motion_K, // right down  | - | ↓ |
    MOTION_COUNT
};

typedef struct {
    struct trail *items;
    size_t count;
    size_t capacity;
    
} Trails_Array;

struct rhytm_server
{
    int score;
    int combo;

    float time;
    float start_time;

    Trails_Array trails[MOTION_COUNT];
};

struct map_level
{
};

/* basically a rendererer */
struct artist
{
};

struct trail
{
    V2f pos;
    int alive;
    float step;
    float spawntime; // in seconds
};

/* GLOBAL VARIABLES */
struct rhytm_server server = {0};


/* functions declarations */
void start_trail(enum MOTIONS motion, V2f map, float starttime, float lifetime);


/* functions implementations */
void start_trail(enum MOTIONS motion, V2f map, float starttime, float lifetime) // TODO: +bezier)
{
    V2f mid = v2f_div(map, v2ff(2));
    V2f pos = map;
    float step=0;

    switch (motion)
    {
        case Motion_F:
                pos.x = (map.x/10) * 2; pos.y = 0;
                step = fabs((float)(pos.y - mid.y)/lifetime);
                break;
        case Motion_D:
                pos.x = (map.x/10) * 2; pos.y = map.y;
                step = fabs((float)(pos.y - mid.y)/lifetime);
                break;
        case Motion_J:
                pos.x = (map.x/10) * 8; pos.y = 0;
                step = fabs((float)(pos.y - mid.y)/lifetime);
                break;
        case Motion_K:
                pos.x = (map.x/10) * 8; pos.y = map.y;
                step = fabs((float)(pos.y - mid.y)/lifetime);
                break;
            default:
            break;
    }
    struct trail t = {pos, true, step, starttime};
    push(server.trails[motion], t);
}

int main(void)
{
    const V2f map = v2f(1000, 1000);
    const V2f map_middle = v2f_div(map, v2ff(2));
    const int radius = 30;

    InitWindow(map.x, map.y, "Sesbian Lex!");
    SetTargetFPS(240);
    server.start_time = GetTime();

    //test
    start_trail(Motion_F, map, 1,   1);
    start_trail(Motion_D, map, 1.5, 1);
    start_trail(Motion_J, map, 2,   1);
    start_trail(Motion_K, map, 2.5, 1);

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        server.time += dt;
        printf("%f:\n", server.time);

        BeginDrawing();

        for (int i = 0; i<MOTION_COUNT; i++)
        {
            Color c = DARKGREEN;
            float dir = 1;

            switch (i)
            {
                case Motion_F: dir = 1;YELLOW; break;
                case Motion_D: dir = -1;BLACK; break;
                case Motion_J: dir = 1; GREEN; break;
                case Motion_K: dir = -1; BLUE; break;
            }

            for (int j = 0; j<server.trails[i].count; j++)
            {
                struct trail *t = &server.trails[i].items[j];
                if (t->alive == false)
                    continue;
                if (t->spawntime > server.time)
                    continue;

                if (dir == 1) // TODO; do this shit better,
                              // it's an abonamination
                {
                    t->pos.y = t->pos.y + (t->step * dt);

                    if (t->pos.y > map_middle.y-radius*2)
                    {
                        c = RED;

                        switch (i)
                        {
                            case Motion_F:
                                if (IsKeyDown(KEY_F))
                                {
                                    server.score += 300;
                                    t->alive = false;
                                }
                                break;
                            case Motion_D:
                                if (IsKeyDown(KEY_D))
                                {
                                    server.score += 300;
                                    t->alive = false;
                                }
                                break;
                            case Motion_J:
                                if (IsKeyDown(KEY_J))
                                {
                                    server.score += 300;
                                    t->alive = false;
                                }
                                break;
                            case Motion_K:
                                if (IsKeyDown(KEY_K))
                                {
                                    server.score += 300;
                                    t->alive = false;
                                }
                                break;
                        }
                    }

                    if (t->pos.y > map_middle.y+radius)
                        t->alive = false;

                    if (t->alive == true)
                        DrawCircle(t->pos.x, t->pos.y, radius, c);
                }
            }
            for (int j = 0; j<server.trails[i].count; j++)
            {
                if (server.trails[i].items[j].alive == false)
                    delete(server.trails[i], server.trails[i].items[j]);
            }

        }

        // middle line RED
        DrawLine(0, map_middle.y, map.x, map_middle.y, RED);
        // activation line GREEN
        DrawLine(0, map_middle.y-radius, map.x, map_middle.y-radius, GREEN);

        char score_text[16];
        sprintf(score_text, "%d", server.score);

        V2f score_text_pos;
        int font_size = 40;
        int text_width = MeasureText(score_text, font_size);
        score_text_pos.x = (map.x - text_width - 10);
        score_text_pos.y = 5;
        DrawText(score_text, score_text_pos.x, score_text_pos.y, font_size, DARKGRAY);

        //char *combo_text = "Sesbian Lex!";
        //V2f combo_text_pos;
        //int font_size = 40;
        //int text_width = MeasureText(combo_text, font_size);
        //combo_text_pos.x = (map.x - text_width) / 2;
        //combo_text_pos.y = ((float)map.y / 10)*2 - (float)font_size / 2;
        //DrawText(combo_text, combo_text_pos.x, combo_text_pos.y, font_size, DARKGRAY);

        ClearBackground(RAYWHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
