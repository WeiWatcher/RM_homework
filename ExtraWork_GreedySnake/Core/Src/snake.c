#include <stdint.h>
#include <stdlib.h>
#include "main.h"
#include "OLED.h"
#include "snake.h"


Pos GetRandomPos()
{
    Pos temp;
    temp.x = rand() % (4 + 1);
    temp.y = rand() % (20 + 1);
    return temp;
}

Pos Snake[120] = {0};
uint8_t length = 1;
direct dir = Right;
Pos Food;
int isEnd = 0;


void PrintAll()//因为C面向过程，我们就把蛇焊死在函数里降低开发复杂度
//该函数没有边界判断，执行时必须保证在此之前执行过检查函数
{
    OLED_operate_gram(PEN_CLEAR);
    for(int i = 0;i < length;i ++)
    {
        OLED_show_char(Snake[i].x,Snake[i].y,'#');
    }
    OLED_show_char(Food.x,Food.y,'F');
}

void Move()//这函数集成了边界判断和吃东西判断，既然都面向过程了，就不要那么多繁文缛节！
{


    for(int i = length - 1;i > 0;i --)//头以外的身子动
    {
        Snake[i].x = Snake[i - 1].x;
        Snake[i].y = Snake[i - 1].y;
    }

    for(int i = length - 1;i > 1;i --)
    {
        if(Snake[0].x == Snake[i].x && Snake[0].y == Snake[i].y)
        {
            isEnd = 1;
            break;
        }
    }

        switch(dir)//头动
    {
        case Up:
        Snake[0].x -- ;
        break;
        case Down:
        Snake[0].x ++ ;
        break;
        case Right:
        Snake[0].y ++ ;
        break;
        case Left:
        Snake[0].y -- ;
        break;
    }

    //检查边界在这里发生
    if(Snake[0].x > 4 || Snake[0].y > 20) isEnd = 1;

    //检查吃东西
    if(Snake[0].x == Food.x && Snake[0].y == Food.y)
    {
        Snake[length].x = Snake[length - 1].x;
        Snake[length].y = Snake[length - 1].y;
        //开启下一段身子
        length ++;
        //长度增加
        Food = GetRandomPos();
        //更新食物位置
    }

    // OLED_refresh_gram();
}