let getFullTextRange = (document: Vscode.Window.document) => {
  let firstLine = document.lineAt(0);
  let lastLine = document.lineAt(document.lineCount - 1);

  Vscode.Range.create(
    0,
    firstLine.range.start.character,
    document.lineCount - 1,
    lastLine.range.end_.character,
  );
};

let getFormatterPath = formatter => {
  let rootPath = Vscode.Workspace.rootPath;
  Js.Promise.(
    ProjectType.(
      {
        ProjectType.detect(rootPath)
        |> then_(projectType => {
             let esy = Node.processPlatform == "win32" ? "esy.cmd" : "esy";
             switch (projectType) {
             | Esy(_) => resolve({j|$esy $formatter|j})
             | Bsb(_) =>
               resolve({j|cd $rootPath/.vscode/esy && $esy $formatter|j})
             | Opam => resolve({j|opam exec $formatter|j})
             };
           });
      }
    )
  );
};
