/*******************************************************************************
 * // Begin statement                                                          *
 *                                                                             *
 * Author:        Dr. Nikolas Askitis                                          *
 * Email:         askitisn@gmail.com                                           *
 * Github.com:    https://github.com/naskitis                                  *
 *                                                                             *
 * Copyright @ 2016.  All rights reserved.                                     *
 *                                                                             *
 * Permission to use my software is granted provided that this statement       *
 * is retained.                                                                *
 *                                                                             *
 * My software is for non-commercial use only.                                 *
 *                                                                             *
 * If you want to share my software with others, please do so by               *
 * sharing a link to my repository on github.com.                              *
 *                                                                             *
 * If you would like to use any part of my software in a commercial or public  *
 * environment/product/service, please contact me first so that I may          *
 * give you written permission.                                                *
 *                                                                             *
 * This program is distributed without any warranty; without even the          *
 * implied warranty of merchantability or fitness for a particular purpose.    *
 *                                                                             *
 * // End statement                                                            *
 ******************************************************************************/
 
#include "include/common.h"

uint32_t NUM_SLOTS=131071, inserted=0, searched =0, search_miss=0;
uint64_t hash_mem=0, mtf_threshold=0, mtf_counter=0, ignored=0, mtf_threshold_basevalue=0;

char **hash_table=NULL;

/* 
 * the bitwise hash function, modified to use a mask.  Note,
 * that if you decide to use a mask, make sure that you subtract 1 
 * from the total number of slots or NUM_SLOTS prior to calling this
 * function, otherwise the mask wont work properly.
 * The original bitwise hash function was developed by Prof. Justin Zobel.
 * The bitwise hash function below is my edit of the original code to enable
 * more efficient modulus calculations. 
 */
uint32_t bitwise_hash(char *word)
{
  char c; 
  uint32_t h= 220373;
  
  for ( ; ( c=*word ) != '\0'; word++ ) h ^= ((h << 5) + c + (h >> 2));

#ifdef MASK

  return ( (uint32_t) ((h&0x7fffffff) & NUM_SLOTS )  );

#else
 
  return ( (uint32_t) ((h&0x7fffffff) % NUM_SLOTS ));  

#endif

}

/* resize a slot entry in the hash table */
void resize_array(uint32_t idx, uint32_t array_offset, uint32_t required_increase)
{
  /* if there is no slot, then create one with respect to the growth policy used */
  if(array_offset == 0)
  {
    #ifdef EXACT_FIT
   
    if ((*(hash_table + idx) = malloc(required_increase)) == NULL) fatal(MEMORY_EXHAUSTED);
    
    /* keep track of the amount of memory allocated */
    hash_mem += required_increase + 16;  
   
    #else    

    /* otherwise, grow the array with paging */
    /* if the required space is less than 32 bytes, than allocate a 32 byte block */
    if(required_increase <= _32_BYTES)
    {
      if ((*(hash_table + idx) = (char *) malloc(_32_BYTES)) == NULL) fatal(MEMORY_EXHAUSTED);  
      hash_mem += _32_BYTES + 16;  
    }
    /* otherwise, allocate as many 64-byte blocks as required */
    else
    {
      uint32_t number_of_blocks = ((int)( (required_increase-1) >> 6)+1);   

      if ((*(hash_table + idx) = (char *) malloc(number_of_blocks << 6)) == NULL) fatal(MEMORY_EXHAUSTED);
         
      /* keep track of the amount of memory allocated */ 
      hash_mem += (number_of_blocks << 6) + 16; 
    }
    
    #endif
  }
  else
  {
    /* otherwise, a slot entry (an array) is found which must be resized */
    #ifdef EXACT_FIT

    char *tmp = malloc(array_offset+required_increase);
    if(tmp == NULL) fatal (MEMORY_EXHAUSTED);
    
    /* copy the existing array into the new one */
    memcpy(tmp, *(hash_table + idx), array_offset+1);
    
    /* free the old array and assign the slot pointer to the new array */ 
    free( *(hash_table + idx) );
    *(hash_table + idx) = tmp;
 
    /* keep track of the amount of memory allocated */
    hash_mem = hash_mem - 1 + required_increase; 
    
    /* else grow the array in blocks or pages */
    #else 
    
    uint32_t old_array_size = array_offset + 1;
    uint32_t new_array_size = (array_offset+required_increase);
    
    /* if the new array size can fit within the previously allocated 32-byte block, 
     * then no memory needs to be allocated.
     */
    if ( old_array_size <= _32_BYTES  &&  new_array_size <= _32_BYTES )
    {
      return;
    }
    /* if the new array size can fit within a 64-byte block, then allocate only a
     * single 64-byte block.
     */
    else if ( old_array_size <= _32_BYTES  &&  new_array_size <= _64_BYTES)
    {  
      char *tmp = malloc(_64_BYTES);
      if(tmp == NULL) fatal (MEMORY_EXHAUSTED);
      
      /* copy the old array into the new */
      memcpy( tmp,  *(hash_table + idx), old_array_size);
      
      /* delete the old array */ 
      free( *(hash_table + idx) );

      /* assign the slot pointer to the new array */
      *(hash_table + idx) = tmp;

      /* accumulate the amount of memory allocate */
      hash_mem += _32_BYTES;
   
      return;
    }
    /* if the new array size can fit within a 64-byte block, then return */
    else if  (old_array_size <= _64_BYTES && new_array_size <= _64_BYTES )
    {
      return;
    }
    /* resize the current array by as many 64-byte blocks as required */
    else
    {
      uint32_t number_of_blocks = ((int)( (old_array_size-1) >> 6) + 1);
      uint32_t number_of_new_blocks = ((int)( (new_array_size-1) >> 6) + 1);

      if(number_of_new_blocks > number_of_blocks)
      {
        /* allocate as many blocks as required */
        char *tmp = malloc(number_of_new_blocks << 6);
        if (tmp==NULL) fatal(MEMORY_EXHAUSTED);
        
        /* copy the old array, a word at a time, into a new array */
        node_cpy( (uint32_t *) tmp, (uint32_t *) *(hash_table + idx), number_of_blocks<<6); 
        
        /* free the old array */
        free( *(hash_table + idx) );
        
        /* assign the slot pointer to the new array */
        *(hash_table + idx) = tmp;

        /* keep track of the number of bytes allocated */
        hash_mem += ((number_of_new_blocks-number_of_blocks)<<6); 
      } 
    } 

  #endif 
  }
}

/* frees up all the memory allocated by the slots of the array hash */
void hash_destroy()
{
  register uint32_t i=0; 
  
  for(i=0; i<NUM_SLOTS; i++) 
  {
    if (*(hash_table+i) != NULL)  free( *(hash_table+i) );
  }
}

/* 
 * checks whether a string exists in the hash table. 1 is returned 
 * if the string is found, 0 otherwise.
 */
uint32_t search(char *query_start)
{
   uint32_t register len=0;
   char *array, *query=query_start;
   char *word_start;
   
#ifdef MTF_ON
   char *array_start;
   char *word_start_with_len;
#endif  

   /* hash the query term using the bitwise hash function to acquire a slot */   
   if( (array = *(hash_table + bitwise_hash(query_start)) ) == NULL)  
   { 
     return false; 
   }
  
#ifdef MTF_ON 
   array_start=array;
#endif 

   /* the main search loop */
   loop:
   
   query=query_start; 
   
#ifdef MTF_ON   
   word_start_with_len=array;
#endif

   /* 
    * strings are length-encoded.  The first byte of each string is its length.  If however,
    * the most significant bit is set, this means that the length of the string is greater than 
    * 127 characters, and so, the next byte also represents the string length. In which case, the 
    * first byte is moved into an integer, followed by the second byte, thus forming the string length. 
    */   
   if( (len = (unsigned int) *array ) >= 128)
   {
     len = (unsigned int) ( ( *array & 0x7f ) << 8 ) | (unsigned int) ( *(++array) & 0xff );
   }

   /* 
    * once the length has been acquired, move to the next byte which represents the
    * first character of the string or the end-of-bucket flag (a null character)
    */
   array++;

   word_start = array;

   /* 
    * compare the query to the word in the array, a character a time until a mismatch
    * occurs or a null character is encountered 
    */   
   for (; *query != '\0' && *query == *array; query++, array++);
 
   /* 
    * if every character of the query string was compared and the length of
    * the query matches the length of the string compared, then its a match
    */
   if ( *query == '\0' && (array-word_start) == len ) 
   {      
       
#ifdef MTF_ON	 

       /* if the word found is the first word in the slot, then we don't need to move-to-front */
       if( word_start_with_len != array_start )
       {    
         /* otherwise move the word found to the start of the array, according to its length */
         if( len < 128 )   
         {
          /* slide the start of the array to the right by the length of the word and a byte (its length), 
           * then copy the query to the start of the array, along with its length to complete the move-to-front
           * process 
           */
           memmove(array_start + len + 1, array_start, (word_start_with_len-array_start));
           memcpy (array_start + 1, query_start, len);
	
           *array_start = (char) len;
         }
         else
         {
           /* move the string to the start of the array, plus 2 bytes for its length */
           memmove(array_start + len + 2, array_start, (word_start_with_len-array_start));
           memcpy (array_start + 2, query_start, len);
	   
           /* store the length of the string, which is broken up into two byte */
           *array_start = (char) ( len >> 8 ) | 0x80;
           *(array_start+1) = (char) ( len ) & 0xff; 
         }
	  
         mtf_counter++;
       }
#endif    

     return true;
   }
   
   /* a mismatch occurred during the string comparison phase. skip to the next word */
   array = word_start + len;
   
   /* if the next character is the end-of-bucket flag, then the search failed */
   if (*array == '\0') 
   {  
     return false;
   }
   
   /* otherwise, jump back up to the main search loop */
   goto loop; 
}


/* 
 * This function checks whether the string exists in the hash table.  If it does not exist,
 * then it can be inserted.  The function returns 0 on insertion failure, that is,
 * when the string is found, and 1 on successful insertion.
 */
uint32_t insert(char *query_start)
{  
   uint32_t register len, idx;
   uint32_t array_offset;
   char *array, *array_start, *query=query_start;
   char *word_start;
   
#ifdef MTF_ON
   char *word_start_with_len;
#endif   

   /* hash the query term to get the required slot */         
   idx=bitwise_hash(query_start);

   /* access the slot, if the slot is empty then proceed directly
    * to insert the string 
    */
   if( (array = *(hash_table + idx)) == NULL)  
   {  
     array_start=array;
     goto insert;
   }
  
   array_start=array;

   /* main search loop */
   loop:

   query=query_start; 

#ifdef MTF_ON   
   word_start_with_len=array;
#endif
   
   /* 
    * strings are length-encoded.  The first byte of each string is its length.  If however,
    * the most significant bit is set, this means that the length of the string is greater than 
    * 127 characters, and so, the next byte also represents the string length. In which case, the 
    * first byte is moved into an integer, followed by the second byte, thus forming the string length. 
    */   
   if( ( len = (unsigned int) *array ) >= 128 )
   {
     len = (unsigned int) ( ( *array & 0x7f ) << 8 ) | (unsigned int) ( *(++array) & 0xff );
   }
   
   /* 
    * once the length has been acquired, move to the next byte which represents the
    * first character of the string or the end-of-bucket flag (a null character)
    */
   array++;

   word_start = array;

   /* 
    * compare the query to the word in the array, a character a time until a mismatch
    * occurs or a null character is encountered 
    */ 
   for (; *query != '\0' && *query == *array; query++, array++);
   
   /* 
    * if every character of the query string was compared and the length of
    * the query matches the length of the string compared, then its a match
    */
   if ( *query == '\0' && (array-word_start) == len ) 
   { 

#ifdef MTF_ON	

       /* if the word found is the first word in the slot, then we don't need to move-to-front */
       if( word_start_with_len != array_start /* && ((word_start_with_len - array_start) > CACHE_LINE_SIZE) */ )
       {    
	       /* otherwise move the word found to the start of the array, according to its length */
         if( len < 128 )   
         {
	        /* slide the start of the array to the right by the length of the word and a byte (its length), 
           * then copy the query to the start of the array, along with its length to complete the move-to-front
           * process 
           */  
           memmove(array_start + len + 1, array_start, (word_start_with_len-array_start));
           memcpy (array_start + 1, query_start, len);
	   
           *array_start = (char) len;
         }
         else
         {
	         /* move the string to the start of the array, plus 2 bytes for its length */   
           memmove(array_start + len + 2, array_start, (word_start_with_len-array_start));
           memcpy (array_start + 2, query_start, len);
	    
	         /* store the length of the string, which is broken up into two byte */  
           *array_start = (char) ( len >> 8 ) | 0x80;
           *(array_start+1) = (char) ( len ) & 0xff; 
         }
	   
         mtf_counter++;
       }
#endif
     return false;   
   }

   /* a mismatch occurred during the string comparison phase. skip to the next word */
   array = word_start + len;
   
   /* if the next character is the end-of-bucket flag, then the search failed */
   if (*array == '\0') 
   {  
     goto insert;
   }
   
   goto loop; 
  
   insert:
 
   /* get the length of the string to insert */
   for(; *query != '\0'; query++);   
   len = query - query_start;
   
   /* get the size of the array */
   array_offset = array-array_start;
   
   /* resize the array to fit the new string */
   resize_array(idx, array_offset, ( len < 128 ) ? len+2 : len+3);
   
   /* reinitialize the array pointers, the point to the end of the array */
   array = *(hash_table+idx);
   array_start = array;
   array += array_offset;
     
   /* if the length of the string is less than 128 characters, then only a single byte is
    * needed to store its length
    */
   if( len < 128 )
   {
     *array = (char) len;
   }
   /* if the length of the string is greater than 128 characters, then two bytes are required to
    * store the string 
    */
   else 
   {
     *array     = (char) ( len >> 8) | 0x80;
     *(++array) = (char) ( len ) & 0xff; 
   }
   array++;

   /* copy the string into the array */
   while( *query_start != '\0' )
   {
     *array++ = *query_start++;
   }
   
   /* make sure the array is null terminated */
   *array='\0';

   return true;    
}

int main(int argc, char **argv)
{ 
   char *to_insert=NULL, *to_search=NULL;
   int num_files=0,i=0,j=0;
   double insert_real_time=0.0, search_real_time=0.0;

   /* get the number of slots to allocate */
   NUM_SLOTS=atoi(argv[1]);
   
   /* get the number of files to insert */
   num_files = atoi(argv[2]);
   
   /* allocate the space for the slots, and keep track of the amount of memory allocated */
   hash_mem += (sizeof(char *) * NUM_SLOTS) + 8;
   hash_table=(char **) calloc(NUM_SLOTS, sizeof(char *));
   if(hash_table == NULL) fatal(MEMORY_EXHAUSTED);
   
   /* if a mask is used, then make sure you subtract one from the total number of slots allocated */
#ifdef MASK
   NUM_SLOTS--;
#endif

   /* insert the sequence of files and accumulate the amount of time required */
   for(i=0, j=3; i<num_files; i++, j++)
   {
     to_insert=argv[j];     
     insert_real_time+=perform_insertion(to_insert);
   }
   
   uint64_t vsize=0;
   {
     pid_t mypid;
     FILE * statf;
     char fname[1024];
     uint64_t ret;
     uint64_t pid; 
     char commbuf[1024];
     char state;
     uint64_t ppid, pgrp, session, ttyd, tpgid;
     uint64_t flags, minflt, cminflt, majflt, cmajflt;
     uint64_t utime, stime, cutime, cstime, counter, priority;
     uint64_t timeout, itrealvalue;
     uint64_t starttime;
     uint64_t rss, rlim, startcode, endcode, startstack, kstkesp, ksteip;
     uint64_t signal, blocked, sigignore, sigcatch;
     uint64_t wchan;
     uint64_t size, resident, share, trs, drs, lrs, dt;
    
     mypid = getpid();
     snprintf(fname, 1024, "/proc/%u/stat", mypid);
     statf = fopen(fname, "r");
     ret = fscanf(statf, "%lu %s %c %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu "
       "%lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
       &pid, commbuf, &state, &ppid, &pgrp, &session, &ttyd, &tpgid,
       &flags, &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime,
       &cutime, &cstime, &counter, &priority, &timeout, &itrealvalue,
       &starttime, &vsize, &rss, &rlim, &startcode, &endcode, &startstack,
       &kstkesp, &ksteip, &signal, &blocked, &sigignore, &sigcatch,
       &wchan);
      
     if (ret != 35) {
        fprintf(stderr, "Failed to read all 35 fields, only %d decoded\n",
          ret);
     }
     fclose(statf);
   }

   /* get the number of files to search */
   num_files = atoi(argv[j++]);
  
   /* search the sequence of files and accumulate the amount of time required */
   for(i=0; i<num_files; i++, j++)
   {
     to_search=argv[j];     
     search_real_time+=perform_search(to_search);
   }
   
   /* Format of output:
    * Array hash 104.43 94.91 3.94 2.61 6000000 6000000 16384 ...
    *           (1)    (2)   (3)  (4)   (5)     (6)    (7)    (8)    
    * Legend:
    * 1.  the virtual memory size (MB)
    * 2.  the estimated memory size (MB)
    * 3.  elapsed time to insert (sec)
    * 4.  elapsed time to search (sec)
    * 5.  number of strings successfully inserted
    * 6.  number of strings successfully found
    * 7.  number of slots allocated
    * 8. contact and copyright info.
    */
   
   printf("Array-hash-table %.2f %.2f %.2f %.2f %d %d %d --- Dr. Nikolas Askitis, Copyright @ 2016, askitisn@gmail.com ",  (double) vsize/TO_MB,
   (double) hash_mem/TO_MB, insert_real_time, search_real_time, get_inserted(), get_found(), NUM_SLOTS);

#ifdef PAGING
   printf("%s", "Paging ");
#endif
 
#ifdef EXACT_FIT
   printf("%s", "Exact-fit ");
#endif

#ifdef MTF_ON
   printf(" MTF");
#endif

   puts("");

   /* free the amount of memory allocated */
   hash_destroy();
   free(hash_table);
   
   return 0; 
}
