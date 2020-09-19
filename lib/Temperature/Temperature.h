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
 *
 * @{
 */

#ifndef __TEMPERATURE_H__
#define __TEMPERATURE_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Heatpump temperature.
 */
class Temperature
{
public:

    /**
     * Constructs a temperature.
     */
    Temperature() :
        m_name(),
        m_floorPart(0),
        m_fracPart(0)
    {
    }

    /**
     * Destroys a temperature.
     */
    ~Temperature()
    {
    }

    /**
     * Get temperature name.
     * 
     * @return Temperature name
     */
    String getName() const
    {
        return m_name;
    }

    /**
     * Set temperature name.
     * 
     * @param[in] name  Temperature name
     */
    void setName(const String& name)
    {
        m_name = name;
    }

    /**
     * Get raw temperature value.
     * 
     * @return Raw temperature value, used by Rego6xx controller.
     */
    uint16_t getRawTemperature() const;

    /**
     * Set raw temperature value, which contains inside the floor and
     * fractional part in degree celsius.
     * 
     * @param[in] value Raw temperature value
     */
    void setRawTemperature(uint16_t value);

    /**
     * Get floor part of temperature in degree celsius.
     * 
     * @return Floor temperature part in degree celsius.
     */
    int8_t getFloorValue() const
    {
        return m_floorPart;
    }

    /**
     * Get fractional part of temperature in degree celsius.
     * 
     * @return Fractional temperature part in degree celsius.
     */
    int8_t getFractionalValue() const
    {
        return m_fracPart;
    }

    /**
     * Get temperature in degree celsius.
     * 
     * @return Temperature in degree celsius
     */
    float getTemperature() const;

    /**
     * Set temperature in degree celsius.
     * 
     * @param[in] temperature   Temperature in degree celsius
     */
    void setTemperature(float temperature);

private:

    String  m_name;         /**< Temperature name */
    int8_t  m_floorPart;    /**< Floor temperature part in degree celsius */
    uint8_t m_fracPart;     /**< Frac temperature part in degree celsius */

};

#endif  /* __TEMPERATURE_H__ */

/** @} */