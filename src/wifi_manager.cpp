/*
* WiFi Events

0  ARDUINO_EVENT_WIFI_READY               < ESP32 WiFi ready
1  ARDUINO_EVENT_WIFI_SCAN_DONE                < ESP32 finish scanning AP
2  ARDUINO_EVENT_WIFI_STA_START                < ESP32 station start
3  ARDUINO_EVENT_WIFI_STA_STOP                 < ESP32 station stop
4  ARDUINO_EVENT_WIFI_STA_CONNECTED            < ESP32 station connected to AP
5  ARDUINO_EVENT_WIFI_STA_DISCONNECTED         < ESP32 station disconnected from AP
6  ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE      < the auth mode of AP connected by ESP32 station changed
7  ARDUINO_EVENT_WIFI_STA_GOT_IP               < ESP32 station got IP from connected AP
8  ARDUINO_EVENT_WIFI_STA_LOST_IP              < ESP32 station lost IP and the IP is reset to 0
9  ARDUINO_EVENT_WPS_ER_SUCCESS       < ESP32 station wps succeeds in enrollee mode
10 ARDUINO_EVENT_WPS_ER_FAILED        < ESP32 station wps fails in enrollee mode
11 ARDUINO_EVENT_WPS_ER_TIMEOUT       < ESP32 station wps timeout in enrollee mode
12 ARDUINO_EVENT_WPS_ER_PIN           < ESP32 station wps pin code in enrollee mode
13 ARDUINO_EVENT_WIFI_AP_START                 < ESP32 soft-AP start
14 ARDUINO_EVENT_WIFI_AP_STOP                  < ESP32 soft-AP stop
15 ARDUINO_EVENT_WIFI_AP_STACONNECTED          < a station connected to ESP32 soft-AP
16 ARDUINO_EVENT_WIFI_AP_STADISCONNECTED       < a station disconnected from ESP32 soft-AP
17 ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED         < ESP32 soft-AP assign an IP to a connected station
18 ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED        < Receive probe request packet in soft-AP interface
19 ARDUINO_EVENT_WIFI_AP_GOT_IP6               < ESP32 ap interface v6IP addr is preferred
19 ARDUINO_EVENT_WIFI_STA_GOT_IP6              < ESP32 station interface v6IP addr is preferred
20 ARDUINO_EVENT_ETH_START                < ESP32 ethernet start
21 ARDUINO_EVENT_ETH_STOP                 < ESP32 ethernet stop
22 ARDUINO_EVENT_ETH_CONNECTED            < ESP32 ethernet phy link up
23 ARDUINO_EVENT_ETH_DISCONNECTED         < ESP32 ethernet phy link down
24 ARDUINO_EVENT_ETH_GOT_IP               < ESP32 ethernet got IP from connected AP
19 ARDUINO_EVENT_ETH_GOT_IP6              < ESP32 ethernet interface v6IP addr is preferred
25 ARDUINO_EVENT_MAX
*/

#include "config.h"
#include "wifi_manager.h"
#include "gvret_comm.h"
#include "SerialConsole.h"
#include <ESPmDNS.h>
#include <Update.h>
#include <WiFi.h>
#include "ELM327_Emulator.h"

// WARNING: This function is called from a separate FreeRTOS task (thread)!
void WiFiEvent(WiFiEvent_t event)
{
    Serial.printf("[WiFi-event] event: %d\n", event);

    switch (event)
    {
    case ARDUINO_EVENT_WIFI_READY:
        Serial.println("WiFi interface ready");
        break;
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
        Serial.println("Completed scan for access points");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        Serial.println("WiFi client started");
        break;
    case ARDUINO_EVENT_WIFI_STA_STOP:
        Serial.println("WiFi clients stopped");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        Serial.println("Connected to access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        Serial.println("Disconnected from WiFi access point");
        break;
    case ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE:
        Serial.println("Authentication mode of access point has changed");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        Serial.print("Obtained IP address: ");
        Serial.println(WiFi.localIP());
        break;
    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
        Serial.println("Lost IP address and IP address is reset to 0");
        break;
    case ARDUINO_EVENT_WPS_ER_SUCCESS:
        Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_FAILED:
        Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_TIMEOUT:
        Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
        break;
    case ARDUINO_EVENT_WPS_ER_PIN:
        Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
        break;
    case ARDUINO_EVENT_WIFI_AP_START:
        Serial.println("WiFi access point started");
        break;
    case ARDUINO_EVENT_WIFI_AP_STOP:
        Serial.println("WiFi access point  stopped");
        break;
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        Serial.println("Client connected");
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        Serial.println("Client disconnected");
        break;
    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
        Serial.println("Assigned IP address to client");
        break;
    case ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED:
        Serial.println("Received probe request");
        break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
        Serial.println("AP IPv6 is preferred");
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP6:
        Serial.println("STA IPv6 is preferred");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP6:
        Serial.println("Ethernet IPv6 is preferred");
        break;
    case ARDUINO_EVENT_ETH_START:
        Serial.println("Ethernet started");
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("Ethernet stopped");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("Ethernet connected");
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("Ethernet disconnected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.println("Obtained IP address");
        break;
    default:
        break;
    }
}

boolean needServerInit = false;

// WARNING: This function is called from a separate FreeRTOS task (thread)!
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(IPAddress(info.got_ip.ip_info.ip.addr));
    needServerInit = true;
    SysSettings.isWifiConnected = true;
}

static IPAddress broadcastAddr(255, 255, 255, 255);

WiFiManager::WiFiManager()
{
    lastBroadcast = 0;
}

void WiFiManager::setup()
{

    // delete old config
    WiFi.disconnect(true);

    if (settings.enableBT != 0)
        return;                 // No wifi if BT is on
    if (settings.wifiMode == 1) // connect to an AP
    {
        // Examples of different ways to register wifi events;
        // these handlers will be called from another thread.
        WiFi.onEvent(WiFiEvent);
        WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
        WiFiEventId_t eventID = WiFi.onEvent(
            [](WiFiEvent_t event, WiFiEventInfo_t info)
            {
                Serial.print("WiFi lost connection. Reason: ");
                Serial.println(info.wifi_sta_disconnected.reason);
                SysSettings.isWifiConnected = false;
                SysSettings.isWifiActive = false;
            },
            WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

        // Remove WiFi event
        Serial.print("WiFi Event ID: ");
        Serial.println(eventID);
        // WiFi.removeEvent(eventID);

        WiFi.mode(WIFI_STA);
        WiFi.setSleep(true); // sleeping could cause delays
        WiFi.begin((const char *)settings.SSID, (const char *)settings.WPA2Key);

        Serial.println();
        Serial.println();
        Serial.println("Wait for WiFi... ");

        while (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            Serial.println("Connection Failed! Rebooting...");
            delay(5000);
            ESP.restart();
        }

        // MDNS.begin wants the name we will register as without the .local on the end. That's added automatically.
        if (!MDNS.begin(deviceName))
            Serial.println("Error setting up MDNS responder!");
        MDNS.addService("telnet", "tcp", 23);   // Add service to MDNS-SD
        MDNS.addService("ELM327", "tcp", 1000); // Add service to MDNS-SD
        wifiServer.begin(23);                   // setup as a telnet server
        wifiServer.setNoDelay(true);
        Serial.println("TCP server started");
        wifiOBDII.begin(1000); // setup for wifi linked ELM327 emulation
        wifiOBDII.setNoDelay(true);
        ArduinoOTA.setPort(3232);
        ArduinoOTA.setHostname(deviceName);
        // No authentication by default
        // ArduinoOTA.setPassword("admin");

        ArduinoOTA
            .onStart([]()
                     {
                      String type;
                      if (ArduinoOTA.getCommand() == U_FLASH)
                         type = "sketch";
                      else // U_SPIFFS
                         type = "filesystem";

                      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
                      Serial.println("Start updating " + type); })
            .onEnd([]()
                   { Serial.println("\nEnd"); })
            .onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
            .onError([](ota_error_t error)
                     {
                      Serial.printf("Error[%u]: ", error);
                      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
                      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
                      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
                      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
                      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

        ArduinoOTA.begin();
    }
    if (settings.wifiMode == 2) // BE an AP
    {
        WiFi.mode(WIFI_AP);
        WiFi.setSleep(true);

        Serial.println();
        Serial.println("Configuring access point...");

        // You can remove the password parameter if you want the AP to be open.
        // a valid password must have more than 7 characters
        if (!WiFi.softAP((const char *)settings.SSID, (const char *)settings.WPA2Key))
        {
            log_e("Soft AP creation failed.");
            while (1)
                ;
        }
        IPAddress myIP = WiFi.softAPIP();
        Serial.print("AP IP address: ");
        Serial.println(myIP);
        needServerInit = true;
    }
}

void WiFiManager::loop()
{
    ArduinoOTA.handle();

    int i;

    if (settings.enableBT != 0)
        return; // No wifi if BT is on

    if (wifiServer.hasClient())
    {
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (!SysSettings.clientNodes[i] || !SysSettings.clientNodes[i].connected())
            {
                if (SysSettings.clientNodes[i])
                    SysSettings.clientNodes[i].stop();
                SysSettings.clientNodes[i] = wifiServer.accept();
                if (!SysSettings.clientNodes[i])
                    Serial.println("Couldn't accept client connection!");
                else
                {
                    Serial.print("New client: ");
                    Serial.print(i);
                    Serial.print(' ');
                    Serial.println(SysSettings.clientNodes[i].remoteIP());
                }
            }
        }
        if (i >= MAX_CLIENTS)
        {
            // no free/disconnected spot so reject
            wifiServer.accept().stop();
        }
    }

    if (wifiOBDII.hasClient())
    {
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (!SysSettings.wifiOBDClients[i] || !SysSettings.wifiOBDClients[i].connected())
            {
                if (SysSettings.wifiOBDClients[i])
                    SysSettings.wifiOBDClients[i].stop();
                SysSettings.wifiOBDClients[i] = wifiOBDII.accept();
                if (!SysSettings.wifiOBDClients[i])
                    Serial.println("Couldn't accept client connection!");
                else
                {
                    Serial.print("New wifi ELM client: ");
                    Serial.print(i);
                    Serial.print(' ');
                    Serial.println(SysSettings.wifiOBDClients[i].remoteIP());
                }
            }
        }
        if (i >= MAX_CLIENTS)
        {
            // no free/disconnected spot so reject
            wifiOBDII.accept().stop();
        }
    }

    // check clients for data
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        if (SysSettings.clientNodes[i] && SysSettings.clientNodes[i].connected())
        {
            if (SysSettings.clientNodes[i].available())
            {
                // get data from the telnet client and push it to input processing
                while (SysSettings.clientNodes[i].available())
                {
                    uint8_t inByt;
                    inByt = SysSettings.clientNodes[i].read();
                    SysSettings.isWifiActive = true;
                    // Serial.write(inByt); //echo to serial - just for debugging. Don't leave this on!
                    wifiGVRET.processIncomingByte(inByt);
                }
            }
        }
        else
        {
            if (SysSettings.clientNodes[i])
            {
                SysSettings.clientNodes[i].stop();
            }
        }

        if (SysSettings.wifiOBDClients[i] && SysSettings.wifiOBDClients[i].connected())
        {
            elmEmulator.setWiFiClient(&SysSettings.wifiOBDClients[i]);
            /*if(SysSettings.wifiOBDClients[i].available())
            {
                //get data from the telnet client and push it to input processing
                while(SysSettings.wifiOBDClients[i].available())
                {
                    uint8_t inByt;
                    inByt = SysSettings.wifiOBDClients[i].read();
                    SysSettings.isWifiActive = true;
                    //wifiGVRET.processIncomingByte(inByt);
                }
            }*/
        }
    }

    if (SysSettings.isWifiConnected && ((micros() - lastBroadcast) > 1000000ul)) // every second send out a broadcast ping
    {
        uint8_t buff[4] = {0x1C, 0xEF, 0xAC, 0xED};
        lastBroadcast = micros();
        wifiUDPServer.beginPacket(broadcastAddr, 17222);
        wifiUDPServer.write(buff, 4);
        wifiUDPServer.endPacket();
    }
}

void WiFiManager::sendBufferedData()
{
    if (settings.enableBT != 0)
        return; // No wifi if BT is on
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        size_t wifiLength = wifiGVRET.numAvailableBytes();
        uint8_t *buff = wifiGVRET.getBufferedBytes();
        if (SysSettings.clientNodes[i] && SysSettings.clientNodes[i].connected())
        {
            SysSettings.clientNodes[i].write(buff, wifiLength);
        }
    }
    wifiGVRET.clearBufferedBytes();
}

// Utility to extract header value from headers
String getHeaderValue(String header, String headerName)
{
    return header.substring(strlen(headerName.c_str()));
}

void onOTAProgress(uint32_t progress, size_t fullSize)
{
    static int OTAcount = 0;
    // esp_task_wdt_reset();
    if (OTAcount++ == 10)
    {
        Serial.println(progress);
        OTAcount = 0;
    }
    else
    {
        Serial.print("...");
        Serial.print(progress);
    }
}

void WiFiManager::attemptOTAUpdate()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\n\n");
        Serial.print("*WIFI STATUS* SSID:");
        Serial.print(WiFi.SSID());
        Serial.print(" with IP Address: ");
        Serial.println(WiFi.localIP());
        Serial.println("Attempting the OTA update from remote host");
    }
    else
    {
        Serial.println("\n\nIt appears there is no wireless connection. Cannot update.");
        return;
    }

    int contentLength = 0;
    bool isValidContentType = false;
    // TODO: figure out where github stores files in releases and/or how and where the images will be stored.
    int port = 80; // Non https. HTTPS would be 443 but that might not work.
    String host = String(otaHost);
    String bin = String(otaFilename);
    // esp_task_wdt_reset(); //in case watchdog was set, update it.
    Update.onProgress(onOTAProgress);

    Serial.println("Connecting to OTA server: " + host);

    if (wifiClient.connect(otaHost, port))
    {
        wifiClient.print(String("GET ") + bin + " HTTP/1.1\r\n" +
                         "Host: " + String(host) + "\r\n" +
                         "Cache-Control: no-cache\r\n" +
                         "Connection: close\r\n\r\n"); // Get the contents of the bin file

        unsigned long timeout = millis();
        while (wifiClient.available() == 0)
        {
            if (millis() - timeout > 5000)
            {
                Serial.println("Timeout trying to connect! Aborting!");
                wifiClient.stop();
                return;
            }
        }

        while (wifiClient.available())
        {
            String line = wifiClient.readStringUntil('\n'); // read line till /n
            line.trim();                                    // remove space, to check if the line is end of headers

            // if the the line is empty,this is end of headers break the while and feed the
            // remaining `client` to the Update.writeStream();

            if (!line.length())
            {
                break;
            }

            // Check if the HTTP Response is 200 else break and Exit Update

            if (line.startsWith("HTTP/1.1"))
            {
                if (line.indexOf("200") < 0)
                {
                    Serial.println("FAIL...Got a non 200 status code from server. Exiting OTA Update.");
                    break;
                }
            }

            // extract headers here
            // Start with content length

            if (line.startsWith("Content-Length: "))
            {
                contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
                Serial.println("              ...Server indicates " + String(contentLength) + " byte file size\n");
            }

            if (line.startsWith("Content-Type: "))
            {
                String contentType = getHeaderValue(line, "Content-Type: ");
                Serial.println("\n              ...Server indicates correct " + contentType + " payload.\n");
                if (contentType == "application/octet-stream")
                {
                    isValidContentType = true;
                }
            }
        } // end while client available
    }
    else
    {
        // Connect to remote failed
        Serial.println("Connection to " + String(host) + " failed. Please check your setup!");
    }

    // Check what is the contentLength and if content type is `application/octet-stream`
    // Serial.println("File length: " + String(contentLength) + ", Valid Content Type flag:" + String(isValidContentType));

    // check contentLength and content type
    if (contentLength && isValidContentType) // Check if there is enough to OTA Update
    {
        bool canBegin = Update.begin(contentLength);
        if (canBegin)
        {
            Serial.println("There is sufficient space to update. Beginning update. \n");
            size_t written = Update.writeStream(wifiClient);

            if (written == contentLength)
            {
                Serial.println("\nWrote " + String(written) + " bytes to memory...");
            }
            else
            {
                Serial.println("\n********** FAILED - Wrote:" + String(written) + " of " + String(contentLength) + ". Try again later. ********\n\n");
                return;
            }

            if (Update.end())
            {
                //  Serial.println("OTA file transfer completed!");
                if (Update.isFinished())
                {
                    Serial.println("Rebooting new firmware...\n");
                    ESP.restart();
                }
                else
                    Serial.println("FAILED...update not finished? Something went wrong!");
            }
            else
            {
                Serial.println("Error Occurred. Error #: " + String(Update.getError()));
                return;
            }
        } // end if can begin
        else
        {
            // not enough space to begin OTA
            // Understand the partitions and space availability

            Serial.println("Not enough space to begin OTA");
            wifiClient.clear();
        }
    } // End contentLength && isValidContentType
    else
    {
        Serial.println("There was no content in the response");
        wifiClient.clear();
    }
}
