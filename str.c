#include "str.h"


#ifdef USE_GC
#include <gc/gc.h>
#define malloc( size ) GC_malloc( size )
#define realloc( ptr, size ) GC_realloc( ptr, size )
#define free( ptr ) GC_free( ptr )
#endif  //USE_GC


// struct str_cstr{ char* cstr; size_t length; };


static inline bool str_resize( string_t* string, size_t size )
{
    size_t cap;
    string->length = size;
    if ( size < 16 ) cap = 16;
    else cap = size + 1;
    if ( cap > string->capacity )
    {
        while ( string->capacity < cap )
        {
            if ( string->capacity <= 1024 )
            {
                string->capacity *= 2;
            }
            else
            {
                string->capacity += 512;
            }
        }
        string->cstr = realloc( string->cstr, sizeof ( char ) * string->capacity );
        if ( string->cstr == NULL ) return ( fputs( "[ERRO]: run out of memory", stderr ), false );
    }
    else
    {
        while ( cap > 1024 )
        {
            if ( cap + 512 < string->capacity )
            {
                string->capacity -= 512;
            }
            else break;
        }
        while ( cap <= 1024 )
        {
            if ( cap * 2 < string->capacity )
            {
                string->capacity /= 2;
            }
            else break;
        }
        string->cstr = realloc( string->cstr, sizeof ( char ) * string->capacity );
        if ( string->cstr == NULL ) return ( fputs( "[ERRO]: run out of memory", stderr ), false );
    }
    string->cstr[ string->length ] = 0;
    return true;
}


string_t str_new_string( char* src )
{
    string_t string = { .length = strlen(src), .capacity = 16 };
    if ( str_resize( &string, string.length ) )
    {
        memmove( string.cstr, src, string.length );
        string.cstr[ string.length ] = 0;
        return string;
    }
    return (string_t) { 0 };
}


string_t str_new_strings( char* src, ... )
{
    va_list ap, _ap;
    va_start( ap, src );
    va_copy( _ap, ap );
    size_t length = 0;
    for ( char* str = src; str != NULL; str = va_arg( ap, char* ) )
    {
        length += strlen(str);
    }
    va_end(ap);
    size_t index = 0;
    string_t result = { .length = length, .capacity = 16 };
    if ( str_resize( &result, length ) )
    {
        for ( char* str = src; str!= NULL; str = va_arg( _ap, char* ) )
        {
            strcpy( result.cstr + index, str );
            index += strlen( str );
        }
        va_end(_ap);
        return result;
    }
    va_end(_ap);
    return (string_t) { 0 };
}


void str_destroy_string( string_t* string )
{
    free( string->cstr );
    string->capacity = 0;
    string->length = 0;
    string->cstr = NULL;
}


void str_destroy_strings( string_t* string, ... )
{
    va_list ap;
    va_start( ap, string );
    for ( string_t* str = string; str != NULL; str = va_arg( ap, string_t* ) )
    {
        str_destroy_string( str );
    }
    va_end( ap );
}


void str_destroy_string_arr( string_t* str_arr )
{
    int index = 0;
    while ( str_arr[index].capacity )
    {
        str_destroy_string( &str_arr[index++] );
    }
    free( str_arr );
}


bool str_clear( string_t* string )
{
    return str_resize( string, 0 );
}


string_t str_append( string_t start, string_t end )
{
    size_t length = start.length + end.length;
    string_t result = { .capacity = 16  };
    if ( str_resize( &result, length ) )
    {
        memmove( result.cstr, start.cstr, start.length );
        memmove( result.cstr + start.length, end.cstr, end.length );
        result.cstr[ result.length ] = 0;
        return result;
    }
    return (string_t) { 0 };
}


string_t str_appends( string_t start, ... )
{
    va_list ap, _ap;
    va_start( ap, start );
    va_copy( _ap, ap );
    size_t length;
    for ( string_t str = start; str.cstr != NULL; str = va_arg( ap, string_t ) )
    {
        length += start.length;
    }
    va_end(ap);
    string_t result = { .length = length, .capacity = 16 };
    if ( str_resize( &result, length ) )
    {
        size_t index = 0;
        for ( string_t str = start; str.cstr != NULL; str = va_arg( _ap, string_t ) )
        {
            strcpy( result.cstr + index, str.cstr );
            index += str.length;
        }
        va_end(_ap);
        return result;
    }
    va_end(_ap);
    return (string_t) { 0 };
}


string_t str_append_cstr( string_t start, const char* end )
{
    size_t start_len = start.length;
    size_t end_len = strlen(end);
    string_t result = { .capacity = 16 };
    if ( str_resize( &result, start_len + end_len ) )
    {
        memmove( result.cstr, start.cstr, start_len );
        memmove( result.cstr, end, end_len );
        result.cstr[ result.length ] = 0;
        return result;
    }
    return (string_t) { 0 };
}


string_t* str_split( string_t src, const char* needle )
{
    size_t nlen = strlen( needle );
    char* str = malloc( sizeof (char) * ( src.length + 1 ) );
    strncpy( str, src.cstr, src.length + 1 );
    char* ptr = str;
    size_t cap = 16;
    string_t* tokens = malloc( sizeof ( string_t ) * cap );
    char* token;
    size_t index = 0;

    token = strstr( str, needle );
    while ( token != NULL )
    {
        *token = 0;
        tokens[index] = (string_t) { .length = strlen(str), .capacity = 16 };
        if ( !str_resize( &tokens[index], tokens[index].length ) ) return ( free(ptr), NULL );
        strcpy( tokens[index].cstr, str );
        str = token + nlen;
        index++;
        token = strstr( str, needle );
        if ( index + 2 == cap )
        {
            cap += 16;
            tokens = realloc( tokens, sizeof ( string_t ) * cap );
        }
    }
    tokens[index] = (string_t) { .length = strlen(str), .capacity = 16 };
    if ( !str_resize( &tokens[index], tokens[index].length ) ) return ( free(ptr), NULL );
    strncpy( tokens[index].cstr, str, tokens[index].length );
    tokens[index].cstr[ tokens[index].length ] = 0;
    index++;
    tokens[index] = (string_t) { 0 };
    free(ptr);
    return tokens;
}


void str_to_upper( string_t* string )
{
    for ( size_t i = 0; i < string->length; i++ )
    {
        string->cstr[i] = toupper( string->cstr[i] );
    }
}


void str_to_lower( string_t* string )
{
    for ( size_t i = 0; i < string->length; i++ )
    {
        string->cstr[i] = tolower( string->cstr[i] );
    }
}


string_t str_substr( string_t src, size_t start, size_t size )
{
    if ( start + size > src.length )
    {
        return ( fputs( "[ERRO]: substring out of bound!", stderr ), (string_t) { 0 } );
    }
    string_t substr = { .length = size, .capacity = 16 };
    str_resize( &substr, size );
    strncpy( substr.cstr, src.cstr + start, size );
    substr.cstr[ substr.length ] = 0;
    return substr;
}
