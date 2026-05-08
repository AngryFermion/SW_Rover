

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Mon Jan 18 22:14:07 2038
 */
/* Compiler settings for win64\mwcomutil.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=IA64 8.01.0622 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        EXTERN_C __declspec(selectany) const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif // !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IMWUtil,0xC47EA90E,0x56D1,0x11d5,0xB1,0x59,0x00,0xD0,0xB7,0xBA,0x75,0x44);


MIDL_DEFINE_GUID(IID, LIBID_MWComUtil,0x25F520E1,0xDB54,0x4720,0xB2,0x44,0x5F,0xB5,0x77,0x67,0xAD,0x42);


MIDL_DEFINE_GUID(CLSID, CLSID_MWField,0xE8C03007,0xD333,0x4C21,0x98,0x1E,0x89,0x7E,0x50,0xAB,0x3C,0x11);


MIDL_DEFINE_GUID(CLSID, CLSID_MWStruct,0xDC2BE083,0x3490,0x4C1D,0x9A,0xF7,0x36,0xD8,0x00,0x5B,0xB8,0x2F);


MIDL_DEFINE_GUID(CLSID, CLSID_MWComplex,0x0008A30D,0x13A2,0x42B3,0x82,0x0A,0x4C,0x7B,0xE1,0x35,0xAA,0xC5);


MIDL_DEFINE_GUID(CLSID, CLSID_MWSparse,0xBAC5AB6C,0x84C6,0x4705,0xBB,0x72,0xD5,0x2B,0x50,0x37,0x60,0xB0);


MIDL_DEFINE_GUID(CLSID, CLSID_MWArg,0xF68D03C5,0xA2F6,0x412B,0x97,0x63,0xA8,0x5D,0x6F,0x4B,0x0F,0x6F);


MIDL_DEFINE_GUID(CLSID, CLSID_MWArrayFormatFlags,0x7074BEBC,0x94D7,0x4771,0x9F,0xCD,0xFC,0x29,0x8F,0xF1,0x16,0xB2);


MIDL_DEFINE_GUID(CLSID, CLSID_MWDataConversionFlags,0x9663559E,0xA92E,0x4495,0xB5,0xD9,0xE0,0xED,0x9B,0x8A,0xFB,0x92);


MIDL_DEFINE_GUID(CLSID, CLSID_MWUtil,0x08CAD1E1,0x8F9F,0x40D3,0x8E,0x72,0xA0,0x2B,0xC8,0x9F,0x94,0x0A);


MIDL_DEFINE_GUID(CLSID, CLSID_MWFlags,0xC8E2CE45,0xD745,0x47F9,0x8B,0xD9,0x4A,0x47,0x5D,0x43,0x41,0xBC);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



