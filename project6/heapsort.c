#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "person.h"
//필요한 경우 헤더 파일과 함수를 추가할 수 있음

// 과제 설명서대로 구현하는 방식은 각자 다를 수 있지만 약간의 제약을 둡니다.
// 레코드 파일이 페이지 단위로 저장 관리되기 때문에 사용자 프로그램에서 레코드 파일로부터 데이터를 읽고 쓸 때도
// 페이지 단위를 사용합니다. 따라서 아래의 두 함수가 필요합니다.
// 1. readPage(): 주어진 페이지 번호의 페이지 데이터를 프로그램 상으로 읽어와서 pagebuf에 저장한다
// 2. writePage(): 프로그램 상의 pagebuf의 데이터를 주어진 페이지 번호에 저장한다
// 레코드 파일에서 기존의 레코드를 읽거나 새로운 레코드를 쓸 때나
// 모든 I/O는 위의 두 함수를 먼저 호출해야 합니다. 즉, 페이지 단위로 읽거나 써야 합니다.

#define RECORD_PER_PAGE (PAGE_SIZE/RECORD_SIZE)

int page_amount;
int record_amount;
int deleted_page_head;
int deleted_record_head;
int heap_size;

//
// 페이지 번호에 해당하는 페이지를 주어진 페이지 버퍼에 읽어서 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET); //읽고 싶은 페이지 위치로 이동
	fread(pagebuf, PAGE_SIZE, 1, fp); //페이지 읽어서 pagebuf에 저장
}

//
// 페이지 버퍼의 데이터를 주어진 페이지 번호에 해당하는 위치에 저장한다. 페이지 버퍼는 반드시 페이지 크기와 일치해야 한다.
//
void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET); //쓰고 싶은 페이지 위치로 이동
	fwrite(pagebuf, PAGE_SIZE, 1, fp); //pagebuf에 있던 거 파일에 쓰기
}

void swap(char *string1, char *string2) {
	char tmpstring[RECORD_SIZE+1];
	strcpy(tmpstring, string1);
	strcpy(string1, string2);
	strcpy(string2, tmpstring);
}

int heapify(char **heaparray, int here)
{
	char tmp[RECORD_SIZE+1];
	char current_key[14];
	char parent_key[14];
	char *ptr;
	
	int parent;
	
	if(here == 0)
		return 0;
	
	//부모노드 인덱스 위치 구하기
	if(here % 2) //현재 들어온게 왼쪽 노드일 경우
		parent = floor(here/2);
	else //현재 들어온게 오른쪽 노드일 경우
		parent = (here/2) - 1;

	//현재 들어온 키값 뽑아내기
	strcpy(tmp, heaparray[here]);
	ptr = strtok(tmp, "#");
	strcpy(current_key, ptr);
	
	//부모의 키값 뽑아내기
	strcpy(tmp, heaparray[parent]);
	ptr = strtok(tmp, "#");
	strcpy(parent_key, ptr);
	
	if ( strcmp(current_key, parent_key) < 0 ){ //부모 키값이 더 크면
		swap(heaparray[here], heaparray[parent]); //바꿔주기
		heapify(heaparray, parent); //바꾸고 그 부모껄로 다시 키값 비교해가며 heap 만들기
	}

}

//
// 주어진 레코드 파일에서 레코드를 읽어 heap을 만들어 나간다. Heap은 배열을 이용하여 저장되며, 
// heap의 생성은 Chap9에서 제시한 알고리즘을 따른다. 레코드를 읽을 때 페이지 단위를 사용한다는 것에 주의해야 한다.
//
void buildHeap(FILE *inputfp, char **heaparray)
{
	char pagebuf[PAGE_SIZE] = {'\0', };
	char recordbuf[RECORD_SIZE] = {'\0', };

	int i, j, k = 0;
	

	readPage(inputfp, pagebuf, 0); //레코드 파일의 헤더 읽기
	if(strlen(pagebuf) == 0){
		fprintf(stderr, "input file에 저장된 내용이 없습니다.\n");
		exit(1);
	}

	memcpy(&page_amount, pagebuf, sizeof(int)); //헤더에 저장된 전체 페이지 수
	memcpy(&record_amount, pagebuf+4, sizeof(int)); //헤더에 저장된 전체 레코드 수
	memcpy(&deleted_page_head, pagebuf+8, sizeof(int)); //헤더에 저장된 삭제된 페이지의 인덱스(이 프로그램에선 삭제 취급x)
	memcpy(&deleted_record_head, pagebuf+12, sizeof(int)); //헤더에 저장된 삭제된 레코드의 인덱스(이 프로그램에선 삭제 취급x)


	for(i = 1; i <= page_amount; i++){
		readPage(inputfp, pagebuf, i); //페이지 하나 읽기
		for(j = 0; j < RECORD_PER_PAGE; j++){ //레코드 하나씩 검사
			if(k == record_amount)
				break;
			memcpy(recordbuf, pagebuf + (j * RECORD_SIZE), RECORD_SIZE); //레코드 뽑아내기
			strcpy(heaparray[k], recordbuf);
			heapify(heaparray, k); //힙에 추가
			k++;
		}
		if(k == record_amount)
			break;
	}
	
}

void pop(char **heaparray, char *pop_record) {
	strncpy(pop_record, heaparray[0], RECORD_SIZE); //제일 첫번째꺼 뽑아내고
	swap(heaparray[0], heaparray[heap_size-1]); //제일 끝에거랑 자리 바꿈
	memset(heaparray[heap_size-1], 0, RECORD_SIZE+1); //제일 끝에거 데이터 지워줌
	heap_size--; //heap사이즈 하나 줄인다.
}

int rebuildHeap(char **heaparray, int here){
	char tmp[RECORD_SIZE+1] = {'\0',};
	char left_key[14];
	char right_key[14];
	char min_key[14];
	char *ptr;
	
	int left = (here * 2) + 1;
	int right = (here * 2) + 2;
	int min = here;
	
	strcpy(tmp, heaparray[min]); //현재 값을 임시 최소값으로 지정
	ptr = strtok(tmp, "#");
	strcpy(min_key, ptr);
	
	if(left < heap_size){
		strcpy(tmp, heaparray[left]);
		ptr = strtok(tmp, "#");
		strcpy(left_key, ptr);
		if(strcmp(left_key, min_key) < 0) { //왼쪽자식이 더 작을 경우
			min = left; //왼쪽이 최소다.
			strcpy(tmp, heaparray[min]); //최소 변경
			ptr = strtok(tmp, "#"); //최소 변경
			strcpy(min_key, ptr); //최소 변경
		}
	}
	if(right < heap_size){
		strcpy(tmp, heaparray[right]);
		ptr = strtok(tmp, "#");
		strcpy(right_key, ptr);
		if(strcmp(right_key, min_key) < 0){ //오른쪽자식이 더 작을 경우
			min = right;
		}
	}
	

	if (min != here) {
		swap(heaparray[here], heaparray[min]);
		rebuildHeap(heaparray, min);
	}
}
//
// 완성한 heap을 이용하여 주민번호를 기준으로 오름차순으로 레코드를 정렬하여 새로운 레코드 파일에 저장한다.
// Heap을 이용한 정렬은 Chap9에서 제시한 알고리즘을 이용한다.
// 레코드를 순서대로 저장할 때도 페이지 단위를 사용한다.
//
void makeSortedFile(FILE *outputfp, char **heaparray)
{	
	char pagebuf[PAGE_SIZE] = {'\0', };
	char recordbuf[RECORD_SIZE] = {'\0', };
	int record_count;
	int page_count = 1;
	int i;
	
	heap_size = record_amount;
	
	//원본 헤더 그대로 다시 pagebuf에 저장
	memset(pagebuf, (char)0xFF, PAGE_SIZE);
	memcpy(pagebuf, &page_amount, sizeof(int));
	memcpy(pagebuf+4, &record_amount, sizeof(int));
	memcpy(pagebuf+8, &deleted_page_head, sizeof(int));
	memcpy(pagebuf+12, &deleted_record_head, sizeof(int));
	
	//원본헤더 다시 파일에 쓰기
	writePage(outputfp, pagebuf, 0);

	//페이지 0xFF로 초기화
	memset(pagebuf, (char)0xFF, PAGE_SIZE);
	record_count = 0;
	while(heap_size > 0){
		pop(heaparray, recordbuf); //제일 작은거 뽑아냄
		memcpy(pagebuf + (record_count*RECORD_SIZE), recordbuf, RECORD_SIZE);//페이지에 집어넣기
		record_count++;
		
		if(record_count == RECORD_PER_PAGE){ //페이지에 넣을거 가득 찼다면
			writePage(outputfp, pagebuf, page_count); //파일에 써주기
			page_count++; //페이지 카운트 늘리기
			record_count = 0; //레코드 카운트 초기화
			memset(pagebuf, (char)0xFF, PAGE_SIZE); //페이지 초기화
		}
		if(heap_size > 0)
			rebuildHeap(heaparray, 0);
	}
}

int main(int argc, char *argv[])
{
	FILE *inputfp;	// 입력 레코드 파일의 파일 포인터
	FILE *outputfp;	// 정렬된 레코드 파일의 파일 포인터
	char **heaparray; // 페이지를 담을 배열
	
	int i;

	heaparray = (char **)calloc(100, sizeof(char *));
	for(i = 0; i < 100; i++)
		heaparray[i] = (char *)calloc(RECORD_SIZE+1, sizeof(char));

	if(argc < 4) {
		fprintf(stderr, "Usage : %s s <infile> <outfile>\n", argv[0]);
		exit(1);
	}
	if(strcmp(argv[1], "s") == 0)
	{
		if((inputfp = fopen(argv[2], "r")) < 0) {
			fprintf(stderr, "fopen error!\n");
			exit(1);
		}
		if((outputfp = fopen(argv[3], "w")) < 0) {
			fprintf(stderr, "fopen error!\n");
			exit(1);
		}
		buildHeap(inputfp, heaparray);
		makeSortedFile(outputfp, heaparray);
	
	}
	else
	{
		fprintf(stderr, "option error!\n");
		exit(1);
	}
	
	for(i=0; i< 100; i++) //heaparray 할당 해제
		free(heaparray[i]);
	free(heaparray);

	fclose(inputfp);
	fclose(outputfp);
	return 1;
}
