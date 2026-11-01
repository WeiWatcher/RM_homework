extern void PrintAll();
extern void Move();


typedef enum{
Up,
Down,
Left,
Right
}direct;

typedef struct{
uint8_t x;
uint8_t y;
}Pos;

extern Pos Snake[120];
extern uint8_t length;
extern direct dir;
extern Pos Food;
extern int isEnd;

extern Pos GetRandomPos();