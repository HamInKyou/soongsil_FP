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
	int length;

	if(argc !=  4){ //입력이 잘되었는지
		fprintf(stderr,"usage : %s <filename> <offset> <data>\n",argv[0]);
		exit(1);
	}
	
	//읽기모드로 입력받은 파일 오픈(내용 읽는 용도)
	if((fd1 = open(argv[1], O_RDONLY)) < 0){
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}
	//똑같은파일을 읽고 쓰기 모드로 오픈(내용 읽어서 뒤로 밀어쓰고, 삽입하는 용도)
	if((fd2 = open(argv[1], O_RDWR)) < 0){
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	length = strlen(argv[3]); //삽입할 데이터 길이
	lseek(fd1, -length, SEEK_END); //끝에서 삽입할 데이터 길이만큼 뒤로 옮긴다.
	lseek(fd2, 0, SEEK_END); //뒤로 밀어지는 부분을 끝에서부터 쓸것이기 때문에
	while(1){
		if(lseek(fd2, 0, SEEK_CUR) == (off_t)atoi(argv[2])) //입력받은 오프셋에 도달하면 그만
			break;
		read(fd1, buf, length); //length만큼 전에서 읽고
		write(fd2, buf, length); //여기다 쓴다.

		//읽고 쓰는 작업을 통해 오프셋이 이동했으므로 다시 전으로 옮겨준다.
		lseek(fd1, 2*(-length), SEEK_CUR);
		lseek(fd2, 2*(-length), SEEK_CUR);
	}
	//삽입할 데이터를 써준다.
	write(fd2, argv[3], length);
	
	printf("finish insert!\n");
	close(fd1);
	close(fd2);
	exit(0);
}
