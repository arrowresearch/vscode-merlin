// Generated by BUCKLESCRIPT, PLEASE EDIT WITH CARE
'use strict';

var Os = require("os");
var $$Node = require("../bindings/Node.bs.js");
var Path = require("path");
var Curry = require("bs-platform/lib/js/curry.js");
var Vscode = require("vscode");
var V4 = require("uuid/v4");
var FormatterUtils = require("./FormatterUtils.bs.js");

function register(param) {
  Vscode.languages.registerDocumentFormattingEditProvider({
        scheme: "file",
        language: "ocaml"
      }, {
        provideDocumentFormattingEdits: (function (param) {
            var match = Vscode.window.activeTextEditor;
            if (match !== undefined) {
              var textEditor = match;
              var id = V4();
              var tempFileName = Path.join(Os.tmpdir(), "vscode-reasonml-refmt-" + (String(id) + ".ml"));
              return $$Node.Fs.writeFile(tempFileName, Curry._1(textEditor.document.getText, /* () */0)).then((function (param) {
                                  return FormatterUtils.getFormatterPath("ocamlformat");
                                })).then((function (formatterPath) {
                                var filePath = textEditor.document.fileName;
                                return $$Node.ChildProcess.exec("" + (String(formatterPath) + (" --enable-outside-detected-project --name=" + (String(filePath) + (" " + (String(tempFileName) + ""))))), { });
                              })).then((function (param) {
                              var textRange = FormatterUtils.getFullTextRange(textEditor.document);
                              $$Node.Fs.unlink(tempFileName);
                              return Promise.resolve(/* array */[Vscode.TextEdit.replace(textRange, param[0])]);
                            })).catch((function (e) {
                            $$Node.Fs.unlink(tempFileName);
                            var message = $$Node.$$Error.ofPromiseError(e);
                            return Vscode.window.showErrorMessage("Error: " + (String(message) + ""));
                          }));
            } else {
              return Promise.resolve(/* array */[]);
            }
          })
      });
  return /* () */0;
}

var P = /* alias */0;

exports.P = P;
exports.register = register;
/* os Not a pure module */
