#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "person.h"

#define RECORD_PER_PAGE (PAGE_SIZE/RECORD_SIZE)

void readPage(FILE *fp, char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fread(pagebuf, PAGE_SIZE, 1, fp);

}

void writePage(FILE *fp, const char *pagebuf, int pagenum)
{
	fseek(fp, PAGE_SIZE*pagenum, SEEK_SET);
	fwrite(pagebuf, PAGE_SIZE, 1, fp);

}

void pack(char *recordbuf, const Person *p)
{
	char *pos;
	pos = recordbuf;

	memset((void *)recordbuf, (char)0xFF, RECORD_SIZE);

	memcpy(pos, p->sn, strlen(p->sn));
	pos = pos + strlen(p->sn);
	memcpy(pos, "#", 1);
	pos += 1;

	memcpy(pos, p->name, strlen(p->name));
	pos = pos + strlen(p->name);
	memcpy(pos, "#", 1);
	pos += 1;

	memcpy(pos, p->age, strlen(p->age));
	pos = pos + strlen(p->age);
	memcpy(pos, "#", 1);
	pos += 1;

	memcpy(pos, p->addr, strlen(p->addr));
	pos = pos + strlen(p->addr);
	memcpy(pos, "#", 1);
	pos += 1;

	memcpy(pos, p->phone, strlen(p->phone));
	pos = pos + strlen(p->phone);
	memcpy(pos, "#", 1);
	pos += 1;
	
	memcpy(pos, p->email, strlen(p->email));
	pos = pos + strlen(p->email);
	memcpy(pos, "#", 1);
	pos += 1;
}

void unpack(const char *recordbuf, Person *p)
{	
	char tmp[RECORD_SIZE];

	char *ptr;

	strcpy(tmp, recordbuf);

	ptr = strtok(tmp, "#");
	strcpy(p->sn, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->name, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->age, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->addr, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->phone, ptr);
	ptr = strtok(NULL, "#");
	strcpy(p->email, ptr);

}

void insert(FILE *fp, const Person *p)
{
	char pagebuf[PAGE_SIZE] = {'\0', };
	char recordbuf[RECORD_SIZE] = {'\0', };
	
	int pagenum = 0;
	int page_amount = 1;
	int record_amount = 0;
	int deleted_page_head = -1;
	int deleted_record_head = -1;
	int tmp_deleted_page_head;
	int tmp_deleted_record_head;
	int can_insert_index;

	readPage(fp, pagebuf, pagenum);
	if(strlen(pagebuf) == 0){
		memset(pagebuf, (char)0xFF, PAGE_SIZE);
		memcpy(pagebuf, &page_amount, sizeof(int));
		memcpy(pagebuf+4, &record_amount, sizeof(int));
		memcpy(pagebuf+8, &deleted_page_head, sizeof(int));
		memcpy(pagebuf+12, &deleted_record_head, sizeof(int));
	}

	memcpy(&page_amount, pagebuf, sizeof(int));
	memcpy(&record_amount, pagebuf+4, sizeof(int));
	memcpy(&deleted_page_head, pagebuf+8, sizeof(int));
	memcpy(&deleted_record_head, pagebuf+12, sizeof(int));
	
	pack(recordbuf, p);

	if(deleted_page_head == -1){
		if((can_insert_index = record_amount%RECORD_PER_PAGE) == 0){
			memset(pagebuf, (char)0xFF, PAGE_SIZE);
			memcpy(pagebuf, recordbuf, RECORD_SIZE);
			writePage(fp, pagebuf, page_amount++);
			record_amount++;
		}
		else{
			readPage(fp, pagebuf, page_amount-1);
			memcpy(pagebuf+(can_insert_index*RECORD_SIZE), recordbuf, RECORD_SIZE);
			writePage(fp, pagebuf, page_amount-1);
			record_amount++;
		}
	}
	else{
		readPage(fp, pagebuf, deleted_page_head);
		memcpy(recordbuf, pagebuf + (deleted_record_head * RECORD_SIZE), RECORD_SIZE);
		memcpy(&tmp_deleted_page_head, recordbuf+1, sizeof(int));
		memcpy(&tmp_deleted_record_head, recordbuf+5, sizeof(int));

		pack(recordbuf, p);
		memcpy(pagebuf + (deleted_record_head * RECORD_SIZE), recordbuf, RECORD_SIZE);
		writePage(fp, pagebuf, deleted_page_head);

		deleted_page_head = tmp_deleted_page_head;
		deleted_record_head = tmp_deleted_record_head;

	}

	readPage(fp, pagebuf, 0);
	memcpy(pagebuf, &page_amount, sizeof(int));
	memcpy(pagebuf+4, &record_amount, sizeof(int));
	memcpy(pagebuf+8, &deleted_page_head, sizeof(int));
	memcpy(pagebuf+12, &deleted_record_head, sizeof(int));
	writePage(fp, pagebuf, 0);

}

void delete(FILE *fp, const char *sn)
{
	Person person_record;
	
	char pagebuf[PAGE_SIZE] = {'\0', };
	char recordbuf[RECORD_SIZE] = {'\0', };

	int page_amount;
	int record_amount;
	int deleted_page_head;
	int deleted_record_head;
	int find_key = 0;
	int i, j;

	readPage(fp, pagebuf, 0);
	
	if(pagebuf[0] == '\0'){
		fprintf(stderr, "file is empty\n");
		exit(1);
	}
	
	//헤드 읽기
	memcpy(&page_amount, pagebuf, sizeof(int));
	memcpy(&record_amount, pagebuf+4, sizeof(int));
	memcpy(&deleted_page_head, pagebuf+8, sizeof(int));
	memcpy(&deleted_record_head, pagebuf+12, sizeof(int));
	
	for(i = 1; i < page_amount; i++){
		readPage(fp, pagebuf, i);
		if(i == page_amount - 1){
			for(j = 0; j < RECORD_PER_PAGE; j++){
				memcpy(recordbuf, pagebuf + (j * RECORD_SIZE), RECORD_SIZE);
				if(recordbuf[0] == (char)0xFF){
					break;
				}
				else{
					unpack(recordbuf, &person_record);
					if(strcmp(person_record.sn, sn) == 0){
						find_key = 1;
						break;
					}
				}
			}
			if(find_key == 1){
				break;
			}
		}
		else{
			for(j = 0; j < RECORD_PER_PAGE; j++){
				memcpy(recordbuf, pagebuf + (j * RECORD_SIZE), RECORD_SIZE);
				if(recordbuf[0] == (char)0xFF){
					break;
				}
				else{
					unpack(recordbuf, &person_record);
					if(strcmp(person_record.sn, sn) == 0){
						find_key = 1;
						break;
					}
				}
			}
			if(find_key == 1){
				break;
			}
		}
	}

	if(find_key == 1){
		readPage(fp, pagebuf, i);
		memcpy(recordbuf, pagebuf + (j * RECORD_SIZE), RECORD_SIZE);
		memcpy(recordbuf, "*", 1);
		memcpy(recordbuf+1, &deleted_page_head, sizeof(int));
		memcpy(recordbuf+5, &deleted_record_head, sizeof(int));
		memcpy(pagebuf + (j * RECORD_SIZE), recordbuf, RECORD_SIZE);
		writePage(fp, pagebuf, i);

		readPage(fp, pagebuf, 0);
		memcpy(pagebuf+8, &i, sizeof(int));
		memcpy(pagebuf+12, &j, sizeof(int));
		writePage(fp, pagebuf, 0);
		
	}
	else{
		printf("sorry, I can't find it\n");
	}

}

int main(int argc, char *argv[])
{
	FILE *fp;  // 레코드 파일의 파일 포인터
	
	Person person_record;

	char recordbuf[RECORD_SIZE] = {'\0', };

	if ((fp = fopen(argv[2], "a")) < 0) {
		fprintf(stderr, "fopen error for %s\n", argv[2]);
	}

	if ((fp = fopen(argv[2], "r+")) < 0) {
		fprintf(stderr, "fopen error for %s\n", argv[2]);
	}

	if(argv[1][0] == 'i'){
		strcpy(person_record.sn, argv[3]);
		strcpy(person_record.name, argv[4]);
		strcpy(person_record.age, argv[5]);
		strcpy(person_record.addr, argv[6]);
		strcpy(person_record.phone, argv[7]);
		strcpy(person_record.email, argv[8]);

		insert(fp, &person_record);
	}
	else if(argv[1][0] == 'd'){
		delete(fp, argv[3]);
	}

	fclose(fp);
	return 1;
}
