// 주의사항
// 1. sectormap.h에 정의되어 있는 상수 변수를 우선적으로 사용해야 함
// 2. sectormap.h에 정의되어 있지 않을 경우 본인이 이 파일에서 만들어서 사용하면 됨
// 3. 필요한 data structure가 필요하면 이 파일에서 정의해서 쓰기 바람(sectormap.h에 추가하면 안됨)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "sectormap.h"

int dd_read(int ppn, char *pagebuf);
int dd_write(int ppn, char *pagebuf);
int dd_erase(int pbn);

extern FILE *flashfp;

typedef struct
{
	int lpn; //logical page number
	int ppn; //physical page number
} address_mapping;

//address_mapping_table 선언
//배열의 크기 플래시메모리상 섹터 수 -1 만큼 (DATAPAGES_PER_DEVICE) 
//address mapping table 의 필드에는 lpn과 ppn이 있다.
address_mapping address_mapping_table[DATAPAGES_PER_DEVICE];

//garbage 관리하기 위해 사용하는 테이블
//유효한 페이지일경우 1, 가비지 페이지일 경우 -1, 할당되지 않은 페이지일 경우 0
int garbage_table[PAGES_PER_BLOCK*BLOCKS_PER_DEVICE] = {0, };

//해당 페이지의 lpn값이 무엇인지 역으로 찾아가기 위해 만든 배열
//해당 페이지가 할당되어 있다면 lpn값, 안되어있다면 -1
int lpn_table[PAGES_PER_BLOCK*BLOCKS_PER_DEVICE] = {-1, };

int freeBlock_pbn; //freeblock의 pbn

// flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
// 초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
// file system에 의해 반드시 먼저 호출이 되어야 한다.
//
void ftl_open()
{
	int i;
	
	//address mapping table 초기화
	for(i = 0; i < DATAPAGES_PER_DEVICE; i++)
		address_mapping_table[i].ppn = -1;
	
	
	//BLOCKS_PER_DEVICE는 플래시 메모리상 블록 개수 ex) 0~15 = 16
	//DATABLKS_PER_DEVICE는 address mapping table상 블록 개수 ex) 0~14 = 15
	//인덱스의 끝은 블록 개수 끝보다 하나 작으므로
   //freeBlock의 실제 pbn은 플래시 메모리 끝 ex) 15
	//따라서 freeBlock_pbn은 BLOCKS_PER_DEVICE보다 1 작은 DATABLKS_PER_DEVICE
	freeBlock_pbn = DATABLKS_PER_DEVICE;
	
	return;
}

void ftl_read(int lsn, char *sectorbuf)
{	
	char pagebuf[PAGE_SIZE];
	int ppn;
	int i = 0;

	//sectorbuf 초기화
	memset(sectorbuf,(char)0xFF, SECTOR_SIZE);

	//address mapping table에서 주어진 lsn에 매핑되어 있는 ppn 값을 구하고,
	//이것이 가리키는 flash memory의 페이지 읽는다.
	//ppn 매핑 안되어있는 lpn인 경우
	if((ppn = address_mapping_table[lsn].ppn) == -1){
		printf("mapping된 page가 존재하지 않습니다.\n");
	}
	//ppn 매핑 되어있는 lpn인 경우
	else{
		dd_read(ppn, pagebuf); //페이지 읽어서
		memset(sectorbuf, 0xFF, SECTOR_SIZE); //sector버퍼 0xFF로 초기화해주고
		memcpy(sectorbuf, pagebuf, SECTOR_SIZE); //sector부분을 떼어 sector buf에 넣어준다.
	}
	return;
}


void ftl_write(int lsn, char *sectorbuf)
{
	char pagebuf[PAGE_SIZE];
	char temp_pagebuf[PAGE_SIZE];
	SpareData sparedata;
	int ppn;
	int gbn; //garbage로 지우기 위한 블록 넘버
	int find_gbn = 0; //garbage 발견했을 경우 반복문 끝내기 위해
	int can_write = 0; //freeblock 제외한 공간에 write할 수 있는지
	int i = 0;
	int j = 0;


	//ppn 정하기 위한 과정
	for(i = 0; i < BLOCKS_PER_DEVICE; i++){
		if(i == freeBlock_pbn) //freeBlock_pbn의 경우 건너뛰고
			continue;
		else{ //freeBlock_pbn이 아닌 경우
			for(j = 0; j < PAGES_PER_BLOCK; j++){  //페이지들 검사
				memset(pagebuf, (char)0xFF, PAGE_SIZE); //페이지 초기화하고
				dd_read(i*PAGES_PER_BLOCK+j, pagebuf); //페이지 읽는다.
				if(pagebuf[0] == (char)0xFF){ //페이지 비어있을 경우
					ppn = i*PAGES_PER_BLOCK+j; //ppn을 설정해준다.
					can_write = 1; //freeblock 제외한 공간에 write 할 수 있다.
					break; //반복문 나갈 수 있도록
				}
			}
		}
		//freeblock 제외한 공간에 write할 수 있으면 반복문 그만
		if(can_write == 1)
			break;
	}


	//프리블럭 제외하고 쓸 수 있는 공간이 하나도 없는 경우
	//가비지 공간 찾아내는 과정
	if(can_write != 1){ 
		for(i = 0; i < BLOCKS_PER_DEVICE; i++){
			for(j = 0; j < PAGES_PER_BLOCK; j++){
				if(garbage_table[i*PAGES_PER_BLOCK+j] == -1){ //가비지 공간 발견했을 때
					gbn = i; //가비지 처리할 블록 설정
					find_gbn = 1; //가비지 공간 발견했음을 표시하고
					break;
				}
			}
			if(find_gbn ==1) //가비지 공간 발견했을 겨우 반복 끝내기
				break;
		}
	
		//가비지 공간의 필요한 데이터만을 free블록으로 옮겨주고, address mapping table과 garbage_table 최신화
		//가비지 블록을 지워주고, address mapping table과 garbage_table 최신화
		//freeBlock_pbn도 최신화 시켜준다.
		can_write = 0; //다른데 새로 쓴 블록에서 쓸 곳을 찾았을 경우 표시하기 위해
		for(i = 0; i < PAGES_PER_BLOCK; i++){
			if(garbage_table[gbn*PAGES_PER_BLOCK+i] != -1){ //가비지 페이지가 아닐 경우
				dd_read(gbn*PAGES_PER_BLOCK+i, temp_pagebuf); //페이지를 읽어서
				dd_write(freeBlock_pbn*PAGES_PER_BLOCK+i, temp_pagebuf); //새 장소에 옮긴다.
				address_mapping_table[lpn_table[gbn*PAGES_PER_BLOCK+i]].ppn = freeBlock_pbn*PAGES_PER_BLOCK+i; //address mapping table 최신화
				garbage_table[freeBlock_pbn*PAGES_PER_BLOCK+i] = 1; //새로 옮긴 페이지가 유효하다고 표시
			}
			else{
				if(can_write == 1){ //이미 ppn 넣을 공간 찾았을 경우
					garbage_table[gbn*PAGES_PER_BLOCK+i] = 0; //가비지 블록부분 garbage_table에서 0으로
					continue;
				}
				else{ //처음 ppn 넣을 공간 찾았을 경우
					can_write = 1;
					ppn = freeBlock_pbn*PAGES_PER_BLOCK+i;
				}
			}
			garbage_table[gbn*PAGES_PER_BLOCK+i] = 0; //가비지 블록부분 garbage_table에서 0으로
		}
		dd_erase(gbn); //가비지 블록부분 다 지워준다.
		freeBlock_pbn = gbn; //프리블록을 다 지워준 블록으로
	}

	

	//pagebuf의 sector부분 채워주는 과정
	memset(pagebuf, (char)0xFF, PAGE_SIZE);
	memcpy(pagebuf, sectorbuf, strlen(sectorbuf));

	//spare부분 채워주는 과정
	sparedata.lpn = lsn; //spare에 lpn 저장해주고
	memcpy(pagebuf+SECTOR_SIZE, &sparedata, SPARE_SIZE);
	

	//address_mapping_table의 lpn 위치에 최초로 쓰기작업 수행한다는 것
	if(address_mapping_table[lsn].ppn == -1){
		address_mapping_table[lsn].ppn = ppn; //address mapping table 최신화
		garbage_table[ppn] = 1; //페이지가 유효하다는 걸 표시
		lpn_table[ppn] = lsn; //lsn값 저장
		dd_write(ppn, pagebuf);
	}
	//garbage_table에 원래 ppn에 해당하는 페이지가 가비지로 변했다는 것을 표시하고
	//다른 ppn 위치 값을 할당한다.
	//바뀐 ppn에 대하여 페이지가 유효하다는 것을 표시해준다.
	else {
		garbage_table[address_mapping_table[lsn].ppn] = -1;
		address_mapping_table[lsn].ppn = ppn; //address mapping table 최신화
		garbage_table[ppn] = 1; //페이지가 유효하다는걸 표시
		lpn_table[ppn] = lsn; //lsn값 저장
		dd_write(ppn, pagebuf);
	}

	return;
}

void ftl_print()
{	
	int i;
	printf("lpn ppn\n");
	for(i = 0; i < DATAPAGES_PER_DEVICE; i++){
		printf("%-3d %-3d\n", i, address_mapping_table[i].ppn);
	}
	printf("free block's pbn=%d\n", freeBlock_pbn);
	return;
}
