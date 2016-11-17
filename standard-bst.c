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

uint64_t memory_used=0;

/* the structure of a standard node */
typedef struct node
{
  struct node *left;
  struct node *right;
  char *str;
}node;

/* a pointer to the root node */
node *root_node=NULL;

/* initialize the standard-chain binary search tree by ensuring that
 * the root node pointer is null 
 */
int init()
{
  memory_used=0;
  root_node=NULL;
  return 0;
}

void in_order(node *node1);

/* perform an in order traversal of the BST to count the memory used and to the memory allocated */
void in_order(node *node1)
{
  if(node1==NULL) return;

  /* the total memory allocated is as follows:
   * the size of the node plus eight bytes for allocating the node,
   * the memory required by the string + one extra byte for its null 
   * character, and then a further eight bytes of allocation overhead 
   */
  memory_used += sizeof(struct node) + 16 + slen(node1->str)+1+16;

  in_order(node1->left);
  in_order(node1->right);
  free(node1->str);
  free(node1);
}

/* free the memory used by the BST and calculate the amount of space allocated */
void destroy()
{
   in_order(root_node);
}

/* search for a string in the BST and Return 1 on success, 0 otherwise  */
int search(char *word)
{
  char *key;
  node *current_node=root_node;
  
  int32_t result;
 
  if(root_node==NULL)
  {
    return 0;  
  }
  
  /* traverse the BST to find a string that matches or until a null pointer is 
   * encountered, on which, the search fails */
  while (1)
  {
    key = current_node->str;
    result=scmp(word, key);
  
    if(result==0)                 /* if the query matches the string in the current node */
    {
      return 1;
    }
    /* if the query is smaller than the string in the current node, then follow the left child-node pointer */
    else if(result < 0)         
    {
      current_node=current_node->left;

      if(current_node ==NULL)
      {
        return 0; 
      }
    }
    /* if the query is larger than the string in the current node, then follow the right child-node pointer */
    else
    {
       current_node = current_node->right;
    
       if(current_node == NULL)
       {
          return 0;
       }
    }
  }
}

/* search for the string to insert in the BST. 
 * if the string is found, then the insertion fails.  Otherwise, when a null
 * pointer is encountered, encapsulate the string in a standard
 * node and attach the node to the null-pointer encountered to complete the insertion 
 */
int insert(char *word)
{
  node *previous_node=NULL;
  node *current_node=root_node;
  char *key;
  int len=0;

  int32_t result;
  
  /* if there is no BST, then create it */
  if(root_node==NULL)
  {
    /* allocate space for the node */
    if ((root_node=malloc(sizeof(node))) == NULL) fatal (MEMORY_EXHAUSTED);
    len = slen(word)+1;

    /* allocate space for the string */
    if ((root_node->str=calloc(len, sizeof(char))) == NULL) fatal (MEMORY_EXHAUSTED);

    /* make sure the left and right child node pointers are null */
    root_node->left = NULL;
    root_node->right= NULL;
    key=root_node->str;  

    /* copy the string into the node to complete the insertion */
    for(; *word != '\0'; word++, key++)
    {
      *key=*word;
    }
    *key='\0';
    
    return 1;  
  }
  
  /* if the BST exists, then search for the string.  If a null pointer is 
   * encountered, then encapsulate the string into a standard node and attach 
   * the node to the null pointer encountered to complete the insertion
   */
  while (1)
  {
    key =  current_node->str;
    previous_node=current_node;

    result=scmp(word, key);
  
    if(result==0)
    {
      return 0;
    }
    else if(result < 0)
    {     
      current_node = current_node->left;
      
      /* if a null pointer is encountered, then allocate a new
       * standard node to encapsulate the string to insert, then assign
       * the node to the pointer to complete the insertion 
       */
      if(current_node ==NULL)
      {
        /* create and assign the node */
        if ((previous_node->left=malloc(sizeof(node))) == NULL) fatal (MEMORY_EXHAUSTED);
        len = slen(word)+1;

        /* allocate space for the string */
        if ((previous_node->left->str = calloc(len, sizeof(char))) == NULL) fatal (MEMORY_EXHAUSTED);
        current_node = previous_node->left;
        key = current_node->str;

        /* make sure the new node has its child pointer nulled */
        current_node->left=current_node->right=NULL;

        /* copy the string into the node to complete the insertion */
        for(; *word != '\0'; word++, key++)
        {
          *key=*word;
        }
        *key='\0';

        return 1; 
      }
    }
    else
    {
       current_node= current_node->right;
          
      /* if a null pointer is encountered, then allocate a new
       * standard node to encapsulate the string, then assign
       * it to the null pointer to complete the insertion 
       */
       if(current_node == NULL)
       {
         /* allocate space for the node */
         if ((previous_node->right=malloc(sizeof(node))) == NULL ) fatal (MEMORY_EXHAUSTED);
         len = slen(word)+1;

         /* allocate space for its string */
         if ((previous_node->right->str = calloc(len, sizeof(char))) == NULL) fatal (MEMORY_EXHAUSTED);
         current_node = previous_node->right;
         key = current_node->str;

         /* make sure the new node has its child pointer nulled */
         current_node->left=current_node->right=NULL;
       
         /* copy the string into the node to complete the insertion */
         for(; *word != '\0'; word++, key++)
         {
           *key=*word;
         }
         *key='\0';

         return 1;
       }
    }
  }
  return 0;
}

int main(int argc, char **argv)
{
   char *to_insert=NULL, *to_search=NULL;
   double mem=0,insert_real_time=0.0, search_real_time=0.0;
   int num_files=0,i=0,j=0;

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

   destroy();

   printf("Standard-BST %.2f %.2f %.2f %.2f %d %d --- Dr. Nikolas Askitis, Copyright @ 2016, askitisn@gmail.com\n", vsize/(double)TO_MB, memory_used/(double)TO_MB, insert_real_time, 
   search_real_time, get_inserted(), get_found());

   return 0; 
}
