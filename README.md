# string
A string library that only depends on libc, with optional bdw-gc support to have a gc manage memory for you

define `USE_GC` when compiling `str.c` to use [`bdw-gc`](https://www.hboehm.info/gc/) to avoid calling any `str_destroy` functions. 

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

#### Getter:
* get the length of the string. 
```c
size_t str_strlen( const string_t* string );
```
* get the current capacity of the string. 
```c
size_t str_capacity( const string_t* string );
```
* get a `const char*` not modifiable c type string. Keep in mind that even though there're tricks in c language that you can do to get around and modify this string, it is not recommend to do so. The address of the returned string is exactly the same with the one managed by the library. 
```c
const char* str_cstr( const string_t* string );
```
* This is not a getter function. It returns a `malloc` allocated `wchar_t*` or `LPWSTR`( if on windows ). This function is caller free if the library is not build with `USE_GC` preprocessor. Please use `str_free` function to free this string in case that the wide string might be allocated by `GC_malloc`. 
```c
wchar_t* str_wstr( const string_t* string );
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