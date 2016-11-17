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

uint64_t array_offset=0;          /* the number of bytes from the start of the array to the last node */
uint64_t array_size=0;            /* the total size (in bytes) of the array */ 
int paging_size=10000000;         /* the block size (in bytes) that is used to resize the dynamic array */

/* the dynamic array housing the binary search tree */
char *bucket=NULL;

int init()
{
  bucket=NULL;     /* initialize the pointer to the dynamic array that houses the BST */
  array_size=0;    /* initialize the total size of the dynamic array (in bytes) that houses the BST */
  return 0;        
}

/* resize the dynamic array that houses the BST. Keep in mind, malloc() may impose an 
 * upper limit on the size of the array, depending upon your O/S and the amount of RAM.  
 * For example, your operating system may prevent you from allocating a single array 
 * that exceeds 2GB .. so if the array BST approaches 2GB in size, you may have to 
 * enable a variant of the compact chain, as detailed in my thesis.
 */
int resize_bst(uint32_t len)
{
  int to_return=0;

  /* if the dynamic array does not exist, then create it */
  if(array_offset == 0)
  {
    array_size=paging_size;  
    if ( (bucket = malloc(array_size)) == NULL) fatal(MEMORY_EXHAUSTED);    
  }
  else
  {   
    /* check whether adding the length of the string to insert and its two pointers
     * to the array offset exceeds the size of the array. If it does, then the array 
     * must be resized. 
     */ 
    if(array_offset + len + 16 >= array_size)
    {
      char *tmp = malloc(array_size+paging_size);     
      if(tmp == NULL) fatal (MEMORY_EXHAUSTED);

      /* copy the existing dynamic array into a new larger one */
      memcpy(tmp, bucket, array_size); 
      array_size+=paging_size;

      free(bucket);
      bucket = tmp;
    }
  }

  /* return the array offset, being the number of bytes from
   * the start of the array where the string is to be stored.  Then
   * increment the array offset to point to the next avaliable byte in the
   * dynamic array
   */
  to_return = array_offset;
  array_offset += len + 16;

  return to_return;
}

/* returns true if the string exists in the BST, false otherwise */
int search(char *word)
{
  char *key;             /* a pointer to a string */
  uint64_t *left;        /* a pointer to the left child-node pointer */
  uint64_t *right;       /* a pointer to the right child-node pointer */
  
  char *current_node=bucket;  /* a pointer to the current node in the array BST */
  int32_t result;            

  /* only continue if a BST exists */
  if(current_node==NULL)
  {
    return 0;
  }
  
  /* until a match is found or until a null child-pointer
   * is encountered, traverse the BST
   */
  while (1)
  {
    key = (current_node+16);                /* access the string in the current node */
    result=scmp(word, key);                 /* compare the string from the current node to the query */

    left = (uint64_t *)(current_node);      /* access to the left child-node pointer */
    right= (uint64_t *)(current_node+8);    /* access to the right child-node pointer */

    /* if the comparison is a match, then the string is found */
    if(result==0)
    {
      return 1; 
    }
    /* on mismatch, follow the left pointer if the query is less than the current node */
    else if(result < 0)
    {
      if(*left == 0)  return 0;
      current_node = (bucket+*left);
    }
    /* otherwise, follow the right pointer */
    else
    {
      if(*right == 0)  return 0;
      current_node = (bucket+*right);
    }
  }

  /* a match was not found */
  return 0;
}

/*
 * insert a string into the BST only after the string was not found.  
 * Returns 1 on success, 0 otherwise.
 */
int insert(char *word)
{
  char *current_node=bucket;       /* a pointer to the current node */
  char *key;                       /* a pointer to a string in the current node */ 
  uint64_t *left=0;                /* a pointer to the left child-node pointer */
  uint64_t *right=0;               /* a pointer to the right child-node pointer */

  int offset=0;
  int result=0;

  /* if the BST does not exist, then create it */
  if(bucket==NULL)
  {
    /* allocate a dynamic array to house the BST */
    offset=resize_bst(slen(word)+1);

    /* the string or root node is stored 8 bytes from the start of
     * the array, because the first 8 bytes store the left and right child-node
     * pointers 
     */
    key = (char *)(bucket+16); 

    /* make sure that the left and right child-node pointers are null */
    *(uint64_t *)(bucket) = *(uint64_t *)(bucket+8) = 0;

    /* copy the string into the dynamic array to complete the insertion */
    for(; *word != '\0'; word++, key++)
    {
      *key=*word;
    }
    *key='\0';
    
    return 1;  
  }
  
  /* until a match is found or a null child-node pointer is encountered,
   * traverse the BST. 
   */
  while(1)
  {
    left = (uint64_t *)(current_node);
    right= (uint64_t *)(current_node+8);
    key = (char *)(current_node+16);

    /* perform a string comparison */
    result=scmp(word, key);
  
    /* only insert the string if it is not found */
    if(result==0)
    {
      return 0;
    }
    else if(result < 0)
    {
      /* if a null left-node pointer is encountered, then we can insert the new string
       * by appending it to the end of the array, then assigning the null child-pointer 
       * to it 
       */  
      if( *left == 0)
      {
        /* check to see whether the dynamic array needs to be resized.  
         * calling resize_bst() will return the array offset where
         * the string is to be stored.  We store the array offset into
         * the "result" variable for brevity. 
         */
        result = resize_bst(slen(word)+1);

        /* assign the the empty left-child node pointer to point to the end of the array */
        left = (uint64_t *)(bucket+offset);
        *left = result;

        /* set the current node to point to the end of the array */
        current_node = (char *)(bucket+*left);

        left = (uint64_t *)(current_node);
        right= (uint64_t *)(current_node+8);
        key = (char *)(current_node+16);

        /* make sure that the current node has its left and right child-node pointers nulled */
        *left=*right=0;

        /* copy the string into the dynamic array to complete the insertion */
        for(; *word != '\0'; word++, key++)
        {
          *key=*word;
        }
        *key='\0';

        return 1;
      }

      /* store the array offset of the next node to access */
      offset=*left;

      /* move the the next node */
      current_node = (char *)(bucket+*left);
    }
    else
    {
      /* if the right child-node pointer is null, then we can insert the string */
      if( *right == 0)
      {
        /* resize the array if required, and return the array offset */
        result =  resize_bst(slen(word)+1);

        /* assign the right child-node pointer to point to the end of the array */
        right = (uint64_t *)(bucket+offset+8);
        *right = result;

        /* set the current node to point to the end of the array */
        current_node = (char *)(bucket+*right);

        left = (uint64_t *)(current_node);
        right= (uint64_t *)(current_node+8);
        key = (char *)(current_node+16);

        /* make sure that the current node has its left and right child node pointer nulled */
        *left=*right=0;
 
        /* append the string to the end of the array */
        for(; *word != '\0'; word++, key++)
        {
          *key=*word;
        }
        *key='\0';

        /* the insertion process is complete */
        return 1;
      }

      /* store the location of the next node to access */
      offset=*right;
  
      /* move the the next node */
      current_node = (char *)(bucket+*right);
    }
  }
}

/* destroy the BST by simply deleting the dynamic array that houses it */
void destroy()
{
  free(bucket);
}

int main(int argc, char **argv)
{
   int num_files=0,i=0,j=0;
   double insert_real_time=0.0, search_real_time=0.0;
   char *to_insert=NULL, *to_search=NULL;

   /* get the number of files to insert */
   num_files = atoi(argv[1]);

   init();
   
   /* insert each file in sequence and accumulate the time required */
   for(i=0, j=2; i<num_files; i++, j++)
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
   
   /* search each file in sequence and accumulate the time required */
   for(i=0; i<num_files; i++, j++)
   {
     to_search=argv[j];     
     search_real_time+=perform_search(to_search);
   }
   
   printf("Array-BST %.2f %.2f %.2f %.2f %u %u --- Dr. Nikolas Askitis, Copyright @ 2016, askitisn@gmail.com\n",
   vsize / (double) TO_MB, ((array_offset+16)/(double)TO_MB), insert_real_time, search_real_time, get_inserted(), get_found());

   destroy();
   return 0; 
}
