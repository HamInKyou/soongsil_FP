#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>

int main(int argc, char *argv[])
{
	int fd1, fd2;
	char c;

	if(argc !=3){ //입력을 제대로 했는지
		fprintf(stderr, "usage : %s <file1> <file2>\n", argv[0]);
		exit(1);
	}
	
	//첫번째로 입력받은 파일을 읽기 모드로 연다.
	if((fd1 = open(argv[1],O_RDWR)) < 0){
		fprintf(stderr, "oepn error for %s\n", argv[1]);
		exit(1);
	}
	//두번째로 입력받은 파일을 읽기쓰기 모드로 연다.
	if((fd2 = open(argv[2],O_RDONLY)) < 0){
		fprintf(stderr, "oepn error for %s\n", argv[2]);
		exit(1);
	}
	
	//파일을 맨 끝의 한칸 전으로 이동한다(개행방지)
	lseek(fd1, -1, SEEK_END);

	//두번 째 파일을 첫번째 파일의 끝에 갖다붙인다.
	while(1){
		if(read(fd2, &c, 1) <= 0)
			exit(1);
		write(fd1, &c, 1);
	}
	
	printf("finish merge!\n");
	close(fd1);
	close(fd2);
	exit(0);
}
