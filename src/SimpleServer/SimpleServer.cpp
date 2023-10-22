/* This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information. */

// NodeIdについてはこのページで学習できる
// https://documentation.unified-automation.com/uasdkhp/1.4.1/html/_l2_ua_node_ids.html

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <signal.h>
#include <stdlib.h>



/// <summary>
/// OPC-UAサーバーに変数を追加する
/// </summary>
/// <param name="server"></param>
static void addVariable(UA_Server* server) {
    /* Define the attribute of the myInteger variable node */
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    UA_Int32 myInteger = 42;    // 初期値の設定
    UA_Variant_setScalar(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
    attr.description = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"the answer");
    attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"the answer");
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;

    /* Add the variable node to the information model */
    // Variable Nodeを情報モデルに追加する
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, (char*)"the.answer");   // 
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, (char*)"the answer");  // 
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);  // 親ノードのID
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES); // 親参照ノードID

    // VariableNodeをServerに追加する
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
        parentReferenceNodeId, myIntegerName,
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);
}



/// <summary>
/// 
/// </summary>
/// <param name="server"></param>
static void addMatrixVariable(UA_Server* server) {
    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Double Matrix");    // 表示名設定
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;  // R/W属性設定

    // 変数設定
    attr.dataType = UA_TYPES[UA_TYPES_DOUBLE].typeId;
    attr.valueRank = UA_VALUERANK_TWO_DIMENSIONS;
    UA_UInt32 arrayDims[2] = { 2,2 };
    attr.arrayDimensions = arrayDims;
    attr.arrayDimensionsSize = 2;

    UA_Double zero[4] = { 0.0, 1.0, 1.3, 2.0 };
    UA_Variant_setArray(&attr.value, zero, 4, &UA_TYPES[UA_TYPES_DOUBLE]);
    attr.value.arrayDimensions = arrayDims;
    attr.value.arrayDimensionsSize = 2;

    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, (char*)"double.matrix");
    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, (char*)"double matrix");
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
        parentReferenceNodeId, myIntegerName,
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
        attr, NULL, NULL);
}


/// <summary>
/// サーバ内部で値を書き換える
/// </summary>
/// <param name="server"></param>
static void writeVariable(UA_Server* server) {
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, (char*)"the.answer");

    /* Write a different integer value */
    UA_Int32 myInteger = 43;
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
}



/// <summary>
/// 故意にint型のNodeにstring型の値を書き込んでみる
/// </summary>
/// <param name="server"></param>
static void writeWrongVariable(UA_Server* server) {
    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, (char*)"the.answer");

    /* Write a string */
    UA_String myString = UA_STRING((char*)"test");
    UA_Variant myVar;
    UA_Variant_init(&myVar);
    UA_Variant_setScalar(&myVar, &myString, &UA_TYPES[UA_TYPES_STRING]);
    UA_StatusCode retval = UA_Server_writeValue(server, myIntegerNodeId, myVar);
    printf("Writing a string returned statuscode %s\n", UA_StatusCode_name(retval));
}


/// <summary>
/// Method実行時のコールバック関数
/// </summary>
/// <param name="server"></param>
/// <param name="sessionId"></param>
/// <param name="sessionContext"></param>
/// <param name="methodId"></param>
/// <param name="methodContext"></param>
/// <param name="objectId"></param>
/// <param name="objectContext"></param>
/// <param name="inputSize"></param>
/// <param name="input"></param>
/// <param name="outputSize"></param>
/// <param name="output"></param>
/// <returns></returns>
static UA_StatusCode IncInt32ArrayMethodCallback(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* methodId, void* methodContext,
    const UA_NodeId* objectId, void* objectContext,
    size_t inputSize, const UA_Variant* input,
    size_t outputSize, UA_Variant* output) 
{
    UA_Int32* inputArray = (UA_Int32*)input[0].data;
    UA_Int32 delta = *(UA_Int32*)input[1].data;

    // Copy the input array
    UA_StatusCode retval = UA_Variant_setArrayCopy(output, inputArray, 5, &UA_TYPES[UA_TYPES_INT32]);

    if (retval != UA_STATUSCODE_GOOD) {
        return retval;
    }

    // Methodの第二引数deltaだけ第一引数の配列要素に加算する
    UA_Int32* outputArray = (UA_Int32*)output->data;
    for (size_t i = 0; i < input->arrayLength; i++) {
        outputArray[i] = inputArray[i] + delta;
    }

    return UA_STATUSCODE_GOOD;

}


/// <summary>
/// ClientからコールされるMethod
/// </summary>
/// <param name="server"></param>
static void addIncInt32ArrayMethod(UA_Server* server) {
    // 2つの引数
    UA_Argument inputArguments[2];   // 引数配列
    
    // 第1引数の設定
    UA_Argument_init(&inputArguments[0]);
    inputArguments[0].description = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"int32[5] array");
    inputArguments[0].name = UA_STRING((char*)"int32[5] array");
    inputArguments[0].dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    inputArguments[0].valueRank = UA_VALUERANK_ONE_DIMENSION;
    UA_UInt32 pInputDimension = 5;
    inputArguments[0].arrayDimensionsSize = 1;
    inputArguments[0].arrayDimensions = &pInputDimension;

    // 第2引数の設定
    UA_Argument_init(&inputArguments[1]);
    inputArguments[1].description = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"int32 delta");
    inputArguments[1].name = UA_STRING((char*)"int32 delta");
    inputArguments[1].dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    inputArguments[1].valueRank = UA_VALUERANK_SCALAR;

    // 戻り値の設定
    UA_Argument outputArgument;
    UA_Argument_init(&outputArgument);
    outputArgument.description = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"int32[5] array");
    outputArgument.name = UA_STRING((char*)"each entry is incremented by the delta");
    outputArgument.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    outputArgument.valueRank = UA_VALUERANK_ONE_DIMENSION;
    UA_UInt32 pOutputDimension = 5;
    outputArgument.arrayDimensionsSize = 1;
    outputArgument.arrayDimensions = &pOutputDimension;

    // Methodノードの追加
    UA_MethodAttributes incAttr = UA_MethodAttributes_default;
    incAttr.description = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"IncInt32ArrayValues");
    incAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"IncInt32ArrayValues");
    incAttr.executable = true;
    incAttr.userExecutable = true;
    UA_Server_addMethodNode(server, UA_NODEID_STRING(1, (char*)"IncInt32ArrayValues"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, (char*)"IncInt32ArrayValues"),
        incAttr, &IncInt32ArrayMethodCallback,
        2, inputArguments, 1, &outputArgument,
        NULL, NULL);
}


static volatile UA_Boolean running = true;
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}



/// <summary>
/// メイン関数
/// </summary>
/// <param name=""></param>
/// <returns></returns>
int main(void) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    UA_ServerConfig* config = UA_Server_getConfig(server);
    config->verifyRequestTimestamp = UA_RULEHANDLING_ACCEPT;
#ifdef UA_ENABLE_WEBSOCKET_SERVER
    UA_ServerConfig_addNetworkLayerWS(UA_Server_getConfig(server), 7681, 0, 0, NULL, NULL);
#endif

    addVariable(server);
    addMatrixVariable(server);
    // addDoubleMatrixMethod(server)
    writeVariable(server);
    writeWrongVariable(server);

    UA_StatusCode retval = UA_Server_run(server, &running);

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}