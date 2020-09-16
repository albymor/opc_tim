#include "open62541.h"
#include "measures_lib.h"

#define N_MEASURES 100000

int timestamp_to_integer(UA_DateTime item){ //vale solo su open62541
	UA_DateTimeStruct dts = UA_DateTime_toStruct(item);
	return dts.milliSec + 1000 * dts.sec + 60000 * dts.min + dts.hour * 1440000;
}

int main(int argc, char *argv[]) {
    UA_Client *client = UA_Client_new(UA_ClientConfig_default);
    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://192.168.2.3:4840/");
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return (int)retval;
    }
	
	UA_ReadRequest request;
	UA_ReadRequest_init(&request);
	UA_ReadValueId ids[1];
	UA_ReadValueId_init(&ids[0]);
	ids[0].attributeId = UA_ATTRIBUTEID_VALUE;
	ids[0].nodeId = UA_NODEID_NUMERIC(1, 200);
	request.nodesToRead = ids;
	request.nodesToReadSize = 1;
	
	long timeVector[N_MEASURES];
	long time_us_initial;
	long time_us_final;
	int i;
	for(i = 0; i < N_MEASURES; i++){
		if(i%1000 == 0){
		    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "%d", i);
		}
		time_us_initial = GetTimeStamp();
		UA_ReadResponse response = UA_Client_Service_read(client, request);
		UA_Int32 value = *(UA_Int32*)response.results->value.data;
		UA_DateTime timestamp = response.responseHeader.timestamp;			
		time_us_final = GetTimeStamp();
		timeVector[i] = time_us_final - time_us_initial;
	}
	
    UA_Client_delete(client); /* Disconnects the client internally */
    return UA_STATUSCODE_GOOD;
}


