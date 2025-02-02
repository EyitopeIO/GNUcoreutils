/* Don't remove files and directories specified in skip file

   Copyright (C) 1998-2023 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

#ifndef USE_LINKED_LIST
#define USE_LINKED_LIST 0
#endif

#ifndef MAX_SKIP_FILES
#define MAX_SKIP_FILES 100      /* Maximum number of files to skip
                                   This is an arbitrary number */
#endif

typedef struct skip_node
{
	ino_t inode;
	struct skip_node *left;
	struct skip_node *right;
} skip_node_t;

skip_node_t *search_skiptree(ino_t inode);
int initialize_skip(char *filename, int fts_flags);
int should_be_skipped(ino_t inode);

#endif