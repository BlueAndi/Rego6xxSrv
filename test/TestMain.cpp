/* MIT License
 *
 * Copyright (c) 2020 - 2024 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  Main test entry point
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <stdio.h>
#include <stdlib.h>

#include <Temperature.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void testTemperature(void);

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Main entry point
 *
 * @param[in] argc  Number of command line arguments
 * @param[in] argv  Command line arguments
 */
extern int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    UNITY_BEGIN();

    RUN_TEST(testTemperature);

    return UNITY_END();
}

/**
 * Setup a test. This function will be called before every test by unity.
 */
extern void setUp(void)
{
    /* Not used. */
}

/**
 * Clean up test. This function will be called after every test by unity.
 */
extern void tearDown(void)
{
    /* Not used. */
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

/**
 * Test temperature class.
 */
static void testTemperature(void)
{
    Temperature     testTemperature;

    const uint16_t  TEMPERATURE_1_RAW   = 0x0138;
    const int8_t    TEMPERATURE_1_FLOOR = 31;
    const uint8_t   TEMPERATURE_1_FRAC  = 2;
    const float     TEMPERATURE_1_FLOAT = 31.2f;

    const uint16_t  TEMPERATURE_2_RAW   = 0xfe1d;
    const int8_t    TEMPERATURE_2_FLOOR = -48;
    const uint8_t   TEMPERATURE_2_FRAC  = 3;
    const float     TEMPERATURE_2_FLOAT = -48.3f;

    const uint16_t  EPSILON_RAW     = 1;
    const uint8_t   EPSILON_FRAC    = 1;
    const float     EPSILON_FLOAT   = 0.1f;

    testTemperature.setRawTemperature(TEMPERATURE_1_RAW);

    TEST_ASSERT_EQUAL_UINT16(TEMPERATURE_1_RAW, testTemperature.getRawTemperature());
    TEST_ASSERT_EQUAL_INT8(TEMPERATURE_1_FLOOR, testTemperature.getFloorValue());
    TEST_ASSERT_EQUAL_UINT8(TEMPERATURE_1_FRAC, testTemperature.getFractionalValue());
    TEST_ASSERT_EQUAL_FLOAT(TEMPERATURE_1_FLOAT, testTemperature.getTemperature());

    testTemperature.setTemperature(TEMPERATURE_1_FLOAT);

    TEST_ASSERT_EQUAL_UINT16(TEMPERATURE_1_RAW, testTemperature.getRawTemperature());
    TEST_ASSERT_EQUAL_INT8(TEMPERATURE_1_FLOOR, testTemperature.getFloorValue());
    TEST_ASSERT_EQUAL_UINT8(TEMPERATURE_1_FRAC, testTemperature.getFractionalValue());
    TEST_ASSERT_EQUAL_FLOAT(TEMPERATURE_1_FLOAT, testTemperature.getTemperature());

    testTemperature.setRawTemperature(TEMPERATURE_2_RAW);

    TEST_ASSERT_EQUAL_UINT16(TEMPERATURE_2_RAW, testTemperature.getRawTemperature());
    TEST_ASSERT_EQUAL_INT8(TEMPERATURE_2_FLOOR, testTemperature.getFloorValue());
    TEST_ASSERT_EQUAL_UINT8(TEMPERATURE_2_FRAC, testTemperature.getFractionalValue());
    TEST_ASSERT_EQUAL_FLOAT(TEMPERATURE_2_FLOAT, testTemperature.getTemperature());

    testTemperature.setTemperature(TEMPERATURE_2_FLOAT);

    TEST_ASSERT_GREATER_OR_EQUAL_UINT16(TEMPERATURE_2_RAW - EPSILON_RAW, testTemperature.getRawTemperature());
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(TEMPERATURE_2_RAW + EPSILON_RAW, testTemperature.getRawTemperature());

    TEST_ASSERT_EQUAL_INT8(TEMPERATURE_2_FLOOR, testTemperature.getFloorValue());

    TEST_ASSERT_GREATER_OR_EQUAL_UINT8(TEMPERATURE_2_FRAC - EPSILON_FRAC, testTemperature.getFractionalValue());
    TEST_ASSERT_LESS_OR_EQUAL_UINT8(TEMPERATURE_2_FRAC + EPSILON_FRAC, testTemperature.getFractionalValue());

    TEST_ASSERT_TRUE((TEMPERATURE_2_FLOAT - EPSILON_FLOAT) <= testTemperature.getTemperature());
    TEST_ASSERT_TRUE((TEMPERATURE_2_FLOAT + EPSILON_FLOAT) >= testTemperature.getTemperature());
}