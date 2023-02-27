# string


define `USE_GC` when compiling `str.c` to use [`bdw-gc`](https://www.hboehm.info/gc/) to avoid calling any `str_destroy` functions. 

APIs: 
```c
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

// add two null terminated c type string together, return a string_t
string_t str_append_cstr( const char* start, const char* end );

// split src string upon needle string, returns an array of string_t with the last element being (string_t) { 0 }
// the easiest way to check if the array ends is to check if the capacity field is 0
// caller free, call `void str_destroy_string_arr( string_t* str_arr )` first then call `free` on the returned pointer
string_t* str_split( string_t src, const char* needle );

// set all the characters to upper case, UB if special characters
void str_toupper( string_t* string );

// set all the characters to lower case, UB if special characters
void str_tolower( string_t* string );

// return a substr starting from index start, with size size
string_t str_substr( string_t src, size_t start, size_t size );

```

### example of using `str_split`:
```c
string_t str = str_new_strings( "Hello World", "  ", "abc", "  ", "123", "  ", NULL );
string_t* str_arr = str_split( str, "  " );
for ( int i = 0; str_arr[i].capacity; i++ )
{
    str_toupper( &str_arr[i] );
    printf( "%zu, %zu, %s\n", str_arr[i].length, str_arr[i].capacity, str_arr[i].cstr );
}
str_destroy_string_arr( str_arr );
str_destroy_string( &str );
```
