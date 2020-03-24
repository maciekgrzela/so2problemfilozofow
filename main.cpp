#include <cstdio>
#include <cstdlib>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdint.h>
#include <ncurses.h>

#define C_STATUS 1

pthread_mutex_t MUTEX;  /* mutex for sync display */

#define LOCK pthread_mutex_lock(&MUTEX)
#define UNLOCK pthread_mutex_unlock(&MUTEX)

struct taskArgs {
    int countOfPhilosophers;
    int philosopherNumber;
    sem_t* forks;
    WINDOW* eat;
    WINDOW* think;
};

int getRightFork(int i, int& countOfPhilosophers) {   return (i + 1) % countOfPhilosophers;   }
int getLeftFork(int i) {    return i;   }

void* task(void *arg) {

    struct taskArgs *args = (taskArgs*) arg;
    int number = args->philosopherNumber;
    int countOfPhilosophers = args->countOfPhilosophers;
    WINDOW* eat = args->eat;
    WINDOW* think = args->think;
    
    while(true){
        usleep(rand() % 800000 + 100000);

        if(getLeftFork(number) < getRightFork(number, countOfPhilosophers)){
            sem_wait(&args->forks[getLeftFork(number)]);
            sem_wait(&args->forks[getRightFork(number, countOfPhilosophers)]);
        } else {
            sem_wait(&args->forks[getRightFork(number, countOfPhilosophers)]);
            sem_wait(&args->forks[getLeftFork(number)]);
        }

        
        LOCK;
        init_pair(0, COLOR_WHITE, COLOR_BLACK);
        init_pair(10, COLOR_BLACK, COLOR_GREEN);
        wbkgd(eat, COLOR_PAIR(10));
        wbkgd(think, COLOR_PAIR(0));
        wrefresh(eat);
        wrefresh(think);
        UNLOCK;

        usleep(rand() % 800000 + 100000);

        if(getLeftFork(number) < getRightFork(number, countOfPhilosophers)){
            sem_post(&args->forks[getRightFork(number, countOfPhilosophers)]);
            sem_post(&args->forks[getLeftFork(number)]);
        }else {
            sem_post(&args->forks[getLeftFork(number)]);
            sem_post(&args->forks[getRightFork(number, countOfPhilosophers)]);
        }
        LOCK;
        init_pair(0, COLOR_WHITE, COLOR_BLACK);
        init_pair(11, COLOR_BLACK, COLOR_RED);
        wbkgd(eat, COLOR_PAIR(0));
        wbkgd(think, COLOR_PAIR(11));
        wrefresh(eat);
        wrefresh(think);
        UNLOCK;
    }

    werase(eat);
    werase(think);
    delwin(eat);
    delwin(think);

    return 0;
}

int main(int argc, char **argv) {

    initscr();
    keypad(stdscr, TRUE);
    cbreak();
    curs_set(0);

    int height, width, start_x, start_y;
    height = 5;
    width = 40;
    start_y = start_x = 1;

    int countOfPhilosophers;

    WINDOW *win = newwin(height, width, start_x, start_y);
    refresh();
    box(win, 0, 0);

    mvwprintw(win, 0, 5, " Problem ucztujacych filozofow ");
    mvwprintw(win, 2, 3, "Wprowadz liczbe filozofow:");
    mvwscanw(win, 2, 30, "%i", &countOfPhilosophers);
    wrefresh(win);

    werase(win);
    wrefresh(win);
    delwin(win);

    WINDOW *eat[countOfPhilosophers];
    WINDOW *think[countOfPhilosophers];
    WINDOW *main = newwin(LINES-1, COLS, 0 ,0);
    box(main, 0, 0);
    mvwprintw(main, 0, (COLS / 2) - 15, " Problem ucztujacych filozofow ");
    wrefresh(main);


    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    init_pair(2, COLOR_BLACK, COLOR_RED);
    init_pair(3, COLOR_BLACK, COLOR_GREEN);

    int temp = 1;
    int offset = 0;
    for(int i = 0; i < countOfPhilosophers; i++){
        if (LINES - temp < 5){
            temp = 1;
            offset += 30;
        }
        mvwprintw(main, temp+1, offset+1, "Filozof numer: %i", (i+1));
        eat[i] = subwin(main, 3, 3, temp, offset+20);
        box(eat[i],0,0);
        wrefresh(eat[i]);
        think[i] = subwin(main, 3, 3, temp, offset+24);
        wbkgd(think[i], COLOR_PAIR(2));
        box(think[i],0,0);
        wrefresh(think[i]);
        temp+=3;
    }

    init_pair(5, COLOR_BLACK, COLOR_RED);
    init_pair(6, COLOR_BLACK, COLOR_GREEN);

    WINDOW *statusBar = newwin(1, COLS, LINES-1, 0);
    wbkgd(statusBar, COLOR_PAIR(1));
    wattron(statusBar, A_BOLD);
    mvwprintw(statusBar, 0, 0, " Ctrl-C ZAKONCZ ");
    wattroff(statusBar, A_BOLD);
    wattron(statusBar, COLOR_PAIR(5));
    mvwprintw(statusBar, 0, 18, " FILOZOF MYSLI ");
    wattroff(statusBar, COLOR_PAIR(5));
    wattron(statusBar, COLOR_PAIR(6));
    mvwprintw(statusBar, 0, 33, " FILOZOF SPOZYWA POSILEK ");
    wattroff(statusBar, COLOR_PAIR(6));
    wmove(main, 0, 0);
    wrefresh(main);
    wrefresh(statusBar);

    sem_t forks[countOfPhilosophers];

    for(int i = 0; i < countOfPhilosophers; i++){
        sem_init(&forks[i], 0, 1);
    }

    pthread_t *philosophersThreads = (pthread_t*) malloc(countOfPhilosophers * sizeof(pthread_t));

    struct taskArgs taskArguments;
    taskArguments.philosopherNumber = 0;
    taskArguments.countOfPhilosophers = countOfPhilosophers;
    taskArguments.forks = forks;

    for(int i = 0; i < countOfPhilosophers; i++){
        taskArguments.philosopherNumber = i;
        taskArguments.eat = eat[i];
        taskArguments.think = think[i];
        pthread_create(&philosophersThreads[i], NULL, task, (void *) (taskArgs *)&taskArguments);
        usleep(500000);
    }

    for(int i = 0; i < countOfPhilosophers; i++){
        pthread_join(philosophersThreads[i], NULL);
    }

    free(philosophersThreads);

    for(int i = 0; i < countOfPhilosophers; i++){
        sem_destroy(&forks[i]);
    }

    endwin();

    return 0;
}