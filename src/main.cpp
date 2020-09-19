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
 * @brief  Main entry point
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <EtherCard.h>
#include <ArduinoHttpServer.h>
#include <ArduinoJson.h>

#include "Logging.h"
#include "EthernetClient.h"
#include "WebReqRouter.h"
#include "Rego6xxCtrl.h"
#include "Rego6xxUtil.h"
#include "SimpleTimer.hpp"

#include <Temperature.h>

#if defined(DEBUG)
#include "Rego6xxSim.h"
#endif  /* defined(DEBUG) */

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This type defines link possible ethernet link status. */
typedef enum
{
    LINK_STATUS_UNKNOWN = 0,  /**< Unknown link status */
    LINK_STATUS_DOWN,         /**< Link is up */
    LINK_STATUS_UP            /**< Link is down */
    
} LinkStatus;

/** This type defines the temperature ids for the temperature sensors. */
typedef enum
{
    TEMPERATURE_ID_GT1 = 0,     /**< Radiator return GT1 */
    TEMPERATURE_ID_GT2,         /**< Outdoor GT2 */
    TEMPERATURE_ID_GT3,         /**< Hot water GT3 */
    TEMPERATURE_ID_GT4,         /**< Forward GT4 */
    TEMPERATURE_ID_GT5,         /**< Room GT5 */
    TEMPERATURE_ID_GT6,         /**< Compressor GT6 */
    TEMPERATURE_ID_GT8,         /**< Heat fluid out GT8 */
    TEMPERATURE_ID_GT9,         /**< Heat fluid in GT9 */
    TEMPERATURE_ID_GT10,        /**< Cold fluid in GT10 */
    TEMPERATURE_ID_GT11,        /**< Cold fulid out GT11 */
    TEMPERATURE_ID_GT3X,        /**< External hot water GT3X */

    TEMPERATURE_ID_GT3_TARGET,  /**< GT3 target value */
    TEMPERATURE_ID_GT3_ON,      /**< GT3 on value */
    TEMPERATURE_ID_GT3_OFF,     /**< GT3 off value */

    TEMPERATURE_ID_MAX          /**< Value used to determine max. number of temperatures. */

} TemperatureId;

/**
 * Status id codes for JSON responses.
 */
typedef enum
{
    STATUS_ID_OK = 0,   /**< Successful */
    STATUS_ID_EPENDING, /**< Already pending */
    STATUS_ID_EINPUT,   /**< Input data invalid */
    STATUS_ID_EPAR      /**< Parameter is missing */

} StatusId;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static String ipToStr(const uint8_t* ip);
static void printNetworkSettings(void);
static void handleNetwork(void);
static void handleRoot(EthernetClient& client, const HttpRequest& httpRequest);
static void handleSensorGetReq(EthernetClient& client, const HttpRequest& httpRequest);
static void handleSensorPostReq(EthernetClient& client, const HttpRequest& httpRequest);
static void handleDebugPostReq(EthernetClient& client, const HttpRequest& httpRequest);
static const Rego6xxStdRsp* readNextTemperatures(const TemperatureId& lastTemperature, TemperatureId& nextTemperature);

/******************************************************************************
 * Variables
 *****************************************************************************/

/** Serial interface baudrate. */
static const uint32_t           SERIAL_BAUDRATE             = 115200U;

/** Ethernet interface MAC address */
static const byte               DEVICE_MAC_ADDR[]           = { 0x00, 0x22, 0xf9, 0x01, 0x0B, 0x82 };

/** Size of ethernet buffer in byte. */
static const size_t             ETHERNET_BUFFER_SIZE        = 768;

/* Defines the provided buffer for ethernet package handling. */
byte                            Ethernet::buffer[ETHERNET_BUFFER_SIZE];

/** Current ethernet link status. */
static LinkStatus               gLinkStatus                 = LINK_STATUS_UNKNOWN;

/** HTML page header, stored in program memory. */
static const char               HTML_PAGE_HEAD[] PROGMEM    = "<!DOCTYPE html>\r\n"
                                                            "<html>\r\n"
                                                            "<head>\r\n"
                                                            "<title>Rego6xx Server</title>\r\n"
                                                            "</head>\r\n"
                                                            "<body>\r\n";

/** HTML page footer, stored in program memory. */
static const char               HTML_PAGE_TAIL[] PROGMEM    = "</body>\r\n"
                                                            "</html>";

/** Number of supported web request routes. */
static const uint8_t            NUM_ROUTES                  = 4;

/** Web request router */
static WebReqRouter<NUM_ROUTES> gWebReqRouter;

/** Duration after the first time all sensors are read. */
static const uint32_t           SENSOR_READ_INITIAL         = (2UL * 1000UL);

/** Period in ms for reading all sensors from heatpump. */
static const uint32_t           SENSOR_READ_PERIOD          = (5UL * 60UL * 1000UL);

/** Pause between every request to the heatpump controller in ms. */
static const uint32_t           REGO6xx_REQ_PAUSE           = (1UL * 1000UL);

/** Timer used to read cyclic all sensors values from heatpump. */
static SimpleTimer              gSensorReadCycleTimer;

/**
 * Timer used to pause between each heatpump request. This shall avoid problems with
 * the Rego6xx controller.
 */
static SimpleTimer              gRego6xxReqPauseTimer;

#if defined(DEBUG)

/** Rego6xx heatpump controller simulator. */
static Rego6xxSim               gRego6xxSim;

/** Rego6xx heatpump controller */
static Rego6xxCtrl              gRego6xxCtrl(gRego6xxSim);

#else

/** Rego6xx heatpump controller */
static Rego6xxCtrl              gRego6xxCtrl(Serial);

#endif  /* defined(DEBUG) */

/** Array of all heatpump temperatures, read in the last interval. */
static Temperature              gTemperatures[TEMPERATURE_ID_MAX];

/** Current requested temperature value. */
static TemperatureId            gReqTemp                    = TEMPERATURE_ID_MAX;

/** Pending Rego6xx response, used to read temperatures. */
static const Rego6xxStdRsp*     gRegoRsp                    = nullptr;

/** Contains the new GT3 target value, which to set. */
static Temperature              gGt3TargetValue;

/** Signals to set a new GT3 target value. */
static bool                     gSetNewTargetValue          = false;

/** Pending Rego6xx response, used to write GT3 target value. */
static const Rego6xxStdRsp*     gRegoRspGt3TargetValue      = nullptr;

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Setup the system.
 */
void setup()
{
    /* Setup serial interface */
    Serial.begin(SERIAL_BAUDRATE);

    LOG_INFO(F("Device starts up."));

    if (0 == ether.begin(ETHERNET_BUFFER_SIZE, DEVICE_MAC_ADDR))
    {
        LOG_ERROR(F("Failed to initialize Ethernet controller"));
    }
    else
    {
        LOG_INFO(F("Ethernet controller initialized."));

        gTemperatures[TEMPERATURE_ID_GT1].setName("gt1");
        gTemperatures[TEMPERATURE_ID_GT2].setName("gt2");
        gTemperatures[TEMPERATURE_ID_GT3].setName("gt3");
        gTemperatures[TEMPERATURE_ID_GT4].setName("gt4");
        gTemperatures[TEMPERATURE_ID_GT5].setName("gt5");
        gTemperatures[TEMPERATURE_ID_GT6].setName("gt6");
        gTemperatures[TEMPERATURE_ID_GT8].setName("gt8");
        gTemperatures[TEMPERATURE_ID_GT9].setName("gt9");
        gTemperatures[TEMPERATURE_ID_GT10].setName("gt10");
        gTemperatures[TEMPERATURE_ID_GT11].setName("gt11");
        gTemperatures[TEMPERATURE_ID_GT3X].setName("gt3X");
        gTemperatures[TEMPERATURE_ID_GT3_TARGET].setName("gt3Target");
        gTemperatures[TEMPERATURE_ID_GT3_ON].setName("gt3On");
        gTemperatures[TEMPERATURE_ID_GT3_OFF].setName("gt3Off");

        gSensorReadCycleTimer.start(SENSOR_READ_INITIAL);

        if (false == gWebReqRouter.addRoute(ArduinoHttpServer::Method::Get, "/", handleRoot))
        {
            LOG_ERROR(F("Failed to add route."));
        }

        if (false == gWebReqRouter.addRoute(ArduinoHttpServer::Method::Get, "/api/sensors/?", handleSensorGetReq))
        {
            LOG_ERROR(F("Failed to add route."));
        }

        if (false == gWebReqRouter.addRoute(ArduinoHttpServer::Method::Post, "/api/sensors", handleSensorPostReq))
        {
            LOG_ERROR(F("Failed to add route."));
        }

        if (false == gWebReqRouter.addRoute(ArduinoHttpServer::Method::Post, "/api/debug", handleDebugPostReq))
        {
            LOG_ERROR(F("Failed to add route."));
        }
    }

    return;
}

/**
 * Main loop, which is called periodically.
 */
void loop()
{
    handleNetwork();

    /* New GT3 target value requested and no other Rego6xx command is pending? 
     * Note, this may pause an ongoing temperature read cycle for a moment.
     */
    if ((true == gSetNewTargetValue) &&
        (nullptr == gRegoRsp))
    {
        if (nullptr == gRegoRspGt3TargetValue)
        {
            if ((false == gRego6xxReqPauseTimer.isTimerRunning()) ||
                (true == gRego6xxReqPauseTimer.isTimeout()))
            {
                gRegoRspGt3TargetValue = gRego6xxCtrl.writeSysReg(Rego6xxCtrl::SYSREG_ADDR_GT3_TARGET, gGt3TargetValue.getRawTemperature());
            }
        }
        else if ((true == gRegoRspGt3TargetValue->isUsed()) &&
                 (false == gRegoRspGt3TargetValue->isPending()))
        {
            gRego6xxCtrl.release(*gRegoRspGt3TargetValue);

            gSetNewTargetValue      = false;
            gRegoRspGt3TargetValue  = nullptr;

            /* Pause sending requests, after response. */
            gRego6xxReqPauseTimer.start(REGO6xx_REQ_PAUSE);
        }
        else
        {
            ;
        }
    }
    /* Read temperature sensors? */
    else if ((true == gSensorReadCycleTimer.isTimerRunning()) &&
             (true == gSensorReadCycleTimer.isTimeout()))
    {
        if (nullptr == gRegoRsp)
        {
            if ((false == gRego6xxReqPauseTimer.isTimerRunning()) ||
                (true == gRego6xxReqPauseTimer.isTimeout()))
            {
                gRegoRsp = readNextTemperatures(gReqTemp, gReqTemp);

                if (nullptr == gRegoRsp)
                {
                    gSensorReadCycleTimer.start(SENSOR_READ_PERIOD);
                }
            }
        }
        else if ((true == gRegoRsp->isUsed()) &&
                 (false == gRegoRsp->isPending()))
        {
            if (true == gRegoRsp->isValid())
            {
                gTemperatures[gReqTemp].setRawTemperature(gRegoRsp->getValue());
            }

            gRego6xxCtrl.release(*gRegoRsp);
            gRegoRsp = nullptr;

            /* Pause sending requests, after response. */
            gRego6xxReqPauseTimer.start(REGO6xx_REQ_PAUSE);
        }
        else
        {
            /* Wait for pending response. */
            ;
        }
    }

    gRego6xxCtrl.process();

    return;
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

/**
 * Convert IP-address in byte form to user friendly string.
 * 
 * @param[in] ip    Array with 4 byte ip address
 * 
 * @return IP-address in user friendly form
 */
static String ipToStr(const uint8_t* ip)
{
    String  ipAddr;
    uint8_t idx = 0;

    for(idx = 0; idx < 4; ++idx)
    {
        if (0 < idx)
        {
            ipAddr += ".";
        }

        ipAddr += ip[idx];
    }

    return ipAddr;
}

/**
 * Show network settings.
 */
static void printNetworkSettings(void)
{
    String tmp;

    tmp = F("IP     : ");
    tmp += ipToStr(ether.myip);
    LOG_INFO(tmp.c_str());

    tmp = F("Subnet : ");
    tmp += ipToStr(ether.netmask);
    LOG_INFO(tmp.c_str());

    tmp = F("Gateway: ");
    tmp += ipToStr(ether.gwip);
    LOG_INFO(tmp.c_str());

    tmp = F("DNS    : ");
    tmp += ipToStr(ether.dnsip);
    LOG_INFO(tmp.c_str());
        
    return;
}

/**
 * Handle network and webserver requests.
 */
static void handleNetwork(void)
{
    uint16_t    len = ether.packetReceive();
    uint16_t    pos = ether.packetLoop(len);

    /* Link down? */
    if (false == ENC28J60::isLinkUp())
    {
        if (LINK_STATUS_DOWN != gLinkStatus)
        {
            LOG_INFO(F("Link is down."));
        }

        gLinkStatus = LINK_STATUS_DOWN;
    }
    else
    /* Link is up */
    {
        if (LINK_STATUS_UP != gLinkStatus)
        {
            LOG_INFO(F("Link is up."));

            /* Get IP address via DHCP */
            if (false == ether.dhcpSetup())
            {
                LOG_ERROR(F("DHCP setup failed."));
            }
            else
            {
                printNetworkSettings();
            }
        }

        gLinkStatus = LINK_STATUS_UP;

        /* Valid TCP data received? */
        if (0 < pos)
        {
            EthernetClient  client(&Ethernet::buffer[pos], len - pos);
            HttpRequest     httpRequest(client);

            /* Parse the request */
            if (true == httpRequest.readRequest())
            {
                if (false == gWebReqRouter.handle(client, httpRequest))
                {
                    /* Send a 404 back, which means "Not Found" */
                    ArduinoHttpServer::StreamHttpErrorReply httpReply(client, httpRequest.getContentType(), "404");

                    LOG_ERROR(F("Requested page not found."));
                    LOG_ERROR(httpRequest.getResource().toString().c_str());

                    httpReply.send("Not Found");
                }
            }
            else
            {
                /* HTTP parsing failed. Client did not provide correct HTTP data or
                 * client requested an unsupported feature.
                 *
                 * Send a 400 back, which means "Bad Request".
                 */
                ArduinoHttpServer::StreamHttpErrorReply httpReply(client, httpRequest.getContentType(), "400");

                LOG_ERROR(F("HTTP parsing failed."));
                LOG_ERROR(httpRequest.getError().cStr());

                httpReply.send("Bad Request");
            }
        }
    }
}

/**
 * Handle GET root access.
 * 
 * @param[in] client        Ethernet client, used to send the response.
 * @param[in] httpRequest   The http request itself.
 */
static void handleRoot(EthernetClient& client, const HttpRequest& httpRequest)
{
    ArduinoHttpServer::StreamHttpReply  httpReply(client, "text/html");
    String                              data;

    data += reinterpret_cast<const __FlashStringHelper*>(HTML_PAGE_HEAD);
    data += F("<h1>Rego6xx Server</h1>\r\n");
    data += reinterpret_cast<const __FlashStringHelper*>(HTML_PAGE_TAIL);

    httpReply.send(data);

    return;
}

/**
 * Handle GET sensor access.
 * 
 * @param[in] client        Ethernet client, used to send the response.
 * @param[in] httpRequest   The http request itself.
 */
static void handleSensorGetReq(EthernetClient& client, const HttpRequest& httpRequest)
{
    ArduinoHttpServer::StreamHttpReply  httpReply(client, "application/json");
    String                              data;
    String                              sensorName      = httpRequest.getResource()[2]; /* /api/sensors/<name> */
    DynamicJsonDocument                 jsonDoc(256);
    JsonObject                          jsonData        = jsonDoc.createNestedObject("data");

    if (0 > sensorName.length())
    {
        jsonDoc["status"] = STATUS_ID_EPAR;
    }
    else
    {
        uint8_t idx     = 0;
        bool    isFound = false;

        while((TEMPERATURE_ID_MAX > idx) && (false == isFound))
        {
            if (0 != gTemperatures[idx].getName().equalsIgnoreCase(sensorName))
            {
                isFound = true;
            }
            else
            {
                ++idx;
            }
        }

        if (false == isFound)
        {
            jsonDoc["status"] = STATUS_ID_EPAR;
        }
        else
        {
            jsonData["name"]    = gTemperatures[idx].getName();
            jsonData["value"]   = gTemperatures[idx].getTemperature();

            jsonDoc["status"] = STATUS_ID_OK;
        }
    }

    (void)serializeJson(jsonDoc, data);

    httpReply.send(data);

    return;
}

/**
 * Handle POST sensor access.
 * 
 * @param[in] client        Ethernet client, used to send the response.
 * @param[in] httpRequest   The http request itself.
 */
static void handleSensorPostReq(EthernetClient& client, const HttpRequest& httpRequest)
{
    ArduinoHttpServer::StreamHttpReply  httpReply(client, "application/json");
    String                              data;
    const char*                         body            = httpRequest.getBody();
    DynamicJsonDocument                 jsonDoc(256);
    DynamicJsonDocument                 jsonDocRsp(128);

    /* If setting a new gt3 target value is still pending, a new can not be set. */
    if ((nullptr != gRegoRspGt3TargetValue) &&
        (true == gRegoRspGt3TargetValue->isUsed()))
    {
        jsonDocRsp["status"] = STATUS_ID_EPENDING;
    }
    /* Deserialization of JSON data failed? */
    else if (DeserializationError::Ok != deserializeJson(jsonDoc, body))
    {
        jsonDocRsp["status"] = STATUS_ID_EINPUT;
    }
    else
    {
        JsonObject  jsonObj = jsonDoc.as<JsonObject>();
        float       value   = 0.0f;

        if (false == jsonObj["gt3Target"].isNull())
        {
            value = jsonObj["gt3Target"].as<float>();

            gGt3TargetValue.setTemperature(value);
            gSetNewTargetValue = true;

            jsonDocRsp["status"] = STATUS_ID_OK;
        }
        else
        {
            jsonDocRsp["status"] = STATUS_ID_EPAR;
        }
    }

    (void)serializeJson(jsonDocRsp, data);

    httpReply.send(data);

    return;
}

/**
 * Handle POST debug access.
 * 
 * @param[in] client        Ethernet client, used to send the response.
 * @param[in] httpRequest   The http request itself.
 */
static void handleDebugPostReq(EthernetClient& client, const HttpRequest& httpRequest)
{
    ArduinoHttpServer::StreamHttpReply  httpReply(client, "application/json");
    String                              data;
    const char*                         body            = httpRequest.getBody();
    DynamicJsonDocument                 jsonDoc(256);
    DynamicJsonDocument                 jsonDocRsp(128);
    JsonObject                          jsonData        = jsonDocRsp.createNestedObject("data");

    /* Any command pending? */
    if ((nullptr != gRegoRsp) ||
        (nullptr != gRegoRspGt3TargetValue))
    {
        jsonDocRsp["status"] = STATUS_ID_EPENDING;
    }
    /* Deserialization of JSON data failed? */
    else if (DeserializationError::Ok != deserializeJson(jsonDoc, body))
    {
        jsonDocRsp["status"] = STATUS_ID_EINPUT;
    }
    else
    {
        JsonObject  jsonObj = jsonDoc.as<JsonObject>();
        uint8_t     cmdId   = jsonObj["cmdId"].as<uint8_t>();
        uint16_t    addr    = jsonObj["addr"].as<uint16_t>();
        uint16_t    value   = jsonObj["value"].as<uint16_t>();
        String      rsp     = gRego6xxCtrl.writeDbg(cmdId, addr, value);

        jsonData["response"] = rsp;
        jsonDocRsp["status"] = STATUS_ID_OK;
    }

    (void)serializeJson(jsonDocRsp, data);

    httpReply.send(data);

    return;
}

/**
 * Read next temperature value from heatpump.
 * 
 * @param[in] lastTemperature   Temperature id of last read temperature
 * @param[in] nextTemperature   Temperature id of next temperature read
 * 
 * @return Rego6xx standard response. A nullptr signals that all temperatures were read.
 */
static const Rego6xxStdRsp* readNextTemperatures(const TemperatureId& lastTemperature, TemperatureId& nextTemperature)
{
    const Rego6xxStdRsp*    stdRsp  = nullptr;

    switch(lastTemperature)
    {
    case TEMPERATURE_ID_GT1:
        nextTemperature = TEMPERATURE_ID_GT2;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT2);
        break;

    case TEMPERATURE_ID_GT2:
        nextTemperature = TEMPERATURE_ID_GT3;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT3);
        break;

    case TEMPERATURE_ID_GT3:
        nextTemperature = TEMPERATURE_ID_GT4;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT4);
        break;

    case TEMPERATURE_ID_GT4:
        nextTemperature = TEMPERATURE_ID_GT5;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT5);
        break;

    case TEMPERATURE_ID_GT5:
        nextTemperature = TEMPERATURE_ID_GT6;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT6);
        break;

    case TEMPERATURE_ID_GT6:
        nextTemperature = TEMPERATURE_ID_GT8;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT8);
        break;

    case TEMPERATURE_ID_GT8:
        nextTemperature = TEMPERATURE_ID_GT9;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT9);
        break;

    case TEMPERATURE_ID_GT9:
        nextTemperature = TEMPERATURE_ID_GT10;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT10);
        break;

    case TEMPERATURE_ID_GT10:
        nextTemperature = TEMPERATURE_ID_GT11;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT11);
        break;

    case TEMPERATURE_ID_GT11:
        nextTemperature = TEMPERATURE_ID_GT3X;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT3X);
        break;

    case TEMPERATURE_ID_GT3X:
        nextTemperature = TEMPERATURE_ID_GT3_TARGET;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT3_TARGET);
        break;

    case TEMPERATURE_ID_GT3_TARGET:
        nextTemperature = TEMPERATURE_ID_GT3_ON;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT3_ON);
        break;

    case TEMPERATURE_ID_GT3_ON:
        nextTemperature = TEMPERATURE_ID_GT3_OFF;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT3_OFF);
        break;

    case TEMPERATURE_ID_GT3_OFF:
        nextTemperature = TEMPERATURE_ID_MAX;
        break;

    case TEMPERATURE_ID_MAX:
        nextTemperature = TEMPERATURE_ID_GT1;
        stdRsp = gRego6xxCtrl.readSysReg(Rego6xxCtrl::SYSREG_ADDR_GT1);
        break;

    default:
        nextTemperature = TEMPERATURE_ID_MAX;
        break;
    }

    return stdRsp;
}