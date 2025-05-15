#ifndef ACCOUNTS_H
#define ACCOUNTS_H

// Maximum characters for status messages and transaction types
#define MAX_STR_LEN 20

/**
 * @brief Account structure representing a bank account
 * 
 * This structure holds the account ID and current balance
 * It will be stored in shared memory for access by multiple processes
 */
typedef struct {
    int account_id;  // Unique identifier for the account
    int balance;     // Current account balance
} Account;

/**
 * @brief TransactionLog structure to track all banking operations
 * 
 * This structure stores information about transactions including
 * the type, source, destination, amount, and status
 */
typedef struct {
    int transaction_id;            // Unique identifier for the transaction
    char type[MAX_STR_LEN];        // "Deposit", "Withdraw", or "Transfer"
    int from_account;              // Source account (-1 if not applicable)
    int to_account;                // Destination account (-1 if not applicable)
    int amount;                    // Amount of money involved in transaction
    char status[MAX_STR_LEN];      // "Success" or "Failed"
} TransactionLog;

#endif // ACCOUNTS_H