# OpenWR

OpenWR (OpenWear) is an open hardware standard for wearable IoT devices.  

## Principles of Design

We are promoting groundbreaking ideas:
- Own your own tech stack (lately companies act like they own everything and only grant you a right to use things and feel free to take away features)
- Invest in your platforms, but invest in a forever purchase (when you buy stuff, buy stuff that will last you for your lifetime if possible. Try not to buy things that will be recycled waste in a few years. Is the company providing you will the ability to keep your gear running over time, or will they suddenly cut off support?)
- Own your data and control your privacy(where is your data going? Is the company requiring you to install their app, set up an account, and tracking you? Are they giving you control over who gets to use your data and for what? Do you own your data?)
- Deplatform (Where possible, get off platforms. Find small ways not to live in a FANG universe).

## Hardware

Hardware developed will have open specs that can be built by anyone, under a permissive license.  The associated software will be delivered along with the hardware design.  Where possible, it should be possible to click to build a PCB or find a reference to places to buy hardware builds and components.

## Projects

Projects are divided into prototypes, products, and archives (of old/unsupported projects).

Prototypes are intended to experiment on small demonstrations of functionality, or as iterations on a design before promoting a concept into a product.

A product is a fully supported design with active software development, and some expectation of hardware components being available, and at least one public source to purchase the components or a completed product.

The archives are where products move to that are no longer supported.  This may happen because of lack of available components.  Or it may be due to a change in focus area by the team.

## Concepts and Terminology

We have some concepts from the world of mobile and from the world of bluetooth.  We need to do a reset in terms of how we think of things you wear and things you hold in your hands vs things that are in your living room that don't move.  The average user doesn't understand the protocols, the concepts of client/server master/node etc.  We think in terms of a physical universe.  Laws of physics are generally understood by people.

If you push a button, you expect something to happen.  If you turn something on, you would like to know its state if the state has changed.  If you turn on some appliance device in your house, you expect to be able to find it on your network.  You don't expect to have to create an account and do anything complicated to access the device.  You do expect there to be some minimal level of security.  That said, you should not be beholden to enterprise security standards, and should be able to opt into whatever standard of security suits you.  For things you wear, you would like them to "link up" with each other to form a unified function.  We do this easily with things like belts->pants, socks-> shoes, jacket->liner->shirt.  At the present, other than the Apple Watch or a Garmin, most IoT wearables are not easily matching themselves to enhance functionality of any other devices.

OpenWR hopes to introduce new design principles to adhere to for each project.  It also hopes to introduce new concepts for the kinds of wearables and IoT devices that aren't worn but used in conjenction with the wearable.  

The ideal we seek is building block components in the form of wearables that can connect and work with each other, providing enhanced functionality when set up to work together.  Ideas:

- A camera that can display to a wearable screen (so you can get your phone out of your hand)
- A sensor that collects data and storaes it in some local device, and another device that lets you view this data

### Design Principles of OpenWR

### Device Categories of OpenWR

- Categories: a set of tags that identify the ways this device can be used.  A device may have 1 or more catagories.
  - Viewer/Display
  - Earbuds/Headphones (implies doesn't make audible sound outside of the user)
  - Speaker
  - Sensor
  - Bracelet
  - Necklace
  - Pocket
  - Tracker
  - Storage
  - PowerSource
  - Charger
  - Controller
  - Camera
  - Projector
- Protocols: these are the low level or high level technical protocols supported
  - Zigbee
  - Bluetooth
  - Wifi
  - Wifi6
  - Thread
  - IR
  - Meshtastic
  - GPS
  - NFC
- Physical Interfaces: the set of one or more physical interfaces available
  - USB-C
  - USB-A
  - Grove
  - Proprietary (should list what it is) ... example magsafe
- 6Sensor: Anything you wear potentially acting as augmentation of your existing senses, including things you cannot sense as a human, hence 6 senses are referenced
  - Brain: anything that does processing and performs some intelligent decision making or thinking.  It does not imply any particular technology stack to implement.
  - Eye: any screen capable of seeing
  - Ear: any device capable of hearing
  - Voice: any device capable of speaking or general sound
  - Nose: any device that performs the sensing capabilities of a nose in whatever capability that may provide
  - Metasense: any sense that a meta-human could have ... gas sensor, ir sensor, humidity sensor, etc.  Humans can only vaguely 

## Prototypes (Ideas)

- HeadsOut Display proto1
- Vision Skin proto1 : customizable e-ink
- Vision Indicator Cam proto1
- RearViewsion Camera for VisionPro
- Vision Screen

## Products

TBD

## Specifications

TBD

## Support

File a bug in the issue tracker.

## How to Help

- File bugs / suggestions / pull requests
- Donate: $ETH/$POLY: 0x00000001.zil , or $SOL: truedat101.sol ... proceeds make it possible to just do open source!

## Inspirations and Attributions

- One of the truly inspiring projects in recent weeks/months is Meshtastic.  Hurray for open source.  Many have tried proprietary networks, non-open solutions, etc., and Meshtastic found a great use case for LoRa.
- Ethernet: I had a chance to meet Bob M. and he told me his story of the first ethernet test.  It took place in my office at PARC.  He had my office years before I arrived there.  The ethernet story is worth learning about.  It's why Novell didn't win and Ethernet did win.  Business people might say PARC didn't succeed with Ethernet, but they did spawn what became CISCO and literally every other connected device out there.
- Apple: Before they were the dog, they were the underdog.  Their stuff had principles but they weren't the best for price or performance.  You could love certain things about the imperfection of a Mac.  Nowadays, they are more like an elite athlete, high performance, high price.  Still, I look backwards at where they came from.
