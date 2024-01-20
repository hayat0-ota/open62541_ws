/*
下記の2つのノードにアクセスするクライアント
    - 変数 ... SampleVariable : int
    - 関数 ... IncreaseVariable(int) : int


*/

#include <iostream>
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>


/// <summary>
/// 変数の値を読みだす
/// </summary>
/// <param name="client">クライアントインスタンスアドレス</param>
void readSampleVariable(UA_Client* client) {
    UA_Int32 value = 0;
    UA_Variant* var = UA_Variant_new();
    UA_StatusCode retval = UA_Client_readValueAttribute(client, UA_NODEID_STRING(1, (char*)"SampleVarNodeId"), var);    // NodeIdを指定する
    
    if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(var) &&
        var->type == &UA_TYPES[UA_TYPES_INT32])
    {
        value = *(UA_Int32*)var->data;
        printf("the value of SampleVariable: %d\n", value);
    }
    else {
        printf("Failed to read SampleVariable\n");
    }
}


/// <summary>
/// 変数の値を書き込む
/// </summary>
/// <param name="client">クライアントインスタンスアドレス</param>
/// <param name="data">書き換える値</param>
UA_StatusCode writeSampleVariable(UA_Client* client, UA_Int32 newValue) {
    UA_Variant newValueVariant;
    UA_Variant_setScalar(&newValueVariant, &newValue, &UA_TYPES[UA_TYPES_INT32]);
    UA_StatusCode retval = UA_Client_writeValueAttribute(client, UA_NODEID_STRING(1, (char*)"SampleVarNodeId"), &newValueVariant);

    if (retval != UA_STATUSCODE_GOOD) {
        printf("Failed to write sample variable value, returned %x\n", retval);
    }
    return retval;
}



/// <summary>
/// サーバに登録している関数を実行する
/// </summary>
void invokeMethod(UA_Client* client) {
    UA_Variant input;
    UA_Int32 argValue = 32; // 追加する値（delta）

    UA_Variant_init(&input);
    UA_Variant_setScalarCopy(&input, &argValue, &UA_TYPES[UA_TYPES_INT32]);

    // 戻り値用の変数
    size_t outputSize;
    UA_Variant* output;
    
    // 関数を呼び出す
    UA_StatusCode retval = UA_Client_call(
        client, 
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),   // オブジェクトID
        UA_NODEID_STRING(1, (char*)"addIncreaseVarNodeId"), // メソッドのID
        1,              // 入力個数 
        &input,         // 入力データ配列
        &outputSize,    // 出力データ個数
        &output         // 出力データ配列
    );

    if (retval == UA_STATUSCODE_GOOD) {
        printf("Method call was successful, and %lu returned values available.\n", (unsigned long)outputSize);
        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    }
    else {
        printf("Method call was unsuccessful, and %x returned values available.\n", retval);
    }
    UA_Variant_clear(&input);
}


int main()
{
    // クライアントインスタンスの生成
    UA_Client* client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(client));
    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:4840");
    if (retval != UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND,
            "The connection failed with status code %s\n",
            UA_StatusCode_name(retval));
        UA_Client_delete(client);
        return 0;
    }

    // 変数の値を読みだす
    printf("readSampleVariable()\n");
    readSampleVariable(client); // 値を取得する

    // 変数の値を書き込む
    printf("=== writeSampleVariable() ===\n");
    writeSampleVariable(client, -1);
    readSampleVariable(client); // 結果を出力する

    // 関数にアクセスする
    printf("=== invokeMethod() ===\n");
    invokeMethod(client);
    readSampleVariable(client); // 結果を出力する


    printf("Press any key to continue ...");
    char* str = (char*)"";
    std::cin >> str;

    return EXIT_SUCCESS;
}

