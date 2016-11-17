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

#define BUCKET_OVERHEAD 16
#define STRING_EXHAUST_TRIE 31
#define STRING_EXHAUST_CONTAINER 8
#define ALLOC_OVERHEAD 16

char **trie_pack=NULL;
uint32_t trie_pack_idx=0;
uint32_t trie_counter=0;
uint32_t trie_pack_entry_capacity=65536;
uint32_t trie_pack_capacity=256;
uint32_t total_trie_pack_memory=0;

int BUCKET_SIZE_LIM=35;
char *trie_buffer;
char *current_bucket;
char *root_trie;
int depth=0;
int num_buckets=0;
int num_tries=0;
int trie_buffer_capacity=65536;
int trie_buffer_size=0;
uint64_t bucket_mem=0;
int max_trie_depth=0;
int depth_accumumlator=0;
int mtf_counter=0;

void destroy();
void split_container(char *bucket, char **c_trie);
void burst_container(char *, char, char **);

/* the structure of a node in a container */
typedef struct node
{
  char *word;
  struct node *next;
}node;

/* the structure that houses a linked list of nodes */
typedef struct head_node
{  
   struct node *head;
   unsigned char overhead;
   //unsigned char a[7];
}head_node;

/* this function returns the address of a new trie node.  Trie
 * nodes are stored within a block of memory.  Once the block is
 * full, then a new block is allocated. 
 */
char * new_trie()
{
  /* check whether you need to allocate a new block */
  if(trie_counter == trie_pack_entry_capacity)
  {
    trie_pack_idx++;
    *(trie_pack+trie_pack_idx) = calloc(trie_pack_entry_capacity*TRIE_SIZE, sizeof(char));
    trie_counter=0;
  }

  /* return the address of the vacant node within the current block */
  return *(trie_pack + trie_pack_idx) + (trie_counter++ * TRIE_SIZE);
}

/* take a pointer and return 1 if it points to a trie node.  This can
 * be determined by checking whether the address lies within the blocks
 * of memory used to store the trie nodes 
 */
int is_it_a_trie(char *x)
{
  register int idx=0;
  for(; idx <= trie_pack_idx; idx++)
  { 
     if ( x >= *(trie_pack+idx) && x <= (*(trie_pack+idx)+(TRIE_SIZE * (trie_pack_entry_capacity-1) )) ) 
       return 1;
  } 

  return 0;
}

/* add a string to a bucket or container, return 1 on success, 0 otherwise */
int add_to_bucket(char *bucket, char *query_start)
{
  char *array;
  char *query=query_start;
  int entries=0;

  head_node *root = (head_node *)bucket;
  node *traverse = root->head;
  node *new_node;
  node *previous=NULL;
 
  /* traverse the container (its linked list) to find a match or
   * until the list is exhausted.  If you find a match, then move
   * the node to the start of the list.  You can only insert a 
   * new string when no match is found
   */
  while(traverse!=NULL)
  {
    array=traverse->word;
    query=query_start;
   
    /* compare the query string to the string held within the node */
    for (; *query == *array && *query != '\0'; query++, array++);
   
    /* if there was a match, then move to node to the start of the 
     * list if required, and terminate the insertion process in failure 
     */
    if (  *query == '\0' && *array == '\0')
    { 
      if(previous!=NULL)
      {
        previous->next=traverse->next;
        traverse->next=root->head;
        root->head=traverse;
        mtf_counter++;
      }
      return 0;   
    }  
   
    /* grab the next node in the list to compare */
    previous=traverse;
    traverse=traverse->next;
    entries++;
  }

  /* allocate a node node */
  if ((new_node = malloc(sizeof(struct node))) == NULL) fatal (MEMORY_EXHAUSTED);
  new_node->next=NULL;
 
  /* get the length of the string to insert */ 
  for(; *query != '\0'; query++);
  
  /* allocate space for the string */
  if ((new_node->word=malloc(query - query_start + 1)) == NULL) fatal(MEMORY_EXHAUSTED);
  array=new_node->word;

  /* copy the string within the node */
  while( *query_start != '\0')
  {
    *array++ = *query_start++;
  }
  *array='\0';
 
  /* assign the node to the list held within the container to complete the insertion */
  new_node->next=root->head;
  root->head=new_node;

  return ++entries;
} 

/* add a string to a container without searching if it exists */
char * add_to_bucket_no_search(char *bucket, char *query_start)
{
  char *array;
  char *query=query_start;
  head_node *root = (head_node *)bucket;
  node *new_node = NULL;

  /* allocate space for the node */
  if ((new_node = malloc(sizeof(struct node))) == NULL) fatal(MEMORY_EXHAUSTED);

  /* get the length of the string to insert */
  for(; *query != '\0'; query++);
  
  /* allocate space for the string */
  if ((new_node->word=malloc(query - query_start + 1)) == NULL) fatal(MEMORY_EXHAUSTED);
  new_node->next=NULL;
  
  array=new_node->word;

  /* copy the string into the node */
  while( *query_start != '\0')
  {
    *array++ = *query_start++;
  }
  *array='\0';
 
  /* assign the node to the linked list that is held within the container */
  new_node->next=root->head;
  root->head=new_node;

  return bucket;
} 

/* initalize the standard-chain burst trie */
int init()
{
  char **c_trie=NULL;
  int i=0;

  /* allocate the array of pointers that will be used to point to the
   * blocks of memory that house the trie nodes.
   */
  trie_pack = (char **) calloc (trie_pack_capacity, sizeof(char *));
  trie_pack_idx=0;
  trie_counter=0;
 
  /* assign the first pointer in the trie_pack array to block of memory */ 
  *(trie_pack+trie_pack_idx) = calloc(trie_pack_entry_capacity*TRIE_SIZE, 
  sizeof(char));

  /* allocate a new trie node and assign it as the root trie node */
  root_trie=new_trie();
  c_trie = (char **)root_trie;
  
  /* make sure its pointers are null */
  for(i=0; i<128; i++) *(c_trie+i)=NULL; 
  
  /* make sure you clear the string-exhaust flag in the trie node */
  *(c_trie+STRING_EXHAUST_TRIE)=0;

  return 0;
}

/* search for a string in a bucket or container. Returns 1 on success,
 * 0 otherwise
 */
int bucket_search(char *bucket, char *query_start)
{
 char *array;
 char *query=query_start;

 head_node *header = (head_node *) bucket;
 node *traverse = header->head;
 node *previous=NULL;
 
 /* traverse the container (its linked list) to find a match or 
  * until the list is exhausted
  */
 while(traverse!=NULL)
 {
   array=traverse->word;
   query=query_start;
   
   /* compare the query string to the string held within the node */
   for (; *query == *array && *query != '\0'; query++, array++);
  
   /* on match, move the node to the start of the list to complete the search */
   if ( *query == '\0' && *array == '\0')
   { 
      if(previous!=NULL)
      {
         previous->next=traverse->next;
         traverse->next=header->head;
         header->head=traverse;
	 mtf_counter++;
      }
      return 1;
   }
   
   /* grab the next node in the list */
   previous=traverse;
   traverse=traverse->next;
 }
 return 0;
}

/* search the standard-chain burst trie for a string.  Returns 1 on 
 * success, 0 otherwise 
 */
int search(char *word)
{
  char **c_trie = (char **)root_trie;
  char *x; 
  
  /* grab the leading character of the query string */
  while( *word != '\0')
  {
    /* use the leading character to fetch a pointer in the current trie,
     * if the pointer is null, then the string does not exist in the burst
     * trie 
     */
    if ( (x = *(c_trie + *word)) == NULL) 
    {
      return 0;
    }
    
    /* check if the pointer is refering to a container or a 
     * trie node.  
     */
    if ( is_it_a_trie(x) )
    {
      c_trie= (char **)x;
    }
    else  /* the pointer refers to a container */
    {
      /* consume the lead character from the query string */
      word++;   

      /* if there are no more characters in the query string, then
       * return the string-exhaust flag in the current container 
       */ 
      if( *word == '\0') 
      {
        if(*(x+STRING_EXHAUST_CONTAINER)) 
        {
          return 1;
        }
        else
        {
          return 0;
        }
      }
      
      /* if the string has not been exhausted, then search the container
       * using the remaining characters (the suffix)
       */
      return bucket_search(x, word); 
    }
    
    /* consume the lead character from the query string and continue to process
     * the next trie node 
     */
    word++;
  }
  
  /* if the string is exhausted prior to reaching a container, then check
   * the string-exhaust flag in the current trie node to decide the status
   * of the search.  
   */
  if( *(uint64_t *)(c_trie+STRING_EXHAUST_TRIE)) 
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

/* allocate a new container */
int new_container(char **c_trie, char path, char *word)
{
  char *x;

  /* allocate space for the container */
  if((x=malloc(BUCKET_OVERHEAD)) == NULL) fatal(MEMORY_EXHAUSTED);
  
  /* make sure the string-exhaust flag is cleared, and the
   * bytes used to store the pointer to the head of the list is
   * null.
   */
  *(uint64_t *)x=0;  
  *(x+STRING_EXHAUST_CONTAINER)=0;

  /* assign the parent pointer to the new container */
  *(c_trie + path)=x;
  
  if( *word == '\0' )
  {
    *(x+STRING_EXHAUST_CONTAINER) = 1;
  }
  else
  {
    add_to_bucket_no_search(x, word); 
  }
  
  return 1;
}

/* insert a string into the standard-chain burst trie */
int insert(char *word)
{
  char **c_trie=  (char **) root_trie;
  char *x; 
  int r=0;

  /* grab the leading character from the query string */
  while( *word != '\0')
  {
    /* if the pointer that maps to the leading character is null,
     * then create a new container to house the string, to complete
     * the insertion process
     */
    if ( (x = *(c_trie +  *word)) == NULL) 
      return new_container(c_trie, *word, word+1); 
         
    /* check whether the pointer that maps to the leading character 
     * leads to a trie node or to a container
     */
    if( is_it_a_trie(x) )
    {
      c_trie=(char  **)x;
    }
    else
    {
      /* consume the lead character */
      word++;
       
      /* if the query string has been consumed entirely, then set
       * the string-exhaust flag within the current node to complete
       * the insertion 
       */
      if( *word == '\0') 
      { 
        if( *(x+STRING_EXHAUST_CONTAINER))
	      {
	         return 0;
	      }
        else
	      {
	        *(x+STRING_EXHAUST_CONTAINER) = 1;
	        return 1;
	      }
      } 

      /* otherwise, a container is acquired.  Attempt to add the string
       * to the container.  If the function returns a non-null value,
       * then the insertion was a success. In this case, check to see
       * whether the container needs to be burst 
       */
      if( (r=add_to_bucket(x, word)) )
      {
        /* if the number of entries in the current container exceed the
         * container limit, then the container needs to be burst 
         */
        if( r > BUCKET_SIZE_LIM ) 
        {
	        burst_container(x, *(word-1), c_trie);
      	}
	
	      return 1;
      }
	
      return 0;
    }
      
    /* consume the current character and continue with the traversal */
    word++;
  }
  
  /* if the string was consumed prior to reaching a container, then 
   * set the string-exhaust flag within the current trie node to 
   * complete the insertion.  If it has already been set, then the
   * insertion is a failure. 
   */
  if(  *(uint64_t *)(c_trie+STRING_EXHAUST_TRIE)) 
  {
    return 0;
  }
  else
  {
    *(uint64_t *)(c_trie+STRING_EXHAUST_TRIE) = 1;
    return 1;
  }
}

int main(int argc, char **argv)
{
   char *to_insert=NULL, *to_search=NULL;
   int num_files=0,i=0,j=0;
   double mem=0, insert_real_time=0.0, search_real_time=0.0;
  
   /* get the container limit */
   BUCKET_SIZE_LIM = atoi(argv[1]);

   /* get the number of files to insert */
   num_files = atoi(argv[2]);
   
   init();
   
   /* insert the files in sequence into the standard-chain burst trie and
    * accumulate the time required
    */
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
  
   /* insert the files in sequence into the standard-chain burst trie 
    * and accumulate the time required
    */
   for(i=0; i<num_files; i++, j++)
   {
     to_search=argv[j];     
     search_real_time+=perform_search(to_search);
   }
   
   destroy();
   
   mem=((total_trie_pack_memory/(double)TO_MB) + ((double)bucket_mem/TO_MB));
   	
   printf("Standard-burst-trie %.2f %.2f %.2f %.2f %d %d %d --- Dr. Nikolas Askitis, Copyright @ 2016, askitisn@gmail.com\n", vsize / (double) TO_MB,
   mem, insert_real_time, search_real_time, get_inserted(), get_found(), BUCKET_SIZE_LIM);

   return 0; 
}

void burst_container(char *bucket, char c_char, char **c_trie)
{
   char *n_trie;
    
   /* allocate a new trie node as a parent */
   n_trie = new_trie();
   *(c_trie+c_char)=n_trie;
   c_trie = (char **) n_trie;  
     
   /* make sure you transfer the string-exhaust flag from the old container to the new trie node */
   *(uint64_t *)(c_trie+STRING_EXHAUST_TRIE) = (uint64_t ) *(bucket+STRING_EXHAUST_CONTAINER);
   
   /* reset the string exhaust flag in the container */
   *(bucket+STRING_EXHAUST_CONTAINER)=0;

   /* split the container, passing the reference to the new trie node into the function */
   split_container(bucket, c_trie);
}

void split_container(char *bucket, char **c_trie)
{
  head_node *root= (head_node *)(bucket);
  node *traverse=root->head;
  node *tmp;
  
  char *x;
  char *array;
   
  /* traverse the container (its linked list) */
  while(traverse!=NULL)
  {
    /* access the word stored within the current node in the container */
    array=traverse->word;

    /* use the leading character of the current word to map into the parent trie */
    x = *(c_trie + *array);
 
    /* if the acquired pointer in the parent trie is null, then create a new container */
    if (x == NULL)
    {
      if ((x=malloc(BUCKET_OVERHEAD))==NULL) fatal(MEMORY_EXHAUSTED);

      *(uint64_t *)(x)=0; 
      *(x+STRING_EXHAUST_CONTAINER)=0;

      /* assign the new container to the parent trie node */      
      *(c_trie + *array)=x;
    }
    
    /* if consuming the lead character results in an empty string, then set the
     * string-exhaust flag within the current container 
     */
    if( *(array+1) == '\0' ) 
    {
      *(x+STRING_EXHAUST_CONTAINER)= 1;
    }
    else
    {
       /* consume the lead character and add it to the container */
       array++; 
       add_to_bucket_no_search(x, array);
     }
     
     /* grab the next node in the list, but first keep a pointer to the current node */
     tmp=traverse;
     traverse=traverse->next;
   
     /* delete the current node and its string, as they are not needed anymore */
     free(tmp->word);
     free(tmp); 
  }
  
  /* once all the string in the current container have been moved into new containers,
   * delete the old container to complete the burst 
   */
  free(bucket);
}

/* run an in-order traversal of the burst trie to accumulate the amount of memory 
 * allocated and to free the space allocated
 */
void in_order(char **c_trie, int local_depth)
{
  int i=0;
  char *x;
 
  if(local_depth > max_trie_depth)  max_trie_depth=local_depth;
  
  num_tries++;
  
  for(i=MIN_RANGE; i<MAX_RANGE; i++)
  { 
    if ( (x = *(c_trie + i)) == NULL) 
    {
      continue;
    }
    
    if ( is_it_a_trie(x) )
    {
      in_order( (char **)x, local_depth+1);
    }
    else
    {
      head_node *header = (head_node *) x;
      node *traverse = header->head;
      node *previous=NULL;
 
      bucket_mem += BUCKET_OVERHEAD+16;
      
      while(traverse!=NULL)
      {
        bucket_mem += sizeof(struct node)+16;
	      bucket_mem += strlen(traverse->word)+1+16;
	
	      free(traverse->word);
        previous=traverse;
        traverse=traverse->next;
	
	      free(previous);
     }
   
     num_buckets++;
 
     depth_accumumlator+=local_depth;
     free(header);      
    }
  }
}

/* free the memory allocated by the standard burst trie, including the trie nodes */
void destroy()
{
  int i=0;
  in_order((char **)root_trie, 1); 

  for(i=0; i<=trie_pack_idx; i++) 
  {
    total_trie_pack_memory += (((trie_pack_entry_capacity*TRIE_SIZE) + sizeof(char))+ALLOC_OVERHEAD);
    free ( *(trie_pack + i ) );
  }
  free(trie_pack);  
}
