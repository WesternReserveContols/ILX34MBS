# 1 "1734-stm32f072/Src/DSC/circular_buffer.c"
# 1 "/home/wrc/Atollic/TrueSTUDIO/STM32_workspace_9.3/wrc-1734-port///"
# 1 "<built-in>"
#define __STDC__ 1
#define __STDC_VERSION__ 201112L
#define __STDC_UTF_16__ 1
#define __STDC_UTF_32__ 1
#define __STDC_HOSTED__ 1
#define __GNUC__ 6
#define __GNUC_MINOR__ 3
#define __GNUC_PATCHLEVEL__ 1
#define __VERSION__ "6.3.1 20170215 (release) [ARM/embedded-6-branch revision 245512]"
#define __ATOMIC_RELAXED 0
#define __ATOMIC_SEQ_CST 5
#define __ATOMIC_ACQUIRE 2
#define __ATOMIC_RELEASE 3
#define __ATOMIC_ACQ_REL 4
#define __ATOMIC_CONSUME 1
#define __OPTIMIZE__ 1
#define __FINITE_MATH_ONLY__ 0
#define __SIZEOF_INT__ 4
#define __SIZEOF_LONG__ 4
#define __SIZEOF_LONG_LONG__ 8
#define __SIZEOF_SHORT__ 2
#define __SIZEOF_FLOAT__ 4
#define __SIZEOF_DOUBLE__ 8
#define __SIZEOF_LONG_DOUBLE__ 8
#define __SIZEOF_SIZE_T__ 4
#define __CHAR_BIT__ 8
#define __BIGGEST_ALIGNMENT__ 8
#define __ORDER_LITTLE_ENDIAN__ 1234
#define __ORDER_BIG_ENDIAN__ 4321
#define __ORDER_PDP_ENDIAN__ 3412
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#define __FLOAT_WORD_ORDER__ __ORDER_LITTLE_ENDIAN__
#define __SIZEOF_POINTER__ 4
#define __SIZE_TYPE__ unsigned int
#define __PTRDIFF_TYPE__ int
#define __WCHAR_TYPE__ unsigned int
#define __WINT_TYPE__ unsigned int
#define __INTMAX_TYPE__ long long int
#define __UINTMAX_TYPE__ long long unsigned int
#define __CHAR16_TYPE__ short unsigned int
#define __CHAR32_TYPE__ long unsigned int
#define __SIG_ATOMIC_TYPE__ int
#define __INT8_TYPE__ signed char
#define __INT16_TYPE__ short int
#define __INT32_TYPE__ long int
#define __INT64_TYPE__ long long int
#define __UINT8_TYPE__ unsigned char
#define __UINT16_TYPE__ short unsigned int
#define __UINT32_TYPE__ long unsigned int
#define __UINT64_TYPE__ long long unsigned int
#define __INT_LEAST8_TYPE__ signed char
#define __INT_LEAST16_TYPE__ short int
#define __INT_LEAST32_TYPE__ long int
#define __INT_LEAST64_TYPE__ long long int
#define __UINT_LEAST8_TYPE__ unsigned char
#define __UINT_LEAST16_TYPE__ short unsigned int
#define __UINT_LEAST32_TYPE__ long unsigned int
#define __UINT_LEAST64_TYPE__ long long unsigned int
#define __INT_FAST8_TYPE__ int
#define __INT_FAST16_TYPE__ int
#define __INT_FAST32_TYPE__ int
#define __INT_FAST64_TYPE__ long long int
#define __UINT_FAST8_TYPE__ unsigned int
#define __UINT_FAST16_TYPE__ unsigned int
#define __UINT_FAST32_TYPE__ unsigned int
#define __UINT_FAST64_TYPE__ long long unsigned int
#define __INTPTR_TYPE__ int
#define __UINTPTR_TYPE__ unsigned int
#define __has_include(STR) __has_include__(STR)
#define __has_include_next(STR) __has_include_next__(STR)
#define __GXX_ABI_VERSION 1010
#define __SCHAR_MAX__ 0x7f
#define __SHRT_MAX__ 0x7fff
#define __INT_MAX__ 0x7fffffff
#define __LONG_MAX__ 0x7fffffffL
#define __LONG_LONG_MAX__ 0x7fffffffffffffffLL
#define __WCHAR_MAX__ 0xffffffffU
#define __WCHAR_MIN__ 0U
#define __WINT_MAX__ 0xffffffffU
#define __WINT_MIN__ 0U
#define __PTRDIFF_MAX__ 0x7fffffff
#define __SIZE_MAX__ 0xffffffffU
#define __INTMAX_MAX__ 0x7fffffffffffffffLL
#define __INTMAX_C(c) c ## LL
#define __UINTMAX_MAX__ 0xffffffffffffffffULL
#define __UINTMAX_C(c) c ## ULL
#define __SIG_ATOMIC_MAX__ 0x7fffffff
#define __SIG_ATOMIC_MIN__ (-__SIG_ATOMIC_MAX__ - 1)
#define __INT8_MAX__ 0x7f
#define __INT16_MAX__ 0x7fff
#define __INT32_MAX__ 0x7fffffffL
#define __INT64_MAX__ 0x7fffffffffffffffLL
#define __UINT8_MAX__ 0xff
#define __UINT16_MAX__ 0xffff
#define __UINT32_MAX__ 0xffffffffUL
#define __UINT64_MAX__ 0xffffffffffffffffULL
#define __INT_LEAST8_MAX__ 0x7f
#define __INT8_C(c) c
#define __INT_LEAST16_MAX__ 0x7fff
#define __INT16_C(c) c
#define __INT_LEAST32_MAX__ 0x7fffffffL
#define __INT32_C(c) c ## L
#define __INT_LEAST64_MAX__ 0x7fffffffffffffffLL
#define __INT64_C(c) c ## LL
#define __UINT_LEAST8_MAX__ 0xff
#define __UINT8_C(c) c
#define __UINT_LEAST16_MAX__ 0xffff
#define __UINT16_C(c) c
#define __UINT_LEAST32_MAX__ 0xffffffffUL
#define __UINT32_C(c) c ## UL
#define __UINT_LEAST64_MAX__ 0xffffffffffffffffULL
#define __UINT64_C(c) c ## ULL
#define __INT_FAST8_MAX__ 0x7fffffff
#define __INT_FAST16_MAX__ 0x7fffffff
#define __INT_FAST32_MAX__ 0x7fffffff
#define __INT_FAST64_MAX__ 0x7fffffffffffffffLL
#define __UINT_FAST8_MAX__ 0xffffffffU
#define __UINT_FAST16_MAX__ 0xffffffffU
#define __UINT_FAST32_MAX__ 0xffffffffU
#define __UINT_FAST64_MAX__ 0xffffffffffffffffULL
#define __INTPTR_MAX__ 0x7fffffff
#define __UINTPTR_MAX__ 0xffffffffU
#define __GCC_IEC_559 0
#define __GCC_IEC_559_COMPLEX 0
#define __FLT_EVAL_METHOD__ 0
#define __DEC_EVAL_METHOD__ 2
#define __FLT_RADIX__ 2
#define __FLT_MANT_DIG__ 24
#define __FLT_DIG__ 6
#define __FLT_MIN_EXP__ (-125)
#define __FLT_MIN_10_EXP__ (-37)
#define __FLT_MAX_EXP__ 128
#define __FLT_MAX_10_EXP__ 38
#define __FLT_DECIMAL_DIG__ 9
#define __FLT_MAX__ 3.4028234663852886e+38F
#define __FLT_MIN__ 1.1754943508222875e-38F
#define __FLT_EPSILON__ 1.1920928955078125e-7F
#define __FLT_DENORM_MIN__ 1.4012984643248171e-45F
#define __FLT_HAS_DENORM__ 1
#define __FLT_HAS_INFINITY__ 1
#define __FLT_HAS_QUIET_NAN__ 1
#define __DBL_MANT_DIG__ 53
#define __DBL_DIG__ 15
#define __DBL_MIN_EXP__ (-1021)
#define __DBL_MIN_10_EXP__ (-307)
#define __DBL_MAX_EXP__ 1024
#define __DBL_MAX_10_EXP__ 308
#define __DBL_DECIMAL_DIG__ 17
#define __DBL_MAX__ ((double)1.7976931348623157e+308L)
#define __DBL_MIN__ ((double)2.2250738585072014e-308L)
#define __DBL_EPSILON__ ((double)2.2204460492503131e-16L)
#define __DBL_DENORM_MIN__ ((double)4.9406564584124654e-324L)
#define __DBL_HAS_DENORM__ 1
#define __DBL_HAS_INFINITY__ 1
#define __DBL_HAS_QUIET_NAN__ 1
#define __LDBL_MANT_DIG__ 53
#define __LDBL_DIG__ 15
#define __LDBL_MIN_EXP__ (-1021)
#define __LDBL_MIN_10_EXP__ (-307)
#define __LDBL_MAX_EXP__ 1024
#define __LDBL_MAX_10_EXP__ 308
#define __DECIMAL_DIG__ 17
#define __LDBL_MAX__ 1.7976931348623157e+308L
#define __LDBL_MIN__ 2.2250738585072014e-308L
#define __LDBL_EPSILON__ 2.2204460492503131e-16L
#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
#define __LDBL_HAS_DENORM__ 1
#define __LDBL_HAS_INFINITY__ 1
#define __LDBL_HAS_QUIET_NAN__ 1
#define __DEC32_MANT_DIG__ 7
#define __DEC32_MIN_EXP__ (-94)
#define __DEC32_MAX_EXP__ 97
#define __DEC32_MIN__ 1E-95DF
#define __DEC32_MAX__ 9.999999E96DF
#define __DEC32_EPSILON__ 1E-6DF
#define __DEC32_SUBNORMAL_MIN__ 0.000001E-95DF
#define __DEC64_MANT_DIG__ 16
#define __DEC64_MIN_EXP__ (-382)
#define __DEC64_MAX_EXP__ 385
#define __DEC64_MIN__ 1E-383DD
#define __DEC64_MAX__ 9.999999999999999E384DD
#define __DEC64_EPSILON__ 1E-15DD
#define __DEC64_SUBNORMAL_MIN__ 0.000000000000001E-383DD
#define __DEC128_MANT_DIG__ 34
#define __DEC128_MIN_EXP__ (-6142)
#define __DEC128_MAX_EXP__ 6145
#define __DEC128_MIN__ 1E-6143DL
#define __DEC128_MAX__ 9.999999999999999999999999999999999E6144DL
#define __DEC128_EPSILON__ 1E-33DL
#define __DEC128_SUBNORMAL_MIN__ 0.000000000000000000000000000000001E-6143DL
#define __SFRACT_FBIT__ 7
#define __SFRACT_IBIT__ 0
#define __SFRACT_MIN__ (-0.5HR-0.5HR)
#define __SFRACT_MAX__ 0X7FP-7HR
#define __SFRACT_EPSILON__ 0x1P-7HR
#define __USFRACT_FBIT__ 8
#define __USFRACT_IBIT__ 0
#define __USFRACT_MIN__ 0.0UHR
#define __USFRACT_MAX__ 0XFFP-8UHR
#define __USFRACT_EPSILON__ 0x1P-8UHR
#define __FRACT_FBIT__ 15
#define __FRACT_IBIT__ 0
#define __FRACT_MIN__ (-0.5R-0.5R)
#define __FRACT_MAX__ 0X7FFFP-15R
#define __FRACT_EPSILON__ 0x1P-15R
#define __UFRACT_FBIT__ 16
#define __UFRACT_IBIT__ 0
#define __UFRACT_MIN__ 0.0UR
#define __UFRACT_MAX__ 0XFFFFP-16UR
#define __UFRACT_EPSILON__ 0x1P-16UR
#define __LFRACT_FBIT__ 31
#define __LFRACT_IBIT__ 0
#define __LFRACT_MIN__ (-0.5LR-0.5LR)
#define __LFRACT_MAX__ 0X7FFFFFFFP-31LR
#define __LFRACT_EPSILON__ 0x1P-31LR
#define __ULFRACT_FBIT__ 32
#define __ULFRACT_IBIT__ 0
#define __ULFRACT_MIN__ 0.0ULR
#define __ULFRACT_MAX__ 0XFFFFFFFFP-32ULR
#define __ULFRACT_EPSILON__ 0x1P-32ULR
#define __LLFRACT_FBIT__ 63
#define __LLFRACT_IBIT__ 0
#define __LLFRACT_MIN__ (-0.5LLR-0.5LLR)
#define __LLFRACT_MAX__ 0X7FFFFFFFFFFFFFFFP-63LLR
#define __LLFRACT_EPSILON__ 0x1P-63LLR
#define __ULLFRACT_FBIT__ 64
#define __ULLFRACT_IBIT__ 0
#define __ULLFRACT_MIN__ 0.0ULLR
#define __ULLFRACT_MAX__ 0XFFFFFFFFFFFFFFFFP-64ULLR
#define __ULLFRACT_EPSILON__ 0x1P-64ULLR
#define __SACCUM_FBIT__ 7
#define __SACCUM_IBIT__ 8
#define __SACCUM_MIN__ (-0X1P7HK-0X1P7HK)
#define __SACCUM_MAX__ 0X7FFFP-7HK
#define __SACCUM_EPSILON__ 0x1P-7HK
#define __USACCUM_FBIT__ 8
#define __USACCUM_IBIT__ 8
#define __USACCUM_MIN__ 0.0UHK
#define __USACCUM_MAX__ 0XFFFFP-8UHK
#define __USACCUM_EPSILON__ 0x1P-8UHK
#define __ACCUM_FBIT__ 15
#define __ACCUM_IBIT__ 16
#define __ACCUM_MIN__ (-0X1P15K-0X1P15K)
#define __ACCUM_MAX__ 0X7FFFFFFFP-15K
#define __ACCUM_EPSILON__ 0x1P-15K
#define __UACCUM_FBIT__ 16
#define __UACCUM_IBIT__ 16
#define __UACCUM_MIN__ 0.0UK
#define __UACCUM_MAX__ 0XFFFFFFFFP-16UK
#define __UACCUM_EPSILON__ 0x1P-16UK
#define __LACCUM_FBIT__ 31
#define __LACCUM_IBIT__ 32
#define __LACCUM_MIN__ (-0X1P31LK-0X1P31LK)
#define __LACCUM_MAX__ 0X7FFFFFFFFFFFFFFFP-31LK
#define __LACCUM_EPSILON__ 0x1P-31LK
#define __ULACCUM_FBIT__ 32
#define __ULACCUM_IBIT__ 32
#define __ULACCUM_MIN__ 0.0ULK
#define __ULACCUM_MAX__ 0XFFFFFFFFFFFFFFFFP-32ULK
#define __ULACCUM_EPSILON__ 0x1P-32ULK
#define __LLACCUM_FBIT__ 31
#define __LLACCUM_IBIT__ 32
#define __LLACCUM_MIN__ (-0X1P31LLK-0X1P31LLK)
#define __LLACCUM_MAX__ 0X7FFFFFFFFFFFFFFFP-31LLK
#define __LLACCUM_EPSILON__ 0x1P-31LLK
#define __ULLACCUM_FBIT__ 32
#define __ULLACCUM_IBIT__ 32
#define __ULLACCUM_MIN__ 0.0ULLK
#define __ULLACCUM_MAX__ 0XFFFFFFFFFFFFFFFFP-32ULLK
#define __ULLACCUM_EPSILON__ 0x1P-32ULLK
#define __QQ_FBIT__ 7
#define __QQ_IBIT__ 0
#define __HQ_FBIT__ 15
#define __HQ_IBIT__ 0
#define __SQ_FBIT__ 31
#define __SQ_IBIT__ 0
#define __DQ_FBIT__ 63
#define __DQ_IBIT__ 0
#define __TQ_FBIT__ 127
#define __TQ_IBIT__ 0
#define __UQQ_FBIT__ 8
#define __UQQ_IBIT__ 0
#define __UHQ_FBIT__ 16
#define __UHQ_IBIT__ 0
#define __USQ_FBIT__ 32
#define __USQ_IBIT__ 0
#define __UDQ_FBIT__ 64
#define __UDQ_IBIT__ 0
#define __UTQ_FBIT__ 128
#define __UTQ_IBIT__ 0
#define __HA_FBIT__ 7
#define __HA_IBIT__ 8
#define __SA_FBIT__ 15
#define __SA_IBIT__ 16
#define __DA_FBIT__ 31
#define __DA_IBIT__ 32
#define __TA_FBIT__ 63
#define __TA_IBIT__ 64
#define __UHA_FBIT__ 8
#define __UHA_IBIT__ 8
#define __USA_FBIT__ 16
#define __USA_IBIT__ 16
#define __UDA_FBIT__ 32
#define __UDA_IBIT__ 32
#define __UTA_FBIT__ 64
#define __UTA_IBIT__ 64
#define __REGISTER_PREFIX__ 
#define __USER_LABEL_PREFIX__ 
#define __GNUC_STDC_INLINE__ 1
#define __STRICT_ANSI__ 1
#define __CHAR_UNSIGNED__ 1
#define __GCC_ATOMIC_BOOL_LOCK_FREE 1
#define __GCC_ATOMIC_CHAR_LOCK_FREE 1
#define __GCC_ATOMIC_CHAR16_T_LOCK_FREE 1
#define __GCC_ATOMIC_CHAR32_T_LOCK_FREE 1
#define __GCC_ATOMIC_WCHAR_T_LOCK_FREE 1
#define __GCC_ATOMIC_SHORT_LOCK_FREE 1
#define __GCC_ATOMIC_INT_LOCK_FREE 1
#define __GCC_ATOMIC_LONG_LOCK_FREE 1
#define __GCC_ATOMIC_LLONG_LOCK_FREE 1
#define __GCC_ATOMIC_TEST_AND_SET_TRUEVAL 1
#define __GCC_ATOMIC_POINTER_LOCK_FREE 1
#define __GCC_HAVE_DWARF2_CFI_ASM 1
#define __PRAGMA_REDEFINE_EXTNAME 1
#define __SIZEOF_WCHAR_T__ 4
#define __SIZEOF_WINT_T__ 4
#define __SIZEOF_PTRDIFF_T__ 4
#undef __ARM_FEATURE_DSP
# 1 "<built-in>"
#undef __ARM_FEATURE_QBIT
# 1 "<built-in>"
#undef __ARM_FEATURE_SAT
# 1 "<built-in>"
#undef __ARM_FEATURE_CRYPTO
# 1 "<built-in>"
#undef __ARM_FEATURE_UNALIGNED
# 1 "<built-in>"
#undef __ARM_FEATURE_QRDMX
# 1 "<built-in>"
#undef __ARM_32BIT_STATE
# 1 "<built-in>"
#undef __ARM_FEATURE_LDREX
# 1 "<built-in>"
#undef __ARM_FEATURE_CLZ
# 1 "<built-in>"
#undef __ARM_FEATURE_SIMD32
# 1 "<built-in>"
#define __ARM_SIZEOF_MINIMAL_ENUM 1
#define __ARM_SIZEOF_WCHAR_T 4
#define __ARM_ARCH_PROFILE 77
#define __arm__ 1
#define __ARM_ARCH 6
#define __APCS_32__ 1
#define __thumb__ 1
#undef __thumb2__
# 1 "<built-in>"
#define __THUMBEL__ 1
#define __ARM_ARCH_ISA_THUMB 1
#define __ARMEL__ 1
#define __SOFTFP__ 1
#define __VFP_FP__ 1
#undef __ARM_FP
# 1 "<built-in>"
#undef __ARM_FEATURE_FMA
# 1 "<built-in>"
#undef __ARM_NEON__
# 1 "<built-in>"
#undef __ARM_NEON
# 1 "<built-in>"
#undef __ARM_NEON_FP
# 1 "<built-in>"
#define __THUMB_INTERWORK__ 1
#define __ARM_ARCH_6M__ 1
#define __ARM_PCS 1
#define __ARM_EABI__ 1
#undef __ARM_ARCH_EXT_IDIV__
# 1 "<built-in>"
#undef __ARM_FEATURE_IDIV
# 1 "<built-in>"
#undef __ARM_ASM_SYNTAX_UNIFIED__
# 1 "<built-in>"
#define __GXX_TYPEINFO_EQUALITY_INLINE 0
#define __ELF__ 1
# 1 "<command-line>"
#define __USES_INITFINI__ 1
#define EXPL_CNXN_SIZE 140
#define FCL 140
#define FPL 140
#define EE_REV_MAJ 2
#define GMM 1
#define FREERTOS_BUILD 1
#define USE_HAL_DRIVER 1
#define STM32F072xB 1
#define __BUILDNUMBER__ 1602177725
# 1 "1734-stm32f072/Src/DSC/circular_buffer.c"



# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 1 3







#define _STDLIB_H_ 

# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/ieeefp.h" 1 3
# 60 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/ieeefp.h" 3
#define __IEEE_LITTLE_ENDIAN 
# 11 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 2 3
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/_ansi.h" 1 3
# 13 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/_ansi.h" 3
#define _ANSIDECL_H_ 

# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/newlib.h" 1 3







#define __NEWLIB_H__ 1





# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/_newlib_version.h" 1 3



#define _NEWLIB_VERSION_H__ 1

#define _NEWLIB_VERSION "2.5.0"
#define __NEWLIB__ 2
#define __NEWLIB_MINOR__ 5
#define __NEWLIB_PATCHLEVEL__ 0
# 15 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/newlib.h" 2 3






#define _WANT_IO_LONG_LONG 1
# 34 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/newlib.h" 3
#define _WANT_REENT_SMALL 1





#define _MB_LEN_MAX 1
# 50 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/newlib.h" 3
#define HAVE_INITFINI_ARRAY 1



#define _ATEXIT_DYNAMIC_ALLOC 1


#define _HAVE_LONG_DOUBLE 1


#define _HAVE_CC_INHIBIT_LOOP_TO_LIBCALL 1


#define _LDBL_EQ_DBL 1


#define _FVWRITE_IN_STREAMIO 1


#define _FSEEK_OPTIMIZATION 1


#define _WIDE_ORIENT 1


#define _UNBUF_STREAM_OPT 1
# 87 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/newlib.h" 3
#define _RETARGETABLE_LOCKING 1
# 16 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/_ansi.h" 2 3
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/config.h" 1 3

#define __SYS_CONFIG_H__ 

# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/ieeefp.h" 1 3
# 5 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/config.h" 2 3
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/features.h" 1 3
# 22 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/features.h" 3
#define _SYS_FEATURES_H 
# 33 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/features.h" 3
#define __GNUC_PREREQ(maj,min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))






#define __GNUC_PREREQ__(ma,mi) __GNUC_PREREQ(ma, mi)
# 244 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/features.h" 3
#define __ATFILE_VISIBLE 0





#define __BSD_VISIBLE 0





#define __GNU_VISIBLE 0




#define __ISO_C_VISIBLE 2011
# 272 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/features.h" 3
#define __LARGEFILE_VISIBLE 0





#define __MISC_VISIBLE 0
# 294 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/features.h" 3
#define __POSIX_VISIBLE 0





#define __SVID_VISIBLE 0
# 314 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/features.h" 3
#define __XSI_VISIBLE 0
# 6 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/config.h" 2 3
# 219 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/config.h" 3
#define _POINTER_INT long





#undef __RAND_MAX



#define __RAND_MAX 0x7fffffff
# 244 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/config.h" 3
#define __EXPORT 



#define __IMPORT 






#define _READ_WRITE_RETURN_TYPE int





#define _READ_WRITE_BUFSIZE_TYPE int
# 274 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/config.h" 3
#define _REENT_SMALL 
# 17 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/_ansi.h" 2 3






#define _HAVE_STDC 
# 44 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/_ansi.h" 3
#define _BEGIN_STD_C 
#define _END_STD_C 
#define _NOTHROW 



#define _PTR void *
#define _AND ,
#define _NOARGS void
#define _CONST const
#define _VOLATILE volatile
#define _SIGNED signed
#define _DOTS , ...
#define _VOID void






#define _EXFUN_NOTHROW(name,proto) name proto _NOTHROW
#define _EXFUN(name,proto) name proto
#define _EXPARM(name,proto) (* name) proto
#define _EXFNPTR(name,proto) (* name) proto

#define _DEFUN(name,arglist,args) name(args)
#define _DEFUN_VOID(name) name(_NOARGS)
#define _CAST_VOID (void)

#define _LONG_DOUBLE long double


#define _PARAMS(paramlist) paramlist
# 101 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/_ansi.h" 3
#define _ATTRIBUTE(attrs) __attribute__ (attrs)
# 127 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/_ansi.h" 3
#define _ELIDABLE_INLINE static __inline__



#define _NOINLINE __attribute__ ((__noinline__))
#define _NOINLINE_STATIC _NOINLINE static
# 12 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 2 3

#define __need_size_t 
#define __need_wchar_t 
#define __need_NULL 
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 1 3 4
# 187 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#define __size_t__ 
#define __SIZE_T__ 
#define _SIZE_T 
#define _SYS_SIZE_T_H 
#define _T_SIZE_ 
#define _T_SIZE 
#define __SIZE_T 
#define _SIZE_T_ 
#define _BSD_SIZE_T_ 
#define _SIZE_T_DEFINED_ 
#define _SIZE_T_DEFINED 
#define _BSD_SIZE_T_DEFINED_ 
#define _SIZE_T_DECLARED 
#define ___int_size_t_h 
#define _GCC_SIZE_T 
#define _SIZET_ 







#define __size_t 






# 216 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
typedef unsigned int size_t;
# 238 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef __need_size_t
# 267 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#define __wchar_t__ 
#define __WCHAR_T__ 
#define _WCHAR_T 
#define _T_WCHAR_ 
#define _T_WCHAR 
#define __WCHAR_T 
#define _WCHAR_T_ 
#define _BSD_WCHAR_T_ 
#define _WCHAR_T_DEFINED_ 
#define _WCHAR_T_DEFINED 
#define _WCHAR_T_H 
#define ___int_wchar_t_h 
#define __INT_WCHAR_T_H 
#define _GCC_WCHAR_T 
#define _WCHAR_T_DECLARED 
# 294 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef _BSD_WCHAR_T_
# 328 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
typedef unsigned int wchar_t;
# 347 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef __need_wchar_t
# 401 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef NULL




#define NULL ((void *)0)





#undef __need_NULL
# 17 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 2 3

# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 1 3
# 11 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
#define _SYS_REENT_H_ 

# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/_ansi.h" 1 3
# 14 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 2 3
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 1 3 4
# 39 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#define _STDDEF_H 
#define _STDDEF_H_ 

#define _ANSI_STDDEF_H 
# 137 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#define _PTRDIFF_T 
#define _T_PTRDIFF_ 
#define _T_PTRDIFF 
#define __PTRDIFF_T 
#define _PTRDIFF_T_ 
#define _BSD_PTRDIFF_T_ 
#define ___int_ptrdiff_t_h 
#define _GCC_PTRDIFF_T 
#define _PTRDIFF_T_DECLARED 



typedef int ptrdiff_t;
# 161 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef __need_ptrdiff_t
# 238 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef __need_size_t
# 347 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef __need_wchar_t
# 401 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef NULL




#define NULL ((void *)0)





#undef __need_NULL




#define offsetof(TYPE,MEMBER) __builtin_offsetof (TYPE, MEMBER)




#define _GCC_MAX_ALIGN_T 



typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
} max_align_t;
# 15 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 2 3
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_types.h" 1 3
# 20 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_types.h" 3
#define _SYS__TYPES_H 



# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_types.h" 1 3





#define _MACHINE__TYPES_H 
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_default_types.h" 1 3





#define _MACHINE__DEFAULT_TYPES_H 
# 15 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_default_types.h" 3
#define __EXP(x) __ ##x ##__
# 26 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_default_types.h" 3
#define __have_longlong64 1






#define __have_long32 1







typedef signed char __int8_t;

typedef unsigned char __uint8_t;



#define ___int8_t_defined 1







typedef short int __int16_t;

typedef short unsigned int __uint16_t;



#define ___int16_t_defined 1
# 77 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_default_types.h" 3
typedef long int __int32_t;

typedef long unsigned int __uint32_t;



#define ___int32_t_defined 1
# 103 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_default_types.h" 3
typedef long long int __int64_t;

typedef long long unsigned int __uint64_t;



#define ___int64_t_defined 1
# 134 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_default_types.h" 3
typedef signed char __int_least8_t;

typedef unsigned char __uint_least8_t;



#define ___int_least8_t_defined 1
# 160 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_default_types.h" 3
typedef short int __int_least16_t;

typedef short unsigned int __uint_least16_t;



#define ___int_least16_t_defined 1
# 182 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_default_types.h" 3
typedef long int __int_least32_t;

typedef long unsigned int __uint_least32_t;



#define ___int_least32_t_defined 1
# 200 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_default_types.h" 3
typedef long long int __int_least64_t;

typedef long long unsigned int __uint_least64_t;



#define ___int_least64_t_defined 1







typedef long long int __intmax_t;







typedef long long unsigned int __uintmax_t;







typedef int __intptr_t;

typedef unsigned int __uintptr_t;
# 244 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_default_types.h" 3
#undef __EXP
# 8 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/_types.h" 2 3
# 25 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_types.h" 2 3
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/lock.h" 1 3

#define __SYS_LOCK_H__ 
# 33 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/lock.h" 3
struct __lock;
typedef struct __lock * _LOCK_T;
#define _LOCK_RECURSIVE_T _LOCK_T

#define __LOCK_INIT(class,lock) extern struct __lock __lock_ ## lock; class _LOCK_T lock = &__lock_ ## lock

#define __LOCK_INIT_RECURSIVE(class,lock) __LOCK_INIT(class,lock)

extern void __retarget_lock_init(_LOCK_T *lock);
#define __lock_init(lock) __retarget_lock_init(&lock)
extern void __retarget_lock_init_recursive(_LOCK_T *lock);
#define __lock_init_recursive(lock) __retarget_lock_init_recursive(&lock)
extern void __retarget_lock_close(_LOCK_T lock);
#define __lock_close(lock) __retarget_lock_close(lock)
extern void __retarget_lock_close_recursive(_LOCK_T lock);
#define __lock_close_recursive(lock) __retarget_lock_close_recursive(lock)
extern void __retarget_lock_acquire(_LOCK_T lock);
#define __lock_acquire(lock) __retarget_lock_acquire(lock)
extern void __retarget_lock_acquire_recursive(_LOCK_T lock);
#define __lock_acquire_recursive(lock) __retarget_lock_acquire_recursive(lock)
extern int __retarget_lock_try_acquire(_LOCK_T lock);
#define __lock_try_acquire(lock) __retarget_lock_try_acquire(lock)
extern int __retarget_lock_try_acquire_recursive(_LOCK_T lock);
#define __lock_try_acquire_recursive(lock) __retarget_lock_try_acquire_recursive(lock)

extern void __retarget_lock_release(_LOCK_T lock);
#define __lock_release(lock) __retarget_lock_release(lock)
extern void __retarget_lock_release_recursive(_LOCK_T lock);
#define __lock_release_recursive(lock) __retarget_lock_release_recursive(lock)
# 26 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_types.h" 2 3


typedef long __blkcnt_t;



typedef long __blksize_t;



typedef __uint64_t __fsblkcnt_t;



typedef __uint32_t __fsfilcnt_t;



typedef long _off_t;





typedef int __pid_t;



typedef short __dev_t;



typedef unsigned short __uid_t;


typedef unsigned short __gid_t;



typedef __uint32_t __id_t;







typedef unsigned short __ino_t;
# 88 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_types.h" 3
typedef __uint32_t __mode_t;





__extension__ typedef long long _off64_t;





typedef _off_t __off_t;


typedef _off64_t __loff_t;


typedef long __key_t;







typedef long _fpos_t;
# 125 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_types.h" 3
#undef __size_t



typedef unsigned int __size_t;
# 144 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_types.h" 3
#define unsigned signed
typedef signed int _ssize_t;
#undef unsigned
# 156 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_types.h" 3
typedef _ssize_t __ssize_t;

#define __need_wint_t 
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 1 3 4
# 161 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef __need_ptrdiff_t
# 238 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef __need_size_t
# 347 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef __need_wchar_t




#define _WINT_T 




typedef unsigned int wint_t;

#undef __need_wint_t
# 401 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 3 4
#undef NULL




#define NULL ((void *)0)





#undef __need_NULL




#define offsetof(TYPE,MEMBER) __builtin_offsetof (TYPE, MEMBER)
# 160 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_types.h" 2 3



typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    unsigned char __wchb[4];
  } __value;
} _mbstate_t;



typedef _LOCK_T _flock_t;




typedef void *_iconv_t;


#define _CLOCK_T_ unsigned long
typedef unsigned long __clock_t;

#define _TIME_T_ long
typedef long __time_t;

#define _CLOCKID_T_ unsigned long
typedef unsigned long __clockid_t;

#define _TIMER_T_ unsigned long
typedef unsigned long __timer_t;


typedef __uint8_t __sa_family_t;



typedef __uint32_t __socklen_t;


typedef unsigned short __nlink_t;
typedef long __suseconds_t;
typedef unsigned long __useconds_t;




typedef char * __va_list;
# 16 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 2 3

#define _NULL 0



#define __Long long
typedef unsigned long __ULong;
# 38 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
struct _reent;

struct __locale_t;






struct _Bigint
{
  struct _Bigint *_next;
  int _k, _maxwds, _sign, _wds;
  __ULong _x[1];
};


struct __tm
{
  int __tm_sec;
  int __tm_min;
  int __tm_hour;
  int __tm_mday;
  int __tm_mon;
  int __tm_year;
  int __tm_wday;
  int __tm_yday;
  int __tm_isdst;
};





#define _ATEXIT_SIZE 32

struct _on_exit_args {
 void * _fnargs[32];
 void * _dso_handle[32];

 __ULong _fntypes;


 __ULong _is_cxa;
};


struct _atexit {
 struct _atexit *_next;
 int _ind;
 void (*_fns[32])(void);
        struct _on_exit_args * _on_exit_args_ptr;
};
#define _ATEXIT_INIT {_NULL, 0, {_NULL}, _NULL}
# 106 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
#define _REENT_INIT_ATEXIT _NULL, _ATEXIT_INIT,
# 117 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
struct __sbuf {
 unsigned char *_base;
 int _size;
};
# 153 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
struct __sFILE_fake {
  unsigned char *_p;
  int _r;
  int _w;
  short _flags;
  short _file;
  struct __sbuf _bf;
  int _lbfsize;

  struct _reent *_data;
};




extern void __sinit (struct _reent *);

#define _REENT_SMALL_CHECK_INIT(ptr) do { if ((ptr) && !(ptr)->__sdidinit) __sinit (ptr); } while (0)
# 181 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
struct __sFILE {
  unsigned char *_p;
  int _r;
  int _w;
  short _flags;
  short _file;
  struct __sbuf _bf;
  int _lbfsize;


  struct _reent *_data;



  void * _cookie;

  int (* _read) (struct _reent *, void *, char *, int)
                                          ;
  int (* _write) (struct _reent *, void *, const char *, int)

                                   ;
  _fpos_t (* _seek) (struct _reent *, void *, _fpos_t, int);
  int (* _close) (struct _reent *, void *);


  struct __sbuf _ub;
  unsigned char *_up;
  int _ur;


  unsigned char _ubuf[3];
  unsigned char _nbuf[1];


  struct __sbuf _lb;


  int _blksize;
  _off_t _offset;






  _flock_t _lock;

  _mbstate_t _mbstate;
  int _flags2;
};
# 287 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
typedef struct __sFILE __FILE;



struct _glue
{
  struct _glue *_next;
  int _niobs;
  __FILE *_iobs;
};
# 312 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
#define _RAND48_SEED_0 (0x330e)
#define _RAND48_SEED_1 (0xabcd)
#define _RAND48_SEED_2 (0x1234)
#define _RAND48_MULT_0 (0xe66d)
#define _RAND48_MULT_1 (0xdeec)
#define _RAND48_MULT_2 (0x0005)
#define _RAND48_ADD (0x000b)
struct _rand48 {
  unsigned short _seed[3];
  unsigned short _mult[3];
  unsigned short _add;


  __extension__ unsigned long long _rand_next;

};


#define _REENT_EMERGENCY_SIZE 25
#define _REENT_ASCTIME_SIZE 26
#define _REENT_SIGNAL_SIZE 24
# 344 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
struct _mprec
{

  struct _Bigint *_result;
  int _result_k;
  struct _Bigint *_p5s;
  struct _Bigint **_freelist;
};


struct _misc_reent
{

  char *_strtok_last;
  _mbstate_t _mblen_state;
  _mbstate_t _wctomb_state;
  _mbstate_t _mbtowc_state;
  char _l64a_buf[8];
  int _getdate_err;
  _mbstate_t _mbrlen_state;
  _mbstate_t _mbrtowc_state;
  _mbstate_t _mbsrtowcs_state;
  _mbstate_t _wcrtomb_state;
  _mbstate_t _wcsrtombs_state;
};



struct _reent
{


  int _errno;




  __FILE *_stdin, *_stdout, *_stderr;

  int _inc;

  char *_emergency;

  int __sdidinit;

  int _unspecified_locale_info;
  struct __locale_t *_locale;

  struct _mprec *_mp;

  void (* __cleanup) (struct _reent *);

  int _gamma_signgam;


  int _cvtlen;
  char *_cvtbuf;

  struct _rand48 *_r48;
  struct __tm *_localtime_buf;
  char *_asctime_buf;


  void (**(_sig_func))(int);



  struct _atexit *_atexit;
  struct _atexit _atexit0;


  struct _glue __sglue;
  __FILE *__sf;
  struct _misc_reent *_misc;
  char *_signal_buf;
};

extern const struct __sFILE_fake __sf_fake_stdin;
extern const struct __sFILE_fake __sf_fake_stdout;
extern const struct __sFILE_fake __sf_fake_stderr;

#define _REENT_INIT(var) { 0, (__FILE *)&__sf_fake_stdin, (__FILE *)&__sf_fake_stdout, (__FILE *)&__sf_fake_stderr, 0, _NULL, 0, 0, _NULL, _NULL, _NULL, 0, 0, _NULL, _NULL, _NULL, _NULL, _NULL, _REENT_INIT_ATEXIT {_NULL, 0, _NULL}, _NULL, _NULL, _NULL }
# 451 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
#define _REENT_INIT_PTR_ZEROED(var) { (var)->_stdin = (__FILE *)&__sf_fake_stdin; (var)->_stdout = (__FILE *)&__sf_fake_stdout; (var)->_stderr = (__FILE *)&__sf_fake_stderr; }
# 462 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
#define __reent_assert(x) ((void)0)







#define _REENT_CHECK(var,what,type,size,init) do { struct _reent *_r = (var); if (_r->what == NULL) { _r->what = (type)malloc(size); __reent_assert(_r->what); init; } } while (0)
# 479 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
#define _REENT_CHECK_TM(var) _REENT_CHECK(var, _localtime_buf, struct __tm *, sizeof *((var)->_localtime_buf), )



#define _REENT_CHECK_ASCTIME_BUF(var) _REENT_CHECK(var, _asctime_buf, char *, _REENT_ASCTIME_SIZE, memset((var)->_asctime_buf, 0, _REENT_ASCTIME_SIZE))




#define _REENT_INIT_RAND48(var) do { struct _reent *_r = (var); _r->_r48->_seed[0] = _RAND48_SEED_0; _r->_r48->_seed[1] = _RAND48_SEED_1; _r->_r48->_seed[2] = _RAND48_SEED_2; _r->_r48->_mult[0] = _RAND48_MULT_0; _r->_r48->_mult[1] = _RAND48_MULT_1; _r->_r48->_mult[2] = _RAND48_MULT_2; _r->_r48->_add = _RAND48_ADD; _r->_r48->_rand_next = 1; } while (0)
# 499 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
#define _REENT_CHECK_RAND48(var) _REENT_CHECK(var, _r48, struct _rand48 *, sizeof *((var)->_r48), _REENT_INIT_RAND48((var)))


#define _REENT_INIT_MP(var) do { struct _reent *_r = (var); _r->_mp->_result_k = 0; _r->_mp->_result = _r->_mp->_p5s = _NULL; _r->_mp->_freelist = _NULL; } while (0)





#define _REENT_CHECK_MP(var) _REENT_CHECK(var, _mp, struct _mprec *, sizeof *((var)->_mp), _REENT_INIT_MP(var))


#define _REENT_CHECK_EMERGENCY(var) _REENT_CHECK(var, _emergency, char *, _REENT_EMERGENCY_SIZE, )


#define _REENT_INIT_MISC(var) do { struct _reent *_r = (var); _r->_misc->_strtok_last = _NULL; _r->_misc->_mblen_state.__count = 0; _r->_misc->_mblen_state.__value.__wch = 0; _r->_misc->_wctomb_state.__count = 0; _r->_misc->_wctomb_state.__value.__wch = 0; _r->_misc->_mbtowc_state.__count = 0; _r->_misc->_mbtowc_state.__value.__wch = 0; _r->_misc->_mbrlen_state.__count = 0; _r->_misc->_mbrlen_state.__value.__wch = 0; _r->_misc->_mbrtowc_state.__count = 0; _r->_misc->_mbrtowc_state.__value.__wch = 0; _r->_misc->_mbsrtowcs_state.__count = 0; _r->_misc->_mbsrtowcs_state.__value.__wch = 0; _r->_misc->_wcrtomb_state.__count = 0; _r->_misc->_wcrtomb_state.__value.__wch = 0; _r->_misc->_wcsrtombs_state.__count = 0; _r->_misc->_wcsrtombs_state.__value.__wch = 0; _r->_misc->_l64a_buf[0] = '\0'; _r->_misc->_getdate_err = 0; } while (0)
# 536 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
#define _REENT_CHECK_MISC(var) _REENT_CHECK(var, _misc, struct _misc_reent *, sizeof *((var)->_misc), _REENT_INIT_MISC(var))


#define _REENT_CHECK_SIGNAL_BUF(var) _REENT_CHECK(var, _signal_buf, char *, _REENT_SIGNAL_SIZE, )


#define _REENT_SIGNGAM(ptr) ((ptr)->_gamma_signgam)
#define _REENT_RAND_NEXT(ptr) ((ptr)->_r48->_rand_next)
#define _REENT_RAND48_SEED(ptr) ((ptr)->_r48->_seed)
#define _REENT_RAND48_MULT(ptr) ((ptr)->_r48->_mult)
#define _REENT_RAND48_ADD(ptr) ((ptr)->_r48->_add)
#define _REENT_MP_RESULT(ptr) ((ptr)->_mp->_result)
#define _REENT_MP_RESULT_K(ptr) ((ptr)->_mp->_result_k)
#define _REENT_MP_P5S(ptr) ((ptr)->_mp->_p5s)
#define _REENT_MP_FREELIST(ptr) ((ptr)->_mp->_freelist)
#define _REENT_ASCTIME_BUF(ptr) ((ptr)->_asctime_buf)
#define _REENT_TM(ptr) ((ptr)->_localtime_buf)
#define _REENT_EMERGENCY(ptr) ((ptr)->_emergency)
#define _REENT_STRTOK_LAST(ptr) ((ptr)->_misc->_strtok_last)
#define _REENT_MBLEN_STATE(ptr) ((ptr)->_misc->_mblen_state)
#define _REENT_MBTOWC_STATE(ptr) ((ptr)->_misc->_mbtowc_state)
#define _REENT_WCTOMB_STATE(ptr) ((ptr)->_misc->_wctomb_state)
#define _REENT_MBRLEN_STATE(ptr) ((ptr)->_misc->_mbrlen_state)
#define _REENT_MBRTOWC_STATE(ptr) ((ptr)->_misc->_mbrtowc_state)
#define _REENT_MBSRTOWCS_STATE(ptr) ((ptr)->_misc->_mbsrtowcs_state)
#define _REENT_WCRTOMB_STATE(ptr) ((ptr)->_misc->_wcrtomb_state)
#define _REENT_WCSRTOMBS_STATE(ptr) ((ptr)->_misc->_wcsrtombs_state)
#define _REENT_L64A_BUF(ptr) ((ptr)->_misc->_l64a_buf)
#define _REENT_GETDATE_ERR_P(ptr) (&((ptr)->_misc->_getdate_err))
#define _REENT_SIGNAL_BUF(ptr) ((ptr)->_signal_buf)
# 747 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
#define _REENT_INIT_PTR(var) { memset((var), 0, sizeof(*(var))); _REENT_INIT_PTR_ZEROED(var); }







#define _Kmax (sizeof (size_t) << 3)







#define __ATTRIBUTE_IMPURE_PTR__ 


extern struct _reent *_impure_ptr ;
extern struct _reent *const _global_impure_ptr ;

void _reclaim_reent (struct _reent *);
# 779 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/reent.h" 3
#define _REENT _impure_ptr


#define _GLOBAL_REENT _global_impure_ptr





#define _GLOBAL_ATEXIT (_GLOBAL_REENT->_atexit)
# 19 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 2 3
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 1 3
# 41 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define _SYS_CDEFS_H_ 



# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 1 3 4
# 46 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 2 3

#define __PMT(args) args
#define __DOTS , ...
#define __THROW 


#define __ASMNAME(cname) __XSTRING (__USER_LABEL_PREFIX__) cname


#define __ptr_t void *
#define __long_double_t long double

#define __attribute_malloc__ 
#define __attribute_pure__ 
#define __attribute_format_strfmon__(a,b) 
#define __flexarr [0]


#define __bounded 
#define __unbounded 
#define __ptrvalue 
# 76 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __has_extension __has_feature


#define __has_feature(x) 0





#define __has_builtin(x) 0






#define __BEGIN_DECLS 
#define __END_DECLS 
# 105 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __GNUCLIKE_ASM 3
#define __GNUCLIKE_MATH_BUILTIN_CONSTANTS 



#define __GNUCLIKE___TYPEOF 1
#define __GNUCLIKE___OFFSETOF 1
#define __GNUCLIKE___SECTION 1


#define __GNUCLIKE_CTOR_SECTION_HANDLING 1


#define __GNUCLIKE_BUILTIN_CONSTANT_P 1






#define __GNUCLIKE_BUILTIN_VARARGS 1
#define __GNUCLIKE_BUILTIN_STDARG 1
#define __GNUCLIKE_BUILTIN_VAALIST 1



#define __GNUC_VA_LIST_COMPATIBILITY 1






#define __compiler_membar() __asm __volatile(" " : : : "memory")



#define __GNUCLIKE_BUILTIN_NEXT_ARG 1
#define __GNUCLIKE_MATH_BUILTIN_RELOPS 


#define __GNUCLIKE_BUILTIN_MEMCPY 1


#define __CC_SUPPORTS_INLINE 1
#define __CC_SUPPORTS___INLINE 1
#define __CC_SUPPORTS___INLINE__ 1

#define __CC_SUPPORTS___FUNC__ 1
#define __CC_SUPPORTS_WARNING 1

#define __CC_SUPPORTS_VARADIC_XXX 1

#define __CC_SUPPORTS_DYNAMIC_ARRAY_INIT 1
# 175 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __P(protos) protos
#define __CONCAT1(x,y) x ## y
#define __CONCAT(x,y) __CONCAT1(x,y)
#define __STRING(x) #x
#define __XSTRING(x) __STRING(x)

#define __const const
#define __signed signed
#define __volatile volatile
# 239 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __weak_symbol __attribute__((__weak__))
# 252 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __dead2 __attribute__((__noreturn__))
#define __pure2 __attribute__((__const__))
#define __unused __attribute__((__unused__))
#define __used __attribute__((__used__))
#define __packed __attribute__((__packed__))
#define __aligned(x) __attribute__((__aligned__(x)))
#define __section(x) __attribute__((__section__(x)))


#define __alloc_size(x) __attribute__((__alloc_size__(x)))




#define __alloc_align(x) __attribute__((__alloc_align__(x)))
# 355 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __generic(expr,t,yes,no) _Generic(expr, t: yes, default: no)
# 364 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __malloc_like __attribute__((__malloc__))
#define __pure __attribute__((__pure__))






#define __always_inline __attribute__((__always_inline__))





#define __noinline __attribute__ ((__noinline__))





#define __nonnull(x) __attribute__((__nonnull__(x)))
#define __nonnull_all __attribute__((__nonnull__))






#define __fastcall __attribute__((__fastcall__))
#define __result_use_check __attribute__((__warn_unused_result__))






#define __returns_twice __attribute__((__returns_twice__))





#define __unreachable() __builtin_unreachable()
# 426 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __restrict restrict
# 459 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __predict_true(exp) __builtin_expect((exp), 1)
#define __predict_false(exp) __builtin_expect((exp), 0)






#define __sentinel __attribute__((__sentinel__))
#define __exported __attribute__((__visibility__("default")))


#define __hidden __attribute__((__visibility__("hidden")))
# 481 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __offsetof(type,field) offsetof(type, field)
#define __rangeof(type,start,end) (__offsetof(type, end) - __offsetof(type, start))
# 492 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __containerof(x,s,m) ({ const volatile __typeof(((s *)0)->m) *__x = (x); __DEQUALIFY(s *, (const volatile char *)__x - __offsetof(s, m));})
# 514 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __printflike(fmtarg,firstvararg) __attribute__((__format__ (__printf__, fmtarg, firstvararg)))

#define __scanflike(fmtarg,firstvararg) __attribute__((__format__ (__scanf__, fmtarg, firstvararg)))

#define __format_arg(fmtarg) __attribute__((__format_arg__ (fmtarg)))
#define __strfmonlike(fmtarg,firstvararg) __attribute__((__format__ (__strfmon__, fmtarg, firstvararg)))

#define __strftimelike(fmtarg,firstvararg) __attribute__((__format__ (__strftime__, fmtarg, firstvararg)))
# 533 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __gnu_inline __attribute__((__gnu_inline__, __artificial__))
# 547 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __printf0like(fmtarg,firstvararg) 




#define __strong_reference(sym,aliassym) extern __typeof (sym) aliassym __attribute__ ((__alias__ (#sym)))




#define __weak_reference(sym,alias) __asm__(".weak " #alias); __asm__(".equ " #alias ", " #sym)


#define __warn_references(sym,msg) __asm__(".section .gnu.warning." #sym); __asm__(".asciz \"" msg "\""); __asm__(".previous")



#define __sym_compat(sym,impl,verid) __asm__(".symver " #impl ", " #sym "@" #verid)

#define __sym_default(sym,impl,verid) __asm__(".symver " #impl ", " #sym "@@" #verid)
# 601 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __FBSDID(s) struct __hack



#define __RCSID(s) struct __hack



#define __RCSID_SOURCE(s) struct __hack



#define __SCCSID(s) struct __hack



#define __COPYRIGHT(s) struct __hack



#define __DECONST(type,var) ((type)(__uintptr_t)(const void *)(var))



#define __DEVOLATILE(type,var) ((type)(__uintptr_t)(volatile void *)(var))



#define __DEQUALIFY(type,var) ((type)(__uintptr_t)(const volatile void *)(var))
# 646 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __arg_type_tag(arg_kind,arg_idx,type_tag_idx) 
#define __datatype_type_tag(kind,type) 
# 665 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/cdefs.h" 3
#define __lock_annotate(x) 



#define __lockable __lock_annotate(lockable)


#define __locks_exclusive(...) __lock_annotate(exclusive_lock_function(__VA_ARGS__))

#define __locks_shared(...) __lock_annotate(shared_lock_function(__VA_ARGS__))



#define __trylocks_exclusive(...) __lock_annotate(exclusive_trylock_function(__VA_ARGS__))

#define __trylocks_shared(...) __lock_annotate(shared_trylock_function(__VA_ARGS__))



#define __unlocks(...) __lock_annotate(unlock_function(__VA_ARGS__))


#define __asserts_exclusive(...) __lock_annotate(assert_exclusive_lock(__VA_ARGS__))

#define __asserts_shared(...) __lock_annotate(assert_shared_lock(__VA_ARGS__))



#define __requires_exclusive(...) __lock_annotate(exclusive_locks_required(__VA_ARGS__))

#define __requires_shared(...) __lock_annotate(shared_locks_required(__VA_ARGS__))

#define __requires_unlocked(...) __lock_annotate(locks_excluded(__VA_ARGS__))



#define __no_lock_analysis __lock_annotate(no_thread_safety_analysis)


#define __guarded_by(x) __lock_annotate(guarded_by(x))
#define __pt_guarded_by(x) __lock_annotate(pt_guarded_by(x))
# 20 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 2 3
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/machine/stdlib.h" 1 3

#define _MACHSTDLIB_H_ 
# 21 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 2 3
# 33 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 3


typedef struct
{
  int quot;
  int rem;
} div_t;

typedef struct
{
  long quot;
  long rem;
} ldiv_t;


typedef struct
{
  long long int quot;
  long long int rem;
} lldiv_t;



#define __compar_fn_t_defined 
typedef int (*__compar_fn_t) (const void *, const void *);


#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define RAND_MAX __RAND_MAX

int __locale_mb_cur_max (void);

#define MB_CUR_MAX __locale_mb_cur_max()

void abort (void) __attribute__ ((__noreturn__));
int abs (int);





int atexit (void (*__func)(void));
double atof (const char *__nptr);



int atoi (const char *__nptr);
int _atoi_r (struct _reent *, const char *__nptr);
long atol (const char *__nptr);
long _atol_r (struct _reent *, const char *__nptr);
void * bsearch (const void * __key, const void * __base, size_t __nmemb, size_t __size, __compar_fn_t _compar)



                                ;
void * calloc (size_t __nmemb, size_t __size) ;
div_t div (int __numer, int __denom);
void exit (int __status) __attribute__ ((__noreturn__));
void free (void *) ;
char * getenv (const char *__string);
char * _getenv_r (struct _reent *, const char *__string);
char * _findenv (const char *, int *);
char * _findenv_r (struct _reent *, const char *, int *);




long labs (long);
ldiv_t ldiv (long __numer, long __denom);
void * malloc (size_t __size) ;
int mblen (const char *, size_t);
int _mblen_r (struct _reent *, const char *, size_t, _mbstate_t *);
int mbtowc (wchar_t *restrict, const char *restrict, size_t);
int _mbtowc_r (struct _reent *, wchar_t *restrict, const char *restrict, size_t, _mbstate_t *);
int wctomb (char *, wchar_t);
int _wctomb_r (struct _reent *, char *, wchar_t, _mbstate_t *);
size_t mbstowcs (wchar_t *restrict, const char *restrict, size_t);
size_t _mbstowcs_r (struct _reent *, wchar_t *restrict, const char *restrict, size_t, _mbstate_t *);
size_t wcstombs (char *restrict, const wchar_t *restrict, size_t);
size_t _wcstombs_r (struct _reent *, char *restrict, const wchar_t *restrict, size_t, _mbstate_t *);
# 133 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 3
char * _mkdtemp_r (struct _reent *, char *);
int _mkostemp_r (struct _reent *, char *, int);
int _mkostemps_r (struct _reent *, char *, int, int);
int _mkstemp_r (struct _reent *, char *);
int _mkstemps_r (struct _reent *, char *, int);
char * _mktemp_r (struct _reent *, char *) __attribute__ ((__deprecated__("the use of `mktemp' is dangerous; use `mkstemp' instead")));
void qsort (void * __base, size_t __nmemb, size_t __size, __compar_fn_t _compar);
int rand (void);
void * realloc (void * __r, size_t __size) ;
# 154 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 3
void srand (unsigned __seed);
double strtod (const char *restrict __n, char **restrict __end_PTR);
double _strtod_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR);

float strtof (const char *restrict __n, char **restrict __end_PTR);







long strtol (const char *restrict __n, char **restrict __end_PTR, int __base);
long _strtol_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR, int __base);
unsigned long strtoul (const char *restrict __n, char **restrict __end_PTR, int __base);
unsigned long _strtoul_r (struct _reent *,const char *restrict __n, char **restrict __end_PTR, int __base);
# 186 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 3
int system (const char *__string);
# 197 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 3
void _Exit (int __status) __attribute__ ((__noreturn__));




int _putenv_r (struct _reent *, char *__string);
void * _reallocf_r (struct _reent *, void *, size_t);



int _setenv_r (struct _reent *, const char *__string, const char *__value, int __overwrite);
# 219 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 3
char * __itoa (int, char *, int);
char * __utoa (unsigned, char *, int);
# 258 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 3
long long atoll (const char *__nptr);

long long _atoll_r (struct _reent *, const char *__nptr);

long long llabs (long long);
lldiv_t lldiv (long long __numer, long long __denom);
long long strtoll (const char *restrict __n, char **restrict __end_PTR, int __base);

long long _strtoll_r (struct _reent *, const char *restrict __n, char **restrict __end_PTR, int __base);

unsigned long long strtoull (const char *restrict __n, char **restrict __end_PTR, int __base);

unsigned long long _strtoull_r (struct _reent *, const char *restrict __n, char **restrict __end_PTR, int __base);
# 279 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 3
int _unsetenv_r (struct _reent *, const char *__string);






char * _dtoa_r (struct _reent *, double, int, int, int *, int*, char**);

void * _malloc_r (struct _reent *, size_t) ;
void * _calloc_r (struct _reent *, size_t, size_t) ;
void _free_r (struct _reent *, void *) ;
void * _realloc_r (struct _reent *, void *, size_t) ;
void _mstats_r (struct _reent *, char *);

int _system_r (struct _reent *, const char *);

void __eprintf (const char *, const char *, unsigned int, const char *);
# 316 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdlib.h" 3
extern long double _strtold_r (struct _reent *, const char *restrict, char **restrict);

extern long double strtold (const char *restrict, char **restrict);







void * aligned_alloc(size_t, size_t) __attribute__((__malloc__)) __attribute__((__alloc_align__(1)))
     __attribute__((__alloc_size__(2)));
int at_quick_exit(void (*)(void));
_Noreturn void
 quick_exit(int);



# 5 "1734-stm32f072/Src/DSC/circular_buffer.c" 2
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stdint.h" 1 3 4
# 9 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stdint.h" 3 4
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 1 3 4
# 10 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define _STDINT_H 


# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 1 3 4
# 10 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#define _SYS__INTSUP_H 





#define __STDINT_EXP(x) __ ##x ##__
# 35 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
       
       
       
       
       
       
       
#undef signed
#undef unsigned
#undef char
#undef short
#undef int
#undef __int20
#undef long
#define signed +0
#define unsigned +0
#define char +0
#define short +1
#define __int20 +2
#define int +2
#define long +4
# 64 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#define _INTPTR_EQ_INT 






#define _INT32_EQ_LONG 







#define __INT8 "hh"
# 90 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#define __INT16 "h"
# 101 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#define __INT32 "l"
# 110 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#define __INT64 "ll"






#define __FAST8 
# 126 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#define __FAST16 






#define __FAST32 
# 144 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#define __FAST64 "ll"



#define __LEAST8 "hh"
# 159 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#define __LEAST16 "h"
# 170 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#define __LEAST32 "l"
# 179 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#define __LEAST64 "ll"

#undef signed
#undef unsigned
#undef char
#undef short
#undef int
#undef long
       
       
       
       
       
# 191 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_intsup.h" 3 4
#undef __int20
       
       
# 14 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 2 3 4
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_stdint.h" 1 3 4
# 10 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_stdint.h" 3 4
#define _SYS__STDINT_H 
# 20 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/sys/_stdint.h" 3 4
typedef __int8_t int8_t ;
#define _INT8_T_DECLARED 


typedef __uint8_t uint8_t ;
#define _UINT8_T_DECLARED 

#define __int8_t_defined 1




typedef __int16_t int16_t ;
#define _INT16_T_DECLARED 


typedef __uint16_t uint16_t ;
#define _UINT16_T_DECLARED 

#define __int16_t_defined 1




typedef __int32_t int32_t ;
#define _INT32_T_DECLARED 


typedef __uint32_t uint32_t ;
#define _UINT32_T_DECLARED 

#define __int32_t_defined 1




typedef __int64_t int64_t ;
#define _INT64_T_DECLARED 


typedef __uint64_t uint64_t ;
#define _UINT64_T_DECLARED 

#define __int64_t_defined 1



typedef __intmax_t intmax_t;
#define _INTMAX_T_DECLARED 



typedef __uintmax_t uintmax_t;
#define _UINTMAX_T_DECLARED 



typedef __intptr_t intptr_t;
#define _INTPTR_T_DECLARED 



typedef __uintptr_t uintptr_t;
#define _UINTPTR_T_DECLARED 
# 15 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 2 3 4






typedef __int_least8_t int_least8_t;
typedef __uint_least8_t uint_least8_t;
#define __int_least8_t_defined 1



typedef __int_least16_t int_least16_t;
typedef __uint_least16_t uint_least16_t;
#define __int_least16_t_defined 1



typedef __int_least32_t int_least32_t;
typedef __uint_least32_t uint_least32_t;
#define __int_least32_t_defined 1



typedef __int_least64_t int_least64_t;
typedef __uint_least64_t uint_least64_t;
#define __int_least64_t_defined 1
# 51 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
  typedef int int_fast8_t;
  typedef unsigned int uint_fast8_t;
#define __int_fast8_t_defined 1







  typedef int int_fast16_t;
  typedef unsigned int uint_fast16_t;
#define __int_fast16_t_defined 1







  typedef int int_fast32_t;
  typedef unsigned int uint_fast32_t;
#define __int_fast32_t_defined 1







  typedef long long int int_fast64_t;
  typedef long long unsigned int uint_fast64_t;
#define __int_fast64_t_defined 1
# 128 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INTPTR_MIN (-__INTPTR_MAX__ - 1)
#define INTPTR_MAX (__INTPTR_MAX__)
#define UINTPTR_MAX (__UINTPTR_MAX__)
# 152 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT8_MIN (-__INT8_MAX__ - 1)
#define INT8_MAX (__INT8_MAX__)
#define UINT8_MAX (__UINT8_MAX__)







#define INT_LEAST8_MIN (-__INT_LEAST8_MAX__ - 1)
#define INT_LEAST8_MAX (__INT_LEAST8_MAX__)
#define UINT_LEAST8_MAX (__UINT_LEAST8_MAX__)
# 174 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT16_MIN (-__INT16_MAX__ - 1)
#define INT16_MAX (__INT16_MAX__)
#define UINT16_MAX (__UINT16_MAX__)







#define INT_LEAST16_MIN (-__INT_LEAST16_MAX__ - 1)
#define INT_LEAST16_MAX (__INT_LEAST16_MAX__)
#define UINT_LEAST16_MAX (__UINT_LEAST16_MAX__)
# 196 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT32_MIN (-__INT32_MAX__ - 1)
#define INT32_MAX (__INT32_MAX__)
#define UINT32_MAX (__UINT32_MAX__)
# 212 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT_LEAST32_MIN (-__INT_LEAST32_MAX__ - 1)
#define INT_LEAST32_MAX (__INT_LEAST32_MAX__)
#define UINT_LEAST32_MAX (__UINT_LEAST32_MAX__)
# 230 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT64_MIN (-__INT64_MAX__ - 1)
#define INT64_MAX (__INT64_MAX__)
#define UINT64_MAX (__UINT64_MAX__)
# 246 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT_LEAST64_MIN (-__INT_LEAST64_MAX__ - 1)
#define INT_LEAST64_MAX (__INT_LEAST64_MAX__)
#define UINT_LEAST64_MAX (__UINT_LEAST64_MAX__)
# 262 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT_FAST8_MIN (-__INT_FAST8_MAX__ - 1)
#define INT_FAST8_MAX (__INT_FAST8_MAX__)
#define UINT_FAST8_MAX (__UINT_FAST8_MAX__)
# 278 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT_FAST16_MIN (-__INT_FAST16_MAX__ - 1)
#define INT_FAST16_MAX (__INT_FAST16_MAX__)
#define UINT_FAST16_MAX (__UINT_FAST16_MAX__)
# 294 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT_FAST32_MIN (-__INT_FAST32_MAX__ - 1)
#define INT_FAST32_MAX (__INT_FAST32_MAX__)
#define UINT_FAST32_MAX (__UINT_FAST32_MAX__)
# 310 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT_FAST64_MIN (-__INT_FAST64_MAX__ - 1)
#define INT_FAST64_MAX (__INT_FAST64_MAX__)
#define UINT_FAST64_MAX (__UINT_FAST64_MAX__)
# 326 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INTMAX_MAX (__INTMAX_MAX__)
#define INTMAX_MIN (-INTMAX_MAX - 1)







#define UINTMAX_MAX (__UINTMAX_MAX__)







#define SIZE_MAX (__SIZE_MAX__)





#define SIG_ATOMIC_MIN (-__STDINT_EXP(INT_MAX) - 1)
#define SIG_ATOMIC_MAX (__STDINT_EXP(INT_MAX))



#define PTRDIFF_MAX (__PTRDIFF_MAX__)



#define PTRDIFF_MIN (-PTRDIFF_MAX - 1)




#define WCHAR_MIN (__WCHAR_MIN__)
# 374 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define WCHAR_MAX (__WCHAR_MAX__)
# 384 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define WINT_MAX (__WINT_MAX__)




#define WINT_MIN (__WINT_MIN__)






#define INT8_C(x) __INT8_C(x)
#define UINT8_C(x) __UINT8_C(x)
# 408 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT16_C(x) __INT16_C(x)
#define UINT16_C(x) __UINT16_C(x)
# 420 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT32_C(x) __INT32_C(x)
#define UINT32_C(x) __UINT32_C(x)
# 433 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INT64_C(x) __INT64_C(x)
#define UINT64_C(x) __UINT64_C(x)
# 449 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/stdint.h" 3 4
#define INTMAX_C(x) __INTMAX_C(x)
#define UINTMAX_C(x) __UINTMAX_C(x)
# 10 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stdint.h" 2 3 4



#define _GCC_WRAP_STDINT_H 
# 6 "1734-stm32f072/Src/DSC/circular_buffer.c" 2
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stddef.h" 1 3 4
# 7 "1734-stm32f072/Src/DSC/circular_buffer.c" 2
# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/assert.h" 1 3
# 11 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/assert.h" 3
#undef assert




#define assert(__e) ((__e) ? (void)0 : __assert_func (__FILE__, __LINE__, __ASSERT_FUNC, #__e))
# 26 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/assert.h" 3
#define __ASSERT_FUNC __func__
# 39 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/arm-atollic-eabi/include/assert.h" 3
void __assert (const char *, int, const char *) __attribute__ ((__noreturn__))
                                 ;
void __assert_func (const char *, int, const char *, const char *) __attribute__ ((__noreturn__))
                                 ;


#define static_assert _Static_assert
# 8 "1734-stm32f072/Src/DSC/circular_buffer.c" 2

# 1 "1734-stm32f072/Src/DSC/circular_buffer.h" 1




#define CIRCULAR_BUFFER_H_ 


# 1 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stdbool.h" 1 3 4
# 29 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stdbool.h" 3 4
#define _STDBOOL_H 



#define bool _Bool
#define true 1
#define false 0
# 52 "/opt/Atollic_TrueSTUDIO_for_STM32_x86_64_9.3.0/ARMTools/lib/gcc/arm-atollic-eabi/6.3.1/include/stdbool.h" 3 4
#define __bool_true_false_are_defined 1
# 9 "1734-stm32f072/Src/DSC/circular_buffer.h" 2


# 10 "1734-stm32f072/Src/DSC/circular_buffer.h"
typedef struct
{
 uint8_t *buffer;
 size_t head;
 size_t tail;
 size_t max;
 
# 16 "1734-stm32f072/Src/DSC/circular_buffer.h" 3 4
_Bool 
# 16 "1734-stm32f072/Src/DSC/circular_buffer.h"
      full;
} circular_buf_t;




void circular_buf_init (circular_buf_t *cbuf, uint8_t *buffer, size_t size);




void circular_buf_free (circular_buf_t *cbuf);



void circular_buf_reset (circular_buf_t *cbuf);




void circular_buf_put_overwrite (circular_buf_t *cbuf, uint8_t data);




int circular_buf_put (circular_buf_t *cbuf, uint8_t data);




int circular_buf_get (circular_buf_t *cbuf, uint8_t *data);





# 51 "1734-stm32f072/Src/DSC/circular_buffer.h" 3 4
_Bool 
# 51 "1734-stm32f072/Src/DSC/circular_buffer.h"
    circular_buf_empty (circular_buf_t *cbuf);





# 56 "1734-stm32f072/Src/DSC/circular_buffer.h" 3 4
_Bool 
# 56 "1734-stm32f072/Src/DSC/circular_buffer.h"
    circular_buf_full (circular_buf_t *cbuf);




size_t circular_buf_capacity (circular_buf_t *cbuf);




size_t circular_buf_size (circular_buf_t *cbuf);
# 10 "1734-stm32f072/Src/DSC/circular_buffer.c" 2

static void advance_pointer (circular_buf_t *cbuf)
{
 
# 13 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 13 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf
# 13 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 13, __func__, 
# 13 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf"
# 13 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 13 "1734-stm32f072/Src/DSC/circular_buffer.c"
             ;

 if (cbuf->full)
 {
  cbuf->tail = (cbuf->tail + 1) % cbuf->max;
 }

 cbuf->head = (cbuf->head + 1) % cbuf->max;


 cbuf->full = (cbuf->head == cbuf->tail);
}

static void retreat_pointer (circular_buf_t *cbuf)
{
 
# 28 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 28 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf
# 28 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 28, __func__, 
# 28 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf"
# 28 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 28 "1734-stm32f072/Src/DSC/circular_buffer.c"
             ;

 cbuf->full = 
# 30 "1734-stm32f072/Src/DSC/circular_buffer.c" 3 4
             0
# 30 "1734-stm32f072/Src/DSC/circular_buffer.c"
                  ;
 cbuf->tail = (cbuf->tail + 1) % cbuf->max;
}

void circular_buf_init (circular_buf_t *cbuf, uint8_t *buffer, size_t size)
{
 
# 36 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 36 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf && buffer && size
# 36 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 36, __func__, 
# 36 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf && buffer && size"
# 36 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 36 "1734-stm32f072/Src/DSC/circular_buffer.c"
                               ;

 cbuf->buffer = buffer;
 cbuf->max = size;
 circular_buf_reset (cbuf);

 
# 42 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 42 "1734-stm32f072/Src/DSC/circular_buffer.c"
circular_buf_empty (cbuf)
# 42 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 42, __func__, 
# 42 "1734-stm32f072/Src/DSC/circular_buffer.c"
"circular_buf_empty (cbuf)"
# 42 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 42 "1734-stm32f072/Src/DSC/circular_buffer.c"
                                  ;
}

void circular_buf_reset (circular_buf_t *cbuf)
{
 
# 47 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 47 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf
# 47 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 47, __func__, 
# 47 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf"
# 47 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 47 "1734-stm32f072/Src/DSC/circular_buffer.c"
             ;

 cbuf->head = 0;
 cbuf->tail = 0;
 cbuf->full = 
# 51 "1734-stm32f072/Src/DSC/circular_buffer.c" 3 4
             0
# 51 "1734-stm32f072/Src/DSC/circular_buffer.c"
                  ;
}

size_t circular_buf_size (circular_buf_t *cbuf)
{
 
# 56 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 56 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf
# 56 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 56, __func__, 
# 56 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf"
# 56 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 56 "1734-stm32f072/Src/DSC/circular_buffer.c"
             ;

 size_t size = cbuf->max;

 if (!cbuf->full)
 {
  if (cbuf->head >= cbuf->tail)
  {
   size = (cbuf->head - cbuf->tail);
  }
  else
  {
   size = (cbuf->max + cbuf->head - cbuf->tail);
  }
 }

 return size;
}

size_t circular_buf_capacity (circular_buf_t *cbuf)
{
 
# 77 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 77 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf
# 77 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 77, __func__, 
# 77 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf"
# 77 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 77 "1734-stm32f072/Src/DSC/circular_buffer.c"
             ;

 return cbuf->max;
}

void circular_buf_put_overwrite (circular_buf_t *cbuf, uint8_t data)
{
 
# 84 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 84 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf && cbuf->buffer
# 84 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 84, __func__, 
# 84 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf && cbuf->buffer"
# 84 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 84 "1734-stm32f072/Src/DSC/circular_buffer.c"
                             ;

 cbuf->buffer[cbuf->head] = data;

 advance_pointer (cbuf);
}

int circular_buf_put (circular_buf_t *cbuf, uint8_t data)
{
 int r = -1;

 
# 95 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 95 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf && cbuf->buffer
# 95 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 95, __func__, 
# 95 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf && cbuf->buffer"
# 95 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 95 "1734-stm32f072/Src/DSC/circular_buffer.c"
                             ;

 if (!circular_buf_full (cbuf))
 {
  cbuf->buffer[cbuf->head] = data;
  advance_pointer (cbuf);
  r = 0;
 }

 return r;
}

int circular_buf_get (circular_buf_t *cbuf, uint8_t *data)
{
 
# 109 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 109 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf && data && cbuf->buffer
# 109 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 109, __func__, 
# 109 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf && data && cbuf->buffer"
# 109 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 109 "1734-stm32f072/Src/DSC/circular_buffer.c"
                                     ;

 int r = -1;

 if (!circular_buf_empty (cbuf))
 {
  *data = cbuf->buffer[cbuf->tail];
  retreat_pointer (cbuf);

  r = 0;
 }

 return r;
}


# 124 "1734-stm32f072/Src/DSC/circular_buffer.c" 3 4
_Bool 
# 124 "1734-stm32f072/Src/DSC/circular_buffer.c"
    circular_buf_empty (circular_buf_t *cbuf)
{
 
# 126 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 126 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf
# 126 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 126, __func__, 
# 126 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf"
# 126 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 126 "1734-stm32f072/Src/DSC/circular_buffer.c"
             ;

 return (!cbuf->full && (cbuf->head == cbuf->tail));
}


# 131 "1734-stm32f072/Src/DSC/circular_buffer.c" 3 4
_Bool 
# 131 "1734-stm32f072/Src/DSC/circular_buffer.c"
    circular_buf_full (circular_buf_t *cbuf)
{
 
# 133 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
((
# 133 "1734-stm32f072/Src/DSC/circular_buffer.c"
cbuf
# 133 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
) ? (void)0 : __assert_func ("1734-stm32f072/Src/DSC/circular_buffer.c", 133, __func__, 
# 133 "1734-stm32f072/Src/DSC/circular_buffer.c"
"cbuf"
# 133 "1734-stm32f072/Src/DSC/circular_buffer.c" 3
))
# 133 "1734-stm32f072/Src/DSC/circular_buffer.c"
             ;

 return cbuf->full;
}
