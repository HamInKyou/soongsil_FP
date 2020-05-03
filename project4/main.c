#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sectormap.h"

void ftl_open();
void ftl_read(int lsn, char *sectorbuf);
void ftl_write(int lsn, char *sectorbuf);
void ftl_print();

int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);

FILE *flashfp;

int main(void)	{
	char sectorbuf[SECTOR_SIZE]; //섹터 data가 임시로 저장될 버퍼
	char c;
	int lsn;
	int i = 0;
	
	//flash memory file 생성
	if ((flashfp = fopen("flashfile", "w")) == NULL) {
		fprintf(stderr, "creat error for %s\n", "flashfile");
		exit(1);
	}
	//모든 블록의 각 바이트 0xff로 초기화
	for(i = 0; i < BLOCKS_PER_DEVICE; i++)
		dd_erase(i);
	fclose(flashfp);
	
	//address mapping table 생성과 초기화
	ftl_open();

	while(1){
		printf("어떤 작업을 하실건가요?(w는 쓰기, r은 읽기, p는 보여주기, e는 끝내기) >> ");
		scanf("%c", &c);
		getchar(); //표준입력 버퍼 비우기(개행)
		switch (c) {
			case 'w' :
				printf("lsn과 data를 입력해주세요 > ");
				scanf("%d %s", &lsn, sectorbuf);
				getchar();
				
				ftl_write(lsn, sectorbuf);
				break;
			case 'r' :
				printf("lsn >> ");
				scanf("%d", &lsn);
				getchar(); //표준입력 버퍼 비우기(개행)
				ftl_read(lsn, sectorbuf);
				if(sectorbuf[0] != (char)0xFF){
					i = 0;
					printf("%s\n", sectorbuf);
				}
				break;
			case 'p' :
				ftl_print();
				break;
			case 'e' :
				printf("프로그램을 종료합니다.\n");
				exit(0);
			default :
				printf("잘못된 입력입니다. 다시한번 입력해주세요\n");
		}
	}

}
