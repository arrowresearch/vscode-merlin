import { run, options, log } from './utils';

export type esyStatus = { isProject: boolean };

export async function getEsyStatus(root: string): Promise<esyStatus> {
  let esyStatus: esyStatus;
  try {
    let { stdout } = await run(`esy status`, { cwd: root });
    esyStatus = JSON.parse(stdout);
  } catch (e) {
    if (e instanceof SyntaxError) {
      log('Running esy status returned non JSON output', e);
    } else {
      log('Unknown error while trying to figure if its a valid esy project', e);
    }
    return { isProject: false };
  }
  return esyStatus;
}
