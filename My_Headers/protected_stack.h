#ifndef PROTECTED_STACK_H
#define PROTECTED_STACK_H
/// @file
///Define OK_DUMP to print dump on every verification call, even if stack is ok. Undef to dump only corrupted stack.
///Define NO_HASH to disable hash calculation.
///Define EXPLOSIVE_DEBUG and soft_assert() wil work like assert(). If undefined soft_assert() will only print message to stderr. Defininf NDEBUG disables soft_assert().
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

///Create Stack_t name variable and initialize tag with name of variable.
#define Stack_t_name(name) Stack_t name = {#name};\
                            stack_init(&name);



static bool assertion_failed(const char condition[], const char file[], const char fun[], int line) {
    fprintf(stderr, "Assert failed: %s, file %s; function %s (%d)\n", condition, file, fun, line);
    abort();
    return 0;
}
#ifndef NDEBUG
#ifdef EXPLOSIVE_DEBUG

///soft_assert() check condition and, if it false, print message to stderr. Returns 1 - condition is true; 0 - condition is false.
    #define soft_assert(condition, file, fun, line) ((!(condition)) ? assertion_failed(#condition, file, fun, line) : 1)
#else
///soft_assert() check condition and, if it false, print message to stderr. Returns 1 - condition is true; 0 - condition is false.
    #define soft_assert(condition, file, fun, line)  ((!(condition)) ? (fprintf(stderr, "Assert failed: %s, file %s; function %s (%d)\n", #condition, file, fun, line) * 0) : 1)
#endif
#endif

typedef int element_t;
#define ELEMENT_PRINT "%d"
typedef unsigned int canary_t;
#define CANARY_PRINT "%#X"

const element_t POISON = -1337;
const size_t DELTA = 4;
const size_t MAX_NAME_LENGTH = 25;
const size_t DEFAULT_STACK_SIZE = 10;
const char DEFAULT_STACK_LOG_NAME[] = "Stack.log";
enum {STACK_UNDERFLOW = 42, ALLOCATION_ERROR = 43} ;
const char ERR_STATE[] = "ERROR";
const char OK_STATE[] = "ok";
const char PARENT_ERR[] = "Called by parent. ERROR";
const char PARENT_OK[] = "Called by parent. ok";
const canary_t CANARY_VALUE = 0xBEDA;


/**
 * Stack with dynamic size and data protection.
 */

struct Stack_t {
    char tag[MAX_NAME_LENGTH];
    canary_t local_canary1 = CANARY_VALUE;
    size_t size = 0;
    size_t max_size = 0;
    unsigned int local_hash = 0;
    unsigned int buf_hash = 0;
    char* buffer = nullptr;
    element_t* data = nullptr;
    canary_t *buf_canary1 = nullptr, *buf_canary2 = nullptr;
    canary_t local_canary2 = CANARY_VALUE;
    FILE* log;
};

/**
 * Stack constructor.
 * @param stack Pointer to stack.
 * @param size Number of elements in stack. Default (10).
 * @param tag Allows to give stack variable name, that will be printed in stack_dump. Default (undefined).
 */
void stack_init(Stack_t *stack, size_t size = 10);

/**
 * Push value to stack.
 * @param stack
 * @param value
 * @return true if succeeded, false if failed.
 */
bool stack_push(Stack_t* stack, element_t value);

/**
 * Pop element from stack.
 * @param stack Pointer to stack.
 * @param error_code Pointer to error code variable. Default (nullptr).
 * Place STACK_UNDERFLOW in case of popping empty stack. ALLOC_ERROR in case of resize error.
 * @return Upper element of stack. POISON in case of error.
 */
element_t stack_pop(Stack_t* stack, int* error_code = nullptr);

/**
 * Destructor of stack.
 * @param stack Pointer to stack.
 */
void stack_destruct(Stack_t* stack);

/**
 * Calculates MurMurHash from buffer containing length bytes.
 * @param buffer
 * @param length
 * @return
 */
unsigned int hash(char* buffer, size_t length);

/**
 * Update stack hash value.
 * @param stack Pointer to stack.
 * @note Hash calculating could be disabled by defining NO_HASH or NDEBUG.
 */
void stack_refresh_hash(Stack_t* stack);

/**
 * Change stack data array size.
 * @param stack Pointer to stack.
 * @param size New size of data array.
 * @return true if succeeded, false if error occurred.
 */
bool stack_resize(Stack_t* stack, size_t size);

/**
 * Print full info about stack.
 * @param stack Pointer to stack.
 * @param reason Message, explaining why dump was called.
 * @param state Indicator of stack correctness. ERR_STATE if dump calling because of error. OK_STATE if stack is correct.
 * @param file String that will be printed as the title of file, that caused stack_dump() generation.
 * @param function String that will be printed as the title of function, that caused stack_dump() generation.
 * @param line Number of line, that will be printed as number of line, that caused stack_dump() generation.
 */
void stack_dump(Stack_t* stack, const char reason[], const char state[], const char file[], const char function[], int line);

/**
 * Stack_t verificator.
 * @param stack Pointer to stack.
 * @param file Title of file, that called stack_OK().
 * @param function Title of function, that called stack_OK().
 * @param line Numder of line, where stack_dump() was called
 * @return TRUE if stack is ok. FALSE if stack is incorrect
 * @note Allows to print Dump on every call, even if stack is correct. To activate this mode define OK_DUMP.
 * @note Defining of NDEBUG will deactivate all stack_OK().
 */
bool stack_OK(Stack_t* stack, const char file[], const char function[], int line);

/**
 * Close previous and open new file for stack dumps.
 * @param stack Stack, which will be dumped in file.
 * @param name Name of log file.
 * @param mode File acces mode.
 * @return Pointer to file.
 * @attention Log file closes in stack_destruct.
 */
FILE* stack_reopen_log(Stack_t* stack, const char *name = DEFAULT_STACK_LOG_NAME, const char *mode = "ab");

#endif