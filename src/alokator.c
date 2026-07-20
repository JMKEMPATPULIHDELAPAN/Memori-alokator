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
	
	struct blok* lanjutkan;
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
	mem->daftar_heap->riwayat_berikutnya 	= NULL;
	mem->daftar_heap->lanjutkan 	= NULL;
	mem->daftar_heap->riwayat_sebelumnya = NULL;
	
	terakhir = mem->daftar_heap;
	
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
			printf("> Riwayat depan				: %p\n", pointer->riwayat_berikutnya);
			break;
		}
		temukan = temukan->riwayat_berikutnya;
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
	Blok* baru = NULL;
	Blok* kosong = NULL;
	
	//hitung ukuran, dan cek kondisi jika memenuhi; 
	if(MAKS - mem->total_ukuran <= total_ukuran) return NULL;
	
	if(terakhir->ukuran == 0 && terakhir->digunakan == false && terakhir->riwayat_berikutnya == NULL){ //Jika blok terakhir menemukan blok ujung, maka bisa diisi
		baru = terakhir;
		baru->digunakan = true;
		baru->ukuran = ukuran;
		baru->lanjutkan = NULL;
		
		if((char*)baru + baru->ukuran + (sizeof(Blok) * 2) <= mem->heap_mulai + MAKS){ //Jika pointer memori masih luas, maka tambahkan blok kosong 
			kosong = (Blok*)((char*)baru + baru->ukuran + sizeof(Blok));
			kosong->digunakan = false;
			kosong->ukuran = 0;
			kosong->riwayat_sebelumnya = baru;
			kosong->lanjutkan = NULL;
			kosong->riwayat_berikutnya = NULL;
		} else {
			kosong = NULL;
		}
		
		baru->riwayat_berikutnya = kosong;
		terakhir = kosong;
		
		mem->total_ukuran = mem->total_ukuran + total_ukuran;
	} else if(terakhir == NULL){
		//Jika ujung blok ATAU Pointer akhir sudah diujung NULL, maka melakukan pencarian Blok dibebaskan
		
		Blok* scan = mem->daftar_heap;
		while(scan != NULL){
			if(!scan->digunakan && scan->ukuran >= ukuran){
				// Simpan data blok kosong sebelum dipotong
				Blok* simpan_pointer_depan = scan->riwayat_berikutnya;
				size_t simpan_ukuran = scan->ukuran;
				
				// Ambil blok ini sebagai blok baru yang dipakai
				baru = scan;
				baru->digunakan = true;
				baru->ukuran = ukuran;
				baru->riwayat_sebelumnya = scan->riwayat_sebelumnya;
				
				// Potong (split) sisa blok kosong jika masih cukup besar untuk header
				if((char*)baru + baru->ukuran + (2 * sizeof(Blok)) <= (char*)simpan_pointer_depan){
					kosong = (Blok*)((char*)baru + baru->ukuran + sizeof(Blok));
					kosong->ukuran = simpan_ukuran - ukuran;
					kosong->digunakan = false;
					kosong->riwayat_berikutnya = simpan_pointer_depan;
					kosong->riwayat_sebelumnya = baru;
					
					baru->riwayat_berikutnya = kosong;
				} else {
					// Sisa memori terlalu kecil, tidak perlu buat blok kosong baru
					baru->riwayat_berikutnya = simpan_pointer_depan;
					if(simpan_pointer_depan != NULL){
						simpan_pointer_depan->riwayat_sebelumnya = baru;
					}
				}
				
				break;
			}
			scan = scan->riwayat_berikutnya;
		}
	
	}
	
	//Validasi jika dua pointer gagal (tetap NULL) karena pointer secara FISIK MENYEMPIT meski luas
	if(baru == NULL) return NULL;
	
	return baru->payload;
}

void* simulasi_bebaskan(void* ptr){
	Blok* pointer = (Blok*)((char*)ptr - sizeof(Blok));
	
	Blok* depan = pointer->riwayat_berikutnya;
	Blok* belakang = pointer->riwayat_sebelumnya;
	
	//Periksa blok sudah dibebaskan, agar tidak double free membuat overflow dan kesalahan saat aturMemori() mengenai fisik lain
	if(!pointer->digunakan) return NULL;
	
	pointer->digunakan = false;
	
	bool bebas_disebelah = false;
	
	uint64_t total_selisih = 0; //ini tidak memakai size_t karena perhitungan tersebut pastinya akan ada minus
	uint64_t selisih_depan = 0;
	uint64_t selisih_tengah = 0;
	uint64_t selisih_belakang  = 0;
	
	//memeriksa sisi blok depan dan belakang
	
	//Memeriksa blok belakang
	bool belakang_terselesaikan = false; //bool ini gunanya sebagai indikator, agar penggabungan blok depan tidak terjadi dua kali lagi karena belakang sudah melakukannya
	
	if(belakang != NULL && !belakang->digunakan){ //Jika blok belakang terdeteksi tidak digunakan => !belakang->digunakan
		selisih_tengah = pointer->ukuran + sizeof(Blok);
		selisih_belakang = belakang->ukuran + sizeof(Blok);
		
		if(depan != NULL && !depan->digunakan) { //Jika blok depan ADA DAN TERDETEJSI TIDAK DIGUNAKAN => !depan->digunakan
		//maka penggabungan 3 blok akan terjadi
			selisih_depan = depan->ukuran + sizeof(Blok);
			if(depan->ukuran == 0 && depan->riwayat_berikutnya == NULL){ //Jika blok depan berukuran 0, dan tidak memiliki blok kedeopannya, maka diasumsikan sebagai Blok kosong
				belakang->ukuran = 0;
				belakang->riwayat_berikutnya = NULL;
				terakhir = belakang;
			} else if(depan->ukuran >= 0 && depan->riwayat_berikutnya != NULL){ //Jika blok depan lebih dari 0, dan memiliki blok kedepannya, maka bisa dikaitkan
				selisih_depan = depan->ukuran + sizeof(Blok);
				
				belakang->ukuran = belakang->ukuran + pointer->ukuran + depan->ukuran;
				belakang->riwayat_berikutnya = depan->riwayat_berikutnya;
				
				depan->riwayat_berikutnya->riwayat_sebelumnya = belakang;
				depan->ukuran = 0;
				depan->riwayat_berikutnya = NULL;
				depan->riwayat_sebelumnya = NULL;
				
			} else if(depan->ukuran >= 0 && depan->riwayat_berikutnya == NULL){ //Jika blok depan lebih dari 0, dan TIDAK memiliki blok kedepannya, maka belakang sudah dianggap sebagai blok kosong
				selisih_depan = depan->ukuran + sizeof(Blok);
				
				belakang->ukuran = 0;
				belakang->riwayat_berikutnya = NULL;
				terakhir = belakang;
				
				depan->ukuran = 0;
				depan->riwayat_berikutnya = NULL;
				depan->riwayat_sebelumnya = NULL;
			}
			
			belakang_terselesaikan = true;
		} else if(depan != NULL && depan->digunakan == true){  //Jika blok depan ADA DAN TERDETEKSI SEDANG DIGUNAKANpl
						
			belakang->ukuran = belakang->ukuran + pointer->ukuran;
			belakang->riwayat_berikutnya = depan;
			
			depan->riwayat_sebelumnya = belakang;
		} else if(depan == NULL){  //Jika blok depan TIDAK DITEMUKAN (ALIAS NULL)
						
			belakang->ukuran = belakang->ukuran + pointer->ukuran;
			belakang->riwayat_berikutnya = NULL;
			
			terakhir = belakang;
		}
		
		pointer->ukuran = 0;
		pointer->riwayat_berikutnya = NULL;
		pointer->riwayat_sebelumnya = NULL;
		pointer->lanjutkan = NULL;
		
		total_selisih = selisih_depan + selisih_tengah + selisih_belakang;
		mem->total_ukuran = mem->total_ukuran - total_selisih;
	}
	
	//periksa blok depan jika indikator belakang_terselesaikan belum menyala
	if(depan != NULL && !depan->digunakan && !belakang_terselesaikan){ //Jika blok depan ADA DAN TERDETEJSI TIDAK DIGUNAKAN => !depan->digunakan
		selisih_tengah = pointer->ukuran + sizeof(Blok);
		selisih_depan = depan->ukuran + sizeof(Blok);
				
		if(depan->ukuran == 0 && depan->riwayat_berikutnya == NULL){ //Jika blok depan berukuran 0, dan tidak memiliki blok kedeopannya, maka diasumsikan sebagai Blok kosong
			pointer->ukuran = 0;
			pointer->riwayat_berikutnya = NULL;
			terakhir = pointer;
		} else if(depan->ukuran >= 0 && depan->riwayat_berikutnya != NULL){ //Jika blok depan berukuran lebih dari 0, dan memiliki blok kedepannya maka bisa digabung!
			pointer->ukuran = pointer->ukuran + depan->ukuran;
			pointer->riwayat_berikutnya = depan->riwayat_berikutnya;
			
			depan->riwayat_berikutnya->riwayat_sebelumnya = pointer;
			
			depan->ukuran = 0;
			depan->riwayat_berikutnya = NULL;
			depan->riwayat_sebelumnya = NULL;
		} else if(depan->ukuran >= 0 && depan->riwayat_berikutnya == NULL){ //Jika blok depan berukuran lebih dari 0, dan TIDAK MEMILIKI blok kedepannya, maka blok pointer dianggap kosong
			pointer->ukuran = 0;
			pointer->riwayat_berikutnya = NULL;
			terakhir = pointer;
			
			depan->ukuran = 0;
			depan->riwayat_berikutnya = NULL;
			depan->riwayat_sebelumnya = NULL;
		}
		
		pointer->ukuran = 0;
		pointer->riwayat_berikutnya = NULL;
		pointer->riwayat_sebelumnya = NULL;
		pointer->lanjutkan = NULL;
		
		total_selisih = selisih_tengah + selisih_depan ;
		mem->total_ukuran = mem->total_ukuran - total_selisih;	
	}
	
	return mem->daftar_heap;
}
