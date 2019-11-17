const assert = require("assert");
const path = require("path");
const vscode = require("vscode");
const os = require("os");
const cp = require("child_process");
const fs = require("fs-extra");
const { Uri } = vscode;

let root = path.dirname(path.dirname(__dirname));
let fixtureSrcDir = path.join(root, "fixtures");

suite("Basic tests", () => {
  test("Esy", async () => {
    let sampleEsySrc = path.join(fixtureSrcDir, "sample-esy");
    let projectPath = path.join(os.tmpdir(), "sample-esy");
    let projectUri = Uri.file(projectPath);

    fs.copySync(sampleEsySrc, projectPath);
    cp.execSync("esy", { cwd: projectPath });

    await vscode.commands.executeCommand("vscode.openFolder", projectUri);
    let reasonDocument = await vscode.workspace.openTextDocument(
      Uri.file(path.join(projectPath, "library", "Util.re"))
    );

    let ocamlDocument = await vscode.workspace.openTextDocument(
      Uri.file(path.join(projectPath, "library", "CamlUtil.ml"))
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

    function delay(timeout) {
      return new Promise(resolve => {
        setTimeout(resolve, timeout);
      });
    }

    await delay(500);
    let diagnostics = await vscode.languages.getDiagnostics(
      Uri.file(path.join(projectPath, "library", "Util.re"))
    );
    assert.equal(diagnostics.length, 1, "There should only be one diagnostic");
    assert.equal(diagnostics[0].message, "Warning 26: unused variable foo.");
    assert.equal(
      diagnostics[0].severity,
      1,
      "Severity of this diagnostic should be 1 (Warning). It was " +
        diagnostics[0].severity
    );
    assert.equal(diagnostics[0].range.start.line, 1);
    assert.equal(diagnostics[0].range.start.character, 6);
    assert.equal(diagnostics[0].range.end.line, 1);
    assert.equal(diagnostics[0].range.end.character, 9);

    // TODO: the plugin could support build related tasks
    // const expected = [
    //   { subcommand: "build", group: vscode.TaskGroup.Build },
    //   { subcommand: "run", group: undefined }
    // ];
    // const tasks = await vscode.tasks.fetchTasks();

    console.log("Cleaning up (esy)...");
    try {
      fs.removeSync(projectPath);
    } catch (e) {}
  }).timeout(100000000000); // Ridiculous I know. Sorry!
});
