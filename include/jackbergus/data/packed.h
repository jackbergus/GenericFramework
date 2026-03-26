//
// Created by gyankos on 11/03/26.
//

#ifndef FLOATMAX_PACKED_H
#define FLOATMAX_PACKED_H

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#else
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#endif //FLOATMAX_PACKED_H