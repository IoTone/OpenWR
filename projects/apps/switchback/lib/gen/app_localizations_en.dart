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
      'The phone is the sensor hub, the glass is the display. Pick a sport and start: time, pace and distance go to the goggle once a second.';

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

  @override
  String get setupTitle => 'Goggle setup';

  @override
  String get setupSettingsSub => 'WiFi, location, units';

  @override
  String get setupNotConnected =>
      'Pair the goggle first; the cards send as you go.';

  @override
  String get setupWifiTitle => 'WiFi';

  @override
  String get setupWifiBody =>
      'The goggle fetches its own forecast, so it needs a network. 2.4 GHz only.';

  @override
  String get setupWifiPick => 'Pick a network';

  @override
  String get setupWifiSsid => 'Network name';

  @override
  String get setupWifiPassword => 'Password';

  @override
  String get setupWifiSend => 'Send to goggle';

  @override
  String get setupWifiSending => 'Sending, waiting for an address…';

  @override
  String setupWifiUp(String ip) {
    return 'Connected, $ip.';
  }

  @override
  String get setupWifiNotYet =>
      'No address after 30 s. Check the password, or it may still be joining.';

  @override
  String setupWifiState(String ip) {
    return 'on $ip';
  }

  @override
  String get setupWifiOff => 'no network';

  @override
  String get setupLocationTitle => 'Location';

  @override
  String get setupLocationBody =>
      'Left alone, the goggle places itself by its IP address, to the city. Set it by hand or from this phone for the exact spot, which also stops the IP lookup.';

  @override
  String get setupLocationUsePhone => 'Use phone location';

  @override
  String get setupLocationSetManual => 'Set';

  @override
  String get setupLocationBackToAuto => 'Back to automatic';

  @override
  String get setupLocationReading => 'Reading the phone\'s position…';

  @override
  String get setupLocationDenied => 'Location permission denied.';

  @override
  String get setupLocationInvalid =>
      'Latitude and longitude please, in degrees.';

  @override
  String get setupLocationSending => 'Sending and fetching the forecast…';

  @override
  String setupLocationSet(String city) {
    return 'Set. Forecast for $city.';
  }

  @override
  String get setupLocationLookingUp => 'Looking the IP up…';

  @override
  String get setupLocationLookupFailed =>
      'Lookup failed; the old location stays.';

  @override
  String setupLocationManual(String city) {
    return '$city, by hand';
  }

  @override
  String setupLocationAuto(String city) {
    return '$city, from IP';
  }

  @override
  String get setupUnitsTitle => 'Units';

  @override
  String get setupUnitsBody =>
      'Picked from the country by default. The glass and this app follow.';

  @override
  String get setupUnitsMetric => '°C · km/h';

  @override
  String get setupUnitsImperial => '°F · mph';

  @override
  String get updateNotConnected => 'Pair the goggle to see what it is running.';

  @override
  String get updateEmbeddedTitle => 'Releases in this app';

  @override
  String get updateEmbeddedBody =>
      'Signed .mnpkg packages built into Switchback. The phone serves one over your WiFi and the goggle checks the signature, hash and target before it writes a byte.';

  @override
  String get updateNoReleases => 'This build carries no releases.';

  @override
  String get updateInstall => 'Install';

  @override
  String get updateInstalled => 'running';

  @override
  String get updateServing => 'Serving…';

  @override
  String updateServingAt(String ip, int port, int n) {
    return 'serving on $ip:$port, $n fetched';
  }

  @override
  String updateInstallBody(String fw, String version, String variant) {
    return 'Install goggl1 $fw ($version, $variant)? The goggle fetches it from this phone, verifies it, and reboots into it.';
  }

  @override
  String updateProgress(int pct) {
    return 'Writing… $pct%';
  }

  @override
  String get updateRebooting =>
      'Verified and staged. The goggle is rebooting into it; pair again in a few seconds.';

  @override
  String get updateNoVerdict => 'No verdict within 150 s. Check the Console.';

  @override
  String get updateBundleTitle => 'Use case in this app';

  @override
  String get updateBundleBody =>
      'The weather HUD bundle as shipped with this build. Pushing it replaces what the goggle has and keeps it across reboots.';

  @override
  String get updateBundlePush => 'Push weather HUD';

  @override
  String get updateBundleSending => 'Pushing bundle…';

  @override
  String pairFound(int n) {
    return 'Found $n goggle(s)';
  }

  @override
  String pairPhaseConnecting(int n) {
    return 'Connecting, attempt $n of 2…';
  }

  @override
  String get pairPhaseReading => 'Connected, reading status…';

  @override
  String pairPhaseRetrying(int n) {
    return 'First link dropped, retrying (attempt $n)…';
  }

  @override
  String get pairPhaseConnected => 'Connected';

  @override
  String get pairPhaseFailed => 'Could not connect';

  @override
  String get pairPhaseDropped => 'The goggle disconnected';

  @override
  String get pairFailedHint =>
      'The goggle advertises for 3 minutes after power-up and after any drop. Plug it in again if the B is gone from the glass, then tap Connect.';

  @override
  String get looksSending => 'Sending…';

  @override
  String looksSendFailed(String e) {
    return 'The goggle did not answer: $e';
  }

  @override
  String get sessionGlassReady =>
      'The goggle is connected; it will show the session.';

  @override
  String get sessionNoGlass =>
      'No goggle connected. The session runs on the phone alone.';

  @override
  String sessionPace(String unit) {
    return 'min/$unit';
  }

  @override
  String get sessionStop => 'Stop';

  @override
  String sessionDone(String time, String dist, String unit) {
    return 'Done: $time, $dist $unit.';
  }

  @override
  String get sessionLogTitle => 'Recent sessions';

  @override
  String get pairMaplet => 'Pair maplet';

  @override
  String get pairMapletTitle => 'Pair maplet';

  @override
  String get pairMapletHeadline => 'Power the maplet up.';

  @override
  String get pairMapletBody =>
      'The patch listens for three minutes after power-up. Switchback finds it and connects.';

  @override
  String get mapletPostcard => 'Postcard';

  @override
  String get mapletSend => 'Send to maplet';

  @override
  String mapletSending(int pct) {
    return 'Sending $pct%';
  }

  @override
  String get mapletSent => 'On the glass. It stays until the next session.';

  @override
  String get mapletNotConnected =>
      'No maplet connected. The preview shows the 2.13in 212 × 104 panel.';

  @override
  String mapletGlassArea(int w, int h) {
    return 'Rendered for the connected glass: $w × $h.';
  }

  @override
  String get mapletNoTrack =>
      'This session has no GPS track, so the map is the banner only.';

  @override
  String get mapletTrackPoints => 'track points';

  @override
  String get mapletAttribution =>
      'Elevation: AWS Terrain Tiles (USGS 3DEP / Mapzen).';

  @override
  String get mapletCardTitle => 'maplet';

  @override
  String get mapletCardSub => 'e-paper session map';

  @override
  String mapletConnected(int w) {
    return 'Connected · glass $w px wide';
  }

  @override
  String get sessionSentToMaplet => 'Sent to the maplet.';

  @override
  String get sessionAddDemo => 'Add a demo session (Forest Park loop)';

  @override
  String get planTitle => 'Plan';

  @override
  String get planBody =>
      'A page for the trailhead: the route, the numbers, and when the light goes. It stays on the patch until you send the Postcard at the end.';

  @override
  String get planHere => 'Where I am';

  @override
  String get planHereTitle => 'Here';

  @override
  String get planLoop => 'Loop';

  @override
  String get mapletFieldNotes => 'Field Notes';

  @override
  String get sessionPlan => 'Plan a session';

  @override
  String get mapletTitle => 'maplet';

  @override
  String get mapletHubBody =>
      'The patch: what it is running, what it shows, and the tools to set it up and update it.';

  @override
  String mapletSlot(int n) {
    return 'Showing map slot $n';
  }

  @override
  String get mapletNoMap => 'Showing the splash';

  @override
  String get mapletShowTest => 'Test pattern';

  @override
  String get mapletShowSplash => 'Splash';

  @override
  String get mapletLastPostcard => 'Last Postcard';

  @override
  String get mapletSetupWifi => 'Set up WiFi';

  @override
  String get mapletUpdate => 'Update firmware';

  @override
  String get mapletConsole => 'Console';

  @override
  String get mapletDisconnect => 'Disconnect';

  @override
  String mapletSafeArea(int w, int total, int x) {
    return 'Glass: $w of $total columns render, from column $x';
  }

  @override
  String get sessionSnap => 'Snap';

  @override
  String get sessionSnapping => 'Taking the picture…';

  @override
  String get rollTitle => 'Roll';

  @override
  String get rollBody =>
      'The snapshots on the maplet\'s card, pulled over Bluetooth. Each takes about ten seconds.';

  @override
  String get rollEmpty =>
      'No snapshots on the card yet. Press Snap during a session.';

  @override
  String get rollNoCard =>
      'No card in the maplet, or the roll could not be read.';

  @override
  String rollFrame(int n) {
    return 'Frame $n';
  }

  @override
  String get rollPull => 'Pull';

  @override
  String get rollPulling => 'Pulling…';

  @override
  String get rollShow => 'Show on glass';

  @override
  String get mapletRoll => 'Roll';

  @override
  String get sportSnowboard => 'Snowboard';

  @override
  String get sportHike => 'Hike';

  @override
  String mapletPanel(String driver, int w, int h) {
    return 'Panel: $driver, $w × $h';
  }
}
