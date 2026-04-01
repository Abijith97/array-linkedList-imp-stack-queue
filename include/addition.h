/**
 * @file addition.h
 * @brief Integer addition utility.
 *
 * @note The result is undefined when @p a + @p b overflows the range of
 *       @c int (C11 §6.5, UB #36). Callers must ensure both operands are
 *       within safe range when overflow matters.
 */

#ifndef ADDITION_H
#define ADDITION_H

/**
 * @brief Return the sum of two integers.
 *
 * @param[in] a First operand.
 * @param[in] b Second operand.
 * @return     Sum of @p a and @p b.
 */
int add(int a, int b);

#endif /* ADDITION_H */
