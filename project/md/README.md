# プロジェクトのMarkdown資料

このフォルダには、ゲームのコード作業やチャット間の引き継ぎに使うMarkdown資料をまとめています。不要になった資料を整理するときは、このフォルダを確認してください。

- [GAME_DESIGN_HANDOFF.md](GAME_DESIGN_HANDOFF.md)：ゲーム概要、進行方針、実装履歴。
- [CLEAR_EFFECT_HANDOFF.md](CLEAR_EFFECT_HANDOFF.md)：次の要素別コンテスト「クリア演出」の目的と、新しいコード作業チャットに渡す文。
- [HANDOFF_SECOND_PROGRESS_PRESENTATION.md](HANDOFF_SECOND_PROGRESS_PRESENTATION.md)：2回目の進捗発表に関するコード・発表資料の引き継ぎ。
- [EFFECTS_HANDOFF.md](EFFECTS_HANDOFF.md)：採掘破片・戦闘時の血・ハンマーの土飛散エフェクト。ハンマー用は武器完成後に着手。
- [WEAPONS_HANDOFF.md](WEAPONS_HANDOFF.md)：剣・ハンマー・やり、簡易モデル、武器別攻撃モーション。
- [CLASS_REFACTOR_HANDOFF.md](CLASS_REFACTOR_HANDOFF.md)：Player・Enemy・Weapon などへの段階的なクラス分割。

発表用のカンペなど成果物として使うMarkdownは、`presentation_output` に置いています。

## 関連作業の推奨順

コード変更を複数チャットで同時に行わず、事前確認をしてから一つずつ進めます。クラス分割と武器追加のどちらを先にするかは、依存範囲を確認して決めます。エフェクトは採掘・被弾の基本分を進め、ハンマーの土エフェクトだけはハンマーの振り下ろしと接地判定が完成・確認された後に行います。
