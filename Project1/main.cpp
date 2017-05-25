#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "assembler.h"
#include "symtbl.h"
#define LASTINST    "ZZZ"
char *types[] = { "REG", "LBL", "UNK" };
enum STATES { DONE, CHK_LABEL, CHK_INST, CHK_OPERAND };
/* Diagnostic messages */
char *byte_word[] = { "Word", "Byte", "Offset" };
char *am_names[] = { "Reg", "Idx", "Rel", "Abs", "Ind", "IndInc", "Imm" };
/* Byte of storage by addr mode */
unsigned am_incr[] = { 0, 2, 2, 2, 0, 0, 2 };
unsigned lineno;   /* Current line number */
unsigned short lc; /* Line counter */
struct symtblentry *symtbl = NULL;/*initialize the symtbl as null and later on it 
will be used to store the entry added from read function and will used to output data
as a source by output functions*/
void display_symtbl()//display symtbl stptr is the pointer of symtblentry
{
	/*
	Display symbol table entries
	*/
	struct symtblentry *stptr;
	stptr = symtbl;
	while (stptr)
	{
		printf("%10s %s %04x\n", stptr->name, types[stptr->type], stptr->value);
		stptr = stptr->next;
	}
}//simple display symble table
struct symtblentry *get_entry(char *name)
{
	/*
	Check symtbl for name
	Return address in list or NULL (not found)
	This is a linear search
	*/
	struct symtblentry *stptr;//this line is same as showed in void display symtbl.
	stptr = symtbl;
	while (stptr)
		if (strcmpi(stptr->name, name) == 0)//compare string stptr's name and name
			return stptr;
		else
			stptr = stptr->next;
			return NULL;  /* Not in symtbl */
			//dose this is a full loop?
}
void add_entry(char *name, int value, enum SYMTBLTYPES type)
{
	/*
	Assumes the entry is unique (caller must do a get_entry() beforehand
	Adds new entry to end of list
	*/
	struct symtblentry *newentry;
	newentry = malloc(sizeof(struct symtblentry));// the size is flexible
	strcpy(newentry->name, name);//store the name to newentry
	/*strcpy: Copies the C string pointed by source into the array pointed 
	by destination, including the terminating null character 
	(and stopping at that point).*/
	newentry->value = value;// store the value
	newentry->type = type;
	newentry->next = symtbl;
	symtbl = newentry;
	//until this I was thinking, does newentry is the same as stptr? 
	//did the newentry stored in symtbl?
	//whats the difference between symtbl and symtblentry?
}
void init_symtbl()// add type REG!   give the initial symtbl
{
	/*
	- Initialize the symbol table with register values
	- Treat registers as labels with type of REGTYPE
	*/
	add_entry("R0", 0, REGTYPE);
	add_entry("PC", 0, REGTYPE); /* Alias for R0 */
	add_entry("R1", 1, REGTYPE);
	add_entry("SP", 1, REGTYPE);
	add_entry("R2", 2, REGTYPE);
	add_entry("R3", 3, REGTYPE);
	add_entry("R4", 4, REGTYPE);
	add_entry("R5", 5, REGTYPE);
	add_entry("R6", 6, REGTYPE); 
	add_entry("R7", 7, REGTYPE); 
	add_entry("R8", 8, REGTYPE); 
	add_entry("R9", 9, REGTYPE); 
	add_entry("R10", 10, REGTYPE); 
	add_entry("R11", 11, REGTYPE); 
	add_entry("R12", 12, REGTYPE); 
	add_entry("R13", 13, REGTYPE); 
	add_entry("R14", 14, REGTYPE);
	add_entry("R15", 15, REGTYPE);
}
struct inst_el inst_list[] = {  // here it stored all the insts, include 12j
	{ "RRC", SINGLE, 0x1000, WORD },
	{ "RRC.B", SINGLE, 0x1000, BYTE },
	{ "RRC.W", SINGLE, 0x1000, WORD },
	{ "SWPB",SINGLE, 0x1080, WORD },
	{ "RRA", SINGLE,0x1100 , WORD },
	{ "RRA.B", SINGLE,0x1100 , BYTE },
	{ "RRA.W", SINGLE,0x1100 , WORD },
	{ "SXT", SINGLE, 0x1180, WORD },
	{ "PUSH", SINGLE, 0x1200, WORD },
	{ "PUSH.B", SINGLE,0x1200 , BYTE },
	{ "PUSH.W", SINGLE,0x1200 , WORD },
	{ "CALL", SINGLE,0x1280 , WORD },
	{ "RETI", NONE, 0x1300, WORD},
	/*ABOVE ARE THE ONE-OPERAND */
	//one operand all start with 1
	{ "JNE", JUMP,0x2000 , OFFSET },
	{ "JNZ", JUMP,0X2000 , OFFSET },
	{ "JEQ", JUMP,0x2400 , OFFSET },
	{ "JZ", JUMP,0x2400 , OFFSET },
	{ "JNC", JUMP,0x2800 , OFFSET },
	{ "JLO", JUMP,0X2800 , OFFSET },
	{ "JC", JUMP, 0x2c00, OFFSET },
	{ "JHS", JUMP, 0x2C00, OFFSET },
	{ "JN", JUMP, 0x3000, OFFSET },
	{ "JGE", JUMP, 0x3400, OFFSET },
	{ "JL", JUMP, 0x3800, OFFSET },
	{ "JMP", JUMP, 0X3c00, OFFSET },
	/*ABOVE ARE THE JUMPS*/
	//jumps all start with 2 and 3
	{ "MOV", DOUBLE, 0x4000, WORD },
	{ "MOV.B", DOUBLE, 0x4000, BYTE },
	{ "MOV.W", DOUBLE, 0x4000, WORD },
	{ "ADD", DOUBLE, 0x5000, WORD },
	{ "ADD.B", DOUBLE, 0x5000, BYTE },
	{ "ADD.W", DOUBLE, 0x5000, WORD },
	{ "ADDC", DOUBLE, 0x6000, WORD },
	{ "ADDC.B", DOUBLE,0x6000 , BYTE },
	{ "ADDC.W", DOUBLE, 0x6000, WORD },
	{ "SUBC", DOUBLE, 0x7000, WORD },
	{ "SUBC.B", DOUBLE, 0x7000, BYTE },
	{ "SUBC.W", DOUBLE, 0x7000, WORD },
	{ "SUB", DOUBLE, 0x8000, WORD },
	{ "SUB.B", DOUBLE, 0x8000, BYTE },	
	{ "SUB.W", DOUBLE, 0x8000, WORD },
	{ "CMP", DOUBLE, 0x9000,WORD },
	{ "CMP.B", DOUBLE, 0x9000, BYTE },
	{ "CMP.W", DOUBLE, 0x9000, WORD },
	{ "DADC", DOUBLE, 0xa000, WORD },
	{ "DADC.B", DOUBLE, 0xa000,BYTE },
	{ "DADC.W", DOUBLE, 0xa000,WORD },
	{ "BIT", DOUBLE, 0xb000, WORD },
	{ "BIT.B", DOUBLE, 0xb000, BYTE },	
	{ "BIT.W", DOUBLE, 0xb000, WORD },
	{ "BIC", DOUBLE, 0xc000, WORD },
	{ "BIC.B", DOUBLE, 0xc000, BYTE },
	{ "BIC.W", DOUBLE, 0xc000, WORD },
	{ "BIS", DOUBLE, 0xd000,WORD },
	{ "BIS.B", DOUBLE, 0xd000, BYTE },
	{ "BIS.W", DOUBLE, 0xd000, WORD },
	{ "XOR", DOUBLE, 0xe000, WORD },
	{ "XOR.B", DOUBLE, 0xe000, BYTE },
	{ "XOR.W", DOUBLE, 0xe000, WORD },
	{ "AND", DOUBLE, 0xf000, WORD },
	{ "AND.B", DOUBLE, 0xf000,BYTE },
	{ "AND.W", DOUBLE, 0xf000,WORD },
	//2 operands all start lager than 3
{ LASTINST, 0, 0, 0 } /* End of list */
};
// here it stored all the insts, include 12j
struct inst_el *get_inst(char *inst)//if returned null means not valid instruction
{
	/*
	Scan the list of instructions for the inst.  If instruction exists, return a
	pointer to the element in question if it exists, otherwise NULL.
	Note: case insensitive search
	*/
	struct inst_el *ie_ptr;

	ie_ptr = inst_list;  /* or inst_list; */

	while (strcmpi(ie_ptr->inst, LASTINST) != 0)//it definately not zero, so why we need this
	{
		if (strcmpi(ie_ptr->inst, inst) == 0)
			return ie_ptr;
		else
			ie_ptr++;  /* Move to next element in list */
	}

	return NULL; /* Instruction not found in list */

}
//return null if inst doesnt exist

void process_inst(struct inst_el *ie_ptr,enum ADDR_MODE am, unsigned short v0, unsigned short v1)
{
	/* Process the instruction pointed to by ie_ptr */
	printf("LC: %04x - Instruction: %s ", lc, ie_ptr->inst);
	printf("Opcode: %4x ", ie_ptr->opcode);
	printf("Type: %d ", ie_ptr->type);
	printf("Byte-word: %s\n", byte_word[ie_ptr->bw]);

	lc += 2; /* Size of instruciton */

	if (ie_ptr != NONE)
	{
		/* AM other than zero */
		printf("Addressing mode: %s ", am_names[am]);
		printf("Value: %04x ", v0);
		if (am == INDEXED)
			printf("Index: %01x", v1);
		lc += am_incr[am]; /* Operand might require storage */
	}
	printf("\n\n");
}
//do i need to add functions to support for two operand mode

enum ADDR_MODE parse_op(char *operand,unsigned short *value0, unsigned short *value1)
{
	/*
	- Parse the supplied operand into:
	* value0 (value from symtbl for all modes)
	* value1 (register value from symtbl for indexed).
	- Return the associated mode:
	R1..R15 - REGISTER
	alpha(R1..R15) - INDEXED
	alpha - RELATIVE
	&alpha - ABSOLUTE
	@R1..R15 - INDIRECT
	@R1+..R15+ - INDIRECT_INCR
	#numeric #alpha - IMMEDIATE
	- Any invalid sequence returns BAD_ADDR_MODE.
	*/
	struct symtblentry *stptr; /* Pointer to extracted symbol's symtbl entry */
	enum ADDR_MODE addr_mode;  /* Addressing mode determined */
	char *opr_ptr;             /* Pointer to operand */
	char *reg_ptr;
	char *sym_ptr;             /* Pointer to extracted symbol */
	unsigned imm_num;
	unsigned sanity;           /* Limit number of characters in symbol */

	opr_ptr = operand;

	/* Check for prefix &, @, # */
	switch (*opr_ptr)
	{
	case '&':
		/* ABSOLUTE - expect symbol to follow '&' */
		opr_ptr++;
		/* Check for '&' followed by NUL */
		if (*opr_ptr == NUL)
		{
			printf("%s - missing identifier\n", operand);
			return BAD_ADDR_MODE;
		}
		/* Followed by at least one character.  Check symbol length.
		Note: This allows a non-alphabetic string to follow.
		*/
		sym_ptr = opr_ptr;
		sanity = 0;
		while ((isalnum(*opr_ptr) || *opr_ptr == '_') && sanity++ < (SYMBOL_LEN - 1))
			opr_ptr++;//isalnum is to decide if it is a character or a number

		if (*opr_ptr != NUL)
		{
			/* Invalid length or invalid character*/
			printf("%s - too long or invalid character\n", operand);
			return BAD_ADDR_MODE;
		}

		/* Check if symbol exists in symtbl */
		stptr = get_entry(sym_ptr);

		if (stptr == NULL)
		{
			/* Does not exist -- add to symtbl */
			add_entry(sym_ptr, 0, UNKTYPE);
			/* Return a dummy value - acceptable for first pass */
			*value0 = 0;
		}
		else
		{
			*value0 = stptr->value; /* Value from symtbl */
			if (stptr->type == REGTYPE)
			{
				printf("%s - reg with ABS\n", operand);
				return BAD_ADDR_MODE;
			}
		}
		return ABSOLUTE;

	case '@':
		/* INDIRECT or INDIRECT-AUTO-INCR
		- '@' followed by register only (Indirect) or
		- '@' followed by register '+' (Indirect-auto-incr)
		*/
		opr_ptr++;
		/* Find symbol -- if it exists (note: does not enforce leading alphabetic) */
		sym_ptr = opr_ptr;
		sanity = 0;
		while ((isalnum(*opr_ptr) || *opr_ptr == '_') && sanity++ < (SYMBOL_LEN - 1))
			opr_ptr++;

		/* Stopped on '+', NUL, or unknown */
		if (*opr_ptr == '+')
		{
			addr_mode = INDIRECT_INCR;
			/*
			- overwrite '+' with NUL
			- can check symbol now ('+' has been removed from operand string)
			*/
			*opr_ptr++ = NUL;  /*  Changes input str - see below */
		}
		else /* Assume stop on NUL */
			addr_mode = INDIRECT;

		/* Check last character */
		if (*opr_ptr != NUL)
		{
			/* Missing end-of-symbol NUL -- error */
			if (addr_mode == INDIRECT_INCR)
			{
				/* Move back one and replace '+' to fix operand string */
				opr_ptr--;
				*opr_ptr = '+';
			}
			printf("%s - invalid indirect fmt\n", operand);
			return BAD_ADDR_MODE;
		}

		/* Check for register */
		stptr = get_entry(sym_ptr);

		if (stptr == NULL || stptr->type != REGTYPE)
		{
			/* missing reg */
			printf("%s - missing reg\n", operand);
			return BAD_ADDR_MODE;
		}

		*value0 = stptr->value; /* Register number 0..15 */
		return addr_mode;

	case '#':
		/* #numeric or #alpha - IMMEDIATE
		- signed, unsigned, and hex (0xhhhh)
		- problems with alpha and fwd ref and constand -1, 0, 1, 2, 8
		- indicate a non-label numeric value
		- call check number -- can use with EQU
		*/
		opr_ptr++;
		/* Find symbol -- if it exists (note: does not enforce leading alphabetic) */
		sym_ptr = opr_ptr;
		sanity = 0;
		while ((isalnum(*opr_ptr) || *opr_ptr == '_') && sanity++ < (SYMBOL_LEN - 1))
			opr_ptr++;
		if (*opr_ptr == '-1'|| *opr_ptr=='1'||*opr_ptr=='0'){
		}
		else if (*opr_ptr == '4' || *opr_ptr == '8') {

		}
		int x = *opr_ptr;//expected a integer
		if (*opr_ptr == x) {

		}
		return BAD_ADDR_MODE;

	default:
		/* Assume alphanumeric string -- one of:
		- alphanumeric - NUL-term - RELATIVE or REGISTER
		- alphanumeric ( register ) - '(' term... - INDEXED
		- or invalid
		*/
		sym_ptr = opr_ptr;
		if (isalpha(*opr_ptr) || *opr_ptr == '_')
		{
			/* alphanumeric string - check for RELATIVE, REGISTER, or INDEXED */
			sym_ptr = opr_ptr;
			while (isalnum(*opr_ptr))
				opr_ptr++;

			if (*opr_ptr == NUL)
			{
				stptr = get_entry(sym_ptr);
				if (stptr == NULL)
					add_entry(sym_ptr, 0, UNKTYPE);
				else
				{
					*value0 = stptr->value;
					/* Return type Register or default Relative */
					return (stptr->type == REGTYPE) ? REGISTER : RELATIVE;
				}
			}
			else
			{
				/*
				- If opr_ptr points to NUL:
				* check symtbl - return either RELATIVE (not a REGTYPE) or
				REGISTER (a REGTYPE)
				* return value of symbol from symtbl in value0
				- If opr_ptr points to '(' - possible indexed:
				* remember first symbol
				* must find second symbol and closing ')'
				* if no ')' - return BAD_ADDR_MODE
				* if ')' check following character:
				- if not NUL, return BAD_ADDR_MODE
				* check symbols:
				- first should be non-register, if so, value0 is value from
				symtbl
				- second should be register, if so, value1 is value from
				symtbl
				- return INDEXED
				* if either symbol invalid, return BAD_ADDR_MODE
				*/
				return BAD_ADDR_MODE;
			}
		}
		else
		{
			printf("%s - expected alphabetic\n", operand);
			return BAD_ADDR_MODE;
		}
	}

}

int main() {
	

	struct symtblentry *stptr; /* Pointer to extracted symbol's symtbl entry */
	unsigned short val0, val1;
	struct inst_el *ie_ptr;
	enum STATES state;
	enum ADDR_MODE am;
	char *instring;
	char *token;
	FILE *fp;
	int rc;
	//functions of ask user to type in two files one for input one for output


	// until here we have two files and start to read the whole file.
	init_symtbl();
	instring = malloc(LINE_LEN);
	lineno = 1;
	/* Reads one line until EOF */
	while (fgets(instring, LINE_LEN, fp) != NULL)
	{//instring is a pointer, line_len is the buf, fp is the file pointer
		/* Truncate lines too long */
		rc = strlen(instring);
		instring[rc - 1] = NUL;//why we use nul here? it should be a /0 in the end

		printf("%03d: >>%s<<\n", lineno, instring);// out put the first line(string)
		lineno += 1;
		state = CHK_LABEL;
		/* Get first token, stop on SP, TAB, NL, or NUL */
		token = strtok(instring, " \t\n");
		//Parse String into tokens separated by characters
		while (state != DONE)// we preset the state is CHK_LABEL
		{
			switch (state)
			{
			case CHK_LABEL:
				/* Check if label or instruction*/
				if (ie_ptr = get_inst(token))
					state = CHK_OPERAND;//means this token is not label is operand
				// should I change this state to CHK_INST so i can decided whether it is 
				//one operand or two operand or jumps
				else
				{
					/* Assume label */
					if (stptr = get_entry(token))
					{
						if (stptr->type == UNKTYPE)
						{
							/* Change to LBLTYPE and LC */
							stptr->type = LBLTYPE;
							stptr->value = lc;
						}
						else
						{
							printf("Label: >>%s<< exists\n", token);
							state = DONE;
						}
					}
					else//if stptr-type!=untype
					{
						printf(">>%s<<: to symtbl\n", token);
						add_entry(token, lc, LBLTYPE);
					}
					state = CHK_INST;
				}
				break;
			case CHK_INST:
				/* Check if valid instruction */
				if (ie_ptr = get_inst(token))
					//check the status of the inst
					// seperate it to three parts
					state = CHK_OPERAND;
				else
				{
					/* Unsupported instruction - stop processing record */
					printf("Unsupported instruction: %s\n", token);
					state = DONE;
				}
				break;
			case CHK_OPERAND:
				/* Check if operand supported (one address only) */
				if ((am = parse_op(token, &val0, &val1)) != BAD_ADDR_MODE)
					process_inst(ie_ptr, am, val0, val1);
				else
					printf("%s: Unsupported addressing mode\n", token);
				state = DONE;
				break;
			}
			token = strtok(NULL, " \t\n");
		}
	}
	
	
	
	
	//after this we analysis all the lines
	display_symtbl();
	//finsh the first pass
	// now is the second pass
	// in second pass, one more job is to identify the instruction if 
	//it need one or two opreands.
	// figure out the location of the address
	fclose(fp);
	// need the function to output to a file
	getchar();
	return 0;
}
