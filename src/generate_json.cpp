#include "generate_json.h"

#include <sstream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>

#include "BlockChainReadInterface.h"

#include "jsonUtils.h"
#include "check.h"
#include "convertStrings.h"

#include "BlockInfo.h"
#include "Workers/NodeTestsBlockInfo.h"

using namespace common;
using namespace torrent_node_lib;

static void addIdToResponse(const RequestId &requestId, rapidjson::Value &json, rapidjson::Document::AllocatorType &allocator) {
    if (requestId.isSet) {
        if (std::holds_alternative<std::string>(requestId.id)) {
            json.AddMember("id", strToJson(std::get<std::string>(requestId.id), allocator), allocator);
        } else {
            json.AddMember("id", std::get<size_t>(requestId.id), allocator);
        }
    }
}

template<typename Int>
static rapidjson::Value intOrString(Int intValue, bool isString, rapidjson::Document::AllocatorType &allocator) {
    if (isString) {
        return strToJson(std::to_string(intValue), allocator);
    } else {
        rapidjson::Value strVal;
        strVal.Set(intValue, allocator);
        return strVal;
    }
}

std::string genErrorResponse(const RequestId &requestId, int code, const std::string &error) {
    rapidjson::Document jsonDoc(rapidjson::kObjectType);
    auto &allocator = jsonDoc.GetAllocator();
    addIdToResponse(requestId, jsonDoc, allocator);
    
    rapidjson::Value errorJson(rapidjson::kObjectType);
    errorJson.AddMember("code", code, allocator);
    errorJson.AddMember("message", strToJson(error, allocator), allocator);
    jsonDoc.AddMember("error", errorJson, allocator);
    return jsonToString(jsonDoc, false);
}

static rapidjson::Value blockHeaderToJson(const BlockHeader &bh, const std::optional<std::reference_wrapper<const BlockHeader>> &nextBlock, rapidjson::Document::AllocatorType &allocator, BlockTypeInfo type, const JsonVersion &version) {
    const bool isStringValue = version == JsonVersion::V2;
    
    CHECK(bh.blockNumber.has_value(), "Block header not set");
    rapidjson::Value resultValue(rapidjson::kObjectType);
    if (type == BlockTypeInfo::Simple) {
        resultValue.AddMember("type", strToJson(bh.getBlockType(), allocator), allocator);
    }
    resultValue.AddMember("hash", strToJson(bh.hash, allocator), allocator);
    resultValue.AddMember("prev_hash", strToJson(bh.prevHash, allocator), allocator);
    if (type == BlockTypeInfo::Simple) {
        resultValue.AddMember("tx_hash", strToJson(bh.txsHash, allocator), allocator);
    }
    resultValue.AddMember("number", intOrString(bh.blockNumber.value(), isStringValue, allocator), allocator);
    if (type == BlockTypeInfo::Simple) {
        resultValue.AddMember("timestamp", intOrString(bh.timestamp, isStringValue, allocator), allocator);
        CHECK(bh.countTxs.has_value(), "Count txs not set");
        resultValue.AddMember("count_txs", intOrString(bh.countTxs.value(), isStringValue, allocator), allocator);
        resultValue.AddMember("sign", strToJson(toHex(bh.signature), allocator), allocator);
    }
    if (type != BlockTypeInfo::Small) {
        resultValue.AddMember("size", bh.blockSize, allocator);
        resultValue.AddMember("fileName", strToJson(bh.filePos.fileName, allocator), allocator);
    }
    
    return resultValue;
}

std::string blockHeaderToJson(const RequestId &requestId, const BlockHeader &bh, const std::optional<std::reference_wrapper<const BlockHeader>> &nextBlock, bool isFormat, BlockTypeInfo type, const JsonVersion &version) {
    if (bh.blockNumber == 0) {
        return genErrorResponse(requestId, -32603, "Incorrect block number: 0. Genesis block begin with number 1");
    }
    rapidjson::Document doc(rapidjson::kObjectType);
    auto &allocator = doc.GetAllocator();
    addIdToResponse(requestId, doc, allocator);
    doc.AddMember("result", blockHeaderToJson(bh, nextBlock, allocator, type, version), allocator);
    return jsonToString(doc, isFormat);
}

std::string genCountBlockJson(const RequestId &requestId, size_t countBlocks, bool isFormat, const JsonVersion &version) {
    const bool isStringValue = version == JsonVersion::V2;
    rapidjson::Document doc(rapidjson::kObjectType);
    auto &allocator = doc.GetAllocator();
    addIdToResponse(requestId, doc, allocator);
    rapidjson::Value resultValue(rapidjson::kObjectType);
    resultValue.AddMember("count_blocks", intOrString(countBlocks, isStringValue, allocator), allocator);
    doc.AddMember("result", resultValue, allocator);
    return jsonToString(doc, isFormat);
}

std::string genBlockDumpJson(const RequestId &requestId, const std::string &blockDump, bool isFormat) {
    rapidjson::Document doc(rapidjson::kObjectType);
    auto &allocator = doc.GetAllocator();
    addIdToResponse(requestId, doc, allocator);
    rapidjson::Value resultValue(rapidjson::kObjectType);
    resultValue.AddMember("dump", strToJson(blockDump, allocator), allocator);
    doc.AddMember("result", resultValue, allocator);
    return jsonToString(doc, isFormat);
}

std::string genTestSignStringJson(const RequestId &requestId, const std::string &responseHex) {
    rapidjson::Document doc(rapidjson::kObjectType);
    auto &allocator = doc.GetAllocator();
    addIdToResponse(requestId, doc, allocator);
    rapidjson::Value resultJson(rapidjson::kObjectType);
    resultJson.AddMember("data", strToJson(responseHex, allocator), allocator);
    doc.AddMember("result", resultJson, allocator);
    return jsonToString(doc, false);
}
