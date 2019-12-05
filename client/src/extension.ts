/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */

import * as fs from "fs";
import * as path from "path";
import { promisify } from "util";
import { execSync } from "child_process";
import { workspace, ExtensionContext, commands } from "vscode";
let exists = promisify(fs.exists);

import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  TransportKind
} from "vscode-languageclient";

let client: LanguageClient;

async function isEsyProject() {
  let root = workspace.rootPath;
  let esyStatus;
  try {
    esyStatus = JSON.parse(execSync('esy status', { cwd: root }).toString());
  } catch (e) {
    if (e instanceof SyntaxError) {
      console.log('[Plugin Error] Running esy status returned non JSON output', e);
    } else {
      console.log('[Plugin Error] Unknown error while trying to figure if its a valid esy project', e);
    }
  } finally {
    esyStatus = { isProject: false };
  }
  return !!esyStatus.isProject;
}

async function getCommandForWorkspace() {
  if (await isEsyProject()) {
    let command = process.platform === "win32" ? "esy.cmd" : "esy";
    let args = ["exec-command", "--include-current-env", "ocamlmerlin-lsp"];
    return { command, args };
  } else {
    let command =
      process.platform === "win32" ? "ocamlmerlin-lsp.exe" : "ocamlmerlin-lsp";
    let args = [];
    return { command, args };
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
