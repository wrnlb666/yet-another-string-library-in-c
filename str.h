#ifndef __STR_H__
#define __STR_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>



// user should never change the value of length or capacity in the code. 
typedef struct string_t
{
    size_t length;
    size_t capacity;
    char* cstr;
} string_t;


// constructor
string_t str_new_string( char* src );

// constructor that take in multiple null terminated strings at once, the last argument should be NULL
string_t str_new_strings( char* src, ... );

// free memory allocated by string internally
void str_destroy_string( string_t* string );

// free multiple string at once, the last argument should be NULL
void str_destroy_strings( string_t* string, ... );

// free an array of string, the last element of the array should be (string_t) { 0 }, at least the capacity should be 0
// the array must be allocated on the heap since this function also calls free on the array. 
void str_destroy_string_arr( string_t* str_arr );

// effectivly clear the string, set the length to 0, may or may not change the capacity. 
bool str_clear( string_t* string );

// append two strings together, address can be overlapped
string_t str_append( string_t start, string_t end );

// append strings together, the last element has to be (string_t) { 0 }
string_t str_appends( string_t start, ... );

// add null terminated c type string to string_t, return a string_t
string_t str_append_cstr( string_t start, const char* end );

// add null terminated c type strings to string_t, the last element has to be NULL, return a string_t
string_t str_append_cstrs( string_t start, ... );

// split src string upon needle string, returns an array of string_t with the last element being (string_t) { 0 }
// the easiest way to check if the array ends is to check if `string_t.cstr == NULL`
// caller free, call `void str_destroy_string_arr( string_t* str_arr )` first then call `free` on the returned pointer
string_t* str_split( string_t src, const char* needle );

// set all the characters to upper case, UB if special characters
void str_to_upper( string_t* string );

// set all the characters to lower case, UB if special characters
void str_to_lower( string_t* string );

// return a substr starting from index start, with size size
string_t str_substr( string_t src, size_t start, size_t size );



#endif  // __STR_H__
