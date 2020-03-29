#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
	int fd1, fd2;
	char buf[BUFFER_SIZE];
	off_t fsize; //filesize 저장하기 위한 변수
	int length; //

	if(argc !=  4){ //입력을 제대로 했는지
		fprintf(stderr,"usage : %s <filename> <offset> <nbyte>\n",argv[0]);
		exit(1);
	}
	
	//입력받은 파일을 읽기모드로 오픈
	if((fd1 = open(argv[1], O_RDONLY)) < 0){
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	//똑같은 파일을 읽기쓰기 모드로 오픈
	if((fd2 = open(argv[1], O_RDWR)) < 0){
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}
	
	fsize = lseek(fd1, 0, SEEK_END); //파일의 크기 저장
	length = atoi(argv[3]); //얼마만큼의 길이를 지우고싶은지 수치화
	//입력받은 오프셋에 지울 데이터 길이만큼의 수치만큼 더 이동
	lseek(fd1, (off_t)atoi(argv[2]) + length, SEEK_SET);
	//입력받은 오프셋만큼 이동
	lseek(fd2, (off_t)atoi(argv[2]), SEEK_SET);
	
	//지워서 땡겨질 데이터를 앞으로 당겨쓰는 작업
	while(1){
		if(lseek(fd1, 0, SEEK_CUR) == fsize)
			break;
		read(fd1, buf, length);
		write(fd2, buf, length);
	}
	//데이터 지운만큼 파일 사이즈가 작아져야하기에
	//당기고 남은 뒤의 여분의 데이터를 지워줌
	ftruncate(fd2, fsize-length);

	printf("finish delete!\n");
	close(fd1);
	close(fd2);
	exit(0);
}
