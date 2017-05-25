//Symtbl.h


#ifndef SYMTBL_H
#define SYMTBL_H
/* Symbol table types:
- REGTYPE - R0..R15, PC, SP, SR, or any other + value (0..15)
- value is not checked for range
- LBLTYPE - Any label + value (LC, signed, char, hex, etc)
- UNKTYPE - Forward reference (unknown label + value at this time)
*/
enum SYMTBLTYPES {	REGTYPE,LBLTYPE,UNKTYPE};
struct symtblentry {
	char *name;//*name &name[ SYMBOL_LEN ] which is better
	int value;
	enum SYMTBLTYPES type;
	struct symtblentry *next;// link list
};

/* Entry points for symbol table code */
extern struct symtblentry *get_entry(char *);
extern void add_entry(char *, int, enum SYMTBLTYPES);
extern void init_symtbl();

#endif
