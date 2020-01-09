let register = () => {
  Vscode.Languages.registerDocumentFormattingEditProvider(
    {"scheme": "file", "language": "ocaml"},
    {
      "provideDocumentFormattingEdits": () => {
        let formatter = Some("ocamlformat");
        switch (Vscode.Window.activeTextEditor, formatter) {
        | (None, _)
        | (Some(_), None) => Js.Promise.resolve([||])
        | (Some(textEditor), Some(formatter)) =>
          let id = Uuid.v4()
          let tempFileName =
            Node.Path.join([|
              Node.Os.tmpdir(),
              {j|vscode-reasonml-refmt-$id.ml|j},
            |]);
          Js.Promise.(
            Node.Fs.writeFile(tempFileName, textEditor.document.getText())
            |> then_(f => {
                 let filePath = textEditor.document.fileName;
                 Node.ChildProcess.exec(
                   {j|$formatter --enable-outside-detected-project --name=$filePath $tempFileName|j},
                   Node.ChildProcess.Options.make(),
                 );
               })
            |> then_(((formattedText, error)) => {
                 let textRange =
                   FormatterUtils.getFullTextRange(textEditor.document);
                 Node.Fs.unlink(tempFileName) |> ignore;
                 [|Vscode.TextEdit.replace(textRange, formattedText)|]
                 |> resolve;
               })
            |> catch(e => {
                Node.Fs.unlink(tempFileName) |> ignore;
                 let message = Bindings.Error.ofPromiseError(e);
                 Vscode.Window.showErrorMessage({j|Error: $message|j});
               })
          );
        };
      },
    },
  );
};
