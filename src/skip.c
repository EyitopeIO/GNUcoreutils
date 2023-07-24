
#include <skip.h>
#include <xfts.h>


static skip_node_t bst_global_root_node;	// binary search tree node

static int nskip;					// number of entries in skip file



#if USE_LINKED_LIST
static int create_link_of_files(char *const file_name);
static gal_list_str_t *list_of_files_to_skip = nullptr;
#endif
static char *const *create_argv_of_files(char *const file_name);
#if SKIPFILE_DEBUG_MODE
static void show_string_array(char *const *array, int n_elements);
#endif
static int create_bsearch_tree(char *const *file_names, int fts_flags);
static skip_node_t *insert_node(skip_node_t *root_node, ino_t inode);
static skip_node_t *search_node(skip_node_t *root_node, ino_t inode);



/*
* Function:  insert_node
* --------------------
* Inserts a node with a given inode in a binary search tree
*
* root_node: pointer to root node of binary search tree
* inode: inode of node to insert
*
* returns: pointer to root node of binary search tree
*
*/
static skip_node_t *insert_node(skip_node_t *root_node, ino_t inode)
{
	if (root_node == nullptr)
	{
		root_node = (skip_node_t*)malloc(sizeof(skip_node_t));
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

/*
* Function:  search_node
* --------------------
* Searches for a node with a given inode in a binary search tree
*
* root_node: pointer to root node of binary search tree
* inode: inode of node to search for
*
* returns: pointer to node with given inode if found, nullptr otherwise
*
*/
static skip_node_t *search_node(skip_node_t *root_node, ino_t inode)
{
	if (root_node == nullptr)
		return nullptr;

	if (inode == root_node->inode)
		return root_node;

	if (inode < root_node->inode)
		return search_node(root_node->left, inode);

	return search_node(root_node->right, inode);

}

/*
* API: search_skiptree
* --------------------
*/
int should_be_skipped(ino_t inode)
{
	skip_node_t *node = search_node(&bst_global_root_node, inode);
	if (node == nullptr)
	{
		// TODO: Tell user you're skipping this file if verbose enabled
#if SKIPFILE_DEBUG_MODE
		printf("should_be_skipped: node with inode %lu not in skiptree\n", inode);
#endif
		return -1;
	}
	return 0;
}


/*
* API: initialize_skip
* --------------------
*/
int initialize_skip(const struct rm_options *options, int fts_flags)
{
	char *const *files = create_argv_of_files(options->file_name);
	int bst = create_bsearch_tree(files, fts_flags);
#if SKIPFILE_DEBUG_MODE
	show_string_array(files, 5);
#endif
	return (files && bst) ? 0 : -1;
}



/*
 * Function:  create_bsearch_tree
 * --------------------
 * Creates a binary search tree using the inodes of the files  specified in argument
 * to `--skip'
 *
 * file_names: pointer to array of strings containing list files
 * specifiesd in file_name
 *
 * returns: 0 on success, -1 on failure
 */
static int create_bsearch_tree(char *const *file_names, int fts_flags)
{
	int rval = 0;
	struct stat file_info;
	for (int i = 0; i < nskip; i++)
	{
		/* TODO:
		1. Allow specifying `directory' instead of `./directory'
		2. Create tree immediately when reading files from the skip file.
	       making argv_of_files redundant
		*/
		if (lstat(file_names[i], &file_info) == -1)
		{
			// TODO: Show warning if verbose enabled
			rval = -1;
		}
		else
		{
			insert_node(&bst_global_root_node, file_info.st_ino);
#if SKIPFILE_DEBUG_MODE
			printf("-----------------------------\n");
			printf("create_bsearch_tree: inserting node\n\
path: %s\n\
inode:%lu\n", file_names[i], file_info.st_ino);
			puts("");
#endif
		}
	}
	return rval;
}


/*
 * Function:  create_argv_of_files
 * --------------------
 * Creates an array of strings from a file containing a list of
 * to not remove in `rm_fts' function in `remove.c'
 *
 * file_name: pointer to file name where user specified files to skip
 *
 * returns: pointer to array of char pointers containing list files
 * specifiesd in file_name
 */
static char *const *create_argv_of_files(char *const file_name)
{
    static char *argv_of_files[MAX_SKIP_FILES] = {nullptr};

    FILE *stream = fopen(file_name, "r");
	if (stream == nullptr)
    {
		perror("Could not open file passed to --skip");
        return nullptr;
    }
    char *lineptr = nullptr;
    size_t len = 0;
    ssize_t nread = 0;
	ssize_t cpysize = 0;

    // Read the first line. If that worked, it makes sense to continue
    nread = getline(&lineptr, &len, stream);
    if (nread == -1)
    {
        free(lineptr);
        fclose(stream);
        return nullptr;
    }
    else
	{
		// Copy all excluding the new line
		cpysize = sizeof(char) * (nread - 1);
        argv_of_files[nskip] = (char*)malloc(cpysize);
		memcpy(argv_of_files[nskip], lineptr, nread - 1);
		argv_of_files[nskip++][nread - 1] = '\0';
	}

    while (((nread = getline(&lineptr, &len, stream)) != -1) && nskip < MAX_SKIP_FILES)
	{
		cpysize = sizeof(char) * (nread - 1); // all minus the new line
        argv_of_files[nskip] = (char*)malloc(cpysize);
		memcpy(argv_of_files[nskip], lineptr, cpysize);
		argv_of_files[nskip++][nread - 1] = '\0';
	}

    if (nskip > MAX_SKIP_FILES)
		printf("only preserving first %d entries in %s\n", MAX_SKIP_FILES, file_name);

	free(lineptr);
    fclose(stream);

	return argv_of_files;
}


#if SKIPFILE_DEBUG_MODE
static void show_string_array(char *const *array, int n_elements)
{
	printf("------show_string_array-------\n");
    for (int i = 0; i < n_elements; i++)
    {
        printf("%d: %s\n", i, array[i]);
    }
	printf("-----------------------------\n");
}
#endif

#if USE_LINKED_LIST
int initialize_skipper_linklist(char *const file_name)
{
    list_of_files_to_skip = nullptr;
    return create_link_of_files(file_name);
}


void free_skipper_linklist(void)
{
    gal_list_str_free(list_of_files_to_skip, true);
}


static int create_link_of_files(char *const file_name)
{

	FILE *stream = fopen(file_name, "r");
	if (stream == nullptr)
    {
#if SKIPFILE_DEBUG_MODE
		puts("create_link_of_files: Could not open file");
#endif
		// TODO: Print this if verbose
		perror("Could not open skip file");
		return -1;
    }
    char *lineptr = nullptr;
    size_t len = 0;
    ssize_t nread = 0;

    // Read the first line. If that worked, it makes sense to continue

    nread = getline(&lineptr, &len, stream);
    if (nread == -1)
    {
        free(lineptr);
        fclose(stream);
#if SKIPFILE_DEBUG_MODE
        puts("create_link_of_files: could not allocate memory");
#endif
        return -1;
    }
    else
        gal_list_str_add(&list_of_files_to_skip, lineptr, nread);

    gal_list_str_print(list_of_files_to_skip);

    // Read and allocate 	FILE *stream = fopen(file_name, "r");
	if (stream == nullptr)
    {
        return -1;
    }
    char *lineptr = nullptr;
    size_t len = 0;
    ssize_t nread = 0;	FILE *stream = fopen(file_name, "r");
	if (stream == nullptr)
        return -1;

    char *lineptr = nullptr;
    size_t len = 0;
    ssize_t nread = 0;

    while ((nread = getline(&lineptr, &len, stream)) != -1)
        gal_list_str_add(&list_of_files_to_skip, lineptr, nread);

    gal_list_str_print(list_of_files_to_skip);

    free(lineptr);
    fclose(stream);

    return 0;
}
#endif
