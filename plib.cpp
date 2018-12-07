#include <string>
#include <stdio.h>
#include <sstream>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include "plib.h"

void continue_input_thread();
void continue_hidden_threads2();

int    hidden_threads_count = DEFAULT_HIDDEN_THREADS_COUNT;

pthread_t t[100];
int       t_count = 0;

sem_t * sem_hidden_node_id;
sem_t * sem_output_node_id;
sem_t * sem_hidden_node_fin;
sem_t * sem_output_node_fin;
sem_t * sem_input_node;
sem_t **sem1_hidden_node;//8
sem_t **sem2_hidden_node;//8
sem_t **sem_output_node;//10
int     ocount = 0,hcount = 0;
int     ofcount = 0,hfcount = 0;

void set_hidden_threads_count(int input){
    hidden_threads_count = input;
}

std::string intToString(const int& input){
    std::ostringstream myCustomStreamString;
    myCustomStreamString<<input;
    return myCustomStreamString.str();
}

std::string name_generator(std::string input,int number){
    return (input+intToString(number));
}

void init_semaphores(){
    std::string name;
    sem_unlink("sem_hidden_node_id");
    sem_unlink("sem_output_node_id");
    sem_unlink("sem_hidden_node_fin");
    sem_unlink("sem_output_node_fin");
    sem_unlink("sem_input_node");
    sem_hidden_node_id = sem_open("sem_hidden_node_id",O_CREAT,0600,1);
    sem_output_node_id = sem_open("sem_output_node_id",O_CREAT,0600,1);
    sem_hidden_node_fin = sem_open("sem_hidden_node_fin",O_CREAT,0600,1);
    sem_output_node_fin = sem_open("sem_output_node_fin",O_CREAT,0600,1);
    sem_input_node     = sem_open("sem_input_node",O_CREAT,0600,0);

    sem1_hidden_node = new sem_t*[hidden_threads_count];
    sem2_hidden_node = new sem_t*[hidden_threads_count];
    for(int i =0;i<hidden_threads_count;i++){
        name = name_generator("sem1_hidden_node",i);
        sem_unlink(name.c_str());
        sem1_hidden_node[i] = sem_open(name.c_str(),O_CREAT,0600,0);
        name = name_generator("sem2_hidden_node",i);
        sem_unlink(name.c_str());
        sem2_hidden_node[i] = sem_open(name.c_str(),O_CREAT,0600,0);
    }

    sem_output_node = new sem_t*[OUTPUT_THREADS_COUNT];
    for(int i =0;i<OUTPUT_THREADS_COUNT;i++){
        name = name_generator("sem_output_node",i);
        sem_unlink(name.c_str());
        sem_output_node[i] = sem_open(name.c_str(),O_CREAT,0600,0);
    }
    continue_input_thread();
    continue_hidden_threads2();
}

void close_semaphores(){
    sem_close(sem_hidden_node_id);
    sem_close(sem_output_node_id);
    sem_close(sem_hidden_node_fin);
    sem_close(sem_output_node_fin);
    sem_close(sem_input_node);
    for(int i =0;i<hidden_threads_count;i++){
        sem_close(sem1_hidden_node[i]);
        sem_close(sem2_hidden_node[i]);
    }
    for(int i =0;i<OUTPUT_THREADS_COUNT;i++){
        sem_close(sem_output_node[i]);
    }
}

void get_hidden_node_id(int &i,int &begin,int &end){
    int dev = NUMBER_OF_HIDDEN_CELLS/hidden_threads_count;
    sem_wait(sem_hidden_node_id);
    i = hcount;
    begin = dev*hcount++;
    if(hcount == hidden_threads_count)
        end = NUMBER_OF_HIDDEN_CELLS;
    else
        end = begin + dev;
    sem_post(sem_hidden_node_id);
}

int get_output_node_id(){
    int ret_value;
    sem_wait(sem_output_node_id);
    ret_value = ocount++;
    sem_post(sem_output_node_id);
    return ret_value;
}

void run_threads(int count,void *(*start_routine) (void *)){
    for (int i=0;i<count;i++){
        pthread_create(&(t[t_count++]),NULL,start_routine,NULL);
    }
}
void join_all(){
    for (int i=0;i<t_count;i++){
        pthread_join(t[i],NULL);
    }
}

void wait_input_thread(){
    sem_wait(sem_input_node);
}
void continue_input_thread(){
    sem_post(sem_input_node);
}

void wait_hidden_thread(int i){
    sem_wait(sem1_hidden_node[i]);
    sem_wait(sem2_hidden_node[i]);
}
void continue_hidden_threads1(){
    for(int i =0;i<hidden_threads_count;i++){
        sem_post(sem1_hidden_node[i]);
    }
}
void continue_hidden_threads2(){
    for(int i =0;i<hidden_threads_count;i++){
        sem_post(sem2_hidden_node[i]);
    }
}

void wait_output_thread(int i){
    sem_wait(sem_output_node[i]);
}
void continue_output_threads(){
    for(int i =0;i<OUTPUT_THREADS_COUNT;i++){
        sem_post(sem_output_node[i]);
    }
}

void input_thread_finished(){
    continue_hidden_threads1();
}

void hidden_thread_finished(){
    sem_wait(sem_hidden_node_fin);
    hfcount = (hfcount+1)%hidden_threads_count;
    if (hfcount == 0){
        continue_input_thread();
        continue_output_threads();
    }
    sem_post(sem_hidden_node_fin);
}

bool output_thread_finished(){
    bool retu;
    sem_wait(sem_output_node_fin);
    ofcount = (ofcount+1)%OUTPUT_THREADS_COUNT;
    retu = (ofcount==0);
    sem_post(sem_output_node_fin);
    return retu;
}
