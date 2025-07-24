const vscode = require('vscode');
const path = require('path');

/**
 * @param {vscode.ExtensionContext} context
 */
function activate(context) {
    console.log('LAML extension is now active!');

    // Register the run command
    let runFileCommand = vscode.commands.registerCommand('laml.runFile', function (uri) {
        // Get the file path
        let filePath;
        if (uri && uri.fsPath) {
            filePath = uri.fsPath;
        } else if (vscode.window.activeTextEditor) {
            filePath = vscode.window.activeTextEditor.document.fileName;
        } else {
            vscode.window.showErrorMessage('No LAML file selected');
            return;
        }

        // Check if it's a .lm file
        if (!filePath.endsWith('.lm')) {
            vscode.window.showErrorMessage('This is not a LAML file (.lm extension required)');
            return;
        }

        // Create and show terminal
        let terminal = vscode.window.createTerminal({
            name: 'LAML Runner',
            cwd: path.dirname(filePath)
        });
        
        terminal.show();
        
        // Run the LAML file
        const fileName = path.basename(filePath);
        terminal.sendText(`laml run "${fileName}"`);
    });

    context.subscriptions.push(runFileCommand);
}

function deactivate() {}

module.exports = {
    activate,
    deactivate
};
