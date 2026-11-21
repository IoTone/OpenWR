/*
    This is an imperfect alternative to our favorite Window interval
    code.

    Limits:
    - Requires access to a ScriptComponent
    - no milliseconds, just seconds
    - not sure what else will fail

    IoTone, Inc. 2025
*/
export class Interval {
    private timeoutInterval? : DelayedCallbackEvent | null = null
    private isTimeoutIntervalCancelled: Boolean = false;
    private intervalName = "";
    
    constructor(sceneObj: BaseScriptComponent) {
        this.timeoutInterval = sceneObj.createEvent("DelayedCallbackEvent");
    }
 
    cancelInterval() : void {
        this.timeoutInterval.cancel();
        this.isTimeoutIntervalCancelled = true;
    }
    
    setTimeoutInSec(func: () => void, timeoutinsec: number) : void {
        this.timeoutInterval.bind(func);
        this.timeoutInterval.reset(timeoutinsec);
    }
    
    resetInterval() : void {
        this.timeoutInterval.cancel();
        this.isTimeoutIntervalCancelled = false;
    }
    
    setIntervalInSec(func: () => void, timeoutinsec: number, intervalName: String): void {
        this.timeoutInterval.bind(function(eventData) {
            // XXX Total hack
            func();
            print("delay is over");
            
            if (this.isTimeoutIntervalCancelled) {
                print("interval for " + this.intervalName + " is cancelled");
                this.timeoutInterval.cancel();
            } else {
                this.setIntervalInSec(func, timeoutinsec, intervalName);
                // timeoutInterval.reset(timeoutinsec); // repeat forever until cancel
            }
        });
        this.timeoutInterval.reset(timeoutinsec);
    }
}
