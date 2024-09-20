/**
 * C program to create a file and write data into file.
 */

#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>


//#define DATA_SIZE 1000

int main(int argc, char *argv[])
{
    /* Variable to store user content */
    //char data[DATA_SIZE];

    char *writefile = argv[1]; //file name with full path
    char *writestr = argv[2]; //string to write to file

  
    openlog(NULL, 0, LOG_USER);

    /* File pointer to hold reference to our file */
    FILE * fPtr;


         if(writefile == 0 || writestr == 0)
    {
        /* File not created hence exit */
        printf("One of parameters is empty.\n");
        syslog(LOG_ERR,"Error One of parameters is empty.\n");
        return 1;
        exit(1);
    }


    /* 
     * Open file in w (write) mode. 
     * "data/file1.txt" is complete path to create file
     */
    fPtr = fopen(writefile, "w");


    /* fopen() return NULL if last operation was unsuccessful */
    if(fPtr == NULL)
    {
        /* File not created hence exit */
        printf("Unable to create file.\n");
        syslog(LOG_ERR,"Error One of parameters is empty.\n");
        return 1;
        exit(1);
    }


    /* Input contents from user to store in file */
    //printf("Enter contents to store in file : \n");
    //fgets(data, DATA_SIZE, stdin);



    /* Write data to file */
    fputs(writestr, fPtr);


    /* Close file to save file data */
    fclose(fPtr);


    /* Success message */
    printf("File created and saved successfully. :) \n");
    syslog(LOG_DEBUG,"Write %s to %s.\n", writestr, writefile);

    return 0;
}