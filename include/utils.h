#ifndef UTILS_H           // Eğer UTILS_H tanımlı değilse, aşağıdakileri ekle
#define UTILS_H           // UTILS_H tanımlanmış olarak işaretle (çift include önleme)

#include <stdio.h>        // Giriş-çıkış işlemleri için (printf, perror vb.)
#include <stdlib.h>       // Genel işlemler (exit, malloc, free vb.)
#include <string.h>       // String işlemleri (strcpy, strcmp, strlen vb.)
#include <unistd.h>       // POSIX sistem fonksiyonları (fork, getpid, sleep vb.)
#include <sys/types.h>    // Temel veri tipleri (pid_t, key_t gibi)
#include <sys/wait.h>     // wait() ve waitpid() gibi fonksiyonlar
#include <sys/ipc.h>      // IPC (Interprocess Communication) için anahtar işlemleri
#include <sys/shm.h>      // Shared memory (paylaşımlı bellek) işlemleri
#include <sys/sem.h>      // Semaphore işlemleri
#include <errno.h>        // Hata numaralarına erişmek için (errno)


// ⛓️ Semaphore işlemlerinde kullanılan union semun yapısı
// Bu union, semctl() fonksiyonuna değer geçirmek için kullanılır.
// Glibc'de tanımlı olmadığından, kullanıcı tarafından tanımlanmalıdır.
union semun {
    int val;                    // Semaphore’a atanacak başlangıç değeri
    struct semid_ds *buf;       // Sistem bilgisi (genellikle kullanılmaz)
    unsigned short *array;      // Birden fazla semaphore için başlangıç değerleri
};


// 🔧 Semaphore başlatma fonksiyonu
// sem_id: Semaphore setinin ID’si
// sem_num: Hangi semaphore (0, 1, 2 ...)
// value: Başlangıç değeri (örneğin 1 ise kilit açık)
// Dönüş: Başarılıysa 0, aksi halde -1
int init_semaphore(int sem_id, int sem_num, int value);


// 🔒 P Operasyonu (wait / lock)
// Semaphore değerini 1 azaltır
// Eğer değer zaten 0 ise process beklemeye alınır
// Kritik bölgeye girişte kullanılır
int sem_p(int sem_id, int sem_num);


// 🔓 V Operasyonu (signal / unlock)
// Semaphore değerini 1 arttırır
// Bekleyen process varsa uyandırılır
// Kritik bölgeden çıkışta kullanılır
int sem_v(int sem_id, int sem_num);


#endif  // UTILS_H
