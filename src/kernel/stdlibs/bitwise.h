
#pragma once
#ifndef _BITWISE_H_
#define _BITWISE_H_

#include <stdint.h>

/**
 * @brief Comma Operator
 * 
 * , - COMMA:
 *      - Comma can be used as an operator.
 *      - Comma operator is having the last precendence among all other operators in C.
 *      - int a = (3, 4, 8);
 *            Output: 8 
 *          - Returns the right most operand in the expression.
 * 
 *      - int b = (printf("%s\n", "HELLO!"), 5);
 *            Output: 
 *              HELLO
 *              5
 *          - Evaluates the others expressions, reject them and return the right most operand in the expression.
 *      - int a;
 *        a = 3, 4, 8;
 *            Output: 3
 *          - EQUALITY(=) operator has higher precendence than COMMA(,) operator.
 * 
 * 
 */

/**
 * @brief Logical Operators
 * 
 * && - AND:
 *      - Takes two values at a time and perform two bitwise AND(&) operations.
 *      - AND(&&) is a Logical Operator.
 *      - Performs a left-to-right evaluation.
 *      - Result of AND is 1 when both values are greather than 0.
 * 
 * || - OR:
 *      - Takes two values at a time and perform two bitwise OR(&) operations.
 *      - OR(||) is a Logical operator.
 *      - Performs a left-to-right evaluation.
 *      - Result of OR is 1 when one of the values are greather than 1.
 */

/**
 * @brief Bitwise Operators
 * 
 * &  - AND: 
 *      - Takes two bits at a time and perform AND operation.
 *      - AND(&) is a binary operator so it requires two operands.
 *      - Performs a left-to-right evaluation.
 *      - Result of AND is 1 when both bits are 1.
 * 
 * |  - OR:
 *      - Also known as Inclusive OR.
 *      - Takes two bits at a time and perform OR operation.
 *      - OR(|) is a binary operator so it requires two operands.
 *      - Performs a left-to-right evaluation.
 *      - Result of OR is 0 when both bits are 0.
 *      - INCLUSIVE_BOTH: Since it's inclusive Result is 1 when both bits are 1.
 * 
 * ~  - NOT:
 *      - Complement each bit one by one.
 *      - NOT(~) is a unary operator so it requires one value.
 *      - Performs a right-to-left evaluation.
 *      - Result of NOT is 1 when bit is 0, and 0 when bit is 1.
 * 
 * << - LEFT SHIFT:
 *      - Shift to the left the first operand bits, by the amount of bits specified by the second operand.
 *      - The trailing positions that was shifted and are empty are filled with 0.
 *      - LEFT_SHIFT(<<) is a binary operator so it requires two operands.
 *      - Is equivalent to (left_operand * 2^right_operand).
 * 
 * >> - RIGHT SHIFT:
 *      - Shift to the right the first operand its, by the amount of bits specified by the second operand.
 *      - The leading positions that was shifted and are empty are filled with 0.
 *      - RIGHT_SHIFT(>>) is a binary operator so it requires two operands.
 *      - Is equivalent to (left_operand / 2^right_operand).
 * 
 * ^  - XOR
 *      - Also known as Exclusive OR.
 *      - Same as OR(|) but result is 0 when both are 1.
 *      - XOR(^) is a binary operator so it requires two operands.
 *      - EXLUSIVE_BOTH: Since it's exclusive Result is 0 when both bits are 1.
 */

/**
 * @brief To get the low bits we perform a AND operation. This means that: 
 * If the addr_low value is less than 0xFFFF value then this value is returned since it is less than 0xFFFF
 * If is greather than 0xFFFF then 0xFFFF is returned.
 * It's an operation that limit the max value and return lower values.
 */
#define low_16(addr) (uint16_t)((addr) & 0xFFFF)

/**
 * @brief To get the high bits we perform a right_shift in addr_high bits moving then to the lower bits.
 * Then we perform the same AND operation. This means that: 
 * If the addr_high value is less than 0xFFFF value then this value is returned since it is less than 0xFFFF
 * If is greather than 0xFFFF then 0xFFFF is returned.
 * It's an operation that limit the max value and return lower values.
 */
#define high_16(addr) (uint16_t)(((addr) >> 16) & 0xFFFF)

#define first_bit_set_index(value) ({ \
    uint8_t offset = 0; \
    if (value > 0) { while(offset < 32 && ((value >> offset) & 0x1) == 0) { offset++; } } \
    offset; \
}) \

#endif
