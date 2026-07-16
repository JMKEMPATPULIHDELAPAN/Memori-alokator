#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
/* run this program using the console pauser or add your own getch, system("pause") or input loop */
#define MAKS 8192

typedef struct blok{
	size_t ukuran;
	bool digunakan;
	
	struct history {
		struct blok* sebelumnya;
		struct blok* berikutnya;
	};
	
	struct blok* berikutnya;
	struct blok* lanjutkan;
	struct blok* sebelumnya;
	char payload[];
} Blok;

typedef struct {
	char* heap_mulai;
	Blok* daftar_heap;
	size_t total_ukuran;
	int total_dialokasi;
} Memori;

static Memori* mem = NULL;

void inisiasi(){
	mem 							= malloc(sizeof(Memori));
	mem->heap_mulai 				= malloc(MAKS);
	
	mem->daftar_heap 				= (Blok*)mem->heap_mulai;
	mem->daftar_heap->ukuran 		= 0;
	mem->daftar_heap->digunakan 	= false;
	mem->daftar_heap->berikutnya 	= NULL;
	mem->daftar_heap->lanjutkan 	= NULL;
	mem->daftar_heap->sebelumnya 	= NULL;
		
	mem->total_dialokasi = 0;
	mem->total_ukuran = 0;
}
size_t jajarkan(size_t ukuran){
	return  (ukuran + 8 -1) & ~(8 - 1);
}
bool temukan_blok(void* ptr){
	Blok* pointer 	= 	(Blok*)((char*)ptr - sizeof(Blok));
	
	Blok* temukan 	= 	mem->daftar_heap;
	bool ditemukan 	= false;
	while(temukan != NULL){
		if(temukan == pointer){
			ditemukan = true;
			printf("Ditemukan\n");
			printf("Pointer berasal				: %p\n", pointer);
			printf("Pointer berukuran			: %zu\n", pointer->ukuran);
			printf("Pointer digunakan			: %s\n", pointer->digunakan ? "ya" : "tidak");
			printf("Pointer belakang			: %p\n", pointer->sebelumnya);
			printf("Pointer depan				: %p\n\n", pointer->berikutnya);
			break;
		}
		temukan = temukan->berikutnya;
	}
	
	if(ditemukan == false){
		printf("Pointer tidak ditemukan\n");
	}
	
	return ditemukan;
}
void* aturMemori(int nilai, void* kepada, size_t ukuran){
	unsigned char n = (unsigned char)nilai;
	unsigned char* k = (unsigned char*)kepada;
	
	for(size_t i = 0; i < ukuran; i++){
		k[i] = n;
	}
	
	return (void*)k;
}

void* alokasi(size_t ukuran){
	ukuran = jajarkan(ukuran);
	
	Blok* awal = mem->daftar_heap;
	Blok* scan = mem->daftar_heap;
	Blok* baru = NULL;
	
	if(awal->digunakan == false && awal->berikutnya == NULL){
		baru = awal;
		baru->digunakan = true;
		baru->ukuran = ukuran;
		baru->berikutnya = NULL;
		baru->lanjutkan = NULL;
		baru->sebelumnya = NULL;
	} else if (awal->digunakan == true){
		
		while(scan != NULL){
			if(scan->berikutnya == NULL && scan->digunakan == true){
				char* alamatBaru = (char*)scan + scan->ukuran + sizeof(Blok);
				baru = (Blok*)alamatBaru;
				baru->digunakan = true;
				baru->ukuran = ukuran;
				baru->sebelumnya = scan;
				baru->lanjutkan = NULL;
				baru->berikutnya = NULL;
				
				scan->berikutnya = baru;
				break;
			}
			
			scan = scan->berikutnya;
		}
	}
	
	mem->total_ukuran = mem->total_ukuran + ukuran + sizeof(Blok);
	
	return baru->payload;
}

void* bebaskan(void* ptr){
	Blok* pointer = (Blok*)((char*)ptr - sizeof(Blok));
	
	Blok* scan = mem->daftar_heap;
	bool ditemukan = false;
	while(scan != NULL){
		if(scan == pointer){
			ditemukan = true;
			break;
		}
		scan = scan->berikutnya;
	}
	
	if(!ditemukan) return NULL;
	
	Blok* belakang = pointer->sebelumnya;
	Blok* depan = pointer->berikutnya;
	
	if(belakang == NULL){
		if(depan != NULL){ //ada blok berikutnya terisi
			depan->sebelumnya = NULL;
			mem->daftar_heap = depan;
		} else { //Tidak punya blok berikutnya
			mem->daftar_heap->digunakan 	= false;
			mem->daftar_heap->berikutnya 	= NULL;
			mem->daftar_heap->sebelumnya 	= NULL;
			mem->daftar_heap->lanjutkan 	= NULL;
			
		}
	} else if (depan == NULL) {
		belakang->berikutnya = NULL;
	} else {
		depan->sebelumnya = belakang;
		belakang->berikutnya = depan;
	}
	
	pointer->digunakan = false;
	pointer->lanjutkan = NULL;
	
	aturMemori(0, pointer->payload, pointer->ukuran);
	
	//isolasi kebelakanga
	Blok* maju_kebelakang = mem->daftar_heap;
	while(maju_kebelakang != NULL){
		if(maju_kebelakang->berikutnya == NULL){
			Blok* kebelakang = maju_kebelakang;
			
			pointer->sebelumnya = kebelakang;
			pointer->berikutnya = NULL;
			kebelakang->berikutnya = pointer;
			
			
			break;
		}
		maju_kebelakang = maju_kebelakang->berikutnya;
	} 
		
	return (void*)mem->daftar_heap;
}

