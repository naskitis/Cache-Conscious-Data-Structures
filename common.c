#include "include/common.h"

/* the following code deals with the user interface */
static int total_searched=0;
static int total_inserted=0;
static int inserted=0;
static int found=0;

/* display an error message and exit the program */
void fatal(char *str) { puts(str); exit(1); }

/* copy a block of memory, a word at a time */
void node_cpy(uint32_t *dest, uint32_t *src, uint32_t bytes)
{
  bytes=bytes>>2;
  while(bytes != 0)
  {
    *dest++=*src++; 
    bytes--;
  } 
}

/* string compare routine  */
int32_t scmp(const char  *s1, const char  *s2)
{
  while(*s1 != '\0'  && *s1 == *s2 )
  {
    s1++;
    s2++;
  }
  return( *s1-*s2 );
}

/* 
 * scan an array of characters and replace '\n' characters
 * with '\0' 
 */
void set_terminator(char *buffer, int length)
{
  register int32_t i=0;
  for(; i<length; ++i)  
  {
    if( *(buffer+i) == '\n' )   
    {
      *(buffer+i) = '\0';
    }
  }
}

/* string length routine */
int32_t slen(char *word)
{
  char *x=word;
  for(; *x != '\0'; ++x);
  return x-word;
}

void reset_counters()
{
  total_searched=total_inserted=inserted=found=0;
}

int32_t get_inserted()
{
  return inserted;
}

int32_t get_found()
{
  return found;
}

/* access the data structure to insert the strings found in the 
 * filename that is provided as a parameter. The file supplied must
 * be smaller than 2GB in size, otherwise, the code below has to be
 * modified to support large files, i.e., open64(), lseek64().
 * This should not be required, however, since the caller to this
 * function should be designed to handle multiple files, to allow you 
 * to break a large file into smaller pieces.  
 */
double perform_insertion(char *to_insert)
{ 
   int32_t  input_file=0;
   int32_t  return_value=0;
   uint32_t input_file_size=0;
   uint32_t read_in_so_far=0;

   char *buffer=0;
   char *buffer_start=0;
   
   timer start, stop;
   double insert_real_time=0.0;
   
   /* open the file for reading */
   if( (input_file=(int32_t) open(to_insert, O_RDONLY))<=0) 
     fatal(BAD_INPUT);  
     
   /* get the size of the file in bytes */
   input_file_size=lseek(input_file, 0, SEEK_END);
     
   /* allocate a buffer in memory to store the file */
   if( (buffer = (char *)calloc(1, input_file_size+1 )) == NULL) 
     fatal(MEMORY_EXHAUSTED);
     
   /* keep a pointer to the start of the buffer */
   buffer_start=buffer;
   
   /* read the file into memory and close the file pointer */
   lseek(input_file, 0, SEEK_SET);

   /* attempt to read the entire file into memory */
   while(read_in_so_far < input_file_size)
   {
     return_value=read(input_file, buffer, input_file_size);
     assert(return_value>=0);
     read_in_so_far+=return_value;
   }
   close(input_file);
   
   /* make sure that all strings are null terminated */
   set_terminator(buffer, input_file_size);
   
   /* start the timer for insertion */  
   gettimeofday(&start, NULL);

   /* main insertion loop */
   time_loop_insert: 

   /* insert the first null-terminated string in the buffer */
   if(insert(buffer))
   {
     inserted++;
   } 
   total_inserted++;

   /* point to the next string in the buffer */
   for(; *buffer != '\0'; buffer++);
   buffer++;

   /* if the buffer pointer has been incremented to beyond the size of the file,
    * then all strings have been processed, and the insertion is complete. 
    */   
   if(buffer - buffer_start >= input_file_size) goto insertion_complete;
   goto time_loop_insert;

   insertion_complete:

   /* stop the insertion timer */
   gettimeofday(&stop, NULL);

   /* do the math to compute the time required for insertion */   
   insert_real_time = 1000.0 * ( stop.tv_sec - start.tv_sec ) + 0.001  
   * (stop.tv_usec - start.tv_usec );
   insert_real_time = insert_real_time/1000.0;

   /* free the temp buffer used to store the file in memory */
   free(buffer_start);
   
   /* return the elapsed insertion time */
   return insert_real_time;
}

/* access the data structure to search for the strings found in the 
 * filename that is provided as a parameter. The file supplied must
 * be smaller than 2GB in size, otherwise, the code below has to be
 * modified to support large files, i.e., open64(), lseek64().
 * This should not be required, however, since the caller to this
 * function should be designed to handle multiple files, to allow you 
 * to break a large file into smaller pieces.  
 */
double perform_search(char *to_search)
{
   int32_t  input_file=0;
   int32_t  return_value=0;
   uint32_t input_file_size=0;
   uint32_t read_in_so_far=0;

   char *buffer=0;
   char *buffer_start=0;
   
   timer start, stop;
   double search_real_time=0.0;

   /* attempt to open the file for reading */
   if( (input_file=(int32_t)open (to_search, O_RDONLY)) <= 0 ) fatal(BAD_INPUT); 
   
   /* get the size of the file */  
   input_file_size=lseek(input_file, 0, SEEK_END);

   /* create a buffer to match the size of the file */
   if( (buffer = (char *)calloc(1, input_file_size+1 )) == NULL) 
     fatal(MEMORY_EXHAUSTED);
     
   /* read the file into memory */
   buffer_start=buffer;
   lseek(input_file, 0, SEEK_SET);

   /* attempt to read the entire file into memory */
   while(read_in_so_far < input_file_size)
   {
     return_value=read(input_file, buffer, input_file_size);
     assert(return_value>=0);
     read_in_so_far+=return_value;
   }
   close(input_file);
   
   /* make sure each string is null terminated */
   set_terminator(buffer, input_file_size); 
 
   /* start the timer for search */
   gettimeofday(&start, NULL);
   
   time_loop_search: 
 
   /* search for the first null terminated string in the buffer */
   if(search(buffer))  
   {
     found++;
   }
   total_searched++;

   /* point to the next string in the buffer */
   for(; *buffer != '\0'; buffer++);
   buffer++;
   
   /* if the buffer pointer is incremented to beyond the size of the file,
    * then all strings have been processed 
    */
   if(buffer - buffer_start >= input_file_size) goto search_complete;
   goto time_loop_search;
   
   search_complete:
   
   /* stop the search timer */
   gettimeofday(&stop, NULL);
 
   /* do the math to compute the total time required for search */
   search_real_time = 1000.0 * ( stop.tv_sec - start.tv_sec ) + 0.001  
   * (stop.tv_usec - start.tv_usec );
   search_real_time = search_real_time/1000.0;
  
   /* free the file buffer */
   free(buffer_start);

   /* return the elapsed search time */
   return search_real_time;
}
