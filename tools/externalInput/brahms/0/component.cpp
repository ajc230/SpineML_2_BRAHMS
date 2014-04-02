

/*
   Autogenerated BRAHMS process from 9ML description.
   Engine: XSLT
   Engine Author: Alex Cope 2012
   Node name: 
*/


#define COMPONENT_CLASS_STRING "dev/SpineML/tools/externalInput"
#define COMPONENT_CLASS_CPP dev_spineml_tools_externalInput_0
#define COMPONENT_RELEASE 0
#define COMPONENT_REVISION 1
#define COMPONENT_ADDITIONAL "Author=SpineML_2_BRAHMS\n" "URL=Not supplied\n"
#define COMPONENT_FLAGS (F_NOT_RATE_CHANGER)

#define OVERLAY_QUICKSTART_PROCESS

//	include the component interface overlay (component bindings 1199)
#include "brahms-1199.h"

//	alias data and util namespaces to something briefer
namespace numeric = std_2009_data_numeric_0;
namespace spikes = std_2009_data_spikes_0;
namespace rng = std_2009_util_rng_0;

using namespace std;

#include "impulse.h"

// for Linux sockets
#ifdef __WIN__

#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

class COMPONENT_CLASS_CPP;

// the network client class
#include "../../../client.h"

////////////////	COMPONENT CLASS (DERIVES FROM Process)

class COMPONENT_CLASS_CPP : public Process
{

public:

	//	use ctor/dtor only if required
	COMPONENT_CLASS_CPP() {}
	~COMPONENT_CLASS_CPP() {}

	//	the framework event function
	Symbol event(Event* event);

private:


// define ints to send


// Ports

numeric::Output out;

spikes::Output outs;

spikes::Output outi;

int numElementsIn;
int numElementsOut;
    
int size;
string server;

spineMLNetworkClient client;

    int portno;
    
    char ack;
    char joinack;
    char exitack;
    
    dataTypes dataType;
    
    VDOUBLE buffer;

};

////////////////	EVENT

Symbol COMPONENT_CLASS_CPP::event(Event* event)
{
	switch(event->type)
	{
		case EVENT_STATE_SET:
		{
		
			//	extract DataML
			EventStateSet* data = (EventStateSet*) event->data;
			XMLNode xmlNode(data->state);
			DataMLNode nodeState(&xmlNode);
			
			// obtain the parameters
            size = nodeState.getField("size").getINT32();
            numElementsOut = size;

			portno = nodeState.getField("port").getINT32();
			
			// get the server name
			if (nodeState.hasField("server")) {
				server = nodeState.getField("server").getSTRING();
			} else {
				server = "localhost";
			}
            
            /*string command = nodeState.getField("command").getSTRING();
            
            // launch the command if it is not blank
            if (!command.compare("")) {
                // use fork to run in new thread
                system(command.c_str());
            }*/
            
			
			// scale buffer	
			buffer.resize(size,0);
			
			return C_OK;


		}

		// CREATE THE PORTS
		case EVENT_INIT_CONNECT:
		{
            if (event->flags & F_FIRST_CALL)
			{
                switch (dataType) {
                case EVENT:
                    outs.setName("out");
                    outs.create(hComponent);
                    outs.setCapacity(numElementsOut);
                    break;
                case ANALOG:
                    out.setName("out");
                    out.create(hComponent);
                    out.setStructure(TYPE_DOUBLE | TYPE_REAL, Dims(numElementsOut).cdims());
                    break;
                case IMPULSE:
                	{berr << "Not implemented";}
                	break;    
                }
            }

			//	on last call
			if (event->flags & F_LAST_CALL)
			{
			
				// connect the socket:
			
				// start client
				if (!client.createClient(server, portno, size, dataType, RESP_AM_TARGET)) {
					berr << client.getLastError();
				}
				
				/*client.connectClient(portno);
			
				// handshake
				client.handShake(RESP_AM_TARGET);
			
				// send data type
				bool ok;
				dataType = client.recvDataType(ok);
			
				// get size
				int confirmSize = client.recvSize(ok);
				if (size != confirmSize)
					berr << "Wrong size of data coming into External Source: " << double(confirmSize) << " should be: " << double(size);
				*/
			}

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{
            
			/*if (time->now > 0) {
				client.sendContinue();
			}*/
            
            // only analog for now - will add spikes etc.. later
            
            switch (dataType) {
                case EVENT:
                    {berr << "Not implemented";}
                    break;
                case ANALOG:
				    client.recvData((char *) &(buffer[0]), buffer.size()*sizeof(double));
					out.setContent(&(buffer[0]));
                    break;
                case IMPULSE:
                	{berr << "Not implemented";}
                	break;    
            }

						
			//	ok
			return C_OK;
		}
		
		case EVENT_RUN_STOP:
		{

			//client.sendEnd();

			client.disconnectClient();		    
		    
			//	ok
			return C_OK;
		}

	}

	//	if we service the event, we return C_OK
	//	if we don't, we should return S_NULL to indicate that we didn't
	return S_NULL;
}







//	include the second part of the overlay (it knows you've included it once already)
#include "brahms-1199.h"


