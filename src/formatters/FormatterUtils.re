module P = Js.Promise;

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
  ProjectType.detect(rootPath)
  |> P.then_(projectType => {
       let esy = Node.processPlatform == "win32" ? "esy.cmd" : "esy";
       switch (projectType) {
       | ProjectType.Esy(_) => P.resolve({j|$esy $formatter|j})
       | Bsb(_) =>
         P.resolve({j|$esy -P $rootPath/.vscode/esy $formatter|j})
       | Opam => P.resolve({j|opam exec $formatter|j})
       };
     });
};
