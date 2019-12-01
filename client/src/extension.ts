/* --------------------------------------------------------------------------------------------
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Licensed under the MIT License. See License.txt in the project root for license information.
 * ------------------------------------------------------------------------------------------ */

import * as fs from 'fs';
import * as path from 'path';
import { promisify } from 'util';
// eslint-disable-next-line import/no-unresolved
import { commands, ExtensionContext, workspace } from 'vscode';
import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
} from 'vscode-languageclient';

const exists = promisify(fs.exists);

let client: LanguageClient;

async function isEsyProject(): Promise<boolean> {
  const root = workspace.rootPath;
  if (await exists(path.join(root, 'package.json'))) {
    return true;
  } else if (await exists(path.join(root, 'esy.json'))) {
    return true;
  } else {
    return false;
  }
}

async function getCommandForWorkspace(): Promise<{
  command: string;
  args: Array<string>;
}> {
  if (await isEsyProject()) {
    const command = process.platform === 'win32' ? 'esy.cmd' : 'esy';
    const args = ['exec-command', '--include-current-env', 'ocamlmerlin-lsp'];
    return { command, args };
  } else {
    const command =
      process.platform === 'win32' ? 'ocamlmerlin-lsp.exe' : 'ocamlmerlin-lsp';
    const args = [];
    return { command, args };
  }
}

export async function activate(context: ExtensionContext): Promise<void> {
  const { command, args } = await getCommandForWorkspace();

  // If the extension is launched in debug mode then the debug server options are used
  // Otherwise the run options are used
  const serverOptions: ServerOptions = {
    run: {
      command,
      args,
      options: {
        env: {
          ...process.env,
          OCAMLRUNPARAM: 'b',
          MERLIN_LOG: '-',
        },
      },
    },

    debug: {
      command,
      args,
      options: {
        env: {
          ...process.env,
          OCAMLRUNPARAM: 'b',
          MERLIN_LOG: '-',
        },
      },
    },
  };

  // Options to control the language client
  const clientOptions: LanguageClientOptions = {
    // Register the server for plain text documents
    documentSelector: [
      { scheme: 'file', language: 'ocaml' },
      { scheme: 'file', language: 'reason' },
    ],
    synchronize: {
      // Notify the server about file changes to '.clientrc files contained in the workspace
      fileEvents: workspace.createFileSystemWatcher('**/.clientrc'),
    },
  };

  const createClient = (): LanguageClient =>
    new LanguageClient(
      'merlin-language-server',
      'Merlin Language Server',
      serverOptions,
      clientOptions,
    );

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
