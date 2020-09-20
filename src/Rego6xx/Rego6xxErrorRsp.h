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
 * @brief  Rego6xx heatpump controller error response
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @{
 */

#ifndef __REGO6XX_ERROR_RSP_H__
#define __REGO6XX_ERROR_RSP_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include "Rego6xxRsp.h"
#include "SimpleTimer.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A response of the Rego6xx heatpump controller, containing a error log message.
 */
class Rego6xxErrorRsp : public Rego6xxRsp
{
public:

    /**
     * Constructs a empty response.
     * 
     * @param[in] stream    Input stream from heatpump controller.
     */
    Rego6xxErrorRsp(Stream& stream) :
        Rego6xxRsp(),
        m_stream(stream),
        m_response(),
        m_timer()
    {
    }

    /**
     * Destroys a response.
     */
    ~Rego6xxErrorRsp()
    {
    }

    /**
     * Is response valid?
     * 
     * @return If response is valid it will return true otherwise false.
     */
    bool isValid() const override;

    /**
     * Get device address.
     * 
     * @return Device address
     */
    uint8_t getDevAddr() const override;

    /**
     * Get error id.
     * 
     * @return Error id
     */
    uint8_t getErrorId() const;

    /**
     * Get error log.
     * 
     * @return Error log
     */
    String getErrorLog() const;

    /**
     * Get error in user friendly form.
     * 
     * @return Error description
     */
    String getErrorDescription() const;

private:

    /** Response size in bytes */
    static const uint8_t    RSP_SIZE    = 42;

    /** Timeout in ms */
    static const uint32_t   TIMEOUT     = (30UL * 1000UL);

    Stream&     m_stream;               /**< Input stream from heatpump controller. */
    uint8_t     m_response[RSP_SIZE];   /**< Response message */
    SimpleTimer m_timer;                /**< Used for response timeout observation. */

    Rego6xxErrorRsp();

    /**
     * Receive response. This is called by the controller.
     */
    void receive() override;

    friend Rego6xxCtrl;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __REGO6XX_ERROR_RSP_H__ */

/** @} */