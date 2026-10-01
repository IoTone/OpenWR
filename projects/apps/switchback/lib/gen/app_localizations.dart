import 'dart:async';

import 'package:flutter/foundation.dart';
import 'package:flutter/widgets.dart';
import 'package:flutter_localizations/flutter_localizations.dart';
import 'package:intl/intl.dart' as intl;

import 'app_localizations_en.dart';
import 'app_localizations_ja.dart';

// ignore_for_file: type=lint

/// Callers can lookup localized strings with an instance of AppLocalizations
/// returned by `AppLocalizations.of(context)`.
///
/// Applications need to include `AppLocalizations.delegate()` in their app's
/// `localizationDelegates` list, and the locales they support in the app's
/// `supportedLocales` list. For example:
///
/// ```dart
/// import 'gen/app_localizations.dart';
///
/// return MaterialApp(
///   localizationsDelegates: AppLocalizations.localizationsDelegates,
///   supportedLocales: AppLocalizations.supportedLocales,
///   home: MyApplicationHome(),
/// );
/// ```
///
/// ## Update pubspec.yaml
///
/// Please make sure to update your pubspec.yaml to include the following
/// packages:
///
/// ```yaml
/// dependencies:
///   # Internationalization support.
///   flutter_localizations:
///     sdk: flutter
///   intl: any # Use the pinned version from flutter_localizations
///
///   # Rest of dependencies
/// ```
///
/// ## iOS Applications
///
/// iOS applications define key application metadata, including supported
/// locales, in an Info.plist file that is built into the application bundle.
/// To configure the locales supported by your app, you’ll need to edit this
/// file.
///
/// First, open your project’s ios/Runner.xcworkspace Xcode workspace file.
/// Then, in the Project Navigator, open the Info.plist file under the Runner
/// project’s Runner folder.
///
/// Next, select the Information Property List item, select Add Item from the
/// Editor menu, then select Localizations from the pop-up menu.
///
/// Select and expand the newly-created Localizations item then, for each
/// locale your application supports, add a new item and select the locale
/// you wish to add from the pop-up menu in the Value field. This list should
/// be consistent with the languages listed in the AppLocalizations.supportedLocales
/// property.
abstract class AppLocalizations {
  AppLocalizations(String locale)
      : localeName = intl.Intl.canonicalizedLocale(locale.toString());

  final String localeName;

  static AppLocalizations of(BuildContext context) {
    return Localizations.of<AppLocalizations>(context, AppLocalizations)!;
  }

  static const LocalizationsDelegate<AppLocalizations> delegate =
      _AppLocalizationsDelegate();

  /// A list of this localizations delegate along with the default localizations
  /// delegates.
  ///
  /// Returns a list of localizations delegates containing this delegate along with
  /// GlobalMaterialLocalizations.delegate, GlobalCupertinoLocalizations.delegate,
  /// and GlobalWidgetsLocalizations.delegate.
  ///
  /// Additional delegates can be added by appending to this list in
  /// MaterialApp. This list does not have to be used at all if a custom list
  /// of delegates is preferred or required.
  static const List<LocalizationsDelegate<dynamic>> localizationsDelegates =
      <LocalizationsDelegate<dynamic>>[
    delegate,
    GlobalMaterialLocalizations.delegate,
    GlobalCupertinoLocalizations.delegate,
    GlobalWidgetsLocalizations.delegate,
  ];

  /// A list of this localizations delegate's supported locales.
  static const List<Locale> supportedLocales = <Locale>[
    Locale('en'),
    Locale('ja')
  ];

  /// No description provided for @appTitle.
  ///
  /// In en, this message translates to:
  /// **'Switchback'**
  String get appTitle;

  /// No description provided for @aboutSubtitle.
  ///
  /// In en, this message translates to:
  /// **'the OpenWR app'**
  String get aboutSubtitle;

  /// No description provided for @aboutVersion.
  ///
  /// In en, this message translates to:
  /// **'Version {version}'**
  String aboutVersion(String version);

  /// No description provided for @aboutDescription.
  ///
  /// In en, this message translates to:
  /// **'Pair, set up, dress and update your goggl1 over Bluetooth, and run sessions with the glass in front of your eye. Open. Weird. Wild. Strong. #PNWbuilt'**
  String get aboutDescription;

  /// No description provided for @aboutCopyright.
  ///
  /// In en, this message translates to:
  /// **'© 2024–2026 IoTone, Inc.'**
  String get aboutCopyright;

  /// No description provided for @aboutLicense.
  ///
  /// In en, this message translates to:
  /// **'MIT License'**
  String get aboutLicense;

  /// No description provided for @aboutLicensesButton.
  ///
  /// In en, this message translates to:
  /// **'Open-source licenses'**
  String get aboutLicensesButton;

  /// No description provided for @aboutMadeWith.
  ///
  /// In en, this message translates to:
  /// **'Built with Flutter.'**
  String get aboutMadeWith;

  /// No description provided for @quickNav.
  ///
  /// In en, this message translates to:
  /// **'Quick navigation'**
  String get quickNav;

  /// No description provided for @commonCancel.
  ///
  /// In en, this message translates to:
  /// **'Cancel'**
  String get commonCancel;

  /// No description provided for @commonClose.
  ///
  /// In en, this message translates to:
  /// **'Close'**
  String get commonClose;

  /// No description provided for @commonRetry.
  ///
  /// In en, this message translates to:
  /// **'Retry'**
  String get commonRetry;

  /// No description provided for @commonCopy.
  ///
  /// In en, this message translates to:
  /// **'Copy'**
  String get commonCopy;

  /// No description provided for @commonCopied.
  ///
  /// In en, this message translates to:
  /// **'Copied'**
  String get commonCopied;

  /// No description provided for @scanStart.
  ///
  /// In en, this message translates to:
  /// **'Start scan'**
  String get scanStart;

  /// No description provided for @scanStop.
  ///
  /// In en, this message translates to:
  /// **'Stop scan'**
  String get scanStop;

  /// No description provided for @scanScanning.
  ///
  /// In en, this message translates to:
  /// **'Scanning…'**
  String get scanScanning;

  /// No description provided for @scanIdle.
  ///
  /// In en, this message translates to:
  /// **'Idle'**
  String get scanIdle;

  /// No description provided for @bluetoothOn.
  ///
  /// In en, this message translates to:
  /// **'Bluetooth ready'**
  String get bluetoothOn;

  /// No description provided for @bluetoothOff.
  ///
  /// In en, this message translates to:
  /// **'Bluetooth is off'**
  String get bluetoothOff;

  /// No description provided for @bluetoothUnauthorized.
  ///
  /// In en, this message translates to:
  /// **'Bluetooth permission needed'**
  String get bluetoothUnauthorized;

  /// No description provided for @bluetoothUnknown.
  ///
  /// In en, this message translates to:
  /// **'Bluetooth state unknown'**
  String get bluetoothUnknown;

  /// No description provided for @radarEmpty.
  ///
  /// In en, this message translates to:
  /// **'No devices in range yet.'**
  String get radarEmpty;

  /// No description provided for @radarSelf.
  ///
  /// In en, this message translates to:
  /// **'You'**
  String get radarSelf;

  /// No description provided for @radarLegendBle.
  ///
  /// In en, this message translates to:
  /// **'Bluetooth device'**
  String get radarLegendBle;

  /// No description provided for @radarLegendWifi.
  ///
  /// In en, this message translates to:
  /// **'WiFi network'**
  String get radarLegendWifi;

  /// No description provided for @radarLegendRings.
  ///
  /// In en, this message translates to:
  /// **'Rings = approximate signal distance (rough).'**
  String get radarLegendRings;

  /// No description provided for @rssiDbm.
  ///
  /// In en, this message translates to:
  /// **'{rssi} dBm'**
  String rssiDbm(int rssi);

  /// No description provided for @metersApprox.
  ///
  /// In en, this message translates to:
  /// **'≈ {meters} m'**
  String metersApprox(String meters);

  /// No description provided for @signalStrong.
  ///
  /// In en, this message translates to:
  /// **'Strong'**
  String get signalStrong;

  /// No description provided for @signalMedium.
  ///
  /// In en, this message translates to:
  /// **'Medium'**
  String get signalMedium;

  /// No description provided for @signalWeak.
  ///
  /// In en, this message translates to:
  /// **'Weak'**
  String get signalWeak;

  /// No description provided for @deviceConnect.
  ///
  /// In en, this message translates to:
  /// **'Connect'**
  String get deviceConnect;

  /// No description provided for @settingsAppearance.
  ///
  /// In en, this message translates to:
  /// **'Appearance'**
  String get settingsAppearance;

  /// No description provided for @settingsTheme.
  ///
  /// In en, this message translates to:
  /// **'Theme'**
  String get settingsTheme;

  /// No description provided for @settingsFontScale.
  ///
  /// In en, this message translates to:
  /// **'Text size'**
  String get settingsFontScale;

  /// No description provided for @settingsReduceMotion.
  ///
  /// In en, this message translates to:
  /// **'Reduce motion'**
  String get settingsReduceMotion;

  /// No description provided for @settingsHighContrast.
  ///
  /// In en, this message translates to:
  /// **'High contrast'**
  String get settingsHighContrast;

  /// No description provided for @settingsLanguage.
  ///
  /// In en, this message translates to:
  /// **'Language'**
  String get settingsLanguage;

  /// No description provided for @settingsLanguageSystem.
  ///
  /// In en, this message translates to:
  /// **'System default'**
  String get settingsLanguageSystem;

  /// No description provided for @settingsPermissions.
  ///
  /// In en, this message translates to:
  /// **'Permissions'**
  String get settingsPermissions;

  /// No description provided for @settingsDiagnostics.
  ///
  /// In en, this message translates to:
  /// **'Diagnostics'**
  String get settingsDiagnostics;

  /// No description provided for @settingsDiagnosticsSub.
  ///
  /// In en, this message translates to:
  /// **'Scan event log'**
  String get settingsDiagnosticsSub;

  /// No description provided for @settingsAbout.
  ///
  /// In en, this message translates to:
  /// **'About'**
  String get settingsAbout;

  /// No description provided for @settingsScanner.
  ///
  /// In en, this message translates to:
  /// **'Scanner'**
  String get settingsScanner;

  /// No description provided for @settingsRssiFloor.
  ///
  /// In en, this message translates to:
  /// **'Signal floor'**
  String get settingsRssiFloor;

  /// No description provided for @settingsRssiFloorValue.
  ///
  /// In en, this message translates to:
  /// **'Hide signals weaker than {dbm} dBm'**
  String settingsRssiFloorValue(int dbm);

  /// No description provided for @settingsRssiFloorOff.
  ///
  /// In en, this message translates to:
  /// **'Show all signals'**
  String get settingsRssiFloorOff;

  /// No description provided for @permBle.
  ///
  /// In en, this message translates to:
  /// **'Bluetooth'**
  String get permBle;

  /// No description provided for @permLocation.
  ///
  /// In en, this message translates to:
  /// **'Location'**
  String get permLocation;

  /// No description provided for @permLocationWhy.
  ///
  /// In en, this message translates to:
  /// **'Android requires location permission to scan for nearby devices.'**
  String get permLocationWhy;

  /// No description provided for @permGranted.
  ///
  /// In en, this message translates to:
  /// **'Granted'**
  String get permGranted;

  /// No description provided for @permDenied.
  ///
  /// In en, this message translates to:
  /// **'Denied'**
  String get permDenied;

  /// No description provided for @permRequest.
  ///
  /// In en, this message translates to:
  /// **'Request'**
  String get permRequest;

  /// No description provided for @permOpenSettings.
  ///
  /// In en, this message translates to:
  /// **'Open settings'**
  String get permOpenSettings;

  /// No description provided for @diagEmpty.
  ///
  /// In en, this message translates to:
  /// **'No scan activity yet.'**
  String get diagEmpty;

  /// No description provided for @diagClear.
  ///
  /// In en, this message translates to:
  /// **'Clear'**
  String get diagClear;

  /// No description provided for @diagCopy.
  ///
  /// In en, this message translates to:
  /// **'Copy log'**
  String get diagCopy;

  /// No description provided for @firstRunTitle.
  ///
  /// In en, this message translates to:
  /// **'Welcome to Switchback'**
  String get firstRunTitle;

  /// No description provided for @firstRunBody.
  ///
  /// In en, this message translates to:
  /// **'This app talks to your goggl1 over Bluetooth. It needs a couple of permissions to find it.'**
  String get firstRunBody;

  /// No description provided for @firstRunPermBle.
  ///
  /// In en, this message translates to:
  /// **'Bluetooth — to find and talk to the goggle.'**
  String get firstRunPermBle;

  /// No description provided for @firstRunPermLocation.
  ///
  /// In en, this message translates to:
  /// **'Location — Android requires it for Bluetooth scanning.'**
  String get firstRunPermLocation;

  /// No description provided for @firstRunGrant.
  ///
  /// In en, this message translates to:
  /// **'Grant permissions'**
  String get firstRunGrant;

  /// No description provided for @firstRunSkip.
  ///
  /// In en, this message translates to:
  /// **'Skip for now'**
  String get firstRunSkip;

  /// No description provided for @langEnglish.
  ///
  /// In en, this message translates to:
  /// **'English'**
  String get langEnglish;

  /// No description provided for @langJapanese.
  ///
  /// In en, this message translates to:
  /// **'日本語'**
  String get langJapanese;

  /// No description provided for @settingsTitle.
  ///
  /// In en, this message translates to:
  /// **'Settings'**
  String get settingsTitle;

  /// No description provided for @settingsExperience.
  ///
  /// In en, this message translates to:
  /// **'Experience'**
  String get settingsExperience;

  /// No description provided for @tabTrailhead.
  ///
  /// In en, this message translates to:
  /// **'Trailhead'**
  String get tabTrailhead;

  /// No description provided for @tabSession.
  ///
  /// In en, this message translates to:
  /// **'Session'**
  String get tabSession;

  /// No description provided for @tabLooks.
  ///
  /// In en, this message translates to:
  /// **'Looks'**
  String get tabLooks;

  /// No description provided for @tabConsole.
  ///
  /// In en, this message translates to:
  /// **'Console'**
  String get tabConsole;

  /// No description provided for @tabUpdate.
  ///
  /// In en, this message translates to:
  /// **'Update'**
  String get tabUpdate;

  /// No description provided for @experiencePinned.
  ///
  /// In en, this message translates to:
  /// **'Pinned'**
  String get experiencePinned;

  /// No description provided for @experienceAuto.
  ///
  /// In en, this message translates to:
  /// **'Auto'**
  String get experienceAuto;

  /// No description provided for @experienceNow.
  ///
  /// In en, this message translates to:
  /// **'Right now: {name}'**
  String experienceNow(String name);

  /// No description provided for @feelsLike.
  ///
  /// In en, this message translates to:
  /// **'feels {t}'**
  String feelsLike(int t);

  /// No description provided for @hiLo.
  ///
  /// In en, this message translates to:
  /// **'hi {hi} · lo {lo}'**
  String hiLo(int hi, int lo);

  /// No description provided for @windNow.
  ///
  /// In en, this message translates to:
  /// **'wind {v} {unit} {dir}'**
  String windNow(int v, String unit, String dir);

  /// No description provided for @chipDry8h.
  ///
  /// In en, this message translates to:
  /// **'dry 8h'**
  String get chipDry8h;

  /// No description provided for @chipRainAt.
  ///
  /// In en, this message translates to:
  /// **'rain {t}'**
  String chipRainAt(String t);

  /// No description provided for @chipUnits.
  ///
  /// In en, this message translates to:
  /// **'{u}'**
  String chipUnits(String u);

  /// No description provided for @trailheadNoForecastTitle.
  ///
  /// In en, this message translates to:
  /// **'No forecast yet'**
  String get trailheadNoForecastTitle;

  /// No description provided for @trailheadWaitingForecast.
  ///
  /// In en, this message translates to:
  /// **'The goggle is connected and fetching. Give it a few seconds, or check it has WiFi in Console.'**
  String get trailheadWaitingForecast;

  /// No description provided for @trailheadNotConnected.
  ///
  /// In en, this message translates to:
  /// **'Pair your goggle to see what it sees.'**
  String get trailheadNotConnected;

  /// No description provided for @startSession.
  ///
  /// In en, this message translates to:
  /// **'Start a session'**
  String get startSession;

  /// No description provided for @pairGoggle.
  ///
  /// In en, this message translates to:
  /// **'Pair goggle'**
  String get pairGoggle;

  /// No description provided for @headlineStormNow.
  ///
  /// In en, this message translates to:
  /// **'Thunderstorm now. Stay in.'**
  String get headlineStormNow;

  /// No description provided for @headlineWetAllWindow.
  ///
  /// In en, this message translates to:
  /// **'Wet for the next eight hours.'**
  String get headlineWetAllWindow;

  /// No description provided for @headlineClearsAt.
  ///
  /// In en, this message translates to:
  /// **'Clears around {t}.'**
  String headlineClearsAt(String t);

  /// No description provided for @headlineRainAt.
  ///
  /// In en, this message translates to:
  /// **'Rain from {t}, {pop}% likely. Go before it.'**
  String headlineRainAt(String t, int pop);

  /// No description provided for @headlineWindy.
  ///
  /// In en, this message translates to:
  /// **'Dry, but gusts to {g}. Pick a sheltered route.'**
  String headlineWindy(int g);

  /// No description provided for @headlineDryDay.
  ///
  /// In en, this message translates to:
  /// **'Dry for the next eight hours. Go.'**
  String get headlineDryDay;

  /// No description provided for @headlineDryNight.
  ///
  /// In en, this message translates to:
  /// **'Clear and dry tonight. Headlamp on.'**
  String get headlineDryNight;

  /// No description provided for @squallWarning.
  ///
  /// In en, this message translates to:
  /// **'Rain {t} · {pop}% · gusts {gust} {unit}'**
  String squallWarning(String t, int pop, int gust, String unit);

  /// No description provided for @goggleNotConnected.
  ///
  /// In en, this message translates to:
  /// **'Not connected'**
  String get goggleNotConnected;

  /// No description provided for @goggleConnecting.
  ///
  /// In en, this message translates to:
  /// **'Connecting…'**
  String get goggleConnecting;

  /// No description provided for @goggleConnected.
  ///
  /// In en, this message translates to:
  /// **'Connected'**
  String get goggleConnected;

  /// No description provided for @ageUnknown.
  ///
  /// In en, this message translates to:
  /// **'no clock'**
  String get ageUnknown;

  /// No description provided for @ageJustNow.
  ///
  /// In en, this message translates to:
  /// **'just now'**
  String get ageJustNow;

  /// No description provided for @ageMinutes.
  ///
  /// In en, this message translates to:
  /// **'{n} min ago'**
  String ageMinutes(int n);

  /// No description provided for @ageHours.
  ///
  /// In en, this message translates to:
  /// **'{n} h ago'**
  String ageHours(int n);

  /// No description provided for @pairTitle.
  ///
  /// In en, this message translates to:
  /// **'Pair'**
  String get pairTitle;

  /// No description provided for @pairHeadline.
  ///
  /// In en, this message translates to:
  /// **'Hold BOOT for a second.'**
  String get pairHeadline;

  /// No description provided for @pairBody.
  ///
  /// In en, this message translates to:
  /// **'The glass shows a B and the goggle listens for three minutes. Switchback finds it and connects.'**
  String get pairBody;

  /// No description provided for @pairScanning.
  ///
  /// In en, this message translates to:
  /// **'Looking for goggl1…'**
  String get pairScanning;

  /// No description provided for @pairFailed.
  ///
  /// In en, this message translates to:
  /// **'Could not connect. Hold BOOT again and retry.'**
  String get pairFailed;

  /// No description provided for @pairNothingYet.
  ///
  /// In en, this message translates to:
  /// **'Nothing yet. Is the B showing on the glass?'**
  String get pairNothingYet;

  /// No description provided for @pairFindRadar.
  ///
  /// In en, this message translates to:
  /// **'Find it on the radar'**
  String get pairFindRadar;

  /// No description provided for @radarLegendGoggle.
  ///
  /// In en, this message translates to:
  /// **'goggl1'**
  String get radarLegendGoggle;

  /// No description provided for @looksGlassTitle.
  ///
  /// In en, this message translates to:
  /// **'On the glass'**
  String get looksGlassTitle;

  /// No description provided for @looksGlassBody.
  ///
  /// In en, this message translates to:
  /// **'Previews are the goggle\'s own renderer. Tap one to switch.'**
  String get looksGlassBody;

  /// No description provided for @looksNotConnected.
  ///
  /// In en, this message translates to:
  /// **'Pair the goggle to change what it shows.'**
  String get looksNotConnected;

  /// No description provided for @looksAnimation.
  ///
  /// In en, this message translates to:
  /// **'Animation'**
  String get looksAnimation;

  /// No description provided for @looksAnimationSub.
  ///
  /// In en, this message translates to:
  /// **'Rain falls, snow drifts, storms flash, stars twinkle'**
  String get looksAnimationSub;

  /// No description provided for @looksBrightness.
  ///
  /// In en, this message translates to:
  /// **'Glass brightness'**
  String get looksBrightness;

  /// No description provided for @looksAppTitle.
  ///
  /// In en, this message translates to:
  /// **'This app'**
  String get looksAppTitle;

  /// No description provided for @looksAppBody.
  ///
  /// In en, this message translates to:
  /// **'The look follows the weather the goggle sees. Pin one to keep it.'**
  String get looksAppBody;

  /// No description provided for @looksRedShift.
  ///
  /// In en, this message translates to:
  /// **'Red shift'**
  String get looksRedShift;

  /// No description provided for @looksRedShiftSub.
  ///
  /// In en, this message translates to:
  /// **'Headlamp goes deep red for night vision'**
  String get looksRedShiftSub;

  /// No description provided for @glassStripBlurb.
  ///
  /// In en, this message translates to:
  /// **'A chart. The shape of the next eight hours.'**
  String get glassStripBlurb;

  /// No description provided for @glassGlanceBlurb.
  ///
  /// In en, this message translates to:
  /// **'Big numbers and one sentence.'**
  String get glassGlanceBlurb;

  /// No description provided for @glassRibbonBlurb.
  ///
  /// In en, this message translates to:
  /// **'Every hour as a column.'**
  String get glassRibbonBlurb;

  /// No description provided for @consoleNotConnected.
  ///
  /// In en, this message translates to:
  /// **'The console needs a goggle.'**
  String get consoleNotConnected;

  /// No description provided for @consoleHistory.
  ///
  /// In en, this message translates to:
  /// **'Previous line'**
  String get consoleHistory;

  /// No description provided for @consoleSend.
  ///
  /// In en, this message translates to:
  /// **'Send'**
  String get consoleSend;

  /// No description provided for @sessionTitle.
  ///
  /// In en, this message translates to:
  /// **'Session'**
  String get sessionTitle;

  /// No description provided for @sessionBody.
  ///
  /// In en, this message translates to:
  /// **'The phone is the sensor hub, the glass is the display. Pick a sport and start: time, pace and distance go to the goggle once a second.'**
  String get sessionBody;

  /// No description provided for @sportRun.
  ///
  /// In en, this message translates to:
  /// **'Run'**
  String get sportRun;

  /// No description provided for @sportRide.
  ///
  /// In en, this message translates to:
  /// **'Ride'**
  String get sportRide;

  /// No description provided for @sportSki.
  ///
  /// In en, this message translates to:
  /// **'Ski'**
  String get sportSki;

  /// No description provided for @sportPaddle.
  ///
  /// In en, this message translates to:
  /// **'Paddle'**
  String get sportPaddle;

  /// No description provided for @sportClimb.
  ///
  /// In en, this message translates to:
  /// **'Climb'**
  String get sportClimb;

  /// No description provided for @sessionComingSoon.
  ///
  /// In en, this message translates to:
  /// **'Sessions arrive in the next build.'**
  String get sessionComingSoon;

  /// No description provided for @sessionPhaseNote.
  ///
  /// In en, this message translates to:
  /// **'Coming: the session-hud bundle, GPS pace and distance, a heart-rate strap.'**
  String get sessionPhaseNote;

  /// No description provided for @updateTitle.
  ///
  /// In en, this message translates to:
  /// **'Update'**
  String get updateTitle;

  /// No description provided for @updateRunning.
  ///
  /// In en, this message translates to:
  /// **'Running v{v} with {b}.'**
  String updateRunning(String v, String b);

  /// No description provided for @updateReadDetails.
  ///
  /// In en, this message translates to:
  /// **'Read details'**
  String get updateReadDetails;

  /// No description provided for @updateOtaTitle.
  ///
  /// In en, this message translates to:
  /// **'Firmware over the air'**
  String get updateOtaTitle;

  /// No description provided for @updateOtaBody.
  ///
  /// In en, this message translates to:
  /// **'Serve the image on the same WiFi as the goggle and paste its URL. The goggle writes the spare slot and reboots; a bad image rolls back.'**
  String get updateOtaBody;

  /// No description provided for @updateUrlLabel.
  ///
  /// In en, this message translates to:
  /// **'Image URL'**
  String get updateUrlLabel;

  /// No description provided for @updateStart.
  ///
  /// In en, this message translates to:
  /// **'Update'**
  String get updateStart;

  /// No description provided for @updateConfirmTitle.
  ///
  /// In en, this message translates to:
  /// **'Update firmware?'**
  String get updateConfirmTitle;

  /// No description provided for @updateConfirmBody.
  ///
  /// In en, this message translates to:
  /// **'The goggle will fetch {url}, write it, and reboot.'**
  String updateConfirmBody(String url);

  /// No description provided for @updateStarted.
  ///
  /// In en, this message translates to:
  /// **'Update started. Progress shows in Console.'**
  String get updateStarted;

  /// No description provided for @updatePhaseNote.
  ///
  /// In en, this message translates to:
  /// **'Coming: Switchback serves the image itself, and pushes use-case bundles.'**
  String get updatePhaseNote;

  /// No description provided for @setupTitle.
  ///
  /// In en, this message translates to:
  /// **'Goggle setup'**
  String get setupTitle;

  /// No description provided for @setupSettingsSub.
  ///
  /// In en, this message translates to:
  /// **'WiFi, location, units'**
  String get setupSettingsSub;

  /// No description provided for @setupNotConnected.
  ///
  /// In en, this message translates to:
  /// **'Pair the goggle first; the cards send as you go.'**
  String get setupNotConnected;

  /// No description provided for @setupWifiTitle.
  ///
  /// In en, this message translates to:
  /// **'WiFi'**
  String get setupWifiTitle;

  /// No description provided for @setupWifiBody.
  ///
  /// In en, this message translates to:
  /// **'The goggle fetches its own forecast, so it needs a network. 2.4 GHz only.'**
  String get setupWifiBody;

  /// No description provided for @setupWifiPick.
  ///
  /// In en, this message translates to:
  /// **'Pick a network'**
  String get setupWifiPick;

  /// No description provided for @setupWifiSsid.
  ///
  /// In en, this message translates to:
  /// **'Network name'**
  String get setupWifiSsid;

  /// No description provided for @setupWifiPassword.
  ///
  /// In en, this message translates to:
  /// **'Password'**
  String get setupWifiPassword;

  /// No description provided for @setupWifiSend.
  ///
  /// In en, this message translates to:
  /// **'Send to goggle'**
  String get setupWifiSend;

  /// No description provided for @setupWifiSending.
  ///
  /// In en, this message translates to:
  /// **'Sending, waiting for an address…'**
  String get setupWifiSending;

  /// No description provided for @setupWifiUp.
  ///
  /// In en, this message translates to:
  /// **'Connected, {ip}.'**
  String setupWifiUp(String ip);

  /// No description provided for @setupWifiNotYet.
  ///
  /// In en, this message translates to:
  /// **'No address after 30 s. Check the password, or it may still be joining.'**
  String get setupWifiNotYet;

  /// No description provided for @setupWifiState.
  ///
  /// In en, this message translates to:
  /// **'on {ip}'**
  String setupWifiState(String ip);

  /// No description provided for @setupWifiOff.
  ///
  /// In en, this message translates to:
  /// **'no network'**
  String get setupWifiOff;

  /// No description provided for @setupLocationTitle.
  ///
  /// In en, this message translates to:
  /// **'Location'**
  String get setupLocationTitle;

  /// No description provided for @setupLocationBody.
  ///
  /// In en, this message translates to:
  /// **'Left alone, the goggle places itself by its IP address, to the city. Set it by hand or from this phone for the exact spot, which also stops the IP lookup.'**
  String get setupLocationBody;

  /// No description provided for @setupLocationUsePhone.
  ///
  /// In en, this message translates to:
  /// **'Use phone location'**
  String get setupLocationUsePhone;

  /// No description provided for @setupLocationSetManual.
  ///
  /// In en, this message translates to:
  /// **'Set'**
  String get setupLocationSetManual;

  /// No description provided for @setupLocationBackToAuto.
  ///
  /// In en, this message translates to:
  /// **'Back to automatic'**
  String get setupLocationBackToAuto;

  /// No description provided for @setupLocationReading.
  ///
  /// In en, this message translates to:
  /// **'Reading the phone\'s position…'**
  String get setupLocationReading;

  /// No description provided for @setupLocationDenied.
  ///
  /// In en, this message translates to:
  /// **'Location permission denied.'**
  String get setupLocationDenied;

  /// No description provided for @setupLocationInvalid.
  ///
  /// In en, this message translates to:
  /// **'Latitude and longitude please, in degrees.'**
  String get setupLocationInvalid;

  /// No description provided for @setupLocationSending.
  ///
  /// In en, this message translates to:
  /// **'Sending and fetching the forecast…'**
  String get setupLocationSending;

  /// No description provided for @setupLocationSet.
  ///
  /// In en, this message translates to:
  /// **'Set. Forecast for {city}.'**
  String setupLocationSet(String city);

  /// No description provided for @setupLocationLookingUp.
  ///
  /// In en, this message translates to:
  /// **'Looking the IP up…'**
  String get setupLocationLookingUp;

  /// No description provided for @setupLocationLookupFailed.
  ///
  /// In en, this message translates to:
  /// **'Lookup failed; the old location stays.'**
  String get setupLocationLookupFailed;

  /// No description provided for @setupLocationManual.
  ///
  /// In en, this message translates to:
  /// **'{city}, by hand'**
  String setupLocationManual(String city);

  /// No description provided for @setupLocationAuto.
  ///
  /// In en, this message translates to:
  /// **'{city}, from IP'**
  String setupLocationAuto(String city);

  /// No description provided for @setupUnitsTitle.
  ///
  /// In en, this message translates to:
  /// **'Units'**
  String get setupUnitsTitle;

  /// No description provided for @setupUnitsBody.
  ///
  /// In en, this message translates to:
  /// **'Picked from the country by default. The glass and this app follow.'**
  String get setupUnitsBody;

  /// No description provided for @setupUnitsMetric.
  ///
  /// In en, this message translates to:
  /// **'°C · km/h'**
  String get setupUnitsMetric;

  /// No description provided for @setupUnitsImperial.
  ///
  /// In en, this message translates to:
  /// **'°F · mph'**
  String get setupUnitsImperial;

  /// No description provided for @updateNotConnected.
  ///
  /// In en, this message translates to:
  /// **'Pair the goggle to see what it is running.'**
  String get updateNotConnected;

  /// No description provided for @updateEmbeddedTitle.
  ///
  /// In en, this message translates to:
  /// **'Releases in this app'**
  String get updateEmbeddedTitle;

  /// No description provided for @updateEmbeddedBody.
  ///
  /// In en, this message translates to:
  /// **'Signed .mnpkg packages built into Switchback. The phone serves one over your WiFi and the goggle checks the signature, hash and target before it writes a byte.'**
  String get updateEmbeddedBody;

  /// No description provided for @updateNoReleases.
  ///
  /// In en, this message translates to:
  /// **'This build carries no releases.'**
  String get updateNoReleases;

  /// No description provided for @updateInstall.
  ///
  /// In en, this message translates to:
  /// **'Install'**
  String get updateInstall;

  /// No description provided for @updateInstalled.
  ///
  /// In en, this message translates to:
  /// **'running'**
  String get updateInstalled;

  /// No description provided for @updateServing.
  ///
  /// In en, this message translates to:
  /// **'Serving…'**
  String get updateServing;

  /// No description provided for @updateServingAt.
  ///
  /// In en, this message translates to:
  /// **'serving on {ip}:{port}, {n} fetched'**
  String updateServingAt(String ip, int port, int n);

  /// No description provided for @updateInstallBody.
  ///
  /// In en, this message translates to:
  /// **'Install goggl1 {fw} ({version}, {variant})? The goggle fetches it from this phone, verifies it, and reboots into it.'**
  String updateInstallBody(String fw, String version, String variant);

  /// No description provided for @updateProgress.
  ///
  /// In en, this message translates to:
  /// **'Writing… {pct}%'**
  String updateProgress(int pct);

  /// No description provided for @updateRebooting.
  ///
  /// In en, this message translates to:
  /// **'Verified and staged. The goggle is rebooting into it; pair again in a few seconds.'**
  String get updateRebooting;

  /// No description provided for @updateNoVerdict.
  ///
  /// In en, this message translates to:
  /// **'No verdict within 150 s. Check the Console.'**
  String get updateNoVerdict;

  /// No description provided for @updateBundleTitle.
  ///
  /// In en, this message translates to:
  /// **'Use case in this app'**
  String get updateBundleTitle;

  /// No description provided for @updateBundleBody.
  ///
  /// In en, this message translates to:
  /// **'The weather HUD bundle as shipped with this build. Pushing it replaces what the goggle has and keeps it across reboots.'**
  String get updateBundleBody;

  /// No description provided for @updateBundlePush.
  ///
  /// In en, this message translates to:
  /// **'Push weather HUD'**
  String get updateBundlePush;

  /// No description provided for @updateBundleSending.
  ///
  /// In en, this message translates to:
  /// **'Pushing bundle…'**
  String get updateBundleSending;

  /// No description provided for @pairFound.
  ///
  /// In en, this message translates to:
  /// **'Found {n} goggle(s)'**
  String pairFound(int n);

  /// No description provided for @pairPhaseConnecting.
  ///
  /// In en, this message translates to:
  /// **'Connecting, attempt {n} of 2…'**
  String pairPhaseConnecting(int n);

  /// No description provided for @pairPhaseReading.
  ///
  /// In en, this message translates to:
  /// **'Connected, reading status…'**
  String get pairPhaseReading;

  /// No description provided for @pairPhaseRetrying.
  ///
  /// In en, this message translates to:
  /// **'First link dropped, retrying (attempt {n})…'**
  String pairPhaseRetrying(int n);

  /// No description provided for @pairPhaseConnected.
  ///
  /// In en, this message translates to:
  /// **'Connected'**
  String get pairPhaseConnected;

  /// No description provided for @pairPhaseFailed.
  ///
  /// In en, this message translates to:
  /// **'Could not connect'**
  String get pairPhaseFailed;

  /// No description provided for @pairPhaseDropped.
  ///
  /// In en, this message translates to:
  /// **'The goggle disconnected'**
  String get pairPhaseDropped;

  /// No description provided for @pairFailedHint.
  ///
  /// In en, this message translates to:
  /// **'The goggle advertises for 3 minutes after power-up and after any drop. Plug it in again if the B is gone from the glass, then tap Connect.'**
  String get pairFailedHint;

  /// No description provided for @looksSending.
  ///
  /// In en, this message translates to:
  /// **'Sending…'**
  String get looksSending;

  /// No description provided for @looksSendFailed.
  ///
  /// In en, this message translates to:
  /// **'The goggle did not answer: {e}'**
  String looksSendFailed(String e);

  /// No description provided for @sessionGlassReady.
  ///
  /// In en, this message translates to:
  /// **'The goggle is connected; it will show the session.'**
  String get sessionGlassReady;

  /// No description provided for @sessionNoGlass.
  ///
  /// In en, this message translates to:
  /// **'No goggle connected. The session runs on the phone alone.'**
  String get sessionNoGlass;

  /// No description provided for @sessionPace.
  ///
  /// In en, this message translates to:
  /// **'min/{unit}'**
  String sessionPace(String unit);

  /// No description provided for @sessionStop.
  ///
  /// In en, this message translates to:
  /// **'Stop'**
  String get sessionStop;

  /// No description provided for @sessionDone.
  ///
  /// In en, this message translates to:
  /// **'Done: {time}, {dist} {unit}.'**
  String sessionDone(String time, String dist, String unit);

  /// No description provided for @sessionLogTitle.
  ///
  /// In en, this message translates to:
  /// **'Recent sessions'**
  String get sessionLogTitle;
}

class _AppLocalizationsDelegate
    extends LocalizationsDelegate<AppLocalizations> {
  const _AppLocalizationsDelegate();

  @override
  Future<AppLocalizations> load(Locale locale) {
    return SynchronousFuture<AppLocalizations>(lookupAppLocalizations(locale));
  }

  @override
  bool isSupported(Locale locale) =>
      <String>['en', 'ja'].contains(locale.languageCode);

  @override
  bool shouldReload(_AppLocalizationsDelegate old) => false;
}

AppLocalizations lookupAppLocalizations(Locale locale) {
  // Lookup logic when only language code is specified.
  switch (locale.languageCode) {
    case 'en':
      return AppLocalizationsEn();
    case 'ja':
      return AppLocalizationsJa();
  }

  throw FlutterError(
      'AppLocalizations.delegate failed to load unsupported locale "$locale". This is likely '
      'an issue with the localizations generation tool. Please file an issue '
      'on GitHub with a reproducible sample app and the gen-l10n configuration '
      'that was used.');
}
