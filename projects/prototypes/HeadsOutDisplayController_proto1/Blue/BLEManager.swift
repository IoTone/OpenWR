//
//  BLEManager.swift
//  Blue
//
//  Created by Ejiii on 3/8/23.
//

import Foundation
import CoreBluetooth

struct Peripheral: Identifiable {
    let id: Int
    let name: String
    let rssi: Int
}


class BLEManager: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    var myCentral: CBCentralManager!
    @Published var isSwitchedOn = false
    @Published var peripherals = [Peripheral]()
    @Published var noMatchPeripherals = [Peripheral]()
    var peripheralsId = [UUID]()
    var displayPeripheral: CBPeripheral!

    // ledService("19B10000-E8F2-537E-4F6C-D104768A1228"); // Bluetooth® Low Energy LED Service

    // Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
    // switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1228", BLERead | BLEWrite | BLENotify);
    // buttonCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1229", BLERead | BLEWrite | BLENotify);

    var serviceId = CBUUID(string:  "19B10000-E8F2-537E-4F6C-D104768A1228")
    // Modes
    var switchUUID = CBUUID(string: "19B10001-E8F2-537E-4F6C-D104768A1228")
    // On/off
    var buttonUUID = CBUUID(string: "19B10001-E8F2-537E-4F6C-D104768A1229")
    
    // NOT USED IN THIS PROJECT, and so we won't look for these
    var uartRxUUID = CBUUID(string: "00000000-0000-43F4-A8D4-ECBE34729BB3")
    var uartTxUUID = CBUUID(string: "00000000-0000-4BD9-BA61-23C647249616")
    
    let serviceName = "HeadsOut-OpenWR"
    var isConnected = false
    var uartRxCharacteristic: CBCharacteristic!
    var uartTxCharacteristic: CBCharacteristic!
    var switchCharacteristic: CBCharacteristic!
    var buttonCharacteristic: CBCharacteristic!
    override init() {
        super.init()
 
        myCentral = CBCentralManager(delegate: self, queue: nil)
        myCentral.delegate = self
    }
    
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        print("central state \(central.state)")
         if central.state == .poweredOn {
             isSwitchedOn = true
         }
         else {
             isSwitchedOn = false
         }
    }
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        let peripheralName = peripheral.name
        
        let newPeripheral = Peripheral(id: peripherals.count, name: peripheralName ?? "-", rssi: RSSI.intValue)
        if (peripheralName != nil) {
            print("Found: \(peripheralName)")
        }
        
        /*
        if (advertisementData != nil) {
            print(advertisementData)
        }
         */

        if (peripheralName == serviceName  || peripheralName == "foobar" || peripheralName == "HeadsOut" /* || peripheralName == "Atmosic ATcmd" */) {
            if (!peripheralsId.contains(peripheral.identifier)) {
                peripheralsId.append(peripheral.identifier)
            }
            stopScanning()
            self.displayPeripheral = peripheral
            self.displayPeripheral.delegate = self
            peripherals.append(newPeripheral)
            self.myCentral.connect(peripheral, options: nil)
            print("*****MATCHED********")
            isConnected = true
        } else {
            if (peripheralName != nil && peripheralName != "") {
                // Speak the name of the unmatched string
                let newNoMatchPeripheral = Peripheral(id: noMatchPeripherals.count, name: peripheralName ?? "-", rssi: RSSI.intValue)
                // Save the improper match
                noMatchPeripherals.append(newNoMatchPeripheral)
            }
        }
    }
    
    func startScanning() {
         print("startScanning")
         myCentral.scanForPeripherals(withServices: nil, options: [CBCentralManagerScanOptionAllowDuplicatesKey: false])
     }
    
    func stopScanning() {
        print("stopScanning")
        myCentral.stopScan()
    }
    
    func sendCommand(mode: DeviceDisplayModes) -> () -> () {
        // Send the command
        return {
            // Convert the mode into and id
            let modeid : DeviceDisplayModeIds = convertDeviceDisplayModeToId(mode: mode.rawValue.lowercased())
            // let data = withUnsafeBytes(of: unsignedIntValue.littleEndian) { Data($0) }
            let data = withUnsafeBytes(of: modeid.rawValue.littleEndian) { Data($0) }
            // let bytes : [UInt16] = [0x4]
            // let data = Data(bytes: bytes, count: bytes.count)
            self.displayPeripheral.writeValue(Data(data), for: self.switchCharacteristic, type: CBCharacteristicWriteType.withResponse)
            print(mode)
        }
    }
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("DidConnect")
        discoverServices(peripheral: peripheral)
    }
    
    /*!
     *  @method centralManager:didDisconnectPeripheral:error:
     *
     *  @param central      The central manager providing this information.
     *  @param peripheral   The <code>CBPeripheral</code> that has disconnected.
     *  @param error        If an error occurred, the cause of the failure.
     *
     *  @discussion         This method is invoked upon the disconnection of a peripheral that was connected by {@link connectPeripheral:options:}. If the disconnection
     *                      was not initiated by {@link cancelPeripheralConnection}, the cause will be detailed in the <i>error</i> parameter. Once this method has been
     *                      called, no more methods will be invoked on <i>peripheral</i>'s <code>CBPeripheralDelegate</code>.
     *
     */
    public func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        isConnected = false
        if error == nil {
            
            print("\(getLocalTimestamp()) Successfully disconnected from device.")
        } else {
            print("\(getLocalTimestamp()) Disconnected from device, error occurred:\n\(error)")
        }
        
        // responseDelegate?.didDisconnect(peripheral, withError: error)
    }
    
    
    // Peripheral characteristics discovered
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        service.characteristics?.forEach { characteristic in
            print("CHAR id: \(characteristic.uuid.uuidString) VALUE: \(String(describing: characteristic.value) )")
            // peripheral.readValue(for: characteristic)
            
            if (characteristic.uuid == buttonUUID) {
                print("Set buttonCharacteristic")
                buttonCharacteristic = characteristic;
            } else if (characteristic.uuid == switchUUID) {
                print("Set switchCharacteristic")
                switchCharacteristic = characteristic;
            } else if (characteristic.uuid == uartTxUUID) {
                uartTxCharacteristic = characteristic;
                peripheral.setNotifyValue(true, for: uartTxCharacteristic)
                
            } else if (characteristic.uuid == uartRxUUID) {
                uartRxCharacteristic = characteristic;

               //  peripheral.writeValue(Data("$vers/#".utf8), for: characteristic, type: CBCharacteristicWriteType.withResponse)
            }
        }
    }
       
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if (error != nil) {
            // do something
        } else {
            let data = characteristic.value
            print("Data response : \(String(data: data!, encoding: .utf8) ?? "")")
            // print(characteristic.value ?? "default")
        }
    }
       
    func peripheral(_ peripheral: CBPeripheral, didModifyServices invalidatedServices: [CBService]) {
        print("didModifyServices")
    }
    
    func discoverServices(peripheral: CBPeripheral) {
        peripheral.discoverServices([serviceId])
    }
        
    // Call after discovering services
    func discoverCharacteristics(peripheral: CBPeripheral) {
        guard let services = peripheral.services else {
            print("not service")
            return
        }
        for service in services {
            peripheral.discoverCharacteristics(nil, for: service)
        }
    }
        
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        guard let services = peripheral.services else {
            print("ERROR didDiscoverServices")
            return
        }
        if services.count > 0 {
            discoverCharacteristics(peripheral: peripheral)
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateNotificationStateFor characteristic: CBCharacteristic, error: Error?) {
        if let error = error {
            print("error \(error)")
            return
        }
        print(characteristic)
    }
       
    // ??? What is this for
    func readValue(characteristic: CBCharacteristic) {
        self.displayPeripheral?.readValue(for: characteristic)
    }
    
    func getLocalTimestamp() -> String {
        let now = Date()
        
        let formatter = DateFormatter()

        formatter.timeZone = TimeZone.current

        formatter.dateFormat = "yyyy-MM-dd HH:mm"

        let dateString = formatter.string(from: now)
        return dateString
    }
}

