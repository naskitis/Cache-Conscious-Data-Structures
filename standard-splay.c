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

#define ROTATEFAC 11

/* the node of a standard splay tree */
typedef struct wordrec
{
    char	   *word;
    struct wordrec *left, *right;
    struct wordrec *par;
} 
TREEREC;

/* the container that houses the root node of the splay tree */
typedef struct ansrec
{
    struct wordrec *root;
    struct wordrec *ans;
} 
ANSREC;

/* a pointer to the container housing the root node of the splay tree */
ANSREC	*ans;


/* the splaying operations are defined below */
#define ONELEVEL(dir,rid) \
    { \
	par->dir = curr->rid; \
	if( par->dir!=NULL ) \
	    par->dir->par = par; \
	curr->rid = par; \
	par->par = curr; \
	curr->par = NULL; \
    }

#define ZIGZIG(dir,rid) \
    { \
	curr->par = gpar->par; \
	if( curr->par!=NULL ) \
	{ \
	    if( curr->par->dir==gpar ) \
		curr->par->dir = curr; \
	    else \
		curr->par->rid = curr; \
	} \
	gpar->dir = par->rid; \
	if( gpar->dir!=NULL ) \
	    gpar->dir->par = gpar; \
	par->dir = curr->rid; \
	if( curr->rid!=NULL ) \
	    curr->rid->par = par; \
	curr->rid = par; \
	par->par = curr; \
	par->rid = gpar; \
	gpar->par = par; \
    }

#define ZIGZAG(dir,rid) \
    { \
	curr->par = gpar->par; \
	if( curr->par!=NULL ) \
	{ \
	    if( curr->par->dir==gpar ) \
		curr->par->dir = curr; \
	    else \
		curr->par->rid = curr; \
	} \
	par->rid = curr->dir; \
	if( par->rid!=NULL ) \
	    par->rid->par = par; \
	gpar->dir = curr->rid; \
	if( gpar->dir!=NULL ) \
	    gpar->dir->par = gpar; \
	curr->dir = par; \
	par->par = curr; \
	curr->rid = gpar; \
	gpar->par = curr; \
    }

int scount = ROTATEFAC;

/* search for word in a splay tree.  If word is found, bring it to
 * root, possibly intermittently.  Structure ans is used to pass
 * in the root, and to pass back both the new root (which may or
 * may not be changed) and the looked-for record. 
 */
int search(char *word)
{
    TREEREC	*curr = ans->root, *par, *gpar;
    int		val=1;

    scount--;

    if( ans->root == NULL )
    {
	ans->ans = NULL;
	return 0;
    }
    while( curr != NULL && (val = scmp(word, curr->word)) != 0 )
    {
	if( val > 0 )
	    curr = curr->right;
	else
	    curr = curr->left;
    }

    ans->ans = curr;

    if( scount<=0 && curr!=NULL )    /* Move node towards root */
    {
	scount = ROTATEFAC;

	while( (par = curr->par) != NULL )
	{
	    if( par->left==curr )
	    {
		if( (gpar = par->par) == NULL )
		    ONELEVEL(left,right)
		else if( gpar->left == par )
		    ZIGZIG(left,right)
		else
		    ZIGZAG(right,left)
	    }
	    else
	    {
		if( (gpar = par->par) == NULL )
		    ONELEVEL(right,left)
		else if( gpar->left == par )
		    ZIGZAG(left,right)
		else
		    ZIGZIG(right,left)
	    }
	}
	ans->root = curr;
    }


    if(val == 0)  return 1;
    return 0;
}

/* insert word into a splay tree.  If word is already present, bring it to
 * root, possibly intermittently.  Structure ans is used to pass
 * in the root, and to pass back both the new root (which may or
 * may not be changed) and the looked-for record. 
 */
int insert(char *word)
{
    TREEREC	*curr = ans->root, *par, *gpar, *prev = NULL, *wcreate();
    int		val=1;

    scount--;

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

    if( curr == NULL )
    {
	if( val > 0 )
	    curr = prev->right = wcreate(word, prev);
	else
	    curr = prev->left = wcreate(word, prev);
    }

    ans->ans = curr;

    if( scount<=0 )    /* Move node towards root */
    {
	scount = ROTATEFAC;

	while( (par = curr->par) != NULL )
	{
	    if( par->left==curr )
	    {
		if( (gpar = par->par) == NULL )
		    ONELEVEL(left,right)
		else if( gpar->left == par )
		    ZIGZIG(left,right)
		else
		    ZIGZAG(right,left)
	    }
	    else
	    {
		if( (gpar = par->par) == NULL )
		    ONELEVEL(right,left)
		else if( gpar->left == par )
		    ZIGZAG(left,right)
		else
		    ZIGZIG(right,left)
	    }
	}
	ans->root = curr;
    }

  if(val == 0)  return 0;
  return 1;
}

/* create a node to hold a word */
TREEREC * wcreate(char *word, TREEREC *par)
{
    TREEREC    *tmp;
    unsigned int len;

    tmp = (TREEREC *) malloc(sizeof(TREEREC));
    tmp->word = (char *) malloc( (len = strlen(word) + 1));
    strcpy(tmp->word, word);
    tmp->left = tmp->right = NULL;

    tmp->par = par;

    /* keep track on the amount of memory allocated, by adding
     * the size of the node allocated, its 8 bytes allocation overhead,
     * the length of the string allocated (which includes the null character)
     * and the eight byte allocation overhread incurred when allocating the string
     */
    memory_used += sizeof(TREEREC) + 16 + len + 16; 
    return(tmp);
}

int init()
{
  ans->root = ans->ans = NULL;
  
  /* initialize the memory counter to the amount of space required by the root
   * structure that houses the splay tree 
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
  
  printf("Standard-splay-tree %.2f %.2f %.2f %.2f %d %d --- Dr. Nikolas Askitis, Copyright @ 2016, askitisn@gmail.com\n", vsize/(double)TO_MB,
  memory_used/(double)TO_MB, insert_real_time, search_real_time, 
  get_inserted(), get_found());

  free(ans);
  return 0; 
}






