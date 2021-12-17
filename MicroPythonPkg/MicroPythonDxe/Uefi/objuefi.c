/** @file
  Object definition specified to EDK2/UEFI.

Copyright (c) 2017 - 2018, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <string.h>
#include <assert.h>
#include <stdint.h>

#include <py/nlr.h>
#include <py/runtime0.h>
#include <py/runtime.h>
#include <py/binary.h>
#include <py/objstr.h>
#include <py/objarray.h>
#include <py/objfun.h>
#include <py/objint.h>

#include "upy.h"
#include "objuefi.h"

STATIC mp_obj_t mem_unary_op(mp_unary_op_t op, mp_obj_t o_in);

STATIC char type_size [] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //          'B'                  'E'                 'F'            'G'               'H'             'I'                        'L'                'N'      'O'
  0, 0, sizeof(UINT8), 0, 0, sizeof(EFI_STATUS), sizeof(void*), sizeof(EFI_GUID), sizeof(UINT16), sizeof(unsigned int), 0, 0, sizeof(UINT32), 0, sizeof(UINTN), 0,
  //     'P'           'Q'                   'T'
  sizeof(void*), sizeof(UINT64), 0, 0, sizeof(EFI_HANDLE), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  //       'a'            'b'                          'h'            'i'                'l'               'n'
  0, sizeof(CHAR8), sizeof(INT8), 0, 0, 0, 0, 0, sizeof(INT16), sizeof(int), 0, 0, sizeof(INT32), 0, sizeof(INTN), 0,
  //       'q'                     'u'
  0, sizeof(INT64), 0, 0, 0, sizeof(CHAR16), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

UINTN get_type_size(const char *typespec)
{
  mp_uint_t   len, i;

  len = 0;
  for (i = 0; unichar_isdigit(typespec[i]); ++i) {
    len = len * 10 + typespec[i] - '0';
  }

  if (typespec[i] > 127) {
    return 0;
  }
  return (len * type_size[(UINT8)typespec[i]]);
}

UINT64 mpz_hash64(const mpz_t *z) {
  UINT64 val = 0;
  mpz_dig_t *d = z->dig + z->len;

  while (d-- > z->dig) {
    val = LShiftU64(val, MPZ_DIG_SIZE) | *d;
  }

  if (z->neg != 0) {
    val = -(INT64)val;
  }

  return val;
}

UINT64 get_uint64(mp_obj_t intobj)
{
  if (MP_OBJ_IS_TYPE(intobj, &mp_type_int)) {
#if MICROPY_LONGINT_IMPL == MICROPY_LONGINT_IMPL_MPZ
    const mp_obj_int_t *self = MP_OBJ_TO_PTR(intobj);
    return mpz_hash64(&self->mpz);
#elif MICROPY_LONGINT_IMPL == MICROPY_LONGINT_IMPL_LONGLONG
    return ((mp_obj_int_t *)MP_OBJ_TO_PTR(intobj))->val;
#else
#endif
  } else {
    return (INT64)mp_obj_get_int_truncated(intobj);
  }
}

STATIC void efi_mem_print(const mp_print_t *print, mp_obj_t o_in, mp_print_kind_t kind)
{
  (void)kind;

  mp_obj_mem_t *self = MP_OBJ_TO_PTR(o_in);

  mp_printf(print, "0x%x", self->addr);
  if (self->bitnum > 0) {
    mp_printf(print, "[%02d:%02d]", self->bitoff, self->bitoff + self->bitnum - 1);
  }
}

STATIC void efi_guid_print(const mp_print_t *print, mp_obj_t o_in, mp_print_kind_t kind)
{
  (void)kind;

  UINT8 *c;

  mp_obj_mem_t *self = MP_OBJ_TO_PTR(o_in);

  c = (UINT8 *)(self->addr);

  mp_printf(print,  "%02X%02X%02X%02X", c[3], c[2], c[1], c[0]);
  mp_printf(print, "-%02X%02X", c[5], c[4]);
  mp_printf(print, "-%02X%02X", c[7], c[6]);
  mp_printf(print, "-%02X%02X%02X%02X%02X%02X%02X%02X", c[8], c[9], c[10], c[11], c[12], c[13], c[14], c[15]);
}

mp_obj_t load_obj_by_namespace(const char *namespace)
{
  char        name[256];
  mp_uint_t   index;
  mp_obj_t    varobj;

  index = 0;
  varobj = mp_const_none;
  while (index < sizeof(name)) {
    if (*namespace != '\0' && *namespace != '.') {
      name[index] = *namespace;
      ++index;
      ++namespace;
      continue;
    }

    name[index] = '\0';
    if (varobj == mp_const_none) {
      varobj = mp_load_name(qstr_from_str(name));
    } else {
      varobj = mp_load_attr(varobj, qstr_from_str(name));
    }

    if (varobj == mp_const_none || *namespace == '\0') {
      break;
    }

    index = 0;
    ++namespace;
  }

  return varobj;
}

mp_obj_t resolve_type_by_name(const char *namestr)
{
  mp_obj_t    varobj;
  size_t      len;

  varobj = mp_const_none;
  while (TRUE) {
    if (namestr == NULL || namestr[0] != '#') {
      break;
    }

    varobj = load_obj_by_namespace(namestr + 1);
    if (!MP_OBJ_IS_STR(varobj)) {
      break;
    }

    namestr = mp_obj_str_get_data(varobj, &len);
  }

  return varobj;
}

STATIC mp_uint_t efi_mem_get_size(mp_obj_t mobj)
{
  mp_obj_mem_t  *mem = MP_OBJ_TO_PTR(mobj);

  if (mem->bitnum == 0) {
    if (mem->size > 0) {
      return mem->size;
    } else {
      return mem->typesize;
    }
  } else if (mem->bitnum > 0) {
    return ((mem->bitoff + mem->bitnum) / (mem->typesize * 8)) * mem->typesize;
  } else {
    mp_raise_msg(&mp_type_ValueError, "cannot get size information");
  }

  return 0;
}

STATIC mp_obj_t efi_mem_typecast(mp_obj_t self_in, mp_obj_t typedesc);
STATIC void efi_mem_typecast_by_dict(mp_obj_mem_t *self, mp_obj_t dictobj)
{
  mp_obj_dict_t     *typedesc = MP_OBJ_TO_PTR(dictobj);
  mp_obj_dict_t     *fields;
  UINT8             *addr = (UINT8 *)(self->addr);
  const char        *typespec;
  mp_obj_mem_t      *thisfield;
  mp_obj_mem_t      *lastfield;

  if (!MP_OBJ_IS_TYPE(dictobj, &mp_type_ordereddict)) {
    mp_raise_msg(&mp_type_TypeError, "_collections.OrderedDict must be used to describe object/structure");
  }

  fields = MP_OBJ_TO_PTR(mp_obj_new_dict(typedesc->map.used));
  fields->map.all_keys_are_qstrs = FALSE;
  fields->map.is_ordered = TRUE;
  fields->map.used = typedesc->map.used;

  lastfield = NULL;
  for (int i = 0; i < typedesc->map.used; i++) {
    typespec = mp_obj_str_get_str(typedesc->map.table[i].value);
    thisfield = efi_mem_new(addr, typespec, self->readonly, self->page);

    fields->map.table[i].key = typedesc->map.table[i].key;
    fields->map.table[i].value = thisfield;

    if (lastfield != NULL && lastfield->bitnum > 0 && thisfield->bitnum > 0) {
      thisfield->bitoff = lastfield->bitoff + lastfield->bitnum;
    }

    addr += efi_mem_get_size(thisfield);
    lastfield = thisfield;
  }

  self->typesize = (mp_uint_t)(addr - (UINT8 *)self->addr);
  self->fields = fields;
}

STATIC void efi_mem_typecast_by_str(mp_obj_mem_t *self, const char *typespec)
{
  mp_uint_t       speclen;
  mp_uint_t       num;
  uint8_t         bitfield;

  num = 0;
  bitfield = FALSE;
  self->typespec = typespec;
  speclen = strlen(typespec);
  for (int i = 0; i < speclen; ++i) {
    const char typecode = typespec[i];

    if (unichar_isdigit(typecode)) {
      num = num * 10 + (typecode - '0');
    } else if (unichar_isalpha(typecode)) {
      num = 0;

      if ((UINTN)typecode < 128) {
        self->typesize = type_size[(UINT8)typecode];
      } else {
        mp_raise_msg(&mp_type_ValueError, "Not supported typecode");
      }

      switch (typecode) {
      case 'O':
        if (typespec[i + 1] != '#') {
          mp_raise_msg(&mp_type_ValueError, "Object/structure '#<name>' must be given after 'O' typecode");
        }
        i += 1;
      case '#':
        efi_mem_typecast(MP_OBJ_FROM_PTR(self), resolve_type_by_name(typespec + i));
        i = speclen;
        break;

      case 'F':
        self->base.type = &mp_type_fptr;

        // check function prototype
        if ((i + 1) < speclen) {
          mp_obj_tuple_t  *args = MP_OBJ_FROM_PTR(mp_obj_new_tuple(11, NULL));
          mp_uint_t       arg_index = 0;
          mp_uint_t       arg_start;

          for (arg_start = i + 1; i < speclen && arg_index < 11; ++i) {
            if (typespec[i] == '(') {
              // check return value type
              if (i > arg_start) {
                args->items[arg_index++] = mp_obj_new_str(typespec + arg_start, i - arg_start);
              } else {
                // no return value
                args->items[arg_index++] = mp_const_none;;
              }
              arg_start = i + 1;
            } else if (typespec[i] == ',' || typespec[i] == ')') {
              if (i > arg_start) {
                args->items[arg_index++] = mp_obj_new_str(typespec + arg_start, i - arg_start);
                if (typespec[arg_start] == 'V') {
                  self->hasvarg = 1;
                }
              }
              arg_start = i + 1;

              if (typespec[i] == ')') {
                break;
              }
            }
          }
          args->len = arg_index;
          self->args = args;
        }
        break;

      case 'T':
      case 'P':
        i = speclen;
        break;

      case 'G':
        self->base.type = &mp_type_guid;
        i = speclen;
        break;

      default:
        break;
      }
    } else {
      switch (typecode) {
      case ':':
        if (self->typesize > sizeof(UINT64)) {
          mp_raise_msg(&mp_type_ValueError, "Bitfield supports base integer type only.");
        }
        self->bitoff = num;
        bitfield = TRUE;
        num = 0;
        break;

      default:
        mp_raise_msg(&mp_type_ValueError, "Invalid type format");
      }
    }
  }

  if (bitfield) {
    self->bitnum = (num > 0) ? num : 1;
  }
}

STATIC mp_obj_t efi_mem_typecast(mp_obj_t self_in, mp_obj_t typedesc)
{
  mp_obj_mem_t *self = MP_OBJ_TO_PTR(self_in);

  if (typedesc == mp_const_none) {
    self->typespec = NULL;
    self->typesize = 1;
  } else if (MP_OBJ_IS_TYPE(typedesc, &mp_type_ordereddict)) {
    efi_mem_typecast_by_dict(self, typedesc);
  } else if (MP_OBJ_IS_STR(typedesc)) {
    mp_uint_t   len, i;
    const char  *typespec;

    typespec = mp_obj_str_get_str(typedesc);
    len = 0;
    i = 0;
    if (typespec != NULL) {
      for (; unichar_isdigit(typespec[i]); ++i) {
        len = len * 10 + typespec[i] - '0';
      }
    }
    len = (len > 0) ? len : 1;

    efi_mem_typecast_by_str(self, typespec + i);
    if (self->size == 0) {
      self->size = len * efi_mem_get_size(self);
    }
  } else {
    mp_raise_msg(&mp_type_ValueError, "Invalid type object");
  }

  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(efi_mem_typecast_obj, efi_mem_typecast);

mp_obj_mem_t* efi_mem_new_n(void *buf, mp_uint_t len, const char *typespec, mp_uint_t readonly, mp_uint_t pmem)
{
  mp_obj_mem_t    *self;

  self = m_new_obj(mp_obj_mem_t);
  self->base.type = &mp_type_mem;
  self->fields = NULL;
  self->typeattr = 0;
  self->readonly = (readonly)? 1 : 0;
  self->typespec = NULL;
  self->addr = buf;

  if (typespec != NULL) {
    efi_mem_typecast_by_str(self, typespec);
  } else {
    self->typesize = 1;
  }
  self->size = self->typesize * len;
  self->overrw = (self->size > 0) ? 0 : 1;
  self->page = (pmem)? 1 : 0;

  return self;
}

mp_obj_mem_t* efi_mem_new(void *buf, const char *typespec, mp_uint_t readonly, mp_uint_t pmem)
{
  mp_uint_t     len;
  mp_uint_t     i;

  len = 0;
  i = 0;
  if (typespec != NULL) {
    for (; unichar_isdigit(typespec[i]); ++i) {
      len = len * 10 + typespec[i] - '0';
    }
    len = (len > 0) ? len : 1;
  }

  return efi_mem_new_n(buf, len, typespec + i, readonly, pmem);
}

mp_obj_t  efi_buffer_new(void *buf, mp_uint_t len, mp_uint_t readonly)
{
  return MP_OBJ_FROM_PTR(efi_mem_new_n(buf, len, "B", readonly, FALSE));
}

mp_obj_t efi_guid_new(EFI_GUID *guid)
{
  mp_obj_mem_t *self = m_new_obj(mp_obj_mem_t);

  self->base.type = &mp_type_guid;
  self->typeattr = 0;
  self->readonly = TRUE;
  self->typesize = sizeof(EFI_GUID);
  self->size = sizeof(EFI_GUID);
  self->fields = NULL;
  self->typespec = "G";
  if (guid != NULL) {
    self->addr = guid;
  } else {
    self->addr = AllocateZeroPool(sizeof(EFI_GUID));
    ASSERT(self->addr != NULL);
  }

  return MP_OBJ_FROM_PTR(self);
}

STATIC void efi_mem_update_addr(mp_obj_mem_t *memobj, void *addr)
{
  memobj->addr = (UINT8 *)addr + (UINTN)memobj->addr;
  if (memobj->fields != NULL && memobj->typespec != NULL && memobj->typespec[0] != 'F') {
    for (int index = 0; index < memobj->fields->map.used; ++index) {
      efi_mem_update_addr(memobj->fields->map.table[index].value, addr);
    }
  }
}

STATIC mp_obj_t efi_mem_make_new(const mp_obj_type_t *type_in, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
  void          *addr;
  mp_uint_t     len;
  mp_uint_t     readonly;
  mp_uint_t     pmem;
  const char    *typespec;
  mp_obj_mem_t  *memobj;

  (void)type_in;
  // args = size_or_typedesc, addr, alloc_page, readonly
  mp_arg_check_num(n_args, n_kw, 0, 4, false);

  pmem = FALSE;
  typespec = NULL;
  addr = NULL;
  len = readonly = 0;
  switch (n_args) {
  case 4:
    readonly = mp_obj_get_int(args[3]);

  case 3:
    pmem = mp_obj_get_int(args[2]);

  case 2:
    addr = (void *)(UINTN)get_uint64(args[1]);

  case 1:
    if (MP_OBJ_IS_STR(args[0])) {
      typespec = mp_obj_str_get_str(args[0]);
      if (typespec != NULL && strlen(typespec) == 0) {
        typespec = NULL;
      }
      len = 0;
    } else if (MP_OBJ_IS_INT(args[0])) {
      len = mp_obj_get_int(args[0]);
    } else {
      nlr_raise(mp_obj_new_exception_msg_varg(
                  &mp_type_TypeError,
                  "The first argument must be string or integer but %s is got.",
                  mp_obj_get_type_str(args[0]))
                );
    }
  }

  if (typespec != NULL) {
    memobj = efi_mem_new(addr, typespec, readonly, pmem);
    len = (memobj->size > 0) ? memobj->size : memobj->typesize;
  } else {
    memobj = efi_mem_new_n(addr, len, "B", readonly, pmem);
    if (pmem) {
      len = EFI_PAGES_TO_SIZE(len);
    }
  }

  if (addr == NULL && len > 0) {
    if (memobj->page) {
      addr = AllocatePages(EFI_SIZE_TO_PAGES(len));
    } else {
      addr = AllocateZeroPool(len);
    }

    efi_mem_update_addr(memobj, addr);
  }
  return MP_OBJ_FROM_PTR(memobj);
}

STATIC mp_obj_t efi_guid_make_new(const mp_obj_type_t *type_in, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
  size_t      len;
  const char  *guidstr;
  mp_obj_t    guidobj;

  (void)type_in;
  // empty guid object is not allowed
  mp_arg_check_num(n_args, n_kw, 1, 1, false);

  guidstr = mp_obj_str_get_data(args[0], &len);
  if (guidstr == NULL || len != (sizeof(gGuidFormat) - 1)) {
    nlr_raise(mp_obj_new_exception_msg_varg(
                &mp_type_ValueError,
                "Invalid format of GUID: %s. It must be like %s.",
                guidstr, gGuidFormat)
              );
  }

  guidobj = efi_guid_new(NULL);
  if (!StringToGuid(guidstr, ((mp_obj_mem_t *)MP_OBJ_TO_PTR(guidobj))->addr)) {
    nlr_raise(mp_obj_new_exception_msg_varg(
                &mp_type_ValueError,
                "Invalid format of GUID: %s. It must be like %s.",
                guidstr, gGuidFormat)
              );
  }

  return guidobj;
}

STATIC mp_obj_t efi_fptr_make_new(const mp_obj_type_t *type_in, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
  mp_obj_mem_t  *fptr;
  void          *func;

  (void)type_in;
  mp_arg_check_num(n_args, n_kw, 1, 2, false);

  func = (void *)mp_obj_get_int(args[0]);
  fptr = efi_mem_new(func, "F", TRUE, FALSE);

  if (n_args == 2) {
    if (MP_OBJ_IS_TYPE(args[1], &mp_type_tuple) || MP_OBJ_IS_TYPE(args[1], &mp_type_list)) {
      fptr->args = MP_OBJ_TO_PTR(args[1]);
    } else {
      mp_raise_msg(&mp_type_ValueError, "function arguments must be specified in tuple or list");
    }
  }

  return MP_OBJ_FROM_PTR(fptr);
}

STATIC mp_obj_t get_bits(mp_obj_mem_t *self)
{
  mp_obj_t        value =  mp_const_none;
  void            *addr = self->addr;
  UINTN           bitend = self->bitoff + self->bitnum - 1;

  if (bitend < 8) {
    value = MP_OBJ_NEW_SMALL_INT(BitFieldRead8(*((UINT8 *)addr), self->bitoff, bitend));
  } else if (bitend < 16) {
    value = MP_OBJ_NEW_SMALL_INT(BitFieldRead16(*((UINT16 *)addr), self->bitoff, bitend));
  } else if (bitend < 32) {
    value = mp_obj_new_int_from_uint(BitFieldRead32(*((UINT32 *)addr), self->bitoff, bitend));
  } else {
    value = mp_obj_new_int_from_ull(BitFieldRead64(*((UINT64 *)addr), self->bitoff, bitend));
  }

  return value;
}

STATIC void set_bits(mp_obj_mem_t *self, mp_obj_t value)
{
  void            *addr = self->addr;
  UINTN           bitend = self->bitoff + self->bitnum - 1;
  UINT64          mask = LShiftU64(1, self->bitnum) - 1;

  switch (self->typespec[0]) {
  case 'B':
    ((UINT8 *)addr)[0] = BitFieldWrite8(((UINT8 *)addr)[0],
                                        self->bitoff,
                                        bitend,
                                        (UINT8)mp_obj_get_int_truncated(value) & mask);
    break;

  case 'H':
    ((UINT16 *)addr)[0] = BitFieldWrite16(((UINT16 *)addr)[0],
                                          self->bitoff,
                                          bitend,
                                          (UINT16)mp_obj_get_int_truncated(value) & mask);
    break;

  case 'I':
  case 'L':
    ((UINT32 *)addr)[0] = BitFieldWrite32(((UINT32 *)addr)[0],
                                          self->bitoff,
                                          bitend,
                                          (UINT32)mp_obj_get_int_truncated(value) & mask);
    break;

  case 'N':
    if (sizeof(UINTN) == 4) {
      ((UINT32 *)addr)[0] = BitFieldWrite32(((UINT32 *)addr)[0],
                                            self->bitoff,
                                            bitend,
                                            (UINT32)mp_obj_get_int_truncated(value) & mask);
    } else {
      ((UINT64 *)addr)[0] = BitFieldWrite64(((UINT64 *)addr)[0],
                                            self->bitoff,
                                            bitend,
                                            get_uint64(value) & mask);
    }
    break;

  case 'Q':
    ((UINT64 *)addr)[0] = BitFieldWrite64(((UINT64 *)addr)[0],
                                          self->bitoff,
                                          bitend,
                                          get_uint64(value) & mask);
    break;

  default:
    mp_raise_msg(&mp_type_ValueError, "Not supported base type of bitfield to dereference");
  }
}

STATIC mp_obj_t get_value(mp_obj_mem_t *self, mp_obj_t index_in)
{
  mp_obj_t        value =  mp_const_none;
  mp_obj_mem_t    *memobj;
  void            *addr = self->addr;
  char            *str;
  char            typechar;
  mp_uint_t       len;

  if (addr == NULL) {
    return mp_const_false;
  }

  if (self->bitnum > 0) {
    return get_bits(self);
  }

  len = (mp_uint_t)mp_obj_get_int(mem_unary_op(MP_UNARY_OP_LEN, MP_OBJ_FROM_PTR(self)));
  if (len == 0) {
    len = (mp_uint_t)-1;
  }

  if (self->typespec == NULL) {
    typechar = 'B';
  } else {
    typechar = self->typespec[0];
  }

  if (0) {
#if MICROPY_PY_BUILTINS_SLICE
  } else if (MP_OBJ_IS_TYPE(index_in, &mp_type_slice)) {
    mp_bound_slice_t slice;
    mp_obj_mem_t *res;

    if (!mp_seq_get_fast_slice_indexes(len, index_in, &slice)) {
      mp_raise_NotImplementedError("only slices with step=1 (aka None) are supported");
    }

    if (len > 0 && slice.stop > len) {
      nlr_raise(
        mp_obj_new_exception_msg_varg(
          &mp_type_IndexError,
          "Index slice is out of buffer"
          ));
    }

    addr = (UINT8 *)addr + (slice.start * self->typesize);
    res = efi_mem_new_n(addr, slice.stop - slice.start, self->typespec, self->readonly, self->page);
    value = MP_OBJ_FROM_PTR(res);
#endif
  } else {
    mp_uint_t index = (index_in != mp_const_none) ? mp_get_index(self->base.type, len, index_in, false) : 0;

    if (index > len || (len > 0 && index == len)) {
      mp_raise_msg(&mp_type_IndexError, "Out of buffer");
    }

    switch (typechar) {
    case 'b':
      value = MP_OBJ_NEW_SMALL_INT(((INT8 *)addr)[index]);
      break;

    case 'B':
      value = MP_OBJ_NEW_SMALL_INT(((UINT8*)addr)[index]);
      break;

    case 'h':
      value = MP_OBJ_NEW_SMALL_INT(((INT16*)addr)[index]);
      break;

    case 'H':
      value = MP_OBJ_NEW_SMALL_INT(((UINT16*)addr)[index]);
      break;

    case 'i':
      value = mp_obj_new_int(((int*)addr)[index]);
      break;

    case 'l':
      value = mp_obj_new_int(((INT32*)addr)[index]);
      break;

    case 'I':
      value = mp_obj_new_int_from_uint(((unsigned int*)addr)[index]);
      break;

    case 'L':
      value = mp_obj_new_int_from_uint(((UINT32*)addr)[index]);
      break;

    case 'n':
      value = mp_obj_new_int(((INTN*)addr)[index]);
      break;

    case 'E':
    case 'N':
      value = mp_obj_new_int_from_ull(((UINTN *)addr)[index]);
      break;

    case 'q':
      value = mp_obj_new_int_from_ll(((INT64*)addr)[index]);
      break;

    case 'Q':
      value = mp_obj_new_int_from_ull(((UINT64*)addr)[index]);
      break;

    case 'T':
    case 'F':
    case 'P':
      if (self->typespec[1] == '\0') {
        if (sizeof(void *) == 4) {
          value = mp_obj_new_int_from_uint((mp_uint_t)((void **)addr)[index]);
        } else {
          value = mp_obj_new_int_from_ull((UINTN)((void **)addr)[index]);
        }
      } else {
        value = efi_mem_new(((void **)addr)[index], self->typespec + 1, self->readonly, self->page);
      }
      break;

    case 'G':
      len = sizeof(gGuidFormat) - 1;
      str = AllocateZeroPool(sizeof(gGuidFormat));
      ASSERT(str != NULL);
      if (GuidToString(((EFI_GUID *)self->addr) + index, str) == len) {
        value = mp_obj_new_str(str, len);
      }
      FreePool(str);
      break;

    case 'a':
      if (index_in == mp_const_none) {
        len = AsciiStrLen(addr);
        len = MIN(self->size, len);
        value = mp_obj_new_str(addr, len);
      } else {
        value = MP_OBJ_NEW_SMALL_INT(((CHAR8 *)addr)[index]);
      }
      break;

    case 'u':
      if (index_in == mp_const_none) {
        len = StrLen(addr);
        len = MIN(self->size/sizeof(CHAR16), len);
        str = UnicodeToUtf8(addr, NULL, &len);
        value = mp_obj_new_str(str, len);
        FreePool(str);
      } else {
        value = MP_OBJ_NEW_SMALL_INT(((CHAR16 *)addr)[index]);
      }
      break;

    case 'O':
      value = efi_mem_new_n((UINT8 *)addr + index * self->typesize, 1, self->typespec, self->readonly, self->page);
      memobj = MP_OBJ_TO_PTR(value);
      // memobj->typespec = self->typespec;
      // memobj->typesize = self->typesize;
      // memobj->size = self->typesize;
      // memobj->fields = self->fields;
      memobj->typeattr = self->typeattr;
      break;

    default:
      mp_raise_msg(&mp_type_ValueError, "Not supported type to subscribe");
    }
  }

  return value;
}

STATIC void set_value(mp_obj_mem_t *self, mp_obj_t index_in, mp_obj_t value);
STATIC void set_mem_from_obj(mp_obj_mem_t *dst, mp_obj_t src, mp_uint_t index, mp_uint_t len)
{
  mp_buffer_info_t        bufinfo;
  mp_obj_t                iterable;
  mp_obj_t                item;
  mp_obj_type_t           *type;
  mp_uint_t               size;

  if (mp_get_buffer(src, &bufinfo, MP_BUFFER_READ) == TRUE) {
    size = len * dst->typesize;
    size = MIN(bufinfo.len, size);

    if ((index * dst->typesize + size) > dst->size) {
      mp_raise_msg(&mp_type_IndexError, "Out of buffer");
    }

    CopyMem (((UINT8 *)dst->addr) + index * dst->typesize, bufinfo.buf, size);
  } else {
    type = mp_obj_get_type(src);
    if (type != NULL && type->getiter != NULL) {
      iterable = mp_getiter(src, NULL);
      while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION && len != 0) {
        set_value(dst, MP_OBJ_NEW_SMALL_INT(index), item);
        ++index;
        --len;
      }
    } else {
      nlr_raise(
        mp_obj_new_exception_msg_varg(
          &mp_type_ValueError,
          "Given object does not support buffer or iterator protocol. Don't know how to get its value."
          )
        );
    }
  }
}

STATIC void set_value(mp_obj_mem_t *self, mp_obj_t index_in, mp_obj_t value)
{
  void                *addr = self->addr;
  const char          *str;
  CHAR16              *unistr;
  size_t              len;

  if (self->readonly) {
    mp_raise_ValueError("modifying read-only memory");
  }

  if (self->bitnum > 0) {
    set_bits(self, value);
    return;
  }

  len = (mp_uint_t)mp_obj_get_int(mem_unary_op(MP_UNARY_OP_LEN, MP_OBJ_FROM_PTR(self)));
  if (0) {
#if MICROPY_PY_BUILTINS_SLICE
  } else if (MP_OBJ_IS_TYPE(index_in, &mp_type_slice)) {
    mp_bound_slice_t  slice;
    mp_obj_t          iterable;
    mp_obj_t          item;
    int               start;

    if (!mp_seq_get_fast_slice_indexes(len, index_in, &slice)) {
       mp_raise_NotImplementedError("only slices with step=1 (aka None) are supported");
    }

    if (len > 0 && slice.stop > len) {
      nlr_raise(
        mp_obj_new_exception_msg_varg(
          &mp_type_IndexError,
          "Index slice is out of buffer"
          ));
    }

    start = slice.start;
    if (mp_obj_get_type(value)->getiter == NULL) {
      for (; start < slice.stop; ++start) {
        set_value(self, MP_OBJ_NEW_SMALL_INT(start), value);
      }
    } else {
      iterable = mp_getiter(value, NULL);
      while ((item = mp_iternext(iterable)) != MP_OBJ_STOP_ITERATION && start < slice.stop) {
        set_value(self, MP_OBJ_NEW_SMALL_INT(start++), item);
      }
    }
#endif
  } else {
    mp_uint_t index = (index_in != mp_const_none) ? mp_get_index(self->base.type, len, index_in, false) : 0;

    if (index > len || (len > 0 && index == len)) {
      mp_raise_msg(&mp_type_IndexError, "Out of buffer");
    }

    switch (self->typespec[0]) {
    case 'b':
      ((INT8 *)addr)[index] = (INT8)mp_obj_get_int_truncated(value);
      break;

    case 'B':
      ((UINT8 *)addr)[index] = (UINT8)mp_obj_get_int_truncated(value);
      break;

    case 'h':
      ((INT16 *)addr)[index] = (INT16)mp_obj_get_int_truncated(value);
      break;

    case 'H':
      ((UINT16 *)addr)[index] = (UINT16)mp_obj_get_int_truncated(value);
      break;

    case 'i':
      ((int *)addr)[index] = (int)mp_obj_get_int_truncated(value);
      break;

    case 'l':
      ((INT32 *)addr)[index] = (INT32)mp_obj_get_int_truncated(value);
      break;

    case 'I':
      ((unsigned int *)addr)[index] = (unsigned int)mp_obj_get_int_truncated(value);
      break;

    case 'L':
      ((UINT32 *)addr)[index] = (UINT32)mp_obj_get_int_truncated(value);
      break;

    case 'n':
      if (sizeof(INTN) == 4) {
        ((INT32 *)addr)[index] = (INT32)mp_obj_get_int_truncated(value);
      } else {
        ((INT64 *)addr)[index] = (INT64)get_uint64(value);
      }
      break;

    case 'E':
    case 'N':
      if (sizeof(UINTN) == 4) {
        ((UINT32 *)addr)[index] = (UINT32)mp_obj_get_int_truncated(value);
      } else {
        ((UINT64 *)addr)[index] = (UINT64)get_uint64(value);
      }
      break;

    case 'q':
      ((INT64 *)addr)[index] = (INT64)get_uint64(value);
      break;

    case 'Q':
      ((UINT64 *)addr)[index] = (UINT64)get_uint64(value);
      break;

    case 'T':
    case 'F':
    case 'P':
      if (MP_OBJ_IS_MEM(value)) {
        set_mem_from_obj(self, value, index, len);
      } else {
        if (sizeof(void *) == 4) {
          ((void **)addr)[index] = (void *)mp_obj_get_int_truncated(value);
        } else {
          ((void **)addr)[index] = (void *)(UINTN)get_uint64(value);
        }
      }
      break;

    case 'G':
      if (MP_OBJ_IS_STR_OR_BYTES(value)) {
        str = mp_obj_str_get_data(value, &len);
        if (str == NULL
            || len != (sizeof(gGuidFormat) - 1)
            || StringToGuid(str, (EFI_GUID *)(((UINT8 *)self->addr) + index * self->typesize)) == FALSE) {
          mp_raise_msg(&mp_type_ValueError, "Invalid GUID string");
        }
      } else {
        set_mem_from_obj(self, value, index, 1);
      }
      break;

    case 'a':
      if (index_in == mp_const_none && MP_OBJ_IS_STR_OR_BYTES(value)) {
        str = mp_obj_str_get_data(value, &len);
        if (str == NULL || len > self->size) {
          mp_raise_msg(&mp_type_ValueError, "Out of buffer");
        }
        SetMem(self->addr, self->size, 0);
        CopyMem(self->addr, str, len * sizeof(CHAR8));
      } else {
        set_mem_from_obj(self, value, index, len - index);
      }
      break;

    case 'u':
      if (index_in == mp_const_none && MP_OBJ_IS_STR_OR_BYTES(value)) {
        str = mp_obj_str_get_data(value, &len);
        unistr = Utf8ToUnicode(str, NULL, (UINTN *)&len, FALSE);
        if (unistr == NULL || ((len + 1) * sizeof(CHAR16)) > self->size) {
          FREE_NON_NULL(unistr);
          mp_raise_msg(&mp_type_ValueError, "Out of buffer");
        }
        SetMem(self->addr, self->size, 0);
        CopyMem(self->addr, unistr, (len + 1) * sizeof(CHAR16));
        FREE_NON_NULL(unistr);
      } else {
        set_mem_from_obj(self, value, index, len - index);
      }
      break;

    case 'O':
    default:
      set_mem_from_obj(self, value, index, len - index);
    }
  }
}

STATIC mp_obj_t efi_mem_deref(size_t n_args, const mp_obj_t *args)
{
  mp_obj_mem_t    *self;

  self = MP_OBJ_TO_PTR(args[0]);
  if (n_args > 1) {
    efi_mem_typecast(self, args[1]);
  }

  if (self->typespec == NULL || strlen(self->typespec) == 0) {
    mp_raise_msg(&mp_type_ValueError, "cannot dereference memory without type information");
  }

  return get_value(self, mp_const_none);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN (efi_mem_deref_obj, 1, 2, efi_mem_deref);

STATIC mp_obj_t efi_mem_addrof(mp_obj_t self_in)
{
  mp_obj_mem_t    *self = MP_OBJ_TO_PTR(self_in);
  char            *typespec;
  int             len;

  len = (self->typespec == NULL) ? 2 : (strlen(self->typespec) + 2);
  typespec = m_malloc(len);
  typespec[0] = 'P';
  typespec[1] = '\0';
  if (self->typespec != NULL) {
    AsciiStrCatS(typespec, len, self->typespec);
  }
  return MP_OBJ_FROM_PTR(efi_mem_new(&(self->addr), typespec, FALSE, self->page));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(efi_mem_addrof_obj, efi_mem_addrof);

STATIC mp_obj_t efi_mem_free(mp_obj_t self_in)
{
  mp_obj_mem_t    *self = MP_OBJ_TO_PTR(self_in);

  if (self->addr != NULL) {
    if (self->page) {
      FreePages(self->addr, EFI_SIZE_TO_PAGES(self->size));
    } else {
      FreePool(self->addr);
    }
    self->addr = NULL;
  }
  return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(efi_mem_free_obj, efi_mem_free);

//this call supports 10 parameters at most
STATIC mp_obj_t efi_fptr_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args)
{
  mp_obj_mem_t  *self = MP_OBJ_TO_PTR(self_in);
  INTN          i,j;
  mp_obj_mem_t  *memobj;
  mp_obj_t      rtnval;
  UINTN         intval;
  UINT64        longval;
  CONST CHAR8   *strval;
  CHAR16        *unival;
  mp_obj_t      arg;
  const char    *argtype;
  VOID          *func;
  BOOLEAN       is_va;
  UINTN         arglist[12];

  if (!self->hasvarg) {
    mp_arg_check_num(n_args, n_kw, self->args->len - 1, self->args->len - 1, false);
  }

  rtnval = mp_const_none;
  is_va = FALSE;
  i = 0;
  strval = NULL;
  unival = NULL;

  // resolve function address
  func = *(void **)(self->addr);

  for (i = 0, j = 0; i < n_args && j < 12; ++i) {
    arg = args[i];
    if (!is_va) {
      argtype = mp_obj_str_get_str(self->args->items[i + 1]);
      if (argtype[0] == 'V' || argtype[0] == 'v') {
        is_va = TRUE;
      }
    }

    if (is_va) {
      if (MP_OBJ_IS_MEM(arg)) {
        memobj = MP_OBJ_TO_PTR(arg);
        argtype = memobj->typespec;
      } else {
        nlr_raise(
          mp_obj_new_exception_msg_varg(
            &mp_type_TypeError,
            "Variable arguments must be passed in 'mem' object."
            ));
      }
    }

    switch (argtype[0]) {
    case 'P':
    case 'T':
    case 'F':
    case 'G':
    case 'O':
      if (is_va || MP_OBJ_IS_MEM(arg)) {
        memobj = MP_OBJ_TO_PTR(arg);
        if (memobj->typespec != NULL &&
            (memobj->typespec[0] == 'P' || memobj->typespec[0] == 'T' || memobj->typespec[0] == 'F')) {
          arglist[j++] = (UINTN)(*(void **)(memobj->addr));
        } else {
          arglist[j++] = (UINTN)memobj->addr;
        }
      } else if (MP_OBJ_IS_INT(arg)) {
        arglist[j++] = (UINTN)get_uint64(arg);
      } else {
        mp_buffer_info_t        bufinfo;

        if (mp_get_buffer(arg, &bufinfo, MP_BUFFER_READ) == TRUE) {
          arglist[j++] = (UINTN)bufinfo.buf;
        } else {
          RAISE_UEFI_EXCEPTION_ON_ERROR(EFI_INVALID_PARAMETER);
        }
      }
      break;

    case 'a':
      // ascii string
      if (MP_OBJ_IS_STR_OR_BYTES(args[i])) {
        arglist[j++] = (UINTN)mp_obj_str_get_str(args[i]);
      } else if (is_va || MP_OBJ_IS_MEM(arg)) {
        memobj = MP_OBJ_TO_PTR(arg);
        arglist[j++] = (UINTN)memobj->addr;
      } else {
        RAISE_UEFI_EXCEPTION_ON_ERROR(EFI_INVALID_PARAMETER);
      }
      break;

    case 'u':
      // unicode string
      if (MP_OBJ_IS_STR_OR_BYTES(args[i])) {
        strval = mp_obj_str_get_data(args[i], (size_t *)&intval);
        unival = Utf8ToUnicode(strval, NULL, &intval, FALSE);
        arglist[j++] = (UINTN)unival;
        strval = NULL;
      } else if (is_va || MP_OBJ_IS_MEM(arg)) {
        memobj = MP_OBJ_TO_PTR(arg);
        arglist[j++] = (UINTN)memobj->addr;
      } else {
        RAISE_UEFI_EXCEPTION_ON_ERROR(EFI_INVALID_PARAMETER);
      }
      break;

#if defined(MDE_CPU_IA32)
    case 'Q':
    case 'q':
      if (is_va || MP_OBJ_IS_MEM(arg)) {
        memobj = MP_OBJ_TO_PTR(arg);
        longval = get_uint64(get_value(memobj, mp_const_none));
      } else {
        longval = get_uint64(arg);
      }
      arglist[j++] = (UINTN)longval;
      arglist[j++] = *((UINTN *)&longval + 1);
      break;
#endif

    default:
      if (is_va || MP_OBJ_IS_MEM(arg)) {
        memobj = MP_OBJ_TO_PTR(arg);
        arglist[j++] = (UINTN)get_uint64(get_value(memobj, mp_const_none));
      } else {
        arglist[j++] = (UINTN)get_uint64(arg);
      }
      break;
    }
  }

  switch (j) {
  case 0:
    longval = ((FUN_NOARG)func)();
    break;

  case 1:
    longval = ((FUN_ARGS)func)(arglist[0]);
    break;

  case 2:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1]);
    break;

  case 3:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1], arglist[2]);
    break;

  case 4:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1], arglist[2], arglist[3]);
    break;

  case 5:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1], arglist[2], arglist[3],
                               arglist[4]);
    break;

  case 6:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1], arglist[2], arglist[3],
                               arglist[4], arglist[5]);
    break;

  case 7:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1], arglist[2], arglist[3],
                               arglist[4], arglist[5], arglist[6]);
    break;

  case 8:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1], arglist[2], arglist[3],
                               arglist[4], arglist[5], arglist[6], arglist[7]);
    break;

  case 9:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1], arglist[2], arglist[3],
                               arglist[4], arglist[5], arglist[6], arglist[7],
                               arglist[8]);
    break;

  case 10:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1], arglist[2], arglist[3],
                               arglist[4], arglist[5], arglist[6], arglist[7],
                               arglist[8], arglist[9]);
    break;

  case 11:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1], arglist[2], arglist[3],
                               arglist[4], arglist[5], arglist[6], arglist[7],
                               arglist[8], arglist[9], arglist[10]);
    break;

  case 12:
    longval = ((FUN_ARGS)func)(arglist[0], arglist[1], arglist[2], arglist[3],
                               arglist[4], arglist[5], arglist[6], arglist[7],
                               arglist[8], arglist[9], arglist[10], arglist[11]);
    break;

  default:
    break;
  }

  if (unival != NULL) {
    FreePool(unival);
  }

  if (self->args->items[0] != mp_const_none) {
    const char *rtype = mp_obj_str_get_str(self->args->items[0]);

    if (rtype != NULL) {
      memobj = efi_mem_new(&longval, rtype, TRUE, FALSE);
      rtnval = get_value(memobj, mp_const_none);

      if (rtype[0] == 'E') {
        RAISE_UEFI_EXCEPTION_ON_ERROR((EFI_STATUS)longval);
      }
    }
  }

  return rtnval;
}

STATIC mp_obj_t mem_it_iternext(mp_obj_t self_in) {
    mp_obj_mem_it_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->memobj->size > 0 &&
        ((self->cur + 1) * self->memobj->typesize) <= self->memobj->size) {
        return get_value(self->memobj, mp_obj_new_int_from_uint(self->cur++));
    } else {
        return MP_OBJ_STOP_ITERATION;
    }
}

STATIC const mp_obj_type_t mem_it_type = {
    { &mp_type_type },
    .name = MP_QSTR_iterator,
    .getiter = mp_identity_getiter,
    .iternext = mem_it_iternext,
};

STATIC mp_obj_t mem_iterator_new(mp_obj_t mem_in , mp_obj_iter_buf_t *buf) {
    mp_obj_mem_t *mem = MP_OBJ_TO_PTR(mem_in);
    mp_obj_mem_it_t *self = (mp_obj_mem_it_t *)buf;

    if (mem->fields != NULL &&
        (mem->typespec == NULL || mem->typespec[0] != 'F')) {
      return mem->fields->base.type->getiter(mem->fields, buf);
    }

    self->base.type = &mem_it_type;
    self->memobj = mem;
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t mem_subscr(mp_obj_t self_in, mp_obj_t index_in, mp_obj_t value) {
  mp_obj_mem_t *self = MP_OBJ_TO_PTR(self_in);

  if (value == MP_OBJ_NULL) {
    return MP_OBJ_NULL; // DELETE op not supported
  }

  if (value == MP_OBJ_SENTINEL) {
    // load
    return get_value(self, index_in);
  } else {
    // store
    set_value(self, index_in, value);
  }

  return mp_const_none;
}

STATIC mp_int_t mem_get_buffer(mp_obj_t o_in, mp_buffer_info_t *bufinfo, mp_uint_t flags) {
  mp_obj_mem_t *self = MP_OBJ_TO_PTR(o_in);

  if (self->addr == NULL || self->size == 0) {
    return 1;
  }

  bufinfo->buf = self->addr;
  bufinfo->len = (self->size > 0) ? self->size : self->typesize;

  if ((self->readonly) && (flags & MP_BUFFER_WRITE)) {
    // read-only
    return 1;
  }

  return 0;
}

STATIC mp_obj_t mem_unary_op(mp_unary_op_t op, mp_obj_t o_in) {
  mp_obj_mem_t  *self = MP_OBJ_TO_PTR(o_in);
  mp_uint_t     len;

  switch (op) {
  case MP_UNARY_OP_BOOL:
    return mp_obj_new_bool(self->addr != NULL && self->size > 0);

  case MP_UNARY_OP_LEN:
    if (self->typesize > 0) {
      len = self->size / self->typesize;
    } else {
      len = 0;
    }
    return mp_obj_new_int_from_uint(len);

  default:
    return MP_OBJ_NULL; // op not supported
  }
}

void mem_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
  mp_obj_mem_t      *self = MP_OBJ_TO_PTR(self_in);
  mp_obj_type_t     *type;
  mp_obj_t          attrobj;

  switch (attr) {
  case MP_QSTR_ADDR:
    if (dest[0] == MP_OBJ_NULL) {
      dest[0] = mp_obj_new_int_from_ull((UINT64)(UINTN)self->addr);
    } else {
      if (self->addr != NULL) {
        nlr_raise(
          mp_obj_new_exception_msg_varg(
            &mp_type_AttributeError,
            "Re-assigning valid memory address may cause memory leak"
            ));
      }
      self->addr = (void *)(UINTN)get_uint64(dest[1]);
      dest[0] = MP_OBJ_NULL;  // means sucess to store
    }
    break;

  case MP_QSTR_SIZE:
    if (dest[0] == MP_OBJ_NULL) {
      dest[0] = mp_obj_new_int_from_uint(efi_mem_get_size(self_in));
    } else {
      self->size = (mp_uint_t)get_uint64(dest[1]);
      dest[0] = MP_OBJ_NULL;  // means sucess to store
    }
    break;

  case MP_QSTR_VALUE:
    if (dest[0] == MP_OBJ_NULL) {
      dest[0] = get_value(self, mp_const_none);
    } else {
      set_value(self, mp_const_none, dest[1]);
      dest[0] = MP_OBJ_NULL;  // means sucess to store
    }
    break;

  case MP_QSTR_TYPE:
    if (dest[0] == MP_OBJ_NULL) {
      if (self->typespec != NULL) {
        dest[0] = mp_obj_new_str(self->typespec, strlen(self->typespec));
      } else {
        dest[0] = mp_const_none;
      }
    } else {
      efi_mem_typecast (self_in, dest[1]);
      dest[0] = MP_OBJ_NULL;  // means sucess to store
    }
    break;

  default:
    type = mp_obj_get_type(self_in);
    attrobj = MP_OBJ_NEW_QSTR(attr);

    if (type->locals_dict != NULL && dest[0] == MP_OBJ_NULL) {
        // generic method lookup
        // this is a lookup in the object (ie not class or type)
        assert(type->locals_dict->base.type == &mp_type_dict); // Micro Python restriction, for now
        mp_map_t *locals_map = &type->locals_dict->map;
        mp_map_elem_t *elem = mp_map_lookup(locals_map, attrobj, MP_MAP_LOOKUP);
        if (elem != NULL) {
            mp_convert_member_lookup(self_in, type, elem->value, dest);
            return;
        }
    }

    if (self->fields != NULL) {
      mp_map_elem_t *elem = mp_map_lookup(&self->fields->map, attrobj, MP_MAP_LOOKUP);
      mp_obj_mem_t  *p;

      if (elem != NULL) {
        if (dest[0] != MP_OBJ_NULL) {
          set_value(MP_OBJ_TO_PTR(elem->value), mp_const_none, dest[1]);
          dest[0] = MP_OBJ_NULL;  // means sucess to store
        } else {
          p = MP_OBJ_TO_PTR(elem->value);
          if (p->fields != NULL || p->size > p->typesize || (sizeof(UINT64) % p->typesize) != 0) {
            // for complex type, array or structure, return 'mem' object itself
            dest[0] = elem->value;
          } else {
            // for simple type, return the value object
            dest[0] = get_value(elem->value, mp_const_none);
          }
        }
        return;
      }
    }

    nlr_raise(
      mp_obj_new_exception_msg_varg(
        &mp_type_AttributeError,
        "Non-existing attribute"
        ));
  }
}

STATIC mp_obj_t mem_binary_op(mp_binary_op_t op, mp_obj_t lhs_in, mp_obj_t rhs_in) {
  mp_buffer_info_t  lhs_bufinfo;
  mp_buffer_info_t  rhs_bufinfo;
  mp_obj_t          value;

  switch (op) {
  case MP_BINARY_OP_EQUAL:
    if (MP_OBJ_IS_INT(rhs_in)) {
      value = get_value(lhs_in, mp_const_none);
      if (MP_OBJ_IS_MEM(value)) {
        return mp_obj_new_bool(((mp_obj_mem_t *)MP_OBJ_TO_PTR(value))->addr
                               ==
                               (void *)(UINTN)get_uint64(rhs_in));
      } else if (MP_OBJ_IS_INT(value)) {
        return mp_binary_op(MP_BINARY_OP_EQUAL, rhs_in, value);
      } else {
        return mp_const_false;
      }
    } else if (mp_get_buffer(rhs_in, &rhs_bufinfo, MP_BUFFER_READ)) {
      if (!mp_get_buffer(lhs_in, &lhs_bufinfo, MP_BUFFER_READ)) {
        return mp_const_false;
      }

      if (lhs_bufinfo.buf == rhs_bufinfo.buf) {
        return mp_const_true;
      } else if (lhs_bufinfo.len != rhs_bufinfo.len) {
        return mp_const_false;
      } else {
        return mp_obj_new_bool(mp_seq_cmp_bytes(op, lhs_bufinfo.buf, lhs_bufinfo.len, rhs_bufinfo.buf, rhs_bufinfo.len));
      }
    } else {
      return mp_const_false;
    }

  default:
    return MP_OBJ_NULL; // op not supported
  }
}

STATIC const mp_rom_map_elem_t efi_mem_locals_dict_table[] = {
  { MP_ROM_QSTR(MP_QSTR_DREF), MP_ROM_PTR(&efi_mem_deref_obj) },
  { MP_ROM_QSTR(MP_QSTR_REF),  MP_ROM_PTR(&efi_mem_addrof_obj) },
  { MP_ROM_QSTR(MP_QSTR_CAST), MP_ROM_PTR(&efi_mem_typecast_obj) },
  { MP_ROM_QSTR(MP_QSTR_FREE), MP_ROM_PTR(&efi_mem_free_obj) },
};
STATIC MP_DEFINE_CONST_DICT(efi_mem_locals_dict, efi_mem_locals_dict_table);

const mp_obj_type_t mp_type_mem = {
  { &mp_type_type },
  .name = MP_QSTR_mem,
  .print = efi_mem_print,
  .make_new = efi_mem_make_new,
  .attr = mem_attr,
  .subscr = mem_subscr,
  .unary_op = mem_unary_op,
  .binary_op = mem_binary_op,
  .buffer_p = { .get_buffer = mem_get_buffer },
  .getiter = mem_iterator_new,
  .locals_dict = (mp_obj_dict_t *)&efi_mem_locals_dict,
};

const mp_obj_type_t mp_type_guid = {
  { &mp_type_type },
  .name = MP_QSTR_guid,
  .print = efi_guid_print,
  .make_new = efi_guid_make_new,
  .attr = mem_attr,
  .getiter = mem_iterator_new,
  .unary_op = mem_unary_op,
  .binary_op = mem_binary_op,
  .buffer_p = { .get_buffer = mem_get_buffer },
  .locals_dict = (mp_obj_dict_t *)&efi_mem_locals_dict,
};

const mp_obj_type_t mp_type_fptr = {
  { &mp_type_type },
  .name = MP_QSTR_fptr,
  .print = efi_mem_print,
  .make_new = efi_fptr_make_new,
  .attr = mem_attr,
  .call = efi_fptr_call,
  .unary_op = mem_unary_op,
  .binary_op = mem_binary_op,
  .buffer_p = { .get_buffer = mem_get_buffer },
  .locals_dict = (mp_obj_dict_t *)&efi_mem_locals_dict,
};

static const void *null_ptr = NULL;
const mp_obj_mem_t mp_nullptr_obj = {
  .base = {&mp_type_mem},
  .addr = &null_ptr,
  .fields = NULL,
  .typeattr = 0,
  .typespec = "P",
  .typesize = sizeof(null_ptr),
  .size = sizeof(null_ptr),
};

STATIC void mp_obj_efistatus_print(const mp_print_t *print, mp_obj_t o_in, mp_print_kind_t kind) {
  mp_obj_exception_t *obj = MP_OBJ_TO_PTR(o_in);
  mp_obj_type_t *parent = MP_OBJ_TO_PTR(((mp_obj_tuple_t*)obj->base.type->parent)->items[0]);
  parent->print(print, o_in, kind);
}

STATIC void efistatus_attr(mp_obj_t self_in, qstr attr, mp_obj_t *dest) {
  mp_obj_exception_t *self = MP_OBJ_TO_PTR(self_in);
  mp_obj_type_t *parent = MP_OBJ_TO_PTR(((mp_obj_tuple_t*)self->base.type->parent)->items[0]);
  parent->attr(self_in, attr, dest);
}

STATIC const mp_rom_obj_tuple_t mp_type_efistatus_base_tuple = {{&mp_type_tuple}, 1, {MP_ROM_PTR(&mp_type_OSError)}};
const mp_obj_type_t mp_type_efistatus = {
    { &mp_type_type },
    .name = MP_QSTR_efistatus,
    .make_new = mp_obj_exception_make_new,
    .print = mp_obj_efistatus_print,
    .attr = efistatus_attr,
    .parent = (mp_obj_tuple_t*)(mp_rom_obj_tuple_t*)&mp_type_efistatus_base_tuple,
};

const char *efi_status_msg[35] = {
  "SUCCESS",
  "LOAD_ERROR",            // ENCODE_ERROR (1)
  "INVALID_PARAMETER",     // ENCODE_ERROR (2)
  "UNSUPPORTED",           // ENCODE_ERROR (3)
  "BAD_BUFFER_SIZE",       // ENCODE_ERROR (4)
  "BUFFER_TOO_SMALL",      // ENCODE_ERROR (5)
  "NOT_READY",             // ENCODE_ERROR (6)
  "DEVICE_ERROR",          // ENCODE_ERROR (7)
  "WRITE_PROTECTED",       // ENCODE_ERROR (8)
  "OUT_OF_RESOURCES",      // ENCODE_ERROR (9)
  "VOLUME_CORRUPTED",      // ENCODE_ERROR (10)
  "VOLUME_FULL",           // ENCODE_ERROR (11)
  "NO_MEDIA",              // ENCODE_ERROR (12)
  "MEDIA_CHANGED",         // ENCODE_ERROR (13)
  "NOT_FOUND",             // ENCODE_ERROR (14)
  "ACCESS_DENIED",         // ENCODE_ERROR (15)
  "NO_RESPONSE",           // ENCODE_ERROR (16)
  "NO_MAPPING",            // ENCODE_ERROR (17)
  "TIMEOUT",               // ENCODE_ERROR (18)
  "NOT_STARTED",           // ENCODE_ERROR (19)
  "ALREADY_STARTED",       // ENCODE_ERROR (20)
  "ABORTED",               // ENCODE_ERROR (21)
  "ICMP_ERROR",            // ENCODE_ERROR (22)
  "TFTP_ERROR",            // ENCODE_ERROR (23)
  "PROTOCOL_ERROR",        // ENCODE_ERROR (24)
  "INCOMPATIBLE_VERSION",  // ENCODE_ERROR (25)
  "SECURITY_VIOLATION",    // ENCODE_ERROR (26)
  "CRC_ERROR",             // ENCODE_ERROR (27)
  "END_OF_MEDIA",          // ENCODE_ERROR (28)
  "END_OF_FILE",           // ENCODE_ERROR (31)
  "INVALID_LANGUAGE",      // ENCODE_ERROR (32)
  "COMPROMISED_DATA",      // ENCODE_ERROR (33)
  "",
  "HTTP_ERROR"             // ENCODE_ERROR (35)
};

