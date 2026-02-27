#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

//Global vars
#define WIDTH 20
#define HEIGHT 15
#define EMPTY ' '
#define FULL '#'
#define DELAY_0 1000000  // 1000 ms
#define DELAY_1 750000   // 750 ms
#define DELAY_2 500000   // 500 ms
#define DELAY_3 250000   // 250 ms
#define DELAY_4 150000   // 150 ms
#define DELAY_5 125000   // 125 ms
#define DELAY_6 100000   // 100 ms
#define DELAY_7 75000    // 75 ms
#define DELAY_8 50000     // 50 ms
#define DELAY_9 25000     // 25 ms

struct Position {
    u_int8_t x;
    u_int8_t y;
};

void clear_screen() {
    printf("\033[H");
    printf("\033[2J");
}

void draw_horizontal_line() {
    printf("<+");
    for (int i = 0; i < WIDTH; i++) printf("-");
    printf("+>\n");
}

void draw_area(char **area) {
    draw_horizontal_line();
    for (int i = 0; i < HEIGHT; i++) {
        printf("<!");
        for (int j = 0; j < WIDTH; j++) {
            printf("%c", area[i][j]);
        }
        printf("!>\n");
    }
    draw_horizontal_line();
    fflush(stdout);
}

void erase_block(char **area, struct Position *block){
    for(int i = 0; i < 4; i++) {
        u_int8_t tmp_x = block[i].x;
        u_int8_t tmp_y = block[i].y;;
        area[tmp_y][tmp_x] = EMPTY;
    }
}

void draw_block(char **area, struct Position *block) {
    for(int i = 0; i < 4; i++) {
        u_int8_t tmp_x = block[i].x;
        u_int8_t tmp_y = block[i].y;
        area[tmp_y][tmp_x] = FULL;
    }
}

bool move_block(char **area, struct Position *block, char direction) {
   erase_block(area, block);

    //TO DO: ADD EDGE CASES
    switch(direction){
        case 'l': //left
            if((block[0].x -1) >= 0) block[0].x -= 1;
            if((block[1].x -1) >= 0) block[1].x -= 1;
            if((block[2].x -1) >= 0) block[2].x -= 1;
            if((block[3].x -1) >= 0) block[3].x -= 1;
            draw_block(area, block);
            break;
        case 'r': //right
            if((block[0].x + 1) < WIDTH) block[0].x += 1;
            if((block[1].x + 1) < WIDTH) block[1].x += 1;
            if((block[2].x + 1) < WIDTH) block[2].x += 1;
            if((block[3].x + 1) < WIDTH) block[3].x += 1;
            draw_block(area, block);
            break;
        case 'd': //instant down
            block[0].y = HEIGHT-1;
            block[1].y = HEIGHT-1;
            block[2].y = HEIGHT-1;
            block[3].y = HEIGHT-1;
            draw_block(area, block);
            break;
        default: //one down
            if((block[0].y + 1) < HEIGHT) block[0].y += 1;
            if((block[1].y + 1) < HEIGHT) block[1].y += 1;
            if((block[2].y + 1) < HEIGHT) block[2].y += 1;
            if((block[3].y + 1) < HEIGHT) block[3].y += 1;
            draw_block(area, block);
            break;
    }

}

int main(void) {
    //Game area declaration on the heap
    char **play_area = malloc(HEIGHT * sizeof(char*));
    for(int i = 0; i < HEIGHT; i++){
        play_area[i] = malloc(WIDTH * sizeof(char));
    }

    //Game area initialization
    for(int i = 0; i < HEIGHT; i++){
        for(int j = 0; j < WIDTH; j++){
            play_area[i][j] = EMPTY;
        }
    }

    //Block array declaration and initialization
    struct Position *block = malloc(4*sizeof(struct Position));
    struct Position tmp = {0,0};
    for(int i = 0; i < 4; i++) block[i] = tmp;

    //Setting the terminal into the right mode
    struct termios info;
    tcgetattr(0, &info);
    info.c_lflag &= ~ICANON; //Disable canonical mode
    info.c_cc[VMIN] = 0; //No waiting for keystrokes
    info.c_cc[VTIME] = 0; //No timeout
    tcsetattr(0, TCSANOW, &info); //Apply the settings now

    //Test values
    block[0].y = 0;
    block[1].y = 0;
    block[2].y = 0;
    block[3].y = 0;

    block[0].x = 10;
    block[1].x = 11;
    block[2].x = 12;
    block[3].x = 13;
    draw_block(play_area, block);
    //Main game loop (to fix, only testing now)
    char input;
    while (input = getchar() != 'q') {
        clear_screen();
        draw_area(play_area);
        switch(input){
            case 'd':
               move_block(play_area, block, 'l');
                break;
            case 'a':
               move_block(play_area, block, 'r');
                break;
            default:
               move_block(play_area, block, 0);
                break;
        }
        usleep(DELAY_0);
    }

    //Reseting the terminal back into canonical mode
    tcgetattr(0, &info);
    info.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &info);

    //Freeing the play_area array
    for(int i = 0; i < HEIGHT; i++) free(play_area[i]);
    free(play_area);

    //Freeing the block array
    free(block);

    return 0;
}
