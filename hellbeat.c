#include <stdio.h>
#include <raylib.h>
#include <stddef.h>
#include <stdlib.h>

#define LA_IMPLEMENTATION
#include "la.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

const int g_KEY_D = KEY_E; //KEY_D;
const int g_KEY_F = KEY_R; //KEY_F;
const int g_KEY_J = KEY_U; //KEY_J;
const int g_KEY_K = KEY_I; //KEY_K;

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
    float spawntime;         // in seconds
    enum MOTIONS motion;     // D, F, J, K
};

typedef struct {
    V2f pos;
    char *buf;
    float lifetime;    // max time in [s] how long text should be dispalyed
    float appear_time; // since it was created we are counting += dt
} Text;

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

    Text *text_arr;
    struct Trail *trails;
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

// combo
void combo_break();
void combo_update();

/* functions declarations */
bool check_keysUP(enum MOTIONS m);
bool check_keysDOWN(enum MOTIONS m);

void check_borders(struct Trail *t, V2f mid);
void add_text(char *text, V2f pos, float lifetime);
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
    struct Trail t = {pos, GRAY, true, step, starttime, motion};
    arrpush(server.trails, t);
}

void check_borders(struct Trail *t, V2f mid)
{
    enum MOTIONS m = t->motion;
    float enp  = server.ear;   // 300ep activation range;
    float henp = server.ehar;  // 150ep activation range
    
    // we have to add activation range 
    float up = mid.y - enp;
    float down = mid.y + enp;
    // we have to add h activation range 
    float hup = mid.y - henp;
    float hdown = mid.y + henp;

    int keyJ = IsKeyPressed(g_KEY_J);
    int keyF = IsKeyPressed(g_KEY_F);
    int keyD = IsKeyPressed(g_KEY_D);
    int keyK = IsKeyPressed(g_KEY_K);

    if (m == Motion_J || m == Motion_F)
    {
        if (t->pos.y > up && t->pos.y < down)
        {
            t->c = RED;
            if ((keyJ && m == Motion_J) || 
                    (keyF && m == Motion_F))
            {
                server.score += 300;
                t->alive = false;

                combo_update();
                char *combo_text = malloc(sizeof(char)*16);
                sprintf(combo_text , "x%d", server.combo);
                add_text(combo_text, mid, 0.5);
            }
        }
        else if (t->pos.y > hup && t->pos.y < hdown)
        {
            t->c = ORANGE;
            if ((keyJ && m == Motion_J) || 
                    (keyF && m == Motion_F))
            {
                server.score += 150;
                t->alive = false;

                combo_update();
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
    else if (m == Motion_D || m == Motion_K)
    {
        if (t->pos.y < down && t->pos.y > up)
        {
            t->c = RED;
            if ((keyD && m == Motion_D) || 
                    (keyK && m == Motion_K))
            {
                server.score += 300;
                t->alive = false;

                combo_update();
                char *combo_text = malloc(sizeof(char)*16);
                sprintf(combo_text , "x%d", server.combo);
                add_text(combo_text, mid, 0.5);
            }
        }
        else if (t->pos.y < hdown && t->pos.y > hup)
        {
            t->c = ORANGE;
            if ((keyD && m == Motion_D) || 
                    (keyK && m == Motion_K))
            {
                server.score += 150;
                t->alive = false;

                combo_update();
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
    for (size_t i = 0; i<arrlenu(server.text_arr); i++)
    {
        Text *t = &server.text_arr[i];
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

void trail_step(struct Trail *t, float dt)
{
    enum MOTIONS m = t->motion;

    if (m == Motion_J || m == Motion_F)
        t->pos.y = t->pos.y + (t->step * dt);
    else if (m == Motion_D || m == Motion_K)
        t->pos.y = t->pos.y - (t->step * dt);
}

void map_run(V2f map)
{
    // ahh yes - https://reddit.com/r/ProgrammerHumor/comments/1jvwlp2
#include "map.h"
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
    
    arrpush(server.text_arr, t);
}

void combo_break()
{
    server.combo = 0;
}

void combo_update()
{
    server.combo++;
}

int main(void)
{
    //const V2f map = v2f(1920, 1080);
    const V2f map = v2f(2560, 1440);
    const V2f map_middle = v2f_div(map, v2ff(2));

    InitWindow(map.x, map.y, "Sesbian Lex!");
    SetTargetFPS(240);
    server.start_time = GetTime();
    server.tr = map.x/40;
    server.ear = server.tr/1.5;             // 300ep
    server.ehar = server.tr + server.tr/2;  // 150ep

    //test
    map_run(map);

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

        BeginDrawing();

        int a = arrlen(server.trails);
        printf("%.2f: arr_size(%d)\n", server.time, a);

        for (size_t j = 0; j<arrlenu(server.trails); j++)
        {
            struct Trail *t = &server.trails[j];

            if (t->alive == false) {
                arrdel(server.trails, j);
                continue;
            }
            if (t->spawntime > server.time) continue;

            if (pause == false)
                trail_step(t, dt);

            check_borders(t, map_middle);

            DrawCircle(t->pos.x, t->pos.y, server.tr, t->c);
            DrawCircle(t->pos.x, t->pos.y, 10, DARKPURPLE);
        }

        drawlines(map, map_middle);
        drawtext(map, dt);

        ClearBackground(BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
