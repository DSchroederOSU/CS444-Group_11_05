#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 256

struct statm_info {
	int total_size;
	int res_size;
	int shared_pgs;
	int text;
	int data_stack;
	int lib;
	int dirty_pgs;
};

void print_statm(struct statm_info * info)
{
	printf( "%d %d %d %d %d %d %d\t", info->total_size, info->res_size, info->shared_pgs, info->text, info->data_stack, info->lib, info->dirty_pgs);
}

int get_statm(struct statm_info * info)
{
	FILE *fp;
	int items;
	fp = fopen("/proc/self/statm", "r");
	if (fp == NULL) {
		printf("Could not open /proc/self/statm: %s\n", strerror(errno));
		return errno;
	}

	items = fscanf(fp, "%d %d %d %d %d %d %d", &info->total_size, &info->res_size, &info->shared_pgs, &info->text, &info->data_stack, &info->lib, &info->dirty_pgs);

	fclose(fp);

	if (items == 7)
		return 0;
	else {
		printf("fscanf error\n");
		return items;
	}
}

int get_status(char *buf)
{
	FILE *fp;
	fp = fopen("/proc/self/status", "r");
	if (fp == NULL) {
		printf("Could not open /proc/self/status: %s\n", strerror(errno));
		return errno;
	}

	while (!feof(fp)) {
		memset(buf, '\0', BUF_SIZE);
		fgets(buf, BUF_SIZE, fp);
		if (buf[0] == 'V' && buf[1] == 'm' && buf[2] == 'S' && buf[3] == 'i')
			break;
	}
	fclose(fp);

	if (feof(fp)) {
		printf("Error: reached end of file.\n");
		return -1;
	}
	return 0;
}

void usage()
{
	printf("Usage: ./a.out <NUM_TEST>\n");
}

int main (int argc, char **argv)
{
	char buf[BUF_SIZE];
	struct statm_info info;
	int ** test_arr, i;
	int NUM_TEST=10;

	if (argc > 2){
		usage();
		exit(1);
	}
	if (argc == 2) {
		NUM_TEST = atoi(argv[1]);
		if (NUM_TEST <= 0) {
			usage();
			exit(1);
		}
	}

	test_arr = malloc(NUM_TEST * sizeof(int *));

	printf("Starting to malloc...\n");

	for (i = 0; i < NUM_TEST; i++) {
		*test_arr = malloc((i+1)*1000*sizeof(int));
		get_statm(&info);
		printf("%d|\tPages: %d\t\t", i+1, info.total_size);
		get_status(buf);
		printf("%s", buf);
	}

//	printf("Done.\nPress [ENTER] to continue.\n");
//	getchar();
	printf("Freeing memory...");

	for (i = 0; i < NUM_TEST; i++) {
		free(test_arr[i]);
	}
	free(test_arr);

	printf("Done.\n");
	return 0;
	
}
