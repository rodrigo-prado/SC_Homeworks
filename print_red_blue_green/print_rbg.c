#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

/* --- Semaphore - definition - begin --- */

sem_t *round_mutex;		/* access to round area */
sem_t *print_mutex;		/* access to print procedure */

void semaphore_init(sem_t **sem, char *sem_str,  unsigned int value);
void semaphore_close(sem_t *);
void semaphore_unlink(char *);

void down(sem_t *);
void up(sem_t *);

/* --- Semaphore - definition - end */

/* --- Thread - definition - begin */

void create_thread(pthread_t *thread, void *(*start_routine)(void *), void * arg);

/* --- Thread - definition - end */

/* --- error handler - definition - begin */

void do_error(char *);

/* --- error handler - definition - end */

/* --- Program - definition - begin */

int pc = 0;                         /* # printing counter */
int ni = 0;                         /* # number of iterations */

void *print_color(int *id);
void do_print(int *id, int inner_pc);

/* --- Program - definition - end */

/* --- Program - implementation - begin --- */

int main(int argc, char *argv[]) {

	/* Let's define the threads */
	pthread_t thr_red, thr_blue, thr_green;
	int n_thr_id_red, n_thr_id_blue, n_thr_id_green;

	n_thr_id_red = 0; n_thr_id_blue = 1; n_thr_id_green = 2;

	printf("--- Begin process! ---\n");

	/* If there is a argument, use it!!! */
	if (argc > 1) ni = atoi(argv[1]);
	else ni = 20; /* default value */

	printf("Number of iterations %d\n", ni);

    /* desassociating mutexes */ 
    semaphore_unlink("/round_mutex");
    semaphore_unlink("/print_mutex");

	/* initiating mutexes */
    semaphore_init(&round_mutex, "/round_mutex", 1);
    semaphore_init(&print_mutex, "/print_mutex", 1);
   
    create_thread(&thr_red, (void *) print_color, &n_thr_id_red);
    create_thread(&thr_blue, (void *) print_color, &n_thr_id_blue);
    create_thread(&thr_green, (void *) print_color, &n_thr_id_green);
    
    /* joining to actives threads */
    pthread_join(thr_red, NULL);
    pthread_join(thr_blue, NULL);
    pthread_join(thr_green, NULL);
    
    /* finalizing mutexes */ 
    semaphore_close(round_mutex);
    semaphore_close(print_mutex);

    /* desassociating mutexes */ 
    semaphore_unlink("/round_mutex");
    semaphore_unlink("/print_mutex");

    printf("--- The end! ---\n");
}

void *print_color(int *id) {
    int inner_pc = 0;
	printf("begin thread:ID: %d\n", *id);
    int i = 0;
    while (i < ni) {    			    /* repeat n times */
    	down(round_mutex);
    	if ((pc % 3) == *id) {
            inner_pc = pc;
    		pc++;
    		down(print_mutex);
    		up(round_mutex);
    		int seconds = random() % 10;
    		printf("%d:sleeping %d seconds\n", inner_pc, seconds);
    		sleep(seconds);
    		do_print(id, inner_pc);
    		i++;
    		up(print_mutex);
    	} else {
    		up(round_mutex);
    	}
    }
}

void do_print(int *id, int inner_pc) {
    if (*id == 0) {
    	printf("%d:%d:\033[31mvermelho\033[0m\n", *id, inner_pc);
    } else if (*id == 1) {
    	printf("%d:%d:\033[34mazul\033[0m\n", *id, inner_pc);
    } else {
    	printf("%d:%d:\033[32mverde\033[0m\n", *id, inner_pc);
    }
}

/* --- Program - implementation - end --- */


/* --- Semaphore - implementation - begin --- */

void semaphore_init(sem_t **sem, char *sem_str,  unsigned int value) {
    if ((*sem = sem_open(sem_str, O_CREAT, 0644, value)) == SEM_FAILED) {
        perror("sem_init");
        do_error("Error in semaphore_init");
    }
}

void semaphore_close(sem_t *sem) {
    if (sem_close(sem) == -1) {
        perror("sem_close");
        /*do_error("Error in semaphore_close!");*/
    }
}

void semaphore_unlink(char *name) {
    if (sem_unlink(name) == -1) {
        /*perror("sem_unlink");*/
        /*do_error("Error in semaphore_unlink!");*/
    }
}

void down(sem_t *sem) {
    if (sem_wait(sem) == -1) {
        perror("sem_wait");
        do_error("Error in down!");
    }
}

void up(sem_t *sem) {
    if (sem_post(sem) == -1) {
        perror("sem_post");
        do_error("Erro in up!");
    }
}

/* --- Semaphore - implementation - end --- */

/* --- Thread - implementation - begin --- */

void create_thread(pthread_t *thread, void *(*start_routine)(void *), void *arg) {
    int rc;
    if (rc = pthread_create(thread, NULL, start_routine, arg)) {
        printf("Thread creation failed: %d\n", rc);
        do_error("Error in create_thread!");
    }
}

/* --- Thread - implementation - end --- */

/* --- error handler - implementation - begin --- */

void do_error(char str[]) {
    printf("%s\n", str);
    fflush(stdout);
    exit(EXIT_FAILURE);
}

/* --- error handler - implementation - end --- */

