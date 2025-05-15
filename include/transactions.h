#ifndef TRANSACTIONS_H          // Eğer TRANSACTIONS_H tanımlı değilse
#define TRANSACTIONS_H          // TRANSACTIONS_H'yi tanımla (header guard)

// Hesap ve işlem log tanımları bu dosyada kullanılacağı için accounts.h dosyası dahil ediliyor
#include "accounts.h"


// 💳 İşlem türleri (Transaction Types) için sabitler tanımlanıyor
#define DEPOSIT 0              // Para yatırma işlemi
#define WITHDRAW 1             // Para çekme işlemi
#define TRANSFER 2             // Para transferi işlemi

// ✔️ İşlem sonucu sabitleri
#define SUCCESS 0              // İşlem başarılı
#define FAILURE -1             // İşlem başarısız (örneğin yetersiz bakiye)


/*
 *  Para yatırma işlemini gerçekleştiren fonksiyonun bildirimi
 * accounts: Paylaşımlı bellek üzerindeki hesap dizisi
 * logs: İşlem geçmişini tutan shared memory dizisi
 * account_id: Hangi hesaba yatırılacak?
 * amount: Ne kadar yatırılacak?
 * transaction_id: Bu işlemin ID'si
 * sem_id: Semaphore set ID’si (kilitleme için)
 */
int process_deposit(Account *accounts, TransactionLog *logs, int account_id, int amount, int transaction_id, int sem_id);


/*
 * 💸 Para çekme işlemini gerçekleştiren fonksiyonun bildirimi
 * Aynı parametreler kullanılır ama bu sefer hesaptan para düşer
 * Bakiye yeterli değilse FAILURE döner
 */
int process_withdraw(Account *accounts, TransactionLog *logs, int account_id, int amount, int transaction_id, int sem_id);


/*
 * 🔁 Transfer işlemini gerçekleştiren fonksiyonun bildirimi
 * from_account → to_account hesabına belirtilen miktarı aktarır
 * Hem iki hesabı kilitler, hem de log kaydı oluşturur
 * Deadlock önlemek için küçük account ID'yi önce kilitler
 */
int process_transfer(Account *accounts, TransactionLog *logs, int from_account, int to_account, int amount, int transaction_id, int sem_id);


/*
 * 📄 İşlem dosyasını okuyan fonksiyon
 * filename: Okunacak dosya adı (örneğin transactions.txt)
 * Okunan işlemler bellekte 4 ayrı diziye atanır:
 * t_type[]        → işlem türü
 * from_acc[]      → gönderici hesap
 * to_acc[]        → alıcı hesap (deposit/withdraw’da -1 olabilir)
 * amount[]        → miktar
 */
int read_transactions(const char *filename, int **t_type, int **from_acc, int **to_acc, int **amount);


/*
 * Başlangıçta hesapları dosyadan okuyan ve accounts dizisini dolduran fonksiyon
 * filename: Hesap bilgilerinin bulunduğu dosya (örneğin accounts.txt)
 * accounts: Shared memory’deki hesap dizisi
 * max_accounts: Maksimum kaç hesap destekleniyor?
 */
int initialize_accounts(const char *filename, Account *accounts, int max_accounts);


#endif  // TRANSACTIONS_H
