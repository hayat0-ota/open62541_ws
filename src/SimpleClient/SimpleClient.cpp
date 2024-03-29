﻿/*
下記の2つのノードにアクセスするクライアント
    - 変数 ... SampleVariable : int
    - 関数 ... IncreaseVariable(int) : int
*/

#include <iostream>
#include <cstdio>
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
    UA_NodeId nodeId = UA_NODEID_STRING(1, (char*)"SampleVarNodeId");
    UA_StatusCode retval = UA_Client_readValueAttribute(client, nodeId, var);
    
    if (retval == UA_STATUSCODE_GOOD && UA_Variant_isScalar(var) &&
        var->type == &UA_TYPES[UA_TYPES_INT32])
    {
        value = *(UA_Int32*)var->data;
        printf("the value of SampleVariable: %d\n\n", value);
    }
    else {
        printf("Failed to read SampleVariable\n\n");
    }

    // メモリ解放
    UA_Variant_delete(var);
    UA_NodeId_delete(&nodeId);
}


/// <summary>
/// 変数の値を書き込む
/// </summary>
/// <param name="client">クライアントインスタンスアドレス</param>
/// <param name="data">書き換える値</param>
UA_StatusCode writeSampleVariable(UA_Client* client, UA_Int32 newValue) {
    UA_Variant newValueVariant;
    UA_Variant_setScalar(&newValueVariant, &newValue, &UA_TYPES[UA_TYPES_INT32]);
    UA_NodeId nodeId = UA_NODEID_STRING(1, (char*)"SampleVarNodeId");
    UA_StatusCode retval = UA_Client_writeValueAttribute(client, nodeId, &newValueVariant);

    if (retval != UA_STATUSCODE_GOOD) {
        printf("Failed to write sample variable value, returned %x\n\n", retval);
    }

    // メモリ解放
    UA_Variant_delete(&newValueVariant);
    UA_NodeId_delete(&nodeId);

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
        printf("Method call was successful, and %lu returned values available.\n\n", (unsigned long)outputSize);
        UA_Array_delete(output, outputSize, &UA_TYPES[UA_TYPES_VARIANT]);
    }
    else {
        printf("Method call was unsuccessful, and %x returned values available.\n\n", retval);
    }

    // メモリ解放
    UA_Variant_delete(&input);
}


/// <summary>
/// メイン関数
/// </summary>
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

    // 変数の現在の値を読み出す
    printf("Press any key to Read Sample Variable ...");
    std::cin.get();
    readSampleVariable(client); // 値を取得する

    // 任意の値を書き込む
    printf("Press any key to Write Sample Variable ...");
    std::cin.get();
    int targetValue = -1;
    writeSampleVariable(client, targetValue);
    readSampleVariable(client); // 書き込んだ結果を出力する

    // 関数にアクセスする
    printf("Press any key to invoke method ...");
    std::cin.get();
    invokeMethod(client);
    readSampleVariable(client); // 結果を出力する


    printf("Press any key to exit ...");
    std::cin.get();

    return EXIT_SUCCESS;
}

