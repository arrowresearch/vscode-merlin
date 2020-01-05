const { ok, fail } = require("assert");
const { run } = require("../../out/utils.js");
const EsyUtils = require("../../out/esy-utils.js");
const rimraf = require("rimraf");
const fs = require("fs");
const path = require("path");
const os = require("os");

/** For Test utils **/
const cp = require("child_process");

describe("command exec utils: run()", function() {
  it("should be able to execute valid bash commands", async function() {
    try {
      await run("echo blah");
      ok(true, "Running a simple bash command should succeed");
    } catch (e) {
      fail("The simple bash command should have failed");
    }
  });

  it("should reject for invalid bash command", async function() {
    try {
      await run("foobar");
      fail("Running invalid bash command (foobar) should have failed");
    } catch (e) {
      ok(true, "Running invalid bash command must reject/throw");
    }
  });

  it("should reject when a valid bash command returns non-zero exit status", async function() {
    try {
      await run("which blah");
      fail("Running which on a non-existent command (blah) must not resolve");
    } catch (e) {
      ok(
        true,
        "Running which on a non-existent command (blah) must always reject"
      );
    }
  });

  it("should be able to run binaries available on the $PATH", async function() {
    try {
      await run("esy status");
      ok(true, "Running a binary available on $PATH must run successfully");
    } catch (e) {
      fail(
        "Running a binary available on $PATH (esy status) should have run without issues"
      );
    }
  });
});

describe("esy wrapper", async function() {
  let tempValidEsyProjectPath, tempInvalidEsyProjectPath;
  before(async function() {
    try {
      tempValidEsyProjectPath = fs.mkdtempSync(
        path.join(os.tmpdir(), "esy-status-valid-test-")
      );
      fs.writeFileSync(
        path.join(tempValidEsyProjectPath, "esy.json"),
        JSON.stringify({ dependencies: {}, name: "foo", version: "0.0.0" })
      );
      tempInvalidEsyProjectPath = fs.mkdtempSync(
        path.join(os.tmpdir(), "esy-status-invalid-test-")
      );
    } catch (e) {
      fail(
        "Error occured during the test setup. Will not be running this suite",
        e
      );
    }
  });
  it("should return a json saying if a given path was a valid esy project or not", async function() {
    let status = await EsyUtils.getEsyStatus(tempValidEsyProjectPath);
    ok(
      status.isProject,
      `must recognise ${tempValidEsyProjectPath} (tempValidEsyProjectPath) as a valid esy project`
    );

    status = await EsyUtils.getEsyStatus(tempInvalidEsyProjectPath);
    ok(
      !status.isProject,
      `must recognise ${tempInvalidEsyProjectPath} (tempInvalidEsyProjectPath) as an invalid esy project`
    );
  });
  after(function() {
    rimraf.sync(tempValidEsyProjectPath);
    rimraf.sync(tempInvalidEsyProjectPath);
  });
});
