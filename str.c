#include "str.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#define vsnprintf stbsp_vsnprintf

#ifdef USE_GC
#include <gc/gc.h>
#define malloc( size ) GC_malloc( size )
#define realloc( ptr, size ) GC_realloc( ptr, size )
#define free( ptr ) (void) ptr
#endif  //USE_GC


// user should never change the value of length or capacity in the code. 
struct string_t
{
    size_t length;
    size_t capacity;
    char cstr[1];
};



static inline bool str_resize( string_t** string, size_t size )
{
    size_t cap;
    size_t capacity;
    if ( *string == NULL )
    {
        *string = malloc( sizeof ( string_t ) );
        (*string)->capacity = 16;
        capacity = 0;
    }
    else
    {
        capacity = (*string)->capacity;
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
        if ( (*string)->capacity == capacity ) return true;
        (*string) = realloc( (*string), sizeof ( string_t ) + sizeof ( char ) * ( (*string)->capacity - 1 ) );
        if ( (*string) == NULL ) return ( fputs( "[ERRO]: run out of memory\n", stderr ), false );
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
        if ( (*string)->capacity == capacity ) return true;
        *string = realloc( *string, sizeof ( string_t ) + sizeof ( char ) * ( (*string)->capacity - 1 ) );
        if ( (*string) == NULL ) return ( fputs( "[ERRO]: run out of memory\n", stderr ), false );
    }
    (*string)->cstr[ (*string)->length ] = 0;
    return true;
}


string_t* str_new_string( const char* src )
{
    string_t* string = NULL;
    if ( str_resize( &string, strlen(src) ) )
    {
        strcpy( string->cstr, src );
        return string;
    }
    return NULL;
}


string_t* str_new_format( const char* format, ... )
{
    va_list ap, _ap;
    va_start( ap, format );
    va_copy( _ap, ap );
    size_t size = vsnprintf( NULL, 0, format, ap );
    va_end(ap);
    string_t* string = NULL;
    if ( str_resize( &string, size ) )
    {
        vsnprintf( string->cstr, string->capacity, format, _ap );
        va_end(_ap);
        return string;
    }
    return NULL;
}


string_t* str_new_strings( const char* src, ... )
{
    va_list ap, _ap;
    va_start( ap, src );
    va_copy( _ap, ap );
    size_t length = 0;
    for ( const char* str = src; str != NULL; str = va_arg( ap, char* ) )
    {
        length += strlen(str);
    }
    va_end(ap);
    size_t index = 0;
    string_t* result = NULL;
    if ( str_resize( &result, length ) )
    {
        for ( const char* str = src; str!= NULL; str = va_arg( _ap, char* ) )
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


string_t* str_new_string_arr( const char** src )
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


string_t* str_new_string_narr( const char** src, size_t size )
{
    size_t length = 0;
    for ( size_t i = 0; i < size; i++ )
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


void str_free( void* string )
{
    free( string );
}


void str_frees( string_t* string, ... )
{
    va_list ap;
    va_start( ap, string );
    for ( string_t* str = string; str != NULL; str = va_arg( ap, string_t* ) )
    {
        free( str );
    }
    va_end( ap );
}


void str_free_arr( string_t** str_arr )
{
    for ( size_t i = 0; str_arr[i] != NULL; i++ )
    {
        free( str_arr[i] );
    }
    free( str_arr );
}


size_t str_strlen( const string_t* string )
{
    return (size_t) string->length;
}


size_t str_capacity( const string_t* string )
{
    return (size_t) string->capacity;
}


const char* str_cstr( const string_t* string )
{
    return (const char*) string->cstr;
}


wchar_t* str_wstr( const string_t* string )
{
    wchar_t* res;
    size_t size = mbstowcs( NULL, string->cstr, string->capacity );
    if ( size != (size_t) -1 )
    {
        res = malloc( sizeof ( wchar_t ) * ( size + 1 ) );
        if ( res != NULL )
        {
            mbstowcs( res, string->cstr, string->capacity );
            return res;
        }
        return ( fputs( "[ERRO]: run out of memory\n", stderr ), NULL );
    }
    return ( fputs( "[ERRO]: invalid multibyte sequence\n", stderr ), NULL );
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


string_t* str_clear_to( string_t* old, string_t* new )
{
    free( old );
    return new;
}


bool str_reserve( string_t* string, size_t length )
{
    size_t size = string->length;
    if ( length <= size )
    {
        return true;
    }
    else
    {
        bool res = str_resize( &string, length );
        string->length = size;
        return res;
    }
}


string_t* str_appended( const string_t* start, const string_t* end )
{
    size_t length = start->length + end->length;
    string_t* result = NULL;
    if ( str_resize( &result, length ) )
    {
        strcpy( result->cstr, start->cstr );
        strcpy( result->cstr + start->length, end->cstr );
        return result;
    }
    return NULL;
}


string_t* str_append( string_t** start, const string_t* end )
{
    size_t length = (*start)->length;
    if ( str_resize( start, (*start)->length + end->length ) )
    {
        strcpy( (*start)->cstr + length, end->cstr );
        return *start;
    }
    return NULL;
}


string_t* str_appendeds( const string_t* start, ... )
{
    va_list ap, _ap;
    va_start( ap, start );
    va_copy( _ap, ap );
    size_t length = 0;
    for ( const string_t* str = start; str != NULL; str = va_arg( ap, string_t* ) )
    {
        length += str->length;
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


string_t* str_appends( string_t** start, ... )
{
    va_list ap, _ap;
    va_start( ap, start );
    va_copy( _ap, ap );
    size_t length = 0;
    for ( const string_t* str = *start; str != NULL; str = va_arg( ap, string_t* ) )
    {
        length += str->length;
    }
    va_end(ap);
    size_t index = (*start)->length;
    if ( str_resize( start, length ) )
    {
        for ( const string_t* str = va_arg( _ap, string_t* ); str != NULL; str = va_arg( _ap, string_t* ) )
        {
            strcpy( (*start)->cstr + index, str->cstr );
            index += str->length;
        }
        va_end(_ap);
        return *start;
    }
    va_end(_ap);
    return NULL;
}


string_t* str_appended_cstr( const string_t* start, const char* end )
{
    size_t start_len = start->length;
    size_t end_len = strlen(end);
    string_t* result = NULL;
    if ( str_resize( &result, start_len + end_len ) )
    {
        strcpy( result->cstr, start->cstr );
        strcpy( result->cstr + start_len, end );
        result->cstr[ result->length ] = 0;
        return result;
    }
    return NULL;
}


string_t* str_append_cstr( string_t** start, const char* end )
{
    size_t start_len = (*start)->length;
    size_t end_len = strlen(end);
    if ( str_resize( start, (*start)->length + end_len ) )
    {
        strcpy( (*start)->cstr + start_len, end );
        return *start;
    }
    return NULL;
}


string_t* str_appended_cstrs( const string_t* start, ... )
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


string_t* str_append_cstrs( string_t** start, ... )
{
    va_list ap, _ap;
    va_start( ap, start );
    va_copy( _ap, ap );
    size_t length = 0;
    for ( const char* string = (*start)->cstr; string != NULL; string = va_arg( ap, char* ) )
    {
        length += strlen( string );
    }
    va_end(ap);
    size_t index = (*start)->length;
    if ( str_resize( start, length ) )
    {
        for ( const char* string = va_arg( _ap, char* ); string != NULL; string = va_arg( _ap, char* ) )
        {
            strcpy( (*start)->cstr + index, string );
            index += strlen( string );
        }
        va_end( _ap );
        return *start;
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
        return ( fputs( "[ERRO]: index out of bound\n", stderr ), NULL );
    }
    string_t* substr = NULL;
    if ( str_resize( &substr, size ) )
    {
        strncpy( substr->cstr, src->cstr + start, size );
        substr->cstr[ substr->length ] = 0;
        return substr;
    }
    return NULL;
}


string_t* str_strdup( const string_t* src )
{
    string_t* result = malloc( sizeof ( string_t ) + sizeof ( char ) * ( src->capacity - 1 ) );
    if ( result != NULL )
    {
        memcpy( result, src, sizeof ( string_t ) + sizeof ( char ) * src->length );
        return result;
    }
    return ( fputs( "[ERRO]: run out of memory\n", stderr ), NULL );
}


string_t* str_inserted( const string_t* src, size_t index, const char* in )
{
    if ( index > src->length )
    {
        return ( fputs( "[ERRO]: index out of bound\n", stderr ), NULL );
    }
    size_t size = strlen(in);
    string_t* new_str = NULL;
    if ( str_resize( &new_str, src->length + size ) )
    {
        memmove( new_str->cstr, src->cstr, index );
        memmove( new_str->cstr + index, in, size );
        memmove( new_str->cstr + index + size, src->cstr + index, src->length - index + 1 );
        return new_str;
    }
    return NULL;
}


string_t* str_insert( string_t** src, size_t index, const char* in )
{
    if ( index > (*src)->length )
    {
        return ( fputs( "[ERRO]: index out of bound\n", stderr ), NULL );
    }
    size_t size = strlen(in);
    size_t len  = (*src)->length;
    if ( str_resize( src, len + size ) )
    {
        memmove( (*src)->cstr + index + size, (*src)->cstr + index, len - index + 1 );
        memmove( (*src)->cstr + index, in, size );
        return *src;
    }
    return NULL;
}


string_t* str_replaced( const string_t* src, const char* old_val, const char* new_val )
{
    // creating string arrays
    size_t old_len = strlen( old_val );
    char* str = malloc( sizeof (char) * ( src->length + 1 ) );
    strncpy( str, src->cstr, src->length + 1 );
    char* ptr = str;
    size_t cap = 16;
    char** tokens = malloc( sizeof (char*) * cap );
    size_t* part_len = malloc( sizeof (size_t) * cap );
    char* token;
    size_t index = 0;
    size_t counter = 0;
    token = strstr( str, old_val );
    while ( token != NULL )
    {
        *token = 0;
        part_len[index] = strlen(str);
        tokens[index] = malloc( sizeof (char) * ( part_len[index] + 1 ) );
        strcpy( tokens[index], str );
        str = token + old_len;
        index++;
        counter++;
        token = strstr( str, old_val );
        if ( index + 2 == cap )
        {
            cap += 16;
            tokens = realloc( tokens, sizeof ( char* ) * cap );
            part_len = realloc( part_len, sizeof (size_t) * cap );
        }
    }
    part_len[index] = strlen(str);
    tokens[index] = malloc( sizeof (char) * ( part_len[index] + 1 ) );
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
        if ( counter != i )
        {
            length += new_len;
        }
    }
    index = 0;
    string_t* result = NULL;
    if ( str_resize( &result, length ) )
    {
        for ( size_t i = 0; tokens[i] != NULL; i++ )
        {
            strcpy( result->cstr + index, tokens[i] );
            index += part_len[i];
            if ( counter != i )
            {
                strcpy( result->cstr + index, new_val );
                index += new_len;
            }
            free( tokens[i] );
        }
        free( tokens );
        free( part_len );
        return result;
    }
    for ( size_t i = 0; tokens[i] != NULL; i++ )
    {
        free( tokens[i] );
    }
    free( tokens );
    free( part_len );
    return NULL;
}


// built in compare function for str_sort and str_sorted
static inline int str_cmp_l( const void* arg1, const void* arg2 )
{
    string_t* str1 = *(string_t**) arg1;
    string_t* str2 = *(string_t**) arg2;
    return ( str1->length - str2->length );
}
static inline int str_cmp_a( const void* arg1, const void* arg2 )
{
    string_t* str1 = *(string_t**) arg1;
    string_t* str2 = *(string_t**) arg2;
    for ( size_t i = 0; i <= str1->length && i <= str2->length; i++ )
    {
        if ( str1->cstr[i] != str2->cstr[i] )
        {
            return str1->cstr[i] - str2->cstr[i];
        }
    }
    return 0;
}
static inline int str_cmp_la( const void* arg1, const void* arg2 )
{
    string_t* str1 = *(string_t**) arg1;
    string_t* str2 = *(string_t**) arg2;
    if ( str1->length != str2->length )
    {
        return str1->length - str2->length;
    }
    else
    {
        for ( size_t i = 0; i < str1->length; i++ )
        {
            if ( str1->cstr[i] != str2->cstr[i] )
            {
                return str1->cstr[i] - str2->cstr[i];
            }
        }
    }
    return 0;
}
static inline int str_cmp_ai( const void* arg1, const void* arg2 )
{
    string_t* str1 = *(string_t**) arg1;
    string_t* str2 = *(string_t**) arg2;
    for ( size_t i = 0; i <= str1->length && i <= str2->length; i++ )
    {
        if ( toupper( str1->cstr[i] ) != toupper( str2->cstr[i] ) )
        {
            return toupper( str1->cstr[i] ) - toupper( str2->cstr[i] );
        }
    }
    return 0;
}
static inline int str_cmp_lai( const void* arg1, const void* arg2 )
{
    string_t* str1 = *(string_t**) arg1;
    string_t* str2 = *(string_t**) arg2;
    if ( str1->length != str2->length )
    {
        return str1->length - str2->length;
    }
    else
    {
        for ( size_t i = 0; i < str1->length; i++ )
        {
            if ( toupper( str1->cstr[i] ) != toupper( str2->cstr[i] ) )
            {
                return toupper( str1->cstr[i] ) - toupper( str2->cstr[i] );
            }
        }
    }
    return 0;
}


string_t** str_sort( string_t** src, size_t size, const char* mode, ... )
{
    enum modes
    {
        c, i, l, a, la, modes_max
    };
    bool option[ modes_max ] = { 0 };
    for ( const char *ch = mode; *ch != 0; ch++ )
    {
        if ( *ch == 'c' )
        {
            option[c] = true;
            break;
        }
        else if ( *ch == 'i' )
        {
            option[i] = true;
        }
        else if ( *ch == 'l' )
        {
            if ( option[a] == true )
            {
                continue;
            }
            else
            {
                option[l] = true;
            }
        }
        else if ( *ch == 'a' )
        {
            if ( option[l] == true )
            {
                option[l] = false;
                option[la] = true;
            }
            else
            {
                option[a] = true;
            }
        }
        else
        {
            return ( fputs( "[ERRO]: invalid mode\n", stderr ), NULL );
        }
    }
    if ( size == 0 )
    {
        for ( ; src[size]!= NULL; size++ );
    }
    int (*compar)(const void *, const void *);
    if ( option[c] == true )
    {
        va_list ap;
        va_start( ap, mode );
        compar = va_arg( ap, int (*)(const void *, const void *) );
        va_end(ap);
    }
    else if ( option[l] == true )
    {
        compar = str_cmp_l;
    }
    else if ( option[a] == true && option[i] == false )
    {
        compar = str_cmp_a;
    }
    else if ( option[a] == true && option[i] == true )
    {
        compar = str_cmp_ai;
    }
    else if ( option[la] == true )
    {
        compar = str_cmp_la;
    }
    else if ( option[la] == true && option[i] == true )
    {
        compar = str_cmp_lai;
    }
    else
    {
        return ( fputs( "[ERRO]: invalid mode\n", stderr ), NULL );
    }
    qsort( src, size, sizeof (string_t*), compar );
    return src;
}


string_t** str_sorted( string_t** src, size_t size, const char* mode, ... )
{
    enum modes
    {
        c, i, l, a, la, modes_max
    };
    bool option[ modes_max ] = { 0 };
    for ( const char *ch = mode; *ch != 0; ch++ )
    {
        if ( *ch == 'c' )
        {
            option[c] = true;
            break;
        }
        else if ( *ch == 'i' )
        {
            option[i] = true;
        }
        else if ( *ch == 'l' )
        {
            if ( option[a] == true )
            {
                continue;
            }
            else
            {
                option[l] = true;
            }
        }
        else if ( *ch == 'a' )
        {
            if ( option[l] == true )
            {
                option[l] = false;
                option[la] = true;
            }
            else
            {
                option[a] = true;
            }
        }
        else
        {
            return ( fputs( "[ERRO]: invalid mode\n", stderr ), NULL );
        }
    }
    if ( size == 0 )
    {
        for ( ; src[size]!= NULL; size++ );
    }
    int (*compar)(const void *, const void *);
    if ( option[c] == true )
    {
        va_list ap;
        va_start( ap, mode );
        compar = va_arg( ap, int (*)(const void *, const void *) );
        va_end(ap);
    }
    else if ( option[l] == true )
    {
        compar = str_cmp_l;
    }
    else if ( option[a] == true && option[i] == false )
    {
        compar = str_cmp_a;
    }
    else if ( option[a] == true && option[i] == true )
    {
        compar = str_cmp_ai;
    }
    else if ( option[la] == true )
    {
        compar = str_cmp_la;
    }
    else if ( option[la] == true && option[i] == true )
    {
        compar = str_cmp_lai;
    }
    else
    {
        return ( fputs( "[ERRO]: invalid mode\n", stderr ), NULL );
    }
    string_t** result = malloc( sizeof ( string_t* ) * ( size + 1 ) );
    for ( size_t i = 0; i < size; i++ )
    {
        result[i] = str_strdup( src[i] );
    }
    result[size] = NULL;
    qsort( result, size, sizeof (string_t*), compar );
    return result;
}


int str_strcmp( const string_t* str1, const string_t* str2 )
{
    for ( size_t i = 0; i <= str1->length && i <= str2->length; i++ )
    {
        if ( str1->cstr[i] != str2->cstr[i] )
        {
            return str1->cstr[i] - str2->cstr[i];
        }
    }
    return 0;
}


bool str_start_with( const string_t* self, const char* str )
{
    size_t size = strlen( str );
    if ( self->length < size )
    {
        return false;
    }
    return ( memcmp( self->cstr, str, size ) == 0 ? true: false );
}


bool str_end_with( const string_t* self, const char* str )
{
    size_t size = strlen( str );
    if ( self->length < size )
    {
        return false;
    }
    return ( memcmp( self->cstr + ( self->length - size ), str, size ) == 0 ? true : false );
}


bool str_has( const string_t* self, const char* str )
{
    if ( strstr( self->cstr, str ) )
    {
        return true;
    }
    return false;
}


char str_char_at( string_t* self, size_t index, char new_val )
{
    if ( index >= self->length )
    {
        return ( fputs( "[ERRO]: index out of bounds\n", stderr ), 0 );
    }
    if ( new_val != 0 )
    {
        self->cstr[index] = new_val;
    }
    return self->cstr[index];
}



