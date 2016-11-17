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

uint32_t NUM_SLOTS=131071, inserted=0, searched=0, search_miss=0;
uint64_t hash_mem=0, mtf_counter=0, mtf_threshold=0, mtf_threshold_basevalue=0;

/* the structure of a node */
typedef struct node  
{      
   char *word;
   struct node *next;
}node;

/* the structure of a slot entry */
typedef struct hash_table   
{
   struct node *head;
}hash_table;

struct hash_table *ds=NULL;

/* 
 * the bitwise hash function, modified to use a mask.  Note,
 * that if you decide to use a mask, make sure that you subtract 1 
 * from the total number of slots or NUM_SLOTS prior to calling this
 * funtion, otherwise the mask wont work properly. 
 * The original bitwise hash function was developed by Prof. Justin Zobel.
 * The bitwise hash function below is my edit of the original code to enable
 * more efficient modulus calculations. 
 */
uint32_t bitwise_hash(char *word)
{
  char c; 
  unsigned int h= 220373;
  
  for ( ; ( c=*word ) != '\0'; word++ ) h ^= ((h << 5) + c + (h >> 2));

#ifdef MASK

  return ( (uint32_t) ((h&0x7fffffff) & NUM_SLOTS )  );

#else
 
  return ( (uint32_t) ((h&0x7fffffff) % NUM_SLOTS ));  

#endif

}

/* free the memory allocated by the standard chain hash table */
void hash_destroy()
{
   node *current, *next;
   register uint32_t i=0;

   for(i=0; i<NUM_SLOTS; i++)
   {
     current = (ds+i)->head; 
     while(current != NULL)
     {
       next=current->next;
       free(current->word); 
       free(current);
       current=next;
     }
     (ds+i)->head=NULL;
   }
}

/* search for a string in the standard chain hash table */
uint32_t search(char *query_start)
{
   node *current_node, *previous_node=NULL;
   char *hashed_word, *query;
   
   uint32_t idx = bitwise_hash(query_start);
   current_node = (ds+idx)->head;
   
   /* traverse the linked list assigned to the slot selected by the
    * bitwise hash function until a match is found or until a null pointer
    * is encountered
    */
   while(current_node != NULL)
   { 
     hashed_word = current_node->word;
     query=query_start;
     
     /* compare the string to the string contained within the current node */
     for (; *query != '\0' && *query == *hashed_word; query++, hashed_word++); 
   
     /* check if the comparison is a match */
     if (*query == '\0' && *hashed_word == '\0') 
     {  
         /* move the current node that matched the string to the start of the list */
         if(previous_node!=NULL)
         {
           previous_node->next=current_node->next;
           current_node->next=(ds+idx)->head;
           (ds+idx)->head=current_node;
	        mtf_counter++;
         }
     
       return true; 
     }
      
     /* move to the next node in the chain, and keep a pointer to the previous node */
     previous_node = current_node;
     current_node  = current_node->next;
   }
   return false;
}

/* insert a string into a standard chain hash table */
uint32_t insert(char *query_start)
{
   node *current_node, *previous_node=NULL, *new_node;
   char *hashed_word, *query=query_start, *word;
   uint32_t len=0, idx = bitwise_hash(query_start);
   
   current_node = (ds+idx) -> head;
   
   /* traverse the slot selected by the bitwise hash table until a match 
    * is found or a until a null pointer is encountered 
    */
   while(current_node != NULL)
   { 
     hashed_word = current_node->word;
     query=query_start;
     
     /* compare the string to the string contained in the current node */
     for (; *query != '\0' && *query == *hashed_word; query++, hashed_word++); 
   
     /* if its a match, then the insertion is a failure */
     if (*query == '\0' && *hashed_word == '\0') 
     { 
         /* move to the node that matched the query to the start of the list */
         if(previous_node!=NULL)
         {
           previous_node->next=current_node->next;
           current_node->next= (ds+idx)->head;
           (ds+idx)->head=current_node;
	         mtf_counter++;
         } 

       return false;
     }
     
     /* move to the next node in the list, keeping track of the previous node */
     previous_node=current_node;
     current_node=current_node->next;
   }
   
   /* compute the length of the string to insert */
   for(; *query != '\0'; query++);
   len = query - query_start;

   /* allocate space for the new node and its string */
   if((new_node=malloc(sizeof(node)))==NULL) fatal(MEMORY_EXHAUSTED);
   if((new_node->word=malloc( len+1 ) )==NULL) fatal(MEMORY_EXHAUSTED);
   
   new_node->next=NULL;
   word = new_node->word;
   
   /* copy the string into the node */
   while( *query_start != '\0')
   {
     *word++ = *query_start++;
   }
   *word='\0';
   
   /* attach the new node to the list */
   if(previous_node==NULL)
     (ds+idx)->head=new_node;
   else
     previous_node->next=new_node;
   
   /* keep track of the amount of space consumed, including the operating
    * system overheads 
    */
   hash_mem += sizeof(node)+(len+1) + 16 + 16;
   
   return true;
}

int main(int argc, char **argv)
{ 
   char *to_insert=NULL, *to_search=NULL;
   int num_files=0,i=0,j=0;
   double insert_real_time=0.0, search_real_time=0.0;

   /* get the number of slots to assign to the hash table, must be a power of 2 */
   NUM_SLOTS=atoi(argv[1]);

   /* get the number of files to insert */
   num_files = atoi(argv[2]);
   
   /* allocate the slots used by the hash table */
   ds=(hash_table *) calloc(NUM_SLOTS, sizeof(struct hash_table));
   hash_mem += ( sizeof(struct hash_table) * NUM_SLOTS)+16;

#ifdef MASK
   NUM_SLOTS--;
#endif

   /* insert each file in sequence and accumulate the time required */
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
   
   /* search each file in sequence and acumulate the time required */
   for(i=0; i<num_files; i++, j++)
   {
     to_search=argv[j];     
     search_real_time+=perform_search(to_search);
   }
 
    printf("Standard-hash-table %.2f %.2f %.2f %.2f %d %d %d --- Dr. Nikolas Askitis, Copyright @ 2016, askitisn@gmail.com\n", (double) vsize/TO_MB, (double) hash_mem/TO_MB, insert_real_time, search_real_time, get_inserted(), get_found(), NUM_SLOTS);

   hash_destroy();
   free(ds);

  
   return 0; 
}
