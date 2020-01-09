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
