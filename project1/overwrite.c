#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>

int main(int argc, char *argv[])
{
	int fd;
	int length;

	if(argc != 4){ //입력이 제대로 잘 되었는지
		fprintf(stderr,"usage : %s <filename> <offset> <data>",argv[0]);
		exit(1);
	}

	if((fd = open(argv[1], O_RDWR)) < 0){ //읽기쓰기모드로 파일 연다.
		fprintf(stderr,"open error for %s\n", argv[1]);
		exit(1);
	}
	
	//입력받은 오프셋 위치로 오프셋 이동
	lseek(fd, (off_t)atoi(argv[2]), SEEK_SET);
	//입력한 데이터의 길이 수치화해서 저장
	length = strlen(argv[3]);
	//데이터를 파일에 덮어쓴다.
	write(fd, argv[3], length);
	
	printf("finish overwrite!\n");
	close(fd);
	exit(0);
}
