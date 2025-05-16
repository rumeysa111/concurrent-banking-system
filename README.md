# Concurrent Banking System

## Project Overview

This project is a concurrent banking system simulation developed as part of an Operating Systems course. It demonstrates practical implementation of core operating system concepts including process management, inter-process communication (IPC), and synchronization mechanisms.

The system simulates a banking environment where multiple accounts can be managed concurrently, with operations such as deposits, withdrawals, and transfers processed in parallel while maintaining data consistency.

## Features

- **Multi-process Architecture**: Each banking transaction runs as a separate process
- **Shared Memory**: System V IPC mechanisms for data sharing between processes
- **Synchronization**: Semaphores for controlling concurrent access to accounts
- **Deadlock Prevention**: Resource hierarchy approach to prevent deadlocks
- **Transaction Types**: Deposit, withdrawal, and transfer operations
- **Transaction Logging**: Detailed tracking of all operations
- **Error Handling**: Retry mechanism for failed transactions

## System Requirements

- Linux-based operating system
- gcc compiler
- GNU make

## Installation and Compilation

To compile the project, follow these steps:

```bash
# Clone the repository
git clone https://github.com/rumeysa111/concurrent-banking-system.git
cd ConcurrentBankingSystem

# Compile using Makefile
make

# Alternatively, compile directly with gcc
gcc -o bank src/*.c -Iinclude -Wall
```

## Usage

Run the system with:

```bash
./bank
```

When executed, the program will:

1. Read account information from accounts.txt (or create default accounts if file not found)
2. Read transaction information from transactions.txt
3. Create separate processes for each transaction
4. Display final account balances and transaction logs after completion

### Input Files

#### accounts.txt

Contains account information. First line indicates the total number of accounts, subsequent lines contain account ID and balance:

```
5
0, 500
1, 500
2, 500
3, 500
4, 500
```

#### transactions.txt

Contains transaction information. Each line represents a transaction:

```
0, -1, 0, 100   # Transaction type, source account, destination account, amount
```

Transaction types:
- 0: Deposit (source account should be -1)
- 1: Withdrawal (destination account should be -1)
- 2: Transfer

## Project Architecture

### File Structure

```
ConcurrentBankingSystem/
├── include/
│   ├── accounts.h      # Account and transaction log data structures
│   ├── transactions.h  # Transaction function declarations
│   └── utils.h         # Synchronization helper functions
├── src/
│   ├── main.c          # Main program flow
│   ├── transactions.c  # Transaction function implementations
│   └── utils.c         # Semaphore operation implementations
├── accounts.txt        # Account information
├── transactions.txt    # Transaction information
└── Makefile            # Compilation rules
```

### Components and Responsibilities

1. **main.c**: Manages the main program flow
   - Creates shared memory and semaphores
   - Reads account and transaction information from files
   - Creates child processes for each transaction
   - Displays results

2. **transactions.c**: Contains transaction functions
   - `process_deposit()`: Handles deposit operations
   - `process_withdraw()`: Handles withdrawal operations
   - `process_transfer()`: Handles transfer operations
   - `read_transactions()`: Reads transactions from file
   - `initialize_accounts()`: Reads accounts from file

3. **utils.c**: Manages synchronization operations
   - `init_semaphore()`: Initializes a semaphore
   - `sem_p()`: Locks a semaphore (P operation)
   - `sem_v()`: Unlocks a semaphore (V operation)

## Concurrent Programming Principles

### Inter-Process Communication (IPC)

- **Shared Memory**: Common memory area accessible to all processes
  - Used for account information and transaction logs
  - Implemented using System V IPC mechanisms (`shmget`, `shmat`, `shmdt`)

### Synchronization

- **Semaphores**: Mechanism to control access to critical sections
  - One semaphore per account
  - P operation (wait/lock) used on entry to critical section
  - V operation (signal/unlock) used on exit from critical section

### Race Condition Prevention

Race conditions can occur when multiple processes perform ordering-dependent operations on the same data. In this project:

- Each account is locked with a semaphore before access
- The lock is released only after the operation is complete

### Deadlock Prevention Strategy

Deadlock occurs when two or more processes are waiting for resources held by each other. In this project:

- Accounts are always locked in order of increasing account ID
- This "resource hierarchy" approach prevents circular wait conditions

## Algorithm Details

### Deposit Algorithm

```
1. Lock the account
2. Add money to the account
3. Create transaction record
4. Release the lock
5. Return SUCCESS
```

### Withdrawal Algorithm

```
1. Lock the account
2. Check account balance
   a. If insufficient, create failed transaction record and return FAILURE
   b. If sufficient, deduct money and create successful transaction record
3. Release the lock
4. Return SUCCESS
```

### Transfer Algorithm

```
1. Lock both accounts (in order of increasing account ID to prevent deadlock)
2. Check source account balance
   a. If insufficient, create failed transaction record and return FAILURE
   b. If sufficient:
      i. Deduct money from source account
      ii. Add money to destination account
      iii. Create successful transaction record
3. Release both locks
4. Return SUCCESS
```

## Sample Output

When the program finishes execution, it produces output similar to:

```
Transaction Log:
Transaction 0: Deposit 100 to Account 0 (Success)
Transaction 1: Withdraw 50 from Account 1 (Success)
Transaction 2: Transfer 615 from Account 2 to Account 3 (Failed)
Transaction 3: Deposit 200 to Account 4 (Success)
Transaction 4: Transfer 30 from Account 1 to Account 0 (Success)
Transaction 5: Withdraw 60 from Account 3 (Success)
Transaction 6: Deposit 120 to Account 2 (Success)
Transaction 7: Transfer 90 from Account 4 to Account 1 (Success)

Retrying 1 failed transactions...
Transaction 2 failed. Retrying once...
Retry result for transaction 2: Success
Transaction 2: Transfer 615 from Account 2 to Account 3 (Success)
Final account balances:
Account 0: 630
Account 1: 470
Account 2: 5
Account 3: 1055
Account 4: 610
```

