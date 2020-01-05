class CommandNotFound extends Error {
  constructor(cmd: string) {
    super(`Command ${cmd} not found on path`);
  }
}

export class OpamNotFound extends CommandNotFound {
  constructor() {
    super('opam');
  }
}

export class EsyNotFound extends CommandNotFound {
  constructor() {
    super('esy')
  }
}

export class ToolNotFound extends Error {
  tool: string;
  packageManager: string;
  constructor(packageManager: string, tool: string) {
    super(`${tool} was not available via ${packageManager}`);
    this.tool = tool;
    this.packageManager = packageManager;
  }
}

export class OpamOnWindowsError extends Error {
  constructor() {
    super('Opam workflow on Windows is not supported yet');
  }
}
