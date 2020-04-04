#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define RECORD_SIZE 100

int main(int argc, char **argv)
{
	int fd; //파일 디스크립터 저장
	struct stat statbuf; //열려는 파일 정보 저장하는 stat 구조체
	struct timeval startTime, endTime; //시간측정하기 위해(시작, 끝)
	double diffTime; //시간 측정하기 위해(차이)
	char buf[RECORD_SIZE]; //100바이트 레코드 받기 위해
	int record_amount; //레코드의 개수 저장하기 위해
	int i;

	if (argc != 2){ //입력 잘못되었을 때
		fprintf(stderr, "usage : %s <datafile>\n", argv[0]);
		exit(1);
	}
	
	if ((stat(argv[1], &statbuf)) < 0){ //열려는 파일의 정보 저장하기
		fprintf(stderr, "stat error\n");
		exit(1);
	}
	record_amount = statbuf.st_size / RECORD_SIZE; //record개수 계산
	printf("#records: %d ", record_amount);
	
	if ((fd = open(argv[1],O_RDONLY))< 0){
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	gettimeofday(&startTime, NULL); //시간측정 시작
	for(i = 0; i < record_amount; i++){ //순차적으로 레코드 끝까지 읽는다.
		read(fd, buf, RECORD_SIZE);
	}

	gettimeofday(&endTime, NULL); //시간측정 끝
	diffTime = endTime.tv_usec - startTime.tv_usec; //시간 계산
	printf("timecost: %d us\n", (int)diffTime);



	close(fd);
	return 0;
}
