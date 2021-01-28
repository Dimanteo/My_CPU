//! @file
#ifndef MY_TXT_FILES
    #define MY_TXT_FILES
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

/**
 * Structure containing pointer to the beginning and length of string
 */

struct String {
    size_t length;
    char *begin;
};

/**
 * Calculate size of file.
 * @param fp Pointer to file.
 * @return Size of file in bytes.
 */
size_t f_size(FILE *fp);

/**
 * Convert buffer to array of Strings.
 * @param buffer Pointer to buffer to convert.
 * @param index Pointer to array of Strings to inflate.
 * @param buf_size Size of buffer.
 * @return Size of index.
 */
size_t fill_index(char *buffer, String *index, int buf_size);

/**
 * Calculate number of char in line.
 * @param Buffer Line.
 * @param Sample Char to count.
 * @return Number of sample in buffer.
 */
int count_char(char *buffer, char sample);

/**
 * Print array of String to file with message before it. Message separates by \\n.
 * @param fout File pointer.
 * @param index Array of String.
 * @param n_lines number of lines in index.
 * @param message Message that will be printed before text.
 */
void file_output(FILE *fout, String *index, int n_lines, const char message[]);

/**
 * Compare String from back to begin.
 * @param s1 First string.
 * @param s2 Second string.
 * @return 1 if s1 > s2. 0 if s1 == s2. -1 if s1 < s2.
 */
int str_reverse_comparator(String *s1, String *s2);

/**
 * strcmp() fo Strings.
 * @param s1 First string.
 * @param s2  Second string.
 * @return strcmp(s1, s2).
 */
int string_strcmp(String *s1, String *s2);

/**
 * Takes file "name", opens it in mode and returns pointer to array, with text from file. Puts size of array in size.
 * @param name File name.
 * @param mode "r" or "rb".
 * @param size Pointer to variable, storing array size.
 * @return Pointer to array with text from file.
 * @attention Free memory after use.
 */
char *read_file_to_buffer_alloc(const char name[], const char mode[], size_t* size);

/**
 * Opens file for writing.
 * @param name name of file.
 * @return pointer to FILE.
 */
FILE* open_write_file(const char name[]);

/**
 * Generate array of Strings. Each String ends with '\0'.
 * @param buffer Text to convert.
 * @param strings_count Variable to put size of generated array.
 * @return Pointer to array of Strings.
 */
String* parse_buffer_strings_alloc(char *buffer, size_t *strings_count);

/**
 * Write Strings array to file
 * @param fout File pointer.
 * @param buffer Array of Strings.
 * @param buffer_size Size of buffer.
 */
void write_strings_file(FILE *fout, char *buffer, size_t buffer_size);

#endif