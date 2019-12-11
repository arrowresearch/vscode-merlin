let { promisify } = require("util");
let fs = require("fs");
let readFile = promisify(fs.readFile);
let writeFile = promisify(fs.writeFile);
let mkdir = promisify(fs.mkdir);
let exists = promisify(fs.exists);

module.exports = { readFile, writeFile, mkdir, exists };
