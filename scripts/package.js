let https = require("https");
let fs = require("fs");
let path = require("path");
let cp = require("child_process");

let log = m => console.log(m);

// if (!fs.existsSync("_downloads")) {
//   fs.mkdirSync("_downloads");
// }

// function fetch(url, file) {
//   cp.execSync(`curl -LO ${url}`);
// }

// let esyVersion = "0.5.8";
// let esyTarBall = `esy-${esyVersion}.tgz`;

// function fetchEsyTarBall() {
//   log("Downloading esy from npm...");
//   return fetch(`https://registry.npmjs.org/esy/-/esy-${esyVersion}.tgz`);
// }

function fetchMerlinLSP() {
  log("Downloading merlin-lsp from github...");
  cp.execSync("git clone https://github.com/ocaml/merlin _downloads/merlin");
}

function fetchReason() {
  log("Downloading reason from github...");
  cp.execSync("git clone https://github.com/facebook/reason _downloads/reason");
}

// if (!fs.existsSync(esyTarBall)) {
//   fetchEsyTarBall();
// }
// cp.execSync(`tar xf ${esyTarBall} -C _downloads`);
if (!fs.existsSync(path.join("_downloads", "merlin"))) {
  fetchMerlinLSP();
}
if (!fs.existsSync(path.join("_downloads", "reason"))) {
  fetchReason();
}
