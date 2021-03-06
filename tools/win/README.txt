RTMP NicoLive Plugin for OBS MultiPlatform
==========================================

ニコニコ生放送用 OBS MultiPlatform プラグイン

概要
----

ニコニコ生放送の配信用URLとストリームキーを自動取得して、OBS MultiPlatform
(obs-studio) で簡単に配信できるようにするプラグインです。

インストール方法
----------------

始めに、OBS MulitPlatform 0.8.2 を https://github.com/jp9000/obs-studio/releases
からダウンロードして、展開して下さい。展開したフォルダに 「obs-plugins」と
「data」をそのままドラッグ＆ドロップしてフォルダを結合して下さい。

使い方
------

配信先を「ニコニコ生放送」にして、ユーザ名とパスワードいれて、「配信開始」した
ら、枠を取っているとそのまま配信されます。RTMPのURLやキーを手動で取得する必要は
ありません。

このプラグイン自体には枠を自動で取ったり、延長したりすることはできませんので、他
のコメビュとかとあわせて使って下さい。

### 便利かもしれない機能その1 Viqo連携 ###

「Viqoの設定を読み込む」を有効にすると、Viqo https://github.com/diginatu/Viqo の
設定からユーザーセッションを取得できます。Viqoを使用している人はセッションを消費
しなくてすみます。

### 便利かもしれない機能その2 ビットレート調整 ###

「映像ビットレートを自動調整」を有効にすると、ニコ生で配信可能なビットレート値を
確認して映像ビットレートを自動的に調整してくれます。ただ、音声ビットレートは調整
してくれません。本当に動いているかは作者もよくわかってないです。

### 便利かもしれない機能その3 監視 ###

「自動で配信開始と枠移動を行う」を有効にすると監視間隔ごとにニコ生の情報を見て、
配信ができるようであれば配信を開始してくれます。また、放送が終了している場合も自
動的に終了します。枠が切り替わるタイミングの場合、放送終了予定時間から約10秒後に
確認し、新しい放送枠があればそちらに配信しなおしてくれます。

Viqoの自動枠取り機能とあわせて使うとほぼ自動化されて、とても楽です。

### 便利かもしれない機能その4 外部コマンドサーバー(β版) ###

「外部コマンドサーバを使用」を有効にすると外部から操作できるTCPサーバーが内部で
起動します。telnetを使ってちょっと操作から、外部アプリでの本格操作まで、何でも…
できるようにはなる予定です。今のところ、ユーザーセッションの設定と配信の開始/終
了ができます。通信方法の詳細は doc/PROTOCOL.md を見て下さい。
