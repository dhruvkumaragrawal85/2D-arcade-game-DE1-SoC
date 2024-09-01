/****************************************************************************
TITLE: FPGA game																																
AUTHOR(s):          Dhruv Kumar Agrawal
ROLL NUMBER(s):     2101CS26    	
Declaration of Authorship
This .c file, game.c, is part of the assignment of CS209 at the 
department of Computer Science and Engineering, IIT Patna . 
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
    int x;
    int y;
    double velocity;
} Character;//stucture for the character

typedef struct
{
    int x;
    int y;
} Pipe;//structure for the pipes


void swap(int *first, int *second);                          //function to swap parameters
void clearScreen();                                          //function to clear screen
void plotPixel(int x, int y, short int lineColor);           //function to plot pixels         
void delay();                                                //function to add delay
void drawStartIcon(int xInit, int yInit);                    //function to plot display icon
void drawCharacter(Character *character);                    //function to draw character icon
void eraseCharacter(Character *character);                   //function to erase character
void selfpipe();                                             //function to map all pipes in a level
void drawpipe(int x, int y);                                 //function to plot individual pipes      
void cleararray();                                           //function to clear collision buffer array 'array'
void write_char(int x, int y, char c);                       //function to print a character string 
void wait_for_vsync();                                       //function to wait for a VSync signal by repeatedly checking the status register of a DMA controller until the signal is received.
void clearbuff();                                            //function to clear the character buffer
void draw_line(int x0, int y0, int x1, int y1, short int colour);//function to draw lines
void draw_box(int x, int y, short int colour);               //function to draw boxes
void animation();                                            //function to plot the endscreen animation


const int SCREEN_WIDTH = 325;  
const int SCREEN_HEIGHT = 245; 

volatile int pixelBufferStart = 0xc8000000;
volatile int led_base = 0xc8000000;

//some utility macros
#define KEY_BASE 0xFF200050
#define CHARACTER_WIDTH 3
#define CHARACTER_HEIGHT 3
#define PIPE_WIDTH 6
#define PIPE_GAP 50
#define MAX_VELOCITY 8
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
#define GREEN 0x07E0
#define BLUE 0x001F
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define GREY 0xC618
#define PINK 0xFC18
#define ORANGE 0xFC00
#define ABS(x) (((x) > 0) ? (x) : -(x))
#define BOX_LEN 2
#define NUM_BOXES 8
#define FALSE 0
#define TRUE 1
#define RESOLUTION_X  320
#define RESOLUTION_Y 240


struct trail {
    float start;
    float end;
    float speed;
} trails[RESOLUTION_X];

void init_trail(struct trail *trail);
void update();

bool gameOver = false;
int score = 0;
int level = 1;
int array[320][240];


int main()
{
    //initializing the collision buffer
    for(int i=0; i<320; i++)
    {
        for(int j=0; j<240; j++)
        {
            array[i][j]=0;
        }
    }
    //clearing the character buffer
    clearbuff();
    int i = 0;
    clearScreen();
    

    //startscreen animation
    while (i < 240)
    {
        delay();
        draw_line(0, i, 319, 239 - i, 0xFFE0);
        i++;
    }
    delay();
    clearScreen();

    //tap to play text
    drawStartIcon(SCREEN_WIDTH / 2 - 83 / 2, 200);

    //button base address intialization
    volatile int *kp = (int *)KEY_BASE;
    unsigned int kv;
    kv = *kp;
    while (1)
    {

        kv = *kp;
        if (kv == 8)
        {
            //If 8('1000') is pressed proceed
            clearScreen();
            break;
        }
    }

    //Character & pipes initialization
    Character character = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 1};
    Pipe pipe = {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    int jump = 0;

    //plotting pipes
    selfpipe();
    while (!gameOver)
    {
        //plot character
        drawCharacter(&character);
        delay();
        eraseCharacter(&character);
        if (character.x == 319)
        {
            clearScreen();
            cleararray();
            level++;
            selfpipe();
        }

        //increment character velocity
        character.velocity = character.velocity + 0.2;

        //increment x coordinate
        character.x = character.x + 1;
        if (character.x > SCREEN_WIDTH)
            character.x = 0;
        //increment y coordinate according to velocity
        character.y += character.velocity;

        //handle edge cases of velocity and screen boundary
        if (character.velocity > MAX_VELOCITY)
        {
            character.velocity = MAX_VELOCITY;
        }
        if (character.y + CHARACTER_HEIGHT >= 238 || character.y - CHARACTER_HEIGHT <= 8)
        {
            character.y = 238 - CHARACTER_HEIGHT;
            character.velocity = 0;
        }



        kv = 0;
        kv = *kp;
        //character jumps on pressing 4('0100')
        if (kv == 4)
        {
            jump=20;
            character.velocity = -0.3;
            character.y -= 2;
            kv = 0;
        }
        //check character's positoion in the collision buffer
        if (array[character.x][character.y] == 1)
        {
            gameOver = true;
        }
    }
    clearScreen();
    //game over message
    char *wela = "GAME OVER";
    int x = 35, y = 30;
    while (*wela)
    {
        write_char(x, 28, *wela);
        wela++;
        x++;
    }
    //endscreen animation
    animation();
}

void delay()
{

    volatile int *pixel_ctrl_ptr = (int *)0xff203020;
    int status;
    *pixel_ctrl_ptr = 1;
    status = *(pixel_ctrl_ptr + 3);

    while ((status & 0x01) != 0)
    {

        status = *(pixel_ctrl_ptr + 3);
    }
}
void draw_ray(int i1, int j1, short int colour)
{

    plotPixel(i1, j1, colour);
    plotPixel(i1 + 1, j1, colour);
    plotPixel(i1, j1 + 1, colour);
    plotPixel(i1 - 1, j1, colour);
    plotPixel(i1, j1 - 1, colour);
    plotPixel(i1 + 2, j1, colour);
    plotPixel(i1, j1 + 2, colour);
    plotPixel(i1 - 2, j1, colour);
    plotPixel(i1, j1 - 2, colour);
    plotPixel(i1 + 1, j1 + 1, colour);
    plotPixel(i1 - 1, j1 + 1, colour);
    plotPixel(i1 + 1, j1 - 1, colour);
    plotPixel(i1 - 1, j1 - 1, colour);
}
void plotPixel(int x, int y, short int lineColor)
{
    *(short int *)(pixelBufferStart + (y << 10) + (x << 1)) = lineColor;
}
void clearScreen()
{
    short int black = 0;
    for (int i = 0; i < SCREEN_WIDTH; i++)
    {
        for (int j = 0; j < SCREEN_HEIGHT; j++)
        {
            plotPixel(i, j, black);
        }
    }
}

void swap(int *first, int *second)
{
    int temp = *first;
    *first = *second;
    *second = temp;
}

const short int tapToPlayIcon[656] = {
    0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, // 0x0010 (16) pixels
    0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, // 0x0020 (32) pixels
    0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0861, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x0030 (48) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, // 0x0040 (64) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0020, 0x1082, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, // 0x0050 (80) pixels
    0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, // 0x0060 (96) pixels
    0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x0070 (112) pixels
    0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, // 0x0080 (128) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0xFFE0, 0xFFE0, // 0x0090 (144) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0xFFE0, 0xFFE0, 0x0000, // 0x00A0 (160) pixels
    0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, // 0x00B0 (176) pixels
    0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x00C0 (192) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, // 0x00D0 (208) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, // 0x00E0 (224) pixels
    0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x10A2, 0xFFE0, 0xFFE0, 0x0000, 0xFFE0, // 0x00F0 (240) pixels
    0xFFE0, 0xFFE0, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, // 0x0100 (256) pixels
    0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, // 0x0110 (272) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0861, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, // 0x0120 (288) pixels
    0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, // 0x0130 (304) pixels
    0xFFE0, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x18E3, 0xFFE0, 0xFFE0, // 0x0140 (320) pixels
    0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, // 0x0150 (336) pixels
    0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, // 0x0160 (352) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0020, 0x0000, 0x0000, 0xFFE0, 0xFFE0, // 0x0170 (368) pixels
    0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, // 0x0180 (384) pixels
    0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, // 0x0190 (400) pixels
    0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, // 0x01A0 (416) pixels
    0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, // 0x01B0 (432) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0841, 0x0000, 0x0000, // 0x01C0 (448) pixels
    0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, // 0x01D0 (464) pixels
    0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x18C3, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, // 0x01E0 (480) pixels
    0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, // 0x01F0 (496) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, // 0x0200 (512) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0841, // 0x0210 (528) pixels
    0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x0220 (544) pixels
    0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, // 0x0230 (560) pixels
    0xFFE0, 0xFFE0, 0x0020, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, // 0x0240 (576) pixels
    0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0xFFE0, 0xFFE0, // 0x0250 (592) pixels
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, // 0x0260 (608) pixels
    0xFFE0, 0x0861, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, // 0x0270 (624) pixels
    0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, 0xFFE0, // 0x0280 (640) pixels
    0x2104, 0x0000, 0xFFE0, 0xFFE0, 0x0020, 0x0000, 0xFFE0, 0xFFE0, 0x0000, 0x0000, 0x0000, 0xFFE0, 0xFFE0, 0xFFE0, 0x0000, 0x0000, // 0x0290 (656) pixels
};

void drawStartIcon(int xInit, int yInit)
{
    
    int index = 0;
    for (int y = 0; y < 8; ++y)
    {
        for (int x = 0; x < 82; ++x)
        {
            plotPixel(xInit + x, yInit + y, tapToPlayIcon[index]);
            index++;
        }
    }
}
void drawCharacter(Character *character)
{
    short int colour = 0xFFE0;
    for (int i = -4; i <= 4; i++)
    {

        for (int j = -4; j <= 4; j++)
        {
            if (i == 0 && j == 0)
            {
                plotPixel(character->x, character->y, colour);
            }
            else if (i == 1 && j == 1)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == -1 && j == 1)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 0 && j == 2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 0 && j == -2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 1 && j == -1)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == -1 && j == -1)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 2 && j == 2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == -2 && j == 2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 0 && j == 3)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 0 && j == -3)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 2 && j == -2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == -2 && j == -2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (j == 0 && i == 2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (j == 0 && i == -2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (j == 0 && i == 3)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (j == 0 && i == -3)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
        }
    }
}
void eraseCharacter(Character *character)
{
    short int colour = 0x0000;
    for (int i = -4; i <= 4; i++)
    {

        for (int j = -4; j <= 4; j++)
        {
            if (i == 0 && j == 0)
            {
                plotPixel(character->x, character->y, colour);
            }
            else if (i == 1 && j == 1)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == -1 && j == 1)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 0 && j == 2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 0 && j == -2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 1 && j == -1)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == -1 && j == -1)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 2 && j == 2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == -2 && j == 2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 0 && j == 3)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 0 && j == -3)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == 2 && j == -2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (i == -2 && j == -2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (j == 0 && i == 2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (j == 0 && i == -2)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (j == 0 && i == 3)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
            else if (j == 0 && i == -3)
            {
                plotPixel(character->x + i, character->y + j, colour);
            }
        }
    }
}

void selfpipe()
{
    short int colour = 0xFFE0;
    int diff = 319 / (level + 1);

    for (int x = diff / 2; x < 320; x = x + diff)
    {
        int y = rand() % (SCREEN_HEIGHT - PIPE_GAP);
        drawpipe(x, y);
    }
}
void drawpipe(int x, int y)
{
    for (int i = 0; i < y; i++)
    {
        for (int j = x; j < x + 11; j++)
        {
            plotPixel(j, i, 0x07FF);
            array[j][i] = 1;
        }
    }
    for (int i = y + PIPE_GAP; i < SCREEN_HEIGHT; i++)
    {
        for (int j = x; j < x + 11; j++)
        {
            plotPixel(j, i, 0x07FF);
            array[j][i] = 1;
        }
    }
}
void cleararray()
{
    for (int x = 0; x < 320; x++)
    {
        for (int y = 0; y < 240; y++)
        {
            array[x][y] = 0;
        }
    }
}
void write_char(int x, int y, char c)
{
    volatile char *cha = (char *)(0xc9000000 + (y << 7) + x);
    *cha = c;
}

void clearbuff()
{
            char *wela = "         ";
        int x = 35, y = 30;
        while (*wela)
        {
            write_char(x, 28, *wela);
            wela++;
            x++;
        }
        
}
void wait_for_vsync(){
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    int status;
    * pixel_ctrl_ptr = 1;
    while(*(pixel_ctrl_ptr + 3)&1);
    return;
}


void draw_line(int x0, int y0, int x1, int y1, short int colour){

    bool is_steep = abs(y1-y0) > abs(x1-x0);
    int temp;
    if(is_steep){
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if(x0 > x1){
        swap(&x0, &x1);
        swap(&y0, &y1);
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int error = -(dx/2);
    int y = y0;
    int y_step;
    if(y0 < y1) y_step = 1;
    else y_step = -1;

    for(int x = x0; x <= x1; x++){
        if(is_steep){
			plotPixel(y,x,colour);
		}
        else {
			plotPixel(x,y,colour);
		}
        error += dy;
        if(error > 0){
            y += y_step;
            error -= dx;
        }
    }

}
void draw_box(int x, int y, short int colour){
    plotPixel(x,y,colour);
    plotPixel(x+1,y,colour);
    plotPixel(x,y+1,colour);
    plotPixel(x+1,y+1,colour);
}
void animation()
{
       /* Clear screen */
    for (int x = 0; x < RESOLUTION_X; x++) {
        for (int y = 0; y < RESOLUTION_Y; y++)
            plotPixel(x, y, 0x0000);
    }

    /* Trails */
    for (int i = 0; i < RESOLUTION_X; i++)
        init_trail(&trails[i]);

    /* Matrix */
    for (;;) {
        delay();
        update();
        delay();
    }

}
void init_trail(struct trail *trail)
{
    trail->start = -(float)(rand() % RESOLUTION_Y);
    trail->end = 0;
    trail->speed = (rand() % 8) / 1.0f + 0.2f;
}
void update()
{
    int trail_RESOLUTION_X = 4; // number of pixels per trail
    int trail_padding = 2; // number of pixels between trails

    for (int x = 0; x < RESOLUTION_X; x += trail_RESOLUTION_X + trail_padding) {
        for (int y = 0; y < RESOLUTION_Y; y++)
            plotPixel(x, y, 0x0000);

        int start = trails[x].start < 0 ? 0 : (int)trails[x].start;
        int end = trails[x].end > RESOLUTION_Y ? RESOLUTION_Y : (int)trails[x].end;

        for (int y = start; y < end; y++)
            plotPixel(x, y, 0x07E0);

        trails[x].start += trails[x].speed;
        trails[x].end += trails[x].speed;

        if (trails[x].start >= RESOLUTION_Y)
            init_trail(&trails[x]);
    }
}
