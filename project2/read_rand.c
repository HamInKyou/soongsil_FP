#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

#define SUFFLE_NUM	10000	
#define RECORD_SIZE  100

void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);

int main(int argc, char **argv)
{
	int *read_order_list;
	int num_of_records;
	int fd;
	struct stat statbuf;
	char buf[RECORD_SIZE];
	int i;
	struct timeval startTime, endTime;
	double diffTime;

	if( argc != 2 ) {
		fprintf(stderr, "usage : %s <datafile>\n", argv[0]);
		exit(1);
	}

	if (stat(argv[1],&statbuf) < 0) {
		fprintf(stderr, "stat error\n");
		exit(1);
	}

	num_of_records = statbuf.st_size / RECORD_SIZE;
	printf("#records: %d ", num_of_records);
	
	if ((fd = open(argv[1],O_RDONLY)) < 0){
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}
	// 아래 함수를 실행하면 'read_order_list' 배열에 추후 랜덤하게 읽어야 할 레코드 번호들이 순서대로 나열되어 저장됨
   // 'num_of_records'는 레코드 파일에 저장되어 있는 전체 레코드의 수를 의미함
	gettimeofday(&startTime, NULL);
	GenRecordSequence(read_order_list, num_of_records);
	
	// 'read_order_list'를 이용하여 표준 입력으로 받은 레코드 파일로부터 레코드를 random 하게 읽어들이고,
   // 이때 걸리는 시간을 측정하는 코드 구현함
	for(i = 0; i < num_of_records; i++){ //모든 레코드 탐색
		lseek(fd, read_order_list[i]*RECORD_SIZE, SEEK_SET); //랜덤한 위치로 이동
		read(fd, buf, RECORD_SIZE); //이동한데에서 record(100바이트)읽기
	}
	gettimeofday(&endTime, NULL);
	diffTime = endTime.tv_usec - startTime.tv_usec;
	printf("timecost: %d us\n", (int)diffTime);
		


	return 0;
}

void GenRecordSequence(int *list, int n)
{
	int i, j, k;

	srand((unsigned int)time(0));

	for(i=0; i<n; i++)
	{
		list[i] = i;
	}
	
	for(i=0; i<SUFFLE_NUM; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}

	return;
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;

	return;
}
