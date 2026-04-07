// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
import * as vscode from 'vscode';
import { exec } from 'node:child_process';

export function activate(context: vscode.ExtensionContext) {
	let disposable = vscode.commands.registerCommand('vscode-windows-reader.readText', () => {
		const editor = vscode.window.activeTextEditor;
		if (!editor) {
			vscode.window.showErrorMessage('没有打开的编辑器！');
			return;
		}

		const text = editor.document.getText(editor.selection);
		if (!text || text.trim().length === 0) {
			vscode.window.showWarningMessage('请先选中一段文字再执行。');
			return;
		}

		const config = vscode.workspace.getConfiguration('windowsReader');
		const speechRate = config.get<number>('rate') || 0;
		const speechVolume = config.get<number>('volume') || 100;

		const base64Text = Buffer.from(text, 'utf8').toString('base64');

		const psScript = [
			`Add-Type -AssemblyName System.Speech;`,
			`$s = New-Object System.Speech.Synthesis.SpeechSynthesizer;`,
			`$s.Rate = ${speechRate};`,
			`$s.Volume = ${speechVolume};`,
			`$t = [System.Text.Encoding]::UTF8.GetString([System.Convert]::FromBase64String('${base64Text}'));`,
			`$s.Speak($t);`
		].join(' ');

		vscode.window.setStatusBarMessage('$(mic) 正在朗读...', 3000);

		exec(`powershell -Command "${psScript}"`, (error) => {
			if (error) {
				vscode.window.showErrorMessage(`播放失败: ${error.message}`);
			}
		});
	});

	context.subscriptions.push(disposable);
}

// This method is called when your extension is deactivated
export function deactivate() { }
