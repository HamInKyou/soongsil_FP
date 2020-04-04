#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>

#define BUFFER_SIZE 100

int main(void)
{
	char *fname = "records.txt";
	char buf[BUFFER_SIZE];
	int fd;
	int i;
	int filesize;

	if ((fd = open(fname,O_RDWR|O_CREAT|O_TRUNC,0666))< 0){
		fprintf(stderr, "open error for %s\n", fname);
		exit(1);
	}

	for(i = 0; i <= 1000; i++){
		write(fd,buf,100);
	}
	
	filesize = lseek(fd,0,SEEK_END);
	printf("file size : %d\n",filesize);


}
