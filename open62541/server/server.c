/*#include <ua_server.h>
#include <ua_config_default.h>
#include <ua_log_stdout.h>*/

#include "open62541.h"
#include <signal.h>

int32_t passate = 0;

//ID must be greater than 119
void addVariable(UA_Server *server, UA_Int32 value, char *name, UA_Int32 ID)
{
    /* Define the attribute of the myInteger variable node */
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 myInteger = value;
    UA_Variant_setScalar(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT("en-US", name);
    attr.displayName = UA_LOCALIZEDTEXT("en-US", name);
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    /* Add the variable node to the information model */
    UA_NodeId myIntegerNodeId = UA_NODEID_NUMERIC(1, ID);
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, name);
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
                              parentReferenceNodeId, myIntegerName,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);
}

#ifdef ENABLE_VARIABLE_INCREMET

#pragma message "INCREMENTO DELLA VARIABILE ATTIVATO"

static void
beforeReadTime(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeid, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data)
{
    UA_Variant value;
    passate++;
    UA_Variant_setScalar(&value, &passate, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(1, 200);
    UA_Server_writeValue(server, currentNodeId, value);
}

static void
afterWriteTime(UA_Server *server,
               const UA_NodeId *sessionId, void *sessionContext,
               const UA_NodeId *nodeId, void *nodeContext,
               const UA_NumericRange *range, const UA_DataValue *data)
{
}

static void
addValueCallbackToCurrentTimeVariable(UA_Server *server)
{
    UA_NodeId currentNodeId = UA_NODEID_NUMERIC(1, 200);
    UA_ValueCallback callback;
    callback.onRead = beforeReadTime;
    callback.onWrite = afterWriteTime;
    UA_Server_setVariableNode_valueCallback(server, currentNodeId, callback);
}
#endif

UA_Boolean running = true;
static void stopHandler(int sig)
{
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}

int main(void)
{
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_ServerConfig *config = UA_ServerConfig_new_default();
    UA_Server *server = UA_Server_new(config);

    addVariable(server, 33, "readItem", 200);
    addVariable(server, 0, "measurementItem", 201);

#ifdef ENABLE_VARIABLE_INCREMET
    addValueCallbackToCurrentTimeVariable(server);
#endif

    UA_StatusCode retval = UA_Server_run(server, &running);
    UA_Server_delete(server);
    UA_ServerConfig_delete(config);
    return (int)retval;
}