grammar ftrpc_grammer;

options {
    language=cpp
}

decl_file : version_decl decl_module+ ;

version_decl : 'version' '=' EDITION_NUMBER ';';

decl_module : 'module' IDENTIFY ':' '{' decl_element* '}';

decl_element
    : decl_struct
    ;

decl_struct : 'struct' IDENTIFY ':' '{' struct_member* '}'  ';';

struct_member
    : type_descript IDENTIFY ';'
    | type_descript IDENTIFY '(' param_item ')' ';'
    ;

param_list
    : param_item (',' param_item)*
    |
    ;

param_item
    : type_descript IDENTIFY
    ;

type_descript
    : type_base ('[' ']')?
    ;

type_base
    : 'int'
    | 'void'
    | 'short'
    | 'string'
    | 'float'
    | 'bool'
    | IDENTIFY
    ;

EDITION_NUMBER : [0-9]+;
IDENTIFY : [A-Za-z_][0-9A-Za-z_]*;
LINE_COMMONT : '//' ~[\r\n]* -> skip;
BLOCK_COMMONT : '/*' .*? '*/' -> skip;