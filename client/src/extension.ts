/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */

import * as fs from "fs";
import * as path from "path";
import { workspace, ExtensionContext, commands } from "vscode";
import * as semver from 'semver';

import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
} from "vscode-languageclient";

import { log, run, options } from './utils';
import { getEsyStatus } from './esy-utils';
import { ToolNotFound, OpamNotFound, EsyNotFound, OpamOnWindowsError } from './errors';
import { promisify } from "util";

let client: LanguageClient;

async function getCommandForWorkspace() {

  let root = workspace.rootPath;
  let esyStatus = await getEsyStatus(root);
  if (esyStatus.isProject) {

    // All npm and opam projects are valid esy projects too! Picking
    // the right package manager is important - we don't want to run
    // `esy` for a user who never intended to. Example: bsb/npm
    // users. Similarly, opam users wouldn't want prompts to run
    // `esy`. Why is prompting `esy i` even necessary in the first
    // place? `esy ocamlmerlin-lsp` needs projects to install/solve deps

    let manifestFile = esyStatus.rootPackageConfigPath;
    // Looking for manifest file. If it is not a json file, it's an opam project
    if (/\.json$/.test(manifestFile)) {

      let manifest;
      try {
        manifest = fs.readFileSync(manifestFile).toString();
      } catch (e) {
        return Promise.reject(e);
      }

      try {
        manifest = JSON.parse(manifest);
      } catch (e) {
        return Promise.reject(e);
      }

      if (/esy\.json$/.test(manifestFile) || !!manifest.esy) {
        // Esy is indeed being used to manage this project
      } else {
        // Esy may not have been used. Let's drop and esy.json with the an appropriate compiler, reason and merlin-lsp
        // Note that this check (npm vs esy) needs to be done only once. Since all npm projects will eventually end up having an esy.json with the toolchain
        let ocaml;
        if (manifest.dependencies && manifest.dependencies['bs-platform'] && semver.lt(manifest.dependencies['bs-platform'], '6.x')) {
          ocaml = '4.2.x';
        } else {
          ocaml = '4.6.x';
        }
        let reason = "*";
        let merlinLsp = "ocaml/merlin:merlin-lsp.opam#f030d5da7a"

        // Creating esy.json
        fs.writeFileSync(path.join(root, 'esy.json'), JSON.stringify({ dependencies: { ocaml, reason, "merlin-lsp": merlinLsp } }));

        // Running esy i && esy b
        log('Creating esy.json and setting up toolchain');
        await run('esy', { cwd: root })

        // It could be an esy or npm
        let command = process.platform === "win32" ? "esy.cmd" : "esy";
        let args = ["exec-command", '--include-current-env', "ocamlmerlin-lsp"];
        return Promise.resolve({ command, args });
      }
    } else {
      try {
        // Check if opam is installed on the system
        try {
          await run('command -v opam')
        } catch (e) {
          return Promise.reject(new OpamNotFound())
        }

        // Check if ocamlmerlin-lsp and ocamlmerlin-reason are installed
        try {
          await run('opam exec command -- -v ocamlmerlin-lsp')
        } catch (e) {
          throw new ToolNotFound('opam', 'ocamlmerlin-lsp');
        }
        try {
          await run('opam exec command -- -v ocamlmerlin-reason');
        } catch (e) {
          throw new ToolNotFound('opam', 'ocamlmerlin-reason')
        }
        if (process.platform === "win32") {
          return Promise.reject(new OpamOnWindowsError());
        } else {
          return Promise.resolve({ command: 'opam', args: ['exec', 'ocamlmerlin-lsp'] })
        }
      } catch (e) {
        return Promise.reject(e);
      }
    }

  } else {
    // let command =
    //   process.platform === "win32" ? "ocamlmerlin-lsp.exe" : "ocamlmerlin-lsp";
    // let args = [];
    // return { command, args };
    log("Running esy status returned the current folder as not a valid esy project. Any invalid esy project can never be a valid opam or bsb project");
    log("Looking for global LSP installations");
  }
}

export async function activate(context: ExtensionContext) {

  process.env.PATH = path.resolve(__dirname, '..', '..', 'node_modules',
    '.bin') + path.delimiter + process.env.PATH;

  let { command, args } = await getCommandForWorkspace();

  // If the extension is launched in debug mode then the debug server options are used
  // Otherwise the run options are used
  let serverOptions: ServerOptions = {
    run: {
      command,
      args,
      options: {
        env: {
          ...process.env,
          OCAMLRUNPARAM: "b",
          MERLIN_LOG: "-"
        }
      }
    },

    debug: {
      command,
      args,
      options: {
        env: {
          ...process.env,
          OCAMLRUNPARAM: "b",
          MERLIN_LOG: "-"
        }
      }
    }
  };

  // Options to control the language client
  let clientOptions: LanguageClientOptions = {
    // Register the server for plain text documents
    documentSelector: [
      { scheme: "file", language: "ocaml" },
      { scheme: "file", language: "reason" }
    ],
    synchronize: {
      // Notify the server about file changes to '.clientrc files contained in the workspace
      fileEvents: workspace.createFileSystemWatcher("**/.clientrc")
    }
  };

  let createClient = () => {
    return new LanguageClient(
      "merlin-language-server",
      "Merlin Language Server",
      serverOptions,
      clientOptions
    );
  };

  // Create the language client and start the client.
  client = createClient();

  // Start the client. This will also launch the server
  client.start();

  commands.registerCommand('merlin-language-server.restart', () => {
    if (client) {
      client.stop();
    }
    activate(context);
  });
}

export function deactivate(): Thenable<void> | undefined {
  if (!client) {
    return undefined;
  }
  return client.stop();
}
