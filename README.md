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



### API:
***
#### Constructor:
* return a new string of type `str_t` or `string_t*`. 
```c
string_t* str_new_string( const char* src );
```
* return a new string of type `str_t` or `string_t*` with format string.
```c
string_t* str_new_format( const char* format, ... );
```
* return a new string of type `str_t` or `string_t*` from mutiple c type strings. The last element has to be `NULL`.
```c
string_t* str_new_strings( const char* src, ... );
```