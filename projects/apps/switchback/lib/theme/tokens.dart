// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The three experiences. An experience is the app's whole look, chosen by
// the weather the goggle reports and the time of day, not a setting buried
// in a menu. Screens reference only the semantic [AppTokens] roles.
import 'package:flutter/material.dart';

enum Experience { dawn, headlamp, squall }

extension ExperienceX on Experience {
  String get label => switch (this) {
        Experience.dawn => 'Dawn Patrol',
        Experience.headlamp => 'Headlamp',
        Experience.squall => 'Squall',
      };
  String get tagline => switch (this) {
        Experience.dawn => 'the early session',
        Experience.headlamp => 'after dark',
        Experience.squall => 'weather coming',
      };
}

/// Semantic colour contract shared by every experience.
@immutable
class AppTokens {
  const AppTokens({
    required this.brightness,
    required this.base,
    required this.surface,
    required this.surfaceAlt,
    required this.line,
    required this.fg,
    required this.fgMuted,
    required this.accent,
    required this.onAccent,
    required this.water,
    required this.alert,
    required this.ok,
  });

  final Brightness brightness;
  final Color base;
  final Color surface;
  final Color surfaceAlt;
  final Color line;
  final Color fg;
  final Color fgMuted;
  final Color accent;
  final Color onAccent;
  final Color water;
  final Color alert;
  final Color ok;
}

// Brand palette — the Pacific Northwest after the clouds come in.
const Color kFog = Color(0xFFF3F4F1);
const Color kBasalt = Color(0xFF1B1F1D);
const Color kEvergreen = Color(0xFF1F4D3A);
const Color kRiver = Color(0xFF2E6F8E);
const Color kAlpenglow = Color(0xFFE0683F);
const Color kHeadlamp = Color(0xFFF2A33A);
const Color kHiVis = Color(0xFFFFD23F);
const Color kSquall = Color(0xFF15202B);

const Map<Experience, AppTokens> kExperiences = <Experience, AppTokens>{
  Experience.dawn: AppTokens(
    brightness: Brightness.light,
    base: kFog,
    surface: Color(0xFFFFFFFF),
    surfaceAlt: Color(0xFFE3ECE6),
    line: Color(0xFFD8DDD8),
    fg: kBasalt,
    fgMuted: Color(0xFF5F6A64),
    accent: kEvergreen,
    onAccent: kFog,
    water: kRiver,
    alert: kAlpenglow,
    ok: kEvergreen,
  ),
  Experience.headlamp: AppTokens(
    brightness: Brightness.dark,
    base: Color(0xFF000000),
    surface: Color(0xFF0E0E0E),
    surfaceAlt: Color(0xFF1A1A1A),
    line: Color(0xFF262626),
    fg: Color(0xFFD9D2C2),
    fgMuted: Color(0xFF8F8777),
    accent: kHeadlamp,
    onAccent: Color(0xFF000000),
    water: Color(0xFFB08A4A),
    alert: Color(0xFFE05A3F),
    ok: kHeadlamp,
  ),
  Experience.squall: AppTokens(
    brightness: Brightness.dark,
    base: kSquall,
    surface: Color(0xFF1D2B38),
    surfaceAlt: Color(0xFF263645),
    line: Color(0xFF2D3F4E),
    fg: Color(0xFFE8EEF3),
    fgMuted: Color(0xFF9FB0BF),
    accent: kHiVis,
    onAccent: kSquall,
    water: Color(0xFF4FA3C7),
    alert: kHiVis,
    ok: Color(0xFF7FC8A3),
  ),
};

/// Red-shifted Headlamp for night vision: everything that was amber goes
/// deep red, everything else dims.
const AppTokens kHeadlampRed = AppTokens(
  brightness: Brightness.dark,
  base: Color(0xFF000000),
  surface: Color(0xFF0A0404),
  surfaceAlt: Color(0xFF160808),
  line: Color(0xFF2A0E0E),
  fg: Color(0xFFD0453A),
  fgMuted: Color(0xFF7A2A22),
  accent: Color(0xFFFF3B2F),
  onAccent: Color(0xFF000000),
  water: Color(0xFFA03A30),
  alert: Color(0xFFFF3B2F),
  ok: Color(0xFFD0453A),
);

const String kFontDisplay = 'BricolageGrotesque';
const String kFontBody = 'AtkinsonHyperlegible';
const String kFontMono = 'IBMPlexMono';

Experience experienceFromId(String? id) => Experience.values.firstWhere(
      (Experience e) => e.name == id,
      orElse: () => Experience.dawn,
    );

/// Build a Material 3 [ThemeData] from a token set.
ThemeData buildAppTheme(AppTokens t) {
  final bool dark = t.brightness == Brightness.dark;
  final ColorScheme scheme = ColorScheme(
    brightness: t.brightness,
    surface: t.base,
    onSurface: t.fg,
    surfaceContainerLowest: dark ? t.base : t.surface,
    surfaceContainerLow: t.surface,
    surfaceContainer: t.surface,
    surfaceContainerHigh: t.surfaceAlt,
    surfaceContainerHighest: t.surfaceAlt,
    surfaceDim: t.base,
    surfaceBright: t.surfaceAlt,
    onSurfaceVariant: t.fgMuted,
    primary: t.accent,
    onPrimary: t.onAccent,
    primaryContainer: t.surfaceAlt,
    onPrimaryContainer: t.fg,
    secondary: t.water,
    onSecondary: t.onAccent,
    secondaryContainer: t.surfaceAlt,
    onSecondaryContainer: t.fg,
    tertiary: t.ok,
    onTertiary: t.onAccent,
    error: t.alert,
    onError: t.onAccent,
    errorContainer: t.alert,
    onErrorContainer: t.onAccent,
    outline: t.line,
    outlineVariant: t.line,
    shadow: const Color(0xFF000000),
    scrim: const Color(0xCC000000),
    inverseSurface: t.fg,
    onInverseSurface: t.base,
    inversePrimary: t.base,
    surfaceTint: Colors.transparent,
  );

  final TextTheme base = ThemeData(brightness: t.brightness).textTheme;
  TextStyle disp(TextStyle? s, double size, FontWeight w) => (s ?? const TextStyle()).copyWith(
        fontFamily: kFontDisplay,
        fontSize: size,
        fontWeight: w,
        color: t.fg,
        letterSpacing: size >= 40 ? -size * 0.03 : -0.2,
        height: 1.05,
      );
  TextStyle body(TextStyle? s, double size, [FontWeight w = FontWeight.w400]) =>
      (s ?? const TextStyle()).copyWith(
        fontFamily: kFontBody,
        fontSize: size,
        fontWeight: w,
        color: t.fg,
        height: 1.4,
      );
  final TextTheme text = TextTheme(
    displayLarge: disp(base.displayLarge, 72, FontWeight.w800),
    displayMedium: disp(base.displayMedium, 56, FontWeight.w800),
    displaySmall: disp(base.displaySmall, 40, FontWeight.w800),
    headlineLarge: disp(base.headlineLarge, 30, FontWeight.w700),
    headlineMedium: disp(base.headlineMedium, 24, FontWeight.w700),
    headlineSmall: disp(base.headlineSmall, 20, FontWeight.w700),
    titleLarge: disp(base.titleLarge, 20, FontWeight.w700),
    titleMedium: body(base.titleMedium, 16, FontWeight.w700),
    titleSmall: body(base.titleSmall, 14, FontWeight.w700),
    bodyLarge: body(base.bodyLarge, 17),
    bodyMedium: body(base.bodyMedium, 15),
    bodySmall: body(base.bodySmall, 13).copyWith(color: t.fgMuted),
    labelLarge: body(base.labelLarge, 15, FontWeight.w700),
    labelMedium: (base.labelMedium ?? const TextStyle()).copyWith(
        fontFamily: kFontMono, fontSize: 12, color: t.fgMuted, letterSpacing: .6),
    labelSmall: (base.labelSmall ?? const TextStyle()).copyWith(
        fontFamily: kFontMono, fontSize: 11, color: t.fgMuted, letterSpacing: .8),
  );

  return ThemeData(
    useMaterial3: true,
    brightness: t.brightness,
    colorScheme: scheme,
    fontFamily: kFontBody,
    scaffoldBackgroundColor: t.base,
    canvasColor: t.base,
    dividerColor: t.line,
    cardColor: t.surface,
    textTheme: text,
    primaryTextTheme: text,
    iconTheme: IconThemeData(color: t.fg),
    primaryIconTheme: IconThemeData(color: t.fg),
    cardTheme: CardThemeData(
      color: t.surface,
      surfaceTintColor: Colors.transparent,
      elevation: 0,
      shape: RoundedRectangleBorder(
        borderRadius: BorderRadius.circular(12),
        side: BorderSide(color: t.line),
      ),
      margin: EdgeInsets.zero,
    ),
    appBarTheme: AppBarTheme(
      backgroundColor: t.base,
      foregroundColor: t.fg,
      surfaceTintColor: Colors.transparent,
      iconTheme: IconThemeData(color: t.fg),
      titleTextStyle: text.titleLarge,
      elevation: 0,
      centerTitle: false,
    ),
    listTileTheme: ListTileThemeData(iconColor: t.fgMuted, textColor: t.fg),
    dividerTheme: DividerThemeData(color: t.line, space: 1),
    filledButtonTheme: FilledButtonThemeData(
      style: FilledButton.styleFrom(
        backgroundColor: t.accent,
        foregroundColor: t.onAccent,
        minimumSize: const Size(64, 52),
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        textStyle: text.labelLarge?.copyWith(fontFamily: kFontDisplay, fontSize: 17),
      ),
    ),
    outlinedButtonTheme: OutlinedButtonThemeData(
      style: OutlinedButton.styleFrom(
        foregroundColor: t.fg,
        side: BorderSide(color: t.line),
        minimumSize: const Size(64, 48),
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
        textStyle: text.labelLarge?.copyWith(fontFamily: kFontDisplay, fontSize: 16),
      ),
    ),
    textButtonTheme: TextButtonThemeData(
      style: TextButton.styleFrom(foregroundColor: t.accent),
    ),
    chipTheme: ChipThemeData(
      backgroundColor: t.surfaceAlt,
      selectedColor: t.accent,
      labelStyle: text.labelMedium?.copyWith(color: t.fg),
      side: BorderSide(color: t.line),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(999)),
    ),
    inputDecorationTheme: InputDecorationTheme(
      filled: true,
      fillColor: t.surface,
      border: OutlineInputBorder(
        borderRadius: BorderRadius.circular(10),
        borderSide: BorderSide(color: t.line),
      ),
      enabledBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(10),
        borderSide: BorderSide(color: t.line),
      ),
      focusedBorder: OutlineInputBorder(
        borderRadius: BorderRadius.circular(10),
        borderSide: BorderSide(color: t.accent, width: 2),
      ),
      labelStyle: TextStyle(color: t.fgMuted),
    ),
    switchTheme: SwitchThemeData(
      thumbColor: WidgetStateProperty.resolveWith((Set<WidgetState> s) =>
          s.contains(WidgetState.selected) ? t.accent : t.fgMuted),
      trackColor: WidgetStateProperty.resolveWith((Set<WidgetState> s) =>
          s.contains(WidgetState.selected) ? t.accent.withValues(alpha: .4) : t.surfaceAlt),
    ),
    sliderTheme: SliderThemeData(
      activeTrackColor: t.accent,
      thumbColor: t.accent,
      inactiveTrackColor: t.line,
    ),
    snackBarTheme: SnackBarThemeData(
      backgroundColor: t.fg,
      contentTextStyle: TextStyle(color: t.base, fontFamily: kFontBody),
    ),
  );
}
