#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

//Global vars
#define WIDTH 20
#define HEIGHT 15
#define EMPTY ' '
#define FULL '#'
#define DELAY_0 1000000 // 100 ms
#define DELAY_1 750000  // 75 ms
#define DELAY_2 750000  // 75 ms
#define DELAY_3 500000  // 50 ms
#define DELAY_4 250000  // 25 ms

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

bool erase_block(struct Position *block, char **area){
    area[block[0].y][block[0].x] = EMPTY;
    area[block[1].y][block[1].x] = EMPTY;
    area[block[2].y][block[2].x] = EMPTY;
    area[block[3].y][block[3].x] = EMPTY;
}

bool move_block(struct Position *block, char **area, char direction){
    erase_block(block, area);

    switch(direction){
        case 'l': //left
            area[block[0].y][block[0].x -= 1] = FULL;
            area[block[1].y][block[1].x -= 1] = FULL;
            area[block[2].y][block[2].x -= 1] = FULL;
            area[block[3].y][block[3].x -= 1] = FULL;
            break;
        case 'r': //right
            area[block[0].y][block[0].x += 1] = FULL;
            area[block[1].y][block[1].x += 1] = FULL;
            area[block[2].y][block[2].x += 1] = FULL;
            area[block[3].y][block[3].x += 1] = FULL;
            break;
        case 'd': //instant down
            area[block[0].y][block[0].x] = FULL;
            area[block[1].y][block[1].x] = FULL;
            area[block[2].y][block[2].x] = FULL;
            area[block[3].y][block[3].x] = FULL;
            break;
        default: //one down
            area[block[0].y -= 1][block[0].x] = FULL;
            area[block[1].y -= 1][block[1].x] = FULL;
            area[block[2].y -= 1][block[2].x] = FULL;
            area[block[3].y -= 1][block[3].x] = FULL;
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
            play_area[i][j] = ' ';
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

    block[0].y = 0;
    block[1].y = 0;
    block[2].y = 0;
    block[3].y = 0;

    block[0].x = 10;
    block[1].x = 11;
    block[2].x = 12;
    block[3].x = 13;

    char input;
    while (input = getchar() != 'q') {
        switch(input){
            case 'd':
               move_block(block, play_area, 'l');
                break;
            case 'a':
               move_block(block, play_area, 'r');
                break;
            default:
               move_block(block, play_area, 0);
                break;
        }
        move_block(block, play_area, 0);

        clear_screen();
        draw_area(play_area);

        usleep(DELAY_1);
    }

    //Reseting the terminal back into canonical mode
    tcgetattr(0, &info);
    info.c_lflag |= ICANON;
    tcsetattr(0, TCSANOW, &info);

    return 0;
}
