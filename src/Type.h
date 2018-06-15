
#ifndef CJVM_TYPE_H
#define CJVM_TYPE_H

#include <cstdint>

using u4 = uint32_t;
using u2 = uint16_t;
using u1 = uint8_t;

#define getu4(buf) \
(((*(u4*)buf) & 0x000000FF) << 24) | \
(((*(u4*)buf) & 0x0000FF00) << 8)  | \
(((*(u4*)buf) & 0x00FF0000) >> 8)  | \
(((*(u4*)buf) & 0xFF000000) >> 24)


#define getu2(buf) \
(((*(u2*)buf) & 0x00FF) << 8) | \
(((*(u2*)buf) & 0xFF00) >> 8)

#define getu1(buf) \
*(u1*)buf

#endif