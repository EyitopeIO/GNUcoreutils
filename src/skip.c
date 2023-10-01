/* skip.c -- core functions for ignoring files/directoeries to be removed
   Copyright (C) 1988-2023 Free Software Foundation, Inc.

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

/* Written by Eyitope Adelowo  */

#include <skip.h>
#include <xfts.h>


static skip_node_t bst_global_root_node;

static int create_bsearch_tree(char *skipfile, int fts_flags);
static skip_node_t *insert_node(skip_node_t *root_node, ino_t inode);
static skip_node_t *search_node(skip_node_t *root_node, ino_t inode);

/* Insert node into binary search tree */
static skip_node_t *insert_node(skip_node_t *root_node, ino_t inode)
{
	if (!root_node)
	  {
      root_node = (skip_node_t*)malloc(sizeof(skip_node_t));
      if (!root_node)
        {
#ifdef SKIPFILE_DEBUG_MODE
          fprintf(stderr, "insert_node: could not allocate memory\n");
#endif
          return nullptr;
        }
      root_node->inode = inode;
      root_node->left = nullptr;
      root_node->right = nullptr;
      return root_node;
	  }

	if (inode < root_node->inode)
		root_node->left = insert_node(root_node->left, inode);
	else if (inode > root_node->inode)
		root_node->right = insert_node(root_node->right, inode);

	return root_node;
}

/* Search for node in binary search tree */
static skip_node_t *search_node(skip_node_t *root_node, ino_t inode)
{
	if (!root_node)
		return nullptr;

	if (inode == root_node->inode)
		return root_node;

	if (inode < root_node->inode)
		return search_node(root_node->left, inode);

	return search_node(root_node->right, inode);
}

/* Returns true if a the inode of a file in the skip list should not be
   removed. Of course removing the inode means deleting the file */
int should_be_skipped(ino_t inode)
{
	skip_node_t *node = search_node(&bst_global_root_node, inode);
	if (!node)
	  {
#ifdef SKIPFILE_DEBUG_MODE
		  fprintf(stderr, "should_be_skipped: node with inode %lu not in skiptree\n", inode);
#endif
		  return -1;
	  }
	return 0;
}

/* Setup the binary search tree for the files specified in the skip
   for the files specified in the skip file */
int initialize_skip(char *filename, int fts_flags)
{
  int bst = create_bsearch_tree(filename, fts_flags);
  fprintf(stderr, "initialize_skip: bst created: %d\n", bst);
  return !bst ? 0 : -1;
}

/* Reads skip file and create a binary search tree out of their inodes */
static int create_bsearch_tree(char *skipfile, int fts_flags)
{
  struct stat file_info;
  int rval = 0;

  FILE *stream = fopen(skipfile, "r");
  if (!stream)
    {
#ifdef SKIPFILE_DEBUG_MODE
      fprintf(stderr, "create_bsearch_tree: could not open skip file\n");
#endif
      return -1;
    }

  char *lineptr = nullptr;
  size_t len = 0;
  ssize_t nread = 0;
  int nskip = 0;

  while (((nread = getline(&lineptr, &len, stream)) != -1))
    {
      if (lineptr[nread - 1] == '\n')
        lineptr[nread - 1] = '\0'; /* Remove the terminating newline.
                                      The last character may be EOF
                                      instead of newline */

      if (!strcmp(lineptr, "."))   /* `.' is interpreted by the shell.
                                      Plus it doesn't make sense to
                                      `rm -rf ./` and then skip `./' */
        {
          fprintf(stderr, "`.' found in skipfile. Nothing to do\n");
          rval = -1;
          break;
        }

      if (nskip > MAX_SKIP_FILES)
        {
          fprintf(stderr, "too many files in skip file (max=%d)\n", MAX_SKIP_FILES);
          rval = -1;
          break;
        }

      if (lstat(lineptr, &file_info) == -1)
        {
          rval = -1;
          break;
  #ifdef SKIPFILE_DEBUG_MODE
          fprintf(stderr,"create_bsearch_tree: could not stat file %s\n", lineptr);
  #endif
        }
      else
        {
          insert_node(&bst_global_root_node, file_info.st_ino);
          nskip++;
  #ifdef SKIPFILE_DEBUG_MODE
        fprintf(stderr, "-----------------------------\n");
        fprintf(stderr,
  "create_bsearch_tree: inserting node\n\
  path: %s\n\
  inode:%lu\n\n", lineptr, file_info.st_ino);
  #endif
        }
    }

  free(lineptr);
  return rval;
}
