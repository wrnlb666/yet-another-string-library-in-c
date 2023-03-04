from ctypes import *
import platform as pf

lib = None
if pf.system() == "Windows":
    lib = cdll.LoadLibrary("./libstr.dll")
if pf.system() == "Linux":
    lib = cdll.LoadLibrary("./libstr.so")

class string_t(Structure):
    _fields_ = [
        ("length", c_size_t),
        ("capacity", c_size_t),
        ("cstr", c_char * 1),
    ]


# size_t str_strlen( const string_t* string );
lib.str_strlen.restype = c_size_t
lib.str_strlen.argtype = [ POINTER(string_t) ]

# size_t str_capacity( const string_t* string );
lib.str_capacity.restype = c_size_t
lib.str_capacity.argtype = [ POINTER(string_t) ]

# char* str_cstr( const string_t* string );
lib.str_cstr.restype = c_char_p
lib.str_cstr.argtype = [ POINTER(string_t) ]

# wchar_t* str_wstr( const string_t* string );
lib.str_wstr.restype = c_wchar_p
lib.str_wstr.argtype = [ POINTER(string_t) ]

# string_t* str_new_string( const char* src );
lib.str_new_string.restype = POINTER(string_t)
lib.str_new_string.argtype = [ c_char_p ]

# bool str_clear( string_t* string );
lib.str_clear.restype = c_bool
lib.str_clear.argtype = [ POINTER(string_t) ]

# string_t* str_append( const string_t* start, const string_t* end );
lib.str_append.restype = POINTER(string_t)
lib.str_append.argtype = [ POINTER(string_t), POINTER(string_t) ]

# string_t** str_split( const string_t* src, const char* needle );
lib.str_split.restype = POINTER( POINTER(string_t) )
lib.str_split.argtype = [ POINTER(string_t), c_char_p ]

# void str_to_upper( string_t* string );
lib.str_to_upper.restype = None
lib.str_to_upper.argtype = [ POINTER(string_t) ]

# void str_to_lower( string_t* string );
lib.str_to_lower.restype = None
lib.str_to_upper.argtype = [ POINTER(string_t) ]

# string_t* str_substr( const string_t* src, size_t start, size_t size );
lib.str_substr.restype = POINTER(string_t)
lib.str_substr.argtype = [ POINTER(string_t), c_size_t, c_size_t ]

# string_t* str_replace( const string_t* src, const char* old_val, const char* new_val );
lib.str_replace.restype = POINTER(string_t)
lib.str_replace.argtype = [ POINTER(string_t), c_char_p, c_char_p ]



class str_t:
    string = None

    def __init__( self, string = "" ) -> None:
        if isinstance( string, str ):
            self.string = lib.str_new_string( c_char_p( bytes( string, "UTF-8" ) ) )
        elif isinstance( string, POINTER(string_t) ):
            self.string = string
        else:
            raise TypeError(f"Invalid argument type: {type(string)}")


    def len( self ) -> int:
        return int( lib.str_strlen(self.string) )
    
    def cap( self ) -> int:
        return int( lib.str_capacity(self.string) )
    
    def cstr( self ) -> str:
        return str( lib.str_cstr(self.string), "UTF-8" )
    
    def __str__( self ) -> str:
        return self.cstr()
    
    def __repr__(self):
        return str(self)
    
    def wstr( self ) -> str:
        return str( lib.str_wstr(self.string) )
    
    def clear( self ) -> bool:
        return bool( lib.str_clear(self.string) )
    
    def __add__( self, other: "str_t" ) -> "str_t":
        return str_t( lib.str_append( self.string, other.string ) )
    
    def split(self, needle: str) -> list:
        l = []
        res = lib.str_split(self.string, c_char_p(bytes(needle, "UTF-8")))
        i = 0
        while res[i]:
            curr_ptr = res[i]
            l.append( str_t( curr_ptr ) )
            i += 1
        return l
    
    def toupper( self ) -> None:
        lib.str_to_upper( self.string )

    def tolower( self ) -> None:
        lib.str_to_lower( self.string )

    def substr( self, start: int, size: int ) -> "str_t":
        return str_t( lib.str_substr( self.string, c_size_t(start), c_size_t(size) ) )
    
    def replace( self, old: str, new: str ) -> "str_t":
        return str_t( lib.str_replace( self.string, c_char_p( bytes( old, "UTF-8" ) ), c_char_p( bytes( new, "UTF-8" ) ) ) )
    
