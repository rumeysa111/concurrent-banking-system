#include "../include/transactions.h"  // İşlem fonksiyonları için tanımlar
#include "../include/utils.h"         // Semaphore fonksiyonları
#include <stdio.h>                    // Dosya işlemleri
#include <stdlib.h>                   // Bellek ayırma
#include <string.h>                   // String kopyalama vs.
// Deadlock oluşmaması için her zaman küçük ID'li hesabı önce kilitler
void lock_accounts(int sem_id, int account1, int account2) {
    if (account1 < account2) {
        sem_p(sem_id, account1); // önce küçük olanı
        sem_p(sem_id, account2);
    } else {
        sem_p(sem_id, account2);
        sem_p(sem_id, account1);
    }
}
void unlock_accounts(int sem_id, int account1, int account2) {
    sem_v(sem_id, account1); // önce biri sonra diğeri
    sem_v(sem_id, account2);
}
int process_deposit(Account *accounts, TransactionLog *logs, int account_id, int amount, int transaction_id, int sem_id) {
    sem_p(sem_id, account_id);  // Hesabı kilitle

    accounts[account_id].balance += amount;  // Parayı ekle

    // Log kaydı
    logs[transaction_id].transaction_id = transaction_id;
    strcpy(logs[transaction_id].type, "Deposit");
    logs[transaction_id].from_account = -1;           // kaynak hesap yok çünkü para sistem dışından geliyor 
    logs[transaction_id].to_account = account_id; // hedef hesap
    logs[transaction_id].amount = amount;
    strcpy(logs[transaction_id].status, "Success");

    sem_v(sem_id, account_id);  // Hesabı aç kilit açılıyor 
    return SUCCESS;
}
int process_withdraw(Account *accounts, TransactionLog *logs, int account_id, int amount, int transaction_id, int sem_id) {
    sem_p(sem_id, account_id);  // Hesabı kilitle

    if (accounts[account_id].balance < amount) {  // Bakiye yeterli mi?
        // Başarısız log kaydı
        logs[transaction_id].transaction_id = transaction_id;
        strcpy(logs[transaction_id].type, "Withdraw");
        logs[transaction_id].from_account = account_id;
        logs[transaction_id].to_account = -1;// hedef hesap yok çünkü para sistem dışına gidiyor
        logs[transaction_id].amount = amount;
        strcpy(logs[transaction_id].status, "Failed");

        sem_v(sem_id, account_id);  // Kilidi bırak
        return FAILURE;
    }

    // Bakiye düşürülür
    accounts[account_id].balance -= amount;

    // Başarılı log kaydı
    logs[transaction_id].transaction_id = transaction_id;
    strcpy(logs[transaction_id].type, "Withdraw");
    logs[transaction_id].from_account = account_id; 
    logs[transaction_id].to_account = -1;
    logs[transaction_id].amount = amount;
    strcpy(logs[transaction_id].status, "Success");

    sem_v(sem_id, account_id);  // Kilidi bırak
    return SUCCESS;
}
int process_transfer(Account *accounts, TransactionLog *logs, int from_account, int to_account, int amount, int transaction_id, int sem_id) {
    lock_accounts(sem_id, from_account, to_account);  // İki hesabı da güvenli şekilde kilitle

    if (accounts[from_account].balance < amount) {  // Yeterli para yoksa
        // Başarısız log
        logs[transaction_id].transaction_id = transaction_id;
        strcpy(logs[transaction_id].type, "Transfer");
        logs[transaction_id].from_account = from_account;
        logs[transaction_id].to_account = to_account;
        logs[transaction_id].amount = amount;
        strcpy(logs[transaction_id].status, "Failed");

        unlock_accounts(sem_id, from_account, to_account);
        return FAILURE;
    }

    // Transfer işlemi
    accounts[from_account].balance -= amount;
    accounts[to_account].balance += amount;

    // Log
    logs[transaction_id].transaction_id = transaction_id;
    strcpy(logs[transaction_id].type, "Transfer");
    logs[transaction_id].from_account = from_account;
    logs[transaction_id].to_account = to_account;
    logs[transaction_id].amount = amount;
    strcpy(logs[transaction_id].status, "Success");

    unlock_accounts(sem_id, from_account, to_account);
    return SUCCESS;
}
int read_transactions(const char *filename, int **t_type, int **from_acc, int **to_acc, int **amount) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening transactions file");
        return -1;
    }

    int count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        count++;  // Kaç işlem olduğunu say
    }

    // Bellek ayır
    *t_type = (int *)malloc(count * sizeof(int));
    *from_acc = (int *)malloc(count * sizeof(int));
    *to_acc = (int *)malloc(count * sizeof(int));
    *amount = (int *)malloc(count * sizeof(int));

    rewind(file);  // Dosyanın başına dön

    for (int i = 0; i < count; i++) {
        if (fgets(line, sizeof(line), file) == NULL) break;

        // <T_type, From_account, To_account, Amount>
        sscanf(line, "%d, %d, %d, %d", &(*t_type)[i], &(*from_acc)[i], &(*to_acc)[i], &(*amount)[i]);
    }

    fclose(file);
    return count;  // Toplam işlem sayısı
}
int initialize_accounts(const char *filename, Account *accounts, int max_accounts) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening accounts file");
        return -1;
    }

    int account_count = 0;
    char line[256];

    // İlk satır: toplam hesap sayısı
    if (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%d", &account_count);

        // Eğer dosyadaki hesap sayısı sınırı aşarsa kes
        if (account_count > max_accounts) {
            printf("Warning: Found %d accounts, but only space for %d. Truncating.\n",
                   account_count, max_accounts);
            account_count = max_accounts;
        }
    }

    // Hesapları oku
    for (int i = 0; i < account_count && i < max_accounts; i++) {
        if (fgets(line, sizeof(line), file) != NULL) {
            int id, balance;
            sscanf(line, "%d, %d", &id, &balance);
            accounts[i].account_id = id;
            accounts[i].balance = balance;
        }
    }

    fclose(file);
    return account_count;
}
