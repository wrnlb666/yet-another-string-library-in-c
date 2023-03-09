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
typedef struct string_t string_t, *str_t;



// get the length of the string, null terminater is not included
size_t str_strlen( const string_t* string );

// get the capacity of how many bytes the current container can hold
size_t str_capacity( const string_t* string );

// get a c type null terminated string from string
char* str_cstr( const string_t* string );

// get a wchar_t string from string, user free, probably useful on windows?
// use str_free to free the returned result
wchar_t* str_wstr( const string_t* string );

// constructor
string_t* str_new_string( const char* src );

// construct using format string
string_t* str_new_format( const char* format, ... );

// constructor that take in multiple null terminated strings at once, the last argument has to be NULL
string_t* str_new_strings( const char* src, ... );

// constructor that take in an array of c strings, the last element of the array has to be NULL
string_t* str_new_string_arr( const char** src );

// exactly the same with with str_new_string_arr, but you need to input array size manually
string_t* str_new_string_narr( const char** src, size_t size );

// free memory allocated by string internally
void str_free( void* string );

// free multiple string at once, the last argument should be NULL
void str_frees( string_t* string, ... );

// free an array of string, the last element of the array has to be NULL
// the array must be allocated on the heap since this function also calls free on the array. 
// this function is intended to free the array returned by this library
void str_free_arr( string_t** str_arr );

// effectivly clear the string, set the length to 0, may or may not change the capacity. 
bool str_clear( string_t* string );

// free the memory used by string, and at the same time return a new string.
// e.g.: str = str_clear_to( str, str_new_format( "random string" ) );
string_t* str_clear_to( string_t* old, string_t* new );

// reserve memory that can hold at least length char, length+1 if count '\0'
bool str_reserve( string_t* string, size_t length );

// append two strings together, address can be overlapped
string_t* str_append( const string_t* start, const string_t* end );

// append strings together, the last element has to be NULL
string_t* str_appends( const string_t* start, ... );

// add null terminated c type string to string_t, return a string_t*
string_t* str_append_cstr( const string_t* start, const char* end );

// add null terminated c type strings to string_t, the last element has to be NULL, return a string_t*
string_t* str_append_cstrs( const string_t* start, ... );

// split src string upon needle string, returns an array of string_t with the last element being NULL
// caller free, use `str_destroy_string_arr` to free the returned array
string_t** str_split( const string_t* src, const char* needle );

// set all the characters to upper case, UB if special characters
void str_to_upper( string_t* string );

// set all the characters to lower case, UB if special characters
void str_to_lower( string_t* string );

// return a substr starting from index start, with size size
string_t* str_substr( const string_t* src, size_t start, size_t size );

// copy a string, return a new string
string_t* str_strcpy( const string_t* src );

// insert a c string into src at index index, returning the new string
string_t* str_insert( const string_t* src, size_t index, const char* in );

// replace old with new, returning a new string_t, src is not changed
string_t* str_replace( const string_t* src, const char* old_val, const char* new_val );

// sort string array of size size, if size is 0, then the array has to be NULL terminated
// mode should be l for length, a for alphabetical, i for ignore case, c for custom
// using "a" or "ai" for multibytes string may not return the correct result
// if used c, other modes will be ignored
// order does matter for "l" and "a", "la" and "al" will return different result
bool str_sort( string_t** src, size_t size, const char* mode, ... );

// compare str, 0 means the same
int str_strcmp( const string_t* str1, const string_t* str2 );

// return if the string start with the input c string
bool str_start_with( const string_t* self, const char* str );

// return if the string end with the input c string
bool str_end_with( const string_t* self, const char* str );



#endif  // __STR_H__
