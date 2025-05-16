#include "../include/accounts.h"
#include "../include/transactions.h"
#include "../include/utils.h"

#define MAX_ACCOUNTS 100                 // Maksimum hesap sayisi
#define MAX_TRANSACTIONS 1000            // Maksimum islem sayisi
#define ACCOUNTS_FILE "accounts.txt"      // Hesap bilgisi dosyasi
#define TRANSACTIONS_FILE "transactions.txt"  // Islem bilgisi dosyasi

int main() {
    // IPC key'leri olusturuluyor
    // ftok benzersiz anahtar oluşturur (farklı programlar arasında çakışmaması için )
    key_t shm_key = ftok(".", 'S');   // Shared memory key
    key_t sem_key = ftok(".", 'M');   // Semaphore key

    // Hesaplar icin shared memory yarat
    int accounts_shm_id = shmget(shm_key, MAX_ACCOUNTS * sizeof(Account), IPC_CREAT | 0666);
    if (accounts_shm_id == -1) {
        perror("shmget failed for accounts");
        exit(EXIT_FAILURE);
    }

    // Shared memory'ye baglan (hesaplar)
    Account *accounts = (Account *)shmat(accounts_shm_id, NULL, 0);
    if (accounts == (void *)-1) {
        perror("shmat failed for accounts");
        exit(EXIT_FAILURE);
    }
    // shmget ortak bellek oluşturur
    // shmat ortak belleği bağlar
    // shmdt ortak belleği ayırır
    // shmctl bellek silme işlemleri için kullanılır
    
    // Transaction log'lar icin shared memory yarat
    int logs_shm_id = shmget(shm_key + 1, MAX_TRANSACTIONS * sizeof(TransactionLog), IPC_CREAT | 0666);
    if (logs_shm_id == -1) {
        perror("shmget failed for transaction logs");
        exit(EXIT_FAILURE);
    }

    // Shared memory'ye baglan (loglar)
    TransactionLog *logs = (TransactionLog *)shmat(logs_shm_id, NULL, 0);
    if (logs == (void *)-1) {
        perror("shmat failed for transaction logs");
        exit(EXIT_FAILURE);
    }

    // Hesaplari dosyadan oku veya varsayilan degerlerle baslat
    // hesap bilgilerini accounts.txt dosyasından okumaya çalışır 
    // eğer dosya yoksa veya boşsa varsayılan hesapları oluşturur
    int num_accounts = initialize_accounts(ACCOUNTS_FILE, accounts, MAX_ACCOUNTS);
    if (num_accounts <= 0) {
        printf("Creating default accounts since no file was found or file was empty\n");
        num_accounts = 5;
        for (int i = 0; i < num_accounts; i++) {
            accounts[i].account_id = i;
            accounts[i].balance = 500;  // Varsayilan bakiye
        }
    }

    // Her hesap icin semaphore seti olustur
    int sem_id = semget(sem_key, num_accounts, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(EXIT_FAILURE);
    }

    // Her semaphore'u 1 olarak baslat
    for (int i = 0; i < num_accounts; i++) {
        if (init_semaphore(sem_id, i, 1) == -1) {
            perror("Failed to initialize semaphore");
            exit(EXIT_FAILURE);
        }
    }

    // Transaction dosyasini oku
    int *t_type = NULL, *from_acc = NULL, *to_acc = NULL, *amount = NULL;
    int num_transactions = read_transactions(TRANSACTIONS_FILE, &t_type, &from_acc, &to_acc, &amount);

    if (num_transactions <= 0) {
        printf("No transactions found in file. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    // Başarısız işlemleri takip etmek için değişkenler
    int failed_transactions[MAX_TRANSACTIONS]; // başarısız işlemlerin numaralarını saklamak için
    int num_failed = 0; // başarısız işlem sayısı
    pid_t transaction_pids[MAX_TRANSACTIONS]; // Tüm işlemlerin PID'leri
    int transaction_results[MAX_TRANSACTIONS]; // İşlemlerin sonuçları
    int completed_transactions = 0; // Tamamlanan işlem sayısı

    // Her transaction için child process yarat
    for (int i = 0; i < num_transactions; i++) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork failed");
            exit(EXIT_FAILURE);
        } 
        else if (pid == 0) {  // Child process
            int result;
            switch (t_type[i]) {
                case DEPOSIT:
                    result = process_deposit(accounts, logs, to_acc[i], amount[i], i, sem_id);
                    break;
                case WITHDRAW:
                    result = process_withdraw(accounts, logs, from_acc[i], amount[i], i, sem_id);
                    break;
                case TRANSFER:
                    result = process_transfer(accounts, logs, from_acc[i], to_acc[i], amount[i], i, sem_id);
                    break;
            }
            exit(result);  // Çıkış kodu: 0 (başarı) veya -1 (hata)
        }
        else {
            transaction_pids[i] = pid; // PID'yi sakla
        }
    }

    // İşlemleri bekle ve başarısız olanları kaydet
    while (completed_transactions < num_transactions) {
        int status;
        pid_t finished_pid = wait(&status);
        
        // Hangi işlem tamamlandı?
        for (int i = 0; i < num_transactions; i++) {
            if (transaction_pids[i] == finished_pid) {
                // İşlem sonucunu kaydet
                if (WIFEXITED(status)) {
                    transaction_results[i] = WEXITSTATUS(status);
                    
                    // Başarısız işlemleri kaydet
                    if (transaction_results[i] != SUCCESS) {
                        failed_transactions[num_failed++] = i;
                        printf("Debug: Transaction %d failed with exit code %d\n", i, transaction_results[i]);  // Hata ayıklama çıktısı

                    }
                }
                completed_transactions++;
                break;
            }
        }
    }

    // Transaction log yazdır (ilk çalıştırma)
    printf("\nTransaction Log:\n");
    for (int i = 0; i < num_transactions; i++) {
        if (strcmp(logs[i].type, "Deposit") == 0) {
            printf("Transaction %d: %s %d to Account %d (%s)\n", 
                   logs[i].transaction_id, logs[i].type, 
                   logs[i].amount, logs[i].to_account, logs[i].status);
        } else if (strcmp(logs[i].type, "Withdraw") == 0) {
            printf("Transaction %d: %s %d from Account %d (%s)\n", 
                   logs[i].transaction_id, logs[i].type, 
                   logs[i].amount, logs[i].from_account, logs[i].status);
        } else if (strcmp(logs[i].type, "Transfer") == 0) {
            printf("Transaction %d: %s %d from Account %d to Account %d (%s)\n", 
                   logs[i].transaction_id, logs[i].type, 
                   logs[i].amount, logs[i].from_account, 
                   logs[i].to_account, logs[i].status);
        }
    }

   // Başarısız işlemleri tekrar dene
if (num_failed > 0) {
    printf("\nRetrying %d failed transactions...\n", num_failed);
}

for (int j = 0; j < num_failed; j++) {
    int i = failed_transactions[j];
    
    printf("Transaction %d failed. Retrying once...\n", i);
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        int result;
        switch (t_type[i]) {
            case DEPOSIT:
                result = process_deposit(accounts, logs, to_acc[i], amount[i], i, sem_id);
                break;
            case WITHDRAW:
                result = process_withdraw(accounts, logs, from_acc[i], amount[i], i, sem_id);
                break;
            case TRANSFER:
                result = process_transfer(accounts, logs, from_acc[i], to_acc[i], amount[i], i, sem_id);
                break;
        }
        exit(result);
    }
    
    // Yeniden denenen işlemin tamamlanmasını bekle
    int status;
    wait(&status);
    
    // Yeniden denenen işlemin sonucu
    int retry_result = FAILURE;
    if (WIFEXITED(status)) {
        retry_result = WEXITSTATUS(status);
    }
    printf("Retry result for transaction %d: %s\n", 
           i, retry_result == SUCCESS ? "Success" : "Failed again");
    
    // Yeniden denenen işlemin logunu yazdır
    if (strcmp(logs[i].type, "Transfer") == 0) {
        printf("Transaction %d: %s %d from Account %d to Account %d (%s)\n", 
               logs[i].transaction_id, logs[i].type, 
               logs[i].amount, logs[i].from_account, 
               logs[i].to_account, logs[i].status);
    } else if (strcmp(logs[i].type, "Deposit") == 0) {
        printf("Transaction %d: %s %d to Account %d (%s)\n", 
               logs[i].transaction_id, logs[i].type, 
               logs[i].amount, logs[i].to_account, logs[i].status);
    } else if (strcmp(logs[i].type, "Withdraw") == 0) {
        printf("Transaction %d: %s %d from Account %d (%s)\n", 
               logs[i].transaction_id, logs[i].type, 
               logs[i].amount, logs[i].from_account, logs[i].status);
    }
}
    // Final hesap bakiyeleri yazdir
    printf("\nFinal account balances:\n");
    for (int i = 0; i < num_accounts; i++) {
        printf("Account %d: %d\n", accounts[i].account_id, accounts[i].balance);
    }

    // Bellek temizligi
    free(t_type);
    free(from_acc);
    free(to_acc);
    free(amount);

    // Shared memory baglantilarini kopar
    shmdt(accounts);
    shmdt(logs);

    // Shared memory ve semaphore'lari tamamen sil
    shmctl(accounts_shm_id, IPC_RMID, NULL);
    shmctl(logs_shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);

    return 0;
}