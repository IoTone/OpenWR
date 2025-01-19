//
//  ControllerAttributes.swift
//  HeadsOutController
//
//  Created by David J kordsmeier on 1/19/25.
//

import Foundation
import SwiftUI

/*
#define HEADSOUT_AVAILABLE      0x0
#define HEADSOUT_DONOTDISTURB   0x1
#define HEADSOUT_INMEETING      0x2
#define HEADSOUT_RELAXING       0x3
#define HEADSOUT_HANDLE         0x4
#define HEADSOUT_LOGO           0x5
 */
// https://medium.com/@kalidoss.shanmugam/swift-enums-best-practices-and-hidden-features-cdce09426c38
enum DeviceDisplayModes: String, CaseIterable, Identifiable {
    case available
    case donotdisturb
    case inmeeting
    case relaxing
    case handle
    case logo
    var id: Self { self }
}

enum DeviceDisplayModeIds: Int {
    case available = 0x0
    case donotdisturb = 0x1
    case inmeeting = 0x2
    case relaxing = 0x3
    case handle = 0x4
    case logo = 0x5
    case undefined = 0xf
    var id: Self { self }
}

func convertDeviceDisplayModeToId(mode: String) -> DeviceDisplayModeIds {
    if mode == DeviceDisplayModes.available.rawValue.lowercased() {
        return DeviceDisplayModeIds.available
    } else if mode == DeviceDisplayModes.donotdisturb.rawValue.lowercased() {
        return DeviceDisplayModeIds.donotdisturb
    } else if mode == DeviceDisplayModes.inmeeting.rawValue.lowercased() {
        return DeviceDisplayModeIds.inmeeting
    } else if mode == DeviceDisplayModes.relaxing.rawValue.lowercased() {
        return DeviceDisplayModeIds.relaxing
    } else if mode == DeviceDisplayModes.handle.rawValue.lowercased() {
        return DeviceDisplayModeIds.handle
    } else if mode == DeviceDisplayModes.logo.rawValue.lowercased() {
        return DeviceDisplayModeIds.logo
    } else {
        return DeviceDisplayModeIds.undefined
    }
}
