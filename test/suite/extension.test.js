const assert = require("assert");
const path = require("path");
const vscode = require("vscode");
const os = require("os");
const cp = require("child_process");
const fs = require("fs-extra");
const { Uri } = vscode;

let root = path.dirname(path.dirname(__dirname));
let fixtureSrcDir = path.join(root, "fixtures");

suite("Sample esy project", () => {
  let sampleEsySrc = path.join(fixtureSrcDir, "sample-esy");
  let projectPath = path.join(os.tmpdir(), "sample-esy");

  suiteSetup(function() {
    this.timeout(100000);
    fs.copySync(sampleEsySrc, projectPath);
    cp.execSync("esy", { cwd: projectPath });
  });

  suiteTeardown(() => {
    try {
      fs.removeSync(projectPath);
    } catch (e) {}
  });

  test("Basic tests", async () => {
    const projectUri = Uri.file(projectPath);

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
  }).timeout(0);
}).timeout(5000);

suite("Sample opam project", function() {
  if (process.platform == "win32" || process.platform == "win64") {
    return;
  }

  let sampleOpamSrc = path.join(fixtureSrcDir, "sample-opam");
  let projectPath = path.join(os.tmpdir(), "sample-opam");
  let opamRoot = path.join(os.tmpdir(), "opam-root");

  suiteSetup(function() {
    this.timeout(10000000);
    fs.copySync(sampleOpamSrc, projectPath);
    cp.execSync(`mkdir -p ${opamRoot}`);
    let env = cp
      .execSync(
        `sh -c 'export OPAMROOT=${opamRoot}; opam init --yes > /dev/null; opam switch create e2e 4.08.1 > /dev/null; eval $(opam env); opam pin add --yes merlin-lsp.dev https://github.com/ocaml/merlin.git > /dev/null; opam install . --deps-only --yes > /dev/null; opam env'`,
        { cwd: projectPath }
      )
      .toString();
    let regexpMatch = env.match(/PATH=[^;]+;/g);
    if (regexpMatch.length >= 1) {
      process.env["PATH"] = Array.prototype.reduce.call(
        regexpMatch,
        function(acc, pathString) {
          return (
            acc +
            pathString
              .replace(/[;'"]/g, "")
              .split("=")[1]
              .split(":")
              .concat(process.env["PATH"].split("=")[1])
              .join(":")
          );
        },
        ""
      );
    }
  });

  suiteTeardown(() => {
    this.timeout(10000000);
    console.log("Cleaning up...");
    try {
      console.log("  Removing switch");
      console.log(cp.execSync("opam switch remove e2e --yes").toString());
      console.log("  Removing test project");
      fs.removeSync(projectPath);
    } catch (e) {}
  });

  test("Basic tests", async () => {
    const projectUri = Uri.file(projectPath);
    await vscode.commands.executeCommand("vscode.openFolder", projectUri);
    let reasonDocument = await vscode.workspace.openTextDocument(
      Uri.file(path.join(projectPath, "foo.re"))
    );

    let ocamlDocument = await vscode.workspace.openTextDocument(
      Uri.file(path.join(projectPath, "main.ml"))
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
  });
}).timeout(0);
