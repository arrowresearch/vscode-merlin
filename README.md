# VS Code Merlin Extension

**THIS IS WORK IN PROGRESS, DO NOT USE**

## Producing the Extension Package

- Run `npm run package` to produce `*.vsix` extension package.
- Run `code --install-extension *.vsix` to install the extension with VS Code.

## Running the Extension in Development Mode

- Run `npm install` in this directory. This installs all necessary npm modules
  in both the client and server directory
- Open VS Code on this directory.
- Press `Ctrl+Shift+B` to compile the client and server.
- Switch to the Debug viewlet.
- Select `Launch Client` from the drop down.
- Run the launch config.
