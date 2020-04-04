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
	// �Ʒ� �Լ��� �����ϸ� 'read_order_list' �迭�� ���� �����ϰ� �о�� �� ���ڵ� ��ȣ���� ������� �����Ǿ� �����
   // 'num_of_records'�� ���ڵ� ���Ͽ� ����Ǿ� �ִ� ��ü ���ڵ��� ���� �ǹ���
	gettimeofday(&startTime, NULL);
	GenRecordSequence(read_order_list, num_of_records);
	
	// 'read_order_list'�� �̿��Ͽ� ǥ�� �Է����� ���� ���ڵ� ���Ϸκ��� ���ڵ带 random �ϰ� �о���̰�,
   // �̶� �ɸ��� �ð��� �����ϴ� �ڵ� ������
	for(i = 0; i < num_of_records; i++){ //��� ���ڵ� Ž��
		lseek(fd, read_order_list[i]*RECORD_SIZE, SEEK_SET); //������ ��ġ�� �̵�
		read(fd, buf, RECORD_SIZE); //�̵��ѵ����� record(100����Ʈ)�б�
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
