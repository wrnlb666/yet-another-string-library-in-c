# Yet Another String Library In C
A string library that only depends on libc, with optional bdw-gc support to have a gc manage memory for you. 

define `YASLI_GC` when compiling `yasli.c` to use [`bdw-gc`](https://www.hboehm.info/gc/) to avoid need of calling any `str_free` functions. 
define `YASLI_DEBUG` when compiling `yasli.c` to let the library print out error messages. 

### Struct:
***
I don't recommand defining this in your own file. But as long as you know what you are doing, just do whatever you want.
```c
struct string_t
{
    size_t length;
    size_t capacity;
    char cstr[1];
} string_t, *str;
```

### Convention:
***
In this library, arrays should end with an element of `NULL`, for variadic arg functions, the last argument should be `NULL` as well to indicate end of argument list. 

All function has prefix `str_`, and for different argument type, the postfix should be easy to remember as well:

* `str_func_arr()` will take an `NULL` terminated arr as arguments. 
* `str_funcs()` will take variadic `NULL` terminated arguments. 
* `str_funced()` means the function will return a newly allocated string. 
* `str_func()` will have two meaning, one being takes only one argument, the other one being that it make changes on the original string. 
* `str_utf8_func()` will treat the string as a utf-8 encoded string. But since utf-8 characters are variadic length, it is much slower than normal functions. 

Since `malloc`, `realloc` may fail when there's not enough memory, any call from this library may fail for the exact same reason. For any unsuccessful call, the library will return a `NULL` pointer as an result. 


### API:
***
#### Constructor:
* return a new string of type `str_t` or `string_t*`. Return `NULL` on failure. 
```c
string_t* str_new_string( const char* src );
```
* return a new string of type `str_t` or `string_t*` with format string. Return `NULL` on failure. 
```c
string_t* str_new_format( const char* format, ... );
```
* return a new string of type `str_t` or `string_t*` from mutiple c type strings. The last element has to be `NULL`. Return `NULL` on failure. 
```c
string_t* str_new_strings( const char* src, ... );
```
* return a new string of type `str_t` or `string_t*` from an array of c type strings. The last element of the array has to be `NULL`. Return `NULL` on failure. 
```c
string_t* str_new_string_arr( const char** src );
```
* return a new string of type `str_t` or `string_t*` from an array of c type string. While the last element of the array doesn't have to be `NULL`, user need to enter size of the array manually. Return `NULL` on failure. 
```c
string_t* str_new_string_narr( const char** src, size_t size );
```
* return a new string of type `str_t` or `string_t*` from a file. Return `NULL` on failure: failed to open file or allocating error. 
```c
string_t* str_from_file( const char* file_name );
```

#### Destructor:
* free a single `str_t` or `string_t*`. 
```c
void str_free( void* string );
```
* free a list of `str_t` or `string_t*`, the last element has to be `NULL`. 
```c
void str_frees( string_t* string, ... );
```
* free an array of `str_t` or `string_t*`, the last element of the array has to be `NULL`. 
```c
void str_free_arr( string_t** str_arr );
```

#### Getter:
* get the length of the string. 
```c
size_t str_strlen( const string_t* string );
```
* get the length of the utf-8 encoded string. 
```c
size_t str_utf8_strlen( const string_t* string );
```
* get the current capacity of the string. 
```c
size_t str_capacity( const string_t* string );
```
* get a `const char*` not modifiable c type string. Keep in mind that even though there're tricks in c language that you can do to get around and modify this string, it is not recommend to do so. The address of the returned string is exactly the same with the one managed by the library. 
```c
const char* str_cstr( const string_t* string );
```
* This is not a typical getter function. It returns a `malloc` allocated `wchar_t*` or `LPWSTR`( if on windows ). This function is caller free if the library is not build with `USE_GC` preprocessor. Please use `str_free` function to free this string in case that the wide string might be allocated by `GC_malloc`. 
```c
wchar_t* str_wstr( const string_t* string );
```
* This is both a setter and a getter, to use it as a getter, use `0` as the value for `new_val`. On success, this function will return the the character at the asked index. On failure, this function will return 0 or `\0`. 
```c
char str_char_at( string_t* self, size_t index, char new_val );
```
*  Get the utf-8 character at specified index. The returned value is a utf-8 encoded string that contains only one utf-8 character. User must not free the returned value, since the buffer is managed by the library. But user should copy the character if it is needed for later use. On success, this function returns the utf-8 character as a c type string, on failure, this function returns a string with 0 length. 
```c
char* str_utf8_char_at( string_t* self, size_t index );
```

#### Setter:
* This is both a setter and a getter, to use it as a setter, give an ascii character to `new_val`. On success, this function will return the new character at the changed index. On failure, this funtion will return 0 or `\0`. 
```c
char str_char_at( string_t* self, size_t index, char new_val );
```
* This function will set all the ASCII characters in the string to upper case. 
```c
void str_to_upper( string_t* string );
```
* This function will set all the ASCII characters in the string to lower case. 
```c
void str_to_lower( string_t* string );
```

#### Append Functions: 
* This function append the `end` string to the end of `start` string and return a new string. both `end` and `start` is not changed. If out of memory, this function will return `NULL`. 
```c
string_t* str_appended( const string_t* start, const string_t* end );
```
* This function append the `end` string to the end of `start` string and return a pointer to `start` string. `start` string may or may not be reallocated. If out of memory, this function will return `NULL`. 
```c
string_t* str_append( string_t** start, const string_t* end );
```
* This function append several `string_t*` to the end of `start` in the same order, the last element of the variadic arguments list must be `NULL`. The return value is a new string. If out of memory, this function will return `NULL`. 
```c
string_t* str_appendeds( const string_t* start, ... );
```
* This function append several `string_t*` to the end of `start` in the same order as input, the last element of the variadic arguments list must be `NULL`. It will return a pointer to `start`, string `start` may or may not reallocate. If out of memory, this function will return `NULL`. 
```c
string_t* str_appends( string_t** start, ... );
```
* This function append the `end` C type null terminated string to the end of `start` string and return a new string. both `end` and `start` is not changed. If out of memory, this function will return `NULL`.
```c
string_t* str_appended_cstr( const string_t* start, const char* end );
```
* This function append the `end` c type null terminated string to the end of `start` string and return a pointer to `start` string. `start` string may or may not be reallocated. If out of memory, this function will return `NULL`. 
```c
string_t* str_append_cstr( string_t** start, const char* end );
```
* This function append several `char*` to the end of `start` in the same order, the last element of the variadic arguments list must be `NULL`. The return value is a new string. If out of memory, this function will return `NULL`. 
```c
string_t* str_appended_cstrs( const string_t* start, ... );
```
* This function append several `char*` to the end of `start` in the same order as input, the last element of the variadic arguments list must be `NULL`. It will return a pointer to `start`, string `start` may or may not reallocate. If out of memory, this function will return `NULL`. 
```c
string_t* str_append_cstrs( string_t** start, ... );
```

#### High Level API:
* This function split src into an array of new strings. The last element of the array is guaranteed to be `NULL`. Assuming the array is called `arr`, you can use `for ( str_t* str = arr; *str; str++ )` to traverse the array. The array is caller free, use `str_destroy_string_arr` to free the returned array. 
```c
string_t** str_split( const string_t* src, const char* needle );
```
* This function returns a sub-string from index `start`, with `size` number of ASCII characters. Return `NULL` on failure. 
```c
string_t* str_substr( const string_t* src, size_t start, size_t size );
```
* This function returns a sub-string from index `start`, with `size` number of utf-8 encoded characters. Return `NULL` on failure. 
```c
string_t* str_utf8_substr( const string_t* src, size_t start, size_t size );
```
* This function copys a string and create a new one from the existing string. Return `NULL` on failure. 
```c
string_t* str_strdup( const string_t* src );
```
* this function replace all `old_val` in the string with `new_val` and return a new string. Return `NULL` on failure. 
```c
string_t* str_replaced( const string_t* src, const char* old_val, const char* new_val );
```
* String slicing, use `YASLI_START` and `YASLI_END` for start of the string and end of the string. This function return a new string, return `NULL` on failure. 
```c
string_t* str_sliced( const string_t* src, int64_t start, int64_t end, int64_t step );
```
* String slicing, use `YASLI_START` and `YASLI_END` for start of the string and end of the string. This function return the address of self, or return `NULL` on failure.
```c
string_t* str_slice( string_t** self, int64_t start, int64_t end, int64_t step );
```
* utf-8 string slicing, use `YASLI_START` and `YASLI_END` for start of the string and end of the string. This function return a new string, or `NULL` on failure. 
```c
string_t* str_utf8_sliced( const string_t* src, int64_t start, int64_t end, int64_t step );
```
* utf-8 string slicing, use `YASLI_START` and `YASLI_END` for start of the string and end of the string. This function return the address of self, or return `NULL` on failure. 
```c
string_t* str_utf8_slice( string_t** self, int64_t start, int64_t end, int64_t step );
```
* This function returns a new `string_t*` that is stripped from src. Every single character in the needle is considered as a "not wanted" character, and will be deleted from the start and end. 
```c
string_t* str_stripped( const string_t* src, const char* needle );
```
* This function returns the address of stripped `self`. Every single character in the needle is considered as a "not wanted" character, and will be deleted from the start and end. 
```c
string_t* str_strip( string_t** self, const char* needle );
```
* Check if the string start with `str`. Return `true` if it does. 
```c
bool str_start_with( const string_t* self, const char* str );
```
* Check if the string end with `str`. Return `true` if it does. 
```c
bool str_end_with( const string_t* self, const char* str );
```
* Check if the string contains `str`. Return `true` if it does. 
```c
bool str_has( const string_t* self, const char* str );
```
* Check if the string is a decimal number. Return `true` if it is. 
```c
bool str_isdigit( const string_t* src );
```
* Check if the string is a hex number. Return `true` if it is. 
```c
bool str_isxdigit( const string_t* src );
```
* Check if the string is a floating number. Return `true` if it is.
```c
bool str_isfloat( const string_t* src );
```
* Read a file as utf-8 sequense to a string and return the string. Return `NULL` if failed to open file or failed to allocate memory. 
```c
string_t* str_from_file( const char* file_name );
```


#### Memory Manipulate Functions:
* clear the string, set the length to 0, may or may not reallocate memory. Return `true` on success, `false` on failure. 
```c
bool str_clear( string_t** string );
```
* free the old string, and return the new string. This function is just for convenient. 
```c
string_t* str_clear_to( string_t* old, string_t* new );
```
* reserve enough memory for string that has `length` ASCII characters or `length` bytes. If `length` is smaller than the current string length, nothing would happen, and the function still return `true` as it is a valid operation. The operation is normally handled by the library. And user shouldn't need to use `str_reserve` manually. Return `true` on success, `false` on allocation failure. 
```c
bool str_reserve( string_t* string, size_t length );
```

#### 