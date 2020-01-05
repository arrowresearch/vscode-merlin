import { execSync, exec } from "child_process";

export type options = { cwd: string };

export function run(cmd: string, options?: options): Promise<{ stdout: string, stderr: string }> {
  return new Promise((resolve, reject) => {
    exec(cmd, options, (err, stdout, stderr) => {
      if (err) {
        reject(err);
      } else {
        resolve({
          stdout: stdout.toString(),
          stderr: stderr.toString()
        });
      }
    });
  });
};

export function log(...m) {
  console.log('[Plugin]', ...m);
};
