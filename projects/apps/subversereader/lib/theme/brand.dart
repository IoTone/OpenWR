// Copyright (c) 2026 IoTone, Inc.
// SPDX-License-Identifier: MIT
//
// The app's look lives behind one interface, so a direction from the brand
// board (https://claude.ai/artifact/34UM1C9wEE8pGQBmDmoxD3) can be added or
// swapped without touching the screens. Xerox Punk (direction 5) is first.
import 'package:flutter/material.dart';

import 'punk.dart';

abstract class Brand {
  String get id;
  String get label;

  ThemeData get theme;

  /// The name as this brand writes it. [onDark]: drawn on a dark ground.
  Widget wordmark({double size = 30, bool onDark = false});

  /// The square mark, for the splash and the app bar.
  Widget logo({double size = 48});

  /// Full-screen splash shown while the catalog loads.
  Widget splash();

  /// A list card around a catalog row; [index] lets a brand vary them.
  Widget card({required int index, required Widget child, VoidCallback? onTap});

  // Copy that carries the brand's voice. Keep it plain enough to act on.
  String get catalogTitle;
  String get catalogTag;
  String get navZines;
  String get navPass;
  String sendLabel(String pass);
  String sentLabel(double seconds, double kb);
  String get splashLine;
}

final List<Brand> kBrands = <Brand>[PunkBrand()];

Brand brandById(String? id) => kBrands.firstWhere((Brand b) => b.id == id, orElse: () => kBrands.first);
