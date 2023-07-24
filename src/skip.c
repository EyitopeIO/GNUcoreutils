
#include <skip.h>
#include <xfts.h>


static skip_node_t *bst_global_root_node = nullptr;	// binary search tree node

static int nskip;					// number of files in skip file



#if USE_LINKED_LIST
static int create_link_of_files(char *const file_name);
static gal_list_str_t *list_of_files_to_skip = nullptr;
#endif
static char *const *create_argv_of_files(char *const file_name);
static void show_string_array(char *const *array, int n_elements);
static int create_bsearch_tree(char *const *file_names, int fts_flags);
static skip_node_t *insert_node(skip_node_t *root_node, ino_t inode);
static skip_node_t *search_node(skip_node_t *root_node, ino_t inode);



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


static skip_node_t *search_node(skip_node_t *root_node, ino_t inode)
{
	if (root_node == nullptr)
		return nullptr;

	if (root_node->inode == inode)
		return root_node;

	if (inode < root_node->inode)
		return search_node(root_node->left, inode);

	return search_node(root_node->right, inode);
}

int should_be_skipped(FTSENT *ent)
{
	if (ent == nullptr)
		return -1;

	skip_node_t *node = search_node(bst_global_root_node, ent->fts_statp->st_ino);
	if (node == nullptr)
		return -1;

	return 0;
}

int initialize_skip(const struct rm_options *options, int fts_flags)
{
	printf("initialize_skip...\n");
	char *const *files = create_argv_of_files(options->file_name);
	int bst = create_bsearch_tree(files, fts_flags);
	show_string_array(files, 5);
	return (files && bst) ? 0 : -1;
}

static int create_bsearch_tree(char *const *file_names, int fts_flags)
{

	int rval = 0;
	struct stat file_info;
	for (int i = 0; i < nskip; i++)
	{
		// TODO: Allow specifying `directory' instead of `./directory'
		if (lstat(file_names[i], &file_info) == -1)
		{
			perror("File not found");
			// TODO: Show warning if verbose enabled
			rval = -1;
		}
		else
		{
			// TODO: Show note if verbose enabled
			insert_node(bst_global_root_node, file_info.st_ino);
			printf("inserted node for: %s and inod=%lu\n", file_names[i], file_info.st_ino);
		}
	}

	return rval;

}


/*
 * Function:  create_argv_of_files
 * --------------------
 * checks if the file should not be deleted
 *
 */
static char *const *create_argv_of_files(char *const file_name)
{
    static char *argv_of_files[MAX_SKIP_FILES] = {nullptr};

    FILE *stream = fopen(file_name, "r");
	if (stream == nullptr)
    {
		perror("Could not fille passed to --skip");
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
		puts("0: Could not open file");
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
        puts("1: could not allocate memory");
        return -1;
    }
    else
        gal_list_str_add(&list_of_files_to_skip, lineptr, nread);


    printf("First line content\n");
    gal_list_str_print(list_of_files_to_skip);
    printf("==================\n");


    // Read and allocate 	FILE *stream = fopen(file_name, "r");
	if (stream == nullptr)
    {
		puts("0: Could not open file");
        return -1;

    }
    char *lineptr = nullptr;
    size_t len = 0;
    ssize_t nread = 0;	FILE *stream = fopen(file_name, "r");
	if (stream == nullptr)
    {
		puts("0: Could not open file");
        return -1;

    }
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
