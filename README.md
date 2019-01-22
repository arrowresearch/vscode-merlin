# VS Code Merlin Extension

## Producing the Extension Packagea

- Run `npm run package` to produce `*.vsix` extension package.
- Run `code --install-extension *.vsix` to install the extension with VS Code.

## Running the Extension in Development Mode

- Run `npm install` in this folder. This installs all necessary npm modules in
  both the client and server folder
- Open VS Code on this folder.
- Press Ctrl+Shift+B to compile the client and server.
- Switch to the Debug viewlet.
- Select `Launch Client` from the drop down.
- Run the launch config.
