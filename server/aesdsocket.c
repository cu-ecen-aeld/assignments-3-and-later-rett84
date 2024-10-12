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
#include<errno.h>
#define PORT 9000
#define MAX_SIZE 100

volatile sig_atomic_t gSignalInterrupt = 0;

bool create_file_and_send_it(char file_full_path[], char *full_packet, size_t length, int new_socket) 
{
    bool success;
    char *line = NULL;
    size_t len = 0;
   // declaring file pointers
    FILE *fp1;

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



int main(int argc, char *argv[])
{
    /*VARIABLES DECLARATIONS*********************
    ********************************
    ********************************
    */
    int server_fd=0;
    int new_socket=0;
    ssize_t valread =0;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char *full_packet = malloc(sizeof(char));
    char buffer[MAX_SIZE+1]=  {0};
    char new_line_char[] = "\n"; //new line variable
    char store_file[] =  "/var/tmp/aesdsocketdata";
    size_t len_packet = 0;
    int step = 0; //state machine steps
    size_t len_buf = 0;
    char *daemon_mode = argv[1]; //daemon parameter
    char *par = "-d";



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
    
    //  //removes aesdsocketdata file
    //  remove(store_file);

    //Open Log file
    //openlog(NULL, 0, LOG_USER);

    while(gSignalInterrupt !=1)
    {

        if (step == 0)
        {

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

            //loop while waiting for client to connect
            //the logic below is to make the listen no-blocking, so a signal handler can interrupt if necessary
            while (gSignalInterrupt !=1) 
            {
                
                FD_ZERO(&read_fds);
                fdmax = server_fd;
                FD_SET(server_fd,&read_fds);

                if (server_fd >= fdmax) 
                {
                    fdmax = server_fd + 1;
                }
                    select_status = select(fdmax, &read_fds, NULL, NULL, &timeout);
                    if (select_status == -1) {
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
                        break;  // we have data, we can accept now
                    }
                    // otherwise (i.e. select_status==0) timeout, continue 
            }         

            if (gSignalInterrupt == 0)
            {
                step = 1;
            }
            

        }
        else if (step ==1)
        {          
            //read buffer
            valread = read(new_socket, buffer, MAX_SIZE);
            if (valread >0)
            {
                //get length of buffer
                len_buf = strlen(buffer)-1;

            
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
                
                    if (create_file_and_send_it(store_file, full_packet, len_packet, new_socket) == true)
                    {
                        len_packet = 0;        
                       // memset(full_packet, 0, strlen(full_packet));
                        printf("All packets sent back to client\n"); 
                    }
                    else
                    {
                        printf("Failure creating file and sending it back to client\n");
                    }
                    
                }
                                                       
                //clear buffer size
                len_buf = 0;
                memset(buffer, 0, sizeof(buffer));

            }
            //client closed connection, buffer empty
            else
            {
                // closing the connected socket
                close(new_socket);
                // closing the listening socket
                close(server_fd);
                printf("Closed connection from  %s\n", inet_ntoa(address.sin_addr));
                syslog(LOG_DEBUG,"Closed connection from  %s\n", inet_ntoa(address.sin_addr));
                //
                step = 0;
            }         
        }

    }

    //removes aesdsocketdata file
    remove(store_file);
    // closing the connected socket
    close(new_socket);
    // closing the listening socket
    close(server_fd);
    //release memory
    free(full_packet);
    printf("Exiting application\n");
    return 0;
    exit (EXIT_SUCCESS);
}