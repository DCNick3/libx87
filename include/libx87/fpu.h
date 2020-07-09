#ifndef LIBX87_FPU_H
#define LIBX87_FPU_H

#include <stdint.h>

namespace libx87 {

#define FLOATX80

#include "libx87/softfloat/softfloat.h"
#include "libx87/softfloat/softfloatx80.h"

#undef FLOATX80

    template<typename CPU_GLUE>
    class fpu {
        static const uint32_t
                EFLAGS_CF = 1,
                EFLAGS_PF = 0x4,
                EFLAGS_AF = 0x10,
                EFLAGS_ZF = 0x40,
                EFLAGS_SF = 0x80,
                EFLAGS_OF = 0x800,

                CR0_PE = 1,

                FPU_ROUND_SHIFT = 10,
                FPU_PRECISION_SHIFT = 8,

                FPU_EXCEPTION_STACK_FAULT = (1 << 6),
                FPU_EXCEPTION_PRECISION = (1 << 5),
                FPU_EXCEPTION_UNDERFLOW = (1 << 4),
                FPU_EXCEPTION_OVERFLOW = (1 << 3),
                FPU_EXCEPTION_ZERO_DIVIDE = (1 << 2),
                FPU_EXCEPTION_DENORMALIZED = (1 << 1),
                FPU_EXCEPTION_INVALID_OPERATION = (1 << 0);

        union {
// This is a really nasty union to make MMX instructions work.
            struct {
                union {
                    uint8_t r8[8];
                    uint16_t r16[4];
                    uint32_t r32[2];
                    uint64_t r64;
                } reg;
                uint16_t dummy;
            } mm[8];

            floatx80 st[8];
        };
        int ftop = 0;
        uint16_t control_word = 0, status_word = 0, tag_word = 0;
        uint32_t fpu_eip = 0, fpu_data_ptr = 0;
        uint16_t fpu_cs = 0, fpu_opcode = 0, fpu_data_seg = 0;

        // kludgy thing
        uint32_t partial_sw = 0, bits_to_clear = 0;

        // These are all values used internally. They are regenerated every time fpu.control_word is modified
        float_status_t status;

        inline CPU_GLUE* cglue() {
            return static_cast<CPU_GLUE*>(this);
        }
        inline void cpu_write32(uint32_t linear_address, uint32_t value) {
            cglue()->write32(linear_address, value);
        }
        inline void cpu_write16(uint32_t linear_address, uint16_t value) {
            cglue()->write16(linear_address, value);
        }
        inline void cpu_write8(uint32_t linear_address, uint8_t value) {
            cglue()->write8(linear_address, value);
        }
        inline void cpu_read32(uint32_t linear_address, uint32_t &value) {
            cglue()->read32(linear_address, value);
        }
        inline void cpu_read16(uint32_t linear_address, uint16_t &value) {
            cglue()->read16(linear_address, value);
        }
        inline int  cpu_access_verify(uint32_t start, uint32_t end) {
            return cglue()->access_verify(start, end);
        }
        inline void cpu_set_cf(bool value) {
            cglue()->set_cf(value);
        }
        inline void cpu_set_pf(bool value) {
            cglue()->set_pf(value);
        }
        inline void cpu_set_zf(bool value) {
            cglue()->set_zf(value);
        }
        inline bool cpu_get_cf() {
            return cglue()->get_cf();
        }
        inline bool cpu_get_pf() {
            return cglue()->get_pf();
        }
        inline bool cpu_get_zf() {
            return cglue()->get_zf();
        }
        inline uint32_t cpu_get_eflags() {
            return cglue()->get_eflags();
        }
        inline void cpu_set_eflags(uint32_t value) {
            cglue()->set_eflags(value);
        }
        inline void cpu_set_ax(uint16_t value) {
            cglue()->set_ax(value);
        }
        inline bool cpu_is_protected() {
            return cglue()->is_protected();
        }
        inline bool cpu_is_code16() {
            return cglue()->is_code16();
        }
        inline uint16_t cpu_get_cs() {
            return cglue()->get_cs();
        }
        inline uint32_t cpu_get_eip() {
            return cglue()->get_eip();
        }
        inline uint16_t cpu_get_seg(uint32_t seg) {
            return cglue()->get_seg(seg);
        }
        inline void cpu_undefined_instruction() {
            cglue()->undefined_instruction();
        }
        inline void cpu_fp_exception() {
            cglue()->fp_exception();
        }

        void set_control_word(uint16_t control_word);
        uint16_t get_status_word();
        int get_tag(int st);
        void set_tag(int st, int v);
        int exception_raised(int flags);
        void stack_fault();
        void commit_sw();
        int check_exceptions2(int commit_sw);
        int check_exceptions();
        void fninit();
        int nm_check(void);
        void set_st(int st, floatx80 data);
        floatx80 get_st(int st);
        floatx80 *get_st_ptr(int st);
        int check_stack_overflow(int st_param);
        int check_stack_underflow(int st, int commit_sw);
        int exception_masked(int excep);
        int push(floatx80 data);
        void pop();
        void update_pointers(uint32_t opcode);
        void update_pointers2(uint32_t opcode, uint32_t virtaddr, uint32_t seg);
        int write_float32(uint32_t linaddr, float32 src);
        int write_float64(uint32_t linaddr, float64 dest);
        int check_push(void);
        int store_f80(uint32_t linaddr, floatx80 *data);
        int read_f80(uint32_t linaddr, floatx80 *data);
        int fcom(floatx80 op1, floatx80 op2, int unordered);
        int fcomi(floatx80 op1, floatx80 op2, int unordered);
        void watchpoint();
        void watchpoint2();

        int fstenv(uint32_t linaddr, int code16);
        int fldenv(uint32_t linaddr, int code16);


        inline void set_c0(bool n) {
            status_word = (status_word & ~(1 << 8)) | (n) << 8;
        }
        inline void set_c1(bool n) {
            status_word = (status_word & ~(1 << 9)) | (n) << 9;
        }
        inline void set_c2(bool n) {
            status_word = (status_word & ~(1 << 10)) | (n) << 10;
        }
        inline void set_c3(bool n) {
            status_word = (status_word & ~(1 << 14)) | (n) << 14;
        }

        void *fpu_get_st_ptr1();

    public:
        void fpu_debug();

        int reg_op(uint32_t opcode);
        int mem_op(uint32_t opcode, uint32_t linaddr, uint32_t virtaddr, uint32_t seg);
        int fwait(void);
    };

};

// x87 FPU operations built on top of the Bochs/SoftFloat floating point emulator.
// originally made for halfix
// C++-ized for uwin-jit

// Quick note:
//         | ST0 | ST1 | | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
//  fld A  |  A  |  -  | |   |   |   |   |   |   |   | A | ftop: 7
//  fld B  |  B  |  A  | |   |   |   |   |   |   | B | A | ftop: 6
//  fadd C | B+A |  A  | |   |   |   |   |   |   |B+A| A | ftop: 6

// List of instructions that do NOT check for exceptions:
//  - FNSTENV, FNINIT, FNCLEX, FNSAVE, FNSTSW, FNSTSW, FNSTCW

// List of instructions that do not update FPU eip and friends:
//  - FNCLEX, FLDCW, FNSTCW, FNSTSW, FNSTENV, FLDENV, and FWAIT

// FLD mem order of operations:
//  1. Read source memory, but don't do anything with it yet
//  2. Save FPU CS:IP and data pointers
//  3. Convert source memory to 80-bit FP register
//  4. Check destination

// For basic arithmetic instructions, these are the possible operands:
/*
FADD:
 ST(0) = ST(0) + float32 <D8>
 ST(0) = ST(0) + int32 <DA>
 ST(0) = ST(0) + float64 <DC>
 ST(0) = ST(0) + int16 <DE>
 ST(0) = ST(0) + ST(i) <D8 mod=3>
 ST(i) = ST(i) + ST(0) <DC mod=3>
 ST(i) = ST(i) + ST(0) & pop <DE mod=3>
*/
// There are no read-modify-write operations, thank god.

//#define UNUSED(x) ((void)x)

#include <cmath>
#include <cstdio>

namespace libx87 {
    enum {
        FPU_TAG_VALID = 0,
        FPU_TAG_ZERO = 1,
        FPU_TAG_SPECIAL = 2,
        FPU_TAG_EMPTY = 3
    };

    enum {
        FPU_ROUND_NEAREST = 0,
        FPU_ROUND_DOWN = 1,
        FPU_ROUND_UP = 2,
        FPU_ROUND_TRUNCATE = 3
    };
    enum {
        FPU_PRECISION_24 = 0, // 32-bit float
        FPU_PRECISION_53 = 2, // 64-bit double
        FPU_PRECISION_64 = 3 // 80-bit st80
    };

// Status word

    static const floatx80 Zero = BUILD_FLOAT_X80(0, 0);
    static const floatx80 IndefiniteNaN = BUILD_FLOAT_X80(0xFFFF, 0xC000000000000000);
//static const floatx80 PositiveInfinity = BUILD_FLOAT(0x7FFF, 0x8000000000000000);
//static const extFloat80_t NegativeInfinity = BUILD_FLOAT(0xFFFF, 0x8000000000000000);
    static const floatx80 Constant_1 = BUILD_FLOAT_X80(0x3fff, 0x8000000000000000);
    static const floatx80 Constant_L2T = BUILD_FLOAT_X80(0x4000, 0xd49a784bcd1b8afe);
    static const floatx80 Constant_L2E = BUILD_FLOAT_X80(0x3fff, 0xb8aa3b295c17f0bc);
    static const floatx80 Constant_PI = BUILD_FLOAT_X80(0x4000, 0xc90fdaa22168c235);
    static const floatx80 Constant_LG2 = BUILD_FLOAT_X80(0x3ffd, 0x9a209a84fbcff799);
    static const floatx80 Constant_LN2 = BUILD_FLOAT_X80(0x3ffe, 0xb17217f7d1cf79ac);

    static const floatx80 *Constants[8] = {
            // Technically, there are only 7 constants according to the x87 spec, but to make this array nice and round, I'm going to assume that were there to be an eighth value, it would be an indefinite NaN.
            &Constant_1, &Constant_L2T, &Constant_L2E, &Constant_PI, &Constant_LG2, &Constant_LN2, &Zero, &IndefiniteNaN
    };

//struct fpu fpu;

// FLDCW
    template<typename C>
    void fpu<C>::set_control_word(uint16_t control_word) {
        control_word |= 0x40; // Experiments with real hardware indicate that bit 6 is always set.
        this->control_word = control_word;

        int rounding = this->control_word >> FPU_ROUND_SHIFT & 3;
        switch (rounding) {
            case FPU_ROUND_NEAREST: // aka round to even
                this->status.float_rounding_mode = float_round_nearest_even;
                break;
            case FPU_ROUND_DOWN:
                this->status.float_rounding_mode = float_round_down;
                break;
            case FPU_ROUND_UP:
                this->status.float_rounding_mode = float_round_up;
                break;
            case FPU_ROUND_TRUNCATE: // aka towards zero
                this->status.float_rounding_mode = float_round_to_zero;
                break;
        }
        int precision = this->control_word >> FPU_PRECISION_SHIFT & 3;
        switch (precision) {
            case FPU_PRECISION_24: // aka float
                this->status.float_rounding_precision = 32;
                break;
            case FPU_PRECISION_53: // aka double
                this->status.float_rounding_precision = 64;
                break;
            case FPU_PRECISION_64: // This is the default
                this->status.float_rounding_precision = 80;
                break;
        }

        // Are these right?
        this->status.float_exception_flags = 0; // clear exceptions before execution
        this->status.float_nan_handling_mode = float_first_operand_nan;
        this->status.flush_underflow_to_zero = 0;
        this->status.float_suppress_exception = 0;
        this->status.float_exception_masks = control_word & 0x3F;
        this->status.denormals_are_zeros = 0;
    }

    template<typename C>
    uint16_t fpu<C>::get_status_word() {
        return status_word | (ftop << 11);
    }

// Helper functions to determine type of floating point number.
    static int is_denormal(uint16_t exponent, uint64_t mantissa) {
        return !(exponent & 0x7FFF) && mantissa;
    }

    static int is_pseudo_denormal(uint16_t exponent, uint64_t mantissa) {
        return is_denormal(exponent, mantissa) && !(mantissa & 0x8000000000000000ULL);
    }

    static int is_zero(uint16_t exponent, uint64_t mantissa) {
        return ((exponent & 0x7FFF) | mantissa) == 0;
    }

// Check if a floating point number is a zero of any sign.
// Returns 1 if it's a positive zero
// Returns -1 if it's a negative zero
// Returns 0 if it's neither.
    static int is_zero_any_sign(uint16_t exponent, uint64_t mantissa) {
        if (is_zero(exponent, mantissa)) {
            if (exponent & 0x8000)
                return -1;
            else
                return 1;
        } else
            return 0;
    }

    static int is_negative(uint16_t exponent, uint64_t mantissa) {
        return !is_zero_any_sign(exponent, mantissa) && (exponent & 0x8000) != 0;
    }

    static int is_invalid(uint16_t exponent, uint64_t mantissa) {
        // Check for pseudo NaN, pseudo Infinity, or Unnormal
        uint16_t exponent_without_sign = exponent & 0x7FFF;
        if (exponent_without_sign != 0)
            return (mantissa & 0x8000000000000000ULL) == 0; // Pseudo-NaN, Pseudo-Infinity, or Unnormal
        return 0;
    }

// Returns:
//   -1 for -Infinity
//    1 for Infinity
//    0 for non-infinity
    static int is_infinity(uint16_t exponent, uint64_t mantissa) {
        if (((exponent & 0x7FFF) == 0x7FFF) && (mantissa == 0x8000000000000000ULL))
            return mantissa >> 15 ? -1 : 1;
        return 0;
    }

// Returns 1 if quiet, 2 if signalling, 0 if neither
    static int is_nan(uint16_t exponent, uint64_t mantissa) {
        if (((exponent & 0x7FFF) == 0x7FFF) && (mantissa != 0x8000000000000000ULL))
            return 1 + ((mantissa & 0x4000000000000000ULL) != 0);
        return 0;
    }

    static int fpu_get_tag_from_value(floatx80 *f) {
        uint16_t exponent;
        uint64_t mantissa;
        floatx80_unpack(f, exponent, mantissa);
        if ((exponent | mantissa) == 0) // Both exponent and mantissa are zero
            return FPU_TAG_ZERO;

        int x = 0;
        x |= is_infinity(exponent, mantissa);
        x |= is_denormal(exponent, mantissa);
        x |= is_pseudo_denormal(exponent, mantissa);
        x |= is_invalid(exponent, mantissa);
        x |= is_nan(exponent, mantissa);

        if (x)
            return FPU_TAG_SPECIAL;
        return FPU_TAG_VALID;
    }

    template<typename C>
    int fpu<C>::get_tag(int st) {
        return tag_word >> (((st + ftop) & 7) << 1) & 3;
    }

    template<typename C>
    void fpu<C>::set_tag(int st, int v) {
        int shift = ((st + ftop) & 7) << 1;
        tag_word &= ~(3 << shift);
        tag_word |= v << shift;
    }

    template<typename C>
    int fpu<C>::exception_raised(int flags) {
        return (status.float_exception_flags & ~status.float_exception_masks) & flags;
    }

// Note that stack faults must be handled before any arith, softfloat.c may clear them.
    template<typename C>
    void fpu<C>::stack_fault() {
        //__asm__("int3");
        status.float_exception_flags = FPU_EXCEPTION_INVALID_OPERATION | FPU_EXCEPTION_STACK_FAULT;
        //if(fpu.status.float_exception_masks & FPU_EXCEPTION_INVALID_OPERATION) return 1;
        //return 0;
    }

    template<typename C>
    void fpu<C>::commit_sw() {
        // XXX this is a really, really bad kludge
        status_word |= partial_sw;
        status_word &= ~bits_to_clear | partial_sw;
        bits_to_clear = 0;
        partial_sw = 0;
    }

    template<typename C>
    int fpu<C>::check_exceptions2(int commit_sw) {
        int flags = status.float_exception_flags;
        int unmasked_exceptions = (flags & ~status.float_exception_masks) & 0x3F;

        // Note: #P is ignored if #U or #O is set.
        if (flags & FPU_EXCEPTION_PRECISION && (flags & (FPU_EXCEPTION_UNDERFLOW | FPU_EXCEPTION_OVERFLOW))) {
            flags &= ~FPU_EXCEPTION_PRECISION;
            unmasked_exceptions &= ~FPU_EXCEPTION_PRECISION;
        }

        // Note: C1 is set if the result was rounded up, but cleared if a stack underflow occurred
        if (flags & 0x10000) {
            // Stack underflow occurred
            flags &= ~(1 << 9);
        }

        // If #I, #D, or #Z, then ignore others.
        if (flags & (FPU_EXCEPTION_INVALID_OPERATION | FPU_EXCEPTION_ZERO_DIVIDE | FPU_EXCEPTION_DENORMALIZED)) {
            unmasked_exceptions &=
                    FPU_EXCEPTION_INVALID_OPERATION | FPU_EXCEPTION_ZERO_DIVIDE | FPU_EXCEPTION_DENORMALIZED;
            flags &= FPU_EXCEPTION_INVALID_OPERATION | FPU_EXCEPTION_ZERO_DIVIDE | FPU_EXCEPTION_DENORMALIZED |
                     FPU_EXCEPTION_STACK_FAULT;
        }

        if (commit_sw)
            status_word |= flags;
        else partial_sw |= flags;

        if (unmasked_exceptions) {
            status_word |= 0x8080;
            if (unmasked_exceptions & ~FPU_EXCEPTION_PRECISION)
                return 1;
            return 0; // Only #P is raised but we can ignore that
        }

        return 0;
    }

    template<typename C>
    int fpu<C>::check_exceptions() {
        return check_exceptions2(1);
    }

    template<typename C>
    void fpu<C>::fninit() {
        // https://www.felixcloutier.com/x86/finit:fninit
        set_control_word(0x37F);
        status_word = 0;
        tag_word = 0xFFFF;
        ftop = 0;
        fpu_data_ptr = 0;
        fpu_data_seg = 0;
        fpu_eip = 0;
        fpu_cs = 0; // Not in the docs, but assuming that it's the case
        fpu_opcode = 0;
    }

    template<typename C>
    int fpu<C>::nm_check(void) {
        // what does it mean, again?
        //if (thread_cpu.cr[0] & (CR0_EM | CR0_TS))
        //    EXCEPTION_NM();
        return 0;
    }

    template<typename C>
    floatx80 *fpu<C>::get_st_ptr(int st_param) {
        return &st[(ftop + st_param) & 7];
    }

    template<typename C>
    floatx80 fpu<C>::get_st(int st_param) {
        return st[(ftop + st_param) & 7];
    }

    template<typename C>
    void fpu<C>::set_st(int st_param, floatx80 data) {
        set_tag(st_param, fpu_get_tag_from_value(&data));
        st[(ftop + st_param) & 7] = data;
    }

// Fault if ST register is not empty.
    template<typename C>
    int fpu<C>::check_stack_overflow(int st_param) {
        int tag = get_tag(st_param);
        if (tag != FPU_TAG_EMPTY) {
            set_c1(1);
            stack_fault();
            return 1;
        }
        set_c1(0);
        return 0;
    }

// Fault if ST register is empty.
    template<typename C>
    int fpu<C>::check_stack_underflow(int st, int commit_sw) {
        int tag = get_tag(st);
        if (tag == FPU_TAG_EMPTY) {
            stack_fault();
            if (commit_sw)
                set_c1(1);
            else
                partial_sw = 1 << 9;
            return 1;
        }
        if (commit_sw)
            set_c1(0);
        else
            bits_to_clear = 1 << 9;
        return 0;
    }

    template<typename C>
    int fpu<C>::exception_masked(int excep) {
        if (excep == FPU_EXCEPTION_STACK_FAULT)
            excep = FPU_EXCEPTION_INVALID_OPERATION;
        return (control_word & excep);
    }

    template<typename C>
    int fpu<C>::push(floatx80 data) {
        ftop = (ftop - 1) & 7;
        set_st(0, data);
        return 0;
    }

    template<typename C>
    void fpu<C>::pop() {
        set_tag(0, FPU_TAG_EMPTY);
        ftop = (ftop + 1) & 7;
    }

    template<typename C>
    void fpu<C>::update_pointers(uint32_t opcode)
    {
        fpu_cs = cpu_get_cs();
        fpu_eip = cpu_get_eip();
        fpu_opcode = opcode;
    }


    template<typename C>
    void fpu<C>::update_pointers2(uint32_t opcode, uint32_t virtaddr, uint32_t seg)
    {
        //if (VIRT_EIP() == 0x759783bb)
        //    __asm__("int3");
        fpu_cs = cpu_get_cs();
        fpu_eip = cpu_get_eip();
        fpu_opcode = opcode;
        fpu_data_ptr = virtaddr;
        fpu_data_seg = cpu_get_seg(seg);
    }



    template<typename C>
    int fpu<C>::write_float32(uint32_t linaddr, float32 src) {
        cpu_write32(linaddr, src);
        return 0;
    }


    template<typename C>
    int fpu<C>::write_float64(uint32_t linaddr, float64 dest) {
        uint64_t x = dest;
        cpu_write32(linaddr, (uint32_t) x);
        cpu_write32(linaddr + 4, (uint32_t) (x >> 32));
        return 0;
    }

    template<typename C>
    int fpu<C>::check_push() {
        if (check_stack_overflow(-1)) {
            check_exceptions();
            if (control_word & FPU_EXCEPTION_INVALID_OPERATION) {
                // masked response
                push(IndefiniteNaN);
            } else status_word |= 0x80; // ?
            return 1;
        }
        return 0;
    }

    template<typename C>
    int fpu<C>::store_f80(uint32_t linaddr, floatx80 *data) {
        uint16_t exponent;
        uint64_t mantissa;
        floatx80_unpack(data, exponent, mantissa);
        cpu_write32(linaddr, (uint32_t) mantissa);
        cpu_write32(linaddr + 4, (uint32_t) (mantissa >> 32));
        cpu_write16(linaddr + 8, exponent);
        return 0;
    }

    template<typename C>
    int fpu<C>::read_f80(uint32_t linaddr, floatx80 *data) {
        uint16_t exponent;
        uint32_t low, hi;
        cpu_read32(linaddr, low);
        cpu_read32(linaddr + 4, hi);
        cpu_read16(linaddr + 8, exponent);
        floatx80_repack(data, exponent, (uint64_t) low | (uint64_t) hi << 32);
        return 0;
    }

// Actual FPU operations
    template<typename C>
    int fpu<C>::fcom(floatx80 op1, floatx80 op2, int unordered) {
        int relation = floatx80_compare_internal(op1, op2, unordered, &status);
        if (check_exceptions())
            return 1;
        int bad = relation == float_relation_unordered;
        set_c0(bad | (relation == float_relation_less));
        set_c2(bad);
        set_c3(bad | (relation == float_relation_equal));

        return 0;
    }

    template<typename C>
    int fpu<C>::fcomi(floatx80 op1, floatx80 op2, int unordered) {
        int relation = floatx80_compare_internal(op1, op2, unordered, &status);
        if (check_exceptions())
            return 1;
        int bad = relation == float_relation_unordered;

        int cf = bad | (relation == float_relation_less);
        int pf = bad;
        int zf = bad | (relation == float_relation_equal);
        cpu_set_cf(cf);
        cpu_set_pf(pf);
        cpu_set_zf(zf);

        return 0;
    }


    template<typename C>
    int fpu<C>::fstenv(uint32_t linaddr, int code16)
    {
        //fpu_debug();
        for (int i = 0; i < 8; i++) {
            if (get_tag(i) != FPU_TAG_EMPTY)
                set_tag(i, fpu_get_tag_from_value(&st[(ftop + i) & 7]));
        }
        // https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-1-manual.pdf
        // page 203
        //fpu_debug();
        //__asm__("int3");
        if (!code16) {
            cpu_write32(linaddr, 0xFFFF0000 | control_word);
            cpu_write32(linaddr + 4, 0xFFFF0000 | get_status_word());
            cpu_write32(linaddr + 8, 0xFFFF0000 | tag_word);
            if (cpu_is_protected()) {
                cpu_write32(linaddr + 12, fpu_eip);
                cpu_write32(linaddr + 16, fpu_cs | (fpu_opcode << 16));
                cpu_write32(linaddr + 20, fpu_data_ptr);
                cpu_write32(linaddr + 24, 0xFFFF0000 | fpu_data_seg);
            } else {
                uint32_t linear_fpu_eip = fpu_eip + (fpu_cs << 4);
                uint32_t linear_fpu_data = fpu_data_ptr + (fpu_data_seg << 4);
                cpu_write32(linaddr + 12, linear_fpu_eip | 0xFFFF0000);
                cpu_write32(linaddr + 16, (fpu_opcode & 0x7FF) | (linear_fpu_eip >> 4 & 0x0FFFF000));
                cpu_write32(linaddr + 20, linear_fpu_data | 0xFFFF0000);
                cpu_write32(linaddr + 24, linear_fpu_data >> 4 & 0x0FFFF000);
            }
        } else {
            cpu_write16(linaddr, control_word);
            cpu_write16(linaddr + 2, get_status_word());
            cpu_write16(linaddr + 4, tag_word);
            if (cpu_is_protected()) {
                cpu_write16(linaddr + 6, fpu_eip);
                cpu_write16(linaddr + 8, fpu_cs);
                cpu_write16(linaddr + 10, fpu_data_ptr);
                cpu_write16(linaddr + 12, fpu_data_seg);
            } else {
                uint32_t linear_fpu_eip = fpu_eip + (fpu_cs << 4);
                uint32_t linear_fpu_data = fpu_data_ptr + (fpu_data_seg << 4);
                cpu_write16(linaddr + 6, linear_fpu_eip);
                cpu_write16(linaddr + 8, (fpu_opcode & 0x7FF) | (linear_fpu_eip >> 4 & 0xF000));
                cpu_write16(linaddr + 10, linear_fpu_data);
                cpu_write16(linaddr + 12, linear_fpu_data >> 4 & 0xF000);
            }
        }
        return 0;
    }
    template<typename C>
    int fpu<C>::fldenv(uint32_t linaddr, int code16)
    {
        uint32_t temp32;
        uint16_t temp16;
        if (!code16) {
            cpu_read32(linaddr, temp32);
            set_control_word(temp32);

            cpu_read16(linaddr + 4, status_word);
            ftop = status_word >> 11 & 7;
            status_word &= ~(7 << 11); // Clear FTOP.

            cpu_read16(linaddr + 8, tag_word);
            if (cpu_is_protected()) {
                cpu_read32(linaddr + 12, fpu_eip);

                cpu_read32(linaddr + 16, temp32);
                fpu_cs = temp32 & 0xFFFF;
                fpu_opcode = temp32 >> 16 & 0x7FF;

                cpu_read32(linaddr + 20, fpu_data_ptr);
                cpu_read32(linaddr + 24, temp32);
                fpu_data_seg = temp32;
            } else {
                fpu_cs = 0;
                fpu_eip = 0;
                cpu_read16(linaddr + 12, temp16);
                fpu_eip = temp16;

                cpu_read32(linaddr + 16, temp32);
                fpu_opcode = temp32 & 0x7FF;
                fpu_eip |= temp32 << 4 & 0xFFFF0000;

                cpu_read32(linaddr + 20, temp32);
                fpu_data_ptr = temp32 & 0xFFFF;

                cpu_read32(linaddr + 24, temp32);
                fpu_eip |= temp32 << 4 & 0xFFFF0000;
            }
        } else {
            cpu_read16(linaddr, temp16);
            set_control_word(temp16);

            cpu_read16(linaddr + 2, status_word);
            ftop = status_word >> 11 & 7;
            status_word &= ~(7 << 11); // Clear FTOP.

            cpu_read16(linaddr + 4, tag_word);
            if (cpu_is_protected()) {
                cpu_read16(linaddr + 6, temp16);
                fpu_eip = temp16;
                cpu_read16(linaddr + 8, fpu_cs);
                cpu_read16(linaddr + 10, temp16);
                fpu_data_ptr = temp16;
                cpu_read16(linaddr + 12, fpu_data_seg);
            } else {
                fpu_cs = 0;
                fpu_eip = 0;
                cpu_read16(linaddr + 6, temp16);
                fpu_eip = temp16;

                cpu_read16(linaddr + 8, temp16);
                fpu_opcode = temp16 & 0x7FF;
                fpu_eip |= temp16 << 4 & 0xF0000;

                cpu_read16(linaddr + 10, temp16);
                fpu_data_ptr = temp16 & 0xFFFF;

                cpu_read32(linaddr + 12, temp32);
                fpu_eip |= temp32 << 4 & 0xF0000;
            }
        }
        if (status_word & ~control_word & 0x3F)
            status_word |= 0x8080;
        else
            status_word &= ~0x8080;
        return 0;
    }

    template<typename C>
    void fpu<C>::watchpoint() {
        // For debugging purposes
        //if(VIRT_EIP() == 0x71961cad) __asm__("int3");
        //if(fpu.st[5].fraction == 0x00000006e8b877f6) __asm__("int3");
    }

    template<typename C>
    void fpu<C>::watchpoint2() {
        // For debugging purposes
        //if(fpu.fpu_opcode == 0x77F8) __asm__("int3");
    }

#define FPU_EXCEP() return 1
#define FPU_ABORT()        \
    do {                   \
        watchpoint2(); \
        return 0;          \
    } while (0) // Not an exception, so keep on goings

// Run a FPU operation that does not require memory
#define OP(op, reg) (op & 7) << 3 | reg

    template<typename C>
    int fpu<C>::reg_op(uint32_t opcode) {
        floatx80 temp80;
        if (nm_check())
            return 1;
        watchpoint();

        status.float_exception_flags = 0;
        int smaller_opcode = (opcode >> 5 & 0x38) | (opcode >> 3 & 7);

        switch (smaller_opcode) {
            case OP(0xD8, 0):
            case OP(0xD8, 1):
            case OP(0xD8, 4):
            case OP(0xD8, 5):
            case OP(0xD8, 6):
            case OP(0xD8, 7):
            case OP(0xDC, 0):
            case OP(0xDC, 1):
            case OP(0xDC, 4):
            case OP(0xDC, 5):
            case OP(0xDC, 6):
            case OP(0xDC, 7):
            case OP(0xDE, 0):
            case OP(0xDE, 1):
            case OP(0xDE, 4):
            case OP(0xDE, 5):
            case OP(0xDE, 6):
            case OP(0xDE, 7): {
                int st_index = opcode & 7;
                floatx80 dst;
                if (fwait())
                    return 1;
                update_pointers(opcode);
                if (check_stack_underflow(0, 1) || check_stack_underflow(st_index, 1))
                    FPU_ABORT();

                switch (smaller_opcode & 7) {
                    case 0: // FADD - Floating point add
                        dst = floatx80_add(get_st(0), get_st(st_index), &status);
                        break;
                    case 1: // FMUL - Floating point multiply
                        dst = floatx80_mul(get_st(0), get_st(st_index), &status);
                        break;
                    case 4: // FSUB - Floating point subtract
                        dst = floatx80_sub(get_st(0), get_st(st_index), &status);
                        break;
                    case 5: // FSUBR - Floating point subtract reverse
                        dst = floatx80_sub(get_st(st_index), get_st(0), &status);
                        break;
                    case 6: // FDIV - Floating point divide
                        dst = floatx80_div(get_st(0), get_st(st_index), &status);
                        break;
                    case 7: // FDIVR - Floating point divide reverse
                        dst = floatx80_div(get_st(st_index), get_st(0), &status);
                        break;
                }
                if (!check_exceptions()) {
                    if (smaller_opcode & 32) {
                        set_st(st_index, dst);
                        if (smaller_opcode & 16)
                            pop();
                    } else
                        set_st(0, dst);
                }
                break;
            }
            case OP(0xD8, 2): // FCOM - Floating point compare
            case OP(0xD8, 3): // FCOMP - Floating point compare and pop
            case OP(0xDC, 2):
            case OP(0xDC, 3):
            case OP(0xDE, 2): { // Aliases of the DB opcodes
                if (fwait())
                    FPU_ABORT();
                if (check_stack_underflow(0, 1) || check_stack_underflow(opcode & 7, 1)) {
                    set_c0(1);
                    set_c2(1);
                    set_c3(1);
                }
                update_pointers(opcode);
                if (!fcom(get_st(0), get_st(opcode & 7), 0)) {
                    if (smaller_opcode & 1 || smaller_opcode == (OP(0xDE, 2)))
                        pop();
                }
                break;
            }
            case OP(0xD9, 0): // FLD - Load floating point value
                if (fwait())
                    FPU_ABORT();
                update_pointers(opcode);
                if (check_stack_underflow(opcode & 7, 1) || check_push())
                    FPU_ABORT();
                temp80 = get_st(opcode & 7);
                push(temp80);
                break;
            case OP(0xD9, 1): // FXCH - Floating point exchange
            case OP(0xDD, 1):
            case OP(0xDF, 1): // alias of FXCH
                if (fwait())
                    FPU_ABORT();
                update_pointers(opcode);
                if (check_stack_underflow(0, 1) || check_stack_underflow(1, 1))
                    FPU_ABORT();
                temp80 = get_st(0);
                set_st(0, get_st(opcode & 7));
                set_st(opcode & 7, temp80);
                break;
            case OP(0xD9, 2): // FNOP
                if (fwait())
                    FPU_ABORT();
                update_pointers(opcode);
                break;
            case OP(0xD9, 4):
                if (fwait())
                    FPU_ABORT();
                update_pointers(opcode);
                if ((opcode & 7) != 5)
                    if (check_stack_underflow(0, 1))
                        FPU_ABORT();
                temp80 = get_st(0);
                switch (opcode & 7) {
                    case 0: // FCHS - Flip sign of floating point number
                        floatx80_chs(&temp80);
                        break;
                    case 1: // FABS - Find absolute value of floating point number
                        floatx80_abs(&temp80);
                        break;
                    case 4: // FTST - Compare floating point register to 0
                        if (fcom(temp80, Zero, 0))
                            FPU_ABORT();
                        return 0;
                    case 5: { // FXAM - Examine floating point number
                        int unordered = 0;
                        uint16_t exponent;
                        uint64_t mantissa;
                        floatx80_unpack(&temp80, exponent, mantissa);
                        if (get_tag(0) == FPU_TAG_EMPTY)
                            unordered = 5;
                        else {
                            if (is_invalid(exponent, mantissa))
                                unordered = 0;
                            else if (is_nan(exponent, mantissa))
                                unordered = 1;
                            else if (is_infinity(exponent, mantissa))
                                unordered = 3;
                            else if (is_zero_any_sign(exponent, mantissa))
                                unordered = 4;
                            else if (is_denormal(exponent, mantissa))
                                unordered = 6;
                            else
                                unordered = 2;
                        }
                        set_c0(unordered & 1);
                        set_c1(exponent >> 15 & 1); // Get sign
                        set_c2(unordered >> 1 & 1);
                        set_c3(unordered >> 2 & 1);
                        return 0;
                    }
                }
                set_st(0, temp80);
                break;
            case OP(0xD9, 5): // FLD - Load floating point constants
                if (fwait())
                    FPU_ABORT();
                update_pointers(opcode);

                if (check_push())
                    FPU_ABORT();
                push(*Constants[opcode & 7]);
                break;
            case OP(0xD9, 6): { // Various complex FPU operations
                if (fwait())
                    FPU_ABORT();
                update_pointers(opcode);
                floatx80 res, temp;
                int temp2, old_rounding;
                switch (opcode & 7) {
                    case 0: // D9 F0: F2XM1 - Compute 2^ST(0) - 1
                        if (check_stack_underflow(0, 1))
                            FPU_ABORT();
                        res = f2xm1(get_st(0), &status);
                        if (!check_exceptions())
                            set_st(0, res);
                        break;
                    case 1: // D9 F1: FYL2X - Compute ST(1) * log2(ST(0)) and then pop
                        if (check_stack_underflow(0, 1) || check_stack_underflow(1, 1))
                            FPU_ABORT();

                        old_rounding = status.float_rounding_precision;
                        status.float_rounding_precision = 80;
                        res = fyl2x(get_st(0), get_st(1), &status);
                        status.float_rounding_precision = old_rounding;

                        if (!check_exceptions()) {
                            set_st(1, res);
                            pop();
                        }
                        break;
                    case 2: // D9 F2: FPTAN - Compute tan(ST(0)) partially
                        if (check_stack_underflow(0, 1))
                            FPU_ABORT();
                        res = get_st(0);
                        if (!ftan(&res, &status))
                            set_st(0, res);
                        break;
                    case 3: // D9 F3: FPATAN - Compute tan-1(ST(0)) partially
                        if (check_stack_underflow(0, 1) || check_stack_underflow(1, 1))
                            FPU_ABORT();
                        res = fpatan(get_st(0), get_st(1), &status);
                        if (!check_exceptions()) {
                            pop();
                            set_st(0, res);
                        }
                        break;
                    case 4: // D9 F4: FXTRACT - Extract Exponent and mantissa of ST0
                        if (check_stack_underflow(0, 1))
                            FPU_ABORT();
                        if (check_stack_overflow(-1))
                            FPU_ABORT();
                        temp = get_st(0);
                        res = floatx80_extract(&temp, &status);
                        if (!check_exceptions()) {
                            set_st(0, res);
                            push(temp);
                        }
                        break;
                    case 5: { // D9 F5: FPREM1 - Partial floating point remainder
                        floatx80 st0 = get_st(0), st1 = get_st(1);
                        uint64_t quo;
                        temp2 = floatx80_ieee754_remainder(st0, st1, &temp, &quo, &status);
                        if (!check_exceptions()) {
                            if (!(temp2 < 0)) {
                                set_c0(0);
                                set_c2(0);
                                set_c3(0);
                                if (temp2 > 0) {
                                    set_c2(1);
                                } else {
                                    // 1 2 4 - 1 3 0
                                    if (quo & 1)
                                        set_c1(1);
                                    if (quo & 2)
                                        set_c3(1);
                                    if (quo & 4)
                                        set_c0(1);
                                }
                            }
                            set_st(0, temp);
                        }
                    }
                        break;
                    case 6: // D9 F6: FDECSTP - Decrement stack pointer
                        set_c1(0);
                        ftop = (ftop - 1) & 7;
                        break;
                    case 7: // D9 F7: FINCSTP - Increment stack pointer
                        set_c1(0);
                        ftop = (ftop + 1) & 7;
                        break;
                }
                break;
            }
            case OP(0xD9, 7): {
                if (fwait())
                    return 1;
                update_pointers(opcode);

                // Check for FPU registers
                if (check_stack_underflow(0, 1))
                    FPU_ABORT();

                int flags;
                bool should_pop = false;
                floatx80 dest;
                uint64_t quotient;
                switch (opcode & 7) {
                    case 0: // FPREM - Floating point partial remainder (8087/80287 compatible)
                        if (check_stack_underflow(1, 1))
                            FPU_ABORT();
                        flags = floatx80_remainder(get_st(0), get_st(1), &dest, &quotient, &status);
                        if (!check_exceptions()) {
                            if (flags < 0) {
                                set_c0(0);
                                set_c1(0);
                                set_c2(0);
                                set_c3(0);
                            } else {
                                if (flags != 0) {
                                    set_c0(0);
                                    set_c1(0);
                                    set_c2(1);
                                    set_c3(0);
                                } else {
                                    set_c0(quotient >> 2 & 1);
                                    set_c1(quotient & 1);
                                    set_c2(0);
                                    set_c3(quotient >> 1 & 1);
                                }
                            }
                            set_st(0, dest);
                        }
                        break;
                    case 1: // FYL2XP1 - Compute ST1 * log2(ST0 + 1) and pop
                        if (check_stack_underflow(1, 1))
                            FPU_ABORT();
                        dest = fyl2xp1(get_st(0), get_st(1), &status);
                        if (!check_exceptions()) {
                            pop();
                            set_st(0, dest);
                        }
                        return 0;
                    case 2: // FSQRT - Compute sqrt(ST0)
                        dest = floatx80_sqrt(get_st(0), &status);
                        break;
                    case 3: { // FSINCOS - Compute sin(ST0) and sin(ST1)
                        // TODO: What if exceptions are masked?
                        if (check_stack_overflow(-1))
                            FPU_ABORT();
                        floatx80 sinres, cosres;
                        if (fsincos(get_st(0), &sinres, &cosres, &status) == -1) {
                            set_c2(1);
                        } else if (!check_exceptions()) {
                            set_st(0, sinres);
                            push(cosres);
                        }
                        return 0;
                    }
                    case 4: // FRNDINT - Round ST0
                        dest = floatx80_round_to_int(get_st(0), &status);
                        break;
                    case 5: // FSCALE - Scale ST0
                        if (check_stack_underflow(0, 1) || check_stack_underflow(1, 1))
                            FPU_ABORT();
                        dest = floatx80_scale(get_st(0), get_st(1), &status);
                        break;
                    case 6: // FSIN - Find sine of ST0
                        dest = get_st(0);
                        if (fsin(&dest, &status) == -1) {
                            set_c2(1);
                            FPU_ABORT();
                        }
                        break;
                    case 7: // FCOS - Find cosine of ST0
                        dest = get_st(0);
                        if (fcos(&dest, &status) == -1) {
                            set_c2(1);
                            FPU_ABORT();
                        }
                        break;
                }
                if (!check_exceptions()) {
                    set_st(0, dest);
                    if (should_pop)
                        pop();
                }
                break;
            }
            case OP(0xDA, 0):
            case OP(0xDB, 0): // FCMOV(N)B - Move floating point to register ST0 if condition code is set
                if (fwait())
                    return 1;
                update_pointers(opcode);
                if (check_stack_underflow(0, 1) && check_stack_underflow(opcode & 7, 1))
                    FPU_ABORT();
                if (cpu_get_cf() ^ (smaller_opcode >> 3 & 1))
                    set_st(0, get_st(opcode & 7));
                break;
            case OP(0xDB, 1):
            case OP(0xDA, 1): // FCMOV(N)E - Move floating point to register ST0 if condition code is set
                if (fwait())
                    return 1;
                update_pointers(opcode);
                if (check_stack_underflow(0, 1) && check_stack_underflow(opcode & 7, 1))
                    FPU_ABORT();
                if (cpu_get_zf() ^ (smaller_opcode >> 3 & 1))
                    set_st(0, get_st(opcode & 7));
                break;
            case OP(0xDB, 2):
            case OP(0xDA, 2): // FCMOV(N)BE - Move floating point to register ST0 if condition code is set
                if (fwait())
                    return 1;
                update_pointers(opcode);
                if (check_stack_underflow(0, 1) && check_stack_underflow(opcode & 7, 1))
                    FPU_ABORT();
                if ((cpu_get_zf() || cpu_get_cf()) ^ (smaller_opcode >> 3 & 1))
                    set_st(0, get_st(opcode & 7));
                break;
            case OP(0xDB, 3):
            case OP(0xDA, 3): // FCMOV(N)U - Move floating point to register ST0 if condition code is set
                if (fwait())
                    return 1;
                update_pointers(opcode);
                if (check_stack_underflow(0, 1) && check_stack_underflow(opcode & 7, 1))
                    FPU_ABORT();
                if (cpu_get_pf() ^ (smaller_opcode >> 3 & 1))
                    set_st(0, get_st(opcode & 7));
                break;
            case OP(0xDA, 5): // FUCOMPP
                if (fwait())
                    return 1;
                update_pointers(opcode);
                if ((opcode & 7) == 1) {
                    if (check_stack_underflow(0, 1) || check_stack_underflow(1, 1)) {
                        set_c0(1);
                        set_c2(1);
                        set_c3(1);
                    }
                    if (fcom(get_st(0), get_st(1), 1))
                        FPU_ABORT();

                    if (!check_exceptions()) {
                        pop();
                        pop();
                    }
                }
                break;
            case OP(0xDB, 4):
                switch (opcode & 7) {
                    case 0 ... 1:
                    case 4:
                        // All 286 opcodes, nop
                        return 0;
                    case 2: // DB E2: FNCLEX - Clear FPU exceptions
                        status_word &= ~(0x80FF);
                        break;
                    case 3: // DB E3: FNINIT - Reset Floating point state
                        fninit();
                        break;
                    default:
                        cpu_undefined_instruction();
                }
                break;
            case OP(0xDB, 5):
            case OP(0xDB, 6): // F(U)COMI : Do an (un)ordered compare, and set EFLAGS.
                if (fwait())
                    return 1;
                update_pointers(opcode);

                cpu_set_eflags(
                        cpu_get_eflags() & ~(EFLAGS_OF | EFLAGS_SF | EFLAGS_ZF | EFLAGS_AF | EFLAGS_PF | EFLAGS_CF));
                if (check_stack_underflow(0, 1) || check_stack_underflow(opcode & 7, 1)) {
                    cpu_set_zf(1);
                    cpu_set_pf(1);
                    cpu_set_cf(1);
                    FPU_ABORT();
                }
                if (fcomi(get_st(0), get_st(opcode & 7), smaller_opcode & 1))
                    FPU_ABORT();
                break;
            case OP(0xD9, 3): // FSTP (alias)
            case OP(0xDD, 2): // FST - Store floating point value
            case OP(0xDD, 3):
            case OP(0xDF, 2):
            case OP(0xDF, 3): { // FSTP - Store floating point value and pop
                if (fwait())
                    FPU_ABORT();
                update_pointers(opcode);
                if (check_stack_underflow(0, 1)) {
                    if (exception_masked(FPU_EXCEPTION_STACK_FAULT))
                        pop();
                    FPU_ABORT();
                }
                set_st(opcode & 7, get_st(0));
                if (smaller_opcode & 1 || (smaller_opcode & ~1) == (OP(0xDF, 2)))
                    pop();
                break;
            }
            case OP(0xDD, 0): // FFREE - Free floating point value
            case OP(0xDF, 0): { // FFREEP - Free floaing point value and pop
                if (fwait())
                    FPU_ABORT();
                update_pointers(opcode);
                set_tag(opcode & 7, FPU_TAG_EMPTY);
                if (smaller_opcode == (OP(0xDF, 0)))
                    pop();
                break;
            }
            case OP(0xDD, 4): // FUCOM - Unordered compare
            case OP(0xDD, 5): // FUCOMP
                if (fwait())
                    return 1;
                update_pointers(opcode);
                if (check_stack_underflow(0, 1) || check_stack_underflow(opcode & 7, 1)) {
                    set_c0(1);
                    set_c2(1);
                    set_c3(1);
                }
                if (fcom(get_st(0), get_st(opcode & 7), 1))
                    FPU_ABORT();

                if (!check_exceptions()) {
                    if (smaller_opcode & 1)
                        pop();
                }
                break;
            case OP(0xDE, 3): // FCOMPP - Floating point compare and pop twice
                if (fwait())
                    FPU_ABORT();
                update_pointers(opcode);
                if (check_stack_underflow(0, 1) || check_stack_underflow(opcode & 7, 1)) {
                    if (!check_exceptions()) {
                        // Masked response
                        set_c0(1);
                        set_c2(1);
                        set_c3(1);
                    }
                    FPU_ABORT();
                }
                if (fcom(get_st(0), get_st(1), 0))
                    FPU_ABORT();
                pop();
                pop();
                break;
            case OP(0xDF, 4): // FSTSW - Store status word
                if ((opcode & 7) != 0) // AX
                    cpu_undefined_instruction();
                cpu_set_ax(get_status_word());
                break;

            case OP(0xDF, 5): // FUCOMIP - Unordered compare and pop
            case OP(0xDF, 6): { // FCOMIP - Ordered compare and pop
                if (fwait())
                    return 1;
                update_pointers(opcode);
                // Clear all flags
                cpu_set_eflags(
                        cpu_get_eflags() & ~(EFLAGS_OF | EFLAGS_SF | EFLAGS_ZF | EFLAGS_AF | EFLAGS_PF | EFLAGS_CF));

                if (check_stack_underflow(0, 1) || check_stack_underflow(opcode & 7, 1)) {
                    cpu_set_zf(1);
                    cpu_set_pf(1);
                    cpu_set_cf(1);
                    FPU_ABORT();
                }
                if (fcomi(get_st(0), get_st(opcode & 7), smaller_opcode & 1))
                    FPU_ABORT();
                pop();
                break;
            }
            case OP(0xDA, 4):
            case OP(0xDA, 6):
            case OP(0xDA, 7):
            case OP(0xDD, 6):
            case OP(0xDD, 7):
            case OP(0xDF, 7): { // Invalid
                int major_opcode = opcode >> 8 | 0xD8;
                (void)major_opcode; // In case log is disabled
                //CPU_LOG("Unknown FPU register operation: %02x %02x [%02x /%d] internal=%d\n", major_opcode, opcode & 0xFF, major_opcode, opcode >> 3 & 7, (opcode >> 5 & 0x38) | (opcode >> 3 & 7));
                //EXCEPTION_UD();
                cpu_undefined_instruction();
                break;
            }
            default: {
                int major_opcode = opcode >> 8 | 0xD8;
                //CPU_FATAL("Unknown FPU register operation: %02x %02x [%02x /%d] internal=%d\n", major_opcode, opcode & 0xFF, major_opcode, opcode >> 3 & 7, (opcode >> 5 & 0x38) | (opcode >> 3 & 7));
                cpu_undefined_instruction();
            }
        }
        watchpoint2();
        return 0;
    }

    template<typename C>
    int fpu<C>::mem_op(uint32_t opcode, uint32_t linaddr, uint32_t virtaddr, uint32_t seg)
    {
        floatx80 temp80;
        float64 temp64;
        float32 temp32;
        uint16_t temp16;

        if (nm_check())
            return 1;

        watchpoint();

        status.float_exception_flags = 0;
        int smaller_opcode = (opcode >> 5 & 0x38) | (opcode >> 3 & 7);
        switch (smaller_opcode) {
            // Basic arithmetic

            case OP(0xD8, 0):
            case OP(0xD8, 1):
            case OP(0xD8, 2):
            case OP(0xD8, 3):
            case OP(0xD8, 4):
            case OP(0xD8, 5):
            case OP(0xD8, 6):
            case OP(0xD8, 7):
            case OP(0xD9, 0):
            case OP(0xDA, 0):
            case OP(0xDA, 1):
            case OP(0xDA, 2):
            case OP(0xDA, 3):
            case OP(0xDA, 4):
            case OP(0xDA, 5):
            case OP(0xDA, 6):
            case OP(0xDA, 7):
            case OP(0xDB, 0):
            case OP(0xDC, 0):
            case OP(0xDC, 1):
            case OP(0xDC, 2):
            case OP(0xDC, 3):
            case OP(0xDC, 4):
            case OP(0xDC, 5):
            case OP(0xDC, 6):
            case OP(0xDC, 7):
            case OP(0xDD, 0):
            case OP(0xDE, 0):
            case OP(0xDE, 1):
            case OP(0xDE, 2):
            case OP(0xDE, 3):
            case OP(0xDE, 4):
            case OP(0xDE, 5):
            case OP(0xDE, 6):
            case OP(0xDE, 7):
            case OP(0xDF, 0): {
                //printf("%08x %02x /%d\n", halfix.phys_eip, (smaller_opcode >> 3) | 0xD8, opcode & 7);
                //if(halfix.phys_eip == 0x0018b3f0) __asm__("int3");
                if (fwait())
                    return 1;
                switch (opcode >> 9 & 3) {
                    case 0:
                        cpu_read32(linaddr, temp32);
                        temp80 = float32_to_floatx80(temp32, &status);
                        break;
                    case 1:
                        cpu_read32(linaddr, temp32);
                        temp80 = int32_to_floatx80(temp32);
                        break;
                    case 2: {
                        uint32_t low, hi;
                        uint64_t res;
                        cpu_read32(linaddr, low);
                        cpu_read32(linaddr + 4, hi);
                        res = (uint64_t)low | (uint64_t)hi << 32;
                        temp80 = float64_to_floatx80(res, &status);
                        break;
                    }
                    case 3: {
                        cpu_read16(linaddr, temp16);
                        temp80 = int32_to_floatx80((int16_t)temp16);
                        break;
                    }
                }
                update_pointers2(opcode, virtaddr, seg);

                // Make sure we won't stack fault
                int op = smaller_opcode & 15;
                if ((op & 8) == 0) { // Don't do this for ST0
                    if (check_stack_underflow(0, 1)) {
                        if ((smaller_opcode & 14) == 2) {
                            // For FCOM/FCOMP, set condition codes to 1
                            set_c0(1);
                            set_c2(1);
                            set_c3(1);
                        }
                        return 0;
                    }
                } else {
                    if (check_push())
                        FPU_ABORT();
                }
                floatx80 st0 = get_st(0);
                switch (op) {
                    case 0: // FADD - Floating point add
                        st0 = floatx80_add(st0, temp80, &status);
                        break;
                    case 1: // FMUL - Floating point multiply
                        st0 = floatx80_mul(st0, temp80, &status);
                        break;
                    case 2: // FCOM - Floating point compare
                    case 3: // FCOMP - Floating point compare and pop
                        if (!fcom(st0, temp80, 0)) {
                            if (smaller_opcode & 1)
                                pop();
                        }
                        return 0;
                    case 4: // FSUB - Floating point subtract
                        st0 = floatx80_sub(st0, temp80, &status);
                        break;
                    case 5: // FSUBR - Floating point subtract with reversed operands
                        st0 = floatx80_sub(temp80, st0, &status);
                        break;
                    case 6: // FDIV - Floating point divide
                        st0 = floatx80_div(st0, temp80, &status);
                        break;
                    case 7: // FDIVR - Floating point divide with reversed operands
                        st0 = floatx80_div(temp80, st0, &status);
                        break;
                    default: // FLD
                        if (!check_exceptions())
                            push(temp80);
                        return 0;
                }
                if (!check_exceptions())
                    set_st(0, st0);
                break;
            }

            case OP(0xD9, 2): // FST - Store floating point register
            case OP(0xD9, 3): { // FSTP m64 - Store floating point register and pop
                if (fwait())
                    return 1;
                update_pointers2(opcode, virtaddr, seg);

                if (check_stack_underflow(0, 0))
                    FPU_ABORT();
                temp32 = floatx80_to_float32(get_st(0), &status);
                if (!check_exceptions2(0)) {
                    if (write_float32(linaddr, temp32))
                        FPU_EXCEP();
                    commit_sw();
                    if (smaller_opcode & 1)
                        pop();
                }
                break;
            }
            case OP(0xD9, 5): { // FLDCW
                uint16_t cw;
                cpu_read16(linaddr, cw);
                set_control_word(cw);
                break;
            }
            case OP(0xD9, 6): { // FSTENV
                int is16 = cpu_is_code16();
                if (fstenv(linaddr, is16))
                    FPU_ABORT();
                break;
            }
            case OP(0xD9, 7): // FSTCW - Store control word to memory
                cpu_write16(linaddr, control_word);
                break;
            case OP(0xDB, 1): // FISTTP - Store floating point register (truncate to 0) to memory and pop
            case OP(0xDB, 2): // FIST - Store floating point register (converted to integer) to memory
            case OP(0xDB, 3): // FISTP - Store floating point register (converted to integer) to memory and pop
            case OP(0xDD, 1): // FISTTP
            case OP(0xDF, 1):
            case OP(0xDF, 2):
            case OP(0xDF, 3): {
                if (fwait())
                    return 1;
                //fpu_debug();
                update_pointers2(opcode, virtaddr, seg);
                if (check_stack_underflow(0, 0))
                    FPU_ABORT();
                switch (smaller_opcode >> 4 & 3) { // Extract the upper 2 bits of the small opcode
                    case 1: { // DB
                        uint32_t res;
                        if (smaller_opcode & 2)
                            res = floatx80_to_int32(get_st(0), &status);
                        else
                            res = floatx80_to_int32_round_to_zero(get_st(0), &status);
                        if (!check_exceptions2(0))
                            cpu_write32(linaddr, res);
                        break;
                    }
                    case 2: { // DD
                        uint64_t res;
                        if (smaller_opcode & 2)
                            res = floatx80_to_int64(get_st(0), &status);
                        else
                            res = floatx80_to_int64_round_to_zero(get_st(0), &status);
                        if (!check_exceptions2(0)) {
                            cpu_write32(linaddr, res);
                            cpu_write32(linaddr + 4, res >> 32);
                        }
                        break;
                    }
                    case 3: { // DF
                        uint16_t res;
                        if (smaller_opcode & 2)
                            res = floatx80_to_int16(get_st(0), &status);
                        else
                            res = floatx80_to_int16_round_to_zero(get_st(0), &status);
                        if (!check_exceptions2(0))
                            cpu_write16(linaddr, res);
                        break;
                    }
                }
                if (!check_exceptions2(0)) { // XXX eliminate this
                    if (smaller_opcode & 1)
                        pop();
                }
                commit_sw();
                break;
            }
            case OP(0xD9, 4): { // FLDENV - Load floating point environment from memory
                if (fldenv(linaddr, cpu_is_code16()))
                    FPU_ABORT();
                break;
            }
            case OP(0xDB, 5): { // FLD - Load floating point register from memory
                if (fwait())
                    return 1;
                if (read_f80(linaddr, &temp80))
                    return 1;
                update_pointers2(opcode, virtaddr, seg);
                if (check_stack_overflow(-1))
                    FPU_ABORT();
                push(temp80);
                break;
            }
            case OP(0xDB, 7): { // FSTP - Store floating point register to memory and pop
                if (fwait())
                    return 1;
                update_pointers2(opcode, virtaddr, seg);
                if (check_stack_underflow(0, 1))
                    FPU_ABORT();
                if (store_f80(linaddr, get_st_ptr(0)))
                    return 1;
                pop();
                break;
            }
            case OP(0xDD, 2): // FST - Store floating point register to memory
            case OP(0xDD, 3): { // FSTP - Store floating point register to memory and pop
                if (fwait())
                    return 1;
                //fpu_debug();
                update_pointers2(opcode, virtaddr, seg);

                if (check_stack_underflow(0, 0))
                    FPU_ABORT();
                temp64 = floatx80_to_float64(get_st(0), &status);
                if (!check_exceptions2(0)) {
                    if (write_float64(linaddr, temp64))
                        FPU_EXCEP();
                    commit_sw();
                    if (smaller_opcode & 1)
                        pop();
                }
                break;
            }
            case OP(0xDD, 4): { // FRSTOR -- Load FPU context
                int is16 = cpu_is_code16();
                if (fldenv(linaddr, is16))
                    FPU_ABORT();
                int offset = 14 << !is16;
                for (int i = 0; i < 8; i++) {
                    if (read_f80(offset + linaddr, &st[(ftop + i) & 7]))
                        FPU_EXCEP();
                    offset += 10;
                }
                break;
            }
            case OP(0xDD, 6): { // FSAVE - Save FPU environment to memory
                int is16 = cpu_is_code16();
                if (fstenv(linaddr, is16))
                    FPU_ABORT();
                int offset = 14 << !is16;
                for (int i = 0; i < 8; i++) {
                    if (store_f80(offset + linaddr, &st[(ftop + i) & 7]))
                        FPU_EXCEP();
                    offset += 10;
                }
                fninit();
                break;
            }
            case OP(0xDD, 7): // FSTSW - Store status word to memory
                cpu_write16(linaddr, get_status_word());
                break;
            case OP(0xDF, 4): { // FBLD - The infamous "load BCD" instruction. Loads BCD integer and converts to floatx80
                uint32_t low, high;
                uint16_t higher;
                if (fwait())
                    return 1;
                cpu_read32(linaddr, low);
                cpu_read32(linaddr + 4, high);
                cpu_read16(linaddr + 8, higher);
                update_pointers2(opcode, virtaddr, seg);

                uint64_t result = 0;
                int sign = higher & 0x8000;
                higher &= 0x7FFF;
                // XXX - use only one loop
                for (int i = 0; i < 4; i++) {
                    result *= 100;
                    uint8_t temp = low & 0xFF;
                    result += temp - (6 * (temp >> 4));
                    low >>= 8;
                }
                for (int i = 0; i < 4; i++) {
                    result *= 100;
                    uint8_t temp = high & 0xFF;
                    result += temp - (6 * (temp >> 4));
                    high >>= 8;
                }
                for (int i = 0; i < 2; i++) {
                    result *= 100;
                    uint8_t temp = higher & 0xFF;
                    result += temp - (6 * (temp >> 4));
                    higher >>= 8;
                }
                temp80 = int64_to_floatx80((uint64_t)sign << (64LL - 16LL) | result);
                if (check_push())
                    FPU_ABORT();
                push(temp80);
                break;
            }
            case OP(0xDF, 5): { // FILD - Load floating point register.
                uint32_t hi, low;
                if (fwait())
                    return 1;
                update_pointers2(opcode, virtaddr, seg);

                cpu_read32(linaddr, low);
                cpu_read32(linaddr + 4, hi);
                temp80 = int64_to_floatx80((uint64_t)low | (uint64_t)hi << 32);
                if (check_push())
                    FPU_ABORT();
                push(temp80);
                break;
            }
            case OP(0xDF, 6): { // FBSTP - Store BCD to memory
                if (fwait())
                    return 1;
                update_pointers2(opcode, virtaddr, seg);
                floatx80 st0 = get_st(0);
                uint64_t bcd = floatx80_to_int64(st0, &status);

                if (check_exceptions())
                    FPU_ABORT();
                // Make sure we didn't cause exception

                for(int i=0;i<9;i++) {
                    int result = bcd % 10;
                    bcd /= 10;
                    result |= (bcd % 10) << 4;
                    bcd /= 10;
                    cpu_write8(linaddr + i, result);
                }

                int result = bcd % 10;
                bcd /= 10;
                result |= (bcd % 10) << 4;
                cpu_write8(linaddr + 9, result | (st0.exp >> 8 & 0x80));

                pop();
                break;
            }
            case OP(0xDF, 7): { // FISTP - Store floating point register to integer and pop
                if (fwait())
                    return 1;
                update_pointers2(opcode, virtaddr, seg);
                if (check_stack_underflow(0, 0))
                    FPU_ABORT();
                uint64_t i64 = floatx80_to_int64(get_st(0), &status);
                if (check_exceptions2(0))
                    FPU_ABORT();
                cpu_write32(linaddr, (uint32_t)i64);
                cpu_write32(linaddr + 4, (uint32_t)(i64 >> 32));
                commit_sw();
                pop();
                break;
            }
            case OP(0xD9, 1):
            case OP(0xDB, 4):
            case OP(0xDB, 6):
            case OP(0xDD, 5): {
                int major_opcode = opcode >> 8 | 0xD8;
                (void)major_opcode;
                //CPU_LOG("Unknown FPU memory operation: %02x %02x [%02x /%d] internal=%d\n", major_opcode, opcode & 0xFF, major_opcode, opcode >> 3 & 7, (opcode >> 5 & 0x38) | (opcode >> 3 & 7));
                break;
            }
            default: {
                int major_opcode = opcode >> 8 | 0xD8;
                //CPU_FATAL("Unknown FPU memory operation: %02x %02x [%02x /%d] internal=%d\n", major_opcode, opcode & 0xFF, major_opcode, opcode >> 3 & 7, (opcode >> 5 & 0x38) | (opcode >> 3 & 7));
            }
        }
        watchpoint2();
        return 0;
    }

#undef FPU_EXCEP
#undef FPU_ABORT
#undef OP

    template<typename C>
    int fpu<C>::fwait(void)
    {
        // Now is as good of a time as any to call FPU exceptions
        if (status_word & 0x80) {
            //if (thread_cpu.cr[0] & CR0_NE)
            //    EXCEPTION_MF();
            //else
            {
                // yucky, but works. OS/2 uses this method
                //pic_lower_irq(13);
                //pic_raise_irq(13);
                cpu_fp_exception();
            }
        }
        return 0;
    }

// Utilities for debugging

    static double f80_to_double(floatx80 *f80) {
        double scale_factor = std::pow(2.0, -63.0);
        uint16_t exponent;
        uint64_t fraction;
        floatx80_unpack(f80, exponent, fraction);

        double f = std::pow(2.0, ((exponent & 0x7FFF) - 0x3FFF));
        if (exponent > 0x8000)
            f = -f;
        f *= fraction * scale_factor;
        return f;
    }

    template<typename C>
    void fpu<C>::fpu_debug(void) {
        std::fprintf(stderr, " === FPU Context Dump ===\n");
        std::fprintf(stderr, "FPU CS:EIP: %04x:%08x Data Pointer: %04x:%08x\n", fpu_cs, fpu_eip, fpu_data_seg, fpu_data_ptr);
        int opcode = fpu_opcode >> 8 | 0xD8;
        std::fprintf(stderr, "Last FPU opcode: %04x [%02x %02x | %02x /%d]\n", fpu_opcode, opcode, fpu_opcode & 0xFF, opcode,
                     fpu_opcode >> 3 & 7);
        std::fprintf(stderr, "Status: %04x (top: %d) Control: %04x Tag: %04x\n", status_word, ftop, control_word, tag_word);
        for (int i = 0; i < 8; i++) {
            int real_index = (i + ftop) & 7;
            floatx80 val = st[real_index];
            double f = f80_to_double(&val);

            uint16_t exponent;
            uint64_t fraction;
            floatx80_unpack(&val, exponent, fraction);

            uint32_t high = fraction >> 32;
            std::fprintf(stderr, "ST%d(%c) [FP%d]: %04x %08x%08x (%.10f)\n", i, "v0se"[get_tag(i)], real_index, exponent,
                         high, (uint32_t) fraction, f);
        }
    }

    static void printFloat80(floatx80 *arg) {
        uint16_t exponent;
        uint64_t fraction;
        floatx80_unpack(arg, exponent, fraction);
        std::printf("%04x %08x%08x\n", exponent, (uint32_t) (fraction >> 32), (uint32_t) fraction);
    }

    template<typename C>
    void *fpu<C>::fpu_get_st_ptr1(void) {
        return &st[0];
    }
}


//extern fpu fpu;
//#define fpu (thread_cpu.fpu)
#endif