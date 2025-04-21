/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#endif

#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{

    struct aesd_buffer_entry *entry;
    entry = malloc(sizeof(struct aesd_buffer_entry));
   
    size_t entry_size_total = 0;
    size_t entry_size_total_prev = 0;
    for (size_t i = 0; i < 10; i++)
    {
        if (i>0) entry_size_total_prev = (*buffer).entry[i-1].size +  entry_size_total_prev;
        entry_size_total = ((*buffer).entry[i].size) + entry_size_total;
        
        if(char_offset < (*buffer).entry[0].size)
        {
            *entry_offset_byte_rtn = char_offset;
            *entry = (*buffer).entry[i];
            printf("%s",(*buffer).entry[i].buffptr);
            return entry;
        }     
        if ((entry_size_total > char_offset))
        {
            *entry_offset_byte_rtn = (char_offset - entry_size_total_prev);
            *entry = (*buffer).entry[i];
            printf("%s", (*entry).buffptr);
            return entry;
        }
    }
    return NULL;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
 
    /**
    * TODO: implement per description
    */
   size_t  in_off;
   size_t  out_off;

  

    in_off = (*buffer).in_offs;
    out_off = (*buffer).out_offs;
  
    if(in_off < 10) (*buffer).entry[in_off] = *add_entry;

    if(in_off < 10) printf("%s",(*buffer).entry[in_off].buffptr);
   
    in_off = in_off+1;


    if( in_off >10)
    {
        
         out_off =  out_off+1;
        for (size_t i = 0; i <10; i++)
        {
            if (i<9) (*buffer).entry[i] =  (*buffer).entry[i+1];
        }
        
        if( out_off >=10)
        {
              out_off = 0;
        }

        in_off = 9;
        (*buffer).entry[in_off] = *add_entry;
    }
        
     (*buffer).in_offs = in_off;
     (*buffer).out_offs = out_off;
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}

 /* static void write_circular_buffer_packet(struct aesd_circular_buffer *buffer,
                                         const char *writestr)
{
    struct aesd_buffer_entry entry;
    entry.buffptr = writestr;
    entry.size=strlen(writestr);
    aesd_circular_buffer_add_entry(buffer,&entry);
}


void main()
{

 
    struct aesd_circular_buffer buffer;
    size_t offset_rtn=0;
    aesd_circular_buffer_init(&buffer);
    write_circular_buffer_packet(&buffer,"write1\n"); 
    write_circular_buffer_packet(&buffer,"write2\n"); 
    write_circular_buffer_packet(&buffer,"write3\n"); 
    write_circular_buffer_packet(&buffer,"write4\n"); 
    write_circular_buffer_packet(&buffer,"write5\n"); 
    write_circular_buffer_packet(&buffer,"write6\n"); 
    write_circular_buffer_packet(&buffer,"write7\n"); 
    write_circular_buffer_packet(&buffer,"write8\n"); 
    write_circular_buffer_packet(&buffer,"write9\n"); 
    write_circular_buffer_packet(&buffer,"write10\n"); 
    write_circular_buffer_packet(&buffer,"write11\n");

    while (1) 
        {
             int n;

            // Reading an integer input
            printf("Input offset\n");
            scanf("%d", &n);             
        // (buffer).entry[1].size = strlen("write1\n");
            struct aesd_buffer_entry *rtnentry = aesd_circular_buffer_find_entry_offset_for_fpos(&buffer,
                                                        n,
                                                        &offset_rtn);
        }
        
}     */