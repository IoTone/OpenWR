// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'dart:async';

import 'package:flutter/material.dart';
import 'package:geolocator/geolocator.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/status.dart';
import '../scanner/scanned_access_point.dart';
import '../scanner/scanner_controller.dart';
import '../scanner/wifi_source.dart';
import '../theme/tokens.dart';

/// Setup: three cards, one Forth line under each. WiFi, location, units.
/// Shown after the first pairing when the goggle has no network, and any
/// time from the device card or Settings.
class SetupScreen extends StatefulWidget {
  const SetupScreen({super.key});

  @override
  State<SetupScreen> createState() => _SetupScreenState();
}

class _SetupScreenState extends State<SetupScreen> {
  final TextEditingController _ssid = TextEditingController();
  final TextEditingController _pass = TextEditingController();
  final TextEditingController _lat = TextEditingController();
  final TextEditingController _lon = TextEditingController();
  bool _showPass = false;
  bool _wifiBusy = false;
  String? _wifiResult;
  bool _geoBusy = false;
  String? _geoResult;
  bool _unitsBusy = false;
  bool? _manual;

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) {
      final ScannerController sc = context.read<ScannerController>();
      if (!sc.isScanning && sc.wifiCapability != WifiCapability.unavailable) unawaited(sc.start());
      unawaited(_readLocation());
    });
  }

  @override
  void dispose() {
    _ssid.dispose();
    _pass.dispose();
    _lat.dispose();
    _lon.dispose();
    super.dispose();
  }

  Future<void> _readLocation() async {
    final GoggleConsole g = context.read<GoggleConsole>();
    if (!g.isConnected) return;
    final GoggleLocation? loc = await g.readLocation();
    if (!mounted || loc == null) return;
    setState(() {
      _manual = loc.manual;
      if (_lat.text.isEmpty) _lat.text = loc.lat.toStringAsFixed(4);
      if (_lon.text.isEmpty) _lon.text = loc.lon.toStringAsFixed(4);
    });
  }

  Future<void> _sendWifi() async {
    final GoggleConsole g = context.read<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final String ssid = _ssid.text.trim();
    if (ssid.isEmpty) return;
    setState(() {
      _wifiBusy = true;
      _wifiResult = l.setupWifiSending;
    });
    try {
      await g.setWifi(ssid, _pass.text);
      final bool up = await g.waitForNetwork(const Duration(seconds: 30));
      if (!mounted) return;
      setState(() => _wifiResult = up ? l.setupWifiUp(g.status?.ip ?? '') : l.setupWifiNotYet);
    } catch (e) {
      if (mounted) setState(() => _wifiResult = e.toString());
    } finally {
      if (mounted) setState(() => _wifiBusy = false);
    }
  }

  Future<void> _usePhoneLocation() async {
    final AppLocalizations l = AppLocalizations.of(context);
    setState(() {
      _geoBusy = true;
      _geoResult = l.setupLocationReading;
    });
    try {
      LocationPermission p = await Geolocator.checkPermission();
      if (p == LocationPermission.denied) p = await Geolocator.requestPermission();
      if (p == LocationPermission.denied || p == LocationPermission.deniedForever) {
        setState(() => _geoResult = l.setupLocationDenied);
        return;
      }
      final Position pos = await Geolocator.getCurrentPosition(
        locationSettings: const LocationSettings(accuracy: LocationAccuracy.low, timeLimit: Duration(seconds: 15)),
      );
      _lat.text = pos.latitude.toStringAsFixed(4);
      _lon.text = pos.longitude.toStringAsFixed(4);
      await _sendLocation();
    } catch (e) {
      if (mounted) setState(() => _geoResult = e.toString());
    } finally {
      if (mounted) setState(() => _geoBusy = false);
    }
  }

  Future<void> _sendLocation() async {
    final GoggleConsole g = context.read<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    final double? lat = double.tryParse(_lat.text.trim());
    final double? lon = double.tryParse(_lon.text.trim());
    if (lat == null || lon == null || lat.abs() > 90 || lon.abs() > 180) {
      setState(() => _geoResult = l.setupLocationInvalid);
      return;
    }
    setState(() {
      _geoBusy = true;
      _geoResult = l.setupLocationSending;
    });
    try {
      await g.setLocation(lat, lon);
      if (mounted) {
        setState(() {
          _manual = true;
          _geoResult = l.setupLocationSet(g.status?.city ?? '');
        });
      }
    } catch (e) {
      if (mounted) setState(() => _geoResult = e.toString());
    } finally {
      if (mounted) setState(() => _geoBusy = false);
    }
  }

  Future<void> _autoLocation() async {
    final GoggleConsole g = context.read<GoggleConsole>();
    final AppLocalizations l = AppLocalizations.of(context);
    setState(() {
      _geoBusy = true;
      _geoResult = l.setupLocationLookingUp;
    });
    try {
      final bool ok = await g.setAutoLocation();
      if (!mounted) return;
      setState(() {
        _manual = false;
        _geoResult = ok ? l.setupLocationSet(g.status?.city ?? '') : l.setupLocationLookupFailed;
      });
      _lat.clear();
      _lon.clear();
      await _readLocation();
    } finally {
      if (mounted) setState(() => _geoBusy = false);
    }
  }

  Future<void> _setUnits(bool imperial) async {
    final GoggleConsole g = context.read<GoggleConsole>();
    setState(() => _unitsBusy = true);
    try {
      await g.setUnits(imperial: imperial);
    } finally {
      if (mounted) setState(() => _unitsBusy = false);
    }
  }

  @override
  Widget build(BuildContext context) {
    final GoggleConsole g = context.watch<GoggleConsole>();
    final ScannerController sc = context.watch<ScannerController>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final GoggleStatus? st = g.status;
    final TextStyle mono = TextStyle(fontFamily: kFontMono, fontSize: 12, color: cs.onSurfaceVariant);
    final List<ScannedAccessPoint> aps = sc.accessPoints.where((ScannedAccessPoint a) => a.ssid.isNotEmpty).toList();

    return Scaffold(
      appBar: AppBar(title: Text(l.setupTitle)),
      body: ListView(
        padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
        children: <Widget>[
          if (!g.isConnected) ...<Widget>[
            Text(l.setupNotConnected, style: t.textTheme.bodyLarge?.copyWith(color: cs.error)),
            const SizedBox(height: 12),
          ],
          // --- WiFi ---------------------------------------------------
          _CardShell(
            title: l.setupWifiTitle,
            state: st == null ? null : (st.netOk ? l.setupWifiState(st.ip) : l.setupWifiOff),
            good: st?.netOk,
            children: <Widget>[
              Text(l.setupWifiBody, style: t.textTheme.bodySmall),
              const SizedBox(height: 10),
              if (aps.isNotEmpty)
                DropdownMenu<String>(
                  width: MediaQuery.of(context).size.width - 72,
                  label: Text(l.setupWifiPick),
                  onSelected: (String? v) {
                    if (v != null) setState(() => _ssid.text = v);
                  },
                  dropdownMenuEntries: <DropdownMenuEntry<String>>[
                    for (final ScannedAccessPoint a in aps)
                      DropdownMenuEntry<String>(
                        value: a.ssid,
                        label: a.ssid,
                        trailingIcon: Text('${a.rssi} dBm', style: mono),
                      ),
                  ],
                ),
              if (aps.isNotEmpty) const SizedBox(height: 8),
              TextField(
                controller: _ssid,
                autocorrect: false,
                decoration: InputDecoration(labelText: l.setupWifiSsid),
              ),
              const SizedBox(height: 8),
              TextField(
                controller: _pass,
                obscureText: !_showPass,
                autocorrect: false,
                enableSuggestions: false,
                decoration: InputDecoration(
                  labelText: l.setupWifiPassword,
                  suffixIcon: IconButton(
                    icon: Icon(_showPass ? Icons.visibility_off : Icons.visibility),
                    onPressed: () => setState(() => _showPass = !_showPass),
                  ),
                ),
              ),
              const SizedBox(height: 10),
              FilledButton(
                onPressed: g.isConnected && !_wifiBusy ? _sendWifi : null,
                child: Text(_wifiBusy ? l.setupWifiSending : l.setupWifiSend),
              ),
              if (_wifiResult != null) ...<Widget>[
                const SizedBox(height: 8),
                Text(_wifiResult!, style: t.textTheme.bodySmall),
              ],
              const SizedBox(height: 6),
              Text('s" ssid" s" password" wifi!', style: mono),
            ],
          ),
          const SizedBox(height: 14),
          // --- Location -----------------------------------------------
          _CardShell(
            title: l.setupLocationTitle,
            state: st == null
                ? null
                : (_manual == true ? l.setupLocationManual(st.city) : l.setupLocationAuto(st.city)),
            good: st?.city != null && st?.city != '-',
            children: <Widget>[
              Text(l.setupLocationBody, style: t.textTheme.bodySmall),
              const SizedBox(height: 10),
              Row(
                children: <Widget>[
                  Expanded(
                    child: TextField(
                      controller: _lat,
                      keyboardType: const TextInputType.numberWithOptions(decimal: true, signed: true),
                      decoration: const InputDecoration(labelText: 'lat'),
                    ),
                  ),
                  const SizedBox(width: 8),
                  Expanded(
                    child: TextField(
                      controller: _lon,
                      keyboardType: const TextInputType.numberWithOptions(decimal: true, signed: true),
                      decoration: const InputDecoration(labelText: 'lon'),
                    ),
                  ),
                ],
              ),
              const SizedBox(height: 10),
              Row(
                children: <Widget>[
                  Expanded(
                    child: FilledButton.icon(
                      onPressed: g.isConnected && !_geoBusy ? _usePhoneLocation : null,
                      icon: const Icon(Icons.my_location),
                      label: Text(l.setupLocationUsePhone),
                    ),
                  ),
                  const SizedBox(width: 8),
                  OutlinedButton(
                    onPressed: g.isConnected && !_geoBusy ? _sendLocation : null,
                    child: Text(l.setupLocationSetManual),
                  ),
                ],
              ),
              const SizedBox(height: 8),
              OutlinedButton(
                onPressed: g.isConnected && !_geoBusy && _manual == true ? _autoLocation : null,
                child: Text(l.setupLocationBackToAuto),
              ),
              if (_geoResult != null) ...<Widget>[
                const SizedBox(height: 8),
                Text(_geoResult!, style: t.textTheme.bodySmall),
              ],
              const SizedBox(height: 6),
              Text('455234 -1226762 geo!   geo-auto', style: mono),
            ],
          ),
          const SizedBox(height: 14),
          // --- Units --------------------------------------------------
          _CardShell(
            title: l.setupUnitsTitle,
            state: st == null ? null : (st.imperial ? l.setupUnitsImperial : l.setupUnitsMetric),
            good: st != null,
            children: <Widget>[
              Text(l.setupUnitsBody, style: t.textTheme.bodySmall),
              const SizedBox(height: 10),
              SegmentedButton<bool>(
                segments: <ButtonSegment<bool>>[
                  ButtonSegment<bool>(value: false, label: Text(l.setupUnitsMetric)),
                  ButtonSegment<bool>(value: true, label: Text(l.setupUnitsImperial)),
                ],
                selected: <bool>{st?.imperial ?? true},
                onSelectionChanged: g.isConnected && !_unitsBusy ? (Set<bool> s) => _setUnits(s.first) : null,
              ),
              const SizedBox(height: 6),
              Text('1 units!   0 units!', style: mono),
            ],
          ),
        ],
      ),
    );
  }
}

class _CardShell extends StatelessWidget {
  const _CardShell({required this.title, required this.children, this.state, this.good});
  final String title;
  final String? state;
  final bool? good;
  final List<Widget> children;

  @override
  Widget build(BuildContext context) {
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    return Card(
      child: Padding(
        padding: const EdgeInsets.fromLTRB(14, 12, 14, 14),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: <Widget>[
            Row(
              children: <Widget>[
                Expanded(child: Text(title, style: t.textTheme.titleMedium?.copyWith(fontFamily: kFontDisplay, fontSize: 18))),
                if (state != null) ...<Widget>[
                  Container(
                    width: 8,
                    height: 8,
                    decoration: BoxDecoration(color: good == true ? cs.tertiary : cs.onSurfaceVariant, shape: BoxShape.circle),
                  ),
                  const SizedBox(width: 6),
                  Flexible(child: Text(state!, style: t.textTheme.labelMedium, overflow: TextOverflow.ellipsis)),
                ],
              ],
            ),
            const SizedBox(height: 8),
            ...children,
          ],
        ),
      ),
    );
  }
}
