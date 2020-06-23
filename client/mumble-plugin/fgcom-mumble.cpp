// Copyright 2019-2020 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

// Include the definitions of the plugin functions
// Note that this will also include PluginComponents.h
#include "globalVars.h"
#include "MumblePlugin.h"
#include "MumbleAPI.h"
#include "fgcom-mumble.h"
#include "plugin_io.h"
#include "radio_model.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <string>
#include <thread>

#ifdef DEBUG
// include debug code
#include "debug.cpp"
#endif


// Mubmle API global vars.
MumbleAPI mumAPI;
mumble_connection_t activeConnection;
plugin_id_t ownPluginID;

// Plugin Version
#define FGCOM_VERSION_MAJOR 0
#define FGCOM_VERSION_MINOR 1
#define FGCOM_VERSION_PATCH 0


// Global plugin state
int  fgcom_specialChannelID = -1;
bool fgcom_inSpecialChannel = false; // adjust using fgcom_setPluginActive()!


/*******************
 * Some helpers    *
 ******************/

// Stream overload for version printing
std::ostream& operator<<(std::ostream& stream, const version_t version) {
	stream << "v" << version.major << "." << version.minor << "." << version.patch;
	return stream;
}


/*
 * Activate the plugin handling
 * 
 * @param bool active if the plugin handling stuff should be active
 */
transmission_mode_t fgcom_prevTransmissionMode = TM_VOICE_ACTIVATION; // we use voice act as default in case something goes wrong
void fgcom_setPluginActive(bool active) {
    mumble_error_t merr;
    fgcom_inSpecialChannel = active;
    if (active) {
        pluginLog("plugin handling activated: ");
        mumAPI.log(ownPluginID, "plugin handling activated");
        
        // switch to push-to-talk
        merr = mumAPI.getLocalUserTransmissionMode(ownPluginID, &fgcom_prevTransmissionMode);
        if (merr == STATUS_OK) {
            merr = mumAPI.requestLocalUserTransmissionMode(ownPluginID, TM_PUSH_TO_TALK);
            mumAPI.log(ownPluginID, "enabled push-to-talk");
        }
        
    } else {
        pluginLog("plugin handling deactivated");
        mumAPI.log(ownPluginID, "plugin handling deactivated");
        
        // restore old transmission mode
        merr = mumAPI.requestLocalUserTransmissionMode(ownPluginID, fgcom_prevTransmissionMode);
    }
    
}
bool fgcom_isPluginActive() {
    return fgcom_inSpecialChannel;
}


/*
 * Handle PTT change of local user
 * 
 * This will check the local radio state and activate the mic if all is operable.
 * When no PTT or no radio is operable, mic is closed.
 */
void fgcom_handlePTT() {
    if (fgcom_isPluginActive()) {
        pluginDbg("Handling PTT state");
        // see which radio was used and if its operational.
        bool radio_serviceable, radio_powered, radio_switchedOn, radio_ptt;
        bool radio_ptt_result = false; // if we should open or close the mic, default no
        if (fgcom_local_client.radios.size() > 0) {
            for (int i=0; i<fgcom_local_client.radios.size(); i++) {
                radio_serviceable = fgcom_local_client.radios[i].serviceable;
                radio_switchedOn  = fgcom_local_client.radios[i].power_btn;
                radio_powered     = (fgcom_local_client.radios[i].volts >= 1.0)? true:false; // some aircraft report boolean here, so treat every volts >=1 as "powered"
                radio_ptt         = fgcom_local_client.radios[i].ptt;
                
                if (radio_ptt) {
                    if (radio_serviceable && radio_switchedOn && radio_powered) {
                        pluginLog("  COM"+std::to_string(i+1)+" PTT active and radio is operable -> open mic");
                        radio_ptt_result = true;
                        break; // we only have one output stream, so further search makes no sense
                    } else {
                        pluginLog("  COM"+std::to_string(i+1)+" PTT active but radio not operable!");
                    }
                } else {
                    pluginDbg("  COM"+std::to_string(i+1)+" PTT off");
                }
            }
        }
        
        if (radio_ptt_result) pluginDbg("final PTT/radio openmic state: "+std::to_string(radio_ptt_result));
        mumAPI.requestMicrophoneActivationOvewrite(ownPluginID, radio_ptt_result);
        
    } else {
        // Todo: do we need to reset something or so? i think no:
        //       plugin deactivation will already handle setting the old transmission mode,
        //       so the mic will be open according to that...
        mumAPI.requestMicrophoneActivationOvewrite(ownPluginID, false);
    }
}


/*
 * To be called when plugin is initialized to set up
 * local stuff. the function gets called from
 *  - mumble_init()                  (plugin is loaded but not neccesarily connected to server)
 *  - mumble_onServerSynchronized()  (we are connected but plugin is not neccesarily loaded)
 *  - mumble_onServerConnected       (we are connected but plugin is not neccesarily loaded)
 */
bool fgcom_offlineInitDone = false;
bool fgcom_onlineInitDone = false;
std::thread::id udpServerThread_id;
mumble_error_t fgcom_initPlugin() {
    if (! fgcom_offlineInitDone && ! fgcom_onlineInitDone) mumAPI.log(ownPluginID, "Plugin initializing");
    
    /*
     * OFFLINE INIT: Here init stuff that can be initialized offline.
     * Do this just once.
     */     
    if (! fgcom_offlineInitDone) {
        pluginLog("performing offline initialization");
        
        #ifdef DEBUG
        // In Debug mode, start a detached thread that puts internal state to stdout every second
        std::thread debug_out_thread(debug_out_internal_state);
        debug_out_thread.detach();
        #endif
        
        // start the local udp server.
        pluginDbg("starting local UDP server");
        std::thread udpServerThread(fgcom_spawnUDPServer);
        udpServerThread_id = udpServerThread.get_id();
        udpServerThread.detach();
        //std::cout << "FGCOM: udp server started; id=" << udpServerThread_id << std::endl;
        pluginDbg("udp server started");
        
        fgcom_offlineInitDone = true;
    }
    
    
    /*
     * ONLINE INIT: Here do things that afford an established connection to the server
     */
    if (fgcom_isConnectedToServer()) {
        if (! fgcom_onlineInitDone) {
            pluginLog("performing online initialization");
            
            // fetch local user id from server, but only if we are already connected
            mumble_userid_t localUser;
            if (mumAPI.getLocalUserID(ownPluginID, activeConnection, &localUser) != STATUS_OK) {
                pluginLog("Failed to retrieve local user ID");
                return EC_USER_NOT_FOUND; // abort online init - something horribly went wrong.
            } else {
                fgcom_local_client.mumid = localUser; // store id to localUser
                pluginLog("got local clientID="+std::to_string(localUser));
                mumAPI.freeMemory(ownPluginID, &localUser);
            }
            
            
            // fetch all channels from server in order to get the special fgcom-mumble channel ID
            //fgcom_specialChannelID
            size_t channelCount;
            mumble_channelid_t *channels;
            if (mumAPI.getAllChannels(ownPluginID, activeConnection, &channels, &channelCount) != STATUS_OK) {
                pluginLog("Failed to retrieve all channel IDs");
                return EC_CHANNEL_NOT_FOUND; // abort online init - something horribly went wrong.
            } else {
                pluginLog("Server has "+std::to_string(channelCount)+" channels:");
                for (size_t ci=0; ci<channelCount; ci++) {
                    pluginDbg("  resolving channel name for id="+std::to_string(channels[ci]));
                    char *channelName;
                    mumble_error_t cfres = mumAPI.getChannelName(ownPluginID, activeConnection, channels[ci], &channelName);
                    if (cfres == STATUS_OK) {
                        pluginDbg("  channelID="+std::to_string(channels[ci])+" '"+channelName+"'");
                        if (strcmp("fgcom-mumble", channelName) == 0) {
                            fgcom_specialChannelID = channels[ci];
                            pluginDbg("    special channel id found! id="+std::to_string(fgcom_specialChannelID));
                            break;
                        }
                        mumAPI.freeMemory(ownPluginID, channelName);
                    } else {
                        pluginDbg("Error fetching channel names: rc="+std::to_string(cfres));
                        return EC_CHANNEL_NOT_FOUND; // abort online init - something horribly went wrong.
                    }
                }
                
                if (fgcom_specialChannelID == -1) {
                    pluginLog("ERROR: FAILED TO RETRIEVE 'fgcom-mumble' CHANNEL! Please setup such an channel.");
                    mumAPI.log(ownPluginID, std::string("Failed to retrieve 'fgcom-mumble' special channel! Please setup such an channel.").c_str());
                }
            }
            mumAPI.freeMemory(ownPluginID, channels);
            
            
            // In case we are already in the special channel, broadcast our state.
            // This is especially for the case when we did connect and join the channel without
            // active plugin and are activating it now.
            pluginDbg("Check if we are already in the special channel and thus need to activate");
            mumble_channelid_t localChannelID;
            mumble_error_t glcres = mumAPI.getChannelOfUser(ownPluginID, activeConnection, fgcom_local_client.mumid, &localChannelID);
            if (glcres == STATUS_OK) {
                if (fgcom_specialChannelID == localChannelID) {
                    pluginDbg("Already in special channel at init time");
                    fgcom_setPluginActive(true);
                    notifyRemotes(0);
                } else {
                    pluginDbg("Channels not equal: special="+std::to_string(fgcom_specialChannelID)+" == cur="+std::to_string(localChannelID));
                }
            } else {
                pluginLog("Error fetching current active channel: rc="+std::to_string(glcres));
                return EC_CHANNEL_NOT_FOUND; // abort online init - something horribly went wrong.
            }
            mumAPI.freeMemory(ownPluginID, &localChannelID);
            
            if (!fgcom_isPluginActive()) fgcom_setPluginActive(fgcom_isPluginActive()); // print some nice message to start
            
            // ... more needed?
            
            
            fgcom_onlineInitDone = true;
            
        }
        
    } else {
        pluginLog("fgcom_initPlugin(): not connected, so no online init possible (will try later)");
        return STATUS_OK; // OK - we will try later
    }
    
    
    // Plugin init complete
    return STATUS_OK;
}


//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////
//////////////////// PLUGIN IMPLEMENTATION ///////////////////
//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////


// Notiz: Loggen im mumble-Fenster: mumAPI.log(ownPluginID, "Received API functions");
// Notiz: Loggen ins terminal-log:  pluginLog("Registered Mumble's API functions");

/*  INIT Sequence
    Call mumble_setMumbleInfo (if implemented)
    Call mumble_getAPIVersion to get the required API version (respected in the next call)
    Call mumble_registerAPIFunctions
    Call mumble_init
*/


//////////////////////////////////////////////////////////////
//////////////////// OBLIGATORY FUNCTIONS ////////////////////
//////////////////////////////////////////////////////////////
// All of the following function must be implemented in order for Mumble to load the plugin
mumble_error_t mumble_init(uint32_t id) {
    pLog() << "Registered PluginID: " << id << std::endl;
	ownPluginID = id;
    
    // perform initialization if not done already (or missing parts of it;
    // this is called when loading the plugin which may be done offline)
    mumble_error_t init_rc = fgcom_initPlugin();
    if (STATUS_OK != init_rc) return init_rc;
	pluginLog("FGCOM: Initialized plugin");


	// STATUS_OK is a macro set to the appropriate status flag (ErrorCode)
	// If you need to return any other status have a look at the ErrorCode enum
	// inside PluginComponents.h and use one of its values
	return STATUS_OK;
}

void mumble_shutdown() {
	pluginLog("Shutdown plugin");

    // Let the UDP server shutdown itself
    fgcom_shutdownUDPServer();
    
    fgcom_setPluginActive(false); // stop plugin handling
    
	mumAPI.log(ownPluginID, "Plugin deactivated");
}

const char* mumble_getName() {
	// The pointer returned by this functions has to remain valid forever and it must be able to return
	// one even if the plugin hasn't loaded (yet). Thus it may not require any variables that are only set
	// once the plugin is initialized
	// For most cases returning a hard-coded String-literal should be what you aim for
	return "FGCom";
}

version_t mumble_getAPIVersion() {
	// MUMBLE_PLUGIN_API_VERSION will always contain the API version of the used header file (the one used to build
	// this plugin against). Thus you should always return that here in order to no have to worry about it.
	return MUMBLE_PLUGIN_API_VERSION;
}

void mumble_registerAPIFunctions(MumbleAPI api) {
	// In this function the plugin is presented with a struct of function pointers that can be used
	// to interact with Mumble. Thus you should store it somewhere safe for later usage.
    // This is called on plugin loading time, where we might not be connected.
	mumAPI = api;

	pluginLog("Registered Mumble's API functions");
}


//////////////////////////////////////////////////////////////
///////////////////// OPTIONAL FUNCTIONS /////////////////////
//////////////////////////////////////////////////////////////
// The implementation of below functions is optional. If you don't need them, don't include them in your
// plugin

void mumble_setMumbleInfo(version_t mumbleVersion, version_t mumbleAPIVersion, version_t minimalExpectedAPIVersion) {
	// this function will always be the first one to be called. Even before init()
	// In here you can get info about the Mumble version this plugin is about to run in.
	pLog() << "Mumble version: " << mumbleVersion << "; Mumble API-Version: " << mumbleAPIVersion << "; Minimal expected API-Version: "
		<< minimalExpectedAPIVersion << std::endl;
}

version_t mumble_getVersion() {
	// Mumble uses semantic versioning (see https://semver.org/)
	// { major, minor, patch }
	return { FGCOM_VERSION_MAJOR, FGCOM_VERSION_MINOR, FGCOM_VERSION_PATCH };
}

const char* mumble_getAuthor() {
	// For the returned pointer the same rules as for getName() apply
	// In short: in the vast majority of cases you'll want to return a hard-coded String-literal
	return "Benedikt Hallinger";
}

const char* mumble_getDescription() {
	// For the returned pointer the same rules as for getName() apply
	// In short: in the vast majority of cases you'll want to return a hard-coded String-literal
	return "FGCOM provides an aircraft radio simulation.";
}

uint32_t mumble_getFeatures() {
	// Tells Mumble whether this plugin delivers some known common functionality. See the PluginFeature enum in
	// PluginComponents.h for what is available.
	// If you want your plugin to deliver positional data, you'll want to return FEATURE_POSITIONAL
	//return FEATURE_NONE;
    return FEATURE_AUDIO;
}

uint32_t mumble_deactivateFeatures(uint32_t features) {
	pLog() << "Asked to deactivate feature set " << features << std::endl;

	// All features that can't be deactivated should be returned
	return features;
}


void mumble_onServerConnected(mumble_connection_t connection) {
    pLog() << "Established server-connection with ID " << connection << std::endl;
    
    // perform initialization if not done already (or missing parts of it;
    // particularly it will run the online part if the plugin was loaded when
    // we were not connected yet)
    activeConnection = connection;
    fgcom_initPlugin();    
    
}

void mumble_onServerDisconnected(mumble_connection_t connection) {
    pLog() << "Disconnected from server-connection with ID " << connection << std::endl;
    
    fgcom_setPluginActive(false);
    activeConnection = -1;
}

void mumble_onServerSynchronized(mumble_connection_t connection) {
	// The client has finished synchronizing with the server. Thus we can now obtain a list of all users on this server
    // This is only called if the module was loaded during connecting time.
    // Sync status can be tested with isConnectionSynchronized()
	pLog() << "Server has finished synchronizing (ServerConnection: " << connection << ")" << std::endl ;

	size_t userCount;
	mumble_userid_t *userIDs;

	if (mumAPI.getAllUsers(ownPluginID, activeConnection, &userIDs, &userCount) != STATUS_OK) {
		pluginLog("[ERROR]: Can't obtain user list");
		return;
	}

	pLog() << "There are " << userCount << " users on this server. Their names are:" << std::endl;

	for(size_t i=0; i<userCount; i++) {
		char *userName;
		mumAPI.getUserName(ownPluginID, connection, userIDs[i], &userName);
		
		pLog() << "\t" << userName << std::endl;

		mumAPI.freeMemory(ownPluginID, userName);
	}

	mumAPI.freeMemory(ownPluginID, userIDs);


    // perform initialization if not done already (or missing parts of it;
    // particularly it will run the online part if the plugin was loaded when
    // we were not connected/synchronized yet)
    fgcom_initPlugin();
}

void mumble_onChannelEntered(mumble_connection_t connection, mumble_userid_t userID, mumble_channelid_t previousChannelID, mumble_channelid_t newChannelID) {
    // Called for each user entering the channel. When newly entering the channel ourself, this gets called for every user.
    
	std::ostream& stream = pLog() << "User with ID " << userID << " entered channel with ID " << newChannelID << ".";

	// negative ID means that there was no previous channel (e.g. because the user just connected)
	if (previousChannelID >= 0) {
		stream << " He came from channel with ID " << previousChannelID << ".";
	}
	stream << " (ServerConnection: " << connection << ")" << std::endl;

    
    if (userID == fgcom_local_client.mumid) {
        stream << " OH! thats me! hello myself!";
        if (newChannelID == fgcom_specialChannelID) {
            pluginDbg("joined special channel, activating plugin functions");
            fgcom_setPluginActive(true);
        }
    } else {
        if (fgcom_isPluginActive()) {
            // if we are in the special channel, update new clinets with our state
            pluginDbg("send state to freshly joined user");
            notifyRemotes(0, -1, userID);
        }
    }

}

void mumble_onChannelExited(mumble_connection_t connection, mumble_userid_t userID, mumble_channelid_t channelID) {
	pLog() << "User with ID " << userID << " has left channel with ID " << channelID << ". (ServerConnection: " << connection << ")" << std::endl;
    
    //pluginDbg("userid="+std::to_string(userID)+"  mumid="+std::to_string(fgcom_local_client.mumid)+"  pluginActive="+std::to_string(fgcom_isPluginActive()));
    if (userID == fgcom_local_client.mumid && fgcom_isPluginActive()) {
        pluginDbg("left special channel, deactivating plugin functions");
        fgcom_setPluginActive(false);
    }
    
}

void mumble_onUserTalkingStateChanged(mumble_connection_t connection, mumble_userid_t userID, talking_state_t talkingState) {
	std::ostream& stream = pLog() << "User with ID " << userID << " changed his talking state to ";

	// The possible values are contained in the TalkingState enum inside PluginComponent.h
	switch(talkingState) {
		case INVALID:
			stream << "Invalid";
			break;
		case PASSIVE:
			stream << "Passive";
			break;
		case TALKING:
			stream << "Talking";
			break;
		case WHISPERING:
			stream << "Whispering";
			break;
		case SHOUTING:
			stream << "Shouting";
			break;
		default:
			stream << "Unknown (" << talkingState << ")";
	}

	stream << ". (ServerConnection: " << connection << ")" << std::endl;
    
}

// Note: Audio input is only possible with open mic. fgcom_hanldePTT() takes care of that.
bool mumble_onAudioInput(short *inputPCM, uint32_t sampleCount, uint16_t channelCount, bool isSpeech) {
	//pLog() << "Audio input with " << channelCount << " channels and " << sampleCount << " samples per channel encountered. IsSpeech: "
	//	<< isSpeech << std::endl;
    /*pluginDbg("  plugin active="+std::to_string(fgcom_isPluginActive()));
    if (fgcom_isPluginActive()) {
        // see which radio was used and if its operational. If not, null out the stream
        //bool activate = true;
        requestMicrophoneActivationOvewrite(ownPluginID, activate);
    }*/
    
    // Recheck that mic is open; close it immediately when radio fails.
    fgcom_handlePTT();
    
	// mark inputPCM as unused
	(void) inputPCM;

	// This function returns whether it has modified the audio stream
	return false;
}

bool mumble_onAudioSourceFetched(float *outputPCM, uint32_t sampleCount, uint16_t channelCount, bool isSpeech, mumble_userid_t userID) {
	/*std::ostream& stream = pLog() << "Audio output source with " << channelCount << " channels and " << sampleCount << " samples per channel fetched.";

	if (isSpeech) {
		stream << " The output is speech from user with ID " << userID << ".";
	}

	stream << std::endl;
    */
    
    /*pluginDbg("  plugin active="+std::to_string(fgcom_isPluginActive()));
    if (fgcom_isPluginActive()) {
        // TODO: check signal strength; if source is in range, let it trough
    } */

	// Mark ouputPCM as unused
	(void) outputPCM;

	// This function returns whether it has modified the audio stream
	return false;
}

/*  I think we don't need this and should implement stuff in the function above.
bool mumble_onAudioOutputAboutToPlay(float *outputPCM, uint32_t sampleCount, uint16_t channelCount) {
	//pLog() << "The resulting audio output has " << channelCount << " channels with " << sampleCount << " samples per channel" << std::endl;

	// mark outputPCM as unused
	(void) outputPCM;

	// This function returns whether it has modified the audio stream
	return false;
}*/

bool mumble_onReceiveData(mumble_connection_t connection, mumble_userid_t sender, const char *data, size_t dataLength, const char *dataID) {
	pLog() << "Received data with ID \"" << dataID << "\" from user with ID " << sender << ". Its length is " << dataLength
		<< ". (ServerConnection:" << connection << ")" << std::endl;

        if (dataLength > 0) {
            // if there is payload: handle it
            return handlePluginDataReceived(sender, std::string(dataID), std::string(data));
        }
        
        return false;
}

void mumble_onUserAdded(mumble_connection_t connection, mumble_userid_t userID) {
    /// Called when a new user gets added to the user model. This is the case when that new user freshly connects to the server the
	/// local user is on but also when the local user connects to a server other clients are already connected to (in this case this
	/// method will be called for every client already on that server).
	pLog() << "Added user with ID " << userID << " (ServerConnection: " << connection << ")" << std::endl;
}

void mumble_onUserRemoved(mumble_connection_t connection, mumble_userid_t userID) {
	pLog() << "Removed user with ID " << userID << " (ServerConnection: " << connection << ")" << std::endl;
}

void mumble_onChannelAdded(mumble_connection_t connection, mumble_channelid_t channelID) {
	pLog() << "Added channel with ID " << channelID << " (ServerConnection: " << connection << ")" << std::endl;
}

void mumble_onChannelRemoved(mumble_connection_t connection, mumble_channelid_t channelID) {
	pLog() << "Removed channel with ID " << channelID << " (ServerConnection: " << connection << ")" << std::endl;
}

void mumble_onChannelRenamed(mumble_connection_t connection, mumble_channelid_t channelID) {
	pLog() << "Renamed channel with ID " << channelID << " (ServerConnection: " << connection << ")" << std::endl;
}

void mumble_onKeyEvent(uint32_t keyCode, bool wasPress) {
	pLog() << "Encountered key " << (wasPress ? "press" : "release") << " of key with code " << keyCode << std::endl;
}

bool mumble_hasUpdate() {
	// This plugin never has an update
	return false;
}

bool mumble_getUpdateDownloadURL(char *buffer, uint16_t bufferSize, uint16_t offset) {
	/*static std::string url = "https://i.dont.exist/testplugin.zip";

	size_t writtenChars = url.copy(buffer, bufferSize, offset);

	if (writtenChars < bufferSize) {
		// URL has fit into the buffer -> append null byte and be done with it
		buffer[writtenChars] = '\0';
		return true;
	} else {
		std::cout << "Overflow" << std::endl;
		return false;
	}*/
    return false;
}
