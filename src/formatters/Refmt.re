module P = Js.Promise;

let register = () => {
  Vscode.Languages.registerDocumentFormattingEditProvider(
    {"scheme": "file", "language": "reason"},
    {
      "provideDocumentFormattingEdits": () => {
        switch (Vscode.Window.activeTextEditor) {
        | None => Js.Promise.resolve([||])
        | Some(textEditor) =>
          let id = Uuid.v4();
          let tempFileName =
            Node.Path.join([|
              Node.Os.tmpdir(),
              {j|vscode-reasonml-refmt-$id.re|j},
            |]);
          Node.Fs.writeFile(tempFileName, textEditor.document.getText())
          |> P.then_(_ => {FormatterUtils.getFormatterPath("refmt")})
          |> P.then_(formatterPath => {
               Node.ChildProcess.exec(
                 {j|$formatterPath $tempFileName|j},
                 Node.ChildProcess.Options.make(),
               )
             })
          |> P.then_(((formattedText, error)) => {
               let textRange =
                 FormatterUtils.getFullTextRange(textEditor.document);
               Node.Fs.unlink(tempFileName) |> ignore;
               [|Vscode.TextEdit.replace(textRange, formattedText)|]
               |> P.resolve;
             })
          |> P.catch(e => {
               Node.Fs.unlink(tempFileName) |> ignore;
               let message = Bindings.Error.ofPromiseError(e);
               Vscode.Window.showErrorMessage({j|Error: $message|j});
             });
        };
      },
    },
  );
};
