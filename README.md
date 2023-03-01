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

### example of using `str_split`:
```c
string_t* str = str_new_strings( "Hello World", "  ", "woc", "  ", "abc", "  ", "123", NULL );

string_t** arr = str_split( str, "  " );
for ( size_t i = 0; arr[i] != NULL; i++ )
{
    printf( "%zu, %zu, %s\n", arr[i]->length, arr[i]->capacity, arr[i]->cstr );
}
// the following two destroy function is not necessary if build with `-D USE_GC`
str_destroy_string_arr( arr );
str_destroy_string( str );
```
