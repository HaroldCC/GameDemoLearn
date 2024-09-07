#include "angelscript.h"
#include "scriptstdstring/scriptstdstring.h"
#include "scriptbuilder/scriptbuilder.h"
#include "datetime/datetime.h"
#include "scriptmath/scriptmathcomplex.h"
#include "scriptarray/scriptarray.h"

#include <format>
#include <iostream>
#include <cassert>

using std::string;

void print1(const string &str)
{
    std::cout << str;
}

void print(asIScriptGeneric *pGen)
{
    std::cout << std::format("print argCount:{} ", pGen->GetArgCount());
    for (int i = 0; i < pGen->GetArgCount(); ++i)
    {
        std::cout << std::format("arg:{} [type:{}]", pGen->GetArgTypeId(i), pGen->GetArgAddress(i));
    }
}

void ErrorCallback(const asSMessageInfo *pMsg, void *pParam)
{
    std::string_view strErrType = "ERR ";
    if (pMsg->type == asMSGTYPE_WARNING)
    {
        strErrType = "WARN";
    }
    else if (pMsg->type == asMSGTYPE_INFORMATION)
    {
        strErrType = "INFO";
    }

    std::cout << std::format("{} ({}, {}) : {} : {}",
                             pMsg->section,
                             pMsg->row,
                             pMsg->col,
                             strErrType,
                             pMsg->message)
              << std::endl;
}

void DumpEnums(asIScriptEngine *pEngine);
void DumpClasses(asIScriptEngine *pEngine);
void DumpGlobalFunctions(asIScriptEngine *pEngine);
void DumpGlobalProperty(asIScriptEngine *pEngine);
void DumpGlobalTypedefs(asIScriptEngine *pEngine);

void DumpRegisterInfo(asIScriptEngine *pEngine)
{
    DumpEnums(pEngine);
    DumpClasses(pEngine);
    DumpGlobalFunctions(pEngine);
    DumpGlobalProperty(pEngine);
    DumpGlobalTypedefs(pEngine);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cout << std::format("Usage: <script file>");
        return -1;
    }

    asIScriptEngine *pEngine = asCreateScriptEngine();
    int              ret     = pEngine->SetMessageCallback(asFUNCTION(ErrorCallback), 0, asCALL_CDECL);

    RegisterStdString(pEngine);
    RegisterScriptDateTime(pEngine);
    RegisterScriptMathComplex(pEngine);
    RegisterScriptArray(pEngine, false);

    std::cout << asGetLibraryOptions() << std::endl;

    int r =
        pEngine->RegisterGlobalFunction("void print(const string&in str)", asFUNCTION(print1), asCALL_CDECL);
    assert(r >= 0);
    // pEngine->RegisterGlobalFunction("void print(string &in, ?&in)",
    //                                 asFUNCTION(print),
    //                                 asCALL_GENERIC);

    DumpRegisterInfo(pEngine);

    CScriptBuilder builder;
    r = builder.StartNewModule(pEngine, "main");
    if (r < 0)
    {
        pEngine->Release();
        return -1;
    }

    r = builder.AddSectionFromFile(argv[1]);
    if (r < 0)
    {
        std::cout << std::format("加载脚本{}出错", argv[1]);
        pEngine->Release();
        return -1;
    }

    r = builder.BuildModule();
    if (r < 0)
    {
        pEngine->WriteMessage(argv[1], 0, 0, asMSGTYPE_ERROR, "编译脚本出错");
        pEngine->Release();
        return -1;
    }
    pEngine->WriteMessage(argv[1], 0, 0, asMSGTYPE_INFORMATION, "编译脚本成功");

    asIScriptContext *pCtx = pEngine->CreateContext();
    if (nullptr == pCtx)
    {
        std::cout << "创建脚本上下文出错" << std::endl;
        pEngine->Release();
        return -1;
    }

    asIScriptFunction *pFunc = pEngine->GetModule("main")->GetFunctionByDecl("void main()");
    if (nullptr == pFunc)
    {
        std::cout << "找不到函数 void main()" << std::endl;
        pCtx->Release();
        pEngine->Release();
        return -1;
    }

    r = pCtx->Prepare(pFunc);
    if (r < 0)
    {
        std::cout << "prepare func 出错" << std::endl;
        pCtx->Release();
        pEngine->Release();
        return -1;
    }

    r = pCtx->Execute();

    pCtx->Release();

    pEngine->ShutDownAndRelease();

    return 0;
}

#include <fstream>

void WriteToFile(std::string_view info)
{
    std::ofstream fileStream("as.predefined", std::ios_base::app);
    if (!fileStream.is_open())
    {
        std::cerr << "open file:as.predefined failed" << std::endl;
        return;
    }
    fileStream << info;
    fileStream.close();
}

void DumpEnums(asIScriptEngine *pEngine)
{
    if (nullptr == pEngine)
    {
        return;
    }

    std::string_view strEnumInfo;
    for (int i = 0; i < pEngine->GetEnumCount(); ++i)
    {
        asITypeInfo *pEnum = pEngine->GetEnumByIndex(i);
        if (nullptr == pEnum)
        {
            continue;
        }

        std::string_view strNameSpace = pEnum->GetNamespace();
        if (!strNameSpace.empty())
        {
            strEnumInfo = std::format("namespace {} {{\n", strNameSpace);
        }

        strEnumInfo = std::format("{}enum {} {{\n", strEnumInfo, pEnum->GetName());
        for (int j = 0; j < pEnum->GetEnumValueCount(); ++j)
        {
            strEnumInfo = std::format("{}   {}", strEnumInfo, pEnum->GetEnumValueByIndex(j, nullptr));
            if (j < pEnum->GetEnumValueCount() - 1)
            {
                strEnumInfo = std::format("{},", strEnumInfo);
            }
            strEnumInfo = std::format("{}\n", strEnumInfo);
        }
        strEnumInfo = std::format("{}}}\n", strEnumInfo);
        if (!strNameSpace.empty())
        {
            strEnumInfo = std::format("{}}}\n", strEnumInfo);
        }
    }

    WriteToFile(strEnumInfo);
}

void DumpClasses(asIScriptEngine *pEngine)
{
    if (nullptr == pEngine)
    {
        return;
    }

    std::string strClassInfo;
    for (int i = 0; i < pEngine->GetObjectTypeCount(); ++i)
    {
        asITypeInfo *pClassType = pEngine->GetObjectTypeByIndex(i);
        if (nullptr == pClassType)
        {
            continue;
        }

        std::string_view strNameSpace = pClassType->GetNamespace();
        if (!strNameSpace.empty())
        {
            strClassInfo.append(std::format("\nnamespace {} {{\n", strNameSpace));
        }

        strClassInfo.append(std::format("\nclass {}\n", pClassType->GetName()));
        if (pClassType->GetSubTypeCount())
        {
            strClassInfo.append("<");
            for (int sub = 0; sub < pClassType->GetSubTypeCount(); ++sub)
            {
                if (sub < pClassType->GetSubTypeCount() - 1)
                {
                    strClassInfo.append(", ");
                }

                asITypeInfo *pSubType = pClassType->GetSubType(sub);
                strClassInfo.append(pSubType->GetName());
            }

            strClassInfo.append(">");
        }

        strClassInfo.append("{\n");

        for (int j = 0; j < pClassType->GetBehaviourCount(); ++j)
        {
            asEBehaviours      behaviours;
            asIScriptFunction *pFunc = pClassType->GetBehaviourByIndex(j, &behaviours);
            if (behaviours == asBEHAVE_CONSTRUCT || behaviours == asBEHAVE_DESTRUCT)
            {
                strClassInfo.append(std::format("    {};\n", pFunc->GetDeclaration(false, true, true)));
            }
        }

        for (int j = 0; j < pClassType->GetMethodCount(); ++j)
        {
            const auto m = pClassType->GetMethodByIndex(j);
            strClassInfo.append(std::format("    {};\n", m->GetDeclaration(false, true, true)));
        }
        for (int j = 0; j < pClassType->GetPropertyCount(); ++j)
        {
            strClassInfo.append(std::format("    {};\n", pClassType->GetPropertyDeclaration(j, true)));
        }
        for (int j = 0; j < pClassType->GetChildFuncdefCount(); ++j)
        {
            auto p = pClassType->GetChildFuncdef(j);
            if (nullptr == p)
            {
                continue;
            }

            strClassInfo.append(std::format("    funcdef {};\n",
                                            p->GetFuncdefSignature()->GetDeclaration(false, true, true)));
        }
        strClassInfo.append("}\n");
        if (!strNameSpace.empty())
        {
            strClassInfo.append("}\n");
        }
    }

    WriteToFile(strClassInfo);
}

void DumpGlobalFunctions(asIScriptEngine *pEngine)
{
}

void DumpGlobalProperty(asIScriptEngine *pEngine)
{
}

void DumpGlobalTypedefs(asIScriptEngine *pEngine)
{
}