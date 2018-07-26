"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
var rpcPack = /** @class */ (function () {
    function rpcPack() {
        // #@{FRAMEWORK_VERSION}@#
        this.funcName = "";
        this.params = null;
        this.serial = -1;
        this.type = "";
        this.version = 0;
    }
    return rpcPack;
}());
var rpcResult = /** @class */ (function () {
    function rpcResult() {
        // #@{FRAMEWORK_VERSION}@#
        this.funcName = "";
        this.return = null;
        this.serial = -1;
        this.success = false;
        this.type = "";
        this.version = 0;
    }
    return rpcResult;
}());
// #@{Non-blocking RPC with callback}@#
var ftrpc_caller = /** @class */ (function () {
    function ftrpc_caller() {
    }
    ftrpc_caller.ReturnRecived = function (answerJson, extraOption) {
        var ansStruct = JSON.parse(answerJson);
        if (ansStruct.type != "rpcAnswer") {
            return false;
        }
        else if (ansStruct.success == false) {
            return false;
        }
        switch (ansStruct.funcName) {
            // #@{Verify Return Value And Call}@#
        }
        if (ansStruct.serial in ftrpc_caller.callbackMap_noret) {
            var ptr = ftrpc_caller.callbackMap_noret[ansStruct.serial];
            ftrpc_caller.callbackMap_noret.delete(ansStruct.serial);
            ptr(extraOption);
            return true;
        }
        else if (ansStruct.serial in ftrpc_caller.callbackMap) {
            var ptr = ftrpc_caller.callbackMap[ansStruct.serial];
            ftrpc_caller.callbackMap.delete(ansStruct.serial);
            ptr(ansStruct.return, extraOption);
            return true;
        }
        else {
            return false;
        }
    };
    ftrpc_caller.serial = 0;
    ftrpc_caller.callbackMap = new Map();
    ftrpc_caller.callbackMap_noret = new Map();
    return ftrpc_caller;
}());
exports.ftrpc_caller = ftrpc_caller;
