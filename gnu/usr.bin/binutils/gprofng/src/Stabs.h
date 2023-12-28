/* Copyright (C) 2021-2023 Free Software Foundation, Inc.
   Contributed by Oracle.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef _STABS_H
#define _STABS_H

#include "dbe_structs.h"
#include "vec.h"

enum cpf_instr_type_t {
    CPF_INSTR_TYPE_LD = 0,              // profiled load instruction
    CPF_INSTR_TYPE_ST,                  // profiled store instruction
    CPF_INSTR_TYPE_PREFETCH,            // profiled prefetch instruction
    CPF_INSTR_TYPE_BRTARGET,            // branch target
    CPF_INSTR_TYPE_UNKNOWN,             // unidentified instruction
    CPF_INSTR_TYPE_NTYPES               // total # of instr types
};

class Function;
class LoadObject;
class Module;
class ComC;
class Elf;
class Dwarf;
class Symbol;
class Reloc;
struct cpf_stabs_t;
class SourceFile;
template <typename Key_t, typename Value_t> class Map;

class Include {
  public:
    typedef struct {
	SourceFile  *srcfile;
	int         lineno;
    } SrcFileInfo;
    Include();
    ~Include();
    void    new_src_file(SourceFile *source, int lineno, Function *func = NULL);
    void    new_include_file(SourceFile *source, Function *func);
    void    end_include_file(Function *func);
    void    push_src_files(Function *func);

  private:
    Vector<SrcFileInfo*> *stack;
};

// Stabs object
class Stabs {
  public:

    enum Stab_status {
	DBGD_ERR_NONE,
	DBGD_ERR_CANT_OPEN_FILE,
	DBGD_ERR_BAD_ELF_LIB,
	DBGD_ERR_BAD_ELF_FORMAT,
	DBGD_ERR_NO_STABS,
	DBGD_ERR_BAD_STABS,
	DBGD_ERR_NO_DWARF,
	DBGD_ERR_CHK_SUM
    };

    static Stabs *NewStabs(char *_path, char *lo_name);
    Stabs(char *_path, char *_lo_name);
    ~Stabs();

    bool	is_relocatable(){ return isRelocatable; }
    long long	get_textsz()	{ return textsz; }
    Platform_t	get_platform()	{ return platform; }
    WSize_t	get_class()	{ return wsize;}
    Stab_status	get_status()    { return status;}

    Stab_status	read_stabs(ino64_t srcInode, Module *module, Vector<ComC*> *comComs, bool readDwarf = false);
    Stab_status	read_archive(LoadObject *lo);
    bool	read_symbols(Vector<Function*> *functions);
    uint64_t	mapOffsetToAddress(uint64_t img_offset);
    char	*sym_name(uint64_t target, uint64_t instr, int flag);
  Elf *openElf (bool dbg_info = false);
    void        read_hwcprof_info(Module *module);
    void        dump();
    void        read_dwarf_from_dot_o(Module *mod);

    static bool is_fortran(Sp_lang_code lc) { return (lc == Sp_lang_fortran) || (lc == Sp_lang_fortran90); }
    static Function *find_func(char *fname, Vector<Function*> *functions, bool fortran, bool inner_names=false);
    Module	*append_Module(LoadObject *lo, char *name, int lastMod = 0);
    Function	*append_Function(Module *module, char *fname);
    Function	*append_Function(Module *module, char *linkerName, uint64_t pc);
    Function	*map_PC_to_func(uint64_t pc, uint64_t &low_pc, Vector<Function*> *functions);
    char		*path;			// path to the object file
    char                *lo_name;       // User name of load object

  private:
  Elf *elfDbg; // ELF with debug info
  Elf *elfDis; // ELF for disasm
    Stab_status		status;			// current stabs status

    long long		textsz;			// text segment size
    Platform_t		platform;		// Sparc, Sparcv9, Intel
    WSize_t		wsize;			// word size: 32 or 64
    bool		isRelocatable;
    Symbol              *last_PC_to_sym;

    Vector<cpf_stabs_t> analyzerInfoMap;        // stabs->section mapping

    bool		check_Comm(Vector<ComC*> *comComs);
    void		check_Info(Vector<ComC*> *comComs);
    void		check_Loop(Vector<ComC*> *comComs);
    void                check_AnalyzerInfo();
    void                append_local_funcs(Module *module, int first_ind);
  Stab_status srcline_Stabs (Module *module, unsigned int StabSec, unsigned int StabStrSec, bool comdat);
  Stab_status archive_Stabs (LoadObject *lo, unsigned int StabSec, unsigned int StabStrSec, bool comdat);

    // Interface with Elf Symbol Table
    void                check_Symtab();
    void                readSymSec(unsigned int sec, Elf *elf);
    void                check_Relocs();
    void                get_save_addr(bool need_swap_endian);
    Symbol              *map_PC_to_sym(uint64_t pc);
    Symbol              *pltSym;
    Vector<Symbol*>	*SymLst;		// list of func symbols
    Vector<Symbol*>	*SymLstByName;		// list of func symbols sorted by Name
    Vector<Reloc*>	*RelLst;		// list of text relocations
    Vector<Reloc*>	*RelPLTLst;		// list of PLT relocations
    Vector<Symbol*>	*LocalLst;		// list of local func symbols
    Vector<char*>	*LocalFile;		// list of local files
    Vector<int>		*LocalFileIdx;		// start index in LocalLst

    Elf         *openElf(char *fname, Stab_status &st);
    Map<const char*, Symbol*> *get_elf_symbols();
    Dwarf       *dwarf;

    bool        st_check_symtab, st_check_relocs;
    Function	*createFunction(LoadObject *lo, Module *module, Symbol *sym);
    void        fixSymtabAlias();

    // Interface with dwarf
    Dwarf       *openDwarf();

    Vector<Module*> *stabsModules;
    static char *get_type_name(int t);
};

#endif  /* _STABS_H */
