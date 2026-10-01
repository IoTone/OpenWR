// ignore: unused_import
import 'package:intl/intl.dart' as intl;
import 'app_localizations.dart';

// ignore_for_file: type=lint

/// The translations for English (`en`).
class AppLocalizationsEn extends AppLocalizations {
  AppLocalizationsEn([String locale = 'en']) : super(locale);

  @override
  String get appTitle => 'Switchback';

  @override
  String get aboutSubtitle => 'the OpenWR app';

  @override
  String aboutVersion(String version) {
    return 'Version $version';
  }

  @override
  String get aboutDescription =>
      'Pair, set up, dress and update your goggl1 over Bluetooth, and run sessions with the glass in front of your eye. Open. Weird. Wild. Strong. #PNWbuilt';

  @override
  String get aboutCopyright => '© 2024–2026 IoTone, Inc.';

  @override
  String get aboutLicense => 'MIT License';

  @override
  String get aboutLicensesButton => 'Open-source licenses';

  @override
  String get aboutMadeWith => 'Built with Flutter.';

  @override
  String get quickNav => 'Quick navigation';

  @override
  String get commonCancel => 'Cancel';

  @override
  String get commonClose => 'Close';

  @override
  String get commonRetry => 'Retry';

  @override
  String get commonCopy => 'Copy';

  @override
  String get commonCopied => 'Copied';

  @override
  String get scanStart => 'Start scan';

  @override
  String get scanStop => 'Stop scan';

  @override
  String get scanScanning => 'Scanning…';

  @override
  String get scanIdle => 'Idle';

  @override
  String get bluetoothOn => 'Bluetooth ready';

  @override
  String get bluetoothOff => 'Bluetooth is off';

  @override
  String get bluetoothUnauthorized => 'Bluetooth permission needed';

  @override
  String get bluetoothUnknown => 'Bluetooth state unknown';

  @override
  String get radarEmpty => 'No devices in range yet.';

  @override
  String get radarSelf => 'You';

  @override
  String get radarLegendBle => 'Bluetooth device';

  @override
  String get radarLegendWifi => 'WiFi network';

  @override
  String get radarLegendRings => 'Rings = approximate signal distance (rough).';

  @override
  String rssiDbm(int rssi) {
    return '$rssi dBm';
  }

  @override
  String metersApprox(String meters) {
    return '≈ $meters m';
  }

  @override
  String get signalStrong => 'Strong';

  @override
  String get signalMedium => 'Medium';

  @override
  String get signalWeak => 'Weak';

  @override
  String get deviceConnect => 'Connect';

  @override
  String get settingsAppearance => 'Appearance';

  @override
  String get settingsTheme => 'Theme';

  @override
  String get settingsFontScale => 'Text size';

  @override
  String get settingsReduceMotion => 'Reduce motion';

  @override
  String get settingsHighContrast => 'High contrast';

  @override
  String get settingsLanguage => 'Language';

  @override
  String get settingsLanguageSystem => 'System default';

  @override
  String get settingsPermissions => 'Permissions';

  @override
  String get settingsDiagnostics => 'Diagnostics';

  @override
  String get settingsDiagnosticsSub => 'Scan event log';

  @override
  String get settingsAbout => 'About';

  @override
  String get settingsScanner => 'Scanner';

  @override
  String get settingsRssiFloor => 'Signal floor';

  @override
  String settingsRssiFloorValue(int dbm) {
    return 'Hide signals weaker than $dbm dBm';
  }

  @override
  String get settingsRssiFloorOff => 'Show all signals';

  @override
  String get permBle => 'Bluetooth';

  @override
  String get permLocation => 'Location';

  @override
  String get permLocationWhy =>
      'Android requires location permission to scan for nearby devices.';

  @override
  String get permGranted => 'Granted';

  @override
  String get permDenied => 'Denied';

  @override
  String get permRequest => 'Request';

  @override
  String get permOpenSettings => 'Open settings';

  @override
  String get diagEmpty => 'No scan activity yet.';

  @override
  String get diagClear => 'Clear';

  @override
  String get diagCopy => 'Copy log';

  @override
  String get firstRunTitle => 'Welcome to Switchback';

  @override
  String get firstRunBody =>
      'This app talks to your goggl1 over Bluetooth. It needs a couple of permissions to find it.';

  @override
  String get firstRunPermBle => 'Bluetooth — to find and talk to the goggle.';

  @override
  String get firstRunPermLocation =>
      'Location — Android requires it for Bluetooth scanning.';

  @override
  String get firstRunGrant => 'Grant permissions';

  @override
  String get firstRunSkip => 'Skip for now';

  @override
  String get langEnglish => 'English';

  @override
  String get langJapanese => '日本語';

  @override
  String get settingsTitle => 'Settings';

  @override
  String get settingsExperience => 'Experience';

  @override
  String get tabTrailhead => 'Trailhead';

  @override
  String get tabSession => 'Session';

  @override
  String get tabLooks => 'Looks';

  @override
  String get tabConsole => 'Console';

  @override
  String get tabUpdate => 'Update';

  @override
  String get experiencePinned => 'Pinned';

  @override
  String get experienceAuto => 'Auto';

  @override
  String experienceNow(String name) {
    return 'Right now: $name';
  }

  @override
  String feelsLike(int t) {
    return 'feels $t';
  }

  @override
  String hiLo(int hi, int lo) {
    return 'hi $hi · lo $lo';
  }

  @override
  String windNow(int v, String unit, String dir) {
    return 'wind $v $unit $dir';
  }

  @override
  String get chipDry8h => 'dry 8h';

  @override
  String chipRainAt(String t) {
    return 'rain $t';
  }

  @override
  String chipUnits(String u) {
    return '$u';
  }

  @override
  String get trailheadNoForecastTitle => 'No forecast yet';

  @override
  String get trailheadWaitingForecast =>
      'The goggle is connected and fetching. Give it a few seconds, or check it has WiFi in Console.';

  @override
  String get trailheadNotConnected => 'Pair your goggle to see what it sees.';

  @override
  String get startSession => 'Start a session';

  @override
  String get pairGoggle => 'Pair goggle';

  @override
  String get headlineStormNow => 'Thunderstorm now. Stay in.';

  @override
  String get headlineWetAllWindow => 'Wet for the next eight hours.';

  @override
  String headlineClearsAt(String t) {
    return 'Clears around $t.';
  }

  @override
  String headlineRainAt(String t, int pop) {
    return 'Rain from $t, $pop% likely. Go before it.';
  }

  @override
  String headlineWindy(int g) {
    return 'Dry, but gusts to $g. Pick a sheltered route.';
  }

  @override
  String get headlineDryDay => 'Dry for the next eight hours. Go.';

  @override
  String get headlineDryNight => 'Clear and dry tonight. Headlamp on.';

  @override
  String squallWarning(String t, int pop, int gust, String unit) {
    return 'Rain $t · $pop% · gusts $gust $unit';
  }

  @override
  String get goggleNotConnected => 'Not connected';

  @override
  String get goggleConnecting => 'Connecting…';

  @override
  String get goggleConnected => 'Connected';

  @override
  String get ageUnknown => 'no clock';

  @override
  String get ageJustNow => 'just now';

  @override
  String ageMinutes(int n) {
    return '$n min ago';
  }

  @override
  String ageHours(int n) {
    return '$n h ago';
  }

  @override
  String get pairTitle => 'Pair';

  @override
  String get pairHeadline => 'Hold BOOT for a second.';

  @override
  String get pairBody =>
      'The glass shows a B and the goggle listens for three minutes. Switchback finds it and connects.';

  @override
  String get pairScanning => 'Looking for goggl1…';

  @override
  String get pairFailed => 'Could not connect. Hold BOOT again and retry.';

  @override
  String get pairNothingYet => 'Nothing yet. Is the B showing on the glass?';

  @override
  String get pairFindRadar => 'Find it on the radar';

  @override
  String get radarLegendGoggle => 'goggl1';

  @override
  String get looksGlassTitle => 'On the glass';

  @override
  String get looksGlassBody =>
      'Previews are the goggle\'s own renderer. Tap one to switch.';

  @override
  String get looksNotConnected => 'Pair the goggle to change what it shows.';

  @override
  String get looksAnimation => 'Animation';

  @override
  String get looksAnimationSub =>
      'Rain falls, snow drifts, storms flash, stars twinkle';

  @override
  String get looksBrightness => 'Glass brightness';

  @override
  String get looksAppTitle => 'This app';

  @override
  String get looksAppBody =>
      'The look follows the weather the goggle sees. Pin one to keep it.';

  @override
  String get looksRedShift => 'Red shift';

  @override
  String get looksRedShiftSub => 'Headlamp goes deep red for night vision';

  @override
  String get glassStripBlurb => 'A chart. The shape of the next eight hours.';

  @override
  String get glassGlanceBlurb => 'Big numbers and one sentence.';

  @override
  String get glassRibbonBlurb => 'Every hour as a column.';

  @override
  String get consoleNotConnected => 'The console needs a goggle.';

  @override
  String get consoleHistory => 'Previous line';

  @override
  String get consoleSend => 'Send';

  @override
  String get sessionTitle => 'Session';

  @override
  String get sessionBody =>
      'The phone is the sensor hub, the glass is the display. Pick a sport; Switchback will stream time, pace and heart rate to the goggle.';

  @override
  String get sportRun => 'Run';

  @override
  String get sportRide => 'Ride';

  @override
  String get sportSki => 'Ski';

  @override
  String get sportPaddle => 'Paddle';

  @override
  String get sportClimb => 'Climb';

  @override
  String get sessionComingSoon => 'Sessions arrive in the next build.';

  @override
  String get sessionPhaseNote =>
      'Coming: the session-hud bundle, GPS pace and distance, a heart-rate strap.';

  @override
  String get updateTitle => 'Update';

  @override
  String updateRunning(String v, String b) {
    return 'Running v$v with $b.';
  }

  @override
  String get updateReadDetails => 'Read details';

  @override
  String get updateOtaTitle => 'Firmware over the air';

  @override
  String get updateOtaBody =>
      'Serve the image on the same WiFi as the goggle and paste its URL. The goggle writes the spare slot and reboots; a bad image rolls back.';

  @override
  String get updateUrlLabel => 'Image URL';

  @override
  String get updateStart => 'Update';

  @override
  String get updateConfirmTitle => 'Update firmware?';

  @override
  String updateConfirmBody(String url) {
    return 'The goggle will fetch $url, write it, and reboot.';
  }

  @override
  String get updateStarted => 'Update started. Progress shows in Console.';

  @override
  String get updatePhaseNote =>
      'Coming: Switchback serves the image itself, and pushes use-case bundles.';
}
