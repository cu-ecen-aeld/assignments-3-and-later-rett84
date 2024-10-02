#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)




void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter

    int wait_to_obtain;
    int wait_to_release;

    int rc;

    struct thread_data* thread_func_args;
    thread_func_args = (struct thread_data *) thread_param;

    wait_to_obtain = (*thread_func_args).wait_to_obtain ; //parameter from struct
    wait_to_release = (thread_func_args->wait_to_release); //parameter from struct diferent way of acessing parameter
    
    
    usleep(wait_to_obtain*1000);
    rc = pthread_mutex_lock (thread_func_args->mutex);
     if (rc!=0)
    {
        printf("mutex lock failed");
    }
     else
    {
         printf("mutex lock succeed");

        usleep(wait_to_release*1000);
    
        rc = pthread_mutex_unlock (thread_func_args->mutex);
        if (rc!=0)
        {
            printf("mutex unlock failed");
        }
        else
        {
            printf("mutex unlock succeed");
            thread_func_args->thread_complete_success = true;
        }
    }

    
     return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    

    struct thread_data *args = malloc(sizeof *args);


    args -> wait_to_obtain = wait_to_obtain_ms;
    args -> mutex = mutex;
    (*args).wait_to_release = wait_to_release_ms;


    int ret;
    ret = pthread_create (thread, NULL, threadfunc, args);
    if (ret) 
    {
        return false;
    }
    else{
         return true;
         free(args);
  
    }

   
}

