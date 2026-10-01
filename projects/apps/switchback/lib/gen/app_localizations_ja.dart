// ignore: unused_import
import 'package:intl/intl.dart' as intl;
import 'app_localizations.dart';

// ignore_for_file: type=lint

/// The translations for Japanese (`ja`).
class AppLocalizationsJa extends AppLocalizations {
  AppLocalizationsJa([String locale = 'ja']) : super(locale);

  @override
  String get appTitle => 'Switchback';

  @override
  String get aboutSubtitle => 'OpenWR のアプリ';

  @override
  String aboutVersion(String version) {
    return 'バージョン $version';
  }

  @override
  String get aboutDescription =>
      'goggl1 を Bluetooth でペアリング・設定・着せ替え・更新し、目の前のガラスでセッションを走る。Open. Weird. Wild. Strong. #PNWbuilt';

  @override
  String get aboutCopyright => '© 2024–2026 IoTone, Inc.';

  @override
  String get aboutLicense => 'MIT ライセンス';

  @override
  String get aboutLicensesButton => 'オープンソースライセンス';

  @override
  String get aboutMadeWith => 'Flutter で構築。';

  @override
  String get quickNav => 'クイックナビ';

  @override
  String get commonCancel => 'キャンセル';

  @override
  String get commonClose => '閉じる';

  @override
  String get commonRetry => '再試行';

  @override
  String get commonCopy => 'コピー';

  @override
  String get commonCopied => 'コピーしました';

  @override
  String get scanStart => 'スキャン開始';

  @override
  String get scanStop => 'スキャン停止';

  @override
  String get scanScanning => 'スキャン中…';

  @override
  String get scanIdle => '待機中';

  @override
  String get bluetoothOn => 'Bluetooth 準備完了';

  @override
  String get bluetoothOff => 'Bluetooth がオフです';

  @override
  String get bluetoothUnauthorized => 'Bluetooth の許可が必要です';

  @override
  String get bluetoothUnknown => 'Bluetooth の状態は不明です';

  @override
  String get radarEmpty => '範囲内にデバイスがありません。';

  @override
  String get radarSelf => '自分';

  @override
  String get radarLegendBle => 'Bluetooth デバイス';

  @override
  String get radarLegendWifi => 'WiFi ネットワーク';

  @override
  String get radarLegendRings => 'リング = おおよその信号距離（目安）。';

  @override
  String rssiDbm(int rssi) {
    return '$rssi dBm';
  }

  @override
  String metersApprox(String meters) {
    return '≈ $meters m';
  }

  @override
  String get signalStrong => '強';

  @override
  String get signalMedium => '中';

  @override
  String get signalWeak => '弱';

  @override
  String get deviceConnect => '接続';

  @override
  String get settingsAppearance => '外観';

  @override
  String get settingsTheme => 'テーマ';

  @override
  String get settingsFontScale => '文字サイズ';

  @override
  String get settingsReduceMotion => '動きを減らす';

  @override
  String get settingsHighContrast => 'ハイコントラスト';

  @override
  String get settingsLanguage => '言語';

  @override
  String get settingsLanguageSystem => 'システム既定';

  @override
  String get settingsPermissions => '権限';

  @override
  String get settingsDiagnostics => '診断';

  @override
  String get settingsDiagnosticsSub => 'スキャンイベントログ';

  @override
  String get settingsAbout => '情報';

  @override
  String get settingsScanner => 'スキャナー';

  @override
  String get settingsRssiFloor => '信号フロア';

  @override
  String settingsRssiFloorValue(int dbm) {
    return '$dbm dBm より弱い信号を非表示';
  }

  @override
  String get settingsRssiFloorOff => 'すべての信号を表示';

  @override
  String get permBle => 'Bluetooth';

  @override
  String get permLocation => '位置情報';

  @override
  String get permLocationWhy => 'Android では近くのデバイスのスキャンに位置情報の許可が必要です。';

  @override
  String get permGranted => '許可済み';

  @override
  String get permDenied => '拒否';

  @override
  String get permRequest => 'リクエスト';

  @override
  String get permOpenSettings => '設定を開く';

  @override
  String get diagEmpty => 'スキャン活動はまだありません。';

  @override
  String get diagClear => 'クリア';

  @override
  String get diagCopy => 'ログをコピー';

  @override
  String get firstRunTitle => 'Switchback へようこそ';

  @override
  String get firstRunBody => 'このアプリは goggl1 と Bluetooth で通信します。見つけるために権限が必要です。';

  @override
  String get firstRunPermBle => 'Bluetooth — ゴーグルを見つけて通信するため。';

  @override
  String get firstRunPermLocation => '位置情報 — Android では Bluetooth スキャンに必要です。';

  @override
  String get firstRunGrant => '権限を許可';

  @override
  String get firstRunSkip => '後で';

  @override
  String get langEnglish => 'English';

  @override
  String get langJapanese => '日本語';

  @override
  String get settingsTitle => '設定';

  @override
  String get settingsExperience => 'エクスペリエンス';

  @override
  String get tabTrailhead => '登山口';

  @override
  String get tabSession => 'セッション';

  @override
  String get tabLooks => '見た目';

  @override
  String get tabConsole => 'コンソール';

  @override
  String get tabUpdate => '更新';

  @override
  String get experiencePinned => '固定中';

  @override
  String get experienceAuto => '自動';

  @override
  String experienceNow(String name) {
    return '現在: $name';
  }

  @override
  String feelsLike(int t) {
    return '体感 $t';
  }

  @override
  String hiLo(int hi, int lo) {
    return '最高 $hi · 最低 $lo';
  }

  @override
  String windNow(int v, String unit, String dir) {
    return '風 $v $unit $dir';
  }

  @override
  String get chipDry8h => '8時間 乾燥';

  @override
  String chipRainAt(String t) {
    return '雨 $t';
  }

  @override
  String chipUnits(String u) {
    return '$u';
  }

  @override
  String get trailheadNoForecastTitle => '予報はまだありません';

  @override
  String get trailheadWaitingForecast =>
      'ゴーグルは接続済みで取得中です。数秒待つか、コンソールで WiFi を確認してください。';

  @override
  String get trailheadNotConnected => 'ゴーグルをペアリングすると、同じ景色が見えます。';

  @override
  String get startSession => 'セッション開始';

  @override
  String get pairGoggle => 'ゴーグルをペアリング';

  @override
  String get headlineStormNow => '雷雨です。屋内にいてください。';

  @override
  String get headlineWetAllWindow => '今後8時間は雨です。';

  @override
  String headlineClearsAt(String t) {
    return '$t ごろに上がります。';
  }

  @override
  String headlineRainAt(String t, int pop) {
    return '$t から雨、$pop% の確率。その前に出発を。';
  }

  @override
  String headlineWindy(int g) {
    return '乾燥していますが突風 $g。風を避けるルートを。';
  }

  @override
  String get headlineDryDay => '今後8時間は乾燥。行きましょう。';

  @override
  String get headlineDryNight => '今夜は晴れて乾燥。ヘッドランプを。';

  @override
  String squallWarning(String t, int pop, int gust, String unit) {
    return '雨 $t · $pop% · 突風 $gust $unit';
  }

  @override
  String get goggleNotConnected => '未接続';

  @override
  String get goggleConnecting => '接続中…';

  @override
  String get goggleConnected => '接続済み';

  @override
  String get ageUnknown => '時計なし';

  @override
  String get ageJustNow => 'たった今';

  @override
  String ageMinutes(int n) {
    return '$n 分前';
  }

  @override
  String ageHours(int n) {
    return '$n 時間前';
  }

  @override
  String get pairTitle => 'ペアリング';

  @override
  String get pairHeadline => 'BOOT を1秒長押し。';

  @override
  String get pairBody => 'ガラスに B が表示され、ゴーグルは3分間待ち受けます。Switchback が見つけて接続します。';

  @override
  String get pairScanning => 'goggl1 を探しています…';

  @override
  String get pairFailed => '接続できませんでした。もう一度 BOOT を長押ししてください。';

  @override
  String get pairNothingYet => 'まだ見つかりません。ガラスに B は出ていますか？';

  @override
  String get pairFindRadar => 'レーダーで探す';

  @override
  String get radarLegendGoggle => 'goggl1';

  @override
  String get looksGlassTitle => 'ガラスの表示';

  @override
  String get looksGlassBody => 'プレビューはゴーグル自身の描画です。タップで切り替え。';

  @override
  String get looksNotConnected => '表示を変えるにはゴーグルをペアリングしてください。';

  @override
  String get looksAnimation => 'アニメーション';

  @override
  String get looksAnimationSub => '雨が降り、雪が舞い、嵐が光り、星がまたたく';

  @override
  String get looksBrightness => 'ガラスの明るさ';

  @override
  String get looksAppTitle => 'このアプリ';

  @override
  String get looksAppBody => '見た目はゴーグルが見ている天気に従います。固定もできます。';

  @override
  String get looksRedShift => 'レッドシフト';

  @override
  String get looksRedShiftSub => '暗視のため Headlamp を深い赤に';

  @override
  String get glassStripBlurb => 'チャート。今後8時間の形。';

  @override
  String get glassGlanceBlurb => '大きな数字と一文。';

  @override
  String get glassRibbonBlurb => '毎時を列で。';

  @override
  String get consoleNotConnected => 'コンソールにはゴーグルが必要です。';

  @override
  String get consoleHistory => '前の行';

  @override
  String get consoleSend => '送信';

  @override
  String get sessionTitle => 'セッション';

  @override
  String get sessionBody =>
      'スマホがセンサーハブ、ガラスがディスプレイ。種目を選んで開始すると、時間・ペース・距離が毎秒ゴーグルへ送られます。';

  @override
  String get sportRun => 'ラン';

  @override
  String get sportRide => 'ライド';

  @override
  String get sportSki => 'スキー';

  @override
  String get sportPaddle => 'パドル';

  @override
  String get sportClimb => 'クライム';

  @override
  String get sessionComingSoon => 'セッションは次のビルドで。';

  @override
  String get sessionPhaseNote => '予定: session-hud バンドル、GPS ペースと距離、心拍ストラップ。';

  @override
  String get updateTitle => '更新';

  @override
  String updateRunning(String v, String b) {
    return 'v$v、$b を実行中。';
  }

  @override
  String get updateReadDetails => '詳細を読む';

  @override
  String get updateOtaTitle => 'ファームウェアを無線で更新';

  @override
  String get updateOtaBody =>
      'ゴーグルと同じ WiFi でイメージを配信し、URL を貼り付けます。ゴーグルは予備スロットに書き込んで再起動。不良イメージはロールバックします。';

  @override
  String get updateUrlLabel => 'イメージの URL';

  @override
  String get updateStart => '更新';

  @override
  String get updateConfirmTitle => 'ファームウェアを更新しますか？';

  @override
  String updateConfirmBody(String url) {
    return 'ゴーグルが $url を取得して書き込み、再起動します。';
  }

  @override
  String get updateStarted => '更新を開始しました。進行はコンソールに表示されます。';

  @override
  String get updatePhaseNote => '予定: Switchback 自身がイメージを配信し、ユースケースのバンドルを送ります。';

  @override
  String get setupTitle => 'ゴーグルの設定';

  @override
  String get setupSettingsSub => 'WiFi、位置、単位';

  @override
  String get setupNotConnected => '先にゴーグルをペアリングしてください。カードは順に送信されます。';

  @override
  String get setupWifiTitle => 'WiFi';

  @override
  String get setupWifiBody => 'ゴーグルは自分で予報を取得するのでネットワークが必要です。2.4 GHz のみ。';

  @override
  String get setupWifiPick => 'ネットワークを選ぶ';

  @override
  String get setupWifiSsid => 'ネットワーク名';

  @override
  String get setupWifiPassword => 'パスワード';

  @override
  String get setupWifiSend => 'ゴーグルへ送信';

  @override
  String get setupWifiSending => '送信中、アドレスを待っています…';

  @override
  String setupWifiUp(String ip) {
    return '接続しました。$ip';
  }

  @override
  String get setupWifiNotYet => '30秒待ってもアドレスがありません。パスワードを確認するか、まだ接続中かもしれません。';

  @override
  String setupWifiState(String ip) {
    return '$ip';
  }

  @override
  String get setupWifiOff => 'ネットワークなし';

  @override
  String get setupLocationTitle => '位置';

  @override
  String get setupLocationBody =>
      'そのままだとゴーグルは IP アドレスから都市単位で位置を決めます。手動またはこのスマホから正確な地点を設定でき、IP 検索は止まります。';

  @override
  String get setupLocationUsePhone => 'スマホの位置を使う';

  @override
  String get setupLocationSetManual => '設定';

  @override
  String get setupLocationBackToAuto => '自動に戻す';

  @override
  String get setupLocationReading => 'スマホの位置を取得中…';

  @override
  String get setupLocationDenied => '位置情報の権限が拒否されました。';

  @override
  String get setupLocationInvalid => '緯度と経度を度で入力してください。';

  @override
  String get setupLocationSending => '送信して予報を取得中…';

  @override
  String setupLocationSet(String city) {
    return '設定しました。$city の予報。';
  }

  @override
  String get setupLocationLookingUp => 'IP を検索中…';

  @override
  String get setupLocationLookupFailed => '検索に失敗。以前の位置のままです。';

  @override
  String setupLocationManual(String city) {
    return '$city、手動';
  }

  @override
  String setupLocationAuto(String city) {
    return '$city、IP から';
  }

  @override
  String get setupUnitsTitle => '単位';

  @override
  String get setupUnitsBody => '既定は国から選ばれます。ガラスとこのアプリが従います。';

  @override
  String get setupUnitsMetric => '°C · km/h';

  @override
  String get setupUnitsImperial => '°F · mph';

  @override
  String get updateNotConnected => 'ゴーグルをペアリングすると実行中のものが見えます。';

  @override
  String get updateEmbeddedTitle => 'このアプリ内のリリース';

  @override
  String get updateEmbeddedBody =>
      'Switchback に組み込まれた署名付き .mnpkg パッケージ。スマホが WiFi 経由で配信し、ゴーグルは署名・ハッシュ・対象を確認してから書き込みます。';

  @override
  String get updateNoReleases => 'このビルドにはリリースがありません。';

  @override
  String get updateInstall => 'インストール';

  @override
  String get updateInstalled => '実行中';

  @override
  String get updateServing => '配信中…';

  @override
  String updateServingAt(String ip, int port, int n) {
    return '$ip:$port で配信中、$n 回取得';
  }

  @override
  String updateInstallBody(String fw, String version, String variant) {
    return 'goggl1 $fw（$version、$variant）をインストールしますか？ゴーグルはこのスマホから取得・検証し、再起動します。';
  }

  @override
  String updateProgress(int pct) {
    return '書き込み中… $pct%';
  }

  @override
  String get updateRebooting => '検証・準備完了。ゴーグルは再起動中です。数秒後にもう一度ペアリングしてください。';

  @override
  String get updateNoVerdict => '150秒以内に結果がありません。コンソールを確認してください。';

  @override
  String get updateBundleTitle => 'このアプリ内のユースケース';

  @override
  String get updateBundleBody =>
      'このビルドに同梱の天気 HUD バンドル。送るとゴーグルのものを置き換え、再起動後も保持されます。';

  @override
  String get updateBundlePush => '天気 HUD を送る';

  @override
  String get updateBundleSending => 'バンドル送信中…';

  @override
  String pairFound(int n) {
    return 'ゴーグルが $n 台見つかりました';
  }

  @override
  String pairPhaseConnecting(int n) {
    return '接続中、$n/2 回目…';
  }

  @override
  String get pairPhaseReading => '接続済み、状態を読み取り中…';

  @override
  String pairPhaseRetrying(int n) {
    return '最初の接続が切れました。再試行中（$n 回目）…';
  }

  @override
  String get pairPhaseConnected => '接続済み';

  @override
  String get pairPhaseFailed => '接続できませんでした';

  @override
  String get pairPhaseDropped => 'ゴーグルが切断されました';

  @override
  String get pairFailedHint =>
      'ゴーグルは電源投入後と切断後の3分間だけ待ち受けます。ガラスの B が消えていたら挿し直してから、接続をタップしてください。';

  @override
  String get looksSending => '送信中…';

  @override
  String looksSendFailed(String e) {
    return 'ゴーグルが応答しません: $e';
  }

  @override
  String get sessionGlassReady => 'ゴーグルは接続済み。セッションを表示します。';

  @override
  String get sessionNoGlass => 'ゴーグル未接続。セッションはスマホだけで動きます。';

  @override
  String sessionPace(String unit) {
    return '分/$unit';
  }

  @override
  String get sessionStop => '停止';

  @override
  String sessionDone(String time, String dist, String unit) {
    return '完了: $time、$dist $unit。';
  }

  @override
  String get sessionLogTitle => '最近のセッション';
}
