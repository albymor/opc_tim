/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>
#include <open62541/plugin/log_stdout.h>

#include <stdint.h>
#include <stdlib.h>
#include <wiringPi.h>

static volatile uint32_t counter = 0;

#ifdef UA_ENABLE_SUBSCRIPTIONS
static void
handler_TheAnswerChanged(UA_Client *client, UA_UInt32 subId, void *subContext,
                         UA_UInt32 monId, void *monContext, UA_DataValue *value) {
    if (counter % 100 == 0)
    {
        printf ("%d \n", counter);
    }
    counter++;
    static int edgeTriggerMeasure = 0;

    if(edgeTriggerMeasure) {
        digitalWrite(0, HIGH);
        edgeTriggerMeasure = 0;
    } else {
        digitalWrite(0, LOW);
        edgeTriggerMeasure = 1;
    }
}
#endif

static UA_StatusCode
nodeIter(UA_NodeId childId, UA_Boolean isInverse, UA_NodeId referenceTypeId, void *handle) {
    if(isInverse)
        return UA_STATUSCODE_GOOD;
    UA_NodeId *parent = (UA_NodeId *)handle;
    printf("%d, %d --- %d ---> NodeId %d, %d\n",
           parent->namespaceIndex, parent->identifier.numeric,
           referenceTypeId.identifier.numeric, childId.namespaceIndex,
           childId.identifier.numeric);
    return UA_STATUSCODE_GOOD;
}

int main(int argc, char *argv[]) {
    UA_Client *client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));

    wiringPiSetup();
    pinMode(0, OUTPUT);

    /* Listing endpoints */
    UA_EndpointDescription* endpointArray = NULL;
    size_t endpointArraySize = 0;
//    UA_StatusCode retval = UA_Client_getEndpoints(client, "opc.tcp://localhost:4840", &endpointArraySize, &endpointArray);
    UA_StatusCode retval = UA_Client_getEndpoints(client, "opc.tcp://192.168.1.185:4840", &endpointArraySize, &endpointArray);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }
    printf("%i endpoints found\n", (int)endpointArraySize);
    for(size_t i=0;i<endpointArraySize;i++) {
        printf("URL of endpoint %i is %.*s\n", (int)i,
               (int)endpointArray[i].endpointUrl.length,
               endpointArray[i].endpointUrl.data);
    }
    UA_Array_delete(endpointArray,endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);

    /* Connect to a server */
    /* anonymous connect would be: retval = UA_Client_connect(client, "opc.tcp://localhost:4840"); */
//    retval = UA_Client_connectUsername(client, "opc.tcp://localhost:4840", "user1", "password");
    retval = UA_Client_connectUsername(client, "opc.tcp://192.168.1.185:4840", "user1", "password");
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return EXIT_FAILURE;
    }

#ifdef UA_ENABLE_SUBSCRIPTIONS
    /* Create a subscription */
    UA_CreateSubscriptionRequest request = UA_CreateSubscriptionRequest_default();
    request.requestedPublishingInterval = 100.0;
    UA_CreateSubscriptionResponse response = UA_Client_Subscriptions_create(client, request,
                                                                            NULL, NULL, NULL);

    UA_UInt32 subId = response.subscriptionId;
    if(response.responseHeader.serviceResult == UA_STATUSCODE_GOOD)
        printf("Create subscription succeeded, id %u\n", subId);

    UA_MonitoredItemCreateRequest monRequest =
        UA_MonitoredItemCreateRequest_default(UA_NODEID_STRING(1, "the.answer"));

    monRequest.requestedParameters.samplingInterval = 100;

    UA_MonitoredItemCreateResult monResponse =
    UA_Client_MonitoredItems_createDataChange(client, response.subscriptionId,
                                              UA_TIMESTAMPSTORETURN_BOTH,
                                              monRequest, NULL, handler_TheAnswerChanged, NULL);
    if(monResponse.statusCode == UA_STATUSCODE_GOOD)
        printf("Monitoring 'the.answer', id %u\n", monResponse.monitoredItemId);


    /* The first publish request should return the initial value of the variable */
    UA_Client_run_iterate(client, 1000);
#endif

while(counter < 100000){

    UA_Client_run_iterate(client, 1000);
    usleep(100);
}

    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return EXIT_SUCCESS;
}
