#ifndef _RUN_CONFIG_H_
#define _RUN_CONFIG_H_


#if defined(PQM4)

#define _M4_ASM_

#endif



//#define _SUPERCOP_



#ifndef GEN_PK_OPTION
#define GEN_PK_OPTION 1
#endif


#if  1 == GEN_PK_OPTION

#define _APPLY_LUP_
#define GEN_PK_T_WIDTH  20

#elif  2 == GEN_PK_OPTION

#define _APPLY_LUP_
#define GEN_PK_T_WIDTH  1

#elif  3 == GEN_PK_OPTION

#define _USE_P_
#define GEN_PK_T_WIDTH  10

#elif 4 == GEN_PK_OPTION

// nothing defined here

#else

error : incorrect GEN_PK_OPTION

#endif


#endif
