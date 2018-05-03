
class rpcPack {
    public type: string = "";
    public version: number = 0;
    public serial: number = -1;
    public funcName: string = "";
    public params: any = null;
}

class rpcResult {
    public type: string = "";
    public serial: number = -1;
    public success: boolean = false;
    public result: any = null;
}

// #@{Non-blocking RPC with callback}@#
/* SED REMOVE
export class Test {
    public static request(req: string, _callback: () => void): string {
        let thisSerial = ftrpc_caller.serial++;
        let reqStruct: rpcPack = {
            type: "rpc",
            version: 1,
            serial: thisSerial,
            funcName: "request",
            params: [],
        };
        ftrpc_caller.callbackMap_noret[thisSerial] = _callback;
        return JSON.stringify(reqStruct);
    }
}
!SED REMOVE*/
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
        if(ansStruct.serial in ftrpc_caller.callbackMap_noret) {
            let ptr = ftrpc_caller.callbackMap_noret[ansStruct.serial];
            ftrpc_caller.callbackMap_noret.delete(ansStruct.serial);
            ptr();
            return true;
        } else if (ansStruct.serial in ftrpc_caller.callbackMap) {
            let ptr = ftrpc_caller.callbackMap[ansStruct.serial];
            ftrpc_caller.callbackMap.delete(ansStruct.serial);
            ptr(ansStruct.result);
            return true;
        } else {
            return false;
        }
    }
}
