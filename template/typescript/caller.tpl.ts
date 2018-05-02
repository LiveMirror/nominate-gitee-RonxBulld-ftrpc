
class rpcPack {
    public type: string = "";
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

export class Test {
    public static request(req: string, _callback: () => void): string {
        let thisSerial = RPC.serial++;
        let reqStruct: rpcPack = {
            type: "rpc",
            serial: thisSerial,
            funcName: "request",
            params: [],
        };
        RPC.callbackMap_noret[thisSerial] = _callback;
        return JSON.stringify(reqStruct);
    }
}

class RPC {
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
        if(RPC.callbackMap_noret.has(ansStruct.serial)) {
            let ptr = RPC.callbackMap_noret[ansStruct.serial];
            RPC.callbackMap_noret.delete(ansStruct.serial);
            ptr();
            return true;
        } else if (RPC.callbackMap.has(ansStruct.serial)) {
            let ptr = RPC.callbackMap[ansStruct.serial];
            RPC.callbackMap.delete(ansStruct.serial);
            ptr(ansStruct.result);
            return true;
        } else {
            return false;
        }
    }
}
