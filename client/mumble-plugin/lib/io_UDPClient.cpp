/* 
 * This file is part of the FGCom-mumble distribution (https://github.com/hbeni/fgcom-mumble).
 * Copyright (c) 2020 Benedikt Hallinger
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// UDP client
//
// Simple UDP client sending RDF packets.
// The RDF client must be configured and started trough special 
// settings to the UDP server.

#include <iostream>
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sstream> 
#include <regex>
#include <sys/types.h> 

#ifdef MINGW_WIN64
    #include <winsock2.h>
    //#include <windows.h>
    //#include <ws2tcpip.h>
    typedef int socklen_t;
#else
    #include <sys/socket.h> 
    #include <arpa/inet.h> 
    #include <netinet/in.h>
#endif

#include <thread>
#include <mutex>
#include <vector>
#include <set>
#include <map>
#include <clocale> // setlocale() 

#include "globalVars.h"
#include "io_plugin.h"
#include "fgcom-mumble.h"
#include "io_UDPClient.h"

    
#define FGCOM_UDPCLIENT_RATE 10       // datarate in packets/seconds

/*****************************************************
 *                   UDP Client                      *
 * The UDP interface is the plugins port to send     *
 * RDF data to the outside world.                    *
 * It is used for example from ATC clients or        *
 * FlightSims to detect radio transmissions (RDF).   *
 ****************************************************/


/*
 * Register new signal data
 */
std::mutex fgcom_rdfInfo_mtx;
std::map<std::string, fgcom_rdfInfo> fgcom_rdf_activeSignals;
void fgcom_rdf_registerSignal(std::string rdfID, fgcom_rdfInfo rdfInfo) {
    fgcom_rdfInfo_mtx.lock();
    fgcom_rdf_activeSignals[rdfID] = rdfInfo;
    fgcom_rdfInfo_mtx.unlock();
}


/*
 * Generates a message from current radio state.
 * For that we inspect all recorded RDF info of this iteration.
 */
std::string fgcom_rdf_generateMsg(uint16_t selectedPort) {
    std::setlocale(LC_NUMERIC,"C"); // decial points always ".", not ","
    fgcom_rdfInfo_mtx.lock();
    
    std::vector<std::string> processed;
    
    // generate message string
    std::string clientMsg;
    for (const auto &rdf : fgcom_rdf_activeSignals) { // inspect all identites of the local client
        std::string rdfID     = rdf.first;
        fgcom_rdfInfo rdfInfo = rdf.second;
        if (rdfInfo.signal.quality > 0.0 && rdfInfo.rxIdentity.clientPort == selectedPort) {
            clientMsg += "RDF:";
            clientMsg += "CS_TX="+rdfInfo.txIdentity.callsign;
            //clientMsg += ",CS_RX="+rdfInfo.rxIdentity.callsign;
            clientMsg += ",FRQ="+rdfInfo.txRadio.frequency;
            clientMsg += ",DIR="+std::to_string(rdfInfo.signal.direction);
            clientMsg += ",VRT="+std::to_string(rdfInfo.signal.verticalAngle);
            clientMsg += ",QLY="+std::to_string(rdfInfo.signal.quality);
            clientMsg += "\n";
            processed.push_back(rdfID);
        }
    }
    
    // clear up
    for(const auto &elem : processed) {
        fgcom_rdf_activeSignals.erase(elem);
    }

    

    // Finally return data
    //pluginDbg("[UDP] client fgcom_udp_generateMsg(): data buld finished, length="+std::to_string(clientMsg.length())+", content="+clientMsg);
    fgcom_rdfInfo_mtx.unlock();
    return clientMsg;
}


// Spawns the UDP client thread
bool udpClientRunning = false;
std::map<int, uint16_t> fgcom_cliudp_portCfg;
void fgcom_spawnUDPClient() {
    pluginLog("[UDP-client] client initializing at rate "+std::to_string(FGCOM_UDPCLIENT_RATE)+" pakets/s");
    const float packetrate = FGCOM_UDPCLIENT_RATE;
    const int datarate = 1000000 * (1/packetrate);  //datarate in seconds*microseconds
    int fgcom_UDPClient_sockfd, rc;
    uint16_t port;  
    struct sockaddr_in cliAddr, remoteServAddr;
    bool portEstablished = false;

    // Prepare addressing
    remoteServAddr.sin_family = AF_INET;
    remoteServAddr.sin_port = htons(port);
#ifdef MINGW_WIN64
    remoteServAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);    // 127.0.0.1 on purpose: don't change for securites sake
#else
	inet_pton(AF_INET, "localhost", &remoteServAddr.sin_addr);  // 127.0.0.1 on purpose: don't change for securites sake
#endif
    //bzero(&(remoteServAddr.sin_zero), 8);     /* zero the rest of the struct */
    
    // Creating socket file descriptor 
    if ( (fgcom_UDPClient_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        pluginLog("[UDP-client] client ERROR: socket creation failed");
        return;
    }
    
    // bind client source port
    cliAddr.sin_family = AF_INET;
    cliAddr.sin_port = htons(0);
    cliAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // restricted to 127.0.0.1 on purpose: don't change
    rc = bind ( fgcom_UDPClient_sockfd, (struct sockaddr *) &cliAddr, sizeof(cliAddr) );
    if (rc < 0) {
        pluginLog("[UDP-client] client ERROR: local source port binding failed");
        return;
    }

    udpClientRunning = true;
    
    // Generate Data packets and send them in a loop
    while (true) {
        
        // sleep for datarate-time microseconds
        //pluginDbg("[UDP-client] client sleep for "+std::to_string(datarate)+" microseconds");
        if(usleep(datarate) < 0) {
            pluginLog("[UDP-client] client socket creation failed"); 
            break;
        }
        
        // evaluate all local identites and generate messages to their configured ports
        for (const auto &lcl_idty : fgcom_local_client) {
            int iid          = lcl_idty.first;
            fgcom_client lcl = lcl_idty.second;
            
            // fetch+configure port for that identities current port config
            port = lcl.clientPort;
            remoteServAddr.sin_port = htons(port);
            if (port <=0) {
                pluginDbg("[UDP-client] client sending skipped: no valid port config for identity="+std::to_string(iid));
                continue;
            }
            
            // Report if the identities port changed
            if (fgcom_cliudp_portCfg[iid] == 0 || fgcom_cliudp_portCfg[iid] != port) {
                pluginLog("[UDP-client] client for '"+lcl.callsign+"' (iid="+std::to_string(iid)+") port="+std::to_string(fgcom_cliudp_portCfg[iid])+" switching to port "+std::to_string(port));
                mumAPI.log(ownPluginID, std::string("UDP sending for '"+lcl.callsign+"' to port "+std::to_string(port)+" enabled").c_str());
                fgcom_cliudp_portCfg[iid] = port;
            }
        
            // generate data.
            std::string udpdata = fgcom_rdf_generateMsg(port);
            
            // send data
            if (udpdata.length() > 0) {
                // If there was data generated, add a FGCOM header
                if (udpdata.length() > 0) udpdata = "FGCOM v" 
                        + std::to_string(FGCOM_VERSION_MAJOR) + "."
                        + std::to_string(FGCOM_VERSION_MINOR) + "."
                        + std::to_string(FGCOM_VERSION_PATCH) 
                        + "\n"
                        + udpdata;
    
                pluginDbg("[UDP-client] client sending msg for iid="+std::to_string(iid)+" to port="+std::to_string(port)+": '"+udpdata+"'");
                rc = sendto (fgcom_UDPClient_sockfd, udpdata.c_str(), strlen(udpdata.c_str()) + 1, 0,
                    (struct sockaddr *) &remoteServAddr,
                    sizeof (remoteServAddr));
                if (rc < 0) {
                    pluginLog("[UDP-client] client ERROR sending "+std::to_string(strlen(udpdata.c_str()))+" bytes of data");
                    close (fgcom_UDPClient_sockfd);
                    return;
                }
                
                pluginDbg("[UDP-client] client sending OK ("+std::to_string(strlen(udpdata.c_str()))+" bytes of data)");
                
            } else {
                // no data generated: do not send anything.
                pluginDbg("[UDP-client] client sending skipped: no data to send for iid="+std::to_string(iid)+" to port="+std::to_string(port)+".");
            }
        }
        
    }
    
    
    close(fgcom_UDPClient_sockfd);
    udpClientRunning = false;
    pluginLog("[UDP-client] client finished.");
}
