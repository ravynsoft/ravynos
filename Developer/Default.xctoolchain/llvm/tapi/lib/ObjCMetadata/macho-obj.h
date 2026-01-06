#ifndef LLVM_LIB_OBJCMETADATA_MACHO_OBJ_H
#define LLVM_LIB_OBJCMETADATA_MACHO_OBJ_H

#include <llvm/Support/SwapByteOrder.h>

// These are structs in the Objective-C meta data and read to produce the
// comments for disassembly.  While these are part of the ABI they are no
// public defintions.  So the are here not in include/llvm/Support/MachO.h .

// The cfstring object in a 64-bit Mach-O file.
struct cfstring64_t {
  uint64_t isa;        // class64_t * (64-bit pointer)
  uint64_t flags;      // flag bits
  uint64_t characters; // char * (64-bit pointer)
  uint64_t length;     // number of non-NULL characters in above
};

// The class object in a 64-bit Mach-O file.
struct class64_t {
  uint64_t isa;        // class64_t * (64-bit pointer)
  uint64_t superclass; // class64_t * (64-bit pointer)
  uint64_t cache;      // Cache (64-bit pointer)
  uint64_t vtable;     // IMP * (64-bit pointer)
  uint64_t data;       // class_ro64_t * (64-bit pointer)
};

struct class32_t {
  uint32_t isa;        /* class32_t * (32-bit pointer) */
  uint32_t superclass; /* class32_t * (32-bit pointer) */
  uint32_t cache;      /* Cache (32-bit pointer) */
  uint32_t vtable;     /* IMP * (32-bit pointer) */
  uint32_t data;       /* class_ro32_t * (32-bit pointer) */
};

struct class_ro64_t {
  uint32_t flags;
  uint32_t instanceStart;
  uint32_t instanceSize;
  uint32_t reserved;
  uint64_t ivarLayout;     // const uint8_t * (64-bit pointer)
  uint64_t name;           // const char * (64-bit pointer)
  uint64_t baseMethods;    // const method_list_t * (64-bit pointer)
  uint64_t baseProtocols;  // const protocol_list_t * (64-bit pointer)
  uint64_t ivars;          // const ivar_list_t * (64-bit pointer)
  uint64_t weakIvarLayout; // const uint8_t * (64-bit pointer)
  uint64_t baseProperties; // const struct objc_property_list (64-bit pointer)
};

struct class_ro32_t {
  uint32_t flags;
  uint32_t instanceStart;
  uint32_t instanceSize;
  uint32_t ivarLayout;     /* const uint8_t * (32-bit pointer) */
  uint32_t name;           /* const char * (32-bit pointer) */
  uint32_t baseMethods;    /* const method_list_t * (32-bit pointer) */
  uint32_t baseProtocols;  /* const protocol_list_t * (32-bit pointer) */
  uint32_t ivars;          /* const ivar_list_t * (32-bit pointer) */
  uint32_t weakIvarLayout; /* const uint8_t * (32-bit pointer) */
  uint32_t baseProperties; /* const struct objc_property_list *
                                                   (32-bit pointer) */
};

/* Values for class_ro{64,32}_t->flags */
#define RO_META (1 << 0)
#define RO_ROOT (1 << 1)
#define RO_HAS_CXX_STRUCTORS (1 << 2)

struct method_list64_t {
  uint32_t entsize;
  uint32_t count;
  /* struct method64_t first;  These structures follow inline */
};

struct method_list32_t {
  uint32_t entsize;
  uint32_t count;
  /* struct method32_t first;  These structures follow inline */
};

/* Values for method_list*_t->entsize. */
#define METHOD_LIST_ENTSIZE_FLAGS_MASK     0xFFFF0000
#define METHOD_LIST_ENTSIZE_VALUE_MASK     0x0000FFFF
#define METHOD_LIST_ENTSIZE_FLAG_RELATIVE  0x80000000

struct method64_t {
  uint64_t name;  /* SEL (64-bit pointer) */
  uint64_t types; /* const char * (64-bit pointer) */
  uint64_t imp;   /* IMP (64-bit pointer) */
};

struct method32_t {
  uint32_t name;  /* SEL (32-bit pointer) */
  uint32_t types; /* const char * (32-bit pointer) */
  uint32_t imp;   /* IMP (32-bit pointer) */
};

struct method_rel_t {
  int32_t name;   /* SEL reference (signed 32-bit offset from this field) */
  int32_t types;  /* const char* (signed 32-bit offset from this field) */
  int32_t imp;    /* IMP (signed 32-bit offset from this field) */
};

struct protocol_list64_t {
  uint64_t count; /* uintptr_t (a 64-bit value) */
  /* struct protocol64_t * list[0];  These pointers follow inline */
};

struct protocol_list32_t {
  uint32_t count; /* uintptr_t (a 32-bit value) */
  /* struct protocol32_t * list[0];  These pointers follow inline */
};

struct protocol64_t {
  uint64_t isa;                     /* id * (64-bit pointer) */
  uint64_t name;                    /* const char * (64-bit pointer) */
  uint64_t protocols;               /* struct protocol_list64_t *
                                                    (64-bit pointer) */
  uint64_t instanceMethods;         /* method_list_t * (64-bit pointer) */
  uint64_t classMethods;            /* method_list_t * (64-bit pointer) */
  uint64_t optionalInstanceMethods; /* method_list_t * (64-bit pointer) */
  uint64_t optionalClassMethods;    /* method_list_t * (64-bit pointer) */
  uint64_t instanceProperties;      /* struct objc_property_list *
                                                       (64-bit pointer) */
};

struct protocol32_t {
  uint32_t isa;                     /* id * (32-bit pointer) */
  uint32_t name;                    /* const char * (32-bit pointer) */
  uint32_t protocols;               /* struct protocol_list_t *
                                                    (32-bit pointer) */
  uint32_t instanceMethods;         /* method_list_t * (32-bit pointer) */
  uint32_t classMethods;            /* method_list_t * (32-bit pointer) */
  uint32_t optionalInstanceMethods; /* method_list_t * (32-bit pointer) */
  uint32_t optionalClassMethods;    /* method_list_t * (32-bit pointer) */
  uint32_t instanceProperties;      /* struct objc_property_list *
                                                       (32-bit pointer) */
};

struct ivar_list64_t {
  uint32_t entsize;
  uint32_t count;
  /* struct ivar64_t first;  These structures follow inline */
};

struct ivar_list32_t {
  uint32_t entsize;
  uint32_t count;
  /* struct ivar32_t first;  These structures follow inline */
};

struct ivar64_t {
  uint64_t offset; /* uintptr_t * (64-bit pointer) */
  uint64_t name;   /* const char * (64-bit pointer) */
  uint64_t type;   /* const char * (64-bit pointer) */
  uint32_t alignment;
  uint32_t size;
};

struct ivar32_t {
  uint32_t offset; /* uintptr_t * (32-bit pointer) */
  uint32_t name;   /* const char * (32-bit pointer) */
  uint32_t type;   /* const char * (32-bit pointer) */
  uint32_t alignment;
  uint32_t size;
};

struct objc_property_list64 {
  uint32_t entsize;
  uint32_t count;
  /* struct objc_property64 first;  These structures follow inline */
};

struct objc_property_list32 {
  uint32_t entsize;
  uint32_t count;
  /* struct objc_property32 first;  These structures follow inline */
};

struct objc_property64 {
  uint64_t name;       /* const char * (64-bit pointer) */
  uint64_t attributes; /* const char * (64-bit pointer) */
};

struct objc_property32 {
  uint32_t name;       /* const char * (32-bit pointer) */
  uint32_t attributes; /* const char * (32-bit pointer) */
};

struct category64_t {
  uint64_t name;               /* const char * (64-bit pointer) */
  uint64_t cls;                /* struct class_t * (64-bit pointer) */
  uint64_t instanceMethods;    /* struct method_list_t * (64-bit pointer) */
  uint64_t classMethods;       /* struct method_list_t * (64-bit pointer) */
  uint64_t protocols;          /* struct protocol_list_t * (64-bit pointer) */
  uint64_t instanceProperties; /* struct objc_property_list *
                                  (64-bit pointer) */
};

struct category32_t {
  uint32_t name;               /* const char * (32-bit pointer) */
  uint32_t cls;                /* struct class_t * (32-bit pointer) */
  uint32_t instanceMethods;    /* struct method_list_t * (32-bit pointer) */
  uint32_t classMethods;       /* struct method_list_t * (32-bit pointer) */
  uint32_t protocols;          /* struct protocol_list_t * (32-bit pointer) */
  uint32_t instanceProperties; /* struct objc_property_list *
                                  (32-bit pointer) */
};

struct objc_image_info64 {
  uint32_t version;
  uint32_t flags;
};
struct objc_image_info32 {
  uint32_t version;
  uint32_t flags;
};
struct imageInfo_t {
  uint32_t version;
  uint32_t flags;
};
/* masks for objc_image_info.flags */
#define OBJC_IMAGE_IS_REPLACEMENT (1 << 0)
#define OBJC_IMAGE_SUPPORTS_GC (1 << 1)

struct message_ref64 {
  uint64_t imp; /* IMP (64-bit pointer) */
  uint64_t sel; /* SEL (64-bit pointer) */
};

struct message_ref32 {
  uint32_t imp; /* IMP (32-bit pointer) */
  uint32_t sel; /* SEL (32-bit pointer) */
};

// Objective-C 1 (32-bit only) meta data structs.

struct objc_module_t {
  uint32_t version;
  uint32_t size;
  uint32_t name;   /* char * (32-bit pointer) */
  uint32_t symtab; /* struct objc_symtab * (32-bit pointer) */
};

struct objc_symtab_t {
  uint32_t sel_ref_cnt;
  uint32_t refs; /* SEL * (32-bit pointer) */
  uint16_t cls_def_cnt;
  uint16_t cat_def_cnt;
  // uint32_t defs[1];        /* void * (32-bit pointer) variable size */
};

struct objc_class_t {
  uint32_t isa;         /* struct objc_class * (32-bit pointer) */
  uint32_t super_class; /* struct objc_class * (32-bit pointer) */
  uint32_t name;        /* const char * (32-bit pointer) */
  int32_t version;
  int32_t info;
  int32_t instance_size;
  uint32_t ivars;       /* struct objc_ivar_list * (32-bit pointer) */
  uint32_t methodLists; /* struct objc_method_list ** (32-bit pointer) */
  uint32_t cache;       /* struct objc_cache * (32-bit pointer) */
  uint32_t protocols;   /* struct objc_protocol_list * (32-bit pointer) */
};

#define CLS_GETINFO(cls, infomask) ((cls)->info & (infomask))
// class is not a metaclass
#define CLS_CLASS 0x1
// class is a metaclass
#define CLS_META 0x2

struct objc_category_t {
  uint32_t category_name;    /* char * (32-bit pointer) */
  uint32_t class_name;       /* char * (32-bit pointer) */
  uint32_t instance_methods; /* struct objc_method_list * (32-bit pointer) */
  uint32_t class_methods;    /* struct objc_method_list * (32-bit pointer) */
  uint32_t protocols;        /* struct objc_protocol_list * (32-bit ptr) */
};

struct objc_ivar_t {
  uint32_t ivar_name; /* char * (32-bit pointer) */
  uint32_t ivar_type; /* char * (32-bit pointer) */
  int32_t ivar_offset;
};

struct objc_ivar_list_t {
  int32_t ivar_count;
  // struct objc_ivar_t ivar_list[1];          /* variable length structure */
};

struct objc_method_list_t {
  uint32_t obsolete; /* struct objc_method_list * (32-bit pointer) */
  int32_t method_count;
  // struct objc_method_t method_list[1];      /* variable length structure */
};

struct objc_method_t {
  uint32_t method_name;  /* SEL, aka struct objc_selector * (32-bit pointer) */
  uint32_t method_types; /* char * (32-bit pointer) */
  uint32_t method_imp;   /* IMP, aka function pointer, (*IMP)(id, SEL, ...)
                            (32-bit pointer) */
};

struct objc_protocol_list_t {
  uint32_t next; /* struct objc_protocol_list * (32-bit pointer) */
  int32_t count;
  // uint32_t list[1];   /* Protocol *, aka struct objc_protocol_t *
  //                        (32-bit pointer) */
};

struct objc_protocol_t {
  uint32_t isa;              /* struct objc_class * (32-bit pointer) */
  uint32_t protocol_name;    /* char * (32-bit pointer) */
  uint32_t protocol_list;    /* struct objc_protocol_list * (32-bit pointer) */
  uint32_t instance_methods; /* struct objc_method_description_list *
                                (32-bit pointer) */
  uint32_t class_methods;    /* struct objc_method_description_list *
                                (32-bit pointer) */
};

struct objc_method_description_list_t {
  int32_t count;
  // struct objc_method_description_t list[1];
};

struct objc_method_description_t {
  uint32_t name;  /* SEL, aka struct objc_selector * (32-bit pointer) */
  uint32_t types; /* char * (32-bit pointer) */
};

inline void swapStruct(struct cfstring64_t &cfs) {
  llvm::sys::swapByteOrder(cfs.isa);
  llvm::sys::swapByteOrder(cfs.flags);
  llvm::sys::swapByteOrder(cfs.characters);
  llvm::sys::swapByteOrder(cfs.length);
}

inline void swapStruct(struct class64_t &c) {
  llvm::sys::swapByteOrder(c.isa);
  llvm::sys::swapByteOrder(c.superclass);
  llvm::sys::swapByteOrder(c.cache);
  llvm::sys::swapByteOrder(c.vtable);
  llvm::sys::swapByteOrder(c.data);
}

inline void swapStruct(struct class32_t &c) {
  llvm::sys::swapByteOrder(c.isa);
  llvm::sys::swapByteOrder(c.superclass);
  llvm::sys::swapByteOrder(c.cache);
  llvm::sys::swapByteOrder(c.vtable);
  llvm::sys::swapByteOrder(c.data);
}

inline void swapStruct(struct class_ro64_t &cro) {
  llvm::sys::swapByteOrder(cro.flags);
  llvm::sys::swapByteOrder(cro.instanceStart);
  llvm::sys::swapByteOrder(cro.instanceSize);
  llvm::sys::swapByteOrder(cro.reserved);
  llvm::sys::swapByteOrder(cro.ivarLayout);
  llvm::sys::swapByteOrder(cro.name);
  llvm::sys::swapByteOrder(cro.baseMethods);
  llvm::sys::swapByteOrder(cro.baseProtocols);
  llvm::sys::swapByteOrder(cro.ivars);
  llvm::sys::swapByteOrder(cro.weakIvarLayout);
  llvm::sys::swapByteOrder(cro.baseProperties);
}

inline void swapStruct(struct class_ro32_t &cro) {
  llvm::sys::swapByteOrder(cro.flags);
  llvm::sys::swapByteOrder(cro.instanceStart);
  llvm::sys::swapByteOrder(cro.instanceSize);
  llvm::sys::swapByteOrder(cro.ivarLayout);
  llvm::sys::swapByteOrder(cro.name);
  llvm::sys::swapByteOrder(cro.baseMethods);
  llvm::sys::swapByteOrder(cro.baseProtocols);
  llvm::sys::swapByteOrder(cro.ivars);
  llvm::sys::swapByteOrder(cro.weakIvarLayout);
  llvm::sys::swapByteOrder(cro.baseProperties);
}

inline void swapStruct(struct method_list64_t &ml) {
  llvm::sys::swapByteOrder(ml.entsize);
  llvm::sys::swapByteOrder(ml.count);
}

inline void swapStruct(struct method_list32_t &ml) {
  llvm::sys::swapByteOrder(ml.entsize);
  llvm::sys::swapByteOrder(ml.count);
}

inline void swapStruct(struct method64_t &m) {
  llvm::sys::swapByteOrder(m.name);
  llvm::sys::swapByteOrder(m.types);
  llvm::sys::swapByteOrder(m.imp);
}

inline void swapStruct(struct method32_t &m) {
  llvm::sys::swapByteOrder(m.name);
  llvm::sys::swapByteOrder(m.types);
  llvm::sys::swapByteOrder(m.imp);
}

inline void swapStruct(struct method_rel_t &m) {
  llvm::sys::swapByteOrder(m.name);
  llvm::sys::swapByteOrder(m.types);
  llvm::sys::swapByteOrder(m.imp);
}

inline void swapStruct(struct protocol_list64_t &pl) {
  llvm::sys::swapByteOrder(pl.count);
}

inline void swapStruct(struct protocol_list32_t &pl) {
  llvm::sys::swapByteOrder(pl.count);
}

inline void swapStruct(struct protocol64_t &p) {
  llvm::sys::swapByteOrder(p.isa);
  llvm::sys::swapByteOrder(p.name);
  llvm::sys::swapByteOrder(p.protocols);
  llvm::sys::swapByteOrder(p.instanceMethods);
  llvm::sys::swapByteOrder(p.classMethods);
  llvm::sys::swapByteOrder(p.optionalInstanceMethods);
  llvm::sys::swapByteOrder(p.optionalClassMethods);
  llvm::sys::swapByteOrder(p.instanceProperties);
}

inline void swapStruct(struct protocol32_t &p) {
  llvm::sys::swapByteOrder(p.isa);
  llvm::sys::swapByteOrder(p.name);
  llvm::sys::swapByteOrder(p.protocols);
  llvm::sys::swapByteOrder(p.instanceMethods);
  llvm::sys::swapByteOrder(p.classMethods);
  llvm::sys::swapByteOrder(p.optionalInstanceMethods);
  llvm::sys::swapByteOrder(p.optionalClassMethods);
  llvm::sys::swapByteOrder(p.instanceProperties);
}

inline void swapStruct(struct ivar_list64_t &il) {
  llvm::sys::swapByteOrder(il.entsize);
  llvm::sys::swapByteOrder(il.count);
}

inline void swapStruct(struct ivar_list32_t &il) {
  llvm::sys::swapByteOrder(il.entsize);
  llvm::sys::swapByteOrder(il.count);
}

inline void swapStruct(struct ivar64_t &i) {
  llvm::sys::swapByteOrder(i.offset);
  llvm::sys::swapByteOrder(i.name);
  llvm::sys::swapByteOrder(i.type);
  llvm::sys::swapByteOrder(i.alignment);
  llvm::sys::swapByteOrder(i.size);
}

inline void swapStruct(struct ivar32_t &i) {
  llvm::sys::swapByteOrder(i.offset);
  llvm::sys::swapByteOrder(i.name);
  llvm::sys::swapByteOrder(i.type);
  llvm::sys::swapByteOrder(i.alignment);
  llvm::sys::swapByteOrder(i.size);
}

inline void swapStruct(struct objc_property_list64 &pl) {
  llvm::sys::swapByteOrder(pl.entsize);
  llvm::sys::swapByteOrder(pl.count);
}

inline void swapStruct(struct objc_property_list32 &pl) {
  llvm::sys::swapByteOrder(pl.entsize);
  llvm::sys::swapByteOrder(pl.count);
}

inline void swapStruct(struct objc_property64 &op) {
  llvm::sys::swapByteOrder(op.name);
  llvm::sys::swapByteOrder(op.attributes);
}

inline void swapStruct(struct objc_property32 &op) {
  llvm::sys::swapByteOrder(op.name);
  llvm::sys::swapByteOrder(op.attributes);
}

inline void swapStruct(struct category64_t &c) {
  llvm::sys::swapByteOrder(c.name);
  llvm::sys::swapByteOrder(c.cls);
  llvm::sys::swapByteOrder(c.instanceMethods);
  llvm::sys::swapByteOrder(c.classMethods);
  llvm::sys::swapByteOrder(c.protocols);
  llvm::sys::swapByteOrder(c.instanceProperties);
}

inline void swapStruct(struct category32_t &c) {
  llvm::sys::swapByteOrder(c.name);
  llvm::sys::swapByteOrder(c.cls);
  llvm::sys::swapByteOrder(c.instanceMethods);
  llvm::sys::swapByteOrder(c.classMethods);
  llvm::sys::swapByteOrder(c.protocols);
  llvm::sys::swapByteOrder(c.instanceProperties);
}

inline void swapStruct(struct objc_image_info64 &o) {
  llvm::sys::swapByteOrder(o.version);
  llvm::sys::swapByteOrder(o.flags);
}

inline void swapStruct(struct objc_image_info32 &o) {
  llvm::sys::swapByteOrder(o.version);
  llvm::sys::swapByteOrder(o.flags);
}

inline void swapStruct(struct imageInfo_t &o) {
  llvm::sys::swapByteOrder(o.version);
  llvm::sys::swapByteOrder(o.flags);
}

inline void swapStruct(struct message_ref64 &mr) {
  llvm::sys::swapByteOrder(mr.imp);
  llvm::sys::swapByteOrder(mr.sel);
}

inline void swapStruct(struct message_ref32 &mr) {
  llvm::sys::swapByteOrder(mr.imp);
  llvm::sys::swapByteOrder(mr.sel);
}

inline void swapStruct(struct objc_module_t &module) {
  llvm::sys::swapByteOrder(module.version);
  llvm::sys::swapByteOrder(module.size);
  llvm::sys::swapByteOrder(module.name);
  llvm::sys::swapByteOrder(module.symtab);
}

inline void swapStruct(struct objc_symtab_t &symtab) {
  llvm::sys::swapByteOrder(symtab.sel_ref_cnt);
  llvm::sys::swapByteOrder(symtab.refs);
  llvm::sys::swapByteOrder(symtab.cls_def_cnt);
  llvm::sys::swapByteOrder(symtab.cat_def_cnt);
}

inline void swapStruct(struct objc_class_t &objc_class) {
  llvm::sys::swapByteOrder(objc_class.isa);
  llvm::sys::swapByteOrder(objc_class.super_class);
  llvm::sys::swapByteOrder(objc_class.name);
  llvm::sys::swapByteOrder(objc_class.version);
  llvm::sys::swapByteOrder(objc_class.info);
  llvm::sys::swapByteOrder(objc_class.instance_size);
  llvm::sys::swapByteOrder(objc_class.ivars);
  llvm::sys::swapByteOrder(objc_class.methodLists);
  llvm::sys::swapByteOrder(objc_class.cache);
  llvm::sys::swapByteOrder(objc_class.protocols);
}

inline void swapStruct(struct objc_category_t &objc_category) {
  llvm::sys::swapByteOrder(objc_category.category_name);
  llvm::sys::swapByteOrder(objc_category.class_name);
  llvm::sys::swapByteOrder(objc_category.instance_methods);
  llvm::sys::swapByteOrder(objc_category.class_methods);
  llvm::sys::swapByteOrder(objc_category.protocols);
}

inline void swapStruct(struct objc_ivar_list_t &objc_ivar_list) {
  llvm::sys::swapByteOrder(objc_ivar_list.ivar_count);
}

inline void swapStruct(struct objc_ivar_t &objc_ivar) {
  llvm::sys::swapByteOrder(objc_ivar.ivar_name);
  llvm::sys::swapByteOrder(objc_ivar.ivar_type);
  llvm::sys::swapByteOrder(objc_ivar.ivar_offset);
}

inline void swapStruct(struct objc_method_list_t &method_list) {
  llvm::sys::swapByteOrder(method_list.obsolete);
  llvm::sys::swapByteOrder(method_list.method_count);
}

inline void swapStruct(struct objc_method_t &method) {
  llvm::sys::swapByteOrder(method.method_name);
  llvm::sys::swapByteOrder(method.method_types);
  llvm::sys::swapByteOrder(method.method_imp);
}

inline void swapStruct(struct objc_protocol_list_t &protocol_list) {
  llvm::sys::swapByteOrder(protocol_list.next);
  llvm::sys::swapByteOrder(protocol_list.count);
}

inline void swapStruct(struct objc_protocol_t &protocol) {
  llvm::sys::swapByteOrder(protocol.isa);
  llvm::sys::swapByteOrder(protocol.protocol_name);
  llvm::sys::swapByteOrder(protocol.protocol_list);
  llvm::sys::swapByteOrder(protocol.instance_methods);
  llvm::sys::swapByteOrder(protocol.class_methods);
}

inline void swapStruct(struct objc_method_description_list_t &mdl) {
  llvm::sys::swapByteOrder(mdl.count);
}

inline void swapStruct(struct objc_method_description_t &md) {
  llvm::sys::swapByteOrder(md.name);
  llvm::sys::swapByteOrder(md.types);
}

template <typename T> inline void swapStruct(T &num) {
  llvm::sys::swapByteOrder(num);
}

#endif
