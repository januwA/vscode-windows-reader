# Windows Reader

按下快捷键调用 Windows 系统 TTS 朗读文本。

## 功能特性

* 选中编辑器中的文本，按下 `Ctrl + Alt + S` 即可朗读。
* 支持在设置中调节语速和音量。

## 使用说明

1. 选中一段文字。
2. 按下快捷键 `Ctrl + Alt + S`。
3. 确保你的系统音量已打开。

## 扩展设置

此扩展提供以下设置：

* `windowsReader.rate`: 设置朗读语速 (-10 到 10)。
* `windowsReader.volume`: 设置朗读音量 (0 到 100)。

## 打包 VSIX

```sh
npm install -g @vscode/vsce
vsce package
```
