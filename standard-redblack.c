/*******************************************************************************
 *                                                                             *
 * Permission to use this software is granted for non-commercial use only,     *
 * provided that this statement is retained.                                   *
 *                                                                             *
 * Developer: Prof. Justin Zobel, edited by Dr. Nikolas Askitis                *
 *                                                                             *
 * Copyright @ 2010.  All rights reserved.                                     *
 * This program is distributed without any warranty; without even the          *
 * implied warranty of merchantability or fitness for a particular purpose.    *
 *                                                                             *
 * Designed for 64-bit Linux platforms                                         *
 ******************************************************************************/

#include "include/common.h"

int inserted=0;
int searched=0;
uint64_t memory_used=0;

/* the representation of a standard red-black node */
typedef struct wordrec
{
    char	*word;
    struct wordrec *left, *right;
    struct wordrec *par;
    char	colour;
} 
TREEREC;

/* the container that houses the red-black tree */
typedef struct ansrec
{
    struct wordrec *root;
    struct wordrec *ans;
} 
ANSREC;

#define RED		0
#define BLACK		1

/* a pointer to the container that houses the red-black tree */
ANSREC	*ans;

/* find word in a redblack tree */
int search(char *word)
{
    TREEREC    *curr = ans->root;
    int		val=1;

    if( ans->root != NULL )
    {
	while( curr != NULL && (val = scmp(word, curr->word)) != 0 )
	{
	    if( val > 0 )
		curr = curr->right;
	    else
		curr = curr->left;
	}
    }

    ans->ans = curr;

    if(val==0) return 1; 
    return 0;
}

/* rotate the right child of par upwards.
 * Could be written as a macro, but not really necessary
 * as it is only called on insertion 
 */
void leftrotate(ANSREC *ans, TREEREC *par)
{
    TREEREC	*curr, *gpar;

    if( ( curr = par->right ) != NULL )
    {
	par->right = curr->left;
	if( curr->left != NULL )
	    curr->left->par = par;
	curr->par = par->par;
	if( ( gpar=par->par ) == NULL )
	    ans->root = curr;
	else
	{
	    if( par==gpar->left )
		gpar->left = curr;
	    else
		gpar->right = curr;
	}
	curr->left = par;
	par->par = curr;
    }
}

/* rotate the left child of par upwards */
void rightrotate(ANSREC *ans, TREEREC *par)
{
    TREEREC	*curr, *gpar;

    if( ( curr = par->left ) != NULL )
    {
	par->left = curr->right;
	if( curr->right != NULL )
	    curr->right->par = par;
	curr->par = par->par;
	if( ( gpar=par->par ) == NULL )
	    ans->root = curr;
	else
	{
	    if( par==gpar->left )
		gpar->left = curr;
	    else
		gpar->right = curr;
	}
	curr->right = par;
	par->par = curr;
    }
}

/* insert word into a redblack tree */
int insert(char *word)
{
    TREEREC    *curr = ans->root, *par, *gpar, *prev = NULL, *wcreate();
    int		val;

    if( ans->root == NULL )
    {
	ans->ans = ans->root = wcreate(word, NULL);
	return 1;
    }
    while( curr != NULL && (val = scmp(word, curr->word)) != 0 )
    {
	prev = curr;
	if( val > 0 )
	    curr = curr->right;
	else
	    curr = curr->left;
    }

    ans->ans = curr;

    if( curr == NULL ) /* insert a new node, rotate up if necessary */
    {
	if( val > 0 )
	    curr = prev->right = wcreate(word, prev);
	else
	    curr = prev->left = wcreate(word, prev);

	curr->colour = RED;
	while( (par = curr->par) != NULL
		&& ( gpar = par->par ) != NULL
		&& curr->par->colour == RED )
	{
	    if( par == gpar->left )
	    {
		if( gpar->right!=NULL && gpar->right->colour == RED )
		{
		    par->colour = BLACK;
		    gpar->right->colour = BLACK;
		    gpar->colour = RED;
		    curr = gpar;
		}
		else
		{
		    if( curr == par->right )
		    {
			curr = par;
			leftrotate(ans, curr);
			par = curr->par;
		    }
		    par->colour = BLACK;
		    if( ( gpar=par->par ) != NULL )
		    {
			gpar->colour = RED;
			rightrotate(ans, gpar);
		    }
		}
	    }
	    else
	    {
		if( gpar->left!=NULL && gpar->left->colour == RED )
		{
		    par->colour = BLACK;
		    gpar->left->colour = BLACK;
		    gpar->colour = RED;
		    curr = gpar;
		}
		else
		{
		    if( curr == par->left )
		    {
			curr = par;
			rightrotate(ans, curr);
			par = curr->par;
		    }
		    par->colour = BLACK;
		    if( ( gpar=par->par ) != NULL )
		    {
			gpar->colour = RED;
			leftrotate(ans, gpar);
		    }
		}
	    }
	}
	if( curr->par == NULL )
	    ans->root = curr;
	ans->root->colour = BLACK;
        return 1;
    }
    
    return 0;
}

/* create a node to hold a word */
TREEREC * wcreate(char *word, TREEREC *par)
{
    TREEREC    *tmp;
    unsigned int len=0;

    tmp = (TREEREC *) malloc(sizeof(TREEREC));
    tmp->word = (char *) malloc( (len = strlen(word) + 1));
    strcpy(tmp->word, word);
    tmp->left = tmp->right = NULL;

    tmp->par = par;

    memory_used += sizeof(TREEREC) + 16 + len + 16; 
    return(tmp);
}

int init()
{
  ans->root = ans->ans = NULL;

 /* initialize the memory counter to the amount of space required by the root
  * structure that houses the red-black tree 
  */ 
  memory_used = sizeof(struct ansrec) + 16;

  return 0;
}

int main(int argc, char **argv)
{
  char *to_insert=NULL, *to_search=NULL;
  double mem=0,insert_real_time=0.0, search_real_time=0.0;
  int num_files=0,i=0,j=0;
  
  if((ans = calloc(sizeof(struct ansrec), 1)) == NULL ) fatal(MEMORY_EXHAUSTED);

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

  printf("Standard-red-black-tree %.2f %.2f %.2f %.2f %d %d --- Dr. Nikolas Askitis, Copyright @ 2016, askitisn@gmail.com\n", vsize/(double)TO_MB,  
  memory_used/(double)TO_MB, insert_real_time, search_real_time, 
  get_inserted(), get_found());

  free(ans);
  return 0; 
}
