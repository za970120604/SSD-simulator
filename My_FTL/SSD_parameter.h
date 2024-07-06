#define _CRT_SECURE_NO_DEPRECATE

#define FreeBlockState 0 // in unRefList 
#define UsedBlockState 1 // in Multilevel_LRI_Lists 
#define OverProvisionBlockState 2 // in SpareList
#define ULNull -1

#define SystemStartTime 0

#define System_hotness_cycle 400 // proposed GC policy
#define Number_Of_Max_Valid_Pages_In_A_Victim_Block 200 // proposed GC policy
#define Vicitm_Selected_Each_Round 1 // proposed GC policy

#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "math.h"
#include "time.h"
#include "conio.h"
#include "memory.h"
#include "math.h"
#include "string.h"
#include <sys/stat.h>
#include <direct.h>  
#include <dirent.h>

typedef unsigned long DWORD ; 
typedef __int64			I64;


#ifndef Counter
#define Counter
extern int user_write_counter;
extern int init_counter;
#endif
