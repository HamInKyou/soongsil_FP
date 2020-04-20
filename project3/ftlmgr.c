#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "flash.h"

int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);

FILE *flashfp;	// fdevicedriver.c에서 사용

int main(int argc, char *argv[])
{	
	char sectorbuf[SECTOR_SIZE];
	char pagebuf[PAGE_SIZE];
	char *blockbuf;
	int i=0;
	
	if (argv[1][0] == 'c') { //c옵션, 파일 생성
		if ((flashfp = fopen(argv[2],"w")) == NULL){
			fprintf(stderr, "creat error for %s\n", argv[2]);
			exit(1);
		}
		//모든 블록의 각 바이트 0xff로 초기화
		for(i = 0; i < atoi(argv[3]); i++){
			dd_erase(i);
		}
		fclose(flashfp);
	}
	else if (argv[1][0] == 'w') { //w옵션, 페이지 쓰기
		if ((flashfp = fopen(argv[2],"r+")) == NULL){
			fprintf(stderr, "open error for %s\n", argv[2]);
			exit(1);
		}
		memset(pagebuf,(char)0xFF, PAGE_SIZE); //일단 0xFF로 다 채워준다.
		memcpy(pagebuf,argv[4],strlen(argv[4])); //sector 부분에 덮어써준다.
		memcpy(pagebuf+SECTOR_SIZE,argv[5],strlen(argv[5])); //spare 부분에 덮어써준다.
		dd_write(atoi(argv[3]), pagebuf); //페이지 단위로 써주는 함수 호출

		fclose(flashfp);
	}
	else if (argv[1][0] == 'r') { //r옵션, 페이지 읽기
		if ((flashfp = fopen(argv[2],"r")) == NULL){
			fprintf(stderr, "open error for %s\n", argv[2]);
			exit(1);
		}
		dd_read(atoi(argv[3]), pagebuf); //페이지 단위로 읽는 함수 호출
		//읽어야 할 페이지에 의미있는 데이터 존재하지 않을 경우 끝내기
		if(pagebuf[0] == (char)0xFF && pagebuf[SECTOR_SIZE] == (char)0xFF)
			exit(1);
		//페이지 읽어서 출력하는 과정
		//sector에 있는 데이터 출력하는 과정
		while(pagebuf[i] != (char)0xFF){
			printf("%c", pagebuf[i]);
			i++;
		}
		printf(" ");
		i = SECTOR_SIZE;
		//spare에 있는 데이터 출력하는 과정
		while(pagebuf[i] != (char)0xFF){
			printf("%c", pagebuf[i]);
			i++;
		}
		printf("\n");

		fclose(flashfp);
	}
	else if (argv[1][0] == 'e') { //e옵션, 블록 소거
		if ((flashfp = fopen(argv[2],"r+")) == NULL){
			fprintf(stderr, "open error for %s\n", argv[2]);
			exit(1);
		}
		dd_erase(atoi(argv[3])); //블록 소거하는 함수 호출
		fclose(flashfp);
	}
	else {
		fprintf(stderr,"option list : c, w, r, e\n");
		exit(1);
	}

	return 0;
}
