#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>

int main(int argc, char *argv[])
{
	int fd;
	int i;
	char c;

	if(argc != 4){ //입력을 제대로 했는지
		fprintf(stderr, "usage : %s <file> <offset> <nbyte>\n",argv[0]);
		exit(1);
	}
	//입력받은 파일을 읽기모드로 연다.
	if((fd = open(argv[1], O_RDONLY)) < 0){
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	//입력받은 오프셋으로 오프셋 이동시키기
	lseek(fd, (off_t)atoi(argv[2]), SEEK_SET);

	//입력받은 바이트만큼 출력
	for(i = 0; i < atoi(argv[3]); i++){	
		if((read(fd, &c, 1)) <= 0)
			exit(1);
		write(1, &c, 1);
	}
	printf("\n");
	exit(0);
	
}
