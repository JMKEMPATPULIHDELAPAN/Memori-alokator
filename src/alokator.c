#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
/* run this program using the console pauser or add your own getch, system("pause") or input loop */
#define MAKS 16384

typedef struct blok{
	size_t ukuran;
	bool digunakan;
	
	struct blok* riwayat_sebelumnya;
	struct blok* riwayat_berikutnya;
	
	
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
Blok* terakhir = NULL;

void inisiasi(){
	mem 							= malloc(sizeof(Memori));
	mem->heap_mulai 				= malloc(MAKS);
	
	mem->daftar_heap 				= (Blok*)mem->heap_mulai;
	mem->daftar_heap->ukuran 		= 0;
	mem->daftar_heap->digunakan 	= false;
	mem->daftar_heap->berikutnya 	= NULL;
	mem->daftar_heap->riwayat_berikutnya 	= NULL;
	mem->daftar_heap->lanjutkan 	= NULL;
	mem->daftar_heap->sebelumnya 	= NULL;
	mem->daftar_heap->riwayat_sebelumnya = NULL;
		
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
			printf("> Riwayat belakang			: %p\n", pointer->riwayat_sebelumnya);
			printf("  Pointer belakang			: %p\n", pointer->sebelumnya);
			printf("> Riwayat depan				: %p\n", pointer->riwayat_berikutnya);
			printf("  Pointer depan				: %p\n\n", pointer->berikutnya);
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
	
	size_t i;
	for(i = 0; i < ukuran; i++){
		k[i] = n;
	}
	
	return (void*)k;
}

void* alokasi(size_t ukuran){
	ukuran = jajarkan(ukuran);
	
	size_t total_ukuran = ukuran + sizeof(Blok);
	
	//hitung ukuran
	if(MAKS - mem->total_ukuran <= total_ukuran) return NULL;
	
	Blok* scan = mem->daftar_heap;
	Blok* baru = NULL;
	Blok* kosong = NULL;
	
	while(scan != NULL){
		if(scan->digunakan == false && scan->ukuran == 0 && scan->berikutnya == NULL){
			baru = scan;
			baru->digunakan = true;
			baru->ukuran = ukuran;
			baru->lanjutkan = NULL;
			
			//Cek sebelumnya
			if(scan->sebelumnya != NULL){
				//Jika ditemukan ada blok sebelumnya
				baru->sebelumnya = scan->sebelumnya;
				baru->riwayat_sebelumnya = scan->sebelumnya;
				
				scan->sebelumnya->berikutnya = baru;
			} else {
				//Jika tidak, berarti itu blok awl
				baru->sebelumnya = NULL;
				baru->riwayat_sebelumnya = NULL;
			}
			
			//Pindai untuk pemisahan blok baru
			if((char*)baru + total_ukuran + sizeof(Blok) <= (char*)mem->heap_mulai + MAKS){
				//Jika masih luas 
				char* alamat_baru = (char*)baru + total_ukuran;
				kosong = (Blok*)alamat_baru;
				kosong->digunakan = false;
				kosong->ukuran = 0;
				kosong->lanjutkan = NULL;
				kosong->sebelumnya = baru;
				kosong->riwayat_sebelumnya = baru;
				kosong->berikutnya = NULL;
				kosong->riwayat_berikutnya = NULL;
				terakhir = kosong;
				
				baru->berikutnya = kosong;
				baru->riwayat_berikutnya = kosong;
			} else {
				//Tidak luas/sempit
				baru->berikutnya = NULL;
				baru->riwayat_berikutnya = NULL;
			}
			
			break;
		}
		scan = scan->berikutnya;
	}
	
	mem->total_ukuran = mem->total_ukuran + total_ukuran;;
	
	return baru->payload;
}

void* simulasi_bebaskan(void* ptr){
	Blok* pointer = (Blok*)((char*)ptr - sizeof(Blok));
	
	Blok* depan = pointer->riwayat_berikutnya;
	Blok* belakang = pointer->riwayat_sebelumnya;
	
	pointer->digunakan = false;
	//Ukuran memori TIDAK BOLEH DIUBAH!
	
	bool bebas_disebelah = false;
	
	//memeriksa sisi blok depan dan belakang
	
	//Memeriksa blok belakang
	bool belakang_terselesaikan = false; //bool ini gunanya ...? kekmana ya sulit aku jelasin
	if(belakang != NULL && !belakang->digunakan){
		
		if(depan != NULL && !depan->digunakan){ //Jika depan ada, tetapi depannya sudah tidak digunakan
			//bndingkan memori depan dengan angka 0
			if(depan->ukuran == 0){ //Jika setara, maka belakang yang dibebaskan juga menjadi Blok kosong!
				belakang->ukuran = 0;
			} else if(depan->ukuran >= 0){ //Jika lebih dari 0, berarti kedua blok tersebut (yang seddang dibebaskan, dan yang sebelumnya sudah dibebaskan atau si blok belakang) maka jumlahkan untuk dipakai
				belakang->ukuran = belakang->ukuran + pointer->ukuran + depan->ukuran;
			}
			
			//Menggunakan riwayat/history pointer
			if(depan->riwayat_berikutnya != NULL){ //jika ada blok kedepannya didepan pointer, maka kaitkan pointernya
				belakang->riwayat_berikutnya = depan->riwayat_berikutnya;
				belakang->berikutnya = depan->riwayat_berikutnya;
				
				depan->riwayat_berikutnya->sebelumnya = belakang;
				depan->riwayat_berikutnya->riwayat_sebelumnya = belakang;
			} else if(depan->riwayat_berikutnya == NULL){
				belakang->riwayat_berikutnya = NULL;
				belakang->berikutnya = NULL;
			}
			
			belakang_terselesaikan = true;
			
		} else if(depan != NULL && depan->digunakan){ //Jika depan ada, tetapi depannya digunakan
			belakang->ukuran = belakang->ukuran + pointer->ukuran;
			belakang->berikutnya = depan;
			belakang->riwayat_berikutnya = depan;
		} else if(depan == NULL){ //jika blok depan tidak ada, maka blok tersebut yang dibebaskan adalah BLOK KOSONG
			belakang->ukuran = 0;
			belakang->berikutnya = NULL;
			belakang->riwayat_berikutnya = NULL;
		}
	}
	
	if(depan != NULL && !depan->digunakan && !belakang_terselesaikan){
		if(depan->ukuran == 0 && depan->riwayat_berikutnya == NULL){
			pointer->ukuran = 0;
			pointer->riwayat_berikutnya = NULL;
			pointer->berikutnya = NULL;
			depan->riwayat_sebelumnya = pointer;
		} else if(depan->ukuran >= 0 && depan->riwayat_berikutnya != NULL){
			pointer->ukuran = pointer->ukuran + depan->ukuran;
			pointer->riwayat_berikutnya = depan->riwayat_berikutnya;
			pointer->berikutnya = depan->riwayat_berikutnya;
			depan->riwayat_sebelumnya = pointer;
		}
	}
	
	return mem->daftar_heap;
}
