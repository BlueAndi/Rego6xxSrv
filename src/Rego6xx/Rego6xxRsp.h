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
 * @brief  Rego6xx heatpump controller response base
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @{
 */

#ifndef __REGO6XX_RSP_H__
#define __REGO6XX_RSP_H__

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

/* Forward declaration. */
class Rego6xxCtrl;

/**
 * Base response of the heatpump Rego6xx controller.
 */
class Rego6xxRsp
{
public:

    /**
     * Constructs a empty response.
     */
    Rego6xxRsp() :
        m_isUsed(false),
        m_isPending(false)
    {
    }

    /**
     * Destroys a response.
     */
    virtual ~Rego6xxRsp()
    {
    }

    /**
     * Is response used?
     * 
     * @return If response is in use by the controller, it will return true otherwise false.
     */
    bool isUsed() const
    {
        return m_isUsed;
    }

    /**
     * Is response pending?
     * 
     * @return If response is pending, it will return true otherwise false.
     */
    bool isPending() const
    {
        return m_isPending;
    }

    /**
     * Is response valid?
     * 
     * @return If response is valid it will return true otherwise false.
     */
    virtual bool isValid() const = 0;

    /**
     * Get device address.
     * 
     * @return Device address
     */
    virtual uint8_t getDevAddr() const = 0;

protected:

    bool    m_isUsed;       /**< Is response used by application. If no, the controller can use it again. */
    bool    m_isPending;    /**< Is response pending or not. */

    /**
     * Acquire response. Used by the controller to signal that this response
     * is used.
     */
    void acquire()
    {
        m_isUsed    = true;
        m_isPending = true;
    }

    /**
     * Release response for the controller.
     * The application shall use this to signal the controller, that the
     * response was handled and is not used anymore.
     */
    void release()
    {
        m_isUsed = false;
    }

    /**
     * Receive response. This is called by the controller.
     */
    virtual void receive() = 0;

    friend Rego6xxCtrl;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __REGO6XX_RSP_H__ */

/** @} */