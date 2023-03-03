# string


define `USE_GC` when compiling `str.c` to use [`bdw-gc`](https://www.hboehm.info/gc/) to avoid calling any `str_destroy` functions. 

### Basic APIs:
```c
// user should never change the value of length or capacity in the code. 
typedef struct string_t
{
    size_t length;
    size_t capacity;
    char cstr[1];
} string_t;


// get the length of the string, null terminater is not included
size_t str_strlen( const string_t* string );

// get the capacity of how many bytes the current container can hold
size_t str_capacity( const string_t* string );

// get a c type null terminated string from string
char* str_cstr( const string_t* string );

// get a wchar_t string from string, user free, probably useful on windows?
wchar_t* str_wstr( const string_t* string );

// constructor
string_t* str_new_string( char* src );

// constructor that take in multiple null terminated strings at once, the last argument has to be NULL
string_t* str_new_strings( char* src, ... );

// constructor that take in an array of c strings, the last element of the array has to be NULL
string_t* str_new_string_arr( char** src );

// free memory allocated by string internally
void str_destroy_string( string_t* string );

// free multiple string at once, the last argument should be NULL
void str_destroy_strings( string_t* string, ... );

// free an array of string, the last element of the array has to be NULL
// the array must be allocated on the heap since this function also calls free on the array. 
// this function is intended to free the array returned by this library
void str_destroy_string_arr( string_t** str_arr );

// effectivly clear the string, set the length to 0, may or may not change the capacity. 
bool str_clear( string_t* string );

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

// replace old with new, returning a new string_t, src is not changed
string_t* str_replace( const string_t* src, const char* old_val, const char* new_val );
```

### example code:
```c
#include "str.h"

int main( void )
{
    str_t str = str_new_strings( "Hello World", "  ", "😊😂😃😆🤔", "  ", "你好世界", "  ", "こんにちは世界", "  ", "헬로 월드", NULL );

    str_t* arr = str_split( str, "  " );
    for ( size_t i = 0; arr[i] != NULL; i++ )
    {
        printf( "%zu, %s\n", str_strlen( arr[i] ), str_cstr(arr[i]) );
    }
    str_destroy_string_arr( arr );

    str_t str2 = str_replace( str, "  ", " " );
    str_t str3 = str_substr( str2, 3, 8 );
    str_t str4 = str_append_cstrs( str2, " ", "123", "456", NULL );

    printf( "%zu, %s\n", str_strlen(str), str_cstr(str) );
    printf( "%zu, %s\n", str_strlen(str2), str_cstr(str2) );
    printf( "%zu, %s\n", str_strlen(str3), str_cstr(str3) );
    printf( "%zu, %s\n", str_strlen(str4), str_cstr(str4) );

    wchar_t* str5 = str_wstr( str );
    wprintf( L"%ls", str5 );
    free( str5 );


    str_destroy_strings( str, str2, str3, str4, NULL );


    return 0;
}
```
