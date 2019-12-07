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



bool assertion_failed(const char condition[], const char file[], const char fun[], int line) {
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
enum {STACK_UNDERFLOW = 42, ALLOCATION_ERROR = 43} ;
const char ERR_STATE[] = "ERROR";
const char OK_STATE[] = "ok";
const char PARENT_ERR[] = "Called by parent. ERROR";
const char PARENT_OK[] = "Called by parent. ok";
const char DEFAULT_STACK_LOG_NAME[] = "stack_log.txt";
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

bool stack_OK(Stack_t* stack, const char file[], const char function[], int line) {
    bool all_OK = true;
#ifndef NDEBUG
    if (stack == nullptr) {
        stack_dump(stack, "stack pointer is NULL", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack != nullptr, file, function, line);

    if (stack->size < 0) {
        stack_dump(stack, "size < 0", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->size >= 0, file, function, line);

    if (stack->max_size <= 0) {
        stack_dump(stack, "max_size ,= 0", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->max_size > 0, file, function, line);

    if (stack->local_canary1 != CANARY_VALUE) {
        stack_dump(stack, "local_canary1 changed", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->local_canary1 == CANARY_VALUE, file, function, line);

    if (stack->local_canary2 != CANARY_VALUE) {
        stack_dump(stack, "local_canary2 changed", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->local_canary2 == CANARY_VALUE, file, function, line);

    if (stack->data == nullptr) {
        stack_dump(stack, "data pointer is NULL", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->data != nullptr, file, function, line);

    if (stack->buf_canary1 == nullptr) {
        stack_dump(stack, "buf_canary1 pointer is NULL", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->buf_canary1 != nullptr, file, function, line);

    if (stack->buf_canary2 == nullptr) {
        stack_dump(stack, "buf_canary2 pointer is NULL", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->buf_canary2 != nullptr, file, function, line);

    if (stack->buffer == nullptr) {
        stack_dump(stack, "buffer pointer is NULL", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->buffer != nullptr, file, function, line);

    if (stack->buf_canary1 == stack->buf_canary2) {
        stack_dump(stack, "buf_canary1 and buf_canary2 pointers are equal", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->buf_canary1 != stack->buf_canary2, file, function, line);

    if (*stack->buf_canary1 != CANARY_VALUE) {
        stack_dump(stack, "buf_canary1 has changed", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(*stack->buf_canary1 == CANARY_VALUE, file, function, line);

    if (*stack->buf_canary2 != CANARY_VALUE) {
        stack_dump(stack, "buf_canary2 has changed", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(*stack->buf_canary2 == CANARY_VALUE, file, function, line);

    if ((element_t*)(stack->buf_canary1 + 1) != stack->data) {
        stack_dump(stack, "buf_canary1 not align to data", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert((element_t*)(stack->buf_canary1 + 1) == stack->data, file, function, line);

    if (stack->data + stack->max_size != (element_t*)stack->buf_canary2) {
        stack_dump(stack, "buf_canary2 not align to data", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->data + stack->max_size == (element_t*)stack->buf_canary2, file, function, line);

    unsigned int previous_local_hash = stack->local_hash;
    unsigned int previous_buf_hash = stack->buf_hash;
    stack_refresh_hash(stack);

    if (stack->local_hash != previous_local_hash) {
        stack_dump(stack, "local_hash mismatch", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->local_hash == previous_local_hash, file, function, line);

    if (stack->buf_hash != previous_buf_hash) {
        stack_dump(stack, "buf_hash mismatch", ERR_STATE, file,  function, line);
        all_OK = false;
    }
    soft_assert(stack->buf_hash == previous_buf_hash, file, function, line);

    #ifdef OK_DUMP
    if (all_OK) {
        stack_dump(stack, "Calm down, It's ok))", OK_STATE, file,  function, line);
    }
    #endif
#endif
    return all_OK;
}

void stack_init(Stack_t *stack, size_t size /*= 10*/) {
    assert(stack);
    assert(size > 0);
    stack->local_canary1 = CANARY_VALUE;
    stack->local_canary2 = CANARY_VALUE;
    stack->size = 0;
    stack->max_size = size;
    stack->local_hash = 0;
    stack->buf_hash = 0;
    stack->buffer = (char*) calloc(stack->max_size * sizeof(element_t) + 2 * sizeof(canary_t), sizeof(char));
    assert(stack->buffer);
    stack->data = (element_t*)(stack->buffer + sizeof(canary_t));
    stack->buf_canary1 = (canary_t*)stack->buffer;
    stack->buf_canary2 = (canary_t*)(stack->buffer + stack->max_size * sizeof(element_t) + sizeof(canary_t));
    for (int i = 0; i < stack->max_size; ++i) {
        stack->data[i] = POISON;
    }
    *stack->buf_canary1 = CANARY_VALUE;
    *stack->buf_canary2 = CANARY_VALUE;
    stack->log = nullptr;

    stack_refresh_hash(stack);
    stack_OK(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);
}

bool stack_push(Stack_t* stack, element_t value) {
    stack_OK(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);

    if (stack->size == stack->max_size) {
        if (!stack_resize(stack, stack->max_size * 2)){
            return false;
        }
        stack_refresh_hash(stack);
        stack_OK(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    }

    stack->data[stack->size++] = value;

    stack_refresh_hash(stack);
    stack_OK(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    return true;
}

element_t stack_pop(Stack_t* stack, int* error_code /*= nullptr*/) {
    stack_OK(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);

    if (stack->size == 0) {
        if(error_code != nullptr) {
            *error_code = STACK_UNDERFLOW;
        }
        return POISON;
    }

    element_t ret_value = stack->data[--(stack->size)];
    stack->data[stack->size] = POISON;
    stack_refresh_hash(stack);
    stack_OK(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);

    if(stack->size < stack->max_size / 2 - DELTA && stack->max_size / 2 > 5) {
        if (!stack_resize(stack, stack->max_size / 2)) {
            *error_code = ALLOCATION_ERROR;
            return POISON;
        }
        stack_refresh_hash(stack);
        stack_OK(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    }

    if (error_code != nullptr) {
        *error_code = 0;
    }
    return ret_value;
}

bool stack_resize(Stack_t* stack, size_t size) {
    stack_OK(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);

    stack->max_size = size;
    char* new_buf = (char*)calloc(stack->max_size * sizeof(element_t) + 2 * sizeof(canary_t), sizeof(char));
    assert(new_buf);
    if (!new_buf) { //check for NDEBUG
        return false;
    }
    stack->buf_canary1 = (canary_t*)new_buf;
    *stack->buf_canary1 = CANARY_VALUE;
    stack->buf_canary2 = (canary_t*)(new_buf + stack->max_size * sizeof(element_t) + sizeof(canary_t));
    *stack->buf_canary2 = CANARY_VALUE;
    for (int i = 0; i < stack->size; ++i) {
        ((element_t*)(new_buf + sizeof(canary_t)))[i] = stack->data[i];
    }
    for (int j = stack->size; j < stack->max_size; ++j) {
        ((element_t*)(new_buf + sizeof(canary_t)))[j] = POISON;
    }
    stack->data = (element_t*)(new_buf + sizeof(canary_t));
    free(stack->buffer);
    stack->buffer = new_buf;

    stack_refresh_hash(stack);
    stack_OK(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);
    return true;
}

unsigned int hash(char* buffer, size_t length) {
    assert(buffer);

    const unsigned int magic_constant = 0x5bd1e995;
    const unsigned int seed = 42;
    unsigned int hash = seed ^ length;
    /*operations with 4 byte blocks*/
    while (length >= 4)
    {
        /*merging together 4 bytes*/
        unsigned int word = buffer[0];//[-][-][-][0]
        word |= buffer[1] << 8;//[-][-][1][0]
        word |= buffer[2] << 16;//[-][2][1][0]
        word |= buffer[3] << 24;//[3][2][1][0]

        word *= magic_constant;
        unsigned int w = word;
        unsigned int s = word;
        word ^= word >> 24;
        word *= magic_constant;
        hash *= magic_constant;
        hash ^= word;
        buffer += 4;
        length -= 4;
    }
    /*operations with unused bytes*/
    switch (length) {
        case 3:
            hash ^= buffer[2] << 16;
        case 2:
            hash ^= buffer[1] << 8;
        case 1:
            hash ^= buffer[0];
            hash *= magic_constant;
    }

    hash ^= hash >> 13;
    hash *= magic_constant;
    hash ^= hash >> 15;

    return hash;
}

unsigned int easy_hash(char* buffer, size_t length) {
    unsigned int hash = 0;
    for (int i = 0; i < length; ++i) {
        hash += i * buffer[i];
    }
    return hash;
}

void stack_refresh_hash(Stack_t* stack) {
#ifdef NO_HASH
    assert(stack);

    stack->local_hash = 0;
    stack->buf_hash = 0;
    stack->local_hash = hash((char*)stack, (char*)(&stack->local_canary2 + 1) - (char*)(&stack->local_canary1));
    stack->buf_hash = hash(stack->buffer, stack->max_size * sizeof(element_t) + 2 * sizeof(canary_t));
#endif
}


void stack_dump(Stack_t* stack, const char reason[], const char state[], const char file[], const char function[], int line) {

    if (stack == nullptr) {
        FILE* log = fopen(DEFAULT_STACK_LOG_NAME, "ab");
        fprintf(log, "\nDump(%s) From %s; %s (%d)\n\tStack_t [NULL pointer] (%s)\n", reason, file, function, line, state);
        fclose(log);
        assert(stack);
        return;
    }

    if (stack->log == nullptr) {
        stack_reopen_log(stack);
    }
    fprintf(stack->log,
            "\nDump(%s) From %s; %s (%d)\n"
            "\tStack_t %s [%p] (%s)\n"
            "\t{\n"
            "\t\tlocal_canary1 = " CANARY_PRINT "\n"
            "\t\tsize = %u\n"
            "\t\tmax_size = %u\n"
            "\t\tbuf_canary1[%p] = " CANARY_PRINT "\n"
            "\t\tbuf_canary2[%p] = " CANARY_PRINT "\n"
            "\t\tbuffer[%p]\n"
            "\t\tdata[%p]\n"
            "\t\t{\n",
            reason, file, function, line, stack->tag, stack, state, stack->local_canary1, stack->size, stack->max_size, stack->buf_canary1, *stack->buf_canary1, stack->buf_canary2, *stack->buf_canary2, stack->buffer, stack->data);
    for (int i = 0; i < stack->max_size; ++i) {
        if (i < stack->size) {
            fprintf(stack->log, "\t\t\t*[%d] = " ELEMENT_PRINT "%s\n", i, stack->data[i], (stack->data[i] == POISON) ? "(POISON?)": "");
        } else {
            fprintf(stack->log, "\t\t\t [%d] = " ELEMENT_PRINT "%s\n", i, stack->data[i], (stack->data[i] == POISON) ? "(POISON?)": "");
        }
    }
    fprintf(stack->log,
            "\t\t}\n"
            "\t\tlocal_hash = %u\n"
            "\t\tbuf_hash = %u\n"
            "\t\tlocal_canary2 = " CANARY_PRINT "\n"
            "\t}\n",
            stack->local_hash, stack->buf_hash, stack->local_canary2);
}

void stack_destruct(Stack_t* stack) {
    stack_OK(stack, __FILE__, __PRETTY_FUNCTION__, __LINE__);

    for (int i = 0; i < stack->max_size - 1; ++i) {
        stack->data[i] = 0;
    }
    *stack->buf_canary1 = 0;
    *stack->buf_canary2 = 0;
    stack->buf_canary1 = nullptr;
    stack->buf_canary2 = nullptr;
    stack->data = nullptr;
    stack->max_size = 0;
    stack->size = 0;
    stack->local_canary1 = 0;
    stack->local_canary2 = 0;
    stack->local_hash = 0;
    stack->buf_hash = 0;
    free(stack->buffer);
    stack->buffer = nullptr;
    fclose(stack->log);
}

FILE* stack_reopen_log(Stack_t *stack, const char *name /*= DEFAULT_STACK_LOG_NAME*/, const char *mode /*= "ab"*/) {
    fclose(stack->log);
    stack->log = fopen(name, mode);
    assert(stack->log);
    return stack->log;
}

#endif