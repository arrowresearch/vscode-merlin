/* import { execSync } from "child_process";
   import * as fs from "fs";
   import * as os from "os";
   import * as path from "path";
   import { v4 as uuidv4 } from "uuid";
   import * as vscode from "vscode";
   import { getFormatter, getFullTextRange } from "../utils";

   export function register() {
     const configuration = vscode.workspace.getConfiguration("reason");

     vscode.languages.registerDocumentFormattingEditProvider(
       { scheme: "file", language: "reason" },
       {
         async provideDocumentFormattingEdits(_document: vscode.TextDocument): Promise<vscode.TextEdit[]> {

         },
       },
     );
   } */

external provideDocumentFormattingEdits: 'a => unit =
  "provideDocumentFormattingEdits";

let register = () => {
  Vscode.Languages.registerDocumentFormattingEditProvider(
    {"scheme": "file", "language": "reason"},
    {
      "provideDocumentFormattingEdits": () => {
        let formatter = Some("bsrefmt");
        switch (Vscode.Window.activeTextEditor, formatter) {
        | (None, _)
        | (Some(_), None) => Js.Promise.resolve([||])
        | (Some(textEditor), Some(formatter)) =>
          let id = Uuid.v4()
          let tempFileName =
            Node.Path.join([|
              Node.Os.tmpdir(),
              {j|vscode-reasonml-refmt-$id.re|j},
            |]);
          Js.Promise.(
            Node.Fs.writeFile(tempFileName, textEditor.document.getText())
            |> then_(f => {
                 Node.ChildProcess.exec(
                   {j|$formatter $tempFileName|j},
                   Node.ChildProcess.Options.make(),
                 )
               })
            |> then_(((formattedText, error)) => {
                 let textRange =
                   FormatterUtils.getFullTextRange(textEditor.document);
                 Node.Fs.unlink(tempFileName) |> ignore;
                 [|Vscode.TextEdit.replace(textRange, formattedText)|]
                 |> resolve;
               })
            |> catch(e => {
                 let message = Bindings.Error.ofPromiseError(e);
                 Vscode.Window.showErrorMessage({j|Error: $message|j});
               })
          );
        /* const tempFileName = path.join(os.tmpdir(), `vscode-reasonml-refmt-${uuidv4()}.re`);
           fs.writeFileSync(tempFileName, textEditor.document.getText(), "utf8");
           try {
             const formattedText = execSync(`${formatter} ${tempFileName}`).toString();
             const textRange = getFullTextRange(textEditor);
             fs.unlinkSync(tempFileName);
             return [vscode.TextEdit.replace(textRange, formattedText)];
           } catch (e) {
             fs.unlinkSync(tempFileName);
             return [];
           } */
        };
      },
    },
  );
};
