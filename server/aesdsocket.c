#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>
#include <syslog.h>
#include <errno.h>
#include <pthread.h>
#include <sys/time.h>
#define PORT 9000
#define MAX_SIZE 100

volatile sig_atomic_t gSignalInterrupt = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
FILE *fp1;


bool create_file_and_send_it(char file_full_path[], char *full_packet, size_t length, int new_socket) 
{
    bool success;
    char *line = NULL;
    size_t len = 0;
   // declaring file pointers
   // FILE *fp1;
   
    // opening file in append mode
    fp1 = fopen(file_full_path, "a");

    //append packet data to file
    fwrite(full_packet, sizeof(char), length, fp1); 
   // fprintf(fp1, "%s",full_packet);
    //close file
    fclose(fp1);

    // opening file in read mode
    fp1 = fopen(file_full_path, "r");

           
        //read line-by-line and send it to client
         while(getline(&line, &len, fp1) != -1) 
         {
            printf("Sending back: %s", line);

            //send data back to client
            send(new_socket, line, strlen(line), 0);
         }

        // Close the file stream once all lines have been
        // read.

    free(line);
    fclose(fp1);

    success = true;
    return success;
}



//handler for SIGINT and SIGTERM
static void signal_handler (int signo)
{
    if (signo == SIGINT)
    {
        printf ("Caught signal, exiting SIGINT!\n");
        gSignalInterrupt = 1;
    }  
    else if (signo == SIGTERM)
    {
        printf ("Caught signal, exiting SIGTERM!\n");
        gSignalInterrupt = 1;
    }
    else 
    {
        /* this should never happen */
        fprintf (stderr, "Unexpected signal!\n");
        exit (EXIT_FAILURE);
    }

}


struct thread_data{
  //  pthread_mutex_t* mutex;
    int new_socket;
    int server_fd;
    char* ip_address;
    char * store_file;
    int t_index;
    bool thread_complete_success;
   // char *full_packet = malloc(sizeof(char));
  //  char *full_packet;
   // pthread_t* thread;  
};

struct node {
    int t_index; //thread index identifier
    pthread_t tid;
    struct node* next;
};

//Function to Insert into head of linked list
void Insert_to_List(struct node** headRef, int t_index) {
    struct node* newNode = malloc(sizeof(struct node));

    (*newNode).t_index = t_index;
    (*newNode).next = *headRef; // The '*' to dereferences back to the real head
    *headRef = newNode; // ditto
}


//Socket Thread
void* threadsocket(void* thread_param)
{
    ssize_t valread =0;
    size_t len_packet = 0;
    size_t len_buf = 0;
    
    char buffer[MAX_SIZE+1]=  {0};
    char new_line_char[] = "\n"; //new line variable
    

    struct thread_data* thread_func_args;
    thread_func_args = (struct thread_data *) thread_param;

    int new_socket = (*thread_func_args).new_socket;
    int server_fd = (*thread_func_args).server_fd;
    char * ip_address = (*thread_func_args).ip_address;
    char * store_file = (*thread_func_args).store_file;
    char * full_packet = malloc(sizeof(char));
    int t_index = (*thread_func_args).t_index; //thread index identifier

    printf("Socket ID:  %i\n", new_socket);

    while(1)
    {
    
        
        //read buffer
        valread = read(new_socket, buffer, MAX_SIZE);
        if (valread >0)
        {
            
            //get length of buffer
            len_buf = strlen(buffer)-1;

           // pthread_mutex_lock(&lock);
            for (int j = 0; j <= len_buf; j++) 
            {
                //load buffer character to build packet message
                full_packet[len_packet++] = buffer[j];

                // Add space for another character to be read.
                full_packet = realloc(full_packet, len_packet+1); 

                //check if realloc not succeeded
                if (full_packet == NULL)
                {
                    // Packet over-lenght, discarding packet;
                    //memset(full_packet, '\0', sizeof(full_packet)); 
                    free(full_packet);
                    printf("Packet over-length, dicarding packet\n");
                    
                }
            }

            //end of packet compare
            if (buffer[len_buf] == new_line_char[0])
            {
                pthread_mutex_lock(&lock);
                if (create_file_and_send_it(store_file, full_packet, len_packet, new_socket) == true)
                {
                    len_packet = 0;        
                    // memset(full_packet, 0, strlen(full_packet));
                    printf("All packets sent back to client\n"); 
                    pthread_mutex_unlock(&lock);
                   // sleep(1);
                }
                else
                {
                    printf("Failure creating file and sending it back to client\n");
                }
                //pthread_mutex_unlock(&lock);
            }
                                                    
            //clear buffer size
            len_buf = 0;
            memset(buffer, 0, sizeof(buffer));
           // pthread_mutex_unlock(&lock);

        }
        //client closed connection, buffer empty
        else
        {
            // closing the connected socket
            close(new_socket);
            free(full_packet);
            printf("Closed connection from  %s\n", ip_address);
            syslog(LOG_DEBUG,"Closed connection from  %s\n", ip_address);
            (*thread_func_args).thread_complete_success = true;
            (*thread_func_args).t_index = t_index;
            pthread_exit(NULL);
           
        }
    } 
   // free(thread_func_args);
    return thread_param;     
}

void *thread_timestamp(void *thread_param)
{

    struct thread_data* thread_func_args;
    thread_func_args = (struct thread_data *) thread_param;
    char * file_full_path = (thread_func_args)->store_file;
    char str[255];
    char rfc_2822[40];

   
    // declaring file pointers
   // FILE *fp1;
    //time_t now;
    time_t rawtime;
    struct tm * timeinfo;
    struct timeval t1, t2;
    double elapsedTime;
   
    // start timer
    gettimeofday(&t1, NULL);// start timer

   
    while(1)
    {
        

        // stop timer
        gettimeofday(&t2, NULL);

        // compute and print the elapsed time in millisec
        elapsedTime = (t2.tv_sec - t1.tv_sec);// sec 
        if (elapsedTime>=10)
        {
            pthread_mutex_lock(&lock);
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            
            strftime(rfc_2822,sizeof(rfc_2822),"%a, %d %b %Y %T %z",timeinfo);
            printf("%s\n", rfc_2822);
            strcpy(str,"timestamp:");
            strcat(str, rfc_2822);
            strcat(str, "\n");
            // printf ( "Current local time and date: %s", asctime (timeinfo) );
            // opening file in append mode
        
            fp1 = fopen(file_full_path, "a");
            //append packet data to file
            fwrite(str, sizeof(char), strlen(str), fp1); 
            //close file
            fclose(fp1);
            pthread_mutex_unlock(&lock);
            elapsedTime = 0;
            gettimeofday(&t1, NULL);// start timer
        }
    }
    free(file_full_path);
    free(thread_func_args);
  //  return thread_param;   
}

void write_timestamp(char file_full_path[], struct timeval* t1)
{

    char str[255];
    char rfc_2822[40];

   
    // declaring file pointers
   // FILE *fp1;
    //time_t now;
    time_t rawtime;
    struct tm * timeinfo;
    struct timeval t2;
   
    double elapsedTime;
   

        

        // stop timer
        gettimeofday(&t2, NULL);

        // compute the elapsed time 
        elapsedTime = (t2.tv_sec - (*t1).tv_sec);
        if (elapsedTime>=10)
        {
            pthread_mutex_lock(&lock);
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            
            strftime(rfc_2822,sizeof(rfc_2822),"%a, %d %b %Y %T %z",timeinfo);
            printf("%s\n", rfc_2822);
            strcpy(str,"timestamp:");
            strcat(str, rfc_2822);
            strcat(str, "\n");
            // printf ( "Current local time and date: %s", asctime (timeinfo) );
            // opening file in append mode
        
            fp1 = fopen(file_full_path, "a");
            //append packet data to file
            fwrite(str, sizeof(char), strlen(str), fp1); 
            //close file
            fclose(fp1);
            pthread_mutex_unlock(&lock);
            elapsedTime = 0;
            gettimeofday(&(*t1), NULL);// start timer
        }
    

}


int main(int argc, char *argv[])
{
    /*VARIABLES DECLARATIONS*********************
    ********************************
    ********************************
    */
    int server_fd=0;
    int new_socket=0;
    char store_file[] =  "/var/tmp/aesdsocketdata";
    struct thread_data *args = malloc(sizeof *args);
    char ip_address[INET_ADDRSTRLEN];
   // char *full_packet;
    
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
  
  
    char *daemon_mode = argv[1]; //daemon parameter
    char *par = "-d";
    //pthread_t tid[60];
    struct node* head = NULL;//
    int i = 0;

    struct timeval* t1;
    gettimeofday(&t1, NULL);// start timer



    //sigaction struct
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;

    //Register signal handlers
    //signal(SIGTERM, signal_handler);
    //signal(SIGINT, signal_handler);
    sigaction(SIGINT, &sa, 0);
    sigaction(SIGTERM, &sa, 0);
    
    //removes aesdsocketdata file
    remove(store_file);

    //Open Log file
    //openlog(NULL, 0, LOG_USER);

    if (pthread_mutex_init(&lock, NULL) != 0) { 
        printf("\n mutex init has failed\n"); 
        return 1; 
    } 

    //TimeStamp Thread
   // pthread_t tid_ts;
    (*args).store_file = store_file;//Parameter for Thread Timestamp function
    //pthread_create(&tid_ts, NULL, thread_timestamp, args);

   // while(gSignalInterrupt !=1)
   // {

        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }


        // Forcefully attaching socket to the PORT
        // Prevents error such as: “address already in use”.
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }

        //parameters of TCP connection
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);


        // Forcefully attaching socket to the PORT
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address))< 0) 
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }

        //check if -d argument was passed
        if (daemon_mode != NULL)
        {
            if (strcmp(daemon_mode,par)  == 0)
            {
                pid_t process_id = 0;
                // Create child process
                process_id = fork();
                // Indication of fork() failure
                if (process_id < 0)
                    {
                        printf("Can't create the child proccess");
                        // Return failure in exit status
                        exit(EXIT_FAILURE);
                    }


                // killing the parent process (child process will be an orphan)
                if (process_id > 0)
                {
                    //printf("process_id of child process %d \n", process_id);
                    // return success in exit status
                    exit(EXIT_SUCCESS);
                }
            }
        }
        


        // Listen for connections
        if (listen(server_fd, 3) < 0) {
            perror("listen");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("Waiting for connection...\n");
        }

        fd_set read_fds;
        int fdmax=0;
        int select_status = 0;

        struct timeval timeout;
        timeout.tv_sec = 0.5;  // timeout for select
        timeout.tv_usec = 0;


        int l;
        //loop while waiting for client to connect
        //the logic below is to make the listen non-blocking, so a signal handler can interrupt if necessary
        while (gSignalInterrupt !=1) 
        {

             if (i>=1)
             {      
                write_timestamp(store_file, &t1);   
            } 

            //Check which therad has completed and join it when completed
            if ((*args).thread_complete_success)
            {
                struct node* current = head;

                for(current = head; current != NULL; current = current->next)
                {                   
                    if((*current).t_index == (*args).t_index)
                    {
                        pthread_join((*current).tid,NULL);
                    };
                }
                (*args).thread_complete_success= false;
              //  pthread_join(tid[ (*args).t_id],NULL);
            }
            
            FD_ZERO(&read_fds);
            fdmax = server_fd;
            FD_SET(server_fd,&read_fds);

            if (server_fd >= fdmax) 
            {
                fdmax = server_fd + 1;
            }

            select_status = select(fdmax, &read_fds, NULL, NULL, &timeout);
            if (select_status == -1) 
            {
                //perror("listen");
                //exit(EXIT_FAILURE);

                gSignalInterrupt = 1;
                break;
            } 
            else if (select_status > 0) 
            {
                // Accept a connection
                new_socket = accept(server_fd, (struct sockaddr*)&address,&addrlen);
                if ((new_socket) < 0) 
                {
                    perror("accept");
                    exit(EXIT_FAILURE);
                }

                printf("Accepted connection from  %s\n", inet_ntoa(address.sin_addr));
                syslog(LOG_DEBUG,"Accepted connection from  %s\n", inet_ntoa(address.sin_addr));


                i++;
                //convert IP address to char
                inet_ntop(AF_INET, &(address.sin_addr), ip_address, INET_ADDRSTRLEN);

                //Parameters for Thread Socket function
                (*args).new_socket = new_socket;
                (*args).server_fd = server_fd;
                //(*args).store_file = store_file;
                (*args).ip_address = ip_address;
                (*args).t_index = i;

                //Add item to head of linked list when there is a new connection
                Insert_to_List(&head, i);
                
                
                //for each client request creates a thread and assign the client request to it to process
                //so the main thread can entertain next request
                // if( pthread_create(&tid[i], NULL, threadsocket, args) != 0 )
                if( pthread_create(&head->tid, NULL, threadsocket, args) != 0 )
                    printf("Failed to create thread\n");
            }
        }

   // }

    printf("Exiting application\n");
    //removes aesdsocketdata file
    remove(store_file);
    // closing the connected socket
    //close(new_socket);
    // closing the listening socket
    close(server_fd);
    //release memory
    //free(full_packet);
    free(args);
    return 0;
    exit (EXIT_SUCCESS);
}





