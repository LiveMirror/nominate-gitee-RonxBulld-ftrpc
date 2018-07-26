
class rpcPack {
    // #@{FRAMEWORK_VERSION}@#
    public funcName: string = "";
    public params: any = null;
    public serial: number = -1;
    public type: string = "";
    public version: number = 0;
}

class rpcResult {
    // #@{FRAMEWORK_VERSION}@#
    public funcName: string = "";
    public return: any = null;
    public serial: number = -1;
    public success: boolean = false;
    public type: string = "";
    public version: number = 0;
}

// #@{Provider Classes}@#
export module ftrpc_provider {
    export function ProviderDoCall(root: any, extraOption?:any): string {
        let ret = new rpcResult();
        ret["success"] = false;
        if (typeof root === "string") {
            try {
                root = JSON.parse(root);
            } catch (e) {
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
        let serial = root["serial"];
        if (typeof serial !== "number") {
            return JSON.stringify(ret);
        }
        ret["serial"] = serial;
        let funcName = root["funcName"];
        if (typeof funcName !== "string") {
            return JSON.stringify(ret);
        }
        ret["funcName"] = funcName;
        let paramCount = -1;
        let param = root["params"];
        if (param instanceof Array) {
            paramCount = param.length;
        } else {
            paramCount = 0;
        }
        // #@{Set Framework Version}@#
        switch (funcName) {
// #@{Function Check and Call}@#
        }
        ret["success"] = true;
        return JSON.stringify(ret);
    }
}