/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

/**
 * Adding Variables to a Server
 * ----------------------------
 *
 * This tutorial shows how to work with data types and how to add variable nodes
 * to a server. First, we add a new variable to the server. Take a look at the
 * definition of the ``UA_VariableAttributes`` structure to see the list of all
 * attributes defined for VariableNodes.
 *
 * Note that the default settings have the AccessLevel of the variable value as
 * read only. See below for making the variable writable.
 */

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <wiringPi.h>



void changeVariableCallback(int signum);
uint32_t counter = 0;



static void
addVariable(UA_Server *server) {
    /* Define the attribute of the myInteger variable node */
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 myInteger = 42;
    UA_Variant_setScalar(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT("en-US","the answer");
    attr.displayName = UA_LOCALIZEDTEXT("en-US","the answer");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    /* Add the variable node to the information model */
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, "the answer");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
                              parentReferenceNodeId, myIntegerName,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);
}

/**
 * Now we change the value with the write service. This uses the same service
 * implementation that can also be reached over the network by an OPC UA client.
 */

static void
writeVariable(UA_Server *server) {
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");

    /* Write a different integer value */
    unsigned char r = (unsigned char)rand();
    if (counter % 1000 == 0)
    {
        printf ("%d \n", counter);
    }
    //UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "the.answer");
    UA_Int32 myInteger = (UA_Int32) r;
    UA_Variant myVar;
    UA_Variant_init(&myVar);
    UA_Variant_setScalar(&myVar, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    UA_Server_writeValue(server, myIntegerNodeId, myVar);

    /* Set the status code of the value to an error code. The function
     * UA_Server_write provides access to the raw service. The above
     * UA_Server_writeValue is syntactic sugar for writing a specific node
     * attribute with the write service. */
    UA_WriteValue wv;
    UA_WriteValue_init(&wv);
    wv.nodeId = myIntegerNodeId;
    wv.attributeId = UA_ATTRIBUTEID_VALUE;
    wv.value.status = UA_STATUSCODE_BADNOTCONNECTED;
    wv.value.hasStatus = true;
    UA_Server_write(server, &wv);

    /* Reset the variable to a good statuscode with a value */
    wv.value.hasStatus = false;
    wv.value.value = myVar;
    wv.value.hasValue = true;
    UA_Server_write(server, &wv);
    counter ++;
}

/** It follows the main server code, making use of the above definitions. */

static volatile UA_Boolean running = true;
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}


UA_Server *server_glo;

void changeVariableCallback(int signum)
{
    writeVariable(server_glo);
    signal (SIGALRM, changeVariableCallback);
    ualarm (30000,0);
}



int main(void) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    wiringPiSetup();
    pinMode(0, OUTPUT);

    server_glo = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server_glo));
    UA_ServerConfig* config = UA_Server_getConfig(server_glo);
    config->verifyRequestTimestamp = UA_RULEHANDLING_ACCEPT;
#ifdef UA_ENABLE_WEBSOCKET_SERVER
    UA_ServerConfig_addNetworkLayerWS(UA_Server_getConfig(server_glo), 7681, 0, 0);
#endif

    addVariable(server_glo);
    writeVariable(server_glo);

    signal (SIGALRM, changeVariableCallback);
    alarm (3);

    UA_StatusCode retval = UA_Server_run(server_glo, &running);

    UA_Server_delete(server_glo);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}