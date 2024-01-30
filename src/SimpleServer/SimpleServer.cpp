#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>

#include <csignal>
#include <cstdlib>


/// <summary>
/// OPC-UAサーバに変数を追加する
/// </summary>
static void addSampleVariable(UA_Server* server) {
    // SampleVariable変数ノードの属性を定義する
    UA_VariableAttributes attr = UA_VariableAttributes_default; // 属性のデフォルト値を設定
    UA_Int32 sampleVarInitValue = 42;    // 初期値の設定
    UA_Variant_setScalar(&attr.value, &sampleVarInitValue, &UA_TYPES[UA_TYPES_INT32]);  // 変数に初期値を設定
    
    // 属性値の設定
    attr.description = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Sample Variable for mamezou-tech"); // 変数の説明
    attr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Sample Variable");  // 表示名
    attr.dataType = UA_TYPES[UA_TYPES_INT32].typeId;    // データ型
    attr.accessLevel = UA_ACCESSLEVELMASK_READ | UA_ACCESSLEVELMASK_WRITE;  // アクセス属性

    // Variable Nodeを情報モデルに追加する
    UA_NodeId sampleVarNodeId = UA_NODEID_STRING(1, (char*)"SampleVarNodeId");  // ノードIDの定義
    UA_QualifiedName sampleVarName = UA_QUALIFIEDNAME(1, (char*)"SampleVar");   // ブラウザ名の定義
    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);      // 親ノードのID
    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES); // 親参照ノードID

    // 定義したVariableNodeをServerに追加する
    UA_Server_addVariableNode(server, sampleVarNodeId, parentNodeId,
        parentReferenceNodeId, sampleVarName,
        UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE), attr, NULL, NULL);

    // メモリ解放
    UA_VariableAttributes_delete(&attr);
    UA_NodeId_delete(&sampleVarNodeId);
    UA_NodeId_delete(&parentNodeId);
    UA_NodeId_delete(&parentReferenceNodeId);
    UA_QualifiedName_delete(&sampleVarName);
}


/// <summary>
/// メソッドのコールバック関数
/// 変数の値に引数で指定した数だけ加算する
/// </summary>
static UA_StatusCode increaseVariableCallback(UA_Server* server,
    const UA_NodeId* sessionId, void* sessionContext,
    const UA_NodeId* methodId, void* methodContext,
    const UA_NodeId* objectId, void* objectContext,
    size_t inputSize, const UA_Variant* input,
    size_t outputSize, UA_Variant* output)
{
    // 引数の値を取得する
    UA_Int32* delta = (UA_Int32*)input[0].data;

    // 変数の値を取得する
    UA_Variant sampleVar;
    UA_NodeId sampleVarNodeId = UA_NODEID_STRING(1, (char*)"SampleVarNodeId");
    UA_Server_readValue(server, sampleVarNodeId, &sampleVar);
    UA_Int32 sampleVarValue = ((UA_Int32*)sampleVar.data)[0];

    // 変数に引数の値を加える
    UA_Variant newVar;
    UA_Int32 newVarValue = sampleVarValue + *delta;
    UA_Variant_init(&newVar);
    UA_Variant_setScalar(&newVar, &newVarValue, &UA_TYPES[UA_TYPES_INT32]);

    // 加算後の値をServerに書き込む
    UA_StatusCode retval = UA_Server_writeValue(server, sampleVarNodeId, newVar);

    // メモリ解放
    UA_Variant_delete(&sampleVar);
    UA_Variant_delete(&newVar);

    return retval;
}



/// <summary>
/// 新規にメソッド をOPC-UAサーバに追加する
/// </summary>
static void addIncreaseVariableMethod(UA_Server* server) {
    // 入力引数の生成
    UA_Argument inputArg;

    // 引数の設定
    UA_Argument_init(&inputArg);
    inputArg.description = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"How much increase the number of the variable");
    inputArg.name = UA_STRING((char*)"delta");
    inputArg.dataType = UA_TYPES[UA_TYPES_INT32].typeId;
    inputArg.valueRank = UA_VALUERANK_SCALAR;

    // Methodノードの追加
    UA_MethodAttributes methodAttr = UA_MethodAttributes_default;
    methodAttr.description = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"Increase the value of a variable by the number of arguments");
    methodAttr.displayName = UA_LOCALIZEDTEXT((char*)"en-US", (char*)"IncreaseVariable");
    methodAttr.executable = true;
    methodAttr.userExecutable = true;
    UA_Server_addMethodNode(server, UA_NODEID_STRING(1, (char*)"addIncreaseVarNodeId"),
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
        UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
        UA_QUALIFIEDNAME(1, (char*)"IncreaseVariable"),
        methodAttr, &increaseVariableCallback,
        1, &inputArg, 0, NULL,
        NULL, NULL);

    // メモリ解放
    UA_Argument_delete(&inputArg);
    UA_MethodAttributes_delete(&methodAttr);
}



static volatile UA_Boolean running = true;

/// <summary>
/// 中止シグナルハンドラ
/// </summary>
static void stopHandler(int sign) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "received ctrl-c");
    running = false;
}

/// <summary>
/// メイン関数
/// </summary>
int main(void) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    // サーバの生成
    UA_Server* server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    UA_ServerConfig* config = UA_Server_getConfig(server);
    config->verifyRequestTimestamp = UA_RULEHANDLING_ACCEPT;

    // 変数の追加
    addSampleVariable(server);

    // メソッドをサーバに追加する
    addIncreaseVariableMethod(server);

    // runningがTrueの間サーバを起動する
    UA_StatusCode retval = UA_Server_run(server, &running);

    // サーバの削除
    UA_Server_delete(server);

    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}