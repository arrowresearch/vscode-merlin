/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */

import * as fs from "fs";
import * as path from "path";
import { workspace, ExtensionContext, commands } from "vscode";

import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
} from "vscode-languageclient";

import { log, run, options } from './utils';
import { getEsyStatus } from './esy-utils';

let client: LanguageClient;

let PackageManager = (function() {

  /* Private variables */
  let $private = {
    esy: 'esy',
    opam: 'opam',
    global: 'bash'
  };

  return {
    init: function() {
      return {
        esy: function esy(cmdString: string, options: options) {
          let cmdStringTokens = cmdString.split(/\s+/);
          return run([$private.esy, "--include$-current-env"].concat(cmdStringTokens).join(' '), options)
        },
        opam: function opam(cmdString: string, options: options) {
          let cmdStringTokens = cmdString.split(/\s+/);
          return run([$private.opam, 'exec'].concat(cmdStringTokens).join(' '), options)
        },
        global: function global(cmdString: string, options: options) {
          return run(cmdString, options);
        }
      }
    }
  }
}())

let { esy, opam, global } = PackageManager.init();


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
    let command = process.platform === "win32" ? "esy.cmd" : "esy";
    let args = ["exec-command", , "ocamlmerlin-lsp"];

    if (fs.existsSync('./package-lock.json') ||
      fs.existsSync('./yarn.lock')) {
      // This is an npm/bsb project
      // So, we'll use the bundled ocamlmerlin-lsp, ocamlmerlin-reason
    }
    return { command, args };
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

  process.env.PATH = path.resolve(__dirname, '..', '..', '_downloads', 'package', 'node_modules',
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
