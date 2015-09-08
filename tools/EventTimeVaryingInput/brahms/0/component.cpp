

/*
   Autogenerated BRAHMS process from 9ML description.
   Engine: XSLT
   Engine Author: Alex Cope 2012
   Node name:
*/


#define COMPONENT_CLASS_STRING "dev/SpineML/tools/EventTimeVaryingInput"
#define COMPONENT_CLASS_CPP dev_spineml_tools_eventtimevaryinginput_0
#define COMPONENT_RELEASE 0
#define COMPONENT_REVISION 1
#define COMPONENT_ADDITIONAL "Author=SpineML_2_BRAHMS\n" "URL=Not supplied\n"
#define COMPONENT_FLAGS (F_NOT_RATE_CHANGER)

#define OVERLAY_QUICKSTART_PROCESS

//	include the component interface overlay (component bindings 1199)
#include "brahms-1199.h"

#include "rng.h"

//	alias data and util namespaces to something briefer
namespace numeric = std_2009_data_numeric_0;
namespace spikes = std_2009_data_spikes_0;
namespace rng = std_2009_util_rng_0;

using namespace std;

class COMPONENT_CLASS_CPP;

#include <sstream>

string stringify(int val) {
    stringstream ss (stringstream::in | stringstream::out);
    ss << float(val);
    string returnVal = ss.str().c_str();
    return returnVal;
}

enum rateType {
	Poisson,
	Regular
};

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

RngData rngData;

struct timePoint {
	float time;
	float value;
};

// Analog Ports

spikes::Output out;

int size;

VDOUBLE values;

VDOUBLE nextSpike;

vector < vector < timePoint > > timePointsArray;

vector < float > logT;
vector < int > logIndex;
vector < int > logMap;
FILE * logFile;

string baseNameForLogs;

bool logOn;
bool logAll;

rateType type;

float dt;

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

                        rngDataInit (&this->rngData);
                        this->rngData.seed = 123;
                        // No need to zigset(), as we only use UNI() here.

			// obtain the parameters
			if (nodeState.hasField("values")) {
				values = nodeState.getField("values").getArrayDOUBLE();
				Dims dims = nodeState.getField("values").getDims();

				vector < timePoint > newVTP;

				for (UINT32 i = 0; i < values.size(); i+=2) {
					timePoint newTP;
					newTP.time = values[i];
					newTP.value = values[i+1];
					newVTP.push_back(newTP);
				}

				timePointsArray.push_back(newVTP);

			} else {

				int i = 0;
				string name = "values" + stringify(i);
				while (nodeState.hasField(name.c_str())) {

					values = nodeState.getField(name.c_str()).getArrayDOUBLE();

					vector < timePoint > newVTP;

					for (UINT32 j = 0; j < values.size()/2; ++j) {
						timePoint newTP;
						newTP.time = values[j];
						newTP.value = values[j+values.size()/2];
						newVTP.push_back(newTP);
					}

					timePointsArray.push_back(newVTP);
					++i;
					name = "values" + stringify(i);
				}

			}

			size = nodeState.getField("size").getUINT32();

			// fill in blanks
			if (timePointsArray.size() != 1) {
			  if (timePointsArray.size() < (unsigned int)size) {
					// add some blank timeseries
					vector < timePoint > newVTP;
					timePointsArray.resize(size, newVTP);
				} else {
					// bad data
					berr << "Error: incorrect number of values for Event Time Varying Input";
				}
			}

			// resize values for use
			values.clear();
			values.resize(timePointsArray.size(),0);

			if (!(values.size() == 1 || values.size() == (unsigned int)size)) {
				// bad data
				berr << "Error: incorrect number of values for Event Time Varying Input";
			}

			type = (rateType) nodeState.getField("rateType").getUINT32();

			// Log base name
			baseNameForLogs = nodeState.getField("logfileNameForComponent").getSTRING();

			logOn = false;

				// check for logs
			if (nodeState.hasField("spikeLOG")) {
				logOn = true;
				// we have a log! Read the data in:
				VDOUBLE tempLogData = nodeState.getField("spikeLOG").getArrayDOUBLE();
				// logmap resize
				logMap.resize(size,-1);
				// set the logmap values - checking for out of range values
				for (unsigned int i = 0; i < tempLogData.size(); ++i) {
					if (tempLogData[i]+0.5 >size) {
						bout << "Attempting to log an index out of range" << D_WARN;
					} else {
						// set in mapping that the ith log value relates to the tempLogData[i]th neuron
						logMap[(int) tempLogData[i]] = i;
					}
				}
				// open the logfile for writing
				string logFileName = baseNameForLogs;
				logFileName.append("_spike_log.csv");
				logFile = fopen(logFileName.c_str(),"w");
			}

			if (nodeState.hasField("spikeLOGALL")) {
				logAll = true;
				logOn = true;

				// open the logfile for writing
				string logFileName = baseNameForLogs;
				logFileName.append("_spike_log.csv");
				logFile = fopen(logFileName.c_str(),"w");
			} else {
				logAll = false;
			}


			nextSpike.resize(size);

			// set initial spike time
			if (values.size() == 1) {
			  for (UINT32 i = 0; i < (UINT32)size; ++i) {
					if (type == Poisson) {
                                            nextSpike[i] =  1000.0*log(1.0-UNI(&this->rngData))/-values[0];
					} else if (type == Regular) {
						nextSpike[i] = 1000.0/values[0];
					}
				}
			} else {
				for (UINT32 i = 0; i < values.size(); ++i) {
					if (type == Poisson) {
						nextSpike[i] = 1000.0*log(1.0-UNI(&this->rngData))/-values[i];
					} else if (type == Regular) {
						nextSpike[i] = 1000.0/values[i];
					}
				}
			}

			dt = 1000.0f * time->sampleRate.den / time->sampleRate.num; // time step in ms

			return C_OK;


		}

		// CREATE THE PORTS
		case EVENT_INIT_CONNECT:
		{

			//	on first call
			if (event->flags & F_FIRST_CALL)
			{

				out.setName("out");
				out.create(hComponent);
				out.setCapacity(size);

			}

			//	on last call
			if (event->flags & F_LAST_CALL)
			{
			}

			//	ok
			return C_OK;
		}

		case EVENT_RUN_SERVICE:
		{

			float t = float(time->now)*dt;

			for (UINT32 i = 0; i < timePointsArray.size(); ++i) {

				for (UINT32 j = 0; j < timePointsArray[i].size(); ++j) {
					if (timePointsArray[i][j].time <= t) {
						timePointsArray[i][j].time = INFINITY;
						values[i] = timePointsArray[i][j].value;

						// reset spiking rate
						if (values.size()==1) {
							// reset all
							for (unsigned int k = 0; k < nextSpike.size(); ++k) {
								nextSpike[k] = INFINITY;
							}
						} else {
							// reset changing index
							nextSpike[i] = INFINITY;
						}
					}
				}
			}

			vector < int > spikes;

			if (values.size() == 1) {
			  for (UINT32 i = 0; i < (UINT32)size; ++i) {
				// restart spikes after quiescence
					if (nextSpike[i] == INFINITY) {
						if (values[0] > 0) {
							if (type == Poisson) {
								nextSpike[i] = t + 1000.0*log(1.0-UNI(&this->rngData))/-values[0];
							} else if (type == Regular) {
								nextSpike[i] = t + 1000.0/values[0];
							}
						}
					}
					// normal spike generation
					else if (nextSpike[i] <= t) {
						spikes.push_back(i);
						if (logOn) {
							if (logAll || logMap[i] != -1) {
								logT.push_back(t);
								logIndex.push_back(i);
							}
						}
						if (values[0] > 0) {
							if (type == Poisson) {
								nextSpike[i] = t + 1000.0*log(1.0-UNI(&this->rngData))/-values[0];
							} else if (type == Regular) {
								nextSpike[i] = t + 1000.0/values[0];
							}
						} else {
							nextSpike[i] = INFINITY;
						}
					}
				}
			} else {
			  for (UINT32 i = 0; i < (UINT32)size; ++i) {
				// restart spikes after quiescence
					if (nextSpike[i] == INFINITY) {
						if (values[i] > 0) {
							if (type == Poisson) {
								nextSpike[i] = t + 1000.0*log(1.0-UNI(&this->rngData))/-values[i];
							} else if (type == Regular) {
								nextSpike[i] = t + 1000.0/values[i];
							}
						}
					}
					// normal spike generation
					else if (nextSpike[i] <= t) {
						spikes.push_back(i);
						if (logOn) {
							if (logAll || logMap[i] != -1) {
								logT.push_back(t);
								logIndex.push_back(i);
							}
						}
						if (values[i] > 0) {
							if (type == Poisson) {
								nextSpike[i] = t + 1000.0*log(1.0-UNI(&this->rngData))/-values[i];
							} else if (type == Regular) {
								nextSpike[i] = t + 1000.0/values[i];
							}
						} else {
							nextSpike[i] = INFINITY;
						}
					}
				}
			}


			if (logOn && logIndex.size() > 100000) {
				for (unsigned int i = 0; i < logIndex.size(); i++) {
					fprintf(logFile, "%f, %d\n", logT[i],logIndex[i]);
				}
				logT.clear();
				logIndex.clear();
			}

			out.setContent(&(spikes[0]), spikes.size());

			//	ok
			return C_OK;
		}

		case EVENT_RUN_STOP:
		{
			float t = float(time->now)*dt;
			if (logOn) {
				for (unsigned int i = 0; i < logIndex.size(); i++) {
					fprintf(logFile, "%f, %d\n", logT[i],logIndex[i]);
				}
				FILE * logRep;
				string logFileName = baseNameForLogs;
				logFileName.append("_spike_logrep.xml");
				logRep = fopen(logFileName.c_str(),"w");
				logFileName = baseNameForLogs;
				logFileName.append("_spike_log.csv");
				fprintf(logRep, "<LogReport>\n");
				fprintf(logRep, "	<EventLog>\n");
				fprintf(logRep, "		<LogFile>%s</LogFile>\n",logFileName.c_str());
				fprintf(logRep, "		<LogFileType>csv</LogFileType>\n");
				fprintf(logRep, "		<LogPort>spike</LogPort>\n");
				fprintf(logRep, "		<LogEndTime>%f</LogEndTime>\n",t);
				if (!logAll) {
					for (unsigned int i = 0; i < logMap.size(); ++i) {
						if (logMap[i] > -0.1) {
							fprintf(logRep,"		<LogIndex>%d</LogIndex>\n",i);
						}
					}
				} else {
					fprintf(logRep, "		<LogAll size=\"%d\" type=\"int\" dims=\"\"/>\n",size);
				}
				fprintf(logRep,"		<LogCol heading=\"t\" dims=\"ms\" type=\"double\"/>\n");
				fprintf(logRep,"		<LogCol heading=\"index\" dims=\"\" type=\"int\"/>\n");
				fprintf(logRep, "	</EventLog>\n");
				fprintf(logRep, "</LogReport>\n");

				fclose(logRep);
				fclose(logFile);
			}

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
