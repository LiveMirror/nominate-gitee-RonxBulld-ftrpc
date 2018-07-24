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
// #@{Provider Classes}@#
var ftrpc_provider;
(function (ftrpc_provider) {
    function ProviderDoCall(root) {
        var ret = new rpcResult();
        ret["success"] = false;
        if (typeof root === "string") {
            try {
                root = JSON.parse(root);
            }
            catch (e) {
                return JSON.stringify(ret);
            }
        }
        if (root["type"] != "rpc") {
            return JSON.stringify(ret);
        }
        if (root["version"] != version) {
            return JSON.stringify(ret);
        }
        ret["version"] = version;
        ret["type"] = "rpcAnswer";
        var serial = root["serial"];
        if (typeof serial !== "number") {
            return JSON.stringify(ret);
        }
        ret["serial"] = serial;
        var funcName = root["funcName"];
        if (typeof funcName !== "string") {
            return JSON.stringify(ret);
        }
        ret["funcName"] = funcName;
        var paramCount = -1;
        var param = root["params"];
        if (param instanceof Array) {
            paramCount = param.length;
        }
        else {
            paramCount = 0;
        }
        // #@{Set Framework Version}@#
        switch (funcName) {
            // #@{Function Check and Call}@#
        }
        ret["success"] = true;
        return JSON.stringify(ret);
    }
    ftrpc_provider.ProviderDoCall = ProviderDoCall;
})(ftrpc_provider = exports.ftrpc_provider || (exports.ftrpc_provider = {}));
