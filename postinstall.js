let path = require("path");
let cp = require("child_process");
let fs = require("fs");
console.log("Running post install.....");
if (!fs.existsSync("_downloads")) {
  console.log(
    "Could not find _downloads folder where merlin and reason are supposed to be installed."
  );
}

cp.execSync("npm i --prefix _downloads -g esy");

process.env.PATH =
  path.resolve(__dirname, "_downloads", "package", "node_modules", ".bin") +
  path.delimiter +
  path.join(__dirname, "_downloads", "bin") +
  path.delimiter +
  process.env.PATH;

cp.execSync("esy i", {
  stdio: "inherit",
  cwd: path.join("_downloads", "merlin")
});
cp.execSync("esy b dune build -p merlin-lsp", {
  stdio: "inherit",
  cwd: path.join("_downloads", "merlin")
});
cp.execSync("esy", {
  stdio: "inherit",
  cwd: path.join("_downloads", "reason")
});
