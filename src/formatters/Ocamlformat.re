module P = Js.Promise;

let register = () => {
  Vscode.Languages.registerDocumentFormattingEditProvider(
    {scheme: "file", language: "ocaml"},
    {
      "provideDocumentFormattingEdits": () => {
        switch (Vscode.Window.activeTextEditor) {
        | None => Js.Promise.resolve([||])
        | Some(textEditor) =>
          let id = Uuid.v4();
          let tempFileName =
            Node.Path.join([|
              Node.Os.tmpdir(),
              {j|vscode-reasonml-refmt-$id.ml|j},
            |]);

          Node.Fs.writeFile(tempFileName, textEditor.document.getText())
          |> P.then_(_ => {FormatterUtils.getFormatterPath("ocamlformat")})
          |> P.then_(formatterPath => {
               let filePath = textEditor.document.fileName;
               Node.ChildProcess.exec(
                 {j|$formatterPath --enable-outside-detected-project --name=$filePath $tempFileName|j},
                 Node.ChildProcess.Options.make(),
               );
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
