#include "../include/utils.h"  // Yardımcı fonksiyonların tanımlı olduğu header dosyası
int init_semaphore(int sem_id, int sem_num, int value) {
    union semun arg;            // semctl() fonksiyonuna değer göndermek için semun union'ı kullanılır
    arg.val = value;            // Semaphore'a atanacak ilk değeri belirliyoruz (örneğin 1)
    return semctl(sem_id, sem_num, SETVAL, arg);  
    // semctl(): Semaphore kontrol fonksiyonu
    // sem_id: Semaphore grubunun ID’si
    // sem_num: Hangi semaphore? (0, 1, ...)
    // SETVAL: Değer atama komutu
    // arg: İçinde .val değeri olan union semun
}
// kilitliyor
int sem_p(int sem_id, int sem_num) {
    struct sembuf sem_op;       // Semaphore işlemi için sembuf yapısı kullanılır
    sem_op.sem_num = sem_num;   // Hangi semaphore üzerinde işlem yapılacak?
    sem_op.sem_op = -1;         // -1 → Semaphore değerini 1 azalt = KİLİTLE
    sem_op.sem_flg = 0;         // 0 → Engellenirse bekle (IPC_NOWAIT olsaydı hemen çıkardı)
    return semop(sem_id, &sem_op, 1);  
    // semop(): Semaphore işlemini uygular
    // 1: Kaç tane işlem yapılacak (1 tane = tek semaphore)
}

// kilit açma işlemi 
int sem_v(int sem_id, int sem_num) {
    struct sembuf sem_op;
    sem_op.sem_num = sem_num;   // Hangi semaphore?
    sem_op.sem_op = 1;          // +1 → Semaphore değerini 1 artır = KİLİDİ AÇ
    sem_op.sem_flg = 0;
    return semop(sem_id, &sem_op, 1);  // Yine sadece 1 işlem yapılıyor
}
