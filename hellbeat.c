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

struct Trail
{
    V2f pos;
    Color c;
    int alive;
    float step;
    float spawntime; // in seconds
};

typedef struct {
    V2f pos;
    char *buf;
    float lifetime;    // max time in [s] how long text should be dispalyed
    float appear_time; // since it was created we are counting += dt
} Text;

typedef struct {
    struct Trail *items;
    size_t count;
    size_t capacity;
} Trails_Array;

typedef struct {
    Text *items;
    size_t count;
    size_t capacity;
} Text_Array;

struct rhythm_server
{
    int combo;       // combo count
    int score;       // score amount
    int total_score; // max score amount

    float tr;        // Trail radius
    float ear;       // 300ep enpoint activation radius
    float ehar;      // 150ep enpoint (half) activation radius
    float time;      // current time
    float start_time;// time when game started

    Text_Array text_arr;
    Trails_Array trails[MOTION_COUNT];
};

struct map_level
{
};

/* basically a rendererer */
struct artist
{
};

/* GLOBAL VARIABLES */
struct rhythm_server server = {0};

/* functions declarations */
bool check_keysUP(enum MOTIONS m);
bool check_keysDOWN(enum MOTIONS m);

void add_text(char *text, V2f pos, float lifetime);
void check_borders(enum MOTIONS m, struct Trail *t, V2f mid, int dir);
void start_trail(enum MOTIONS motion, V2f map, float starttime, float lifetime);

/* functions implementations */
void start_trail(enum MOTIONS motion, V2f map, float starttime, float lifetime) // TODO: +bezier)
{
    V2f mid = v2f_div(map, v2ff(2));
    V2f pos = map;
    float step=0;
    float lpos = 4;
    float rpos = 6;

    switch (motion)
    {
        case Motion_F:
                pos.x = (map.x/10) * lpos; pos.y = 0;
                step = fabs((float)(pos.y - mid.y)/lifetime);
                break;
        case Motion_D:
                pos.x = (map.x/10) * lpos; pos.y = map.y;
                step = fabs((float)(pos.y - mid.y)/lifetime);
                break;
        case Motion_J:
                pos.x = (map.x/10) * rpos; pos.y = 0;
                step = fabs((float)(pos.y - mid.y)/lifetime);
                break;
        case Motion_K:
                pos.x = (map.x/10) * rpos; pos.y = map.y;
                step = fabs((float)(pos.y - mid.y)/lifetime);
                break;
            default:
            break;
    }
    struct Trail t = {pos, GRAY, true, step, starttime};
    push(server.trails[motion], t);
}

void check_borders(enum MOTIONS m, struct Trail *t, V2f mid, int dir)
{
    float enp  = server.ear;   // 300ep activation range;
    float henp = server.ehar;  // 150ep activation range
    
    // we have to add activation range 
    float up = mid.y - enp;
    float down = mid.y + enp;
    // we have to add h activation range 
    float hup = mid.y - henp;
    float hdown = mid.y + henp;

    if (dir == 1)
    {
        if (t->pos.y > up && t->pos.y < down)
        {
            t->c = RED;
            if (check_keysUP(m))
            {
                server.score += 300;
                t->alive = false;

                server.combo++;
                char *combo_text = malloc(sizeof(char)*16);
                sprintf(combo_text , "x%d", server.combo);
                add_text(combo_text, mid, 0.5);
            }
        }
        else if (t->pos.y > hup && t->pos.y < hdown)
        {
            t->c = ORANGE;
            if (check_keysUP(m))
            {
                server.score += 150;
                t->alive = false;

                server.combo++;
                char *combo_text = malloc(sizeof(char)*16);
                sprintf(combo_text , "x%d", server.combo);
                add_text(combo_text, mid, 0.5);
            }
        }
        else if (t->pos.y > down)
        {
            t->c = YELLOW;
            server.combo = 0;
            t->alive = false;
        }
        else
            t->c = GRAY;
    }
    else
    {
        if (t->pos.y < down && t->pos.y > up)
        {
            t->c = RED;
            if (check_keysDOWN(m))
            {
                server.score += 300;
                t->alive = false;

                server.combo++;
                char *combo_text = malloc(sizeof(char)*16);
                sprintf(combo_text , "x%d", server.combo);
                add_text(combo_text, mid, 0.5);
            }
        }
        else if (t->pos.y < hdown && t->pos.y > hup)
        {
            t->c = ORANGE;
            if (check_keysDOWN(m))
            {
                server.score += 150;
                t->alive = false;

                server.combo++;
                char *combo_text = malloc(sizeof(char)*16);
                sprintf(combo_text , "x%d", server.combo);
                add_text(combo_text, mid, 0.5);
            }
        }
        else if (t->pos.y < up)
        {
            t->c = YELLOW;
            server.combo = 0;
            t->alive = false;
        }
        else
            t->c = GRAY;
    }
}

bool check_keysUP(enum MOTIONS m)
{
    switch (m)
    {
        case Motion_F:
            if (IsKeyPressed(KEY_F))
            {
                return true;
            }
            break;
        case Motion_J:
            if (IsKeyPressed(KEY_J))
            {
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}

bool check_keysDOWN(enum MOTIONS m)
{
    switch (m)
    {
        case Motion_D:
            if (IsKeyPressed(KEY_D))
            {
                return true;
            }
            break;
        case Motion_K:
            if (IsKeyPressed(KEY_K))
            {
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}
void drawlines(V2f map, V2f mid)
{
    int enp  = server.ear;
    int henp = server.ehar;

    // middle line RED
    DrawLine(0, mid.y, map.x, mid.y, RED);

    // activation line GREEN
    DrawLine(0, mid.y-enp, map.x, mid.y-enp, GREEN);
    // activation line GREEN
    DrawLine(0, mid.y+enp, map.x, mid.y+enp, GREEN);

    // semi-activation line ORANGE
    DrawLine(0, mid.y-henp, map.x, mid.y-henp, ORANGE);
    // semi-activation line ORANGE
    DrawLine(0, mid.y+henp, map.x, mid.y+henp, ORANGE);
}

void drawtext(V2f map, float dt)
{
    for (size_t i = 0; i<server.text_arr.count; i++)
    {
        Text *t = &server.text_arr.items[i];
        t->appear_time += dt;

        if (t->appear_time > t->lifetime)
        {
            // TODO: deleting while iteration through array here
        }
        else
        {
            // COMBO
            V2f combo_text_pos;
            int font_size = 40;
            int text_width = MeasureText(t->buf, font_size);
            combo_text_pos.x = (map.x - text_width) / 2;
            combo_text_pos.y = ((float)map.y / 10)*2 - (float)font_size / 2;
            DrawText(t->buf, combo_text_pos.x, combo_text_pos.y, font_size, WHITE);
        }
    }

    // SCORE
    char score_text[16];
    sprintf(score_text, "%d", server.score);
    V2f score_text_pos;
    int font_size = 40;
    int text_width = MeasureText(score_text, font_size);
    score_text_pos.x = (map.x - text_width - 10);
    score_text_pos.y = 5;
    DrawText(score_text, score_text_pos.x, score_text_pos.y, font_size, WHITE);
}

void trail_step(struct Trail *t, float dt, int dir)
{
    if (dir == 1)
        t->pos.y = t->pos.y + (t->step * dt);
    else
        t->pos.y = t->pos.y - (t->step * dt);
}

void map_run(V2f map)
{
    // ahh yes - https://reddit.com/r/ProgrammerHumor/comments/1jvwlp2
#include "temp.h"
}
//void map_run(V2f map)
//{
//    //start_trail(Motion_F, map, 1, 2);
//    //start_trail(Motion_D, map, 2, 2);
//    //start_trail(Motion_J, map, 3, 2);
//    //start_trail(Motion_K, map, 4, 2);
//
//    start_trail(Motion_D, map, 1.0f, 6.0f);
//    start_trail(Motion_J, map, 1.5f, 6.0f);
//
//    start_trail(Motion_F, map, 2.0f, 6.0f);
//    start_trail(Motion_J, map, 3.5f, 6.0f);
//
//    start_trail(Motion_F, map, 4.0f, 6.0f);
//    start_trail(Motion_K, map, 4.5f, 6.0f);
//    
//    start_trail(Motion_D, map, 5.0f, 6.0f);
//    start_trail(Motion_J, map, 5.5f, 6.0f);
//}

void add_text(char *text, V2f pos, float lifetime)
{
    Text t;
    t.pos = pos;
    t.buf = text;
    t.appear_time = 0;
    t.lifetime = lifetime;
    
    push(server.text_arr, t);
}

int main(void)
{
    const V2f map = v2f(1920, 1080);
    const V2f map_middle = v2f_div(map, v2ff(2));

    InitWindow(map.x, map.y, "Sesbian Lex!");
    SetTargetFPS(240);
    server.start_time = GetTime();
    server.tr = map.x/40;
    server.ear = server.tr/1.5;             // 300ep
    server.ehar = server.tr + server.tr/2;  // 150ep

    //test
    map_run(map);
    //start_trail(Motion_F, map, 1, 4);
    //start_trail(Motion_D, map, 2, 4);
    //start_trail(Motion_J, map, 3, 4);
    //start_trail(Motion_K, map, 4, 4);

    bool pause = false;
    float factor = 1;
    while (!WindowShouldClose())
    {
        if (IsKeyDown(KEY_SPACE) && pause == false)
            pause = true;
        else if (IsKeyReleased(KEY_SPACE))
            pause = false;

        if (IsKeyPressed(KEY_ONE))
            factor = 1;
        if (IsKeyPressed(KEY_TWO))
            factor = 2;
        if (IsKeyPressed(KEY_THREE))
            factor = 4;

        float dt = GetFrameTime() / factor;
        if (pause == false)
            server.time += dt;

        printf("%f\n", server.time);

        BeginDrawing();

        for (int i = 0; i<MOTION_COUNT; i++)
        {
            float dir = 1;

            switch (i)
            {
                case Motion_F: dir = 1;  break;
                case Motion_D: dir = -1; break;
                case Motion_J: dir = 1;  break;
                case Motion_K: dir = -1; break;
            }

            for (size_t j = 0; j<server.trails[i].count; j++)
            {
                struct Trail *t = &server.trails[i].items[j];

                if (t->alive == false) continue;
                if (t->spawntime > server.time) continue;

                if (pause == false)
                    trail_step(t, dt, dir);
                check_borders(i, t, map_middle, dir);

                if (t->alive == true)
                {
                    DrawCircle(t->pos.x, t->pos.y, server.tr, t->c);
                    DrawCircle(t->pos.x, t->pos.y, 2, DARKPURPLE);
                }
            }
            // TODO: deleting while iteration through array here
            //for (int j = 0; j<server.trails[i].count; j++)
            //{
            //    if (server.trails[i].items[j].alive == false)
            //        delete(server.trails[i], server.trails[i].items[j]);
            //}
        }

        drawlines(map, map_middle);
        drawtext(map, dt);
        ClearBackground(BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
