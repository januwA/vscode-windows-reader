import * as vscode from 'vscode';
import { execFile } from 'node:child_process';
import * as path from 'node:path';

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
        const speechRate = config.get<number>('rate') ?? 0;
        const speechVolume = config.get<number>('volume') ?? 100;

        // --- 核心修改：定位 EXE 路径 ---
        // context.extensionPath 是插件安装后的根目录
        const exePath = path.join(context.extensionPath, 'bin', 'cpp-windows-reader.exe');

        const statusBarMessage = vscode.window.setStatusBarMessage('$(mic) 正在朗读 (背景音已压低)...');

        // 使用 execFile 直接运行二进制，参数通过数组传递，无需担心引号转义
        execFile(exePath, [text, speechRate.toString(), speechVolume.toString()], (error) => {
            statusBarMessage.dispose(); // 朗读结束（EXE退出）后移除状态栏消息
            
            if (error) {
                // 如果是系统找不到文件，通常是打包路径不对
                if ((error as any).code === 'ENOENT') {
                    vscode.window.showErrorMessage(`找不到核心组件：${exePath}`);
                } else {
                    vscode.window.showErrorMessage(`朗读出错: ${error.message}`);
                }
            }
        });
    });

    context.subscriptions.push(disposable);
}

export function deactivate() { }
