MassParticle
============

[![MassParticle](http://img.youtube.com/vi/TYdrYMecql0/0.jpg)](http://www.youtube.com/watch?v=TYdrYMecql0)


相互作用するパーティクルをいっぱい出す Unity のプラグインです。Windows とMac に対応しています。プラグインなので使用するには Pro 版の Unity が必要です。  
MPWorld の中がパーティクル群の生存できるエリアになっており、MPWorld を配置してその中に MPEmitter を配置することでパーティクルを発生させます。
デフォルトで Unity の通常の Collider とインタラクションを取るようになっており、Collider を配置すればそれと衝突判定をとって跳ね返るようになります。この挙動は MPColliderAttributes コンポーネントを追加することで変更することができます。
パーティクルの衝突、コールバック関数を登録するか、MPAPI.mpGetParticles() でパーティクルを巡回することで検出することができます。
(いずれもっと詳細なドキュメントを書く予定。具体的な使い方はサンプルシーンを参照)


This is massive particle plug-in for Unity.  It supports Windows and Mac.
(todo: write)
