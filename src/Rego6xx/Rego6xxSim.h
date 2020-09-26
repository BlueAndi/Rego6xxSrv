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
 * @brief  Rego6xx heatpump controller simulator
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @{
 */

#ifndef __REGO6xx_SIM_H__
#define __REGO6xx_SIM_H__

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

/**
 * Rego6xx heatpump controller simulator.
 * Used for testing purposes.
 */
class Rego6xxSim : public Stream
{
public:

    /**
     * Constructs the Rego6xx heatpump controller simulator.
     */
    Rego6xxSim() :
        m_readIndex(0),
        m_rspBuffer(),
        m_rspSize(0)
    {
    }

    /**
     * Destroys the Rego6xx heatpump controller simulator.
     */
    ~Rego6xxSim()
    {
    }

    /**
     * Get the number of available data.
     * 
     * @return Number of byte which are available
     */
    int available() override
    {
        return m_rspSize - m_readIndex;
    }

    /**
     * Read a single data byte.
     * 
     * @return Single data byte
     */
    int read() override;

    /**
     * Read a single data byte, but without increasing the internal read position.
     * 
     * @return Single data byte
     */
    int peek() override;

    /**
     * Not supported!
     * 
     * @param[in] data  Single data byte
     * 
     * @return Number of written data byte
     */
    size_t write(uint8_t data) override
    {
        /* Not supported. */
        return 0;
    }

    /**
     * Write a TCP message.
     * 
     * @param[in] buffer    Message buffer
     * @param[in] size      Message buffer size in byte
     * 
     * @return Number of written data byte
     */
    size_t write(const uint8_t* buffer, size_t size) override;

private:

    static const uint8_t    RSP_BUFFER_SIZE = 64;    /**< Rego6xx response buffer size in byte. */

    uint8_t m_readIndex;                    /**< Read index in the standard response buffer. */
    uint8_t m_rspBuffer[RSP_BUFFER_SIZE];   /**< Standard response buffer */
    size_t  m_rspSize;                      /**< Size of current filled response buffer */

    /**
     * Generate a valid standard response with the given value.
     * 
     * @param[in] value Value used for the response
     */
    void generateStdRsp(uint16_t value);

    /**
     * Generate a valid confirmation response.
     */
    void generateConfirmRsp();

    /**
     * Generate a valid text response with the given value.
     * 
     * @param[in] text Text used for the response
     */
    void generateTextRsp(const String& text);
    
    /**
     * Generate a valid text response with the given value.
     */
    void generateErrprRsp();

    /**
     * Generate a valid boolean response with the given value.
     * 
     * @param[in] value Value used for the response
     */
    void generateBoolRsp(bool value);

    /**
     * Prepare response by checking the received command.
     * 
     * @param[in] buffer    Command buffer
     * @param[in] size      Command buffer size in byte
     */
    void prepareRsp(const uint8_t* buffer, size_t size);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __REGO6xx_SIM_H__ */

/** @} */