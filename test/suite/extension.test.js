const assert = require("assert");
const path = require("path");
const vscode = require("vscode");
const cp = require("child_process");
const { Uri } = vscode;

const fixtureDir = path.resolve(path.join(__dirname, "..", "..", "fixtures"));

suite("Extension Tests", () => {
  test("Basic tests", async () => {
    const projectPath = path.join(fixtureDir, "sample-esy");
    //    cp.execSync("esy", { cwd: projectPath });

    const projectUri = Uri.file(projectPath);

    await vscode.commands.executeCommand("vscode.openFolder", projectUri);
    let reasonDocument = await vscode.workspace.openTextDocument(
      Uri.file(path.join(projectPath, "library", "Util.re"))
    );

    let ocamlDocument = await vscode.workspace.openTextDocument(
      Uri.file(path.join(projectPath, "library", "CamlUtil.ml"))
    );

    let invalidReasonDocument = await vscode.workspace.openTextDocument(
      Uri.file(path.join(projectPath, "library", "InvalidSyntaxReason.re"))
    );

    assert.equal(
      reasonDocument.languageId,
      "reason",
      "Must be identified as a Reason document"
    );

    assert.equal(
      ocamlDocument.languageId,
      "ocaml",
      "Must be identified as an OCaml document"
    );

    let a = await vscode.languages.getDiagnostics(
      Uri.file(path.join(projectPath, "library", "InvalidSyntaxReason.re"))
    );
    console.log(">>>>>>>>>>>>", a.length, "<<<<<");

    // TODO: the plugin could support build related tasks
    // const expected = [
    //   { subcommand: "build", group: vscode.TaskGroup.Build },
    //   { subcommand: "run", group: undefined }
    // ];
    // const tasks = await vscode.tasks.fetchTasks();
  }).timeout(0);
});
