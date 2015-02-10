OBSニコ生プラグイン 通信プロトコル仕様書
========================================

プロトコルバージョン: NCLVP 0.1

概要
----

この仕様書は obs-rtmp-nicolive の外部命令受け付けサーバの通信プロトコルに関する仕様書である。以下、obs-rtmp-nicolive 側をサーバと定義し、接続するソフトをクライアントと定義する。

表記
----

サーバ側から送信されるデータは先頭に `S:` を、クライアント側の通信から送信されるデータは先頭に `C:` を付けて表記する。"C:" と "S:" という文字が実際に送信されるわけでは無い。

```
C: <クライアントから送信されるデータ>
S: <サーバから送信されるデータ>
```

`/` 同士で囲まれた表記は正規表現である。可能な正規表現の記法は Onigumo (鬼雲) に基づく。`\` から始まる表記は C11 における文字定数リテラルと同じとする。英数字 `/[A-Za-z0-9]/` とピリオド `.`、ハイフン `-`、アンダーバー `_` チルダ `~` はそのまま文字を表す。`<` と `>` で囲まれた表記は、特殊な文字やコマンドなどの表現を表す。表記内の空白 (`\x20`) は実際の空白では無く、ただの区切りであり、間に空白は存在しない。任意の文字列の後にオプションを表す `?` をつけることができる。`(` と対応する `)` でトークンをグループ化するができる。`=` で対応付けを表す。

### 特殊な文字

特殊な文字表記として、下記を用いる。

* `<SP> = \x20`
* `<CR> = \r`
* `<LF> = \n`
* `<CRLF> = <CR> <LF>`
* `<BS> = \b`
* `<DEL> = \x7F`
* `<HT> = \t`
* `<VT> = \v`
* `<NUL> = \0`

通信
----

通信は TCP/IP を用いる。サーバ側はループバックアドレスである IPv4 の 127.0.0.1 と IPv6 の ::1 で TCP 接続を受け付ける。受け付けるポートはデフォルトで 25083 である。外部マシンからの命令を直接受け取ることはできない。

クライアント側の送信元ポートは任意とする。ただし、IPv4については 127.0.0.1 以外の ループバックアドレス (127.0.0.2 など) からの接続は動作保証外とする。

通信は 8 bits を 1 word (1 byte) とする。ただし、文字については ASCII コード (0x00 - 0x7F) の 7bits 範囲を使用し、最上位ビットは使用してはいけない。ASCII 文字以外を送信する必要がある場合は、コマンドの項で個別にエンコード方法および文字コードを指定する。1 word 以上の数値を表す場合はネットワークバイトオーダー (ビッグエンディアン) とする。

サーバはクライアントからの TCP SYNC を受け付け、TCP ネゴシエーションを行う。TCP ネゴシエーション完了後、クライアントはコマンドを発行することができる。一つのコマンドに付き、サーバは処理終了後に応答コードを送信する。

サーバの応答前にクライアントが次のコマンドを送信した場合も、対応する応答コードを送信する。ただし、処理されるかどうかは保証されない。

1 分以上無通信状態が続いた場合、サーバは接続を切断する場合がある。

コマンド
--------

クライアントが送信するデータは全てコマンドである。
コマンドは下記形式になっている。

    C: <command> ( <SP> <target> ( <SP> <option> )? )? <CRLF>

* `<command> = /[A-Za-z][A-Za-z0-9_]*/`
* `<target> = /[A-Za-z][A-Za-z0-9_]*/`
* `<option> = /\x21-\x7E/`

`<command>` はコマンドを表す。`<target>` はコマンドを送るターゲットを表す。`<option>` はコマンドのオプションを表す。コマンドによって `<taregt>` と `<option>` が必要または不要が決まっている。不要な場合に負荷した場合はエラーになる。`<option>` は UTF-8 を URL エンコードした文字列でなければならない。デコードされた文字列は 256 **文字** 以内でなければならない。

telnet を使用した手動コマンドテストを想定し、下記が許される可能性がある。ただし、実装は任意とし、エラーになる可能性もある。アプリケーションから自動的にコマンドを発行する場合は下記が受け付け可能であることを想定してはならない。

* `<command>` および `<target>` は大文字小文字が無視される。 (すべて大文字として処理される)
* `<SP>` 部分は複数の連続する `<SP>` でもよい。
* `<command>` の前に任意の数の `<SP>` を置いてもよい。
* `<CRLF>` の前に任意の数の `<SP>` を置いてもよい。
* 上記について `<SP>` の代わりに `<HT>` または `<VT>` を用いてもよい。
* `<CRLF>` の代わりに `<LF>` のみでもよい。しかし、`<CR>` のみにしてはいけない。
* `<BS>` が含まれる場合、前の文字を削除して処理を行う。しかし、`<DEL>` が含まれる場合の動作は未定義とする。
* `<CRLF>` のみの場合は単純に無視され、何もしない。

制限事項として、一行は先頭から `<CRLF>` を含めて 4095 bytes 以内でなければならない。4095 bytes を越えた場合は強制的に切断される場合がある。

### GET

*未実装* 将来のバージョンで実装予定。

### SET

    C: SET <SP> <setting_name> <SP> <setting_value> <CRLF>

`<setting_name>`

* `SESSION` ユーザーセッション (user_session のクッキー値)

`<setting_name>` の設定に `<setting_value>` の値を設定する。設定可能な設定は下記である。`<setting_value>` は URL デコードされ UTF-8 として解釈される。設定値はサーバ側で記憶され、サーバ側のアプリケーションで再設定や終了を行わない限り保持される。しかし、サーバ側で設定ファイルに保存されるとは限らないため、再起動した後も保持されるかは状況依存である。

### STAT

    C: STAT <SP> <status_name> <CRLF>

`<status_name>`

* `SESSION` ニコ生に接続可能な有効なセッションを保持しているかどうか。実際に有効であるかの確認を行う。
* `STREAMING` 配信中かどうか。

`<status_name>` の状態を返す

### STRT

    C: STRT <SP> <service_name> <CRLF>

`<service_name>`

* `STREAMING` 配信を開始する。

サービスを開始する。

### STOP

    C: STATUS <SP> <service_name> <CRLF>

`<service_name>`

* `STREAMING` 配信を終了する。

サービスを終了するする。

### HELO

    C: HELO <CRLF>

プロトコルバージョンを返す。

### KEEP

    C: KEEP <CRLF>

キープアライブを行う。無通信状態の切断を防止する以外は、特に何も行わない。通信を保持したい場合は、1 分毎に実施することを推奨する。

### QUIT

    C: QUIT <CRLF>

接続を終了する。サーバ側はすぐさま切断を行い、応答は行わない。

応答コード
---------

    S: <code> <SP> <name> ( <SP> <desc> )? <CRLF>

* `<code> = /[1-9][0-9]{2}/`
* `<name> = /[A-Za-z][A-Za-z0-9_]*/`
* `<desc> = /\x21-\x7E/`

`<code>` は 3 桁の数字になる。1 桁目は 0 以外で、100 から 999 の値を取る。`<desc>` は無い場合がある。その場合は各応答コードの表で *none* で表記する。`<desc>` は UTF-8 を URL エンコードした文字列を返す。デコードした文字列は必ず 256 **文字** 以内になる。

応答コードは一行は先頭から `<CRLF>` を含めて 4095 bytes 以内になる。

### 100 番台 情報

| `<code>` | `<name>` | `<desc>`  | 意味                 |
|----------|----------|-----------|----------------------|
| 100      | HELO     | NCLVP_0.1 | プロトコルバージョン |

現バージョンではプロトコルバージョンは `NCLVP_0.1` で固定である。将来は `0.1` が更新される場合がある。マイナーバージョンアップの場合は互換性が維持される。

### 200 番台 成功

| `<code>` | `<name>` | `<desc>` | 意味 |
|----------|----------|----------|------|
| 200      | OK       | *none*   | 成功 |
| 210      | TRUE     | *none*   | 真   |
| 220      | FASE     | *none*   | 偽   |

### 300 番台 処理要求 (クライアント側で追加処理必要)

定義無し。

### 400 番台 失敗 (クライアント側の問題)

| `<code>` | `<name>`        | `<desc>`   | 意味                       |
|----------|-----------------|------------|----------------------------|
| 400      | SYNTAX_ERROR    | エラー内容 | 文法エラー                 |
| 401      | SIZE_OVER       | *none*     | 文字数が多すぎる           |
| 402      | INVALID_ENCODE  | *none*     | URL エンコード文字列が不正 |
| 410      | UNKNOWN_COMMAND | *none*     | 不明なコマンド             |
| 411      | UNKNOWN_TARGET  | *none*     | 不明なターゲット           |
| 412      | UNKNOWN_OPTION  | *none*     | 不明なオプション           |

エラー内容は無い場合がある。

### 500 番台 失敗 (サーバ側の問題)

| `<code>` | `<name>`     | `<desc>`   | 意味           |
|----------|--------------|------------|----------------|
| 500      | SERVER_ERR   | エラー原因 | サーバエラー   |
| 510      | BUSY         | *none*     | ビジー状態     |
| 520      | ALREADY_DONE | *none*     | 既に実行済み   |
| 590      | SERVER_STOP  | *none*     | サーバ停止     |
| 591      | MAX_CONNECT  | *none*     | 接続数オーバー |

エラー原因は無い場合がある。`ALREADY_DONE` は既に実行済みを表す。例えば、配信開始状態で配信開始を命令した場合に返す。

`590` 番台はコマンドの読込を待たず、強制的に切断する。

通信サンプル
------------

`#` 以下はコメントを表す。

```
# 通信切断
C: HELO <CRLF>
S: 100 <SP> HELO <SP> NCLVP_0.1 <CRLF>
C: KEEP <CRLF>
S: 200 <SP> OK <CRLF> # KEEP は何もしない。
C: STAT <SP> SESSION <CRLF>
S: 220 <SP> FALSE <CRLF>
C: SET <SP> SESSION <SP> user_session_123... <CRLF>
S: 200 <SP> OK # 設定が成功しただけで、セッションが有効かは確認していない。
C: STATUS <SP> SESSION <CRLF>
S: 210 <SP> TRUE <CRLF>
C: STAT <SP> STREAMING <CRLF>
S: 220 <SP> FALSE <CRLF>
C: STRT <SP> STREAMING <CRLF>
S: 200 <SP> OK <CRLF> # 開始を押しただけで、本当に開始されたかはわからない。
C: STAT <SP> STREAMING <CRLF> # 開始したことを確認すること。
S: 210 <SP> TRUE <CRLF>
C: STRT <SP> STREAMING <CRLF>
S: 520 <SP> ALREADY_DONE <CRLF> # すでに開始しされている。
C: STOP <SP> STREAMING <CRLF>
S: 200 <SP> OK <CRLF>
C: STAT <SP> STREAMING <CRLF>
S: 220 <SP> FALSE <CRLF>
C: QUIT <CRLF>
# 通信切断
```

補足
----

### URL エンコードと URL デコード

URL エンコードと URL デコードは Qt5 ライブラリの `QUrl::toPercentEncoding()` と `QUrl::fromPercentEncoding()` を使用する。ただし、デコード前に "+" は " " に変換しするが、エンコード時は " " は "%20" に変換する。英数字 `/[A-Za-z0-9]/` とピリオド `.`、ハイフン `-`、アンダーバー `_` チルダ `~` はデコードされず、そのままになる。

### セキュリティ

認証機能は無いため、同一コンピュータ内の任意のアプリケーションから接続できる。将来のバージョンでは簡易なパスワード認証機能を付けるかもしれない。暗号化の機能を付ける予定は現在の所ない。サーバのアドレスはハードコードされるため、ループバックアドレス以外に変更できない。外部のコンピュータから接続する場合は、セキュリティを考慮して SSH forward 等を使用することを推奨する。

応答コードは 4095 bytes 以内であると規定しているが、クライアント側は 4096 bytes 以上のデータを受け取ることも考慮して作るべきである (その場合はサーバ側の不具合であるため、クライアントは処理を中断すべきである)。