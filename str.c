#include "str.h"


#ifdef USE_GC
#include <gc/gc.h>
#define malloc( size ) GC_malloc( size )
#define realloc( ptr, size ) GC_realloc( ptr, size )
#define free( ptr ) (void) ptr
#endif  //USE_GC



static inline bool str_resize( string_t** string, size_t size )
{
    size_t cap;
    if ( *string == NULL )
    {
        *string = malloc( sizeof ( string_t ) );
        (*string)->capacity = 16;
    }
    (*string)->length = size;
    if ( size < 16 ) cap = 16;
    else cap = size + 1;
    if ( cap > (*string)->capacity )
    {
        while ( (*string)->capacity < cap )
        {
            if ( (*string)->capacity <= 1024 )
            {
                (*string)->capacity *= 2;
            }
            else
            {
                (*string)->capacity += 512;
            }
        }
        (*string) = realloc( (*string), sizeof ( string_t ) + sizeof ( char ) * ( (*string)->capacity - 1 ) );
        if ( (*string) == NULL ) return ( fputs( "[ERRO]: run out of memory", stderr ), false );
    }
    else
    {
        while ( cap > 1024 )
        {
            if ( cap + 512 < (*string)->capacity )
            {
                (*string)->capacity -= 512;
            }
            else break;
        }
        while ( cap <= 1024 )
        {
            if ( cap * 2 < (*string)->capacity )
            {
                (*string)->capacity /= 2;
            }
            else break;
        }
        *string = realloc( *string, sizeof ( string_t ) + sizeof ( char ) * ( (*string)->capacity - 1 ) );
        if ( (*string) == NULL ) return ( fputs( "[ERRO]: run out of memory", stderr ), false );
    }
    (*string)->cstr[ (*string)->length ] = 0;
    return true;
}


string_t* str_new_string( char* src )
{
    string_t* string = NULL;
    if ( str_resize( &string, strlen(src) ) )
    {
        strcpy( string->cstr, src );
        return string;
    }
    return NULL;
}


string_t* str_new_strings( char* src, ... )
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
    string_t* result = NULL;
    if ( str_resize( &result, length ) )
    {
        for ( char* str = src; str!= NULL; str = va_arg( _ap, char* ) )
        {
            strcpy( result->cstr + index, str );
            index += strlen( str );
        }
        va_end(_ap);
        return result;
    }
    va_end(_ap);
    return NULL;
}


string_t* str_new_string_arr( char** src )
{
    size_t length = 0;
    for ( size_t i = 0; src[i] != NULL; i++ )
    {
        length += strlen(src[i]);
    }
    size_t index = 0;
    string_t* result = NULL;
    if ( str_resize( &result, length ) )
    {
        for ( size_t i = 0; src[i] != NULL; i++ )
        {
            strcpy( result->cstr + index, src[i] );
            index += strlen( src[i] );
        }
        return result;
    }
    return NULL;
}


void str_destroy_string( string_t* string )
{
    free( string );
}


void str_destroy_strings( string_t* string, ... )
{
    va_list ap;
    va_start( ap, string );
    for ( string_t* str = string; str != NULL; str = va_arg( ap, string_t* ) )
    {
        free( str );
    }
    va_end( ap );
}


void str_destroy_string_arr( string_t** str_arr )
{
    for ( size_t i = 0; str_arr[i] != NULL; i++ )
    {
        free( str_arr[i] );
    }
    free( str_arr );
}


bool str_clear( string_t* string )
{
    if ( str_resize( &string, 0 ) )
    {
        memset( string->cstr, 0, sizeof (char) * 16 );
        return true;
    }
    return false;
}


string_t* str_append( const string_t* start, const string_t* end )
{
    size_t length = start->length + end->length;
    string_t* result = NULL;
    if ( str_resize( &result, length ) )
    {
        memmove( result->cstr, start->cstr, start->length );
        memmove( result->cstr + start->length, end->cstr, end->length );
        result->cstr[ result->length ] = 0;
        return result;
    }
    return NULL;
}


string_t* str_appends( const string_t* start, ... )
{
    va_list ap, _ap;
    va_start( ap, start );
    va_copy( _ap, ap );
    size_t length;
    for ( const string_t* str = start; str != NULL; str = va_arg( ap, string_t* ) )
    {
        length += start->length;
    }
    va_end(ap);
    string_t* result = NULL;
    if ( str_resize( &result, length ) )
    {
        size_t index = 0;
        for ( const string_t* str = start; str != NULL; str = va_arg( _ap, string_t* ) )
        {
            strcpy( result->cstr + index, str->cstr );
            index += str->length;
        }
        va_end(_ap);
        return result;
    }
    va_end(_ap);
    return NULL;
}


string_t* str_append_cstr( const string_t* start, const char* end )
{
    size_t start_len = start->length;
    size_t end_len = strlen(end);
    string_t* result = NULL;
    if ( str_resize( &result, start_len + end_len ) )
    {
        memmove( result->cstr, start->cstr, start_len );
        memmove( result->cstr, end, end_len );
        result->cstr[ result->length ] = 0;
        return result;
    }
    return NULL;
}


string_t* str_append_cstrs( const string_t* start, ... )
{
    va_list ap, _ap;
    va_start( ap, start );
    va_copy( _ap, ap );
    size_t length = 0;
    for ( const char* string = start->cstr; string != NULL; string = va_arg( ap, char* ) )
    {
        length += strlen( string );
    }
    va_end(ap);
    string_t* result = NULL;
    if ( str_resize( &result, length ) )
    {
        size_t index = 0;
        for ( const char* string = start->cstr; string != NULL; string = va_arg( _ap, char* ) )
        {
            strcpy( result->cstr + index, string );
            index += strlen( string );
        }
        va_end( _ap );
        return result;
    }
    va_end( _ap );
    return NULL;
}


string_t** str_split( const string_t* src, const char* needle )
{
    size_t nlen = strlen( needle );
    char* str = malloc( sizeof (char) * ( src->length + 1 ) );
    strncpy( str, src->cstr, src->length + 1 );
    char* ptr = str;
    size_t cap = 16;
    string_t** tokens = malloc( sizeof ( string_t* ) * cap );
    char* token;
    size_t index = 0;

    token = strstr( str, needle );
    while ( token != NULL )
    {
        *token = 0;
        tokens[index] = NULL;
        if ( !str_resize( &tokens[index], strlen(str) ) ) return ( free(ptr), NULL );
        strcpy( tokens[index]->cstr, str );
        str = token + nlen;
        index++;
        token = strstr( str, needle );
        if ( index + 2 == cap )
        {
            cap += 16;
            tokens = realloc( tokens, sizeof ( string_t* ) * cap );
        }
    }
    tokens[index] = NULL;
    if ( !str_resize( &tokens[index], strlen(str) ) ) return ( free(ptr), NULL );
    strcpy( tokens[index]->cstr, str );
    index++;
    tokens[index] = NULL;
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


string_t* str_substr( const string_t* src, size_t start, size_t size )
{
    if ( start + size > src->length )
    {
        return ( fputs( "[ERRO]: substring out of bound!", stderr ), NULL );
    }
    string_t* substr = NULL;
    str_resize( &substr, size );
    strncpy( substr->cstr, src->cstr + start, size );
    substr->cstr[ substr->length ] = 0;
    return substr;
}


string_t* str_replace( const string_t* src, const char* old_val, const char* new_val )
{
    // creating string arrays
    size_t old_len = strlen( old_val );
    char* str = malloc( sizeof (char) * ( src->length + 1 ) );
    strncpy( str, src->cstr, src->length + 1 );
    char* ptr = str;
    size_t cap = 16;
    char** tokens = malloc( sizeof (char*) * cap );
    char* token;
    size_t index = 0;
    size_t counter = 0;
    token = strstr( str, old_val );
    while ( token != NULL )
    {
        *token = 0;
        tokens[index] = malloc( sizeof (char) * ( strlen(str) + 1 ) );
        strcpy( tokens[index], str );
        str = token + old_len;
        index++;
        counter++;
        token = strstr( str, old_val );
        if ( index + 2 == cap )
        {
            cap += 16;
            tokens = realloc( tokens, sizeof ( char* ) * cap );
        }
    }
    tokens[index] = malloc( sizeof (char) * ( strlen(str) + 1 ) );
    strcpy( tokens[index], str );
    index++;
    tokens[index] = NULL;
    free(ptr);

    // creating new string
    size_t length = 0;
    size_t new_len = strlen( new_val );
    for ( size_t i = 0; tokens[i] != NULL; i++ )
    {
        length += strlen(tokens[i]);
        length += new_len;
    }
    index = 0;
    string_t* result = NULL;
    if ( str_resize( &result, length ) )
    {
        for ( size_t i = 0; tokens[i] != NULL; i++ )
        {
            strcpy( result->cstr + index, tokens[i] );
            index += strlen( tokens[i] );
            if ( counter != i )
            {
                strcpy( result->cstr + index, new_val );
                index += new_len;
            }
            free( tokens[i] );
        }
        free( tokens );
        return result;
    }
    for ( size_t i = 0; tokens[i] != NULL; i++ )
    {
        free( tokens[i] );
    }
    free( tokens );
    return NULL;
}
