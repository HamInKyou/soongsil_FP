#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>

#define BUFFER_SIZE 100
int main(int argc, char *argv[])
{
	char buf[BUFFER_SIZE]; //파일에서 읽은걸 임시로 저장할 버퍼
	int fd1, fd2;
	int length;

	if(argc < 3){ //입력을 제대로 했는지
		fprintf(stderr, "usage : %s infile outfile\n", argv[0]);
		exit(1);
	}

	//입력받은 파일을 읽기모드로 연다.
	if((fd1 = open(argv[1],O_RDONLY)) < 0){
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}
	
	//복사해서 넣을 파일을 새로 생성한다.
	if((fd2 = creat(argv[2], 0666)) < 0){
		fprintf(stderr, "creat error for %s\n", argv[2]);
		exit(1);
	}

	//파일을 100바이트씩 읽어 복사할 파일에 쓴다.
	while((length = read(fd1, buf, BUFFER_SIZE)) > 0){
		write(fd2, buf, length);
	}
	
	printf("copy finish!\n");
	close(fd1);
	close(fd2);
	exit(0);
	
}
