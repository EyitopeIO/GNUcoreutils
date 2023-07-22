
#ifndef SKIP_FILES_OR_DIRECTORIES_H
#define SKIP_FILES_OR_DIRECTORIES_H

#include <config.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <string.h>
#include <search.h>
#include <remove.h>


#define SKIPFILE_DEBUG_MODE 1

#define USE_LINKED_LIST 0


#define MAX_SKIP_FILES 64			// maximum number of files to skip
									// doesn't make sense to have so many files


#if USE_LINKED_LIST
#include <gnuastro/list.h>

/*
 * Function:  initialize_skipper
 * --------------------
 *
 */
int initialize_skipper_linklist(char *const file_name);

/*
 * Function:  free_skipper
 * --------------------
 *
 */
void free_skipper_linklist(void);


static int create_link_of_files(char *const file_name);

#endif


/*
 * Function:  should_be_skipped
 * --------------------
 * checks if the file should not be deleted

 *  file: pointer to structure returned by `fts_read'
 *
 *  returns: 0 if file is to be skipped; otherwise 1
 */
// int should_be_skipped(const FTSENT *const file);


/*
 * Function:  create_bst_from_files
 * --------------------
 * checks if the file should not be deleted
 *
 */
// static int create_bst_from_files(char *const file_name);


#endif