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
    for (size_t i = 0; i < 10; i++)
    {
        entry_size_total = (*buffer).entry[i].size + entry_size_total;
        if ((char_offset <= entry_size_total + (*buffer).entry[i+1].size) & (char_offset >= entry_size_total))
        {
            *entry_offset_byte_rtn = (char_offset - entry_size_total)-1;
            *entry = (*buffer).entry[i+1];
            return entry;
        }
        else if((char_offset <= entry_size_total))
        {
            *entry_offset_byte_rtn = char_offset;
            *entry = (*buffer).entry[i];
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

}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}

static void write_circular_buffer_packet(struct aesd_circular_buffer *buffer,
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
   // aesd_circular_buffer_init(&buffer);
   // write_circular_buffer_packet(&buffer,"write1\n"); 
    // for (size_t i = 0; i < 10; i++)
    // {
    //     char c[] ="write1\n";
    //     //har msg[] = strcat("write",c);
    //     (buffer).entry[i].buffptr= c;
    //     (buffer).entry[i].size = strlen(c);
    // }

    (buffer).entry[0].buffptr= "escrita0\n";
    (buffer).entry[0].size = strlen((buffer).entry[0].buffptr);

    (buffer).entry[1].buffptr= "escrita12\n";
    (buffer).entry[1].size = strlen((buffer).entry[1].buffptr);

    (buffer).entry[2].buffptr= "escrita123456\n";
    (buffer).entry[2].size = strlen((buffer).entry[2].buffptr);

    (buffer).entry[3].buffptr= "escrita1234567890\n";
    (buffer).entry[3].size = strlen((buffer).entry[3].buffptr);

    (buffer).entry[4].buffptr= "escrita1234567890abcd\n";
    (buffer).entry[4].size = strlen((buffer).entry[4].buffptr);
    
 
 // (buffer).entry[1].size = strlen("write1\n");
     struct aesd_buffer_entry *rtnentry = aesd_circular_buffer_find_entry_offset_for_fpos(&buffer,
                                                11,
                                                &offset_rtn);

   
}
