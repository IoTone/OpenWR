//
//  ContentView.swift
//  Blue
//
//  Created by Astra on 3/8/23.
//

import SwiftUI

struct ContentView: View {
    
    @ObservedObject var bleManager = BLEManager()
    @State private var scan = false
    @State private var adv = false
    @State var cmdresult: String = ""
    @State private var selectedMode: DeviceDisplayModes = .available

    let timer = Timer.publish(every: 1, on: .main, in: .common).autoconnect()
    let tenTimes = Timer.publish(every: 0.1, on: .main, in: .common).autoconnect()
    var body: some View {
        VStack (spacing: 10) {
            Text("STATUS")
                .font(.headline)
            
            if bleManager.isSwitchedOn {
                Text("ON")
                    .foregroundColor(.green)
            }
            else {
                Text("OFF")
                    .foregroundColor(.red)
            }
            HStack {
                Spacer()
                Toggle("Scan", isOn: $scan).toggleStyle(SwitchToggleStyle(tint: .red))
                    .onChange(of: scan) { value in
                        // action...
                        self.bleManager.startScanning()
                    }
            }
            Text("Bluetooth Devices")
                .font(.largeTitle)
                .frame(maxWidth: .infinity, alignment: .center)
            List(bleManager.peripherals) { peripheral in
                HStack {
                    Text(peripheral.name)
                    Spacer()
                    Text(String(peripheral.rssi))
                }
            }
            Spacer()
            Text("No Match")
            List(bleManager.noMatchPeripherals) { peripheral in
                HStack {
                    Text(peripheral.name)
                    Spacer()
                    Text(String(peripheral.rssi))
                }
            }
            Spacer()
            if (bleManager.isSwitchedOn && bleManager.isConnected) {
                Text("Result: \(cmdresult)")
                Divider()
                Spacer()
                Picker("Display Mode", selection: $selectedMode) {
                    ForEach(DeviceDisplayModes.allCases) { dispmode in
                        Text(dispmode.rawValue.capitalized)
                    }
                }
        Button(action: bleManager.sendCommand(mode: selectedMode)) {
                    Text("Send Command")
                }
            } else {
                if (bleManager.isSwitchedOn && !bleManager.isConnected) {
                    VStack {
                                // Bleeds into NavigationView
                                Rectangle()
                                    .frame(height: 0)
                                    .background(Color.blue)
                                Text("Result: \(cmdresult)")
                                Divider()
                                Spacer()
                                
                            }
                    .background(Color.red)
                            .navigationTitle(Text("Style"))
                }
                // Text("Time elapsed: \(timeElapsed) sec")
            }
        }
    }
}


struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
            .previewDevice("Apple Vision Pro")
    }
}

public class resetDate {
    var calendar = Date()
    var since = 0
}

