'use strict';

const path = require('path');

module.exports = (_env, argv) => {
  const isProduction = argv.mode == "production";

  return {
    target: 'node',
    entry: './src/extension.bs.js',
    output: {
      path: path.resolve(__dirname, 'dist'),
      filename: 'extension.js'
    },
    devtool: isProduction ? false : 'source-map',
    externals: {
      vscode: 'commonjs vscode'
    },
    resolve: {
      extensions: ['.js']
    }
  }
}
