
#pragma once
#ifndef _BITWISE_H_
#define _BITWISE_H_

#define low_16(addr) (uint16_t)((addr) & 0xFFFF)
#define high_16(addr) (uint16_t)(((addr) >> 16) & 0xFFFF)

#endif
