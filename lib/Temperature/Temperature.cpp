/* MIT License
 *
 * Copyright (c) 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Temperature class
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Temperature.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

uint16_t Temperature::getRawTemperature() const
{
    uint16_t value = 0;

    if (0 > m_floorPart)
    {
        value = m_floorPart * (-1);
    }
    else
    {
        value = m_floorPart;
    }

    value *= 10;
    value += m_fracPart;

    if (0 > m_floorPart)
    {
        value = (0xffff - value) + 1;
    }

    return value;
}

void Temperature::setRawTemperature(uint16_t value)
{
    int8_t sign = 1;
    
    if (0 != (value & 0x8000))
    {
        value   = (0xffff - value) + 1;
        sign    = -1;
    }

    m_floorPart = sign * static_cast<int8_t>(value / 10);
    m_fracPart  = static_cast<uint8_t>(value % 10);
}

float Temperature::getTemperature() const
{
    float   ffloor  = static_cast<float>(m_floorPart);
    float   ffrac   = static_cast<float>(m_fracPart) / 10.0f;
    float   result  = (0.0f > ffloor) ? (ffloor - ffrac) : (ffloor + ffrac);

    return result;
}

void Temperature::setTemperature(float temperature)
{
    const float LOWER_LIMIT = -100.0f;
    const float UPPER_LIMIT = 100.0f;

    if ((LOWER_LIMIT <= temperature) &&
        (UPPER_LIMIT >= temperature))
    {
        int16_t temp = static_cast<int16_t>(temperature * 10.0f);

        m_floorPart = static_cast<int8_t>(temp / 10);
        m_fracPart  = static_cast<uint8_t>(abs(temp) % 10);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
