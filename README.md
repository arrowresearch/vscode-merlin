# VS Code Merlin Extension

🚧 This is work in progress, use at your own risk 🚧

## Prerequisites

This extension requires global installation of `esy >= 0.6.0` to allow building of [`ocaml-lsp-server`](https://github.com/ocaml/ocaml-lsp#esy) for your projects.
```
npm i -g @esy-nightly/esy
or
yarn global add @esy-nightly/esy
```

## Producing the Extension Package

```bash
# Clone the repo
git clone git@github.com:arrowresearch/vscode-merlin.git

# Install dependencies
cd vscode-merlin
npm i

# Build from reason
npm run build

# Produce package
npm run package

# Install produced package
code --install-extension vscode-merlin-[VERSION].vsix
```

## Usage
- Open any of your projects (esy/bsb/opam are supported)
- Open any Reason or Ocaml file in it
- Extension will start initialization, vscode will display progressbar (usually in the bottom left corner)
- Be mindful, that first initialization can take significant amount of time, as extension will download and compile right version of `ocaml`, `ocaml-lsp` and other dependencies. Future inits in similar project (with the same version of compiler) will be almost instant.
- If extension fails to initialize for some reason, please [file an issue](https://github.com/arrowresearch/vscode-merlin/issues)


## Running the Extension in Development Mode

- Run `npm install` in this directory. This installs all necessary npm modules
  in both the client and server directory
- Open VS Code on this directory.
- Press `Ctrl+Shift+B` to compile the client and server.
- Switch to the Debug viewlet.
- Select `Launch Client` from the drop down.
- Run the launch config.
