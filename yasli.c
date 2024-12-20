#include "yasli.h"

#ifdef _MSC_VER
#    define _CRT_SECURE_NO_WARNINGS 1
#endif  // _MSC_VER

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"
#define vsnprintf stbsp_vsnprintf

#ifdef YASLI_GC
#    include <gc/gc.h>
#    define malloc(size)       GC_malloc(size)
#    define realloc(ptr, size) GC_realloc(ptr, size)
#    define free(ptr)          (void)ptr
#endif  // YASLI_GC

// user should never change the value of length or capacity in the code.
struct string_t {
    size_t length;
    size_t capacity;
    char cstr[1];
};

static inline bool str_resize(string_t **string, size_t size) {
    size_t cap;
    size_t capacity;
    if (*string == NULL) {
        *string = malloc(sizeof(string_t));
        (*string)->capacity = 16;
        capacity = 0;
    } else {
        capacity = (*string)->capacity;
    }
    (*string)->length = size;
    if (size < 16)
        cap = 16;
    else
        cap = size + 1;
    if (cap > (*string)->capacity) {
        while ((*string)->capacity < cap) {
            if ((*string)->capacity <= 1024) {
                (*string)->capacity *= 2;
            } else {
                (*string)->capacity += 512;
            }
        }
        if ((*string)->capacity == capacity) return true;
        (*string) = realloc((*string), sizeof(string_t) + sizeof(char) * ((*string)->capacity - 1));
        if ((*string) == NULL) {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: out of memory\n", stderr), false);
#else
            return false;
#endif
        }

    } else {
        while (cap > 1024) {
            if (cap + 512 < (*string)->capacity) {
                (*string)->capacity -= 512;
            } else
                break;
        }
        while (cap <= 1024) {
            if (cap * 2 <= (*string)->capacity) {
                (*string)->capacity /= 2;
            } else
                break;
        }
        if ((*string)->capacity == capacity) return true;
        *string = realloc(*string, sizeof(string_t) + sizeof(char) * ((*string)->capacity - 1));
        if ((*string) == NULL) {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: out of memory\n", stderr), false);
#else
            return false;
#endif
        }
    }
    (*string)->cstr[(*string)->length] = 0;
    return true;
}

string_t *str_new_string(const char *src) {
    string_t *string = NULL;
    if (str_resize(&string, strlen(src))) {
        memmove(string->cstr, src, string->length);
        string->cstr[string->length] = 0;
        return string;
    }
    return NULL;
}

string_t *str_new_format(const char *format, ...) {
    va_list ap, _ap;
    va_start(ap, format);
    va_copy(_ap, ap);
    size_t size = vsnprintf(NULL, 0, format, ap);
    va_end(ap);
    string_t *string = NULL;
    if (str_resize(&string, size)) {
        vsnprintf(string->cstr, string->capacity, format, _ap);
        va_end(_ap);
        return string;
    }
    return NULL;
}

string_t *str_new_strings(const char *src, ...) {
    va_list ap, _ap;
    va_start(ap, src);
    va_copy(_ap, ap);
    size_t length = 0;
    for (const char *str = src; str != NULL; str = va_arg(ap, char *)) {
        length += strlen(str);
    }
    va_end(ap);
    size_t index = 0;
    string_t *result = NULL;
    if (str_resize(&result, length)) {
        for (const char *str = src; str != NULL; str = va_arg(_ap, char *)) {
            strcpy(result->cstr + index, str);
            index += strlen(str);
        }
        va_end(_ap);
        return result;
    }
    va_end(_ap);
    return NULL;
}

string_t *str_new_string_arr(const char **src) {
    size_t length = 0;
    for (size_t i = 0; src[i] != NULL; i++) {
        length += strlen(src[i]);
    }
    size_t index = 0;
    string_t *result = NULL;
    if (str_resize(&result, length)) {
        for (size_t i = 0; src[i] != NULL; i++) {
            strcpy(result->cstr + index, src[i]);
            index += strlen(src[i]);
        }
        return result;
    }
    return NULL;
}

string_t *str_new_string_narr(const char **src, size_t size) {
    size_t length = 0;
    for (size_t i = 0; i < size; i++) {
        length += strlen(src[i]);
    }
    size_t index = 0;
    string_t *result = NULL;
    if (str_resize(&result, length)) {
        for (size_t i = 0; src[i] != NULL; i++) {
            strcpy(result->cstr + index, src[i]);
            index += strlen(src[i]);
        }
        return result;
    }
    return NULL;
}

void str_free(void *string) {
    free(string);
}

void str_frees(string_t *string, ...) {
    va_list ap;
    va_start(ap, string);
    for (string_t *str = string; str != NULL; str = va_arg(ap, string_t *)) {
        free(str);
    }
    va_end(ap);
}

void str_free_arr(string_t **str_arr) {
    for (size_t i = 0; str_arr[i] != NULL; i++) {
        free(str_arr[i]);
    }
    free(str_arr);
}

size_t str_strlen(const string_t *string) {
    return (size_t)string->length;
}

size_t str_utf8_strlen(const string_t *string) {
    if (string == NULL) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: input is NULL\n", stderr), 0);
#else
        return 0;
#endif
    }
    size_t current_char = 0;
    size_t length = 0;
    const unsigned char *ptr = (const unsigned char *)string->cstr;

    while (length <= string->length) {
        if (*ptr == 0) {
            return current_char;
        } else if ((*ptr & 0x80) == 0) {
            ptr++;
            length++;
        } else if ((*ptr & 0xE0) == 0xC0) {
            ptr += 2;
            length += 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            ptr += 3;
            length += 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            ptr += 4;
            length += 4;
        } else {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid utf-8 sequence\n", stderr), 0);
#else
            return 0;
#endif
        }
        current_char++;
    }
    return current_char;
}

size_t str_capacity(const string_t *string) {
    return (size_t)string->capacity;
}

const char *str_cstr(const string_t *string) {
    return (const char *)string->cstr;
}

const wchar_t *str_wstr(const string_t *string) {
    wchar_t *res;
    size_t size = mbstowcs(NULL, string->cstr, string->capacity);
    if (size != (size_t)-1) {
        res = malloc(sizeof(wchar_t) * (size + 1));
        if (res != NULL) {
            mbstowcs(res, string->cstr, string->capacity);
            return (const wchar_t *)res;
        }
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: run out of memory\n", stderr), NULL);
#else
        return NULL;
#endif
    }
#ifdef YASLI_DEBUG
    return (fputs("[ERRO]: invalid multibyte sequence\n", stderr), NULL);
#else
    return NULL;
#endif
}

bool str_clear(string_t **string) {
    if (str_resize(string, 0)) {
        memset((*string)->cstr, 0, sizeof(char) * 16);
        return true;
    }
    return false;
}

string_t *str_clear_to(string_t *old, string_t *new) {
    free(old);
    return new;
}

bool str_reserve(string_t *string, size_t length) {
    size_t size = string->length;
    if (length <= size) {
        return true;
    } else {
        bool res = str_resize(&string, length);
        string->length = size;
        return res;
    }
}

string_t *str_appended(const string_t *start, const string_t *end) {
    size_t length = start->length + end->length;
    string_t *result = NULL;
    if (str_resize(&result, length)) {
        strcpy(result->cstr, start->cstr);
        strcpy(result->cstr + start->length, end->cstr);
        return result;
    }
    return NULL;
}

string_t *str_append(string_t **start, const string_t *end) {
    size_t length = (*start)->length;
    if (str_resize(start, (*start)->length + end->length)) {
        strcpy((*start)->cstr + length, end->cstr);
        return *start;
    }
    return NULL;
}

string_t *str_appendeds(const string_t *start, ...) {
    va_list ap, _ap;
    va_start(ap, start);
    va_copy(_ap, ap);
    size_t length = 0;
    for (const string_t *str = start; str != NULL; str = va_arg(ap, string_t *)) {
        length += str->length;
    }
    va_end(ap);
    string_t *result = NULL;
    if (str_resize(&result, length)) {
        size_t index = 0;
        for (const string_t *str = start; str != NULL; str = va_arg(_ap, string_t *)) {
            strcpy(result->cstr + index, str->cstr);
            index += str->length;
        }
        va_end(_ap);
        return result;
    }
    va_end(_ap);
    return NULL;
}

string_t *str_appends(string_t **start, ...) {
    va_list ap, _ap;
    va_start(ap, start);
    va_copy(_ap, ap);
    size_t length = 0;
    for (const string_t *str = *start; str != NULL; str = va_arg(ap, string_t *)) {
        length += str->length;
    }
    va_end(ap);
    size_t index = (*start)->length;
    if (str_resize(start, length)) {
        for (const string_t *str = va_arg(_ap, string_t *); str != NULL; str = va_arg(_ap, string_t *)) {
            strcpy((*start)->cstr + index, str->cstr);
            index += str->length;
        }
        va_end(_ap);
        return *start;
    }
    va_end(_ap);
    return NULL;
}

string_t *str_appended_cstr(const string_t *start, const char *end) {
    size_t start_len = start->length;
    size_t end_len = strlen(end);
    string_t *result = NULL;
    if (str_resize(&result, start_len + end_len)) {
        strcpy(result->cstr, start->cstr);
        strcpy(result->cstr + start_len, end);
        result->cstr[result->length] = 0;
        return result;
    }
    return NULL;
}

string_t *str_append_cstr(string_t **start, const char *end) {
    size_t start_len = (*start)->length;
    size_t end_len = strlen(end);
    if (str_resize(start, (*start)->length + end_len)) {
        strcpy((*start)->cstr + start_len, end);
        return *start;
    }
    return NULL;
}

string_t *str_appended_cstrs(const string_t *start, ...) {
    va_list ap, _ap;
    va_start(ap, start);
    va_copy(_ap, ap);
    size_t length = 0;
    for (const char *string = start->cstr; string != NULL; string = va_arg(ap, char *)) {
        length += strlen(string);
    }
    va_end(ap);
    string_t *result = NULL;
    if (str_resize(&result, length)) {
        size_t index = 0;
        for (const char *string = start->cstr; string != NULL; string = va_arg(_ap, char *)) {
            strcpy(result->cstr + index, string);
            index += strlen(string);
        }
        va_end(_ap);
        return result;
    }
    va_end(_ap);
    return NULL;
}

string_t *str_append_cstrs(string_t **start, ...) {
    va_list ap, _ap;
    va_start(ap, start);
    va_copy(_ap, ap);
    size_t length = 0;
    for (const char *string = (*start)->cstr; string != NULL; string = va_arg(ap, char *)) {
        length += strlen(string);
    }
    va_end(ap);
    size_t index = (*start)->length;
    if (str_resize(start, length)) {
        for (const char *string = va_arg(_ap, char *); string != NULL; string = va_arg(_ap, char *)) {
            strcpy((*start)->cstr + index, string);
            index += strlen(string);
        }
        va_end(_ap);
        return *start;
    }
    va_end(_ap);
    return NULL;
}

string_t **str_split(const string_t *src, const char *needle) {
    size_t nlen = strlen(needle);
    char *str = malloc(sizeof(char) * (src->length + 1));
    strncpy(str, src->cstr, src->length + 1);
    char *ptr = str;
    size_t cap = 16;
    string_t **tokens = malloc(sizeof(string_t *) * cap);
    char *token;
    size_t index = 0;

    token = strstr(str, needle);
    while (token != NULL) {
        *token = 0;
        tokens[index] = NULL;
        if (!str_resize(&tokens[index], strlen(str))) return (free(ptr), NULL);
        strcpy(tokens[index]->cstr, str);
        str = token + nlen;
        index++;
        token = strstr(str, needle);
        if (index + 2 == cap) {
            cap += 16;
            tokens = realloc(tokens, sizeof(string_t *) * cap);
        }
    }
    tokens[index] = NULL;
    if (!str_resize(&tokens[index], strlen(str))) return (free(ptr), NULL);
    strcpy(tokens[index]->cstr, str);
    index++;
    tokens[index] = NULL;
    free(ptr);
    return tokens;
}

void str_to_upper(string_t *string) {
    for (size_t i = 0; i < string->length; i++) {
        string->cstr[i] = toupper(string->cstr[i]);
    }
}

void str_to_lower(string_t *string) {
    for (size_t i = 0; i < string->length; i++) {
        string->cstr[i] = tolower(string->cstr[i]);
    }
}

string_t *str_substr(const string_t *src, size_t start, size_t size) {
    if (start + size > src->length) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bound\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    string_t *substr = NULL;
    if (str_resize(&substr, size)) {
        strncpy(substr->cstr, src->cstr + start, size);
        substr->cstr[substr->length] = 0;
        return substr;
    }
    return NULL;
}

string_t *str_utf8_substr(const string_t *src, size_t start, size_t size) {
    if (start + size > src->length) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t current_char = 0;
    const unsigned char *ptr = (const unsigned char *)src->cstr;
    while (current_char < start) {
        if (*ptr == 0) {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
            return NULL;
#endif
        } else if ((*ptr & 0x80) == 0) {
            ptr++;
        } else if ((*ptr & 0xE0) == 0xC0) {
            ptr += 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            ptr += 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            ptr += 4;
        } else {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid utf-8 sequence\n", stderr), NULL);
#else
            return NULL;
#endif
        }
        current_char++;
    }
    current_char = 0;

    const unsigned char *str = ptr;
    size_t total_size = 0;
    while (current_char < size) {
        if (*str == 0) {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
            return NULL;
#endif
        } else if ((*str & 0x80) == 0) {
            str++;
            total_size++;
        } else if ((*str & 0xE0) == 0xC0) {
            str += 2;
            total_size += 2;
        } else if ((*str & 0xF0) == 0xE0) {
            str += 3;
            total_size += 3;
        } else if ((*str & 0xF8) == 0xF0) {
            str += 4;
            total_size += 4;
        } else {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid utf-8 sequence\n", stderr), NULL);
#else
            return NULL;
#endif
        }
        current_char++;
    }

    // create string
    string_t *result = NULL;
    if (str_resize(&result, total_size)) {
        memcpy(result->cstr, ptr, total_size);
        return result;
    }
    return NULL;
}

string_t *str_strdup(const string_t *src) {
    string_t *result = malloc(sizeof(string_t) + sizeof(char) * (src->capacity - 1));
    if (result != NULL) {
        memcpy(result, src, sizeof(string_t) + sizeof(char) * src->length);
        return result;
    }
#ifdef YASLI_DEBUG
    return (fputs("[ERRO]: run out of memory\n", stderr), NULL);
#else
    return NULL;
#endif
}

string_t *str_inserted_cstr(const string_t *src, size_t index, const char *in) {
    if (index > src->length) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bound\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t size = strlen(in);
    string_t *new_str = NULL;
    if (str_resize(&new_str, src->length + size)) {
        memmove(new_str->cstr, src->cstr, index);
        memmove(new_str->cstr + index, in, size);
        memmove(new_str->cstr + index + size, src->cstr + index, src->length - index + 1);
        return new_str;
    }
    return NULL;
}

string_t *str_insert_cstr(string_t **src, size_t index, const char *in) {
    if (index > (*src)->length) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bound\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t size = strlen(in);
    size_t len = (*src)->length;
    if (str_resize(src, len + size)) {
        memmove((*src)->cstr + index + size, (*src)->cstr + index, len - index + 1);
        memmove((*src)->cstr + index, in, size);
        return *src;
    }
    return NULL;
}

string_t *str_replaced(const string_t *src, const char *old_val, const char *new_val) {
    // creating string arrays
    size_t old_len = strlen(old_val);
    char *str = malloc(sizeof(char) * (src->length + 1));
    strncpy(str, src->cstr, src->length + 1);
    char *ptr = str;
    size_t cap = 16;
    char **tokens = malloc(sizeof(char *) * cap);
    size_t *part_len = malloc(sizeof(size_t) * cap);
    char *token;
    size_t index = 0;
    size_t counter = 0;
    token = strstr(str, old_val);
    while (token != NULL) {
        *token = 0;
        part_len[index] = strlen(str);
        tokens[index] = malloc(sizeof(char) * (part_len[index] + 1));
        strcpy(tokens[index], str);
        str = token + old_len;
        index++;
        counter++;
        token = strstr(str, old_val);
        if (index + 2 == cap) {
            cap += 16;
            tokens = realloc(tokens, sizeof(char *) * cap);
            part_len = realloc(part_len, sizeof(size_t) * cap);
        }
    }
    part_len[index] = strlen(str);
    tokens[index] = malloc(sizeof(char) * (part_len[index] + 1));
    strcpy(tokens[index], str);
    index++;
    tokens[index] = NULL;
    free(ptr);

    // creating new string
    size_t length = 0;
    size_t new_len = strlen(new_val);
    for (size_t i = 0; tokens[i] != NULL; i++) {
        length += strlen(tokens[i]);
        if (counter != i) {
            length += new_len;
        }
    }
    index = 0;
    string_t *result = NULL;
    if (str_resize(&result, length)) {
        for (size_t i = 0; tokens[i] != NULL; i++) {
            strcpy(result->cstr + index, tokens[i]);
            index += part_len[i];
            if (counter != i) {
                strcpy(result->cstr + index, new_val);
                index += new_len;
            }
            free(tokens[i]);
        }
        free(tokens);
        free(part_len);
        return result;
    }
    for (size_t i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
    free(part_len);
    return NULL;
}

// built in compare function for str_sort and str_sorted
static inline int str_cmp_l(const void *arg1, const void *arg2) {
    string_t *str1 = *(string_t **)arg1;
    string_t *str2 = *(string_t **)arg2;
    return (str1->length - str2->length);
}
static inline int str_cmp_a(const void *arg1, const void *arg2) {
    string_t *str1 = *(string_t **)arg1;
    string_t *str2 = *(string_t **)arg2;
    for (size_t i = 0; i <= str1->length && i <= str2->length; i++) {
        if (str1->cstr[i] != str2->cstr[i]) {
            return str1->cstr[i] - str2->cstr[i];
        }
    }
    return 0;
}
static inline int str_cmp_la(const void *arg1, const void *arg2) {
    string_t *str1 = *(string_t **)arg1;
    string_t *str2 = *(string_t **)arg2;
    if (str1->length != str2->length) {
        return str1->length - str2->length;
    } else {
        for (size_t i = 0; i < str1->length; i++) {
            if (str1->cstr[i] != str2->cstr[i]) {
                return str1->cstr[i] - str2->cstr[i];
            }
        }
    }
    return 0;
}
static inline int str_cmp_ai(const void *arg1, const void *arg2) {
    string_t *str1 = *(string_t **)arg1;
    string_t *str2 = *(string_t **)arg2;
    for (size_t i = 0; i <= str1->length && i <= str2->length; i++) {
        if (toupper(str1->cstr[i]) != toupper(str2->cstr[i])) {
            return toupper(str1->cstr[i]) - toupper(str2->cstr[i]);
        }
    }
    return 0;
}
static inline int str_cmp_lai(const void *arg1, const void *arg2) {
    string_t *str1 = *(string_t **)arg1;
    string_t *str2 = *(string_t **)arg2;
    if (str1->length != str2->length) {
        return str1->length - str2->length;
    } else {
        for (size_t i = 0; i < str1->length; i++) {
            if (toupper(str1->cstr[i]) != toupper(str2->cstr[i])) {
                return toupper(str1->cstr[i]) - toupper(str2->cstr[i]);
            }
        }
    }
    return 0;
}

string_t **str_sort(string_t **src, size_t size, const char *mode, ...) {
    enum modes { c, i, l, a, la, modes_max };
    bool option[modes_max] = {0};
    for (const char *ch = mode; *ch != 0; ch++) {
        if (*ch == 'c') {
            option[c] = true;
            break;
        } else if (*ch == 'i') {
            option[i] = true;
        } else if (*ch == 'l') {
            if (option[a] == true) {
                continue;
            } else {
                option[l] = true;
            }
        } else if (*ch == 'a') {
            if (option[l] == true) {
                option[l] = false;
                option[la] = true;
            } else {
                option[a] = true;
            }
        } else {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid mode\n", stderr), NULL);
#else
            return NULL;
#endif
        }
    }
    if (size == 0) {
        for (; src[size] != NULL; size++);
    }
#ifdef _MSC_VER
    void *temp;
#endif
    int (*compar)(const void *, const void *);
    if (option[c] == true) {
        va_list ap;
        va_start(ap, mode);
#ifdef _MSC_VER
        temp = va_arg(ap, void *);
        compar = (int (*)(const void *, const void *))temp;
#else
        compar = va_arg(ap, int (*)(const void *, const void *));
#endif
        va_end(ap);
    } else if (option[l] == true) {
        compar = str_cmp_l;
    } else if (option[a] == true && option[i] == false) {
        compar = str_cmp_a;
    } else if (option[a] == true && option[i] == true) {
        compar = str_cmp_ai;
    } else if (option[la] == true) {
        compar = str_cmp_la;
    } else if (option[la] == true && option[i] == true) {
        compar = str_cmp_lai;
    } else {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: invalid mode\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    qsort(src, size, sizeof(string_t *), compar);
    return src;
}

string_t **str_sorted(string_t **src, size_t size, const char *mode, ...) {
    enum modes { c, i, l, a, la, modes_max };
    bool option[modes_max] = {0};
    for (const char *ch = mode; *ch != 0; ch++) {
        if (*ch == 'c') {
            option[c] = true;
            break;
        } else if (*ch == 'i') {
            option[i] = true;
        } else if (*ch == 'l') {
            if (option[a] == true) {
                continue;
            } else {
                option[l] = true;
            }
        } else if (*ch == 'a') {
            if (option[l] == true) {
                option[l] = false;
                option[la] = true;
            } else {
                option[a] = true;
            }
        } else {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid mode\n", stderr), NULL);
#else
            return NULL;
#endif
        }
    }
    if (size == 0) {
        for (; src[size] != NULL; size++);
    }
#ifdef _MSC_VER
    void *temp;
#endif
    int (*compar)(const void *, const void *);
    if (option[c] == true) {
        va_list ap;
        va_start(ap, mode);
#ifdef _MSC_VER
        temp = va_arg(ap, void *);
        compar = (int (*)(const void *, const void *))temp;
#else
        compar = va_arg(ap, int (*)(const void *, const void *));
#endif
        va_end(ap);
    } else if (option[l] == true) {
        compar = str_cmp_l;
    } else if (option[a] == true && option[i] == false) {
        compar = str_cmp_a;
    } else if (option[a] == true && option[i] == true) {
        compar = str_cmp_ai;
    } else if (option[la] == true) {
        compar = str_cmp_la;
    } else if (option[la] == true && option[i] == true) {
        compar = str_cmp_lai;
    } else {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: invalid mode\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    string_t **result = malloc(sizeof(string_t *) * (size + 1));
    for (size_t i = 0; i < size; i++) {
        result[i] = str_strdup(src[i]);
    }
    result[size] = NULL;
    qsort(result, size, sizeof(string_t *), compar);
    return result;
}

int str_strcmp(const string_t *str1, const string_t *str2) {
    for (size_t i = 0; i <= str1->length && i <= str2->length; i++) {
        if (str1->cstr[i] != str2->cstr[i]) {
            return str1->cstr[i] - str2->cstr[i];
        }
    }
    return 0;
}

bool str_streq(const string_t *str1, const string_t *str2) {
    if (str1->length != str2->length) {
        return false;
    }
    for (size_t i = 0; i < str1->length; i++) {
        if (str1->cstr[i] != str2->cstr[i]) {
            return false;
        }
    }
    return true;
}

bool str_start_with(const string_t *self, const char *str) {
    size_t size = strlen(str);
    if (self->length < size) {
        return false;
    }
    return (memcmp(self->cstr, str, size) == 0 ? true : false);
}

bool str_end_with(const string_t *self, const char *str) {
    size_t size = strlen(str);
    if (self->length < size) {
        return false;
    }
    return (memcmp(self->cstr + (self->length - size), str, size) == 0 ? true : false);
}

bool str_has(const string_t *self, const char *str) {
    if (strstr(self->cstr, str)) {
        return true;
    }
    return false;
}

char str_char_at(string_t *self, size_t index, char new_val) {
    if (index >= self->length) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bounds\n", stderr), 0);
#else
        return 0;
#endif
    }
    if (new_val != 0) {
        self->cstr[index] = new_val;
    }
    return self->cstr[index];
}

string_t *str_stripped(const string_t *src, const char *needle) {
    size_t len = strlen(needle);
    size_t lcount = 0;
    size_t rcount = 0;
    for (size_t i = 0; i < src->length; i++) {
        for (size_t j = 0; j < len; j++) {
            if (src->cstr[i] == needle[j]) {
                lcount++;
                goto lout;
            }
        }
        break;
    lout:
        continue;
    }
    string_t *result = NULL;
    if (lcount == src->length) {
        if (str_resize(&result, 0)) {
            return result;
        }
        return NULL;
    }
    for (size_t i = src->length - 1; i > 0; i--) {
        for (size_t j = 0; j < len; j++) {
            if (src->cstr[i] == needle[j]) {
                rcount++;
                goto rout;
            }
        }
        break;
    rout:
        continue;
    }
    if (str_resize(&result, (src->length - lcount - rcount))) {
        memcpy(result->cstr, src->cstr + lcount, src->length - lcount - rcount);
        result->cstr[result->length] = 0;
        return result;
    }
    return NULL;
}

string_t *str_strip(string_t **self, const char *needle) {
    size_t len = strlen(needle);
    size_t lcount = 0;
    size_t rcount = 0;
    for (size_t i = 0; i < (*self)->length; i++) {
        for (size_t j = 0; j < len; j++) {
            if ((*self)->cstr[i] == needle[j]) {
                lcount++;
                goto lout;
            }
        }
        break;
    lout:
        continue;
    }
    if (lcount == (*self)->length) {
        if (str_resize(self, 0)) {
            (*self)->cstr[0] = 0;
            return *self;
        }
        return NULL;
    }
    for (size_t i = (*self)->length - 1; i > 0; i--) {
        for (size_t j = 0; j < len; j++) {
            if ((*self)->cstr[i] == needle[j]) {
                rcount++;
                goto rout;
            }
        }
        break;
    rout:
        continue;
    }
    memmove((*self)->cstr, (*self)->cstr + lcount, (*self)->length - lcount - rcount);
    if (str_resize(self, ((*self)->length - lcount - rcount))) {
        (*self)->cstr[(*self)->length] = 0;
        return *self;
    }
    return NULL;
}

string_t *str_from_file(const char *file_name) {
    errno = 0;
    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) {
        return (fprintf(stderr, "[ERRO]: failed opening file: \"%s\": %s\n", file_name, strerror(errno)), errno = 0, NULL);
    }
    fseek(fp, 0, SEEK_END);
    size_t length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    string_t *result = NULL;
    if (str_resize(&result, length)) {
        fread(result->cstr, sizeof(char), length, fp);
        result->cstr[result->length] = 0;
        fclose(fp);
        return result;
    }
    fclose(fp);
    return NULL;
}

bool str_isdigit(const string_t *src) {
    for (size_t i = 0; i < src->length; i++) {
        if (isdigit(src->cstr[i]) == false) {
            return false;
        }
    }
    return true;
}

bool str_isxdigit(const string_t *src) {
    for (size_t i = 0; i < src->length; i++) {
        if (isxdigit(src->cstr[i]) == false) {
            if (i == 1) {
                if ((src->cstr[i] == 'x' || src->cstr[i] == 'X') && src->cstr[0] == '0') {
                    continue;
                }
            } else {
                return false;
            }
        }
    }
    return true;
}

bool str_isfloat(const string_t *src) {
    bool dot = false;
    for (size_t i = 0; i < src->length; i++) {
        if (isdigit(src->cstr[i]) == false) {
            if (src->cstr[i] == '.') {
                if (dot) {
                    return false;
                }
                dot = true;
            } else {
                return false;
            }
        }
    }
    return true;
}

long str_strtol(const string_t *src, bool *err, int base) {
    char *res = NULL;
    errno = 0;
    long result = strtol(src->cstr, &res, base);
    if (errno != 0) {
        if (err != NULL) {
            *err = false;
            return 0;
        }
        return 0;
    }
    if (*res == 0 && (size_t)(res - src->cstr) == src->length) {
        if (err != NULL) {
            *err = true;
            return result;
        }
        errno = EINVAL;
        return result;
    } else {
        if (err != NULL) {
            *err = false;
            return 0;
        }
        errno = EINVAL;
        return 0;
    }
}

double str_strtod(const string_t *src, bool *err) {
    char *res = NULL;
    errno = 0;
    double result = strtod(src->cstr, &res);
    if (errno != 0) {
        if (err != NULL) {
            *err = false;
            return 0;
        }
        return 0;
    }
    if (*res == 0 && (size_t)(res - src->cstr) == src->length) {
        if (err != NULL) {
            *err = true;
            return result;
        }
        errno = EINVAL;
        return result;
    } else {
        if (err != NULL) {
            *err = false;
            return 0;
        }
        errno = EINVAL;
        return 0;
    }
}

char *str_utf8_char_at(string_t *self, size_t index) {
    static char buf[5];
    memset(buf, 0, sizeof(buf));

    size_t current_char = 0;
    const unsigned char *ptr = (const unsigned char *)self->cstr;

    while (current_char < index) {
        if (*ptr == 0) {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: index out of bounds\n", stderr), buf);
#else
            return buf;
#endif
        } else if ((*ptr & 0x80) == 0) {
            ptr++;
        } else if ((*ptr & 0xE0) == 0xC0) {
            ptr += 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            ptr += 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            ptr += 4;
        } else {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid utf-8 sequence\n", stderr), buf);
#else
            return buf;
#endif
        }
        current_char++;
    }
    if (*ptr == 0) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bounds\n", stderr), buf);
#else
        return buf;
#endif
    } else if ((*ptr & 0x80) == 0) {
        memcpy(buf, ptr, 1);
        return buf;
    } else if ((*ptr & 0xE0) == 0xC0) {
        memcpy(buf, ptr, 2);
        return buf;
    } else if ((*ptr & 0xF0) == 0xE0) {
        memcpy(buf, ptr, 3);
        return buf;
    } else if ((*ptr & 0xF8) == 0xF0) {
        memcpy(buf, ptr, 4);
        return buf;
    }
#ifdef YASLI_DEBUG
    return (fputs("[ERRO]: invalid utf-8 sequence\n", stderr), buf);
#else
    return buf;
#endif
}

int str_print(string_t *self, FILE *fp, const char *end) {
    return fprintf(fp, "%s%s", self->cstr, end);
}

string_t *str_sliced(const string_t *src, int64_t start, int64_t end, int64_t step) {
    if (step == 0) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: slice step cannot be zero\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t start_index;
    size_t end_index;
    if (step > 0) {
        start_index = start == YASLI_START ? 0 : start == YASLI_END ? src->length : start >= 0 ? (size_t)start : src->length + start;
        end_index = end == YASLI_START ? 0 : end == YASLI_END ? src->length : end > 0 ? (size_t)end : src->length + end;
    } else {
        start_index = end == YASLI_START ? 0 : end == YASLI_END ? src->length : end == 0 ? (size_t)end + 1 : end > 0 ? (size_t)end + 1 : src->length + end + 1;
        end_index = start == YASLI_START ? 0
                    : start == YASLI_END ? src->length
                    : start == 0         ? src->length
                    : start > 0          ? (size_t)start + 1
                                         : src->length + start + 1;
    }
    if (start_index > src->length) {
        start_index = src->length;
    }
    if (end_index > src->length) {
        end_index = src->length;
    }
    if (start_index > end_index) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t sub_len = end_index - start_index;
    const char *ptr = src->cstr + start_index;
    size_t abs_step = llabs(step);
    size_t length = sub_len % abs_step == 0 ? sub_len / abs_step : sub_len / abs_step + 1;
    string_t *result = NULL;
    if (str_resize(&result, length)) {
        size_t index = 0;
        if (step > 0) {
            for (size_t i = 0; i < sub_len; i += step) {
                result->cstr[index++] = ptr[i];
            }
        } else {
            // i <= sub_len for unsigned integer overflow
            for (size_t i = sub_len - 1; i <= sub_len; i += step) {
                result->cstr[index++] = ptr[i];
            }
        }
        result->cstr[result->length] = 0;
        return result;
    }
    return NULL;
}

string_t *str_slice(string_t **self, int64_t start, int64_t end, int64_t step) {
    if (step == 0) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: slice step cannot be zero\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t start_index;
    size_t end_index;
    if (step > 0) {
        start_index = start == YASLI_START ? 0 : start == YASLI_END ? (*self)->length : start >= 0 ? (size_t)start : (*self)->length + start;
        end_index = end == YASLI_START ? 0 : end == YASLI_END ? (*self)->length : end > 0 ? (size_t)end : (*self)->length + end;
    } else {
        start_index = end == YASLI_START ? 0
                      : end == YASLI_END ? (*self)->length
                      : end == 0         ? (size_t)end + 1
                      : end > 0          ? (size_t)end + 1
                                         : (*self)->length + end + 1;
        end_index = start == YASLI_START ? 0
                    : start == YASLI_END ? (*self)->length
                    : start == 0         ? (*self)->length
                    : start > 0          ? (size_t)start + 1
                                         : (*self)->length + start + 1;
    }
    if (start_index > (*self)->length) {
        start_index = (*self)->length;
    }
    if (end_index > (*self)->length) {
        end_index = (*self)->length;
    }
    if (start_index > end_index) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t sub_len = end_index - start_index;
    const char *ptr = (*self)->cstr + start_index;
    size_t abs_step = llabs(step);
    size_t length = sub_len % abs_step == 0 ? sub_len / abs_step : sub_len / abs_step + 1;
#if defined(__STDC_NO_VLA__)
    char *result = malloc(length);
#else
    char result[length];
#endif
    size_t index = 0;
    if (step > 0) {
        for (size_t i = 0; i < sub_len; i += step) {
            result[index++] = ptr[i];
        }
    } else {
        // i <= sub_len for unsigned integer overflow
        for (size_t i = sub_len - 1; i <= sub_len; i += step) {
            result[index++] = ptr[i];
        }
    }
    if (str_resize(self, length)) {
        memcpy((*self)->cstr, result, length);
        (*self)->cstr[(*self)->length] = 0;
#ifdef __STDC_NO_VLA__
        free(result);
#endif
        return *self;
    }
#ifdef __STDC_NO_VLA__
    free(result);
#endif
    return NULL;
}

string_t *str_utf8_sliced(const string_t *src, int64_t start, int64_t end, int64_t step) {
    if (step == 0) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: slice step cannot be zero\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t str_length = str_utf8_strlen(src);
    size_t start_index;
    size_t end_index;
    if (step > 0) {
        start_index = start == YASLI_START ? 0 : start == YASLI_END ? str_length : start >= 0 ? (size_t)start : str_length + start;
        end_index = end == YASLI_START ? 0 : end == YASLI_END ? str_length : end > 0 ? (size_t)end : str_length + end;
    } else {
        start_index = end == YASLI_START ? 0 : end == YASLI_END ? str_length : end == 0 ? (size_t)end + 1 : end > 0 ? (size_t)end + 1 : str_length + end + 1;
        end_index = start == YASLI_START ? 0
                    : start == YASLI_END ? str_length
                    : start == 0         ? str_length
                    : start > 0          ? (size_t)start + 1
                                         : str_length + start + 1;
    }
    if (start_index > str_length) {
        start_index = str_length;
    }
    if (end_index > str_length) {
        end_index = str_length;
    }
    if (start_index > end_index) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t sub_len = end_index - start_index;
    size_t abs_step = llabs(step);

    // get to the starting index.
    size_t current_char = 0;
    const unsigned char *ptr = (const unsigned char *)src->cstr;
    while (current_char < start_index) {
        if (*ptr == 0) {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
            return NULL;
#endif
        } else if ((*ptr & 0x80) == 0) {
            ptr++;
        } else if ((*ptr & 0xE0) == 0xC0) {
            ptr += 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            ptr += 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            ptr += 4;
        } else {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid utf-8 sequence\n", stderr), NULL);
#else
            return NULL;
#endif
        }
        current_char++;
    }

    // get result length and utf-8 related information
    current_char = 0;
    const unsigned char *str = ptr;
    size_t total_size = 0;
#if defined(__STDC_NO_VLA__)
    uint8_t *info = malloc(sizeof(uint8_t) * sub_len);
#else
    uint8_t info[sub_len];
#endif

    while (current_char < sub_len) {
        if (*str == 0) {
#if defined(__STDC_NO_VLA__)
            free(info);
#endif
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
            return NULL;
#endif
        } else if ((*str & 0x80) == 0) {
            str++;
            info[current_char] = 1;
            if (step > 0) {
                if (current_char % abs_step == 0) {
                    total_size += 1;
                }
            } else {
                if ((sub_len - current_char - 1) % abs_step == 0) {
                    total_size += 1;
                }
            }
        } else if ((*str & 0xE0) == 0xC0) {
            str += 2;
            info[current_char] = 2;
            if (step > 0) {
                if (current_char % abs_step == 0) {
                    total_size += 2;
                }
            } else {
                if ((sub_len - current_char - 1) % abs_step == 0) {
                    total_size += 2;
                }
            }
        } else if ((*str & 0xF0) == 0xE0) {
            str += 3;
            info[current_char] = 3;
            if (step > 0) {
                if (current_char % abs_step == 0) {
                    total_size += 3;
                }
            } else {
                if ((sub_len - current_char - 1) % abs_step == 0) {
                    total_size += 3;
                }
            }
        } else if ((*str & 0xF8) == 0xF0) {
            str += 4;
            info[current_char] = 4;
            if (step > 0) {
                if (current_char % abs_step == 0) {
                    total_size += 4;
                }
            } else {
                if ((sub_len - current_char - 1) % abs_step == 0) {
                    total_size += 4;
                }
            }
        } else {
#if defined(__STDC_NO_VLA__)
            free(info);
#endif
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid utf-8 sequence\n", stderr), NULL);
#else
            return NULL;
#endif
        }
        current_char++;
    }

    string_t *result = NULL;
    if (str_resize(&result, total_size)) {
        size_t index = 0;
        size_t curr = 0;
        if (step > 0) {
            for (size_t i = 0; i < sub_len;) {
                memcpy(&result->cstr[index], &ptr[curr], info[i]);
                index += info[i];
                for (size_t j = 0; j < abs_step; j++) {
                    if (i < sub_len) {
                        curr += info[i];
                        i++;
                    }
                }
            }
        } else {
            curr = str - ptr - info[sub_len - 1];
            // i <= sub_len for unsigned integer overflow
            for (size_t i = sub_len - 1; i <= sub_len;) {
                memcpy(&result->cstr[index], &ptr[curr], info[i]);
                index += info[i];
                for (size_t j = 0; j < abs_step; j++) {
                    if (i <= sub_len) {
                        curr -= info[i];
                        i--;
                    }
                }
            }
        }
        result->cstr[result->length] = 0;
#if defined(__STDC_NO_VLA__)
        free(info);
#endif
        return result;
    }
#if defined(__STDC_NO_VLA__)
    free(info);
#endif
    return NULL;
}

string_t *str_utf8_slice(string_t **self, int64_t start, int64_t end, int64_t step) {
    if (step == 0) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: slice step cannot be zero\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t str_length = str_utf8_strlen(*self);
    size_t start_index;
    size_t end_index;
    if (step > 0) {
        start_index = start == YASLI_START ? 0 : start == YASLI_END ? str_length : start >= 0 ? (size_t)start : str_length + start;
        end_index = end == YASLI_START ? 0 : end == YASLI_END ? str_length : end > 0 ? (size_t)end : str_length + end;
    } else {
        start_index = end == YASLI_START ? 0 : end == YASLI_END ? str_length : end == 0 ? (size_t)end + 1 : end > 0 ? (size_t)end + 1 : str_length + end + 1;
        end_index = start == YASLI_START ? 0
                    : start == YASLI_END ? str_length
                    : start == 0         ? str_length
                    : start > 0          ? (size_t)start + 1
                                         : str_length + start + 1;
    }
    if (start_index > str_length) {
        start_index = str_length;
    }
    if (end_index > str_length) {
        end_index = str_length;
    }
    if (start_index > end_index) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    size_t sub_len = end_index - start_index;
    size_t abs_step = llabs(step);

    // get to the starting index.
    size_t current_char = 0;
    const unsigned char *ptr = (const unsigned char *)(*self)->cstr;
    while (current_char < start_index) {
        if (*ptr == 0) {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
            return NULL;
#endif
        } else if ((*ptr & 0x80) == 0) {
            ptr++;
        } else if ((*ptr & 0xE0) == 0xC0) {
            ptr += 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            ptr += 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            ptr += 4;
        } else {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid utf-8 sequence\n", stderr), NULL);
#else
            return NULL;
#endif
        }
        current_char++;
    }

    // get result length and utf-8 related information
    current_char = 0;
    const unsigned char *str = ptr;
    size_t total_size = 0;
#if defined(__STDC_NO_VLA__)
    uint8_t *info = malloc(sizeof(uint8_t) * sub_len);
#else
    uint8_t info[sub_len];
#endif

    while (current_char < sub_len) {
        if (*str == 0) {
#if defined(__STDC_NO_VLA__)
            free(info);
#endif
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
            return NULL;
#endif
        } else if ((*str & 0x80) == 0) {
            str++;
            info[current_char] = 1;
            if (step > 0) {
                if (current_char % abs_step == 0) {
                    total_size += 1;
                }
            } else {
                if ((sub_len - current_char - 1) % abs_step == 0) {
                    total_size += 1;
                }
            }
        } else if ((*str & 0xE0) == 0xC0) {
            str += 2;
            info[current_char] = 2;
            if (step > 0) {
                if (current_char % abs_step == 0) {
                    total_size += 2;
                }
            } else {
                if ((sub_len - current_char - 1) % abs_step == 0) {
                    total_size += 2;
                }
            }
        } else if ((*str & 0xF0) == 0xE0) {
            str += 3;
            info[current_char] = 3;
            if (step > 0) {
                if (current_char % abs_step == 0) {
                    total_size += 3;
                }
            } else {
                if ((sub_len - current_char - 1) % abs_step == 0) {
                    total_size += 3;
                }
            }
        } else if ((*str & 0xF8) == 0xF0) {
            str += 4;
            info[current_char] = 4;
            if (step > 0) {
                if (current_char % abs_step == 0) {
                    total_size += 4;
                }
            } else {
                if ((sub_len - current_char - 1) % abs_step == 0) {
                    total_size += 4;
                }
            }
        } else {
#if defined(__STDC_NO_VLA__)
            free(info);
#endif
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid utf-8 sequence\n", stderr), NULL);
#else
            return NULL;
#endif
        }
        current_char++;
    }

#if defined(__STDC_NO_VLA__)
    char *result = malloc(sizeof(char) * total_size);
#else
    char result[total_size];
#endif

    size_t index = 0;
    size_t curr = 0;
    if (step > 0) {
        for (size_t i = 0; i < sub_len;) {
            memcpy(&result[index], &ptr[curr], info[i]);
            index += info[i];
            for (size_t j = 0; j < abs_step; j++) {
                if (i < sub_len) {
                    if (i <= sub_len) {
                        curr -= info[i];
                        i--;
                    }
                }
            }
        }
    } else {
        curr = str - ptr - info[sub_len - 1];
        // i <= sub_len for unsigned integer overflow
        for (size_t i = sub_len - 1; i <= sub_len;) {
            memcpy(&result[index], &ptr[curr], info[i]);
            index += info[i];
            for (size_t j = 0; j < abs_step; j++) {
                curr -= info[i];
                i--;
            }
        }
    }

    if (str_resize(self, total_size)) {
        memcpy((*self)->cstr, result, total_size);
        (*self)->cstr[(*self)->length] = 0;
#if defined(__STDC_NO_VLA__)
        free(result);
        free(info);
#endif
        return (*self);
    }
#if defined(__STDC_NO_VLA__)
    free(result);
    free(info);
#endif
    return NULL;
}

int64_t str_find(const string_t *src, const char *needle, size_t number) {
    if (number == 0) {
        return -1;
    }
    const char *ptr = src->cstr;
    for (size_t i = 0; i < number; i++) {
        ptr = strstr(ptr, needle);
        if (ptr == NULL) {
            return -1;
        }
    }
    return (int64_t)(ptr - src->cstr);
}

int64_t str_utf8_find(const string_t *src, const char *needle, size_t number) {
    if (number == 0) {
        return -1;
    }
    const char *ptr = src->cstr;
    for (size_t i = 0; i < number; i++) {
        ptr = strstr(ptr, needle);
        if (ptr == NULL) {
            return -1;
        }
    }
    int64_t index = ptr - src->cstr;

    int64_t length = 0;
    int64_t current_char = 0;
    while (length <= index) {
        if (*ptr == 0) {
            return current_char;
        } else if ((*ptr & 0x80) == 0) {
            ptr++;
            length++;
        } else if ((*ptr & 0xE0) == 0xC0) {
            ptr += 2;
            length += 2;
        } else if ((*ptr & 0xF0) == 0xE0) {
            ptr += 3;
            length += 3;
        } else if ((*ptr & 0xF8) == 0xF0) {
            ptr += 4;
            length += 4;
        } else {
#ifdef YASLI_DEBUG
            return (fputs("[ERRO]: invalid utf-8 sequence\n", stderr), 0);
#else
            return 0;
#endif
        }
        current_char++;
    }
    return current_char;
}

string_t *str_removed(const string_t *src, size_t index, size_t length) {
    if (src == NULL) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: input is NULL\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    if (index + length > src->length) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    string_t *result = NULL;
    if (str_resize(&result, src->length - length)) {
        memmove(result->cstr, src->cstr, index);
        memmove(result->cstr + index, src->cstr + index + length, src->length - index - length);
        result->cstr[result->length] = 0;
        return result;
    }
    return NULL;
}

string_t *str_remove(string_t **self, size_t index, size_t length) {
    if ((*self) == NULL) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: input is NULL\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    if (index + length > (*self)->length) {
#ifdef YASLI_DEBUG
        return (fputs("[ERRO]: index out of bounds\n", stderr), NULL);
#else
        return NULL;
#endif
    }
    memmove((*self)->cstr + index, (*self)->cstr + index + length, (*self)->length - index - length);
    if (str_resize(self, (*self)->length - length)) {
        (*self)->cstr[(*self)->length] = 0;
        return (*self);
    }
    return NULL;
}
