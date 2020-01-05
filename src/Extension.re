open Vscode;
open LSP;

let createClient = (~id, ~name, ~folder) =>
  Js.Promise.(
    Server.make(folder)
    |> then_(serverOptions => {
         LanguageClient.make(
           ~id,
           ~name,
           ~serverOptions,
           ~clientOptions=Client.make(),
         )
         |> resolve
       })
  );

let activate = _context => {
  Js.Promise.(
    createClient(
      ~id="merlin-language-server",
      ~name="Merlin Language Server",
      ~folder=Workspace.rootPath,
    )
    |> then_((client: LanguageClient.t) => client.start(.) |> resolve)
    |> catch(e => {
         let message = Bindings.Error.ofPromiseError(e);
         Window.showErrorMessage({j|Error: $message|j});
       })
  );
};
