#include <stdlib.h>
#include <stdio.h>
//#include <conio.h>
#include <termios.h> 
#include <string.h>
#include <unistd.h>  
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define KEYCODE_R 0x43
#define KEYCODE_L 0x44
#define KEYCODE_U 0x41
#define KEYCODE_D 0x42
#define KEYCODE_q 0x71
#define KEYCODE_ENTER 0x0A
#define KEYCODE_w 0x77
#define KEYCODE_a 0x61
#define KEYCODE_s 0x73
#define KEYCODE_d 0x64
#define KEYCODE_e 0x65
#define KEYCODE_z 0x7A
#define KEYCODE_x 0x78
#define KEYCODE_j 0x6A
#define KEYCODE_k 0x6B
#define KEYCODE_l 0x6C
#define KEYCODE_i 0x69
#define KEYCODE_u 0x75
#define KEYCODE_y 0x79
#define KEYCODE_n 0x6e

//#define KEYCODE_ESC 0x1B

int kfd = 0;
struct termios cooked, raw;

#define ECHOFLAGS (ECHO | ECHOE | ECHOK | ECHONL)  

#define CURSOR_SHOW 0
#define CURSOR_HIDDEN 1

#define MAXIMUS 18 //定义棋盘大小

#define PLAYER_BLACK 1
#define PLAYER_WHITE 2

#define GAME_STATUS_BLACK_WIN 1
#define GAME_STATUS_WHITE_WIN 2
#define GAME_STATUS_RESET_GAME 3
#define GAME_STATUS_QUITE_GAME 4

#define MSG_TYPE_ROUND 1
#define MSG_TYPE_STEP 2
#define MSG_TYPE_INSTRUCTIONS 3
#define MSG_TYPE_GAME_RESULT 4

//范围在1～MAXIMUS-2
int p[MAXIMUS-1][MAXIMUS-1];//存储对局信息
char buff[MAXIMUS*2+1][MAXIMUS*4+3];//输出缓冲器
int Cx,Cy;//当前光标位置
int Ox,Oy;//记录之前的光标位置
int Now;//当前走子的玩家，1代表黑，2代表白
int wl,wp;//当前写入缓冲器的列数和行数位置
char* showText;//在棋盘中央显示的文字信息
int count;//回合数
int step;
int win;


int set_disp_mode(int fd,int option); 
void ShowCursor(int flag);
void gotoxy(int x,int y);
void DoKeyPressed();
void PrintCursor();
void Print();
void PrintChessman();
void InitScreen();
void InitKeyBoard();
int CheckGameResult();
void PrintGameMessage(int MsgType);
void StartGame();
void QuitGame();
/*if(p[i][j]==1)//1为黑子
return "●";
else if(p[i][j]==2)//2为白子
return "○";
else if(i==0&&j==0)//以下为边缘棋盘样式
return "┏";
else if(i==MAXIMUS-1&&j==0)
return "┓";
else if(i==MAXIMUS-1&&j==MAXIMUS-1)
return "┛";
else if(i==0&&j==MAXIMUS-1)
return "┗";
else if(i==0)
return "┠";
else if(i==MAXIMUS-1)
return "┨";
else if(j==0)
return "┯";
else if(j==MAXIMUS-1)
return "┷";
return "┼";//中间的空位
}*/

 //如果option为0，则关闭回显，为1则打开回显  
int set_disp_mode(int fd,int option)  
{  
   int err;  
   struct termios term;  
   if(tcgetattr(fd,&term)==-1)
   {  
     perror("Cannot get the attribution of the terminal");  
     return 1;  
   }  
   if(option)  
        term.c_lflag|=ECHOFLAGS;  
   else  
        term.c_lflag &=~ECHOFLAGS;  
   err=tcsetattr(fd,TCSAFLUSH,&term);  
   if(err==-1 && err==EINTR)
   {  
        perror("Cannot set the attribution of the terminal");  
        return 1;  
   }  
   return 0;  
}  

void ShowCursor(int flag)
{
  if(flag == CURSOR_HIDDEN)
    printf("\033[?25l");
  else if(flag == CURSOR_SHOW)
    printf("\33[?25h");
}

void gotoxy(int x,int y)   //Fantasy  
{  
  printf("%c[%d;%df",0x1B,y,x);  
} 


void PrintChessman()
{
  step ++;
  count += step % 2;
  if(step % 2 != 0)
  {
    Now = PLAYER_BLACK;
    p[Cx][Cy] = Now;
    gotoxy(4 * Cx , Cy + Cy+1);
    printf(" ● ");
  }else
  {
    Now = PLAYER_WHITE;
    p[Cx][Cy] = Now;
    gotoxy(4 * Cx , Cy + Cy+1);
    printf(" ○ ");
  }
  
  PrintGameMessage(MSG_TYPE_ROUND);
  PrintGameMessage(MSG_TYPE_STEP);
  fflush(stdout);
  
  if(CheckGameResult() != 0)
  {
    win = 1;
    PrintGameMessage(MSG_TYPE_GAME_RESULT);
  }
}

void StartGame()
{
  system("clear");
  printf("\033[30;47m");
  ShowCursor(CURSOR_HIDDEN);
  Print();
  set_disp_mode(STDIN_FILENO,0);
  
  Cx = MAXIMUS / 2 - 1,Cy = MAXIMUS / 2; 
  
  Ox = Cx;
  Oy = Cy;
  
  int i , j;
  for(i = 0;i < MAXIMUS - 2; i++)
    for(j = 0;j < MAXIMUS - 2; j++)
      p[i][j] = 0;
  
  count = 0;
  step = 0;
  
  win = 0;
  
  PrintCursor();
  
  PrintGameMessage(MSG_TYPE_INSTRUCTIONS);
}

void QuitGame()
{
  printf("\033[49;37m");
  ShowCursor(CURSOR_SHOW);
  set_disp_mode(STDIN_FILENO,1);
  system("clear");
  
  exit(0);
}

void DoKeyPressed()
{
  char key;
  if(read(kfd, &key, 1) < 0)
  {
    perror("read():");
    exit(-1);
  }
  
  if (key == KEYCODE_q)
  {
    QuitGame();
  }
  
  if(win == 0)
  {
    switch(key)
    {
    case KEYCODE_L:
      Ox = Cx;
      Oy = Cy;
      Cx--;
      PrintCursor();
      break;
    case KEYCODE_R:
      Ox = Cx;
      Oy = Cy;
      Cx++;
      PrintCursor();
      break;
    case KEYCODE_U:
      Ox = Cx;
      Oy = Cy;
      Cy--;
      PrintCursor();
      break;
    case KEYCODE_D:
      Ox = Cx;
      Oy = Cy;
      Cy++;
      PrintCursor();
      break;
    case KEYCODE_ENTER:
      if(p[Cx][Cy] == 0)
      {
        PrintChessman();
      }
      break;
    }
  }
  else
  {
    if(key == KEYCODE_y)
    {
      StartGame();
    }
    else if(key == KEYCODE_n)
    {
      QuitGame();
    }
    
  }
}


int CheckGameResult()
{
  int w = 1 , x = 1 , y = 1 , z = 1 , i;//累计横竖正斜反邪四个方向的连续相同棋子数目

  for(i = 1 ; i < 5 ; i ++)
    if(Cy + i <= MAXIMUS - 2 && p[Cx][Cy + i] == Now )
      w ++;
    else break;//向下检查

  for(i = 1 ;i < 5; i ++)
    if(Cy - i >= 1 && p[Cx][Cy - i] == Now)
      w ++;
    else break;//向上检查

  if(w >= 5)
    return Now;//若果达到5个则判断当前走子玩家为赢家

  
  for(i = 1 ; i < 5 ; i ++)
    if(Cx + i <= MAXIMUS - 2 && p[Cx + i][Cy] == Now)
      x ++;
    else break;//向右检查

  for(i = 1 ; i < 5 ; i ++)
    if( Cx - i >=1 && p[Cx - i][Cy] == Now)
      x ++;
    else break;//向左检查

  if (x >= 5)
    return Now;//若果达到5个则判断当前走子玩家为赢家
  
  for(i = 1 ; i < 5 ; i ++)
    if(Cx + i <= MAXIMUS - 2 && Cy + i <= MAXIMUS - 2 && 
      p[Cx + i][Cy + i] == Now)
      y ++;
    else break;//向右下检查
  
  for(i = 1 ; i < 5 ; i ++)
    if(Cx - i >= 1 && Cy - i >= 1 && p[Cx - i][Cy - i] == Now)
      y ++;
    else break;//向左上检查
  if(y >= 5)
    return Now;//若果达到5个则判断当前走子玩家为赢家

  for(i = 1 ; i < 5 ; i ++)
    if(Cx + i <= MAXIMUS-2 && Cy - i >= 1 && p[Cx + i][Cy - i] == Now)
      z ++;
    else break;//向右上检查
  for(i = 1 ; i < 5 ; i ++)
    if(Cx - i >= 1 && Cy + i <= MAXIMUS - 2 && p[Cx - i][Cy + i] == Now)
    z ++;
    else break;//向左下检查
  if(z >= 5)
    return Now;//若果达到5个则判断当前走子玩家为赢家
    
  return 0;//若没有检查到五连珠，则返回0表示还没有玩家达成胜利
}


void PrintCursor()
{
  
  if(Cx >= MAXIMUS - 1 )
    Cx = 1;
  if(Cx <= 0 )
    Cx = MAXIMUS - 2;
  if(Cy >= MAXIMUS - 1 )
    Cy = 1;
  if(Cy <= 0 )
    Cy = MAXIMUS - 2;
    
  gotoxy(4 * Ox - 1 , Oy + Oy);
  printf("　");
  gotoxy(4 * Ox - 1 , Oy + Oy + 2);
  printf("　");
  gotoxy(4 * Ox + 3 , Oy + Oy);
  printf("　");
  gotoxy(4 * Ox + 3 , Oy + Oy + 2);
  printf("　");
  
  fflush(stdout);
  
  gotoxy(4 * Cx - 1 , Cy + Cy);
  printf("┏");
  gotoxy(4 * Cx - 1 , Cy + Cy + 2);
  printf("┗");
  gotoxy(4 * Cx + 3 , Cy + Cy);
  printf("┓");
  gotoxy(4 * Cx + 3 , Cy + Cy + 2);
  printf("┛");
  fflush(stdout);
}

void Print()
{
  int i,j,k;
  
  for(i = 0 ; i < MAXIMUS + MAXIMUS - 1 ; i++)
  {
    for(j = 0 ; j < MAXIMUS ; j++)
    {
      k = i / 2;
      if(i % 2 == 0)
      {
        if(k == 0 && j == 0)
          printf("┏");
        else if(k == MAXIMUS - 1 && j == 0)
          printf("┗");
        else if(k==MAXIMUS-1&&j==MAXIMUS-1)
          printf("━━━┛");
        else if(k==0&&j==MAXIMUS-1)
          printf("━━━┓");
        else if(k==0&&j!=MAXIMUS-1&&j!=0)
          printf("━━━┯");
        else if(k==MAXIMUS-1&&j!=MAXIMUS-1&&j!=0)
          printf("━━━┷");
        else if(j==0&&k!=MAXIMUS-1&&k!=0)
          printf("┠");
        else if(j==MAXIMUS-1&&k!=MAXIMUS-1&&k!=0)
          printf("━━━┨");
        else
          printf("━━━┼"); 
      }
      else
      {
        if(j == 0 )
        {
          printf("┃");
        }
        else if(j == MAXIMUS-1)
        {
          printf("   ┃");
        }else
        {
          printf("   │");
        }
      }
    }
    printf("\n");
  }
}


void InitScreen()
{

}

void DestroyScreen()
{
  
}

/*
MSG_TYPE_ROUND 1
MSG_TYPE_STEP 2
MSG_TYPE_INSTRUCTIONS 3
MSG_TYPE_GAME_RESULT 4
*/
void PrintGameMessage(int MsgType)
{

  printf("\033[49;37m");
  if(MsgType == MSG_TYPE_ROUND)
  {
    gotoxy(4 * MAXIMUS - 1 , 1);
    printf("回合数：%d",count);
    
  } else if(MsgType == MSG_TYPE_STEP)
  {
    gotoxy(4 * MAXIMUS - 1 , 2);
    printf("步  数：%d",step);
  } else if(MsgType == MSG_TYPE_INSTRUCTIONS)
  {
    gotoxy(4 * MAXIMUS - 1 , MAXIMUS + MAXIMUS - 3);
    printf("↑ ↓ ← → ：移动光标");
    gotoxy(4 * MAXIMUS - 1 , MAXIMUS + MAXIMUS - 2);
    printf(" Enter  ：设置棋子");
    gotoxy(4 * MAXIMUS - 1 , MAXIMUS + MAXIMUS - 1);
    printf("   Q    ：结束游戏");
  } else if(MsgType == MSG_TYPE_GAME_RESULT)
  {
    gotoxy(4 * MAXIMUS - 1 , MAXIMUS);
    if(Now == PLAYER_BLACK)
    {
      printf("黑子获胜！");
    } else if(Now == PLAYER_WHITE)
    {  
      printf("白子获胜！");
    }
    gotoxy(4 * MAXIMUS - 1 , MAXIMUS+1);
    printf("是否继续游戏？（按Y确定，按N取消）");
  }
  fflush(stdout);
  printf("\033[30;47m");
}


void InitKeyBoard()
{
// get the console in raw mode
  tcgetattr(kfd, &cooked); // 得到 termios 结构体保存，然后重新配置终端
  memcpy(&raw, &cooked, sizeof(struct termios));
  raw.c_lflag &=~ (ICANON | ECHO);
  // Setting a new line, then end of file
  raw.c_cc[VEOL] = 1;
  raw.c_cc[VEOF] = 2;
  tcsetattr(kfd, TCSANOW, &raw);
}

int main()
{
 
  InitKeyBoard();
  StartGame();
  
  while(1)
  {
    // get the next event from the keyboard
    DoKeyPressed();
  }
  QuitGame();
  return 0;
}


