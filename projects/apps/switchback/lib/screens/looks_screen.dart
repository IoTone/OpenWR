// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
import 'package:flutter/material.dart';
import 'package:go_router/go_router.dart';
import 'package:provider/provider.dart';

import '../gen/app_localizations.dart';
import '../goggle/console.dart';
import '../goggle/status.dart';
import '../theme/theme_controller.dart';
import '../theme/tokens.dart';

/// Looks: what the glass shows (theme, animation, brightness) and how the
/// app looks (experience pin, red shift). Previews are the real driver's
/// output from the firmware repo.
class LooksScreen extends StatefulWidget {
  const LooksScreen({super.key});

  @override
  State<LooksScreen> createState() => _LooksScreenState();
}

class _LooksScreenState extends State<LooksScreen> {
  bool _anim = true;
  double _contrast = 255;

  @override
  Widget build(BuildContext context) {
    final GoggleConsole g = context.watch<GoggleConsole>();
    final ThemeController tc = context.watch<ThemeController>();
    final AppLocalizations l = AppLocalizations.of(context);
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final GlassTheme? current = g.status?.theme;

    return ListView(
      padding: const EdgeInsets.fromLTRB(20, 8, 20, 32),
      children: <Widget>[
        if (!g.isConnected) ...<Widget>[
          Container(
            padding: const EdgeInsets.fromLTRB(14, 10, 8, 10),
            decoration: BoxDecoration(color: cs.surfaceContainerHigh, borderRadius: BorderRadius.circular(10), border: Border.all(color: cs.outline)),
            child: Row(
              children: <Widget>[
                Icon(Icons.bluetooth_disabled, color: cs.onSurfaceVariant),
                const SizedBox(width: 10),
                Expanded(child: Text(l.looksNotConnected, style: t.textTheme.bodyMedium)),
                TextButton(onPressed: () => context.push('/pair'), child: Text(l.pairGoggle)),
              ],
            ),
          ),
          const SizedBox(height: 14),
        ],
        Text(l.looksGlassTitle, style: t.textTheme.headlineMedium),
        const SizedBox(height: 4),
        Text(l.looksGlassBody, style: t.textTheme.bodySmall),
        const SizedBox(height: 14),
        for (final GlassTheme gt in GlassTheme.values) ...<Widget>[
          _GlassChoice(
            theme: gt,
            selected: current == gt,
            enabled: g.isConnected,
            onTap: () => g.setTheme(gt),
          ),
          const SizedBox(height: 10),
        ],
        const SizedBox(height: 6),
        SwitchListTile(
          contentPadding: EdgeInsets.zero,
          title: Text(l.looksAnimation),
          subtitle: Text(l.looksAnimationSub),
          value: _anim,
          onChanged: g.isConnected
              ? (bool v) {
                  setState(() => _anim = v);
                  g.setAnimation(v);
                }
              : null,
        ),
        ListTile(
          contentPadding: EdgeInsets.zero,
          title: Text(l.looksBrightness),
          subtitle: Slider(
            value: _contrast,
            min: 8,
            max: 255,
            divisions: 12,
            label: _contrast.round().toString(),
            onChanged: g.isConnected ? (double v) => setState(() => _contrast = v) : null,
            onChangeEnd: g.isConnected ? (double v) => g.setContrast(v.round()) : null,
          ),
        ),
        const Divider(height: 32),
        Text(l.looksAppTitle, style: t.textTheme.headlineMedium),
        const SizedBox(height: 4),
        Text(l.looksAppBody, style: t.textTheme.bodySmall),
        const SizedBox(height: 12),
        Wrap(
          spacing: 8,
          runSpacing: 8,
          children: <Widget>[
            ChoiceChip(
              label: Text(l.experienceAuto),
              selected: tc.pinned == null,
              onSelected: (_) => tc.pin(null),
            ),
            for (final Experience e in Experience.values)
              ChoiceChip(
                label: Text(e.label),
                selected: tc.pinned == e,
                onSelected: (_) => tc.pin(e),
              ),
          ],
        ),
        const SizedBox(height: 8),
        Text(l.experienceNow(tc.auto.label), style: t.textTheme.labelMedium?.copyWith(color: cs.primary)),
        const SizedBox(height: 8),
        SwitchListTile(
          contentPadding: EdgeInsets.zero,
          title: Text(l.looksRedShift),
          subtitle: Text(l.looksRedShiftSub),
          value: tc.redShift,
          onChanged: tc.setRedShift,
        ),
      ],
    );
  }
}

class _GlassChoice extends StatelessWidget {
  const _GlassChoice({required this.theme, required this.selected, required this.enabled, required this.onTap});
  final GlassTheme theme;
  final bool selected;
  final bool enabled;
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final ThemeData t = Theme.of(context);
    final ColorScheme cs = t.colorScheme;
    final AppLocalizations l = AppLocalizations.of(context);
    final String blurb = switch (theme) {
      GlassTheme.strip => l.glassStripBlurb,
      GlassTheme.glance => l.glassGlanceBlurb,
      GlassTheme.ribbon => l.glassRibbonBlurb,
    };
    return Opacity(
      opacity: enabled ? 1 : .55,
      child: Material(
        color: cs.surface,
        shape: RoundedRectangleBorder(
          borderRadius: BorderRadius.circular(12),
          side: BorderSide(color: selected ? cs.primary : cs.outline, width: selected ? 2 : 1),
        ),
        clipBehavior: Clip.antiAlias,
        child: InkWell(
          onTap: enabled ? onTap : null,
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: <Widget>[
              Container(
                color: const Color(0xFF0C0C10),
                padding: const EdgeInsets.all(10),
                child: Image.asset(theme.asset, fit: BoxFit.fitWidth, filterQuality: FilterQuality.none, cacheWidth: 536, gaplessPlayback: true),
              ),
              Padding(
                padding: const EdgeInsets.fromLTRB(14, 10, 14, 12),
                child: Row(
                  children: <Widget>[
                    Expanded(
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: <Widget>[
                          Text(theme.label, style: t.textTheme.titleMedium?.copyWith(fontFamily: kFontDisplay)),
                          Text(blurb, style: t.textTheme.bodySmall),
                        ],
                      ),
                    ),
                    if (selected) Icon(Icons.check_circle, color: cs.primary),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
