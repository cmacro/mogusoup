/*
 * config.h
 * 
 *  Created on: 2015-02-17
 *      Author: CrystalIce
 *
 *  用于界面无关跨平台配置
 *  平台基础配置单元，针对平台特殊属性配置。此单元有别与mlplatform.h单元。
 *  
 *  mlplateform.h 单元用于界面有关平台配置  
 * 
 */

#ifndef _MLCONFIG_H_
#define _MLCONFIG_H_

#include <stdio.h>
#include <stdint.h>

typedef unsigned char       byte;       /* 1-byte (or larger) unsigned integer */
typedef unsigned short int  word;       /* 2-byte (or larger) signed integer */
typedef unsigned int        uint;       /* 4-byte unsigned integer */
typedef char                int8;
typedef short int           int16;
typedef int                 int32;
typedef long long int       int64;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;
typedef int                 mlbool;
typedef unsigned char       uchar;


#ifndef ML_ALIGNMENT
#define ML_ALIGNMENT   sizeof(unsigned long)    /* platform word */
#endif

#define ml_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
#define ml_align_ptr(p, a)                                                   \
    (uchar *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))


typedef int                     mlStatus;
#define ML_OK                   0       // 没有错误
#define ML_ERROR               -1
#define ML_AGAIN               -2
#define ML_BUSY                -3
#define ML_DONE                -4
#define ML_DECLINED            -5
#define ML_ABORT               -6

#define MLERR_ALLOCMEMORY           101                     // 内存分配错误
#define MLERR_INCOMPLETEPARAM       102                     // 参数不完整
#define MLERR_INCOMPLETEFUNC        103                     // 回调函数不完整
#define MLERR_NODATA                104                     // 没有数据
#define MLERR_INVALIDATEDATA        105                     // 无效数据
#define MLERR_TYPEMISMATCH          106                     // 类型不匹配, 
#define MLERR_TYPEUNKNOWN           107                     // 类型为定义，未知类型
#define MLERR_CONVECT               108                     // 类型转换错误

#define MLSTATUS_SUCC(v)         (v) == ML_OK
#define MLSTATUS_FAIL(v)         (v) != ML_OK


#endif // _MLCONFIG_H_