// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';

var LSP = require("./LSP.bs.js");
var $$Node = require("./bindings/Node.bs.js");
var Refmt = require("./formatters/Refmt.bs.js");
var Vscode = require("vscode");
var Ocamlformat = require("./formatters/Ocamlformat.bs.js");
var VscodeLanguageclient = require("vscode-languageclient");

function createClient(id, name, folder) {
  return LSP.Server.make(folder).then((function (serverOptions) {
                return Promise.resolve(new VscodeLanguageclient.LanguageClient(id, name, serverOptions, LSP.Client.make(/* () */0)));
              }));
}

function activate(_context) {
  Refmt.register(/* () */0);
  Ocamlformat.register(/* () */0);
  return createClient("merlin-language-server", "Merlin Language Server", Vscode.workspace.rootPath).then((function (client) {
                  return Promise.resolve(client.start());
                })).catch((function (e) {
                var message = $$Node.$$Error.ofPromiseError(e);
                return Vscode.window.showErrorMessage("Error: " + (String(message) + ""));
              }));
}

exports.activate = activate;
/* LSP Not a pure module */
