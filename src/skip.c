
#include <skip.h>
#include <xfts.h>



#if USE_LINKED_LIST
static int create_link_of_files(char *const file_name);
#endif
static char **create_argv_of_files(char *const file_name);
static void show_string_array(char **const array, int n_elements);
static void create_bsearch_tree(char **const arrayp, int *flags);


#if USE_LINKED_LIST
static gal_list_str_t *list_of_files_to_skip = nullptr;
#endif

typedef struct bst_node
{
    ino64_t *file_n;
    struct bst_node *left;
    struct bst_node *right;
} bst_node_t;


// static bst_node_t *root_node = nullptr;

static int nskip;					// number of files in skip file


int initialize_skipper(struct rm_options const *x, int *flags)
{

#if USE_LINKED_LIST
	return initialize_skipper_linklist(file_name) ? 0 : -1;
#else
	char **const arrayp = create_argv_of_files(x->file_name);
	create_bsearch_tree(arrayp, flags);
	return arrayp ? 0 : -1;
#endif
}

void free_skip_resources(void)
{

}

// int should_be_skipped(const FTSENT *const file)
// {
// 	return 1;
// }


/*
 * Function:  create_argv_of_files
 * --------------------
 * checks if the file should not be deleted
 *
 */
static char **create_argv_of_files(char *const file_name)
{
    static char *argv_of_files[MAX_SKIP_FILES] = {nullptr};

    FILE *stream = fopen(file_name, "r");
	if (stream == nullptr)
    {
		puts("0: Could not open file");
        return nullptr;

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
        return nullptr;
    }
    else
	{
		// Copy all excluding the new line
        argv_of_files[nskip] = (char*)malloc(nread - 1);
		strncpy(argv_of_files[nskip++], lineptr, nread - 1);
	}

    while (((nread = getline(&lineptr, &len, stream)) != -1) && nskip < MAX_SKIP_FILES)
	{
        argv_of_files[nskip] = (char*)malloc(nread - 1);
		strncpy(argv_of_files[nskip++], lineptr, nread - 1);
		// printf("arg: %s\n", argv_of_files[nskip - 1]);
	}

    if (nskip > MAX_SKIP_FILES)
        printf("only preserving first %d entries in %s\n", MAX_SKIP_FILES, file_name);

    show_string_array(argv_of_files, 5);

	free(lineptr);
    fclose(stream);

	return argv_of_files;
}


static void create_bsearch_tree(char **const arrayp, int *flags)
{

}

#if SKIPFILE_DEBUG_MODE
static void show_string_array(char **const array, int n_elements)
{
	printf("------show_string_array-------\n");
    for (int i = 0; i < n_elements; i++)
    {
        printf("%d: %s", i, array[i]);
        puts("");
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
