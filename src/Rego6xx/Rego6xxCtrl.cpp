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
 * @brief  Rego6xx heatpump controller
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Rego6xxCtrl.h"
#include "Rego6xxUtil.h"

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

const Rego6xxStdRsp* Rego6xxCtrl::readSysReg(Rego6xxCtrl::SysRegAddr sysRegAddr)
{
    const Rego6xxStdRsp*    rsp = nullptr;

    if (nullptr == m_pendingRsp)
    {
        writeCmd(DEV_ADDR_HEATPUMP, CMD_ID_READ_SYSTEM_REG, sysRegAddr, 0);
        m_stdRsp.acquire();
        m_pendingRsp    = &m_stdRsp;
        rsp             = &m_stdRsp;
    }

    return rsp;
}

const Rego6xxConfirmRsp* Rego6xxCtrl::writeSysReg(Rego6xxCtrl::SysRegAddr sysRegAddr, uint16_t data)
{
    const Rego6xxConfirmRsp*    rsp = nullptr;

    if (nullptr == m_pendingRsp)
    {
        writeCmd(DEV_ADDR_HEATPUMP, CMD_ID_WRITE_SYSTEM_REG, sysRegAddr, data);
        m_confirmRsp.acquire();
        m_pendingRsp    = &m_confirmRsp;
        rsp             = &m_confirmRsp;
    }

    return rsp;
}

const Rego6xxErrorRsp* Rego6xxCtrl::readLastError()
{
    const Rego6xxErrorRsp*  rsp = nullptr;

    if (nullptr == m_pendingRsp)
    {
        writeCmd(DEV_ADDR_HEATPUMP, CMD_ID_READ_LAST_ERROR, 0, 0);
        m_errorRsp.acquire();
        m_pendingRsp    = &m_errorRsp;
        rsp             = &m_errorRsp;
    }

    return rsp;
}

const Rego6xxStdRsp* Rego6xxCtrl::readRegoVersion()
{
    const Rego6xxStdRsp*    rsp = nullptr;

    if (nullptr == m_pendingRsp)
    {
        writeCmd(DEV_ADDR_HEATPUMP, CMD_ID_READ_REGO_VERSION, 0, 0);
        m_stdRsp.acquire();
        m_pendingRsp    = &m_stdRsp;
        rsp             = &m_stdRsp;
    }

    return rsp;
}

String Rego6xxCtrl::writeDbg(uint8_t cmdId, uint16_t addr, uint16_t data)
{
    uint8_t             cmdBuffer[CMD_SIZE];
    const size_t        RCV_BUFFER_SIZE = 64;
    uint8_t             rcvBuffer[RCV_BUFFER_SIZE];
    const unsigned long TIMEOUT         = 4000;
    unsigned long       lastTimeout     = m_stream.getTimeout();
    size_t              read            = 0;
    size_t              idx             = 0;
    String              rsp;

    /*
        *----------------*------------*------------------*------*----------*
        |       1        |      1     |         3        |   3  |     1    | <- Number of bytes
        *----------------*------------*------------------*------*----------*
        | Device Address | Command ID | Register Address | Data | Checksum |
        *----------------*------------*------------------*------*----------*
    */
    
    /* Common rules:
        - MSB first
        - 7 bit communication is used,
            e.g. register address 0x1234 in binary form 00010010 001101000
            will be expanded to 7bit form 00 0100100 01101000
    */
    cmdBuffer[0] = DEV_ADDR_HEATPUMP;
    cmdBuffer[1] = cmdId;
    cmdBuffer[2] = (addr >> 14) & 0x03;
    cmdBuffer[3] = (addr >>  7) & 0x7f;
    cmdBuffer[4] = (addr >>  0) & 0x7f;
    cmdBuffer[5] = (data >> 14) & 0x03;
    cmdBuffer[6] = (data >>  7) & 0x7f;
    cmdBuffer[7] = (data >>  0) & 0x7f;
    cmdBuffer[8] = Rego6xxUtil::calculateChecksum(&cmdBuffer[2], CMD_SIZE - 3);

    (void)m_stream.write(cmdBuffer, CMD_SIZE);

    m_stream.setTimeout(TIMEOUT);
    read = m_stream.readBytes(rcvBuffer, RCV_BUFFER_SIZE);
    m_stream.setTimeout(lastTimeout);

    while(read > idx)
    {
        char buffer[3];

        sprintf(buffer, "%02X", rcvBuffer[idx]);
        rsp += buffer;

        ++idx;
    }

    return rsp;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void Rego6xxCtrl::writeCmd(uint8_t devAddr, CmdId cmdId, uint16_t regAddr, uint16_t data)
{
    uint8_t cmdBuffer[CMD_SIZE];

    /*
        *----------------*------------*------------------*------*----------*
        |       1        |      1     |         3        |   3  |     1    | <- Number of bytes
        *----------------*------------*------------------*------*----------*
        | Device Address | Command ID | Register Address | Data | Checksum |
        *----------------*------------*------------------*------*----------*
    */
    
    /* Common rules:
        - MSB first
        - 7 bit communication is used,
            e.g. register address 0x1234 in binary form 00010010 001101000
            will be expanded to 7bit form 00 0100100 01101000
    */
    cmdBuffer[0] = devAddr;
    cmdBuffer[1] = cmdId;
    cmdBuffer[2] = (regAddr >> 14) & 0x03;
    cmdBuffer[3] = (regAddr >>  7) & 0x7f;
    cmdBuffer[4] = (regAddr >>  0) & 0x7f;
    cmdBuffer[5] = (data >> 14) & 0x03;
    cmdBuffer[6] = (data >>  7) & 0x7f;
    cmdBuffer[7] = (data >>  0) & 0x7f;
    cmdBuffer[8] = Rego6xxUtil::calculateChecksum(&cmdBuffer[2], CMD_SIZE - 3);

    (void)m_stream.write(cmdBuffer, CMD_SIZE);

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
