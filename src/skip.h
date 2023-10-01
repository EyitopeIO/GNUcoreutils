
#ifndef SKIP_FILES_OR_DIRECTORIES_H
#define SKIP_FILES_OR_DIRECTORIES_H

#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <search.h>
#include <remove.h>


#define USE_LINKED_LIST 0


#define MAX_SKIP_FILES 9			// maximum number of files to skip
									// doesn't make sense to have so many files



typedef struct skip_node
{
	ino_t inode;
	struct skip_node *left;
	struct skip_node *right;
} skip_node_t;


#if USE_LINKED_LIST
#include <gnuastro/list.h>
int initialize_skipper_linklist(char *const file_name);
void free_skipper_linklist(void);
static int create_link_of_files(char *const file_name);
#endif

skip_node_t *search_skiptree(ino_t inode);
int initialize_skip(struct rm_options const *options, int fts_flags);
int should_be_skipped(ino_t inode);

#endif