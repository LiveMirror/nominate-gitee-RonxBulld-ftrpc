
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

// #@{Non-blocking RPC with callback}@#
export class ftrpc_caller {
    public static serial: number = 0;
    public static callbackMap = new Map<number,(data:any)=>void>();
    public static callbackMap_noret = new Map<number, ()=>void>();

    public static ReturnRecived(answerJson: string): boolean {
        let ansStruct: rpcResult = <rpcResult>JSON.parse(answerJson);
        if(ansStruct.type != "rpcAnswer") {
            return false;
        } else if(ansStruct.success == false) {
            return false;
        }
        switch (ansStruct.funcName) {
// #@{Verify Return Value And Call}@#
        }
        if(ansStruct.serial in ftrpc_caller.callbackMap_noret) {
            let ptr = ftrpc_caller.callbackMap_noret[ansStruct.serial];
            ftrpc_caller.callbackMap_noret.delete(ansStruct.serial);
            ptr();
            return true;
        } else if (ansStruct.serial in ftrpc_caller.callbackMap) {
            let ptr = ftrpc_caller.callbackMap[ansStruct.serial];
            ftrpc_caller.callbackMap.delete(ansStruct.serial);
            ptr(ansStruct.return);
            return true;
        } else {
            return false;
        }
    }
}
